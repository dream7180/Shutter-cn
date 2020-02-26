/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoTagsCollection.h
//
// collection of tags; tags common to all the photographs to be collected here
//
// (strings are kept in a vector in the same order user defined them; strings are
//  also duplicated in a hashed set where they are stored for quick access)
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Signals.h"

class FileStream;
class PhotoTags;


class PhotoTagsCollection
{
public:
	PhotoTagsCollection();
	PhotoTagsCollection(const PhotoTagsCollection& src);
	~PhotoTagsCollection();

	void AddTag(const String& tag);

	void OpenTagGroup(const String& name);
	void CloseTagGroup();

	void AppendTags(const PhotoTags& tags);

	int GetTagId(const String& tag) const;

	// all tags '\n' separated
	String AsString() const;

	// reset current content using '\n' separated substrings from 'tags'
	void FromString(const String& tags);

	// compare elements
	bool operator == (const PhotoTagsCollection& col);

	const String& Get(size_t index) const;
	const String& operator [] (size_t index) const;

	size_t GetCount() const;
	size_t size() const;

	bool Empty() const;

	void Clear();

	bool HasTag(const String& tag) const;
	bool operator [] (const String& tag) const;

	void assign(const PhotoTagsCollection& src);

	// events
	typedef boost::signals2::signal<void ()> OnChanged;	// collection has been modified

	// connect handler to the "on change" event
	slot_connection ConnectOnChange(OnChanged::slot_function_type fn) const;

	// convenience methods to keep track of modifications
	void SetModified(bool modified= true);
	bool IsModified() const;

	// group support
	size_t GroupCount() const;
	const String GroupName(size_t index) const;
	std::pair<size_t, size_t> GroupSpan(size_t index) const;

private:
	PhotoTagsCollection& operator = (const PhotoTagsCollection&);

	struct Impl;
	std::auto_ptr<Impl> impl_;
};


void AppendTagsToCollection(PhotoTagsCollection& collection, const PhotoTags& tags);
