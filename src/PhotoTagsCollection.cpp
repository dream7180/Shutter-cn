/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoTagsCollection.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoTagsCollection.h"
#include "PhotoTags.h"
#include "File.h"
#include <boost/algorithm/string/trim.hpp>
using namespace boost::algorithm;
#include "set.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

typedef ::set<String> Set;


struct PhotoTagsCollection::Impl
{
	// tags in a set collection:
	//   kept in a vector in the same order user defined them; strings are
	//   also duplicated in a hashed set where they are stored for quick access
	Set set_;
	OnChanged on_change_;
	bool modified_;

	Impl()
	{
		modified_ = false;
	}

	void notify_on_change();

	bool add_tag(const String& tag)
	{
		return set_.push_back(tag);
	}

	void assign(const Impl* impl)
	{
		set_.clear();

		const size_t count= impl->set_.size();
		for (size_t i= 0; i < count; ++i)
			add_tag(impl->set_[i]);
	}
};


PhotoTagsCollection::PhotoTagsCollection() : impl_(new Impl())
{
	impl_->set_.reserve(20);
}

PhotoTagsCollection::PhotoTagsCollection(const PhotoTagsCollection& src) : impl_(new Impl())
{
	impl_->assign(src.impl_.get());
}


slot_connection PhotoTagsCollection::ConnectOnChange(OnChanged::slot_function_type fn) const
{
	return impl_->on_change_.connect(fn);
}


PhotoTagsCollection::~PhotoTagsCollection()
{}


bool PhotoTagsCollection::HasTag(const String& tag) const
{
	return impl_->set_[tag];
}

bool PhotoTagsCollection::operator [] (const String& tag) const
{
	return HasTag(tag);
}

const String& PhotoTagsCollection::Get(size_t index) const
{
	ASSERT(index < impl_->set_.size());
	return impl_->set_[index];
}

const String& PhotoTagsCollection::operator [] (size_t index) const
{
	return Get(index);
}


size_t PhotoTagsCollection::GetCount() const
{
	return impl_->set_.size();
}

size_t PhotoTagsCollection::size() const
{
	return impl_->set_.size();
}


bool PhotoTagsCollection::Empty() const
{
	return impl_->set_.empty();
}


void PhotoTagsCollection::Clear()
{
	bool changed= !Empty();
	impl_->set_.clear();
	if (changed)
	{
		impl_->modified_ = true;
		impl_->notify_on_change();
	}
}


void PhotoTagsCollection::AddTag(const String& tag)
{
	if (impl_->add_tag(tag))
	{
		impl_->modified_ = true;
		impl_->notify_on_change();
	}
}


void PhotoTagsCollection::OpenTagGroup(const String& name)
{
	impl_->set_.open_new_group(name);
}


void PhotoTagsCollection::CloseTagGroup()
{
	impl_->set_.close_group();
}


struct AccStringLength
{
	size_t operator () (size_t size, const String& str) const
	{
		return size + str.length() + 1;	// name + separator
	}

	size_t operator () (size_t size, const Set::group& g) const
	{
		return size + 1 + g.name().length() + 1;	// separator + name + separator
	}
};

//struct AccStrings
//{
//	String& operator () (String& accum, const String& str) const
//	{
//		accum += str;
//		accum += _T('\n');
//		return accum;
//	}
//};


String PhotoTagsCollection::AsString() const
{
	size_t size= 0;
	size = accumulate(impl_->set_.begin(), impl_->set_.end(), size, AccStringLength());
	size = accumulate(impl_->set_.begin_group(), impl_->set_.end_group(), size, AccStringLength());

	String tags;
	tags.reserve(size);	// space for all strings and their separators

	Set::group_iterator end= impl_->set_.end_group();
	for (Set::group_iterator it= impl_->set_.begin_group(); it != end; ++it)
	{
		if (!it->name().empty())
			tags += L'\n' + it->name() + L'\n';

		for (size_t i= it->begin(); i != it->end(); ++i)
			tags += impl_->set_[i] + L'\n';
	}

	return tags;
}


void PhotoTagsCollection::FromString(const String& tags)
{
	impl_->set_.clear();

	iStringstream si(tags);

	String tag;
	bool group_name= false;

	for (;;)
	{
		skipws(si);

		if (!getline(si, tag, _T('\n')))
			break;
//TODO: this is not quite right
		// remove trailing white chars and other junk (less then or equal to ' ')
//		tag.erase(find_if(tag.rbegin(), tag.rend(), bind2nd(greater<TCHAR>(), _T(' '))).base(), tag.end());
		trim_if(tag, is_from_range(_T('\0'), _T('\x20')));

		//TODO: limit string length to something reasonable?

		if (tag.empty())	// empty line signals new group of tags
		{
			impl_->set_.close_group();
			group_name = true;
			continue;
		}

		if (group_name)		// new group of tags?
		{
			impl_->set_.open_new_group(tag);
			group_name = false;
		}
		else
			impl_->set_.push_back(tag);
	}

	impl_->modified_ = true;
	impl_->notify_on_change();
}


int PhotoTagsCollection::GetTagId(const String& tag) const
{
	Set::iterator it= find(impl_->set_.begin(), impl_->set_.end(), tag);
	if (it == impl_->set_.end())
	{
		ASSERT(false);
		return 0;
	}
	return static_cast<int>(distance(impl_->set_.begin(), it));
}


bool PhotoTagsCollection::operator == (const PhotoTagsCollection& col)
{
	return impl_->set_ == col.impl_->set_;
}


void PhotoTagsCollection::Impl::notify_on_change()
{
	if (!on_change_.empty())
	{
		try
		{
			on_change_();
		}
		catch (...)
		{
			ASSERT(false);
		}
	}
}


void PhotoTagsCollection::AppendTags(const PhotoTags& tags)
{
	bool changed= false;
	for (PhotoTags::const_iterator it= tags.begin(); it != tags.end(); ++it)
		if (impl_->add_tag(*it))
			changed = true;

	if (changed)
	{
		impl_->modified_ = true;
		impl_->notify_on_change();
	}
}


void PhotoTagsCollection::assign(const PhotoTagsCollection& src)
{
	if (*this == src)
		return;

	impl_->assign(src.impl_.get());

	impl_->modified_ = true;
	impl_->notify_on_change();
}


void PhotoTagsCollection::SetModified(bool modified/*= true*/)
{
	impl_->modified_ = modified;
}


bool PhotoTagsCollection::IsModified() const
{
	return impl_->modified_;
}


size_t PhotoTagsCollection::GroupCount() const
{
	return impl_->set_.end_group() - impl_->set_.begin_group();
}


const String PhotoTagsCollection::GroupName(size_t index) const
{
	return impl_->set_.get_group(index).name();
}


std::pair<size_t, size_t> PhotoTagsCollection::GroupSpan(size_t index) const
{
	return impl_->set_.get_group(index).span();
}
