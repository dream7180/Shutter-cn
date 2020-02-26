/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfo_XMP.h"


class PhotoInfoNEF : public PhotoInfo_XMP
{
public:
	PhotoInfoNEF();
	virtual ~PhotoInfoNEF();

	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual CImageDecoderPtr GetDecoder() const;

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;

	virtual bool ReadExifBlock(ExifBlock& exif) const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;

	virtual bool GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const;

private:
	PhotoInfoNEF& operator = (const PhotoInfoNEF&);
	PhotoInfoNEF(const PhotoInfoNEF&);

	uint32 ParseIFD(FileStream& ifs, uint32 base);
	void ParseExif(FileStream& ifs, uint32 base);
	void ParseMakerNote(FileStream& ifs);

	OutputStr output_;
	uint32 jpeg_data_offset_;
	uint32 jpeg_data_size_;
	uint32 jpeg_thm_data_offset_;
	uint16 sub_img_orientation_;
};
