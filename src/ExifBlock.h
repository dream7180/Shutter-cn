/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "MemPointer.h"


struct ExifBlock
{
	ExifBlock()
	{}

	void GetExifMarkerBlock(std::vector<uint8>& block)
	{
		block.clear();
		if (offsetExifMarkerBlock + exifBlockSize <= exif_buffer.size())
			block.assign(exif_buffer.begin() + offsetExifMarkerBlock, exif_buffer.begin() + offsetExifMarkerBlock + exifBlockSize);
	}

	void clear()
	{
		bigEndianByteOrder = false;
		exif_buffer.clear();
		offsetExifMarkerBlock = 0;
		exifBlockSize = 0;
		ifd0Start = 0;
		ifd0Entries = 0;
		offsetIfd0Entries = 0;
		ifd1Start = 0;
		is_raw = false;
	}

	bool empty() const
	{
		return exif_buffer.empty();
	}

	// modify size fields (if present)
	void ModifySizeFields(CSize img_size, bool clearOrientation);

	bool bigEndianByteOrder;
	uint32 offsetExifMarkerBlock;
	uint32 exifBlockSize;			// size including app marker (2 bytes) and size field (2 bytes)
	uint32 ifd0Start;
	uint16 ifd0Entries;
	uint32 offsetIfd0Entries;
	uint32 ifd1Start;
	std::vector<uint8> exif_buffer;
	bool is_raw;
};
