/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ResizingThread.cpp: implementation of the ResizingThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ResizingThread.h"
#include "SlideShowGenerator.h"
#include "JPEGEncoder.h"
#include "BmpFunc.h"
#include "ExifBlock.h"
#include "PhotoInfo.h"
#include "FileDataSource.h"
#include "FileDataDestination.h"
#include "Dib.h"
#include "MemoryDataDestination.h"
#include "IPTCReadWrite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ResizingThread::ResizingThread(std::vector<ResizePhotoInfo>& files, const ResizeFormat& fmt, CSlideShowGenerator* slide_show/*= 0*/)
	: ImgProcessingThread(files.size()),
	  files_(files), params_(fmt), slide_show_(slide_show),
	  encoder_(new JPEGEncoder(fmt.quality_, fmt.progressive_, fmt.baseline_))
{
	encoder_->SetProgressCallback(boost::bind(&ResizingThread::LinesEncoded, this, _1, _2, _3));
}


bool IsRatioSame(CSize size1, CSize size2, double epsilon)
{
	if (size1.cx <= 0 || size1.cy <= 0 ||
		size2.cx <= 0 || size2.cy <= 0)
		return false;	// too small to judge

	double r1= double(size1.cx) / size1.cy;
	double r2= double(size2.cx) / size2.cy;

	return fabs(r1 - r2) < epsilon;
}


void ResizingThread::Process(size_t index)
{
	ASSERT(index < files_.size());
	ResizePhotoInfo& img= files_[index];

	// resize photo; return size of resized image (and thumbnail);
	// they may differ from input params if rotation was applied

	const TCHAR* input_file= img.src_file_.c_str();
	const TCHAR* output_file= img.dest_file_.c_str();
	const TCHAR* thumb_output_file= img.dest_thumbnail_.c_str();
	CSize original_img_size= img.photo_size_;
	CSize& photo_size= img.photo_size_;
	CSize& thumb_size= img.thumbnail_size_;

	// start loading
	SetOperationLabel(IDS_RESIZE_LOADING);

	// temp photo
	Dib dib;

	bool rescaling= !!(params_.img_size_ != CSize(0, 0));	// size (0, 0) means no rescaling (use original photo)
	CSize image_size= params_.img_size_;
	if (!rescaling)
		image_size = params_.thumb_size_;

	ASSERT(image_size.cx != 0 && image_size.cy != 0);

	CImageDecoderPtr decoder= img.decoder_;

	// decoder will send progress info to LinesDecoded()
	decoder->SetProgressCallback(boost::bind(&ResizingThread::LinesDecoded, this, _1, _2, _3));

	// request high quality at the expense of time
	decoder->SetQuality(ImageDecoder::SLOW_HIGH_QUALITY);

	// load and scale it down (coarse)
	if (ImageStat status= decoder->DecodeImg(dib, image_size, true))
		throw ImageStatMsg(status);

	if (StopProcessing())
		return;

	// resize to requested size (exact)
	if (dib.GetSize() != image_size)
	{
		if (image_size.cx < 0)		// negative value means percent
			image_size.cx = std::max(dib.GetWidth() * (-image_size.cx) / 100, 1L);
		if (image_size.cy < 0)
			image_size.cy = std::max(dib.GetHeight() * (-image_size.cy) / 100, 1L);

		dib.ResizeToFit(image_size, params_.method_);
	}

	if (rescaling)
		photo_size = dib.GetSize(); //image_size;
	else
		photo_size = decoder->GetOriginalSize();

	// EXIF block
	std::vector<uint8> exif;
	if (params_.preserve_exif_block_ && img.photo_ != 0 && img.photo_->IsExifDataPresent())
	{
		ExifBlock exifBlock;
		if (img.photo_->ReadExifBlock(exifBlock))
		{
			exifBlock.ModifySizeFields(photo_size, true);
			exifBlock.GetExifMarkerBlock(exif);
		}
	}

	bool rotated= false;

	// check orientation: 6 & 8 values come from EXIF orientation field;
	// check if it's not physically rotated already!
	// check ratio (rather than sizes directly; important for raw images for instance where
	// big preview image is smaller than reported raw image)
	if ((img.orientation_ == 6 || img.orientation_ == 8) &&
		dib.GetWidth() > dib.GetHeight() &&
		IsRatioSame(original_img_size, decoder->GetOriginalSize(), 0.1))
	{
		// rotate resized photo
		AutoPtr<Dib> copy= dib.RotateCopy(img.orientation_ == 6);
		if (copy.get() == 0)
		{
			String str= _T("Error rotating image file ");
			str += input_file;
			throw str;
		}
		if (rescaling)
			std::swap(photo_size.cx, photo_size.cy);
		dib.Swap(*copy);
		rotated = true;
	}

	// start storing result
	SetOperationLabel(IDS_RESIZE_STORING);

	if (params_.format_ == 0)	// JPEG? (destination format)
	{
		if (slide_show_)
		{
			// destination memory
			CMemoryDataDestination mem;
			// encode resized photo
			encoder_->Encode(mem, &dib, &exif);
			// add slide
			if (mem.GetJPEG() && mem.GetSize() > 0)
				slide_show_->AddJpegSlide(mem.GetJPEG(), mem.GetSize(), 0);
		}
		else
		{
			if (rescaling)
			{
				// destination file
				CFileDataDestination fdest(output_file);

				// write resized photo
				encoder_->Encode(fdest, &dib, &exif);

				// copy tags
				if (params_.copyTags_ && img.photo_ != 0 && !img.photo_->GetTags().empty())
				{
					// TODO: copy XMP data as well!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

					IPTCRecord iptc;
					img.photo_->GetIPTCInfo(iptc);

					img.photo_->GetTags().CopyTagsToArray(iptc.keywords_);
					iptc.LimitKeywordsLength();

					WriteIPTC(output_file, iptc);
				}
			}
			else
			{
				// no rescaling--for now just copy (ideally rotation if necessary)
				if (!::CopyFile(input_file, output_file, false))
				{
					String msg= _T("Error copying file.\n\nFrom: ");
					msg += input_file;
					msg += _T("\n\nTo:   ");
					msg += output_file;
					throw msg;
				}
			}

			if (thumb_output_file && *thumb_output_file)
			{
				thumb_size = params_.thumb_size_;
				if (rotated)
					std::swap(thumb_size.cx, thumb_size.cy);

				if (dib.GetSize() != thumb_size)
					dib.ResizeToFit(thumb_size, params_.method_);

				// destination file--thumbnail
				CFileDataDestination fdest(thumb_output_file);
				// write resized photo thumbnail (no EXIF in thumbnail)
				encoder_->Encode(fdest, &dib);

				thumb_size = dib.GetSize();
			}
		}
	}
	else
	{
		dib.Save(output_file);
	}
}


String ResizingThread::GetSourceFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index].src_file_;
}


String ResizingThread::GetDestFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index].dest_file_;
}


ResizingThread* ResizingThread::Clone()
{
	return new ResizingThread(files_, params_, slide_show_);
}
