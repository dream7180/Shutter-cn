/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfo.h"
#include "Config.h"
#include "PhotoAttr.h"
#include "MemMappedFile.h"
#include "RString.h"
#include "resource.h"
#include "Columns.h"
#include <math.h>
#include "DibCache.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "scan.h"
#include "ColorProfile.h"
#include "BmpFunc.h"
#include "ImageDraw.h"
#include "ExifTags.h"
#include "Database/ImageDatabase.h"
#include "ImgDb.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "XmpData.h"
#include "XmpAccess.h"
#include "StringConversions.h"
#include "DateTimeUtils.h"

extern Config g_Settings;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void IptcToXmp(const IPTCRecord& iptc, XmpData& xmp);
extern void XmpToIptc(const XmpData& xmp, IPTCRecord& iptc);

extern AutoPtr<DibCache> global_dib_cache;

CImageList PhotoInfo::img_list_errors_;
CCriticalSection g_init_photo_info;

//=============================================================================

PhotoInfo::PhotoInfo()
{
#ifdef _DEBUG
	object_is_deleted_ = false;
#endif
	orientation_ = ~0;
	width_ = 0;
	height_ = 0;
	exposure_program_ = ~0;
	iso_speed_ = static_cast<uint16>(ISO_SPEED_UNKNOWN);
	metering_mode_ = ~0;
	light_source_ = ~0;
	flash_ = ~0;
	file_size_ = 0;
	portrait_ = false;
	no_std_thumbnail_ = false;
	exif_data_present_ = false;
	jpeg_offset_ = 0;
	jpeg_size_ = 0;
	dir_visited_ = 0;
	thumbnail_orientation_ = 0;
	file_type_index_ = 0;
	thumbnail_stat_ = IS_NO_IMAGE;
	photo_stat_ = IS_NO_IMAGE;
	rotation_flag_ = RF_INTACT;
	lossless_crop_ = false;
	exposure_bias_ = g_UninitializedRationalVal;
	record_offset_ = 0;
	can_delete_ = true;
	can_rename_ = true;
	image_rate_ = 0;
	color_space_ = 0;
	date_stamp_ = 0;
	is_raw_ = false;
	subsecond_ = 0;

	if (img_list_errors_.m_hImageList == 0)
	{
		// allow only single thread to init
		CSingleLock lock(&g_init_photo_info, true);
		if (img_list_errors_.m_hImageList == 0)
		{
			int bmp_id= IDB_IMAGE_ERROR;
			HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(bmp_id), RT_BITMAP);
			HIMAGELIST img_list= ImageList_LoadImage(inst, MAKEINTRESOURCE(bmp_id),
				23, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
			ASSERT(img_list != 0);
			if (img_list)
				img_list_errors_.Attach(img_list);
		}
	}

	cached_effective_orientation_ = CalcPhotoOrientation();
}


PhotoInfo::~PhotoInfo()
{}



String PhotoInfo::ExposureProgram() const
{
	return ::ExposureProgram(exposure_program_);
}


String PhotoInfo::MeteringMode() const
{
	return ::MeteringMode(metering_mode_);
}


String PhotoInfo::LightSource() const
{
	return ::LightSource(light_source_);
}


String PhotoInfo::Flash() const
{
	return ::Flash(flash_);
}


String PhotoInfo::Orientation() const
{
	return ::Orientation(orientation_);
}


String PhotoInfo::ISOSpeed() const
{
	return ::ISOSpeed(iso_speed_);
}


uint16 PhotoInfo::GetISOSpeed() const
{
	return iso_speed_;
}


void PhotoInfo::SetISOSpeed(uint32 iso)
{
	if (iso <= ISO_SPEED_MAX_VAL && iso > 0)
		iso_speed_ = static_cast<uint16>(iso);
	else
	{
//		ASSERT(false);
		iso_speed_ = static_cast<uint16>(ISO_SPEED_UNKNOWN);
	}
}


bool PhotoInfo::IsISOSpeedValid() const
{
	return iso_speed_ <= static_cast<uint16>(ISO_SPEED_MAX_VAL);
}


void PhotoInfo::SetAutoISOSpeed()
{
	iso_speed_ = static_cast<uint16>(ISO_SPEED_AUTO);
}


PhotoInfo::ImgOrientation PhotoInfo::GetOrientation() const
{
	switch (orientation_)
	{
	case EXIF_ORIENTATION_NORMAL:	return ORIENT_NORMAL;
	case EXIF_ORIENTATION_UPSDN:	return ORIENT_UPSDN;
	case EXIF_ORIENTATION_90CCW:	return ORIENT_90CCW;
	case EXIF_ORIENTATION_90CW:		return ORIENT_90CW;
	case EXIF_ORIENTATION_UNKNOWN:	return ORIENT_UNKNOWN;
	default:						return ORIENT_NO_INFO;
	}
}


double PhotoInfo::ExposureTimeValue() const
{
	if (exposure_time_.Valid())
		return exposure_time_.Double();
	else if (shutter_speed_value_.Valid())
		return pow(2.0, -shutter_speed_value_.Double());
	else
		return 0.0;
}


String PhotoInfo::ExposureTime() const
{
	if (exposure_time_.Valid())
		return ::ExposureTime(exposure_time_);
	else
		return ::ExposureTimeFromSV(shutter_speed_value_);
}


double PhotoInfo::FNumberValue() const
{
	if (f_number_.Valid())
		return f_number_.Double();
	else if (aperture_value_.Valid())
		return pow(1.4142135623730950488016887242097, aperture_value_.Double());
	else
		return 0.0;
}


String PhotoInfo::FNumber() const
{
	if (f_number_.Valid())
	{
		if (f_number_.Double() == 0.0)
			return _T("-");
		return f_number_.UnitNumerator();
	}
	else
		return ::FNumberFromAV(aperture_value_);
}


String PhotoInfo::Size() const
{
	if (width_ == 0 || height_ == 0)
		return _T("-");
	oStringstream ost;
	ost << width_ << _T(" x ") << height_;
	return ost.str();
}


String PhotoInfo::ToolTipInfo(const std::vector<uint16> selected_fields) const
{
	oStringstream ost;
	static const TCHAR* EOL= _T("\r\n");

	Columns cols;

	const size_t count= selected_fields.size();
	for (size_t i= 0; i < count; ++i)
	{
		uint16 column= selected_fields[i];
		if (const TCHAR* name= cols.ShortName(column))
		{
			const size_t len= _tcslen(name);

			ost << name << (len > 6 || column == COL_PATH ? _T(":\t") : _T(":\t\t"));

			String value;
			cols.GetInfo(value, column, *this);
			ost << value;

			if (i != count - 1)
				ost << EOL;
		}
	}

	if (!GetTags().empty())
	{
		if (count > 0)
			ost << EOL;

		const size_t tags_count= GetTags().size();

		ost << _T("Tags:\t\t");

		for (int i= 0; i < tags_count; ++i)
		{
			const String& str= GetTags()[i];
			ost << _T("\"") << str << _T("\" ");
		}
	}

	return ost.str();
}


String PhotoInfo::ToolTipInfo(bool path/*= false*/) const
{
	oStringstream ost;
	static const TCHAR* EOL= _T("\r\n");

	Columns cols;

	ost << cols.ShortName(COL_PHOTO_NAME) << _T(":\t\t") << name_ << EOL;
	ost << cols.ShortName(COL_DIMENSIONS) << _T(":\t") << Size() << EOL;
	ost << cols.ShortName(COL_DATE_TIME) << _T(":\t") << DateTimeStr().c_str() << EOL;
	ost << cols.ShortName(COL_FNUMBER) << _T(":\t") << FNumber() << EOL;
	ost << cols.ShortName(COL_EXP_TIME) << _T(":\t") << ExposureTime() << EOL;
	ost << cols.ShortName(COL_FOCAL_LENGTH) << _T(":\t") << FocalLength() << EOL;
	ost << cols.ShortName(COL_ISO) << _T(":\t") << ISOSpeed() << EOL;
	ost << cols.ShortName(COL_EXP_BIAS) << _T(":\t") << ExposureBias() << EOL;
	ost << cols.ShortName(COL_EXP_PROG) << _T(":\t") << ExposureProgram() << EOL;
	ost << cols.ShortName(COL_FILE_SIZE) << _T(":\t") << FileSize();
	if (path)
		ost << EOL << cols.ShortName(COL_PATH) << _T(":\t") << path_;

	return ost.str();
}


double PhotoInfo::GetFieldOfViewCrop() const	// preconfigured value (like 1.5 for Nikon D70)
{
	return g_Settings.GetFocalLengthMultiplier(model_);
}


double PhotoInfo::GetFocalLength35mm() const
{
	double mul= g_Settings.GetFocalLengthMultiplier(model_);
	return focal_length_.Double() * mul;
}


String PhotoInfo::FocalLength35mm() const
{
	int FL= static_cast<int>(floor(GetFocalLength35mm() * 10.0));
	if (FL <= 0)
		return _T("-");
	oStringstream ost;
	ost << FL / 10;
	FL %= 10;
	if (FL)
		ost << _T(".") << FL;
	return ost.str();
}


String PhotoInfo::FocalLength() const
{
	double fl= focal_length_.Double();
	if (fl == 0.0)
		return _T("-");
	oStringstream ost;
	ost << std::fixed << std::setprecision(1) << fl;
	return ost.str();
}

/*
bool PhotoInfo::IsDescriptionEditable() const
{
	MemMappedFile photo;
	if (!photo.CreateWriteView(path_.c_str(), 0, true))
	{
		// if it's only access violation lets hope it'll be unblocked later
		if (::GetLastError() == ERROR_SHARING_VIOLATION)
			return true;

		RString msg;
		msg.Format(IDS_CANNOT_OPEN, path_.c_str());
		AfxMessageBox(msg, MB_OK);
		return false;
	}
	return true;
}*/


//bool PhotoInfo::WriteDescription(const wstring& text, CWnd* parent)
//{
//	extern bool InsertUserDesc(PhotoInfo& inf, const wstring& description, bool silent, CWnd* parent);
//
//	if (InsertUserDesc(*this, text, false, parent))
//	{
//		photo_desc_ = text;
//		return true;
//	}
//
//	return false;
//}


//void PhotoInfo::DescriptionAscii(string& rstrText)
//{
//	if (photo_desc_.empty())
//	{
//		rstrText.erase();
//		return;
//	}
//
//	vector<char> buff;
//	buff.resize(photo_desc_.size() + 32);
//	int len= ::WideCharToMultiByte(CP_ACP, 0, photo_desc_.data(),
//		static_cast<int>(photo_desc_.size()), &buff.front(), static_cast<int>(buff.size()), 0, 0);
//	rstrText.assign(&buff.front(), len);
//}


void PhotoInfo::Description(String& text) const
{
#ifdef _UNICODE
	text = photo_desc_;
#else
	USES_CONVERSION;
	text = W2A(photo_desc_.c_str());
#endif
}


// draw thumbnail image preserving aspect ratio; create thumbnail if it's not in the cache
//
void PhotoInfo::Draw(CDC* dc, CRect& dest_rect, COLORREF rgb_back, COLORREF rgb_selection/*=RGB(0,0,0)*/,
					 COLORREF rgb_outline/*= RGB(0,0,0)*/, COLORREF rgb_text/*=RGB(0,0,0)*/, COLORREF rgb_text_bk/*= RGB(0,0,0)*/,
					 UINT flags/*= DRAW_DIBDRAW | DRAW_BACKGND*/, const String* label/*= 0*/)
{
	try
	{
		ASSERT(dc);

		CSize dest_size= ImageDraw::GetImageSize(0, dest_rect, flags).Size();

		Dib* dib= GetThumbnail(dest_size);

		if (dib != 0 && dib->IsValid())
		{
			Draw(dib, dc, dest_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);
		}
		else
		{
			Draw(0, dc, dest_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);

			IMAGEINFO ii;
			if (img_list_errors_.GetImageInfo(0, &ii))
			{
				CRect rect= ii.rcImage;
				if (rect.Width() <= dest_rect.Width() && rect.Height() < dest_rect.Height())
				{
					CPoint pt= dest_rect.CenterPoint() + CPoint(-11, -9);	// center status img
					img_list_errors_.Draw(dc, 0, pt, ILD_TRANSPARENT);
				}
				else
				{
					rect = dest_rect;
					rect.DeflateRect(1, 1);
					COLORREF gray= RGB(200,200,200);
					dc->Draw3dRect(rect, gray, gray);	// status img too big, so draw gray rect instead
				}
			}
		}


/*		else if (!global_dib_cache.empty())
		{
			pair<DibImg*, bool> dib= global_dib_cache->GetEntry(this);
			bool decode= false;

			CSize dest_size= ImageDraw::GetImageSize(0, dest_rect, flags).Size();

			if (dib.first->dib.IsValid())		// has valid image?
			{
				CSize size= dest_size;
				CSize img= Dib::SizeToFit(size, dib.first->original_size);
				if (img.cx > dib.first->original_size.cx || img.cy > dib.first->original_size.cy)
					img = dib.first->original_size;
				if (dib.first->dib.GetWidth() < dib.first->dib.GetHeight() && img.cx > img.cy)
					swap(img.cx, img.cy);

				// if img is too small then do not re-decode

				if (dib.first->dib.GetWidth() < img.cx || dib.first->dib.GetHeight() < img.cy)
					decode = true;		// existing (decoded) img is too small, decode it again
			}

			if (decode || !dib.second)		// no img yet?
			{
				thumbnail_stat_ = CreateThumbnail(dib.first->dib);
				if (thumbnail_stat_ == IS_OK)
				{
					dib.first->original_size = dib.first->dib.GetSize();
					dib.first->dib.ResizeToFit(dest_size, Dib::RESIZE_HALFTONE);
					OrientThumbnail(dib.first->dib);
				}
				else
					dib.first->original_size = CSize(0, 0);
			}

			if (dib.first->dib.IsValid())	// has valid image?
			{
				Draw(&dib.first->dib, dc, dest_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);
			}
			else
			{
				Draw(0, dc, dest_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);

				CPoint pt= dest_rect.CenterPoint() + CPoint(-11, -9);	// center status img
				img_list_errors_.Draw(dc, 0, pt, ILD_TRANSPARENT);
			}
		} */
	}
	catch (...)
	{
		//TODO: log
		ASSERT(false);
	}
}


void PhotoInfo::DrawErrIcon(CDC* dc, CRect& dest_rect)
{
	CPoint pt= dest_rect.CenterPoint() + CPoint(-11, -9);	// center status img
	img_list_errors_.Draw(dc, 0, pt, ILD_TRANSPARENT);
}


bool PhotoInfo::ReadJpegThumbnail(ImageDatabase& db, CJpeg& jpg) const
{
	jpg.Empty();

	ImgDataRecord img;
	if (!db.ReadImageAt(record_offset_, img))
		return false;

	if (path_ != img.path_)
	{
		ASSERT(false);	// out of sync
		return false;
	}

	jpg.SwapBuffer(img.buf_thumbnail_);
	return !jpg.IsEmpty();
}


ImageStat PhotoInfo::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		if (!jpeg_thumb_.IsEmpty())
			return jpeg_thumb_.GetDib(bmp);
		else
		{
			CJpeg jpg;
			if (ReadJpegThumbnail(GetImageDataBase(true, true), jpg))
			{
				ImageStat stat= jpg.GetDib(bmp);
				if (stat == IS_OK)
					OrientThumbnail(bmp);
				return stat;
			}
			else
			{
				CFileDataSource fsrc(path_.c_str(), jpeg_offset_);
				JPEGDecoder dec(fsrc, JDEC_INTEGER_HIQ);
				dec.SetFast(true, true);
				dec.SetProgressClient(progress);
				return dec.DecodeImg(bmp, GetThumbnailSize(), true);
			}
		}
	}
	catch (...) // TODO: store error code
	{
	}
	return IS_READ_ERROR;
}


// calc size of bitmap (bmp_size) resized uniformly to fit into given area (dest_size) and return result in photo_size
bool PhotoInfo::FitToSize(CSize bmp_size, CSize dest_size, CSize& photo_size)
{
	if (bmp_size.cx < 1 || bmp_size.cy < 1)
	{
		ASSERT(false);
		return false;
	}

	if (dest_size.cx <= 0 || dest_size.cy <= 0)
		return false;

	double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);

	double dest_ratio= double(dest_size.cx) / double(dest_size.cy);

	double epsillon= 0.01;

	if (fabs(bmp_ratio - dest_ratio) < epsillon)
	{
		// simple uniform scaling
		photo_size = dest_size;
	}
	else	// different ratios
	{
		// calc how to rescale bmp to fit into dest area
		double scale_w= double(dest_size.cx) / double(bmp_size.cx);
		double scale_h= double(dest_size.cy) / double(bmp_size.cy);

		double scale= MIN(scale_w, scale_h);

		// rescale bmp
		photo_size.cx = static_cast<LONG>(bmp_size.cx * scale);
		photo_size.cy = static_cast<LONG>(bmp_size.cy * scale);
	}

	return true;

	//TODO: take orientation into account?
}


void PhotoInfo::Draw(Dib* bmp, CDC* dc, CRect& destination_rect,
					 COLORREF rgb_back, COLORREF rgb_selection, COLORREF rgb_outline,
					 COLORREF rgb_text, COLORREF rgb_text_bk, UINT flags, const String* label)
{
	ImageDraw::Draw(bmp, dc, destination_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);
}


// format date & time string
//
String PhotoInfo::DateTimeStr() const
{
	return ::DateTimeFmt(date_time_, _T("  "));
}


void PhotoInfo::SetSubSeconds(int t)
{
	subsecond_ = t;
}


int PhotoInfo::GetSubSeconds() const
{
	return subsecond_;
}


void PhotoInfo::SetDateTime(const DateTime& t)
{
	date_time_ = t;

	if (t.is_not_a_date_time())
		date_stamp_ = 0;
	else
	{
		auto d= t.date().year_month_day();
		date_stamp_ = d.year * 12 * 31 + (d.month - 1) * 31 + (d.day - 1);
	}
}


DateTime PhotoInfo::GetDateTime() const
{
	return date_time_;
}


uint32 PhotoInfo::GetDateStamp() const
{
	return date_stamp_;
}


String PhotoInfo::ExposureBias() const
{
	return ::ExposureBias(exposure_bias_);
}


bool PhotoInfo::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	return ::Scan(filename, this, nullptr/*&output_*/, &exifData, logger);
}

bool PhotoInfo::Scan(const TCHAR* filename, const std::vector<uint8>& exif_data_buffer, ImgLogger* logger)
{
	return ::ScanMem(filename, exif_data_buffer, this, nullptr/*&output_*/, logger);
}


// can edit IPTC info?
bool PhotoInfo::CanEditIPTC(int& err_code) const
{
	err_code = 0;
	return false;
/*
	extern int CanEditIPTCRecord(const TCHAR* file);
	err_code = CanEditIPTCRecord(path_.c_str());
	return err_code == 0 || err_code == ERROR_SHARING_VIOLATION;	// optimistic 'ok'
*/
}


bool PhotoInfo::IsRotationFeasible() const
{
	// return true if this image can be rotated (physically, on a disc)
	return true;
}


bool PhotoInfo::IsFileEditable() const
{
	MemMappedFile photo;
	if (!photo.CreateWriteView(path_.c_str(), 0, true))
	{
		// if it's only sharing violation lets hope it'll be unlocked later
		if (::GetLastError() == ERROR_SHARING_VIOLATION)
			return true;

		return false;
	}
	return true;
}


bool PhotoInfo::GetThumbnailMirrorFlip() const
{
	if (thumbnail_orientation_ != 0 || rotation_flag_ != RF_INTACT)
		return !!(thumbnail_orientation_ & 0x40);
	return false;
}


int PhotoInfo::GetThumbnailOrientation() const
{
	int orientation= 0;

	bool photo_is_rotated= width_ < height_;

	if (thumbnail_orientation_ != 0 || rotation_flag_ != RF_INTACT)
	{
		// this is orientation state as remembered in an APP marker (rotated by ExifPro)
		orientation = thumbnail_orientation_ & 3;

		// add non-permanent rotation (if any)
		orientation += rotation_flag_ & 3;

		orientation &= 3;

		//if (thumbnail_orientation_ != 0)	// photo was rotated using ExifPro
		//	orientation = thumbnail_orientation_ & 0x03;
		//else
		//	orientation = 1; // just a guess
	}
	else if (photo_is_rotated) // photo is altered
	{
		orientation = 0;
	}
	else
	{
		// if EXIF info indicates rotation prepare a rotated bitmap
		switch (GetOrientation())
		{
		case PhotoInfo::ORIENT_90CW:
			orientation = 3; // TODO: verify it's 1 and not 3
			break;
		case PhotoInfo::ORIENT_90CCW:
			orientation = 1; // TODO: ditto
			break;
		}
	}

	return orientation;
}


int PhotoInfo::CalcPhotoOrientation() const
{
	int orientation= 0;

	if (thumbnail_orientation_ != 0 || rotation_flag_ != RF_INTACT)
	{
		// this is orientation state as remembered in an APP marker (rotated by ExifPro)
//		orientation = thumbnail_orientation_ & 3;

		// add non-permanent rotation (if any)
		orientation += rotation_flag_ & 3;

		orientation &= 3;

		//if (thumbnail_orientation_ != 0)	// photo was rotated using ExifPro
		//	orientation = thumbnail_orientation_ & 0x03;
		//else
		//	orientation = 1; // just a guess
	}
	else if (width_ < height_) // photo is rotated? then it's been altered
	{
		// ignore orientation request for altered photo
		orientation = 0;
	}
	else
	{
		// if EXIF info indicates rotation prepare a rotated bitmap
		switch (orientation_)
		{
		case EXIF_ORIENTATION_90CW:
			orientation = 3; // TODO: verify it's 1 and not 3
			break;
		case EXIF_ORIENTATION_90CCW:
			orientation = 1; // TODO: ditto
			break;
		}
	}

	return orientation;
}


void PhotoInfo::OrientThumbnail(Dib& bmp) const
{
	AutoPtr<Dib> copy;

	bool mirror= GetThumbnailMirrorFlip();

	if (mirror)
		::FlipBitmap(bmp, true, false);

	int orientation= GetThumbnailOrientation();

	if (orientation == 1)
		copy = bmp.RotateCopy(true);
	else if (orientation == 3)
		copy = bmp.RotateCopy(false);
	else if (orientation == 2)
	{
		// upside down
		copy = bmp.RotateCopy(true);
		copy = copy->RotateCopy(true);
	}

	if (copy)
		bmp.Swap(*copy);	// replace thumbnail by rotated copy
}


void PhotoInfo::RotateThumbnail(bool clockwise, bool mirror)
{
	if (bmp_)
	{
		bmp_ = bmp_->RotateCopy(clockwise);
		if (bmp_)
			::FlipBitmap(*bmp_, mirror, false);
	}
	else
	{
		if (global_dib_cache.get())
			RemoveAllPhotoDibs(this, *global_dib_cache);

/*
		// modify copy in the cache

//TODO: Revise: if cache is empty this line will add empty entry and thumbnail won't be read when Draw() is invoked?
		pair<DibImg*, bool> dib= global_dib_cache->GetEntry(this);

		if (dib.second)		// img found?
		{
			if (jpeg_thumb_.GetDib(dib.first->dib) == IS_OK)
				OrientThumbnail(dib.first->dib);
			else
				dib.first->dib.RotateInPlace(clockwise);	// that's not accurate
		}
*/
	}
}


// return pointer to temporary dib with thumbnail image
//
Dib* PhotoInfo::GetThumbnail(CSize dest_size/* = CSize(0, 0)*/)
{
	if (bmp_.get() != 0)
	{
		// photo maintains its own copy of thumbnail
		return bmp_.get();
	}

	if (global_dib_cache.empty())
		return 0;

	std::pair<DibImg*, bool> dib= global_dib_cache->GetEntry(DibImgKey(this, dest_size));
	bool decode= false;

//		CSize dest_size= ImageDraw::GetImageSize(0, dest_rect, flags).Size();

	if (dib.first->dib.IsValid() && dest_size.cx && dest_size.cy)		// has valid image?
	{
		CSize size= dest_size;
		CSize img= Dib::SizeToFit(size, dib.first->original_size);
		if (img.cx > dib.first->original_size.cx || img.cy > dib.first->original_size.cy)
			img = dib.first->original_size;
		if (dib.first->dib.GetWidth() < dib.first->dib.GetHeight() && img.cx > img.cy)
			std::swap(img.cx, img.cy);

		// if img is too small then re-decode

		if (dib.first->dib.GetWidth() < img.cx || dib.first->dib.GetHeight() < img.cy)
			decode = true;		// existing (decoded) img is too small, decode it again
	}

	if (decode || !dib.second)		// no img yet?
	{
		thumbnail_stat_ = CreateThumbnail(dib.first->dib, 0);
		if (thumbnail_stat_ == IS_OK)
		{
			// rotate thumbnail if needed
			OrientThumbnail(dib.first->dib);
			// record original size; note that thumbnail could have been rotated at this point
			dib.first->original_size = dib.first->dib.GetSize();
			if (dest_size.cx > 0 && dest_size.cy > 0)
				dib.first->dib.ResizeToFit(dest_size, Dib::RESIZE_HALFTONE);
		}
		else
			dib.first->original_size = CSize(0, 0);
	}

/*			if (dib.first->dib.IsValid())	// has valid image?
			{
				Draw(&dib.first->dib, dc, dest_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);
			}
			else
			{
				Draw(0, dc, dest_rect, rgb_back, rgb_selection, rgb_outline, rgb_text, rgb_text_bk, flags, label);

				CPoint pt= dest_rect.CenterPoint() + CPoint(-11, -9);	// center status img
				img_list_errors_.Draw(dc, 0, pt, ILD_TRANSPARENT);
			} */

/*		if (!dib.second)		// no img yet?
		{
			thumbnail_stat_ = CreateThumbnail(dib.first->dib);
			if (thumbnail_stat_ == IS_OK)
				OrientThumbnail(dib.first->dib);
		}
*/

	return &dib.first->dib;		// has valid image?
}


Dib* PhotoInfo::IsThumbnailAvailable(CSize dest_size) const
{
	if (global_dib_cache.empty())
		return 0;

//	CSize resized= Dib::SizeToFit(img_size, CSize(width_, height_));

	DibImg* dib= global_dib_cache->FindEntry(DibImgKey(this, dest_size), false);
	bool decode= false;

	if (dib && dib->dib.IsValid() && dest_size.cx && dest_size.cy)		// has valid image?
	{
		CSize size= dest_size;
		CSize img= Dib::SizeToFit(size, dib->original_size);
		if (img.cx > dib->original_size.cx || img.cy > dib->original_size.cy)
			img = dib->original_size;
		if (dib->dib.GetWidth() < dib->dib.GetHeight() && img.cx > img.cy)
			std::swap(img.cx, img.cy);

		// if img is too small then re-decode

		if (dib->dib.GetWidth() < img.cx || dib->dib.GetHeight() < img.cy)
			return 0;		// existing (decoded) img is too small, decode it again

		return &dib->dib; // good (big enough)
	}

	return 0;
}


void PhotoInfo::PutThumbnail(Dib* thumbnail, CSize dest_size)
{
	if (global_dib_cache.empty())
		return;
TRACE(L"PutThumbnail: (%d,%d) - %s\n", dest_size.cx, dest_size.cy, name_.c_str());
	std::pair<DibImg*, bool> dib= global_dib_cache->GetEntry(DibImgKey(this, dest_size));
	bool decode= false;

	if (thumbnail && thumbnail->IsValid())
	{
		dib.first->dib.Swap(*thumbnail);

		thumbnail_stat_ = IS_OK; //CreateThumbnail(dib.first->dib);

		if (thumbnail_stat_ == IS_OK)
		{
			// rotate thumbnail if needed
			OrientThumbnail(dib.first->dib);
			// record original size; note that thumbnail could have been rotated at this point
			dib.first->original_size = dib.first->dib.GetSize();
			//if (dest_size.cx > 0 && dest_size.cy > 0)
			//	dib.first->dib.ResizeToFit(dest_size, Dib::RESIZE_HALFTONE);
		}
//		else
//			dib.first->original_size = CSize(0, 0);
	}

//	return &dib.first->dib;		// has valid image?
}


CImageDecoderPtr PhotoInfo::GetDecoder() const
{
	AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_offset_);
	JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
	return new JPEGDecoder(file, dct_method);
}


// save tags inside photo
void PhotoInfo::SaveTags(const WriteAccessFn& get_write_access)
{
	throw String(_T("此类型图像不能保存标记."));
}

// load saved tags
void PhotoInfo::LoadTags()
{
	// nop
}


bool PhotoInfo::HorizontalOrientation() const
{
	bool horz= true;
	switch (GetPhotoOrientation())
	{
	case 1:
	case 3:
		horz = false;
		break;
	default:
		horz = true;
		break;
	}

	if (width_ < height_)
		horz = !horz;

	return horz;
}


int PhotoInfo::RotatePhoto(RotationTransformation transform, bool mirror, CWnd* parent)
{
	if (IsRotationFeasible() && IsFileEditable())
		return ::RotatePhoto(*this, transform, mirror, parent);
	else
		return -99;	// cannot rotate this type of photo or it's read-only/not accessible
}


bool PhotoInfo::IsLosslessJPEGCropPossible() const
{
	return lossless_crop_;
}


bool PhotoInfo::ReadExifBlock(ExifBlock& exif) const
{
	if (!exif_data_present_)
		return false;

	return ::Scan(path_.c_str(), nullptr, nullptr, &exif, nullptr);
}

/*
String PhotoInfo::CompleteInfo(bool raw) const
{
	return output_.GetInfo(raw);
}

const OutputStr& PhotoInfo::CompleteInfo() const
{
	return output_;
}
*/


void PhotoInfo::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out.Clear();

	if (record_offset_ == 0)	// is this img in a cache database? if not, bail out
		return;

	ImgDataRecord img;
	if (!db.ReadImageAt(record_offset_, img))
		return;

	if (path_ != img.path_)
	{
		ASSERT(false);	// out of sync
		return;
	}

	::ScanMem(path_.c_str(), img.bufEXIF_, 0, &out, nullptr);
}


String PhotoInfo::Width() const			{ return ::PhotoWidth(width_); }

String PhotoInfo::Height() const		{ return ::PhotoHeight(height_); }


String PhotoInfo::GetFileTypeName() const
{
	String ext= path_.GetExtension();
	for (size_t i= 0; i < ext.size(); ++i)
		ext[i] = _totupper(ext[i]);
	return ext;
}


String PhotoInfo::GetNameAndExt() const
{
	return path_.GetFileNameAndExt();
}


// this is size of an image; if it's rotated (non-permanent rotation) size will reflect that
CSize PhotoInfo::GetSize() const
{
	if (HorizontalOrientation())
		// landscape orientation
		return width_ > height_ ? CSize(width_, height_) : CSize(height_, width_);
	else
		// portrait orientation
		return height_ > width_ ? CSize(width_, height_) : CSize(height_, width_);
}


bool PhotoInfo::OrientationAltered() const
{
	return thumbnail_orientation_ != 0 || rotation_flag_ != 0;
}


void PhotoInfo::PhotoRotated(bool b90CW)
{
	// remember any non-permanent rotation here;
	// it's to be applied on top of permanent rotation (if any);
	// GetOrientation takes that into account
	rotation_flag_ = (rotation_flag_ + (b90CW ? 1 : -1)) & 3;
	rotation_flag_ |= RF_AUTO;
	cached_effective_orientation_ = CalcPhotoOrientation();
	RotateThumbnail(b90CW, false);
}


bool PhotoInfo::CanDelete() const
{
	return can_delete_;
}


bool PhotoInfo::CanRename() const
{
	if (!can_rename_)
		return false;

	// check if it's read only

	DWORD attrib= ::GetFileAttributes(path_.c_str());
	if (attrib == INVALID_FILE_ATTRIBUTES)
		return false;

	if (attrib & FILE_ATTRIBUTE_READONLY)
		return false;

	return true;
}


String PhotoInfo::FileSize() const
{
	oStringstream ost;
	if (file_size_ >= 1024 * 100)
		ost << uint32(file_size_ / 1024) << _T(" KB");
	else if (file_size_ >= 1024)
	{
		ost.setf(std::ios::fixed, std::ios::floatfield);
		ost.precision(1);
		ost << double(file_size_ / 1024.0) << _T(" KB");
	}
	else
		ost << file_size_ << _T(" B");
	return ost.str();
}


const Path& PhotoInfo::GetPhysicalPath() const
{
	return path_;
}

Path PhotoInfo::GetDisplayPath() const
{
	return path_;
}

Path PhotoInfo::GetOriginalPath() const
{
	return path_;
}

void PhotoInfo::SetPhysicalPath(const String& path)
{
	path_ = path;
}

Path PhotoInfo::GetAssociatedFilePath(size_t index) const
{
	return Path();
}

// no-throw rename function
void PhotoInfo::Rename(Path& path, String& name) throw()
{
	path_.swap(path);
	name_.swap(name);
}


void PhotoInfo::SaveMetadata(const XmpData& xmp, const WriteAccessFn& get_write_access) const
{
	throw String(_T("不支持保存元数据."));
}

bool PhotoInfo::LoadMetadata(XmpData& xmp) const
{
	return false;	// no metadata
}


bool PhotoInfo::GetMetadata(XmpData& xmp) const
{
	if (xmp_.get() == 0)
		return false;

	xmp = *xmp_;

	return true;
}

const XmpData* PhotoInfo::GetMetadata() const
{
	return xmp_.get();
}

// remember XMP metadata; update tags
void PhotoInfo::SetMetadata(const XmpData& xmp)
{
	if (xmp_.get() == 0)
		xmp_.reset(new XmpData());

	*xmp_ = xmp;

#ifdef _UNICODE
	photo_desc_ = xmp.Description;
#else
	::MultiByteToWideString(xmp.Description, photo_desc_);
#endif

	// update vector of tags
	std::vector<String> keys;
	Xmp::ParseItems(xmp_->Keywords, keys);
	tags_.AssignKeywords(keys);

	int rating= xmp.ImageRating.empty() ? 0 : _ttoi(xmp.ImageRating.c_str());

	if (rating >= 0 && rating <= 5)
		image_rate_ = rating;
	else
		image_rate_ = 0;
}


void PhotoInfo::SetIPTCInfo(const IPTCRecord& iptc)
{
	if (xmp_.get() == 0)
		xmp_.reset(new XmpData());

	IptcToXmp(iptc, *xmp_);

	tags_.AssignKeywords(iptc.keywords_);

	if (!photo_desc_.empty())
#ifdef _UNICODE
		xmp_->Description = photo_desc_;
#else
		::WideStringToMultiByte(photo_desc_, xmp_->Description);
#endif
}


bool PhotoInfo::GetIPTCInfo(IPTCRecord& iptc) const			// set IPTC record based on the internal XMP metadata
{
	if (xmp_.get() == 0)
		return false;

	XmpToIptc(*xmp_, iptc);

	return true;
}


bool PhotoInfo::HasMetadata() const
{
	return xmp_.get() != 0;
}


void PhotoInfo::SaveRating(int rating, const WriteAccessFn& get_write_access)
{
	// save rating in the XMP data (derived classes)
	image_rate_ = rating;
}


void PhotoInfo::SetExifInfo(Offset exif_offset, uint32 exif_size, Offset ifd_offset, bool big_endian)
{
	exif_info_.big_endian_byte_order = big_endian;
	exif_info_.exif_offset = exif_offset;
	//Offset offset_to_exif_marker;	// app2 marker's position in a file (JPEG mainly)
	exif_info_.exif_block_size = exif_size;
	exif_info_.offset_to_Ifd_start = ifd_offset;
	exif_info_.offset_to_Ifd0_entries = 0;
}


void PhotoInfo::SetColorSpace(uint16 val)
{
	color_space_ = val;
}


bool PhotoInfo::IsSRGB() const
{
	return color_space_ == 1;
}


bool PhotoInfo::IsAdobeRGB() const
{
	return color_space_ == 0xffff;	// best guess; 0xffff means uncalibrated
}


void PhotoInfo::AssignColorProfile(ColorProfilePtr icc)
{
	icc_profile_ = icc;
}


bool PhotoInfo::HasColorProfile() const
{
	if (icc_profile_.get() != 0)
		return true;

	if (IsAdobeRGB())
		return true;

	return false;
}


bool PhotoInfo::HasEmbeddedColorProfile() const
{
	return icc_profile_.get() != 0;
}


static BYTE data[]= {	// AdobeRGB color profile
  0x00, 0x00, 0x02, 0x30, 0x41, 0x44, 0x42, 0x45, 0x02, 0x10, 0x00, 0x00, 0x6d, 0x6e, 0x74, 0x72, 
  0x52, 0x47, 0x42, 0x20, 0x58, 0x59, 0x5a, 0x20, 0x07, 0xd0, 0x00, 0x08, 0x00, 0x0b, 0x00, 0x13, 
  0x00, 0x33, 0x00, 0x3b, 0x61, 0x63, 0x73, 0x70, 0x41, 0x50, 0x50, 0x4c, 0x00, 0x00, 0x00, 0x00, 
  0x6e, 0x6f, 0x6e, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd3, 0x2d, 
  0x41, 0x44, 0x42, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0a, 0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x32, 
  0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x30, 0x00, 0x00, 0x00, 0x6b, 0x77, 0x74, 0x70, 0x74, 
  0x00, 0x00, 0x01, 0x9c, 0x00, 0x00, 0x00, 0x14, 0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x01, 0xb0, 
  0x00, 0x00, 0x00, 0x14, 0x72, 0x54, 0x52, 0x43, 0x00, 0x00, 0x01, 0xc4, 0x00, 0x00, 0x00, 0x0e, 
  0x67, 0x54, 0x52, 0x43, 0x00, 0x00, 0x01, 0xd4, 0x00, 0x00, 0x00, 0x0e, 0x62, 0x54, 0x52, 0x43, 
  0x00, 0x00, 0x01, 0xe4, 0x00, 0x00, 0x00, 0x0e, 0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xf4, 
  0x00, 0x00, 0x00, 0x14, 0x67, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00, 0x14, 
  0x62, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x00, 0x14, 0x74, 0x65, 0x78, 0x74, 
  0x00, 0x00, 0x00, 0x00, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x73, 0x74, 0x72, 0x69, 
  0x6e, 0x67, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x69, 0x6d, 0x61, 0x67, 0x65, 0x20, 0x63, 0x6f, 0x6c, 
  0x6f, 0x72, 0x20, 0x70, 0x72, 0x6f, 0x66, 0x69, 0x6c, 0x65, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 
  0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x41, 0x64, 0x6f, 0x62, 
  0x65, 0x20, 0x52, 0x47, 0x42, 0x20, 0x28, 0x31, 0x39, 0x39, 0x38, 0x29, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x51, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x16, 0xcc, 
  0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x02, 0x33, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x18, 
  0x00, 0x00, 0x4f, 0xa5, 0x00, 0x00, 0x04, 0xfc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x34, 0x8d, 0x00, 0x00, 0xa0, 0x2c, 0x00, 0x00, 0x0f, 0x95, 0x58, 0x59, 0x5a, 0x20, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x31, 0x00, 0x00, 0x10, 0x2f, 0x00, 0x00, 0xbe, 0x9c
};

static ColorProfilePtr g_adobe_rgb_profile= new ColorProfile(data, sizeof(data));


ColorProfilePtr PhotoInfo::GetColorProfile() const
{
	if (icc_profile_.get() != 0)
		return icc_profile_;

	if (IsAdobeRGB())
		return g_adobe_rgb_profile;

	return ColorProfilePtr();
}


bool PhotoInfo::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
{
	return false;	// no preview
}


const String&		PhotoInfo::GetName() const				{ return name_; }
uint64				PhotoInfo::GetFileSize() const			{ return file_size_; }
Rational			PhotoInfo::GetExposureTime() const		{ return exposure_time_; }
SRational			PhotoInfo::GetExposureBias() const		{ return exposure_bias_; }
Rational			PhotoInfo::GetFStop() const				{ return f_number_; }
uint32				PhotoInfo::GetWidth() const				{ return width_; }
uint32				PhotoInfo::GetHeight() const			{ return height_; }
Rational			PhotoInfo::GetFocalLength() const		{ return focal_length_; }
uint16				PhotoInfo::GetExposureProgram() const	{ return exposure_program_; }
const MakerNote*	PhotoInfo::GetMakerNote() const			{ return maker_note_.get(); }
const String&		PhotoInfo::GetMake() const				{ return make_; }
const String&		PhotoInfo::GetModel() const				{ return model_; }
uint16				PhotoInfo::GetMeteringMode() const		{ return metering_mode_; }
uint16				PhotoInfo::GetLightSource() const		{ return light_source_; }
uint16				PhotoInfo::GetFlash() const				{ return flash_; }
const std::wstring&	PhotoInfo::GetExifDescription() const	{ return description_; }
uint32				PhotoInfo::GetVisitedDirId() const		{ return dir_visited_; }
const GPSData*		PhotoInfo::GetGpsData() const			{ return gps_data_.get(); }
Rational			PhotoInfo::GetSubjectDistance() const	{ return subject_distance_; }
const std::wstring&	PhotoInfo::PhotoDescription() const		{ return photo_desc_; }
String				PhotoInfo::GetLensModel() const			{ return lens_model_; }


void PhotoInfo::SetVisitedDirId(uint32 dir)
{
	dir_visited_ = dir;
}

void PhotoInfo::SetGpsData(AutoPtr<GPSData> gps)
{
	gps_data_ = gps;
}

void PhotoInfo::SetExifDescription(const std::wstring& desc)
{
	description_ = desc;
}

void PhotoInfo::SetSubjectDistance(Rational sd)
{
	subject_distance_ = sd;
}

void PhotoInfo::SetApertureValue(Rational av)
{
	aperture_value_ = av;
}

void PhotoInfo::SetShutterSpeedValue(SRational ssv)
{
	shutter_speed_value_ = ssv;
}

void PhotoInfo::SetFocalLength(Rational fl)
{
	focal_length_ = fl;
}

void PhotoInfo::SetFlash(uint16 f)
{
	flash_ = f;
}

void PhotoInfo::SetLightSource(uint16 ls)
{
	light_source_ = ls;
}

void PhotoInfo::SetMeteringMode(uint16 mm)
{
	metering_mode_ = mm;
}

void PhotoInfo::SetModel(const String& model)
{
	model_ = model;
}

void PhotoInfo::SetMake(const String& make)
{
	make_ = make;
}

void PhotoInfo::SetMakerNote(AutoPtr<MakerNote> maker_note)
{
	maker_note_ = maker_note;
}

void PhotoInfo::SetFileSize(uint64 size)
{
	file_size_ = size;
}

void PhotoInfo::SetExposureBias(SRational eb)
{
	exposure_bias_ = eb;
}

void PhotoInfo::SetExposureProgram(uint16 ep)
{
	exposure_program_ = ep;
}

void PhotoInfo::SetExposureTime(Rational et)
{
	exposure_time_ = et;
}

void PhotoInfo::SetFStop(Rational fn)
{
	f_number_ = fn;
}

void PhotoInfo::SetPhotoName(const TCHAR* name)
{
	name_ = name;
}

void PhotoInfo::SetPhotoName(const String& name)
{
	name_ = name;
}

void PhotoInfo::SetSize(uint32 w, uint32 h)
{
	width_ = w;
	height_ = h;
	cached_effective_orientation_ = CalcPhotoOrientation();
}

void PhotoInfo::SetLensModel(const String& lens)
	{
	lens_model_ = lens;
	}


uint16 PhotoInfo::ThumbnailOrientation() const
{
	return static_cast<uint16>(thumbnail_orientation_);
}


uint16 PhotoInfo::OrientationField() const
{
	return orientation_;
}


void PhotoInfo::SetWidth(uint32 w)
{
	width_ = w;
	cached_effective_orientation_ = CalcPhotoOrientation();
}


void PhotoInfo::SetHeight(uint32 h)
{
	height_ = h;
	cached_effective_orientation_ = CalcPhotoOrientation();
}


void PhotoInfo::SetThumbnailOrientation(uint16 o)
{
	thumbnail_orientation_ = o;
	cached_effective_orientation_ = CalcPhotoOrientation();
}


void PhotoInfo::SetOrientation(uint16 o)
{
	orientation_ = o;
	cached_effective_orientation_ = CalcPhotoOrientation();
}


bool PhotoInfo::IsRaw() const
{
	return is_raw_;
}
