/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransformationThread.cpp: implementation of the CTransformationThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransformationThread.h"
#include "JPEGEncoder.h"
#include "PhotoInfo.h"
#include <boost/bind.hpp>
#include "ExifBlock.h"
#include "FileDataDestination.h"
#include "ImageStat.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CTransformationThread::CTransformationThread(VectPhotoInfo& files,
		std::vector<Transformation>& transformation, const TransformationParams& params)
  : ImgProcessingThread(files.size()), files_(files), transformation_(transformation),
	encoder_(new JPEGEncoder(params.jpeg_quality_, params.progressive_, params.baseline_)), params_(params)
{
	ASSERT(!transformation_.empty());
	encoder_->SetProgressCallback(boost::bind(&CTransformationThread::LinesEncoded, this, _1, _2, _3));
}


void CTransformationThread::Process(size_t index)
{
	ASSERT(index < files_.size());
	ASSERT(files_[index] != 0);
	ASSERT(index < files_.size());

	PhotoInfo& photo= *files_[index];

	// output file
	Path output= GetDestFileName(index);

	if (transformation_.size() == 1 && transformation_.front().needs_input_file_)
	{
		transformation_.front().fn_transform_img_(photo.GetOriginalPath(), output);

		return;		// exclusive transformation done, exit
	}

	// start loading
	SetOperationLabel(IDS_IMG_TRANSFORM_LOADING);

	CImageDecoderPtr decoder= photo.GetDecoder();

	decoder->SetProgressCallback(boost::bind(&CTransformationThread::LinesDecoded, this, _1, _2, _3));

	CSize img_size(0, 0);
	Dib dib;
	if (ImageStat status= decoder->DecodeImg(dib, img_size, false))
		throw ImageStatMsg(status);

	SetOperationLabel(IDS_IMG_TRANSFORM_APPLYING);

	for (size_t i= 0; i < transformation_.size(); ++i)
	{
		ASSERT(!transformation_[i].needs_input_file_);

		transformation_[i].fn_transform_bmp_(dib);
	}

	// EXIF block
	std::vector<uint8> exif;
	if (params_.copy_exif_ && photo.IsExifDataPresent())
	{
		ExifBlock exifBlock;
		if (photo.ReadExifBlock(exifBlock))
		{
			exifBlock.ModifySizeFields(dib.GetSize(), true);
			exifBlock.GetExifMarkerBlock(exif);
		}
	}

	// start storing result
	SetOperationLabel(IDS_IMG_TRANSFORM_STORING);

	// destination file
	CFileDataDestination fdest(output.c_str());

	// write transformed photo
	if (!encoder_->Encode(fdest, &dib, &exif))
		throw _T("Error encoding JPEG file.");
}


String CTransformationThread::GetSourceFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetOriginalPath();
}


String CTransformationThread::GetDestFileName(size_t index) const
{
	ASSERT(index < files_.size());
	PhotoInfo& photo= *files_[index];
	Path output;

	if (params_.use_src_path_ == TransformationParams::USE_SRC_PATH)
	{
		output = photo.GetOriginalPath();
		if (!params_.suffix_.empty())
			output.AppendToFileName(params_.suffix_.c_str());
		if (!params_.dest_file_extension_.empty())
			output.ReplaceExtension(params_.dest_file_extension_.c_str());
	}
	else if (params_.use_src_path_ == TransformationParams::USE_DEST_PATH)
	{
		output = params_.dest_path_;
		Path name= photo.GetOriginalPath().GetFileNameAndExt();
		if (!params_.suffix_.empty())
			name.AppendToFileName(params_.suffix_.c_str());
		if (!params_.dest_file_extension_.empty())
			name.ReplaceExtension(params_.dest_file_extension_.c_str());
		output.AppendDir(name.c_str(), false);
	}
	else
	{
		ASSERT(params_.use_src_path_ == TransformationParams::USE_GIVEN_FILENAME);
		output = params_.dest_file_;
	}

	ASSERT(!output.empty());

	return output;
}


ImgProcessingThread* CTransformationThread::Clone()
{
	return new CTransformationThread(files_, transformation_, params_);
}
