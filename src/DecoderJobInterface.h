/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once;
class Dib;
#include "PhotoInfoPtr.h"
typedef boost::shared_ptr<Dib> DibPtr;


class DecoderJobInterface
{
public:
	virtual void Quit() = 0;

	virtual DibPtr GetBitmap() const = 0;

//	virtual Dib* Bitmap() const = 0;

	virtual int ReductionFactor() const = 0;

	virtual ConstPhotoInfoPtr GetPhoto() const = 0;

	virtual CSize GetOriginalSize() const = 0;

	virtual CSize GetRequestedSize() const = 0;

	virtual UINT_PTR GetUniqueId() const = 0;

	virtual ~DecoderJobInterface()
	{}
};
