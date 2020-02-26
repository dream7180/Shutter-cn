/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

class Path;
class MemPointer;
struct XmpData;
class MemMappedFile;


namespace Xmp
{
	// one include is needed for XMP_Error exception spec; consider not leaking those exceptions
#define WIN_ENV	1
#include "XMP-Toolkit\public\include\xmp_const.h"
#undef WIN_ENV

// this function updates existing XMP file with new tags and rating or creates a new one if no XMP exists
//extern void SavePhotoTags(Path photoPath, const vector<String>* tags, int rating, const String& creatorTool);

	// not used
//extern void LoadPhotoTags(Path photoPath, vector<String>& tags);

extern void SaveXmpIntoJpeg(const XmpData& xmp, const TCHAR* jpegFile);

// low-level saving routine; make a backup copy of the original before calling it
extern void SaveXmpIntoJpeg(const XmpData& xmp, MemMappedFile& jpeg_file, const TCHAR* file_name);

extern bool CheckIfXmpData(MemPointer& data, uint16 dataSize);

// check to see if XMP file accompanying image can be edited
extern bool CanEditXmpFile(Path photoPath, int& err_code);

// save XMP data into stand-alone XML file (overwrites existing file!)
extern void SaveToXmpFile(const XmpData& xmp, const TCHAR* xmpFile);

// save XMP data into stand-alone XML file (updates existing file)
extern void UpdateXmpFile(const XmpData& xmp, const TCHAR* xmpFile);

// load XMP file into 'XmpData' struct
extern void LoadFromXmpFile(XmpData& xmp, const TCHAR* xmpFile);

extern const char* HEADER_NAMESPACE;
extern const size_t HEADER_NAMESPACE_LEN;

std::string XmpDataToMeta(const XmpData& xmp, bool insert_namespace_header, bool insert_packet_wrapper, size_t ideal_size, size_t size_limit);

void MetaToXmpData(const std::vector<char>& xmp_buf, XmpData& xmp);

void ParseItems(const String& items, std::vector<String>& keys);

// read XMP file directly into xmp struct
bool ReadXmpFile(const TCHAR* xmpFile, XmpData& xmp);

// strip out surrounding xpacket bracket; returns tru if magic key found ("W5M0MpCehiHzreSzNTczkc9d")
bool RemoveXPacket(std::vector<char>& xmp_buf);

// when saving XmpData it may need to be merged with existing meta data
std::string MergeXmpDataToMeta(const XmpData& xmp, std::vector<char>& existing_xmp, bool insert_namespace_header, bool insert_packet_wrapper, size_t ideal_size, size_t size_limit);

}
