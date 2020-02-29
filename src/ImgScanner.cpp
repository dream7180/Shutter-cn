/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImgScanner.h"
#include "BmpFunc.h"
#include "resource.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "JPEGException.h"
#include "ReloadJob.h"
#include "PhotoInfoCRW.h"
#include "PhotoInfoJPEG.h"
#include "MemPointer.h"
#include "Config.h"
#include "JPEGEncoder.h"
#include "MemoryDataDestination.h"
#include "Database/ImageDatabase.h"
#include "ThumbnailSize.h"
#include <math.h>
#include "ExifBlock.h"
#include "GenThumbMode.h"
#include "CatchAll.h"
#include "CatalogFile.h"
#include "CatalogImgRecord.h"
#include "PhotoInfoCatalog.h"
#include <boost/shared_ptr.hpp>
#include "CatalogPath.h"
#include "PhotoFactory.h"
#include "XMPAccess.h"
#include "PhotoAttr.h"
#include "DateTimeUtils.h"
#include "FileStatus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#define TEST_SPEED

#ifdef TEST_SPEED

#define START_TIMING	\
				LARGE_INTEGER _tm; \
				char _buf[16]; \

#define PRINT_TIMING(str)	\
				::QueryPerformanceCounter(&_tm); \
				ultoa((unsigned long)_tm.QuadPart, _buf, 10); \
				::OutputDebugStringA("\n" str "\t"); \
				::OutputDebugStringA(_buf);
#else

#define START_TIMING
#define PRINT_TIMING(str)

#endif

extern void ReadFileTimeStamp(PhotoInfo& photo, const Path& path);
extern int GetLogicalProcessorInfo();

namespace {
	const DWORD SYNCHRO_DELAY= 400; // ms

	void ShowMessageBox(const TCHAR* msg)
	{
		::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), msg, _T("ExifPro"), MB_OK | MB_ICONERROR);
	}
}


ImgScanner::ImgScanner(const TCHAR* path, const TCHAR* selectedFile/*= 0*/)
  : dir_to_scan_(path)
{
	const size_t cores= GetLogicalProcessorInfo();

	for (size_t i= 0; i < cores; ++i)
		processor_.create_thread(boost::bind(&ImgScanner::ProcessingThread, this));

	if (selectedFile)
		selected_file_ = selectedFile;
	visited_dir_id_ = 0x40000000;
	cancel_ = false;
	dir_visited_ = 0;
}


// make sure description contains no junk characters
static void EliminateNonPrintableChars(std::wstring& str)
{
	std::wstring::size_type n= str.find_last_not_of(L'\0'); // == wstring::npos)
	if (n != std::wstring::npos)
		str.erase(n + 1);

	size_t size= str.length();
	for (size_t i= 0; i < size; ++i)
	{
		int c= str[i];
//TODO
		if (c == 0)
			c = 0;
	}
}


extern void PostProcessTime(PhotoInfoPtr info)
{
	// scanner stored offset (fraction of the second; 1/100 s) of the time photo was taken;
	// now add base time/date to come up with an exact photo creation time in [ms]

//	info->SetExactTime(uint64(info->GetDateTime().GetTime()) * 1000 + info->GetPhotoExactTime() * 10);
	if (auto s= info->GetSubSeconds())
	{
		auto dt= info->GetDateTime();
		// milliseconds
		const auto adjustor= TimeDuration::ticks_per_second() / 1000;
		dt += TimeDuration(0, 0, 0, s * 10 * adjustor);
		info->SetDateTime(dt);
	}
}


// detecting black (this may be specific for current jpeg lib)
// huge amount of noise, so tolerance is fairly big
static bool line_test_yuv(const BYTE* line)		{ return line[0] > 0x38U || line[1] - 0x6bU > 0x2aU || line[2] - 0x6bU > 0x2aU; }
static bool line_test_rgb(const BYTE* line)		{ return line[0] > 0x20U || line[1] > 0x20U || line[2] > 0x20U; }
static uint32 difference_yuv(const BYTE* line)	{ return line[0] + abs(line[1] - 0x80) + abs(line[2] - 0x80); }
static uint32 difference_rgb(const BYTE* line)	{ return line[0] + line[1] + line[2]; }


// remove black strips at the top and bottom of dib (if there are any); input bmp in YCbCr format!
//
extern bool StripBlackFrame(Dib& dib, bool YUV)
{
	if (dib.GetColorComponents() != 3)
		return false;

	int width= dib.GetWidth();
	int height= dib.GetHeight();

	const int cut_off= 9;
	int top_line= 0;
	int bottom_line= height;
	int left_edge= 0;
	int right_edge= width;
	const int TOLERANCE= 20;

	bool (*line_test)(const BYTE* line)= YUV ? line_test_yuv : line_test_rgb;
	uint32 (*difference_fn)(const BYTE* line)= YUV ? difference_yuv : difference_rgb;

	if (height >= 80 && width >= 80)	// real image?
	{
		bool black_stripes= true;
		for (int line= 0; line < cut_off; ++line)
		{
			uint32 difference= 0;
			BYTE* line_buf= dib.LineBuffer(line);

			for (int x= 0; x < width; ++x, line_buf += 3)
				if (line_test(line_buf))
				{
					black_stripes = false;
					break;
				}
				else
					difference += difference_fn(line_buf);

			if (!black_stripes)
				break;

			int tolerance= line > 5 ? TOLERANCE * 2 : TOLERANCE;

			if (difference / width > tolerance)
				break;

			top_line = line + 2;	// note: extra line to remove
		}

		black_stripes = true;
		for (int line= height - 1; line > height - cut_off - 1; --line)
		{
			uint32 difference= 0;
			BYTE* line_buf= dib.LineBuffer(line);

			for (int x= 0; x < width; ++x, line_buf += 3)
				if (line_test(line_buf))
				{
					black_stripes = false;
					break;
				}
				else
					difference += difference_fn(line_buf);

			if (!black_stripes)
				break;

			int tolerance= height - line > 5 ? TOLERANCE * 2 : TOLERANCE;

			if (difference / width > tolerance)
				break;

			bottom_line = line - 1;	// extra line removed
		}

		black_stripes = true;
		for (int x= 0; x < cut_off; ++x)
		{
			uint32 difference= 0;

			for (int y= top_line; y < bottom_line; ++y)
			{
				BYTE* line_buf= dib.LineBuffer(y) + 3 * x;
				if (line_test(line_buf))
				{
					black_stripes = false;
					break;
				}
				else
					difference += difference_fn(line_buf);
			}

			if (!black_stripes)
				break;

			int tolerance= x > 5 ? TOLERANCE * 2 : TOLERANCE;

			if (difference / (bottom_line - top_line) > tolerance)
				break;

			left_edge = x + 2;	// extra column removed
		}

		black_stripes = true;
		for (int x= width - 1; x > width - cut_off - 1; --x)
		{
			uint32 difference= 0;

			for (int y= top_line; y < bottom_line; ++y)
			{
				BYTE* line_buf= dib.LineBuffer(y) + 3 * x;
				if (line_test(line_buf))
				{
					black_stripes = false;
					break;
				}
				else
					difference += difference_fn(line_buf);
			}

			if (!black_stripes)
				break;

			int tolerance= width - x > 5 ? TOLERANCE * 2 : TOLERANCE;

			if (difference / (bottom_line - top_line) > tolerance)
				break;

			right_edge = x - 1;	// extra column removed
		}

		if (top_line > 0 || bottom_line < height || left_edge > 0 || right_edge < width)
		{
			// remove black stripes

			int lines= bottom_line - top_line;
			int cols= right_edge - left_edge;
			Dib copy(cols, lines, dib.GetBitsPerPixel());

			for (int line= 0; line < lines; ++line)
			{
				BYTE* line_buf= dib.LineBuffer(line + top_line) + left_edge * 3;
				memcpy(copy.LineBuffer(line), line_buf, copy.GetBytesPerLine());
			}

			dib.Swap(copy);

			return true;
		}
	}

	return false;
}


bool operator == (const FILETIME& f1, const FILETIME& f2)
{
	return f1.dwLowDateTime == f2.dwLowDateTime && f1.dwHighDateTime == f2.dwHighDateTime;
}


// after reading image check its orientation EXIF field: if it indicates
// orientation other than normal store it in the rotation flag, so decoder job
// will use it
extern void SetAutoRotationFlag(PhotoInfoPtr photo)
{
	bool photoIsRotated= photo->GetWidth() < photo->GetHeight();

	// if photo is physically rotated, then ignore EXIF orientation; it only
	// applies to the pristine files output by the digicam, not postprecessed
	if (photoIsRotated)
		return;

	PhotoInfo::ImgOrientation orient= photo->GetOrientation();

	if (orient == PhotoInfo::ORIENT_90CCW)
		photo->rotation_flag_ = PhotoInfo::RF_90CW | PhotoInfo::RF_AUTO;
	else if (orient == PhotoInfo::ORIENT_90CW)
		photo->rotation_flag_ = PhotoInfo::RF_90CCW | PhotoInfo::RF_AUTO;
	else if (orient == PhotoInfo::ORIENT_UPSDN)
		photo->rotation_flag_ = PhotoInfo::RF_UPDN | PhotoInfo::RF_AUTO;
}


//////////////////////////////////////


void ImgScanner::ProcessingThread()
{
//	ImgScanner* scanner= static_cast<ImgScanner*>(param);

	for (;;)
	{
		FilePtr file= wait_list_.remove_head();

		if (!file)
			break;

		SmartPhotoPtr photo= FileScan(file->path, file->file_length, file->file_time, file->dir_visited, true, false);

		if (photo.get())
		{
			// add new photo (thread safe)
			photos_->Append(photo);
			Notify();
		}
	}
}


ImgScanner::~ImgScanner()
{
	wait_list_.shutdown();
}


void ImgScanner::QueueFileEntry(FilePtr p)
{
	for (;;)
	{
		if (wait_list_.is_shut_down())
			break;	// no need to add anything, the queue is shut down
		
		if (wait_list_.add_tail(p))
			break;	// adding succeeded

		// when there's no space left in a queue, wait for processing thread to catch up
		::Sleep(1);

		if (cancel_)
			break;
	}
}

//////////////////////////////////////


// when generating images limit their size (to fit the square box of given size)
static const int LIMIT_THUMBNAIL_SIZE= 180;

bool ImgScanner::FileScanAndAppend(Path path, uint64 file_length, const CFileFind& find, uint32 dir_visited, bool scanSubdirs, bool selectedFile)
{
	if (path.MatchExtension(_T("catalog")))
	{
		if (!scanSubdirs || !g_Settings.ScanFileType(FT_CATALOG))
			return true;

		// open catalog & iterate through its dirs
		ScanCatalog(path, visited_dir_id_, scanSubdirs);

//		ResetDirVisitedId(dir_visited);

		return true;
	}

	if (!selectedFile)
	{
		PhotoFactory::CreateFn fn= 0;
		int id= 0;

		if (GetPhotoFactory().MatchPhotoType(path.GetExtension(), fn, id) && g_Settings.ScanFileType(id))
			QueueFileEntry(FilePtr(new File(path, file_length, find, dir_visited)));
	}
	else
	{
		FILETIME ftWrite= { 0, 0 };
		find.GetLastWriteTime(&ftWrite);

		SmartPhotoPtr photo= FileScan(path, file_length, ftWrite, dir_visited, scanSubdirs, selectedFile);
		if (photo.get())
		{
			photos_->Append(photo);

			Notify();
		}
	}

	return true;
}


SmartPhotoPtr ImgScanner::FileScan(Path path, uint64 file_length, const FILETIME& file_time, uint32 dir_visited, bool scanSubdirs, bool selectedFile)
{
START_TIMING
PRINT_TIMING("Img start")

	bool generate_thumbnails= true;

	switch (g_Settings.regenerate_thumbnails_)
	{
	case GEN_THUMB_ALWAYS:
		generate_thumbnails = true;
		break;
	case GEN_THUMB_EXCEPT_REMOVABLE_DRV:
		// do NOT regenerate thumbnails when reading from removable drive
		generate_thumbnails = ::GetDriveType(path.GetRoot().c_str()) != DRIVE_REMOVABLE;
		break;
	case GEN_THUMB_NEVER:
		generate_thumbnails = false;
		break;
	default:
		ASSERT(false);
		break;
	}

	bool CRW= path.MatchExtension(_T("crw")) && (g_Settings.ScanFileType(FT_CRW) || selectedFile);
	bool jpeg_ext= (path.MatchExtension(_T("jpg")) || path.MatchExtension(_T("jpeg")) ||
		path.MatchExtension(_T("jpe"))) && (g_Settings.ScanFileType(FT_JPEG) || selectedFile);

	bool thumb_generated_from_img= false;
	SmartPhotoPtr photo;

	if (CRW || jpeg_ext)
	{
		try
		{
			SmartPhotoPtr info(CRW ? static_cast<PhotoInfo*>(new PhotoInfoCRW) : static_cast<PhotoInfo*>(new PhotoInfoJPEG));

			bool has_exif= false;
			bool found_in_db= false;
			ImgDataRecord img;
			uint64 record_offset= 0;

			if (
#ifdef _DEBUG
				(::GetAsyncKeyState(VK_SHIFT) >= 0 || ::GetAsyncKeyState(VK_CONTROL) >= 0) &&
#endif
				Database() && Database()->IsOpen() && (record_offset = Database()->FindImage(path, img)) != 0)
			{
				// verify file length & modification time
				FILETIME ftWrite= file_time;// { 0, 0 };//, ftAccess= { 0, 0 };
				//find.GetLastWriteTime(&ftWrite);

				if (img.file_size_ == file_length && img.time_stamp_ == ftWrite)
				{
					if (generate_thumbnails && (img.flags_ & ImgDataRecord::REGENERATED_THUMBNAIL) == 0)
					{
						// regenerate thumbnail, the one we have here comes from photo
						found_in_db = false;
					}
					else
					{
PRINT_TIMING("Found in db")
						found_in_db = true;
						generate_thumbnails = false;	// the one in db has already been generated
						img.bufEXIF_.reserve(img.bufEXIF_.size() + 2);
						// append 'end of image'
						img.bufEXIF_.push_back(0xff);
						img.bufEXIF_.push_back(0xd9);
						// scan 'img'
						has_exif = info->Scan(path.c_str(), img.bufEXIF_, Logger());

						// if we have thumbnail image provided separately--use it (even instead
						// of one in EXIF block--my thumbnail may have black strips removed)
						if (/*info->jpeg_thumb_.IsEmpty() &&*/ !img.buf_thumbnail_.empty())
							info->jpeg_thumb_.SwapBuffer(img.buf_thumbnail_);
//info->jpeg_thumb_.Empty();
//info->output_.Clear();
						// restore index for similarity sorting
						if (!img.buf_index_.empty())
							info->index_.ConstructFromBuffer(img.buf_index_);

						//if (!has_exif || info->width_ == 0 && info->height_ == 0)
						// img struct should always contain real image size; EXIF may be bogus
						{
							info->SetSize(img.img_width_, img.img_height_);
						}

						info->jpeg_offset_ = img.jpeg_offset_;
						info->jpeg_size_ = img.jpeg_img_size_;

						// this is for CRW only: restore orientation
						if (!has_exif && info->OrientationField())
							info->SetOrientation(img.exif_orientation_);
					}
				}
PRINT_TIMING("Decoded")
			}

			// if not found in db then load file and parse it
			if (!found_in_db)
			{
				// this is crucial: remove old img remainings
				img.Clear();

				ExifBlock exif;
				has_exif = info->Scan(path.c_str(), exif, false, Logger());	// scan image to find EXIF data

				if (CRW && !has_exif)
				{
					Path thm= path;
					thm.ReplaceExtension(_T("thm"));
					if (thm.FileExists())
						has_exif = info->Scan(thm.c_str(), exif, false, Logger());	// scan thumbnail image to find EXIF data
				}

				img.bufEXIF_.swap(exif.exif_buffer);
			}

			if (!has_exif && ReadExifImagesOnly() && !selectedFile)
				return SmartPhotoPtr();

			info->SetVisitedDirId(dir_visited);
			info->SetFileSize(file_length);
			info->SetPhotoName(path.GetFileName());
			info->SetPhysicalPath(path);
			info->exif_data_present_ = has_exif;
			if (jpeg_ext)
				info->lossless_crop_ = true;

			if (!has_exif || info->GetDateTime().is_not_a_date_time())
				ReadFileTimeStamp(*info, path);

#ifdef _DEBUG
			if (!generate_thumbnails)
				generate_thumbnails = ::GetAsyncKeyState(VK_SHIFT) < 0 && ::GetAsyncKeyState(VK_CONTROL) < 0;
#endif

			// is there no thumbnail (or regenerate it)?
			if (generate_thumbnails ||
				((!has_exif || info->bmp_.get() == 0 || !info->bmp_->IsValid()) && info->jpeg_thumb_.IsEmpty()))
			{
				try
				{
					AutoPtr<Dib> bmp= new Dib;

					CFileDataSource fsrc(path.c_str(), info->jpeg_offset_);

					AutoPtr<JPEGDecoder> dec(new JPEGDecoder(fsrc, JDEC_INTEGER_HIQ));
					dec->SetFast(true, true);
					int big= LIMIT_THUMBNAIL_SIZE;
					CSize thumb_size(big, big);
					ImageStat stat= dec->DecodeImgToYCbCr(*bmp, thumb_size, true, RGB(0,0,0));
					if (stat == IS_OPERATION_NOT_SUPPORTED)
					{
						// grayscale images cannot be converted to YCbCr by turbo jpeg lib
						dec = new JPEGDecoder(fsrc, JDEC_INTEGER_HIQ);
						dec->SetFast(true, true);
						stat = dec->DecodeImg(*bmp, thumb_size, true, RGB(0,0,0));
					}
					if (stat != IS_OK)
						img.flags_ |= ImgDataRecord::BROKEN_PHOTO;

					CSize s= dec->GetOriginalSize();
					info->SetSize(s.cx, s.cy);//dec.image_width;//dec.image_height;

					img.flags_ |= ImgDataRecord::REGENERATED_THUMBNAIL;

					int orientation= info->ThumbnailOrientation() & ~PhotoAttr::MODIFIED_FLAG;
					if (orientation)
					{
						bool mirror= !!(orientation & PhotoAttr::MIRROR_FLAG);

						orientation &= ~PhotoAttr::MIRROR_FLAG;

						if (orientation)
						{
							// photo was rotated in ExifPro--"unrotate" thumbnail generated from this
							// altered photo, so it remains in an original landscape state (just
							// like an original one embedded by a camera)
							bmp->RotateInPlace(orientation == 3);
							if (orientation == 2)
								bmp->RotateInPlace(false);
						}

						if (mirror)
							::FlipBitmap(*bmp, true, false);
					}
					else if (info->GetWidth() < info->GetHeight())
					{
						// photo was rotated in some other application
						//TODO?
//						bmp->RotateInPlace(false);
					}

					info->bmp_ = bmp;
					thumb_generated_from_img = true;
				}
				catch (JPEGException& ex)	// jpeg decoding error?
				{
					info->bmp_ = 0;
					LogError(String(_T("生成缩略图出错. ")) + ex.GetMessage(), path);
					return SmartPhotoPtr();	// skip this image
				}
			}

			if (Dib* thumbnail= info->bmp_.get())
			{
				//if (info->jpeg_thumb_.IsEmpty())		// no JPEG thumbnail source available?
				//	if (info->bmp_.get())			// then leave decoded thumbnail intact and orient it
				//		info->OrientThumbnail(*info->bmp_);

				if (!found_in_db)
					info->index_.CalcHistogram(*info->bmp_);
			}

			// update cache file if db is open and image is not yet there
			if (!found_in_db && Database() && Database()->IsOpen())
			{
				img.path_ = info->GetPhysicalPath(); //TODO: virtual paths as keys in the cache db
				img.file_size_ = file_length;
				//find.GetLastWriteTime(&img.time_stamp_);
				img.time_stamp_ = file_time;
				info->index_.Serialize(img.buf_index_);
				img.img_width_ = info->GetWidth();
				img.img_height_ = info->GetHeight();
				img.jpeg_offset_ = info->jpeg_offset_;
				img.jpeg_img_size_ = info->jpeg_size_;
				// HACK: this is orientation extracted from CRW file; when reading info from
				// cache CRW is not scanned, so orientation cannot be set
				img.exif_orientation_ = has_exif ? 0u : info->OrientationField();

				if (info->bmp_.get())
				{
					// check photo & thumbnail size ratio

					bool same_ratio= false;

					CSize bmp_size= info->bmp_->GetSize();

					if (info->GetWidth() > 0 && info->GetHeight() > 0 && bmp_size.cx > 0 && bmp_size.cy > 0)
					{
						double ratio_photo= info->GetWidth() > info->GetHeight() ?
							double(info->GetWidth()) / info->GetHeight() : double(info->GetHeight()) / info->GetWidth();

						double ratio_thumb= bmp_size.cx > bmp_size.cy ?
							double(bmp_size.cx) / bmp_size.cy : double(bmp_size.cy) / bmp_size.cx;

						same_ratio = fabs(ratio_photo - ratio_thumb) < 0.001;
					}

					// some cameras (like Canon D-60) store thumbnail in 160x120 format with black stips;
					// remove black strips (if any) and modify the dib
					if (!same_ratio && !thumb_generated_from_img)
					{
						if (bmp_size.cy > bmp_size.cx)
							info->bmp_->RotateInPlace(true);

						if (::StripBlackFrame(*info->bmp_, true))
							info->jpeg_thumb_.Empty();	// remove this thumbnail; it'll be regenerated below

						if (bmp_size.cy > bmp_size.cx)
							info->bmp_->RotateInPlace(false);
					}
				}

				if (generate_thumbnails || info->jpeg_thumb_.IsEmpty())	// no valid thumbnail present in EXIF data?
				{
					// create one now (compress info->bmp_)
					if (info->bmp_.get())
					{
						try
						{
							info->bmp_->ConvertYCbCr2RGB();

							Dib dib;
							CSize size= info->bmp_->GetSize();
							int big= LIMIT_THUMBNAIL_SIZE;
							if (size.cx > big || size.cy > big)
								info->bmp_->ResizeToFit(CSize(big, big), Dib::RESIZE_HALFTONE);

							// compress thumbnail into JPEG
							CMemoryDataDestination memdest;
							JPEGEncoder dec(98, false, false);
							dec.Encode(memdest, info->bmp_.get());
							memdest.SwapJPEG(img.buf_thumbnail_);
						}
						catch (...)
						{
							LogError(_T("解码缩略图出错"), path);
						}
					}
				}

				// add/modify record
				record_offset = Database()->AddModify(img);

				if (generate_thumbnails || info->jpeg_thumb_.IsEmpty())	// no valid thumbnail present in EXIF data?
					if (img.buf_thumbnail_.size() > 0)
						info->jpeg_thumb_.SwapBuffer(img.buf_thumbnail_);

			}

			SetAutoRotationFlag(info.get());

			info->bmp_ = 0;						// discard decoded thumbnail
			info->SetRecordOffset(record_offset);	// remember db record's location for later access

			//TODO: maybe?
			if (g_Settings.read_thumbs_from_db_)
				info->jpeg_thumb_.Empty();

			EliminateNonPrintableChars(info->photo_desc_);
			PostProcessTime(info.get());

//			photos_->Append(info);
			//int index= photos_->GetCount() - 1;
			photo = info;
		}
		catch (const JPEGException& ex)	// jpeg decoding error?
		{
			//CString msg= path.c_str();
			//msg += _T('\n');
			//msg += ex.GetMessage();
			LogError(ex.GetMessage(), path);
			//::ShowMessageBox(msg);
		}
		catch (MemPointer::MemPtrException&)
		{
			//CString msg= _T("Error parsing file: ");
			//msg += path.c_str();
			//::ShowMessageBox(msg);
			LogError(_T("解析文件出错"), path);
		}
		catch (CFileException* ex)		// file reading/opening problem
		{
			//ex->ReportError();
			const int MAX= 512;
			TCHAR msg[MAX+2];
			if (ex->GetErrorMessage(msg, MAX))
				LogError(msg, path);
			else
				LogError(_T("文件错误"), path);
			ex->Delete();
		}
		catch (Exception& ex)
		{
			LogError(ex.GetDescription(), path);
		}
#ifndef _DEBUG
		catch (...)
		{
			LogError(_T("遭遇致命错误"), path);
		}
#endif
	}
	else
	{
		PhotoFactory::CreateFn fn= 0;
		int id= 0;

		// find photo type using factory of supported types
		if (GetPhotoFactory().MatchPhotoType(path.GetExtension(), fn, id) &&
			(g_Settings.ScanFileType(id) || selectedFile))
		{
			photo = fn();
		}

PRINT_TIMING("Parent notified")

		if (photo.get())
		{
			try
			{
				PhotoInfoPtr photo_tmp= photo.get();	// keep pointer here
				ReadImage(photo, path, file_length, dir_visited, generate_thumbnails, selectedFile);
				EliminateNonPrintableChars(photo_tmp->photo_desc_);
				PostProcessTime(photo_tmp);
			}
			catch (MemPointer::MemPtrException&)
			{
				LogError(_T("解析文件出错"), path);
				//CString msg= _T("Error parsing file: ");
				//msg += path.c_str();
				//::ShowMessageBox(msg);
			}
			catch (CMemoryException*)
			{
				LogError(_T("读取文件时内存溢出"), path);
				//CString msg= _T("Out of memory reading file: ");
				//msg += path.c_str();
				//::ShowMessageBox(msg);
			}
			catch (std::exception& ex)
			{
				CString msg(ex.what());
				LogError(msg, path);
				//msg += _T("\n");
				//msg += path.c_str();
				//::ShowMessageBox(msg);
			}
			catch (ImageStat err)
			{
				// continue; skipping this image
				LogError(::ImageStatMsg(err), path);
			}
#ifndef _DEBUG
			catch (...)
			{
				LogError(_T("遭遇致命错误"), path);
				ASSERT(false);
			}
#endif
		}
	}

	return photo;
}


void ImgScanner::ReadImage(SmartPhotoPtr& info, const Path& path, uint64 file_length, uint32 dir_visited, bool generateThumbnails, bool selectedFile)
{
	info->SetPhysicalPath(path);

	//TODO: cache non-jpeg images too
	bool found_in_db= false;

	ExifBlock exif;
	bool has_exif= false;
	try
	{
		has_exif = info->Scan(path.c_str(), exif, generateThumbnails, Logger());	// scan image to find EXIF data
	}
	catch (String& str)
	{
		LogError(_T("加载照片出错. ") + str, path);
	}
	catch (std::exception& ex)
	{
		LogError(_T("加载照片出错. ") + CString(ex.what()), path);
	}
	catch (ImageStat err)
	{
		LogError(::ImageStatMsg(err), path);
	}
	catch (...)
	{
		LogError(_T("加载照片出错."), path);
	}

	try
	{
		// load XMP (note: this is physical file read; when db is supported this may not be needed...)
		XmpData xmp;
		if (info->LoadMetadata(xmp))
		{
			//Xmp::MetaToXmpData(xmp_buf, xmp);
			info->SetMetadata(xmp);
		}
//		vector<String> tags;
//		Xmp::LoadPhotoTags(path, tags);
//		info->tags_.AssignKeywords(tags);
	}
	catch (String& str)
	{
		LogError(_T("加载 XMP 元数据出错: ") + str, path);
	}
	catch (...)
	{
		//log? report?
		LogError(_T("加载 XMP 元数据出错"), path);
	}

	if (!has_exif && ReadExifImagesOnly() && !selectedFile)
		return;

	if (!has_exif || info->GetDateTime().is_not_a_date_time())
		ReadFileTimeStamp(*info, path);

	info->SetVisitedDirId(dir_visited);
	info->SetFileSize(file_length);
	info->SetPhotoName(path.GetFileName());
	info->exif_data_present_ = has_exif;

	SetAutoRotationFlag(info.get());

//	if (!has_exif || info->bmp_.get() == 0 || !info->bmp_->IsValid())
//	{
/*
		try
		{
			info->bmp_ = AutoPtr<Dib>(new Dib);

			CImageDecoderPtr decoder= info->GetDecoder();
			CSize thumb_size(STD_THUMBNAIL_WIDTH, STD_THUMBNAIL_HEIGHT);
			// TODO: ideally decode to YCbCr
			if (decoder->DecodeImg(*info->bmp_, thumb_size, true) != IS_OK)
				info->bmp_ = 0;
			else
			{
				CSize s= decoder->GetOriginalSize();
				info->width_ = s.cx;
				info->height_ = s.cy;
			}
			//dec.SetFast(true, true);
			//dec.DecodeImgToYCbCr(*info->bmp_, thumb_size, true, RGB(0,0,0));

			//info->width_ = dec.image_width;
			//info->height_ = dec.image_height;
		}
		catch (...)	// decoding error?
		{
//			info->bmp_ = 0;
			return;	// skip this image
		} */
//	}

	if (Dib* thumbnail= info->bmp_.get())
	{
		if (!found_in_db)
		{
			//TODO ConvertRGB2YCbCr()
			info->index_.CalcHistogram(*info->bmp_);
		}
	}

	info->bmp_ = 0;

//	photos_->Append(info.release());
}


extern void ReadFileTimeStamp(PhotoInfo& photo, const Path& path)
{
	auto d= GetFileModifiedTime(path.c_str());
	if (d.is_not_a_date_time())
		return;

	photo.SetDateTime(d);
}


void ImgScanner::MessageBox(const TCHAR* msg, int, int)	// msg box for CATCH_ALL
{
	ShowMessageBox(msg);
}


void ImgScanner::ForwardNotification(ImageScanner* scanner)
{
	if (cancel_)
		scanner->CancelScan();
	else
		Notify();
}


void ImgScanner::ScanCatalog(const Path& path, uint32& dirVisited, bool scanSubdirs)
{
	try
	{
		// create catalog scanner

		CatalogPath catalog_path(path);

		std::auto_ptr<ImageScanner> scanner= catalog_path.GetScanner();

		// use our database
		// not needed for catalog... scanner.SetImageDatabase();

		// use our notification callback
		scanner->SetNotificationCallback(boost::bind(&ImgScanner::ForwardNotification, this, scanner.get()));

		// scan
		scanner->Scan(scanSubdirs, dirVisited, *photos_);
	}
	CATCH_ALL
}


bool ImgScanner::Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store)
{
	photos_ = &store;

	dir_visited_ = dirVisited;

	bool finished= ScanDir(dir_to_scan_, visitSubDirs, &selected_file_);

	dirVisited = dir_visited_;

	const size_t size= processor_.size();
	for (size_t i= 0; i < size; ++i)
		QueueFileEntry(FilePtr()); // signal end

	// wait for image processing threads to finish processing queue
	processor_.join_all();

	return finished;
}


bool ImgScanner::ScanDir(Path path, bool scan_subdirs, const String* selectedFile)
{
	ASSERT(!path.empty());

	CFileFind find;
	path.AppendAllMask();
	BOOL working= find.FindFile(path.c_str());
	if (!working)
		return true;	// just continue

	uint32 dir_visited= ++dir_visited_;

	if (selectedFile && !selectedFile->empty())
	{
		// selected file is a file name from the root folder; it is to be scanned first
		Path selected= path.GetDir();
		selected.AppendDir(selectedFile->c_str(), false);

		CFileFind selFile;
		if (selFile.FindFile(selected.c_str()))
		{
			selFile.FindNextFile();
			if (!FileScanAndAppend(Path(selFile.GetFilePath()), selFile.GetLength(), selFile, dir_visited, scan_subdirs, true))
				return true;
		}
	}

	while (working)
	{
		if (cancel_)
			return false;

		working = find.FindNextFile();

		if (find.IsDots())
			continue;

		if (find.IsDirectory())
		{
			if (scan_subdirs)
			{
				if (!ScanDir(Path(find.GetFilePath()), scan_subdirs, 0))
					return false;
			}
			continue;
		}

#if _MSC_VER < 1310	// not vc 7.1?
		int64 len= find.GetLength64();
#else
		auto len= find.GetLength();
#endif
		if (selectedFile && !selectedFile->empty() && find.GetFileName().CompareNoCase(selectedFile->c_str()) == 0)
			continue;	// skip selected file, it has already been scanned

		if (!FileScanAndAppend(Path(find.GetFilePath()), len, find, dir_visited, scan_subdirs, false))
			return true;
	}

	return true;
}


void ImgScanner::CancelScan()
{
	cancel_ = true;
	wait_list_.shutdown();
}


SmartPhotoPtr ImgScanner::SingleFileScan(Path path, uint32 dir_visited)
{
	CFileFind file;
	if (!file.FindFile(path.c_str()))
		return SmartPhotoPtr();

	file.FindNextFile();

	FILETIME file_time;
	file.GetLastWriteTime(&file_time);

	return FileScan(Path(file.GetFilePath()), file.GetLength(), file_time, dir_visited, false, false);
}
