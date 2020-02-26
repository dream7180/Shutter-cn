/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGEncoder.h: interface for the JPEGEncoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEGENCODER_H__D771C533_F16E_4FD4_B32A_78F9FF2FEC87__INCLUDED_)
#define AFX_JPEGENCODER_H__D771C533_F16E_4FD4_B32A_78F9FF2FEC87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "jpeglib.h"
#include <boost/function.hpp>
class JPEGDataDestination;
class Dib;


class JPEGEncoder : public jpeg_compress_struct
{
public:
	JPEGEncoder(int quality= 90, bool progressive= true, bool baseline= true);
	virtual ~JPEGEncoder();

	bool Encode(JPEGDataDestination& dest, Dib* bmp, const std::vector<uint8>* app_markers= 0);

	virtual bool LinesEncoded(int lines_ready, bool finished);
	int ScanLines() const		{ return image_height; }

	typedef boost::function<bool (int lines_ready, int img_height, bool finished)> Callback;

	void SetProgressCallback(const Callback& fn)	{ fn_progress_ = fn; }

private:
	bool progressive_;
	bool baseline_;
	int quality_;
	struct jpeg_error_mgr jerr_;
	Callback fn_progress_;
};

#endif // !defined(AFX_JPEGENCODER_H__D771C533_F16E_4FD4_B32A_78F9FF2FEC87__INCLUDED_)
