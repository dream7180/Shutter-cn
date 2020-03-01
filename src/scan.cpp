/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Data.h"
#include "Markers.h"
#include "ExifTags.h"
#include "ExifFields.h"
#include "Config.h"
#include "MemoryDataSource.h"
#include "JPEGDecoder.h"
#include "AutoPtr.h"
#include "MakerNote.h"
#include "MemPointer.h"
#include "PhotoAttr.h"
#include "GPSData.h"
#include "OutputStr.h"
#include "PhotoInfo.h"
#include "JPEGException.h"
#include "PhotoAttrAccess.h"
#include "CatchAll.h"
#include "ColorProfile.h"
#include "ExifBlock.h"
#include "StringConversions.h"
#include "ReadMarker.h"
#include "IPTCReadWrite.h"
#include "XmpAccess.h"
#include "XmpData.h"
#include "scan.h"
#include "PhotoInfoPtr.h"
#include "DateTimeUtils.h"
#include "ImgLogger.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#ifdef _DEBUG
	#define DBG_ONLY(a)		{}// a; }
#else
	#define DBG_ONLY(a)		{}
#endif

extern std::pair<uint32, uint32> ParseCRW(FileStream& ifs, uint32 offset, uint32 length, uint16& orientation);


void RecordInfo(uint16 tag, const Data& val, String& rstrOutput)
{
	oStringstream ost;
	ost << std::hex;
	ost.width(4);
	ost.fill(_T('0'));
	ost << int(tag);
	ost << _T("\t") << TagName(tag);
	// record data; treat user comment (0x9286) as string in case of undefined type (tUNDEF)
	ost << _T("\t") << val.AsString(true, tag == EXIF_USER_DESC).c_str();

	rstrOutput.append(ost.str());
	rstrOutput.append(_T("\r\n"));
}



AutoPtr<MakerNote> FindMakerNote(const String& make, const String& model, ConstPhotoInfoPtr inf)
{
	if (make == _T("Canon"))
		return new CanonNote(model);
	else if (make == _T("NIKON") || make == _T("NIKON CORPORATION"))
		return new NikonNote();
	else if (make == _T("CASIO"))
		return new CasioNote();

	return 0;
}


void ReadMakerNote(FileStream& ifs, Offset ifd_start, Data& val, OutputStr* output, String make, String model, PhotoInfoPtr& info)
{
	if (!val.IsUndefData() || val.Length() < 20)
		return;

	OutputStr dummy;
	if (output == 0)
		output = &dummy;

	AutoPtr<MakerNote> note;
	bool original_order= ifs.GetByteOrder();
	Offset temp_pos= ifs.RPosition();

	const int MAX= 16;
	char start[MAX + 1];
	val.ReadChar(start, MAX);
	start[MAX] = 0;

	if (strcmp(start, "OLYMP") == 0)
	{
		note = new OlympusNote();

		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset + 8);
	}
	//else if (strcmp(start, "OLYMPUS") == 0)		// this new Oly MakerNote format uses non-standard tag entries...
	//{
	//	note = new OlympusNote();

	//	Offset offset= val.GetData();
	//	ifs.RPosFromBeg(ifd_start + offset + 8);
	//}
	else if (strcmp(start, "Nikon") == 0)
	{
		if (start[6] >= 2 && start[10] == start[11] && (start[10] == 'M' || start[10] == 'I'))
		{
			ifs.SetByteOrder(start[10] == 'M');

			Offset offset= val.GetData();
			ifs.RPosFromBeg(ifd_start + offset + 12);

			if (ifs.GetUInt16() == 0x2a)	// has to be 0x2a (this is IFD header)
			{
				ifs.RPosition(ifs.GetUInt32() - 8);
			// offset to the field count
//			ifs.RPosition(ifd_start, offset + 18, false);

				// unlike all the rest of maker notes this particular one has offsets to data
				// relative to the maker note itself rather than the whole file (absolute);
				// this MakerNote is a separate IFD
				ifd_start += offset + 10;

				note = new NikonNote(true);
			}
		}
		else
		{
			note = new NikonNote(false);	// old format

			Offset offset= val.GetData();
			ifs.RPosFromBeg(ifd_start + offset + 8);
		}
	}
	else if (strncmp(start, "FUJIFILM", 8) == 0)
	{
		// offset to IFD in Intel format follows:
		if (start[8] == 0x0c && start[9] == 0x00 &&
			start[10] == 0x00 && start[11] == 0x00)
		{
			note = new FujiNote();
			ifs.SetByteOrder(false);	// low-endian order for FujiNote

			Offset offset= val.GetData();
			ifs.RPosFromBeg(ifd_start + offset);

			ifs.RPosition(8 + 4);		// skip "FUJIFILM" name and offset
		}
	}
	else if (strncmp(start, "SONY DSC ", 9) == 0 || strncmp(start, "SONY CAM ", 9) == 0)
	{
		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset + 12);

		// Sony MakerNote

		//TODO: for the time disabled: Sony's MakerNote remains a mystery
		note = 0; //new SonyNote();
	}
	else if (memcmp(start, "AOC\0", 4) == 0)	// Pentax MakerNote?
	{
		int skip= 2;
		if (start[4] == 'M' && start[5] == 'M')
			ifs.SetByteOrder(true);
		else if (start[4] == 'I' && start[5] == 'I')
			ifs.SetByteOrder(true);
		else
		{
			//skip = 0;
			//ifs.SetByteOrder(false);
			// big endian?
			ifs.SetByteOrder(start[6] == 0 && start[7] != 0);
		}

		note = new PentaxNote();

		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset + 4 + skip);
	}
	else if (strcmp(start, "SANYO") == 0)
	{
		note = new SanyoNote();

		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset + 8);
	}
	else if (memcmp(start, "QVC\0", 4) == 0)	// new Casio MakerNote?
	{
		note = new Casio2Note();

		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset + 6);

		ifs.SetByteOrder(true);	// always Motorola(?)
	}
	else if (strcmp(start, "Panasonic") == 0)
	{
		note = new PanasonicNote();

		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset + 12);
	}
	else
	{
		uint8 first= start[0];
		uint8 snd= start[1];
		bool intel_order= false;

		if (first != 0 && snd == 0)
			intel_order = true;
		else if (first == 0 && snd != 0)
			intel_order = false;
		else
			return;

		note = FindMakerNote(make, model, info);

		if (note.get() == 0)
			return;

		Offset offset= val.GetData();
		ifs.RPosFromBeg(ifd_start + offset);

		ifs.SetByteOrder(!intel_order);
	}

	if (note)
	{
		uint16 entries= ifs.GetUInt16();	// no of entries in IFD

		if (entries < 100)					// sanity check
		{
			//{{ HACK for Sony DSC-V1
			if (make == _T("SONY") && model == _T("DSC-V1") && entries == 12)
				entries = 8;	// this camera lies about real amount of entries
			// end of HACK }}

			// MakerNote begin
			output->Indent();

			for (uint32 i= 0; i < entries; ++i)
			{
				uint16 tag= ifs.GetUInt16();
				Data val(ifs, ifd_start);
				note->RecordInfo(tag, val, *output);
			}

			// MakerNote end
			output->Outdent();
		}
	}

	if (info)
	{
		// store MakerNote
		info->SetMakerNote(note);

		if (info->GetMakerNote())
			info->GetMakerNote()->CompletePhotoInfo(*info);
	}

	// restore byte order
	ifs.SetByteOrder(original_order);

	// restore position
	ifs.RPosition(temp_pos, FileStream::beg);
}


AutoPtr<GPSData> ReadGPSInfo(FileStream& ifs, Offset ifd_start, Data& val, OutputStr* output)
{
	if (val.IsUndefData()) // || val.Length() < 20)
		return 0;

	Offset temp_pos= ifs.RPosition();

	Offset offset= val.GetData();
	ifs.RPosFromBeg(ifd_start + offset);

	uint16 entries= ifs.GetUInt16();	// no of entries in IFD

	if (entries < 2)					// there might be GPS version recorded only, so skip it
	{
		// restore position
		ifs.RPosition(temp_pos, FileStream::beg);
		return 0;
	}

	ifs.RPosition(-2);					// read no of entries again

	AutoPtr<GPSData> gps= new GPSData(ifs, ifd_start, output);

	// restore position
	ifs.RPosition(temp_pos, FileStream::beg);

	return gps;
}


bool ReadDescription(const Data& val, std::wstring& description);


// read EXIF date/time ("2012:08:15 08:41:03")

//static CTime GetPhotoDateTime(const String& date_time)
//{
//	try
//	{
//		// in case of HP PhotoSmart C850 d/t contains spaces (not so smart, but legal)
//
//		if (date_time.length() >= 19 && date_time[0] != _T(' '))
//			if (const TCHAR* p= date_time.c_str())
//				return CTime(_ttoi(p), _ttoi(p + 5), _ttoi(p + 8), _ttoi(p + 11), _ttoi(p + 14), _ttoi(p + 17));
//	}
//	catch (...)
//	{}
//
//	return CTime();
//}


extern String TrimSpaces(const String& str)
{
	if (str.empty())
		return str;

	String::size_type i= 0;
	while (str[i] == _T(' '))
		++i;

	String::size_type j= str.length() - 1;
	while (j > i && str[j] == _T(' '))
		--j;

	return str.substr(i, j - i + 1);
}


static uint32 ReadEntry(FileStream& ifs, Offset ifd_start, String& make, String& model, PhotoInfoPtr& info, OutputStr* output, FieldCallback* field_callback)
{
	uint16 tag= ifs.GetUInt16();
	Data val(ifs, ifd_start);

	if (field_callback)
		(*field_callback)(ifs, tag, val);

	if (output)
		output->RecordInfo(tag, TagName(tag), val);

	switch (tag)
	{
	case 0x010e:
		if (info && info->photo_desc_.empty())
		{
#ifdef _UNICODE
			info->photo_desc_ = TrimSpaces(val.AsString());
#else
			std::string str= TrimSpaces(val.AsString());
			MultiByteToWideString(str, info->photo_desc_);
#endif
		}
		break;
	case 0x010f:
		make = TrimSpaces(val.AsString());
		if (info) info->SetMake(make);					break;
	case 0x0110:
		model = TrimSpaces(val.AsString());
		if (info) info->SetModel(model);				break;
	case EXIF_DateTime: // 0x0132
		{
			auto tm= ExifDateToDateTime(val.AsString());
			if (info) info->SetDateTime(tm);
			if (output)	output->SetInterpretedInfo(::DateTimeFmt(tm));		break;
		}
	case EXIF_ORIENTATION: // 0x0112
		if (info) info->SetOrientation(val.GetData());
		if (output) output->SetInterpretedInfo(::Orientation(val.GetData()));		break;
	case 0x8822:
		if (info) info->SetExposureProgram(val.GetData());
		if (output) output->SetInterpretedInfo(::ExposureProgram(val.GetData()));	break;
	case 0x8827:
		if (info) info->SetISOSpeed(val.GetData());
		if (output) output->SetInterpretedInfo(::ISOSpeed(val.GetData()));			break;
	case 0x829a:
		{
			Rational et= val.Rational();
			if (info) info->SetExposureTime(et);
			if (output) output->SetInterpretedInfo(::ExposureTime(et));		break;
		}
	case 0x829d:
		{
			Rational fn= val.Rational();
			if (info) info->SetFStop(fn);
			if (output) output->SetInterpretedInfo(::FNumber(fn));			break;
		}
	case 0x9204:
		{
			Rational eb= val.Rational();
			if (info) info->SetExposureBias(eb);
			if (output) output->SetInterpretedInfo(::ExposureBias(eb));		break;
		}
	case 0x9207:
		if (info) info->SetMeteringMode(val.GetData());
		if (output) output->SetInterpretedInfo(::MeteringMode(val.GetData()));		break;
	case 0x9208:
		if (info) info->SetLightSource(val.GetData());
		if (output) output->SetInterpretedInfo(::LightSource(val.GetData()));		break;
	case 0x9209:
		if (info) info->SetFlash(val.GetData());
		if (output) output->SetInterpretedInfo(::Flash(val.GetData()));				break;
	case 0x920a:
		{
			Rational fl= val.Rational();
			if (info) info->SetFocalLength(fl);
			if (output) output->SetInterpretedInfo(::FocalLength(fl));		break;
		}
	case EXIF_IMG_WIDTH:
		if (info) info->SetWidth(val.GetData());
		if (output) output->SetInterpretedInfo(::PhotoWidth(val.GetData()));		break;
	case EXIF_IMG_HEIGHT:
		if (info) info->SetHeight(val.GetData());
		if (output) output->SetInterpretedInfo(::PhotoHeight(val.GetData()));		break;
	case 0x9201:
		{
			SRational sv= val.SRational();
			if (info) info->SetShutterSpeedValue(sv);
			if (output) output->SetInterpretedInfo(ExposureValAndShutterSpeed(sv));	break;
		}
	case 0x9202:
		{
			Rational av= val.Rational();
			if (info) info->SetApertureValue(av);
			if (output) output->SetInterpretedInfo(ApertureValAndFNum(av));	break;
		}
	case 0x9206:
		{
			Rational sd= val.Rational();
			if (info) info->SetSubjectDistance(sd);
			if (output) output->SetInterpretedInfo(SubjectDistance(sd));	break;
		}
	case 0x927c:
		ReadMakerNote(ifs, ifd_start, val, output, make, model, info);
		break;
	case EXIF_USER_DESC:
		{
			std::wstring str;
			if (ReadDescription(val, str))
				if (info)
					info->SetExifDescription(str);
#ifdef _UNICODE
			if (output)
				output->SetInterpretedInfo(str);
#else
			std::string s;
			WideStringToMultiByte(str, s);
			if (output)
				output->SetInterpretedInfo(s);
#endif
		}
		break;
	case EXIF_DateTimeOriginal:		// sometimes date & time tag (0x0132) is missing (Ricoh RDC-7)
		{
			auto tm= ExifDateToDateTime(val.AsString());
			// use date of photo creation instead of modification (Photoshop modifies field '0132')
			if (!tm.is_not_a_date_time())
				if (info) info->SetDateTime(tm);
			if (output) output->SetInterpretedInfo(DateTimeFmt(tm));
		}
		break;
	case EXIF_DateTimeDigitized:	//sometimes date & time tag (0x0132) is missing (Ricoh RDC-7)
		{
			auto tm= ExifDateToDateTime(val.AsString());
			if (info && info->GetDateTime().is_not_a_date_time() && !tm.is_not_a_date_time())
				info->SetDateTime(tm);
			if (output) output->SetInterpretedInfo(DateTimeFmt(tm));
		}
		break;

	case EXIF_SubSecTimeOriginal:	// sub seconds for original time
		if (info) info->SetSubSeconds(_ttoi(val.AsString().c_str()));	// this value will be later combined with EXIF time stamp
		break;

	case EXIF_GPSInfo_IFD_Pointer:	// GPSInfo
		{
			// decode GPS block; fill in 'output' in the process
			AutoPtr<GPSData> gps= ReadGPSInfo(ifs, ifd_start, val, output);
			if (info) info->SetGpsData(gps);
		}
		break;

	case 0x0002:	if (output) output->SetInterpretedInfo(InteroperabilityVersion(val));		break;
	case 0x0103:	if (output) output->SetInterpretedInfo(ImgCompressionType(val.GetData()));	break;
	case 0x0106:	if (output) output->SetInterpretedInfo(PhotometricInterpretation(val.GetData()));	break;
	case 0x011a:	if (output) output->SetInterpretedInfo(val.AsDouble());						break;
	case 0x011b:	if (output) output->SetInterpretedInfo(val.AsDouble());						break;
	case 0x0128:	if (output) output->SetInterpretedInfo(ResolutionUnit(val.GetData()));		break;
	case 0x9000:	if (output) output->SetInterpretedInfo(ExifVersion(val));					break;
	case 0x9101:	if (output) output->SetInterpretedInfo(ComponentConfiguration(val));		break;
	case 0x9203:	if (output) output->SetInterpretedInfo(BrightnessValue(val.SRational()));	break;
	case 0x9205:	if (output) output->SetInterpretedInfo(ApertureValAndFNum(val.Rational()));	break;	// MaxApertureValue
		//		case 0x9286:	if (output) output->SetInterpretedInfo(val.AsString(false, true));			break;	// user comment
	case 0xa000:	if (output) output->SetInterpretedInfo(FlashPixVersion(val));				break;
	case 0xa001:
		if (output)
			output->SetInterpretedInfo(ColorSpace(val.GetData()));
		if (info)
			info->SetColorSpace(static_cast<uint16>(val.AsULong()));
		break;
	case 0xa20b:	if (output) output->SetInterpretedInfo(val.Rational().AsString() + _T(" Beam Candle Power Seconds"));	break;
	case 0xa20e:	if (output) output->SetInterpretedInfo(val.AsDouble());						break;
	case 0xa20f:	if (output) output->SetInterpretedInfo(val.AsDouble());						break;
	case 0xa210:	if (output) output->SetInterpretedInfo(ResolutionUnit(val.GetData()));		break;
	case 0xa217:	if (output) output->SetInterpretedInfo(SensingMethod(val.GetData()));		break;
	case 0xa300:	if (output) output->SetInterpretedInfo(FileSource(val.GetData()));			break;
	case 0xa302:
		if (val.IsLongData())
		{
			Offset temp= ifs.RPosition();
			ifs.RPosFromBeg(ifd_start + val.GetData());
			String pattern;
			if (CFAPattern(ifs, val.Components(), pattern))
				if (output) output->SetInterpretedInfo(pattern);
			ifs.RPosition(temp, FileStream::beg);
		}
		break;
	case 0xa401:	if (output) output->SetInterpretedInfo(CustomRendered(val.GetData()));		break;
	case 0xa402:	if (output) output->SetInterpretedInfo(ExposureMode(val.GetData()));		break;
	case 0xa403:	if (output) output->SetInterpretedInfo(WhiteBalance(val.GetData()));		break;
	case 0xa404:	if (output) output->SetInterpretedInfo(val.Rational().AsString());			break;
	case 0xa406:	if (output) output->SetInterpretedInfo(SceneCaptureType(val.GetData()));	break;
	case 0xa407:	if (output) output->SetInterpretedInfo(GainControl(val.GetData()));			break;
	case 0xa408:	if (output) output->SetInterpretedInfo(Contrast(val.GetData()));			break;
	case 0xa409:	if (output) output->SetInterpretedInfo(Saturation(val.GetData()));			break;
	case 0xa40a:	if (output) output->SetInterpretedInfo(Sharpness(val.GetData()));			break;
	case 0xa40c:	if (output) output->SetInterpretedInfo(SubjectDistanceRange(val.GetData()));	break;
	case 0xa434:	if (info) info->SetLensModel(TrimSpaces(val.AsString()));					break;


		// TIFF/EP
	case 0x9217:	if (output) output->SetInterpretedInfo(SensingMethod(val.GetData()));		break;

	case EXIF_WIN_F1:
	case EXIF_WIN_F2:
	case EXIF_WIN_F3:
	case EXIF_WIN_F4:
	case EXIF_WIN_F5:
		if (output) output->SetInterpretedInfo(val.AsUnicodeString());
		break;

		//		case 0xa301:
		//			/*info->scene_type_ = val.Rational(); */	output->SetInterpretedInfo(SceneType(val.GetData()));	break;
		//			break;
	}

	if (tag == EXIF_SUB_IFD)
		return val.GetData();
	else if (tag == EXIF_SUB_INTEROP)
		return val.GetData();
	else
		return 0;
}


extern uint32 ReadEntry(FileStream& ifs, Offset ifd_start, String& make, String& model, PhotoInfoPtr info, OutputStr& output)
{
	return ReadEntry(ifs, ifd_start, make, model, info, &output, 0);
}


void FindThumbnail(FileStream& ifs, Offset ifd_start, uint32& thumbnail_offset, uint32& thumbnail_size, OutputStr* output)
{
	uint16 tag= ifs.GetUInt16();
	Data val(ifs, ifd_start);

	if (output)
		output->RecordInfo(tag, TagName(tag), val);

//	if (info)
	{
		switch (tag)
		{
		case 0x0103:	if (output) output->SetInterpretedInfo(ImgCompressionType(val.GetData()));	break;
		case 0x0128:	if (output) output->SetInterpretedInfo(ResolutionUnit(val.GetData()));		break;
		}
	}

	if (tag == 0x0201)				// JpegIFOffset
		thumbnail_offset = val.GetData();
	else if (tag == 0x0202)		// JpegIFByteCount
		thumbnail_size = val.GetData();
}


static bool AssembleColorProfile(const std::vector<std::vector<uint8>>& icc_profile, std::vector<uint8>& profile_data)
{
	size_t count= icc_profile.size();
	size_t total_size= 0;

	for (size_t i= 0; i < count; ++i)
		total_size += icc_profile[i].size();

	profile_data.resize(total_size);
	std::vector<uint8>::iterator it= profile_data.begin();

	for (size_t i= 0; i < count; ++i)
	{
		copy(icc_profile[i].begin(), icc_profile[i].end(), it);
		it += icc_profile[i].size();
	}

	return !profile_data.empty();
}


// this function knows about a few markers that are commonly used in the JPEG 'header';
// those markers may be skipped when searching for app marker 13
//
extern bool IsJpegMarkerToBeSkipped(uint16 marker)
{
	if (marker >= MARK_APP0 && marker <= MARK_APP15)
		return true;

	switch (marker)
	{
	case MARK_COM:
//careful here: monitor amount of jpeg stored in a database!
//	case MARK_DQT:
//	case MARK_DNL:
//	case MARK_DRI:
		return true;

	case MARK_DQT:		// quantization table (it's small); GISTEQ Photo Tracker inserts it before EXIF block
		return true;

	default:
		return false;
	}
}


static uint32 ReadPhotoMarkers(FileStream& ifs, PhotoInfoPtr& info, ImgLogger* logger, const TCHAR* file)
{
	if (info) info->photo_desc_.erase();

	std::vector<std::vector<uint8>> icc_profile;
	bool icc_profile_valid= true;

	Offset temp= ifs.RPosition();

	bool original_order= ifs.GetByteOrder();

	bool result= false;

	XmpData xmpData;
	bool hasXmp= false;

//	for (uint16 app_marker= MARK_APP0; app_marker <= MARK_APP13; ++app_marker)
	for (int i= 0; i < 1000; ++i)	// safety counter
	{
		// restore byte order (if any marker reading proc changed it)
		ifs.SetByteOrder(original_order);

		// read marker
		uint16 marker= ReadMarker(ifs);

		// looking for app markers only (and make an exception for comment marker)
		if (!IsJpegMarkerToBeSkipped(marker))
			break;

		// data length
		uint16 dataSize= ifs.GetUInt16();

		// current pos to skip this marker
		Offset cur_pos= ifs.RPosition();

		if (dataSize < 2)
			break;	// not a valid length

		switch (marker)
		{
		case MARK_APP1:		// XMP metadata?
			if (info && dataSize > Xmp::HEADER_NAMESPACE_LEN)
			{
				ASSERT(Xmp::HEADER_NAMESPACE_LEN < 50);
				char buf[50];
				ifs.Read(buf, Xmp::HEADER_NAMESPACE_LEN);
				if (memcmp(buf, Xmp::HEADER_NAMESPACE, Xmp::HEADER_NAMESPACE_LEN) == 0)
				{
					try
					{
						std::vector<char> xmp_buf(dataSize - Xmp::HEADER_NAMESPACE_LEN, 0);
						ifs.Read(&xmp_buf.front(), xmp_buf.size());
						if (Xmp::RemoveXPacket(xmp_buf))
						{
							XmpData xmp;
							Xmp::MetaToXmpData(xmp_buf, xmpData);
							hasXmp = true;
						}
					}
					catch (Exception& ex)
					{
						if (logger)
							logger->AddEntry(ex.GetDescription(), file);
					}
					catch (Xmp::XMP_Error& err)
					{
						if (logger)
							logger->AddEntry(CString(err.GetErrMsg()), file);
					}
					catch (String& s)
					{
						if (logger)
							logger->AddEntry(s.c_str(), file);
					}
					catch (std::exception& ex)
					{
						if (logger)
							logger->AddEntry(CString(ex.what()), file);
					}
				}
			}
			break;

		case MARK_APP2:		// embedded ICC Profile?
			{
				const uint16 OVERHEAD= 14;

				if (dataSize <= OVERHEAD)
					break;

				uint8 buf[OVERHEAD];
				ifs.Read(&buf[0], OVERHEAD);

				if (memcmp(&buf[0], "ICC_PROFILE", 12) == 0 && icc_profile_valid)
				{
					uint8 chunk= buf[12];
					uint8 count= buf[13];

					// color profile may come in chunks
					if (chunk > 0 && count > 0 && chunk <= count)
					{
						icc_profile.resize(count);
						int index= chunk - 1;
						if (icc_profile[index].empty())
						{
							uint32 size= dataSize - OVERHEAD;
							icc_profile[index].resize(size);
							ifs.Read(&icc_profile[index][0], size);
						}
						else
						{
							// this is error in profile data; typical for Corel Photo Paint 11 for instance
							icc_profile_valid = false;
						}
					}
				}
			}
			break;

		case MARK_APP6:		// my data: description, rotation flags
			if (dataSize >= PhotoAttrAccess::GetAppMarkerSize())
			{
				PhotoAttr pa;
				ifs.Read(reinterpret_cast<char*>(&pa), sizeof pa);
				if (pa.IsValid())
				{
					int len= PhotoAttr::MAX_DESC_LEN;
					for (int i= 0; i < PhotoAttr::MAX_DESC_LEN; ++i)
					{
						if (pa.description_[i] == 0)
						{
							len = i;
							break;
						}
					}
					ASSERT(sizeof(wchar_t) == sizeof(uint16));
					if (info && info->photo_desc_.empty())
						info->photo_desc_.assign(reinterpret_cast<wchar_t*>(pa.description_), len);
					if (info) info->SetThumbnailOrientation(pa.orientation_info_);

					result = true;
				}
			}
			break;

		case MARK_APP13:	// Photoshp IPTC info?
			if (info && dataSize > 20)
			{
				char PS_marker[g_photoshop_len];
				ifs.Read(PS_marker);

				if (memcmp(PS_marker, g_photoshop, g_photoshop_len) == 0)
				{
					ifs.SetByteOrder(true);	// big endian

					try
					{
						int ReadIPTC(FileStream& ifs, IPTCRecord& IPTC);

						IPTCRecord IPTC;
						if (ReadIPTC(ifs, IPTC))
						{
							info->SetIPTCInfo(IPTC);
							//info->IPTC_ = new IPTCRecord(IPTC);
							//info->tags_.AssignKeywords(info->IPTC_->keywords_);
						}
					}
					catch (...)
					{
						ASSERT(false);
					}
				}
			}
			break;

		case MARK_COM:
			{
				ASSERT(dataSize >= 2);
				size_t len= dataSize - 2;
				if (info && len > 0 && info->photo_desc_.empty())
				{
					std::string str(len, 0);
					ifs.Read(&str[0], len);
					MultiByteToWideString(str, info->photo_desc_);
				}
			}
			break;

		default:
			// ignore other markers
			break;
		}

		// go to the next marker
		ifs.RPosition(cur_pos + dataSize - 2, FileStream::beg);	// skip marker data
	}

	// now assign XMP data (if any) and overwrite IPTC info--XMP takes precedence over IPTC
	if (info && hasXmp)
		info->SetMetadata(xmpData);

	uint32 header_length= ifs.RPosition();

	ifs.RPosition(temp, FileStream::beg);

	if (info && !icc_profile.empty() && icc_profile_valid)
	{
		std::vector<uint8> profile_data;
		if (AssembleColorProfile(icc_profile, profile_data))
		{
			ColorProfilePtr profile= new ColorProfile(&profile_data[0], static_cast<DWORD>(profile_data.size()));

			if (profile->profile_)
				info->AssignColorProfile(profile);
		}
	}

	return header_length;
}



// scan JPEG file, parse EXIF block and copy it to the exif
extern bool Scan(const TCHAR* file, FileStream& ifs, PhotoInfoPtr info, OutputStr* output, ExifBlock* exif_ptr, bool decode_thumbnail, ImgLogger* logger, FieldCallback* field_callback/*= 0*/)
{
//LARGE_INTEGER tm[9];
//::QueryPerformanceCounter(&tm[0]);
	ExifBlock dummy;
	ExifBlock& exif= exif_ptr ? *exif_ptr : dummy;

	exif.clear(); 

//::QueryPerformanceCounter(&tm[1]);

	// ifs.SkipZeros(); // CFileDataSource to be modified too?

	Offset jpeg_start= ifs.RPosition();
	uint32 header_size= 0;

	uint16 start= ReadMarker(ifs);

	if (start == MARK_SOI)				// JPEG image?
	{
		header_size = ReadPhotoMarkers(ifs, info, logger, file) - jpeg_start;

		// copy all app markers (including entire EXIF block) into the buffer
		if (exif_ptr != 0 && header_size > 0)
		{
			Offset temp= ifs.RPosition();
			exif.exif_buffer.resize(header_size);
			ifs.RPosition(jpeg_start, FileStream::beg);
			ifs.Read(&exif.exif_buffer.front(), exif.exif_buffer.size());
			ifs.RPosition(temp, FileStream::beg);
		}
	}
	else								// looking for Canon raw
	{
		ifs.RPosition(jpeg_start, FileStream::beg);
		start = ifs.GetUInt16();

		if (start != 'II' && start != 'MM')
			return false;
		auto len= _tcslen(file);
		if (len < 4 || file[len - 4] != '.' || tolower(file[len - 3]) != 'c' ||
			tolower(file[len - 2]) != 'r' || tolower(file[len - 1]) != 'w')
			return false;

		ifs.SetByteOrder(start == 'MM');
		uint32 heap_len= ifs.GetUInt32();
		char id[8];
		ifs.Read(id, 8);
		if (memcmp(id, "HEAPCCDR", 8))
			return false;

		ifs.RPosition(0, FileStream::end);
		uint16 orientation= 0;
		std::pair<uint32, uint32> offset_size= ParseCRW(ifs, heap_len, ifs.RPosition() - heap_len, orientation);	// find JPEG preview image inside CRW

		if (offset_size.first == 0)
			return false;

		ifs.SetByteOrder(true);	// back to big endian now

		if (info)
		{
			info->jpeg_offset_ = offset_size.first;
			info->jpeg_size_ = offset_size.second;
			info->SetOrientation(orientation);
		}

		//TODO: look for EXIF block if there is one inside CRW file
		//...
		// Probably there isn't, so I rely on accompanying 'THM' file
		// There might be there some camera settings though

		return false;	// no EXIF data
	}

	std::pair<uint32, uint32> rangeExif;
	uint32 exif_size= 0;

	for (;;)
	{
		uint16 marker= 0;

		for (;;)
		{
			marker = ReadMarker(ifs);

			if (marker == MARK_APP1)
				break;	// examine it

			if (!IsJpegMarkerToBeSkipped(marker))
				return false;

			uint16 len= ifs.GetUInt16();
			if (len < 2)
				return false;		// bogus data--wrong size

			ifs.RPosition(len - 2);		// skip marker data
		}

		ASSERT(marker == MARK_APP1);		// app1 marker

		exif.offsetExifMarkerBlock = ifs.RPosition() - 2 - jpeg_start;

		uint16 len= ifs.GetUInt16();
		if (len < 2)
			return false;	// bogus data
		exif_size = len + 2;			// app1 data size (here including size field itself and marker)

		Offset marker_data_offset= ifs.RPosition();

		exif.exifBlockSize = exif_size;

		// range of EXIF: starts right after app marker's length field, ends where app marker ends
		// this is strict size, no padding
		rangeExif = std::make_pair(ifs.RPosition(), ifs.RPosition() + len - 2);

		char exif_id[6];
		ifs.Read(exif_id);					// Exif id
		if (exif_id[4] || /*exif_id[5] ||*/ strcmp(exif_id, "Exif"))
		{
			// that's not an EXIF block; try to find next APP1 marker
			ifs.RPosFromBeg(marker_data_offset + len - 2);	// skip marker data
			continue;
		}
		else
			break;	// found EXIF block, scan it
	}
//	info.output_.StartRecording();

	Offset ifd_start= ifs.RPosition();
	exif.ifd0Start = ifd_start - jpeg_start;

	if (ifs.GetUInt16() == 'II')		// Intel format?
		ifs.SetByteOrder(false);
	else
		ifs.SetByteOrder(true);

	exif.bigEndianByteOrder = ifs.GetByteOrder();

	ifs.GetUInt16();			// skip size

	ifs.RPosition(ifs.GetUInt32() - 8);	// offset to first IFD (Image File Directory)
	Offset exif_offset= ifs.RPosition();

	String model, make; // those are not yet known
	bool ret= ScanExif(file, ifs, ifd_start, rangeExif, make, model, info, &exif, output, decode_thumbnail, field_callback);

	exif.offsetIfd0Entries -= jpeg_start;
	exif.ifd1Start -= jpeg_start;

	if (info && ret)
		info->SetExifInfo(exif_offset, exif_size, ifd_start, ifs.GetByteOrder());

	return ret;
}


bool InRange(const std::pair<uint32, uint32>& range, uint32 position, uint32 length)
{
	if (position < range.first || position > range.second)
		return false;

	uint32 end= position + length;
	if (end < range.first || end > range.second)
		return false;

	return true;
}


class StorePos
{
public:
	StorePos(FileStream& ifs, uint32 new_pos) : ifs_(ifs), offset_(ifs.RPosition())
	{
		ifs_.RPosition(new_pos, FileStream::beg);
	}
	~StorePos()
	{
		ifs_.RPosition(offset_, FileStream::beg);
	}
private:
	FileStream& ifs_;
	uint32 offset_;
};


extern bool ScanExif(const TCHAR* file, FileStream& ifs, Offset ifd_start, std::pair<uint32, uint32> rangeExif, String make, String model, PhotoInfoPtr info, ExifBlock* exif, OutputStr* file_info, bool decode_thumbnail, FieldCallback* field_callback/*= 0*/)
{
	if (!InRange(rangeExif, ifs.RPosition(), 2))
	{
		ASSERT(false);	// bogus offset to the EXIF block
		return false;
	}

	uint16 entries1= ifs.GetUInt16();	// number of entries in IFD0

	const int ENTRY_SIZE= 12;
	if (!InRange(rangeExif, ifs.RPosition(), entries1 * ENTRY_SIZE))
	{
		ASSERT(false);	// bogus offset to the EXIF block
		return false;
	}

	if (exif)
	{
		exif->ifd0Entries = entries1;
		exif->offsetIfd0Entries = ifs.RPosition(); // - jpeg_start;
	}

DBG_ONLY(if (file_info) file_info += _T("----->IFD0\r\n"))

	for (uint32 i= 0; i < entries1; ++i)
		if (uint32 offset1= ReadEntry(ifs, ifd_start, make, model, info, file_info, field_callback))	// offset to sub IFD?
		{
			if (!InRange(rangeExif, ifd_start + offset1, 2))
			{
				ASSERT(false);	// bogus sub IFD offset
				continue;
			}

			StorePos temp(ifs, ifd_start + offset1);

DBG_ONLY(if (file_info) file_info += _T("----->subIFD\r\n"))
			uint32 entries2= ifs.GetUInt16();	// no of entries in sub IFD
			if (!InRange(rangeExif, ifs.RPosition(), entries2 * ENTRY_SIZE))
			{
				ASSERT(false);	// bogus sub IFD block
				continue;
			}

			for (uint32 i= 0; i < entries2; ++i)
				if (uint32 offset2= ReadEntry(ifs, ifd_start, make, model, info, file_info, field_callback))
					// offset to interoperability IFD?
				{
					if (!InRange(rangeExif, ifd_start + offset2, 2))
					{
//						ASSERT(false);	// bogus interop. IFD
						continue;
					}

					StorePos temp(ifs, ifd_start + offset2);

					uint32 entries3= ifs.GetUInt16();	// no of entries in interoperability IFD
					if (!InRange(rangeExif, ifs.RPosition(), entries3 * ENTRY_SIZE))
					{
						ASSERT(false);	// bogus interop. IFD
						continue;
					}

					for (uint32 i= 0; i < entries3; ++i)
						ReadEntry(ifs, ifd_start, make, model, info, file_info, field_callback);
				}
DBG_ONLY(if (file_info) file_info += _T("-----<subIFD\r\n"))
		}
DBG_ONLY(if (file_info) file_info += _T("-----<IFD0\r\n"))

	if (exif)
		exif->ifd1Start = ifs.RPosition(); // - jpeg_start;

	// IFD1
	Offset link= ifs.GetUInt32();
	if (link != 0 && InRange(rangeExif, ifd_start + link, 2))
	{
DBG_ONLY(if (file_info) file_info += _T("----->IFD1\r\n"))
		ifs.RPosFromBeg(ifd_start + link);

		uint32 entries = ifs.GetUInt16();		// no of entries in IFD1

		if (entries > 0)
			if (file_info) file_info->RecordInfo(0, _T("缩略图信息"), _T(""), _T(""));

		uint32 thumbnail_offset= 0;
		uint32 thumbnail_size= 0;

		if (InRange(rangeExif, ifs.RPosition(), entries * ENTRY_SIZE))
		{
			for (uint32 i= 0; i < entries; ++i)
				FindThumbnail(ifs, ifd_start, thumbnail_offset, thumbnail_size, file_info);
		}
		else
		{
			ASSERT(false);	// broken thumbnail IFD block; skipping
		}

		if (thumbnail_offset != 0 && thumbnail_size != 0 && thumbnail_size < 100 * 1024)
		{
			// HACK: Canon 20D lies: thumbnail is smaller in photos with ADOBE1998 color space
			uint32 end= ifd_start + thumbnail_offset + thumbnail_size;
			if (end > rangeExif.second && end - rangeExif.second < 20 && thumbnail_size > 1000)
				thumbnail_size -= end - rangeExif.second;
			// HACK end

			if (InRange(rangeExif, ifd_start + thumbnail_offset, thumbnail_size))
			{
				if (info != 0)
				{
					ifs.RPosFromBeg(ifd_start + thumbnail_offset);

					try
					{
						info->jpeg_thumb_.Empty();

						std::vector<uint8> buf;
						uint32 rest= ifs.RemainingBytes();
						if (thumbnail_size > rest)
							thumbnail_size = rest;		// Canon 20D lies: thumbnail is smaller in photos with ADOBE1998 color space
						buf.resize(thumbnail_size);
						ifs.Read(&buf.front(), thumbnail_size);
						if (decode_thumbnail)
						{
							CMemoryDataSource memsrc(&buf.front(), thumbnail_size);
							JPEGDecoder dec(memsrc, g_Settings.dct_method_);
							info->bmp_ = AutoPtr<Dib>(new Dib);

							ImageStat stat= dec.DecodeImgToYCbCr(*info->bmp_, CSize(0, 0), false);
							if (stat == IS_OPERATION_NOT_SUPPORTED)
							{
								JPEGDecoder dec2(memsrc, g_Settings.dct_method_);
								dec2.DecodeImg(*info->bmp_);
							}
							//TODO: error handling

							if (stat == IS_OK)
								info->jpeg_thumb_.SwapBuffer(buf);
						}
						else
							info->jpeg_thumb_.SwapBuffer(buf);

					}
					catch (const JPEGException& ex)	// jpeg decoding error?
					{
						// don't keep broken thumbnail JPEG in the buffer, it's useless
						info->jpeg_thumb_.Empty();

						if (g_Settings.warn_about_broken_thumbnail_img_)
						{
							CString msg= _T("读取内嵌缩略图出错.\n\nFile: ");
							msg += file;
							msg += _T("\n解码器消息: ");
							msg += ex.GetMessage();
							msg += _T(".\n\nEXIF 数据块损坏.");
							AfxGetMainWnd()->MessageBox(msg, 0, MB_OK | MB_ICONWARNING);
						}
					}
				}
			}
			else
			{
				ASSERT(false);	// thumbnail img outside EXIF block?!?
			}
		}
DBG_ONLY(if (file_info) file_info += _T("-----<IFD1\r\n"));
	}

	return true;
}


extern bool Scan(const TCHAR* file, PhotoInfoPtr info, OutputStr* output, ExifBlock* exif, ImgLogger* logger)
{
	FileStream ifs;

	if (!ifs.Open(file))
		return false;					// error opening file

	return Scan(file, ifs, info, output, exif, true, logger);
}


extern bool ScanMem(const TCHAR* file, const std::vector<uint8>& exif_data_buffer, PhotoInfoPtr info, OutputStr* output, ImgLogger* logger)
{
	FileStream ifs;

	if (!ifs.Open(exif_data_buffer))
		return false;

	// when reading from memory instruct ifs to return zeros instead of throwing
	// exceptions on out of bound reads
	ifs.SetExceptions(false);

	return Scan(file, ifs, info, output, 0, false, logger);
}



bool ReadDescription(const Data& val, std::wstring& description)
{
	description = L"";

	const int HEADER= 8;

	if (val.Components() <= HEADER)
		return false;

	char vchCodeDesignation[HEADER];
	val.ReadChar(vchCodeDesignation, array_count(vchCodeDesignation));

	int type= 0;		// ?
	if (memcmp(vchCodeDesignation, "ASCII\0\0", HEADER) == 0)
		type = 1;		// ASCII
	else if (memcmp(vchCodeDesignation, "UNICODE", HEADER) == 0)
		type = 2;		// Unicode
	else if (memcmp(vchCodeDesignation, "\0\0\0\0\0\0\0", HEADER) == 0)
		type = 3;		// undefined

	if (type == 0)		// unhandled or unknown code
	{
		description = L"[未知编码]";
		return false;
	}

	switch (type)
	{
	case 1:
	case 3:
		{
			std::vector<char> buff;
			buff.resize(val.Components() + 1);
			val.ReadChar(&buff.front(), static_cast<int>(buff.size()));

			const char* text= &buff.front() + HEADER;
			int length= val.Components() - HEADER;

			if (*text == '\0')		// empty description?
				break;
			if (*text == ' ')		// empty description (spaces only)?
			{
				buff.back() = '\0';
				bool has_text= false;
				// look for text
				for (int i= 0; i < length; ++i)
					if (text[i] != ' ')
					{
						has_text = true;
						break;
					}

				if (!has_text)
					break;
			}

			// trim spaces and nuls on the end
			for (int i= length - 1; i > 0; --i)
				if (text[i] == ' ' || text[i] == '\0')
					--length;
				else
					break;

			std::vector<wchar_t> output;
			output.resize(length + 8);
			int len= ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, text, length, &output.front(), static_cast<int>(output.size()));

			description.assign(&output.front(), len);
		}
		break;

	case 2:
		{
			std::vector<wchar_t> buff;
			buff.resize(val.Components() / 2);
			ASSERT(sizeof(wchar_t) == sizeof(uint16));
			val.ReadWords(reinterpret_cast<uint16*>(&buff.front()), static_cast<int>(buff.size()));
//			val.ReadWords(&buff.front(), buff.size());
			int offset= HEADER / 2;
			if (buff[offset] == 0)		// empty description?
				break;

			int size= static_cast<int>(buff.size());

			// trim spaces and nuls on the end
			for (int i= size - 1; i > offset; --i)
			{
				if (buff[i] != L' ' && buff[i] != L'\0')
					break;
				--size;
			}

			description.assign(&buff[offset], size - offset);
		}
		break;
	}

	return true;
}


// 'ifs' is expected to be positioned at the start of EXIF block (short num of entries first)
// CalcExifSize will traverse all fields and calculate how big EXIF block is.
// If EXIF block is bogus this function may throw
size_t CalcExifSize(FileStream& ifs)
{
	size_t exif_size= 0;

	uint16 entries= ifs.GetUInt16();	// number of entries in IFD0
	exif_size += 2;

	for (uint32 i= 0; i < entries; ++i)
	{
		uint16 tag= ifs.GetUInt16();
		exif_size += 2;

		Data val(ifs, 0);
		exif_size += val.Length();
	}

	return exif_size;
}
