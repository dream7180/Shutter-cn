/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DecoderJobInterface.h"
#include "boost/noncopyable.hpp"
#include "PhotoInfoPtr.h"


class ThumbnailDecoder : public DecoderJobInterface, boost::noncopyable
{
public:
	ThumbnailDecoder(PhotoInfo& photo, CSize size, CWnd* receiver);

	virtual ~ThumbnailDecoder();

	virtual void Quit();

	virtual DibPtr GetBitmap() const;

	virtual int ReductionFactor() const;

	virtual ConstPhotoInfoPtr GetPhoto() const;

	virtual CSize GetOriginalSize() const;

	virtual CSize GetRequestedSize() const;

	virtual UINT_PTR GetUniqueId() const;

private:
	struct Impl;
	std::auto_ptr<Impl> impl_;
};
