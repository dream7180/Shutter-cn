/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfo.h"
#include <boost/shared_ptr.hpp>
struct CatalogImgRecord;
class Database;


class PhotoInfoCatalog : public PhotoInfo
{
public:
	PhotoInfoCatalog(const CatalogImgRecord& img, boost::shared_ptr<Database> db, uint64 record_offset);
	virtual ~PhotoInfoCatalog();

	virtual bool CanEditIPTC(int& err_code) const;
	virtual bool IsRotationFeasible() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual CImageDecoderPtr GetDecoder() const;

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;

	virtual bool ReadExifBlock(ExifBlock& exif) const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;

	virtual const Path& GetPhysicalPath() const;

private:
	PhotoInfoCatalog& operator = (const PhotoInfoCatalog&);
	PhotoInfoCatalog(const PhotoInfoCatalog&);

	uint32 ParseIFD(FileStream& ifs, uint32 base);
	void ParseExif(FileStream& ifs, uint32 base);

	boost::shared_ptr<Database> db_;
	uint64 record_offset_;

	int marker_index_;
};
