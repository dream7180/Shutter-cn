/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoCollection.h
//
// This is central object for sending notifications regarding changes to the
// metadata in loaded photos (as well as related tags/rating).
//
// Views of photos subscribe to the data change events, and routine that modifies photos
// is supposed to fire appropriate event, so views can refresh themselves.
//
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Signals.h"
#include <boost/noncopyable.hpp>
#include "VectPhotoInfo.h"


class PhotoCollection : boost::noncopyable
{
public:
	static const PhotoCollection& Instance();

	void FireTagsChanged(PhotoInfoPtr photo) const;
	void FireTagsChanged(const VectPhotoInfo& photos) const;
	void FireMetadataChanged(PhotoInfoPtr photo) const;
	void FireMetadataChanged(const VectPhotoInfo& photos) const;
	void FireRatingChanged(PhotoInfoPtr photo) const;
	void FireRatingChanged(const VectPhotoInfo& photos) const;

	// events
	typedef boost::signals2::signal<void (const VectPhotoInfo&)> OnTagsChanged;
	typedef boost::signals2::signal<void (const VectPhotoInfo&)> OnMetadataChanged;
	typedef boost::signals2::signal<void (const VectPhotoInfo&)> OnRatingChanged;

	// connect handler to the tags/meta/rates changed event
	slot_connection ConnectOnTagsChanged(OnTagsChanged::slot_function_type fn) const;
	slot_connection ConnectOnMetadataChanged(OnTagsChanged::slot_function_type fn) const;
	slot_connection ConnectOnRatingChanged(OnTagsChanged::slot_function_type fn) const;

private:
	PhotoCollection();
	~PhotoCollection();

	struct Impl;
	std::auto_ptr<Impl> impl_;
};
