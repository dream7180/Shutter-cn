/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class MemMappedFile;

extern const char* g_photoshop;
const uint32 g_photoshop_len= 14;

extern bool WriteIPTC(MemMappedFile& photo, const TCHAR* file_name, const IPTCRecord& iptc);

extern bool WriteIPTC(const TCHAR* file, const IPTCRecord& iptc);

//extern bool WriteIPTCWithBackup(const TCHAR* file, const IPTCRecord& iptc);
