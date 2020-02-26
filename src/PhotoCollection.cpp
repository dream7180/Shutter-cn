/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoCollection.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoInfo.h"
#include "PhotoCollection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct PhotoCollection::Impl
{
	OnTagsChanged on_tags_changed_;
	OnMetadataChanged on_meta_changed_;
	OnRatingChanged on_rating_changed_;

	Impl()
	{}

	template<class EV>
	void notify_on_changed(EV& event, const VectPhotoInfo& photos)
	{
		if (!event.empty())
		{
			try
			{
				event(photos);
			}
			catch (...)
			{
				ASSERT(false);
			}
		}
	}
};


PhotoCollection::PhotoCollection() : impl_(new Impl())
{}


slot_connection PhotoCollection::ConnectOnTagsChanged(OnTagsChanged::slot_function_type fn) const
{
	return impl_->on_tags_changed_.connect(fn);
}

slot_connection PhotoCollection::ConnectOnMetadataChanged(OnMetadataChanged::slot_function_type fn) const
{
	return impl_->on_meta_changed_.connect(fn);
}

slot_connection PhotoCollection::ConnectOnRatingChanged(OnRatingChanged::slot_function_type fn) const
{
	return impl_->on_rating_changed_.connect(fn);
}


PhotoCollection::~PhotoCollection()
{}


void PhotoCollection::FireTagsChanged(PhotoInfoPtr photo) const
{
	VectPhotoInfo v(1, photo);
	impl_->notify_on_changed(impl_->on_tags_changed_, v);
}

void PhotoCollection::FireTagsChanged(const VectPhotoInfo& photos) const
{
	impl_->notify_on_changed(impl_->on_tags_changed_, photos);
}


void PhotoCollection::FireMetadataChanged(PhotoInfoPtr photo) const
{
	VectPhotoInfo v(1, photo);
	impl_->notify_on_changed(impl_->on_meta_changed_, v);
}

void PhotoCollection::FireMetadataChanged(const VectPhotoInfo& photos) const
{
	impl_->notify_on_changed(impl_->on_meta_changed_, photos);
}


void PhotoCollection::FireRatingChanged(PhotoInfoPtr photo) const
{
	VectPhotoInfo v(1, photo);
	impl_->notify_on_changed(impl_->on_rating_changed_, v);
}

void PhotoCollection::FireRatingChanged(const VectPhotoInfo& photos) const
{
	impl_->notify_on_changed(impl_->on_rating_changed_, photos);
}


const PhotoCollection& PhotoCollection::Instance()
{
	const static PhotoCollection pc;
	return pc;
}
