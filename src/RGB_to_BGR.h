/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

extern void RGB_to_BGR(const BYTE* line, uint32 line_size, BYTE* out);
extern void RGB16_to_BGR(const BYTE* line, uint32 line_size, BYTE* out, bool little_endian);
