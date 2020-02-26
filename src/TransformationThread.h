/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransformationThread.h: interface for the CTransformationThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Path.h"
#include "ImgProcessingThread.h"
#include "Dib.h"
#include "VectPhotoInfo.h"
class CSlideShowGenerator;
class ImageDecoder;
class JPEGEncoder;
class ImgPage;


struct Transformation
{
	typedef boost::function<void (Dib& dib)> FnTransformBmp;
	typedef boost::function<void (Path& pathPhoto, Path& pathOutput)> FnTransformFile;

	Transformation(const FnTransformBmp& fn) : needs_input_file_(false), fn_transform_bmp_(fn)
	{}

	Transformation(const FnTransformFile& fn) : needs_input_file_(true), fn_transform_img_(fn)
	{}

	bool needs_input_file_;
	FnTransformBmp fn_transform_bmp_;
	FnTransformFile fn_transform_img_;
};

struct TransformationParams
{
	enum { USE_SRC_PATH, USE_DEST_PATH, USE_GIVEN_FILENAME } use_src_path_;
//	bool use_src_path_;		// use image's source folder to write transformed img
	String suffix_;			// suffix to append to the result file
	Path dest_path_;		// selected output folder if src one not used
	Path dest_file_;		// selected output file (for single file processing)
	String dest_file_extension_;	// new file extension for destination files (for instance NEF -> jpg); batch mode only
	bool copy_exif_;		// preserve EXIF
	bool baseline_;			// JPEG params...
	bool progressive_;
	int jpeg_quality_;
};


class CTransformationThread : public ImgProcessingThread
{
public:
	CTransformationThread(VectPhotoInfo& files, std::vector<Transformation>& transformation, const TransformationParams& params);

	virtual ImgProcessingThread* Clone();

private:
	// process one image
	virtual void Process(size_t index);

	virtual String GetSourceFileName(size_t index) const;
	virtual String GetDestFileName(size_t index) const;

	VectPhotoInfo& files_;
	CWnd* status_wnd_;
	std::auto_ptr<JPEGEncoder> encoder_;
	std::vector<Transformation>& transformation_;
	const TransformationParams& params_;
};
