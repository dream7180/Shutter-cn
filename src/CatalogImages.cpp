/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CatalogImages.h"
#include "PhotoInfo.h"
#include "PhotoFactory.h"
#include "Dib.h"
#include "Config.h"
#include "JPEGEncoder.h"
#include "MemoryDataDestination.h"
#include "Database/Database.h"
#include "ExifBlock.h"
#include "intrusive_ptr.h"
#include "DirScanner.h"
#include <deque>
#include <map>
#include "MemPointer.h"
#include "CatalogImgRecord.h"
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include "CatalogHeader.h"
#include "StringConversions.h"
#include "JPEGException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct CatalogImages::Impl : public DirScanner
{
	HWND parentWnd_;
	bool readOnlyPhotosWithEXIF_;
	Database& dbImages_;
	//CSize thumbnail_size_;
	//int jpeg_compression_level_;
	std::deque<std::pair<Path, CatalogHeader::DirPtr>> files_;
	bool break_;
	std::deque<CatalogHeader::DirPtr> dir_stack_;
	CatalogHeader::DirPtr root_dir_;
	CatalogHeader::DirPtr cur_dir_;
	size_t counter_;
	std::vector<bool> scan_types_;
	CatalogHeader header_;
	// this map keeps track of tags and offsets to images that have those tags set
	typedef std::map<String, std::vector<uint64> > TagMap;
	TagMap tags_;
	std::vector<Path> failed_;	// remember files that were skipped due to the image decoding failure

	Impl(Database& db) : dbImages_(db)
	{
		//thumbnail_size_ = CSize(320, 200);
		//jpeg_compression_level_ = 98;
		break_ = false;
	}

	void ReadFileTimeStamp(PhotoInfo& photo, const Path& path);
	void ProcessFiles(int img_size, int jpeg_compression_level);
	void ProcessFiles(const String& dir, bool subdirs, const std::vector<bool>& scan_types,
		const String& title, const String& description, int img_size, bool folder_scan, int jpeg_compr);
	void PrepareHeader(std::vector<uint8>& header, Path path, const String& title, const String& description,
		CatalogHeader::DirPtr dir, bool folder_scan);

	virtual void EnteringDir(const Path& path, int id);
	virtual void ExitingDir(int id);
	virtual bool FileScan(Path path, int64 file_length, const CFileFind& find, int dir_visited, bool scanSubdirs);
};



namespace {
	const DWORD SYNCHRO_DELAY= 400; // ms

	void ShowMessageBox(const TCHAR* msg)
	{
		::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), msg, _T("ExifPro"), MB_OK);
	}
}


CatalogImages::~CatalogImages()
{}


CatalogImages::CatalogImages(HWND parent_wnd, bool read_only_photos_withEXIF, Database& dbImages)
 : pImpl_(new Impl(dbImages))
{
	pImpl_->parentWnd_ = parent_wnd;
	pImpl_->readOnlyPhotosWithEXIF_ = read_only_photos_withEXIF;
	pImpl_->counter_ = 0;
}


bool CatalogImages::Impl::FileScan(Path path, int64 file_length, const CFileFind& find, int dir_visited, bool scanSubdirs)
{
	if (break_)
		return false;

	bool generate_thumbnails= true;

	PhotoFactory::CreateFn fn= 0;
	int id= 0;

	if (!GetPhotoFactory().MatchPhotoType(path.GetExtension(), fn, id))
		return true;

	const size_t size= scan_types_.size();
	if (id >= size)
	{
		ASSERT(false);
		return true;
	}
	if (!scan_types_[id])
		return true;

	files_.push_back(make_pair(path, cur_dir_));

	// reserved_capacity_ keeps track of max amount of files in a given folder; real number
	// can be smaller when decoding fails and some images are not saved;
	cur_dir_->reserved_capacity_++;

	if (parentWnd_)
		::PostMessage(parentWnd_, MESSAGE, files_.size(), DIR_SCANNING);

	return true;
}


extern void ReadFileTimeStamp(PhotoInfo& photo, const Path& path);

void CatalogImages::Impl::ReadFileTimeStamp(PhotoInfo& photo, const Path& path)
{
	::ReadFileTimeStamp(photo, path);
}


size_t CatalogImages::Count() const
{
	return pImpl_->files_.size();
}


void CatalogImages::ProcessFiles(const String& dir, bool subdirs, const std::vector<bool>& scan_types, const String& title,
								 const String& description, int img_size, bool folder_scan, int jpeg_compr)
{
	pImpl_->scan_types_ = scan_types;
	pImpl_->ProcessFiles(dir, subdirs, scan_types, title, description, img_size, folder_scan, jpeg_compr);
}


void CatalogImages::Impl::PrepareHeader(std::vector<uint8>& header, Path path,
										const String& title, const String& description,
										CatalogHeader::DirPtr dir, bool folder_scan)
{
	header_.version_ = 1;

	header_.img_count_ = 0;

	FILETIME ft;
	::GetSystemTimeAsFileTime(&ft);
	header_.creation_time_ = ft;		// catalog creation time (UTC)

	header_.scan_types_ = scan_types_;	// what types were scanned (FT_*)
	header_.title_ = String2WStr(title);
	header_.description_ = String2WStr(description);

	String root= path.GetRoot();
	int drv_type= 0;
	for (;;)
	{
		drv_type = ::GetDriveType(path.c_str());
		if (drv_type != DRIVE_NO_ROOT_DIR)
			break;

		if (path.length() > root.length())
			path = path.GetDir();
		else
			break;
	}
	header_.drive_type_ = drv_type;
	header_.drive_or_folder_ = folder_scan;

	TCHAR volume[MAX_PATH]= { 0 };
	DWORD serial_no= 0;
	DWORD dummy= 0;
	DWORD vol_flags= 0;
	if (::GetVolumeInformation(path.c_str(), volume, MAX_PATH, &serial_no, &dummy, &vol_flags, 0, 0) == 0)
	{
		path += _T("\\");
		if (::GetVolumeInformation(path.c_str(), volume, MAX_PATH, &serial_no, &dummy, &vol_flags, 0, 0) == 0)
			::GetVolumeInformation(root.c_str(), volume, MAX_PATH, &serial_no, &dummy, &vol_flags, 0, 0);
	}

	header_.volume_name_ = String2WStr(volume);
	header_.volume_serial_no_ = serial_no;
	header_.volume_flags_ = vol_flags;

	header_.directory_ = dir;

	header_.Write(header);
}


static void ResizeBuffer(std::vector<uint8>* buffer, size_t size, MemPointer* ptr)
{
	buffer->resize(size);
	ptr->Resize(&buffer->front(), buffer->size());
}


void CatalogImages::Impl::ProcessFiles(const String& dir, bool subdirs, const std::vector<bool>& scan_types,
								   const String& title, const String& description, int img_size, bool folder_scan,
								   int jpeg_compr)
{
	dir_stack_.clear();

	cur_dir_ = 0;
	root_dir_ = 0;

	ScanDir(dir, subdirs);

	if (root_dir_ == 0)
		return;

	if (parentWnd_)
		::PostMessage(parentWnd_, MESSAGE, files_.size(), DIR_SCAN_DONE);

	//TODO: save header record with catalog info
	//no of images, types scanned, directory structure, etc...
	std::vector<uint8> header;
	PrepareHeader(header, dir, title, description, root_dir_.get(), folder_scan);
	uint64 header_offset= dbImages_.Append(header);

	tags_.clear();

	failed_.clear();

	// now write images
	ProcessFiles(img_size, jpeg_compr);

	// write special record with statistics

	//TODO:

	std::vector<uint8> stats;
	stats.reserve(200);
	MemPointer p;
//	p.SetResizeCallback(boost::bind(&vector<uint8>::resize, &stats, _1));
	p.SetResizeCallback(boost::bind(&ResizeBuffer, &stats, _1, &p));
	p.SetByteOrder(false);	// little endian
	p.PutUInt32('tats');	// stat
	p.PutUInt32(static_cast<uint32>(tags_.size()));
	for (TagMap::const_iterator it= tags_.begin(); it != tags_.end(); ++it)
	{
		p.WriteWString(it->first);

		const size_t count= it->second.size();
		p.PutUInt32(static_cast<uint32>(count));
		for (size_t i= 0; i < count; ++i)
			p.PutUInt64(it->second[i]);
	}
	p.PutUInt32(0);
	p.PutUInt32(0);
	p.PutUInt32(0);
	p.PutUInt32(0);

	uint64 stats_offset_= dbImages_.Append(stats);

	if (parentWnd_)
		::PostMessage(parentWnd_, MESSAGE, 0, IMG_PROC_DONE);

	// update total amount of images in the catalog
	header_.img_count_ = static_cast<uint32>(counter_);
	header_.statistics_record_offset_ = stats_offset_;
	header_.Write(header);
	dbImages_.Update(header_offset, header);

}


void CatalogImages::Impl::ProcessFiles(int img_size, int jpeg_compression_level)
{
	const size_t count= files_.size();

	counter_ = 0;

	for (size_t i= 0; i < count; ++i)
	{
		if (parentWnd_)
			::PostMessage(parentWnd_, MESSAGE, i, IMG_PROCESSING);

		if (break_)
			return;

		const Path& path= files_[i].first;

		PhotoFactory::CreateFn fn= 0;
		int id= 0;

		if (!GetPhotoFactory().MatchPhotoType(path.GetExtension(), fn, id))
		{
			ASSERT(false);
			continue;
		}

		SmartPhotoPtr photo(fn());

		try
		{
			CatalogImgRecord img;

//			ExifBlock exif;
//			exif.clear();

			bool has_exif= photo->Scan(path.c_str(), img.exif_, false, nullptr);	// scan image to find EXIF data

			if (!has_exif && readOnlyPhotosWithEXIF_)
				continue;

			if (!has_exif || photo->GetDateTime().is_not_a_date_time())
				ReadFileTimeStamp(*photo, path);

			uint64 fileLength= 0;
			// write to db

			img.type_ = id;

			// get file length and last write time
			{
				WIN32_FIND_DATA findFileData;
				HANDLE find= ::FindFirstFile(path.c_str(), &findFileData);
				if (find != INVALID_HANDLE_VALUE)
				{
					VERIFY(::FindClose(find));
					fileLength = uint64(findFileData.nFileSizeLow) + (uint64(findFileData.nFileSizeHigh) << 32);

					img.time_stamp_ = findFileData.ftLastWriteTime;
					img.access_time_ = findFileData.ftLastAccessTime;
					img.creation_time_ = findFileData.ftCreationTime;
					img.file_attribs_ = findFileData.dwFileAttributes;
				}
			}
//			uint64 fileLength= path.GetFileLength();

			//photo->dir_visited_ = static_cast<uint32>(dir_visited);
			photo->SetFileSize(fileLength);
			photo->SetPhotoName(path.GetFileName());
			photo->SetPhysicalPath(path);
			photo->exif_data_present_ = has_exif;

			//			SetAutoRotationFlag(photo.get());

			img.path_ = String2WStr(path);
			img.make_ = String2WStr(photo->GetMake());
			img.model_ = String2WStr(photo->GetModel());
			img.orientation_ = photo->OrientationField();

			img.file_size_ = fileLength;
//			find.GetLastWriteTime(&img.time_stamp_);
			//photo->index_.Serialize(img.buf_index_);

	// verify this: ===========
			//		img.jpeg_offset_ = photo->jpeg_offset_;
			// HACK: this is orientation extracted from CRW file; when reading info from
			// cache CRW is not scanned, so orientation cannot be set
			img.exif_orientation_ = has_exif ? 0u : photo->OrientationField();
	// =========================

			// EXIF block
			img.has_exif_ = has_exif;
//			img.exif_ifd_offset_ = static_cast<int32>(exif.ifd0Start);
//			if (exif.is_raw)
//				img.exif_type_ = exif.bigEndianByteOrder ?
//					CatalogImgRecord::RAW_MM_EXIF_BLOCK : CatalogImgRecord::RAW_II_EXIF_BLOCK;
//			else
//				img.exif_type_ = CatalogImgRecord::JPEG_EXIF_BLOCK;
//			img.exif_.swap(exif.exif_buffer);

			// prepare preview
			Dib bmp;
			CSize thumbnail_size(img_size, img_size);
			CImageDecoderPtr decoder= photo->GetDecoder();
			bool ycbcr_image= true;
			ImageStat stat= decoder->DecodeImgToYCbCr(bmp, thumbnail_size, true);
			if (stat != IS_OK)
			{
				ycbcr_image = false;

				if (stat == IS_OPERATION_NOT_SUPPORTED)
					stat = decoder->DecodeImg(bmp, thumbnail_size, true);
			}

			if (stat != IS_OK || !bmp.IsValid())
			{
				failed_.push_back(path);
				continue;
			}

			// if EXIF indicates rotation and photo is already physically rotated AND exif reset EXIF orientation
			// flag back to normal to avoid problems later on...
			PhotoInfo::ImgOrientation orient= photo->GetOrientation();
			if ((orient == PhotoInfo::ORIENT_90CCW || orient == PhotoInfo::ORIENT_90CW) &&
				photo->GetWidth() > photo->GetHeight() && bmp.GetWidth() < bmp.GetHeight())
			{
				//photo->orientation_ = 0;
				std::swap(img.img_width_, img.img_height_);
				uint32 w= photo->GetWidth();
				uint32 h= photo->GetHeight();
				photo->SetSize(h, w);
			}
			else if ((orient == PhotoInfo::ORIENT_NORMAL || orient == PhotoInfo::ORIENT_NO_INFO ||
				orient == PhotoInfo::ORIENT_UNKNOWN) &&
				photo->GetWidth() > photo->GetHeight() && bmp.GetWidth() < bmp.GetHeight())
			{
				// orientation is 'normal', but reported size and size of decoded image do not agree
				uint32 w= photo->GetWidth();
				uint32 h= photo->GetHeight();
				photo->SetSize(h, w);
			}

			img.photo_width_ = photo->GetWidth();
			img.photo_height_ = photo->GetHeight();

			//TODO: convert non-ycbcr from rgb if needed

			// calculate 'index' using YCbCr image
			photo->index_.CalcHistogram(bmp);
			photo->index_.Serialize(img.index_);

			// now to RGB
			if (ycbcr_image)
				bmp.ConvertYCbCr2RGB();

			Dib thm;	// little thumbnail
			if (img_size > 160)
				bmp.ResizeToFit(CSize(160, 160), Dib::RESIZE_HALFTONE, thm);

			CSize size= bmp.GetSize();
			int big= img_size;
			if (size.cx > big || size.cy > big)
				bmp.ResizeToFit(CSize(big, big), Dib::RESIZE_HALFTONE);

			img.img_width_ = bmp.GetWidth();
			img.img_height_ = bmp.GetHeight();

			// compress preview into JPEG
			JPEGEncoder enc(jpeg_compression_level, false, false);

			if (thm.IsValid())
			{
				// we have both little thumbnail image and preview image
				{
					CMemoryDataDestination memdest;
					enc.Encode(memdest, &thm);
					memdest.SwapJPEG(img.thumbnail_);
				}
				{
					CMemoryDataDestination memdest;
					enc.Encode(memdest, &bmp);
					memdest.SwapJPEG(img.preview_);
				}
			}
			else
			{
				// only small preview available;
				// store it in the thumbnail leaving preview field empty

				CMemoryDataDestination memdest;
				enc.Encode(memdest, &bmp);
				memdest.SwapJPEG(img.thumbnail_);

				img.preview_.clear();
			}

			// IPTC present?
			//if (photo->IPTC_.get())
			//	img.iptc_.reset(photo->IPTC_.release());
			if (photo->HasMetadata())
			{
				img.iptc_.reset(new IPTCRecord());
				photo->GetIPTCInfo(*img.iptc_);
			}

			img.description_ = photo->GetExifDescription();
			img.marker_index_ = photo->GetFileTypeIndex();

			//TODO: store XMP too!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//TODO:
//	grab tags;
//	and description;
			//			EliminateNonPrintableChars(photo->photo_desc_);
			//			PostProcessTime(photo.get());

			CatalogHeader::DirPtr dir= files_[i].second;
			img.dir_visited_ = dir->id_;

			// add/modify record
			std::vector<uint8> buf;
			img.Serialize(buf, 1);
			uint64 record_offset= dbImages_.Append(buf);

			// examine tags
			if (!photo->GetTags().empty())	// has tags?
			{
				PhotoTags::const_iterator end= photo->GetTags().end();

				for (PhotoTags::const_iterator it = photo->GetTags().begin(); it != end; ++it)
					tags_[*it].push_back(record_offset);
			}

			if (dir->records_.empty())
				dir->records_.reserve(dir->reserved_capacity_);

			if (dir->records_.size() < dir->reserved_capacity_)
				dir->records_.push_back(record_offset);
			else
			{
				ASSERT(false);
				continue;
			}

			++counter_;
		}
		catch (MemPointer::MemPtrException&)
		{
			CString msg= _T("解析文件出错: ");
			msg += path.c_str();
			::ShowMessageBox(msg);
		}
		catch (CMemoryException*)
		{
			CString msg= _T("读取文件时内存溢出: ");
			msg += path.c_str();
			::ShowMessageBox(msg);
		}
		catch (JPEGException& ex)
		{
			CString msg= _T("处理文件出错: ");
			msg += path.c_str();
			msg += _T("\n\n");
			msg += ex.GetMessage();
			::ShowMessageBox(msg);
		}
#ifndef _DEBUG
		catch (...)
		{
//			ASSERT(false);
		}
#endif

	}
}


String CatalogImages::GetFile(size_t index) const
{
	if (index < pImpl_->files_.size())
		return pImpl_->files_[index].first;

	ASSERT(false);
	return _T("");
}


void CatalogImages::Impl::EnteringDir(const Path& path, int id)
{
//TRACE(L"===ENTERING: (%d) %s\n", id, path.c_str());
	bool root= id == 1;

	// for a root use absolute path, for remaining folders use relative paths
	String name= root ? path.GetDir() : Path(path.GetDir()).GetFileNameAndExt();

	DWORD attribs= ::GetFileAttributes(path.GetDir().c_str());

	CatalogHeader::DirPtr dir= new CatalogHeader::Dir(String2WStr(name).c_str(), id, attribs);

	if (cur_dir_)
	{
		dir_stack_.push_back(cur_dir_);
		cur_dir_->subdirs_.push_back(dir);
	}

	cur_dir_ = dir;

	if (root)
		root_dir_ = cur_dir_;
}


void CatalogImages::Impl::ExitingDir(int id)
{
//TRACE(L"===EXITING: (%d)  ", id);
	if (!dir_stack_.empty())
	{
		cur_dir_ = dir_stack_.back();
		dir_stack_.pop_back();
	}
	else
	{
//TRACE(L"===EXITING: -empty-", id);
	}
//if (cur_dir_)
//	TRACE(L" cur dir: (%d) %s", cur_dir_->id_, cur_dir_->name_.c_str());
//TRACE(L"\n");
}


void CatalogImages::Break()
{
	pImpl_->break_ = true;
}
