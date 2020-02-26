/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
#include "PhotoInfoPtr.h"

struct ExifBlock;
class ImgLogger;

extern bool Scan(const TCHAR* file, PhotoInfoPtr info, OutputStr* output, ExifBlock* exif, ImgLogger* logger);

// callback for an EXIF field visitor
typedef boost::function<void (FileStream& ifs, uint16 exif_field, const Data& val)> FieldCallback;

extern bool Scan(const TCHAR* file, FileStream& ifs, PhotoInfoPtr info, OutputStr* output, ExifBlock* exif, bool decode_thumbnail, ImgLogger* logger, FieldCallback* field_callback= nullptr);

extern bool ScanMem(const TCHAR* file, const std::vector<uint8>& exif_data_buffer, PhotoInfoPtr info, OutputStr* output, ImgLogger* logger);

extern uint32 ReadEntry(FileStream& ifs, Offset ifd_start, String& make, String& model, PhotoInfoPtr info, OutputStr& output);

// scan EXIF block extracting info and storing it in info, exif, and file_info
// note: any and all info, exif, file_info, and field_callback may be null
extern bool ScanExif(const TCHAR* file, FileStream& ifs, Offset ifd_start, std::pair<uint32, uint32> rangeExif, String make, String model, PhotoInfoPtr info, ExifBlock* exif, OutputStr* file_info, bool decode_thumbnail, FieldCallback* field_callback= nullptr);

extern size_t CalcExifSize(FileStream& ifs);

extern AutoPtr<GPSData> ReadGPSInfo(FileStream& ifs, Offset ifd_start, Data& val, OutputStr* output);
