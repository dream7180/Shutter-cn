/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfoPtr.h"


class PhotoFactory
{
public:
	typedef PhotoInfoPtr (*CreateFn)();

	// Scott Mayer's singleton: photo factory

	static PhotoFactory& GetPhotoFactory();

	PhotoInfoPtr CreatePhotoInfo(const String& ext) const;

	// return function that creates type matching given extension; return an id of this type too
	bool MatchPhotoType(const String& extensions, CreateFn& fn, int& id) const;

	// type is registered by providing function that creates it and its identifier
	void RegisterType(const String& extensions, CreateFn fn, int id);

	// return semicolon separated extensions for all registered types
	String GetRegisteredExtensions() const;

private:
	PhotoFactory();
	PhotoFactory(const PhotoFactory&);
	PhotoFactory& operator = (PhotoFactory&);
	~PhotoFactory();

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};


inline PhotoFactory& GetPhotoFactory()	{ return PhotoFactory::GetPhotoFactory(); }


// type registration helper

template <class T>
struct RegisterPhotoType
{
	RegisterPhotoType(const String& extensions, int id)
	{
		GetPhotoFactory().RegisterType(extensions, &RegisterPhotoType::Create, id);
	}

private:
	static PhotoInfoPtr Create()		{ return new T(); }
};
