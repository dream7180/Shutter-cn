/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtractingThread - worker thread for extracting JPEG images from raw photos
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ExtractingThread.h"
#include "JPEGDecoder.h"
#include "ExifBlock.h"
#include "PhotoInfo.h"
#include "FileDataSource.h"
#include "FileDataDestination.h"
#include "Transform.h"
#include "Scan.h"
#include "ReadMarker.h"
#include "Markers.h"
#include "IPTCReadWrite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ExtractingThread::ExtractingThread(std::vector<ExtractJpegInfo>& files, const ExtractFormat& fmt)
	: ImgProcessingThread(files.size()), files_(files), params_(fmt)
{}


void ExtractingThread::Process(size_t index)
{
	ASSERT(index < files_.size());
	ExtractJpegInfo& info= files_[index];

	// extract JPEG

	// start loading
	SetOperationLabel(IDS_RESIZE_LOADING);

	if (StopProcessing())
		return;

	// EXIF block
	std::vector<uint8> exif;
	if (params_.preserve_exif_block_ && info.photo_ != 0 && info.photo_->IsExifDataPresent())
	{
		ExifBlock exifBlock;
		if (info.photo_->ReadExifBlock(exifBlock))
		{
//			exifBlock.ModifySizeFields(photo_size, true);
			exifBlock.GetExifMarkerBlock(exif);
		}
	}

	uint32 bytes_left= info.jpeg_data_size_;

	if (bytes_left == 0)
		throw String(_T("Cannot find valid embedded image in a source photo\n") + info.src_file_path_);

	// start storing result
	SetOperationLabel(IDS_RESIZE_STORING);

	// read raw source
	CFile fsrc(info.src_file_path_.c_str(), CFile::modeRead | CFile::shareDenyWrite);

	fsrc.Seek(info.jpeg_data_offset_, CFile::begin);

	Path temp_dest= info.dest_file_path_ + _T(".$%$");

	// create destination file
	CFile fdest(temp_dest.c_str(), CFile::modeWrite | CFile::modeCreate);

	// copy JPEG now

	uint32 block= std::min<uint32>(0x400, bytes_left);
	std::vector<BYTE> buffer(block, 0);

	while (bytes_left > 0)
	{
		if (bytes_left > block)
			bytes_left -= block;
		else
		{
			block = bytes_left;
			bytes_left = 0;
		}

		if (fsrc.Read(&buffer[0], block) != block)
			throw _T("error reading source file");

		fdest.Write(&buffer[0], block);
	}

	fdest.Close();

	if (StopProcessing())
		return;

	// copy tags?
	if (params_.copy_tags_ && info.photo_ != 0 && !info.photo_->GetTags().empty())
	{
		// TODO: copy XMP data as well!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		IPTCRecord iptc;
		info.photo_->GetIPTCInfo(iptc);

		info.photo_->GetTags().CopyTagsToArray(iptc.keywords_);
		iptc.LimitKeywordsLength();

		WriteIPTC(temp_dest.c_str(), iptc);
	}

	if (StopProcessing())
		return;

	// find out what's the JPEG image real size in pixels
	std::auto_ptr<JPEGDataSource> img_src(new CFileDataSource(temp_dest.c_str(), 0));
	CSize img_size= JPEGDecoder::DecodeJpegImageDimensions(*img_src);
	img_src.reset();

	if (img_size.cx > img_size.cy)	// image not altered? processing software could have rotated it
	{
		// auto-rotation
		RotationTransformation operation= AUTO_ROTATE;

		// determine necessary rotation
		if (info.photo_->GetOrientation() == PhotoInfo::ORIENT_90CW)
			operation = CONDITIONAL_ROTATE_90_DEG_COUNTERCW;
		else if (info.photo_->GetOrientation() == PhotoInfo::ORIENT_90CCW)
			operation = CONDITIONAL_ROTATE_90_DEG_CW;

		if (operation != AUTO_ROTATE)
		{
			int new_width= 0;
			int new_height= 0;
			uint16 thumbnail_orientation= 0;
			if (::Transform(operation, false, info.photo_->GetWidth(), info.photo_->GetHeight(), temp_dest.c_str(),
					0, &new_width, &new_height, &thumbnail_orientation, 0) == 0)
			{
				img_size.cx = new_width;
				img_size.cy = new_height;
			}
		}
	}
#if 0 // TODO
	// check if extracted JPEG happens to have its own EXIF block
	bool exif_block_present= false;
	{
		ExifBlock existing_exif;
		FileStream f;
		if (f.Open(temp_dest.c_str()))
		{
			bool ok= ::Scan(temp_dest.c_str(), f, 0, 0, &existing_exif, false);
			exif_block_present = existing_exif.exifBlockSize > 0;
		}
	}

	if (!exif_block_present && !exif.empty())
	{
		// embedded JPEG image does not have EXIF block yet
		// use EXIF copy from the raw file, but fix size & orientation

		MemMappedFile photo;
		if (!photo.CreateWriteView(temp_dest.c_str(), 0)) //exif.size()))
			throw L"Cannot open temp file for writing";

		MemPointer data(photo.GetBaseAddrChr(), photo.GetFileSize());

		if (ReadMarker(data) == MARK_SOI)		// must be JPEG image
		{
			int length= photo.GetFileSize() - data.GetPos();
			memmove(data.GetPtr() + exif.size(), data.GetPtr(), length);
			memcpy(data.GetPtr(), &exif[0], exif.size());
		}
	}
#endif
	// temp to final destination

	if (!::MoveFile(temp_dest.c_str(), info.dest_file_path_.c_str()))
	{
		// move failed, probably dest. file already exists
		// but delete temp file now
		::DeleteFile(temp_dest.c_str());
	}

}


String ExtractingThread::GetSourceFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index].src_file_path_;
}


String ExtractingThread::GetDestFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index].dest_file_path_;
}


ImgProcessingThread* ExtractingThread::Clone()
{
	return new ExtractingThread(files_, params_);
}
