/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000-2011 Michael Kowalski
____________________________________________________________________________*/

// DecoderJob.h: interface for the DecoderJob class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DECODERJOB_H__76D51CFE_3859_46F6_8C3C_ED6DB72254B8__INCLUDED_)
#define AFX_DECODERJOB_H__76D51CFE_3859_46F6_8C3C_ED6DB72254B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ImageDecoder.h"
#include "AutoPtr.h"
#include "Dib.h"
#include "PhotoInfo.h"
#include "PhotoInfoPtr.h"
#include "ICMProfile.h"
#include "DecoderJobInterface.h"


class DecoderJob : public DecoderJobInterface, public mik::counter_base, boost::noncopyable
{
public:
	DecoderJob(const PhotoInfo& photo, CSize photo_size, bool auto_rotate,
		double scale_requested, CSize requested_size, CWnd* display_wnd, unsigned int orientation);

	void Restart(CSize requested_size);

	virtual void Quit();

	// is decoding operation still pending?
	bool Pending() const;

	// return bitmap
	virtual DibPtr GetBitmap() const;

	// provide access to decoded photo
	//virtual Dib* Bitmap() const;

	const String& FileName() const;

	// +/-1 if bitmap is rotated CCW/CW 90 deg.
	int Rotated() const;
	bool AutoRotation() const;

	// return no of photo scan lines decoded so far
	int LinesReady() const;

	bool GammaEnabled() const;

	// current ICM transformation
	const ICMTransform& GetICMTransform() const;

	// how many times img was reduced
	virtual int ReductionFactor() const;

	// orientation
	PhotoInfo::ImgOrientation GetOrientation() const;

	CSize GetExifImgSize() const;

	virtual CSize GetOriginalSize() const;

	virtual CSize GetRequestedSize() const;

	virtual ConstPhotoInfoPtr GetPhoto() const;

	virtual UINT_PTR GetUniqueId() const;

	enum { LINES_PER_STEP= 143 };

	ImageStat GetDecodingStatus() const;

	virtual ~DecoderJob();

private:
	struct Impl;
	boost::scoped_ptr<Impl> impl_;
};


DibPtr GetDecoderBitmap(DecoderJob* decoder);

UINT_PTR GetDecoderUniqueId(DecoderJob* decoder);


#endif // !defined(AFX_DECODERJOB_H__76D51CFE_3859_46F6_8C3C_ED6DB72254B8__INCLUDED_)
