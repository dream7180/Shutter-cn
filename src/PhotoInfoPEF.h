/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfo_XMP.h"

// Pentax raw file (this is also base for a new Sony ARW file)

class PhotoInfoPEF : public PhotoInfo_XMP
{
public:
	PhotoInfoPEF();
	virtual ~PhotoInfoPEF();

	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual CImageDecoderPtr GetDecoder() const;

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;

	virtual bool ReadExifBlock(ExifBlock& exif) const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;

	virtual bool GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const;

private:
	PhotoInfoPEF& operator = (const PhotoInfoPEF&);
	PhotoInfoPEF(const PhotoInfoPEF&);

	uint32 ParseIFD(FileStream& ifs, uint32 base, bool main_image, bool thumbnail, bool big_jpeg);
	void ParseExif(FileStream& ifs, uint32 base);
	virtual void ParseMakerNote(FileStream& ifs);

	uint32 jpeg_data_offset_;
	uint32 jpeg_data_size_;
	uint32 jpeg_thm_data_offset_;
	//uint16 sub_img_orientation_;

	// meaning of the IFD directories present in the raw file
	virtual bool IsMainImage(int ifd_index);
	virtual bool IsThumbnailImage(int ifd_index);
	virtual bool IsBigImage(int ifd_index);

	String type_name_;

protected:
	OutputStr output_;

	PhotoInfoPEF(const TCHAR* type_name);
};
