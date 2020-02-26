/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoFactory.h"
#include "PhotoInfo.h"
#include <map>
#include <boost/algorithm/string/case_conv.hpp>

using namespace boost::algorithm;


PhotoFactory& PhotoFactory::GetPhotoFactory()
{
	static PhotoFactory factory;
	return factory;
}


typedef std::map<String, std::pair<PhotoFactory::CreateFn, int> > CallbackMap;


struct PhotoFactory::Impl
{
	CallbackMap callbacks_;
};


PhotoFactory::PhotoFactory() : pImpl_(new Impl)
{}

PhotoFactory::~PhotoFactory()
{}


PhotoInfoPtr PhotoFactory::CreatePhotoInfo(const String& ext) const
{
	CallbackMap::const_iterator it= pImpl_->callbacks_.find(to_lower_copy(ext));
	if (it == pImpl_->callbacks_.end())
		return 0;

	return it->second.first();
}


bool PhotoFactory::MatchPhotoType(const String& ext, CreateFn& fn, int& id) const
{
	CallbackMap::const_iterator it= pImpl_->callbacks_.find(to_lower_copy(ext));
	if (it == pImpl_->callbacks_.end())
		return false;

	fn = it->second.first;
	id = it->second.second;

	return true;
}


void PhotoFactory::RegisterType(const String& extensions, CreateFn fn, int id)
{
	// register function 'fn' for all semicolon separated extensions

	for (String::size_type start= 0; ; )
	{
		String::size_type idx= extensions.find(';', start);

		size_t len= idx == String::npos ? extensions.length() - start : idx - start;

		String ext= extensions.substr(start, len);

		to_lower(ext);

		ASSERT(pImpl_->callbacks_.find(ext) == pImpl_->callbacks_.end());

		pImpl_->callbacks_[ext] = std::make_pair(fn, id);

		if (idx == String::npos)
			break;

		start = idx + 1;
	}
}


String PhotoFactory::GetRegisteredExtensions() const
{
	oStringstream ost;
//	String ext;

	for (CallbackMap::const_iterator it= pImpl_->callbacks_.begin(); it != pImpl_->callbacks_.end(); ++it)
		ost << _T("*.") << it->first << _T(";");
//		ext += it->first + _T(";");

	return ost.str();
}
