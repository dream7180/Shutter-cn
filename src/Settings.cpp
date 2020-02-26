/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Settings.h"
#include <boost/variant.hpp>


namespace settings {

//typedef std::vector<BYTE> Blob;
typedef boost::variant<String, int, Blob> Variant;
typedef std::map<String, Variant> Map;


struct Settings::Impl
{
	Impl()
	{}

	Map store_;
};


Settings::Settings()
{
}

Settings::~Settings()
{}


void Settings::Store(const String& id, const String& v)
{
	impl_->store_[id] = v;
}

void Settings::Store(const String& id, int v)
{
	impl_->store_[id] = v;
}

void Settings::Store(const String& id, const Blob& v)
{
	impl_->store_[id] = v;
}


String Settings::RestoreString(const String& id, const TCHAR* default_val) const
{
	ASSERT(false);
	return String();
}

int Settings::RestoreInt(const String& id, int default_val) const
{
	ASSERT(false);
	return 0;
}

Blob Settings::RestoreBlob(const String& id) const
{
	ASSERT(false);
	return Blob();
}


void Settings::Remove(const String& id)
{
	impl_->store_.erase(id);
}


bool Settings::Exist(const String& id) const
{
	return impl_->store_.count(id) > 0;
}


void save(CFile& f, char c)
{
	f.Write(&c, sizeof(c));
}


void save(CFile& f, const String& s)
{
	uint32 len= static_cast<uint32>(s.length());
	f.Write(&len, sizeof(len));
	f.Write(s.data(), len * sizeof(TCHAR));
}


void save(CFile& f, int32 i)
{
	f.Write(&i, sizeof(i));
}


void save(CFile& f, const Blob& b)
{
	uint32 s= static_cast<uint32>(b.size());
	f.Write(&s, sizeof(s));
	if (s > 0)
		f.Write(&b.front(), s);
}


class saving_visitor : public boost::static_visitor<>
{
public:
	saving_visitor(CFile& f) : f_(f)
	{}

	void operator () (String& s) const
	{
		save(f_, 's');
		save(f_, s);
	}

	void operator () (int& i) const
	{
		save(f_, 'i');
		save(f_, i);
	}

	void operator () (Blob& v) const
	{
		save(f_, 'v');
		save(f_, v);
	}

private:
	CFile& f_;
};


String read_str(CFile& f)
{
	uint32 len= 0;
	if (f.Read(&len, sizeof(len)) != sizeof(len))
		return String();

	std::vector<TCHAR> buffer(len + 1, 0);
	f.Read(&buffer.front(), len * sizeof(TCHAR));

	return String(&buffer.front());
}


char read_char(CFile& f)
{
	char c= 0;
	f.Read(&c, 1);
	return c;
}


int32 read_int(CFile& f)
{
	int32 i= 0;
	f.Read(&i, sizeof(i));
	return i;
}


Blob read_blob(CFile& f)
{
	uint32 s= read_int(f);
	Blob b(s, 0);
	if (s > 0)
	{
		if (f.Read(&b.front(), s) != s)
			throw std::exception("");
	}
	return b;
}


static BYTE header[8]= { 0, 0xff, 0, 0, 'm', 'k', 's', sizeof(TCHAR) };

void Settings::SaveToFile(const TCHAR* filename)
{
	CFile f(filename, CFile::modeWrite | CFile::modeCreate);

	f.Write(&header, sizeof(header));

	for (Map::iterator it= impl_->store_.begin(); it != impl_->store_.end(); ++it)
	{
		// write key
		save(f, it->first);

		// write value
		boost::apply_visitor(saving_visitor(f), it->second);
	}
}


bool Settings::ReadFromFile(const TCHAR* filename)
{
	impl_->store_.clear();

	CFile f(filename, CFile::modeRead);

	BYTE h[sizeof(header)];
	if (f.Read(h, sizeof(h)) != sizeof(h) || memcmp(header, h, sizeof(h)) != 0)
		return false;

	for (;;)
	{
		// read key
		String key= read_str(f);
		if (key.empty())
			break;

		// read value
		char type= read_char(f);
		switch (type)
		{
		case 's':
			impl_->store_[key] = read_str(f);
			break;

		case 'i':
			impl_->store_[key] = read_int(f);
			break;

		case 'v':
			impl_->store_[key] = read_blob(f);
			break;

		default:
			ASSERT(false);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool keep_settings_in_registry= true;

// are setings persisted in registry (true) or file (false)?
bool SettingsInRegistry()
{
	return keep_settings_in_registry;
}

// persist settings in registry or file in ExifPro's folder
void SetSettingsInRegistry(bool use_registry)
{
	keep_settings_in_registry = use_registry;
}


Settings& Instance()
{
	static Settings settings;
	return settings;
}


String Id(const TCHAR* section, const TCHAR* key)
{
	String id(section);
	id += _T(':');
	id += key;
	return id;
}


void Store(const TCHAR* section, const TCHAR* key, const String& v)
{
	if (SettingsInRegistry())
		AfxGetApp()->WriteProfileString(section, key, v.c_str());
	else
		Instance().Store(Id(section, key), v);
}


void Store(const TCHAR* section, const TCHAR* key, int v)
{
	if (SettingsInRegistry())
		AfxGetApp()->WriteProfileInt(section, key, v);
	else
		Instance().Store(Id(section, key), v);
}


void Store(const TCHAR* section, const TCHAR* key, const std::vector<BYTE>& blob)
{
	if (SettingsInRegistry())
		AfxGetApp()->WriteProfileBinary(section, key, const_cast<BYTE*>(blob.empty() ? 0 : &blob.front()), static_cast<UINT>(blob.size()));
	else
		Instance().Store(Id(section, key), blob);
}


CString Restore(const TCHAR* section, const TCHAR* key, const TCHAR* default_val)
{
	if (SettingsInRegistry())
		return AfxGetApp()->GetProfileString(section, key, default_val);
	else
		return Instance().RestoreString(Id(section, key), default_val).c_str();
}


int Restore(const TCHAR* section, const TCHAR* key, int default_val)
{
	if (SettingsInRegistry())
		return AfxGetApp()->GetProfileInt(section, key, default_val);
	else
		return Instance().RestoreInt(Id(section, key), default_val);
}


} // namespace
