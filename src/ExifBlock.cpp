/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ExifBlock.h"
#include "ExifTags.h"
#include "Data.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static bool SetField(MemPointer& mem, uint32 val)
{
	uint16 fmt= mem.GetUInt16();
	uint32 components= mem.GetUInt32();

	if (components != 1)
	{
		mem += 4;
		return false;
	}

	switch (fmt)
	{
	case Data::tUSHORT:
	case Data::tSSHORT:
		mem.PutUInt16(static_cast<uint16>(val));
		mem += 2;
		return true;

	case Data::tULONG:
	case Data::tSLONG:
		mem.PutUInt32(val);
		return true;

	default:
		mem += 4;
		return false;
	}
}


// modify size fields (if present); clear orientation field (if present)
void ExifBlock::ModifySizeFields(CSize img_size, bool clearOrientation)
{
	if (exif_buffer.empty() || exif_buffer.size() < offsetIfd0Entries)
		return;

	MemPointer mem(&exif_buffer.front(), exif_buffer.size());
	mem.SetByteOrder(bigEndianByteOrder);

	mem += offsetIfd0Entries;

	for (uint32 i= 0; i < ifd0Entries; ++i)
	{
		uint16 tag= mem.GetUInt16();
		if (tag == EXIF_IMG_WIDTH || tag == EXIF_IMG_HEIGHT)
		{
			SetField(mem, tag == EXIF_IMG_WIDTH ? img_size.cx : img_size.cy);
//			ModifySize(img_size, tag, mem);
		}
		else if (tag == EXIF_ORIENTATION)
		{
			SetField(mem, 1);	// 'normal' orientation
		}
		else if (tag == EXIF_SUB_IFD)
		{
			uint16 fmt= mem.GetUInt16();
			uint32 components= mem.GetUInt32();
			uint32 offset= mem.GetUInt32();

			// read sub IFD
			ptrdiff_t temp= mem.GetPos();

			mem.SetPos(ifd0Start + offset);

			uint16 entries= mem.GetUInt16();	// no of entries in sub IFD
			for (uint32 i= 0; i < entries; ++i)
			{
				uint16 tag= mem.GetUInt16();
				if (tag == EXIF_IMG_WIDTH || tag == EXIF_IMG_HEIGHT)
					SetField(mem, tag == EXIF_IMG_WIDTH ? img_size.cx : img_size.cy);
//					ModifySize(img_size, tag, mem);
				else
					mem += 2 + 4 + 4;
			}

			mem.SetPos(temp);
		}
		else
			mem += 2 + 4 + 4;
	}
}
