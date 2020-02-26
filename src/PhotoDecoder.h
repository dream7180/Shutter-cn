/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2002 Michal Kowalski
____________________________________________________________________________*/

// PhotoDecoder.h: interface for the CPhotoDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTODECODER_H__6C6A5D57_D526_4B36_958D_38EADE09559D__INCLUDED_)
#define AFX_PHOTODECODER_H__6C6A5D57_D526_4B36_958D_38EADE09559D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageDecoder.h"
class CDecoderJob;


class CPhotoDecoder : public CDecoderProgress
{
public:
	CPhotoDecoder(CDecoderJob* decoder_job); //CWnd* owner_wnd);
	virtual ~CPhotoDecoder();


	virtual bool LinesDecoded(int lines_ready, bool finished);

	void Break()	{ break_ = true; }

private:
//	CWnd* owner_wnd_;
	CImageDecoder* image_decoder;
	CDecoderJob* decoder_job_;
	bool break_;
};

#endif // !defined(AFX_PHOTODECODER_H__6C6A5D57_D526_4B36_958D_38EADE09559D__INCLUDED_)
