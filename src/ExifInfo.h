/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// struct used to record EXIF block's location, size and other relevant params

struct ExifInfo
{
	ExifInfo()
	{
		big_endian_byte_order = false;
		exif_offset = 0;
		exif_block_size = 0;
		offset_to_Ifd_start = 0;
		offset_to_Ifd0_entries = 0;
	}

	bool big_endian_byte_order;		// motorola (true) vs intel (false)
	Offset exif_offset;				// physical offset from the beggning of file to the place where EXIF
									// block starts (that is, it's 2-byte entries count)
	//Offset offset_to_exif_marker;	// app2 marker's position in a file (JPEG mainly)
	uint32 exif_block_size;			// size including app marker (2 bytes) and size field (2 bytes)
	Offset offset_to_Ifd_start;		// confusing offset that some long entries use to locate their data; often 0 for raw images
	Offset offset_to_Ifd0_entries;	// not used?
};
