/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGDataDestination.h: interface for the JPEGDataDestination class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEGDATADESTINATION_H__AD6323E2_EF5F_4CD0_9A51_803DBD010A31__INCLUDED_)
#define AFX_JPEGDATADESTINATION_H__AD6323E2_EF5F_4CD0_9A51_803DBD010A31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "jpeglib.h"


class JPEGDataDestination : public jpeg_destination_mgr
{
public:
	JPEGDataDestination();
	virtual ~JPEGDataDestination();

	virtual void InitDestination();
	virtual bool EmptyOutputBuffer()= 0;
	virtual void TermDestination();
	virtual void Abort();

	void Output(const std::vector<uint8>& v)
	{
		if (v.empty())
			return;

		const uint8* data= &v.front();
		size_t count= v.size();
		while (count > 0)
		{
			size_t write= std::min(count, free_in_buffer);

			memcpy(next_output_byte, data, write);
			next_output_byte += write;
			data += write;

			free_in_buffer -= write;
			count -= write;

			if (free_in_buffer == 0)
				EmptyOutputBuffer();
		}
	}

private:
	static void _InitDestination(j_compress_ptr cinfo);
	static boolean _EmptyOutputBuffer(j_compress_ptr cinfo);
	static void _TermDestination(j_compress_ptr cinfo);
};

#endif // !defined(AFX_JPEGDATADESTINATION_H__AD6323E2_EF5F_4CD0_9A51_803DBD010A31__INCLUDED_)
