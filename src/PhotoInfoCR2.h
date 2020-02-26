/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfo_XMP.h"


class PhotoInfoCR2 : public PhotoInfo_XMP
{
public:
	PhotoInfoCR2();
	virtual ~PhotoInfoCR2();

	virtual bool IsRotationFeasible() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual CImageDecoderPtr GetDecoder() const;

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;

	virtual bool ReadExifBlock(ExifBlock& exif) const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;

	virtual bool GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const;

private:
	PhotoInfoCR2& operator = (const PhotoInfoCR2&);
	PhotoInfoCR2(const PhotoInfoCR2&);

	uint32 ParseIFD(FileStream& ifs, uint32 base, bool main_image);
//	void ParseExif(FileStream& ifs, uint32 base);
//	void ParseMakerNote(FileStream& ifs);

	OutputStr output_;
	uint32 jpeg_data_offset_;
	uint32 jpeg_data_size_;
	uint32 jpeg_thm_data_offset_;
};
