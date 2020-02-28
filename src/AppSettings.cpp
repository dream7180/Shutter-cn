/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "AppSettings.h"
#include "StringConversions.h"
#include <boost/variant.hpp>


typedef boost::variant<String, double, int, bool, std::vector<BYTE> > Variant;

typedef std::map<String, Variant> Map;


struct AppSettings::Impl
{
	Map map_;

	String Key(const TCHAR* section, const TCHAR* key)
	{
		oStringstream ost;
		ost << section << _T("\\") << key;
		return ost.str();
	}

	Map::iterator Get(const TCHAR* section, const TCHAR* key)
	{
		Map::iterator it= map_.find(Key(section, key));
		return it;
	}

	template <class T> bool ValueExtract(const TCHAR* section, const TCHAR* key, T& ret)
	{
		Map::iterator it= Get(section, key);
		if (it == map_.end())
			return false;
		ret = boost::get<T>(it->second);
		return true;
	}
};


AppSettings::AppSettings() : impl_(new AppSettings::Impl())
{
}

AppSettings::~AppSettings()
{
}


bool AppSettings::ValueExists(const TCHAR* section, const TCHAR* key)
{
	return impl_->map_.count(impl_->Key(section, key)) > 0;
}


int AppSettings::GetProfileInt(const TCHAR* section, const TCHAR* key, int default_val)
{
	int ret= default_val;
	impl_->ValueExtract(section, key, ret);
	return ret;
}


double AppSettings::GetProfileDouble(const TCHAR* section, const TCHAR* key, double default_val)
{
	double ret= default_val;
	impl_->ValueExtract(section, key, ret);
	return ret;
}


String AppSettings::GetProfileString(const TCHAR* section, const TCHAR* key, const TCHAR* default_val)
{
	String str;
	if (impl_->ValueExtract(section, key, str))
		return str;

	return default_val != 0 ? String(default_val) : String();
}


bool AppSettings::GetProfileBinary(const TCHAR* section, const TCHAR* key, std::vector<BYTE>& bytes)
{
	if (impl_->ValueExtract(section, key, bytes))
		return true;

	bytes.clear();
	return false;
}


String EscapeString(const String& str)
{
	//TODO:
	//
	return str;
}


class save_var_visitor : public boost::static_visitor<>
{
public:
	save_var_visitor(std::iostream& out) : out_(out)
	{}

	void operator () (const String& str) const
	{
		out_ << TStr2AStr(EscapeString(str)).c_str();
	}

	void operator () (const std::vector<BYTE>& v) const
	{
		const size_t count= v.size();

		out_ << count;

		for (size_t i= 0; i < count; ++i)
			out_ << ' ' << std::hex << static_cast<unsigned int>(v[i]);
	}

	void operator () (bool b) const
	{
		out_ << (b ? "true" : "false");
	}

	template <typename T>
	void operator () (const T& val) const
	{
		out_ << val;
	}

private:
	std::iostream& out_;
};


class var_type_visitor : public boost::static_visitor<const char*>
{
public:
	result_type operator () (const String&) const
	{
		return "s";
	}

	result_type operator () (const int&) const
	{
		return "i";
	}

	result_type operator () (const bool&) const
	{
		return "b";
	}

	result_type operator () (const double&) const
	{
		return "d";
	}

	result_type operator () (const std::vector<BYTE>&) const
	{
		return "v";
	}

	template <typename T>
	result_type operator () (T& val) const
	{
		throw String(_T("不支持的类型"));
	}
};


bool AppSettings::Save(const TCHAR* file_name)
{
	CString name= file_name;
	name += _T(".$");

	std::fstream file(name, std::ios::out);

	file << '\xef' << '\xbb' << '\xbf';	// UTF-8 BOM

	for (Map::const_iterator it= impl_->map_.begin(); it != impl_->map_.end(); ++it)
	{
		// key
		file << it->first.c_str() << ':';

		// actual type
		const char* type= boost::apply_visitor(var_type_visitor(), it->second);
		file << type << '\t';

		// value
		boost::apply_visitor(save_var_visitor(file), it->second);

		file << std::endl;
	}

	file.close();

	// rename temp to the proper name
	if (!::DeleteFile(file_name))
	{
		::DeleteFile(name);
		return false;
	}

	return !!::MoveFile(name, file_name);
}


bool AppSettings::Load(const TCHAR* file_name)
{
	std::ifstream file(file_name, std::ios::in);

	char c1, c2, c3;
	file >> c1;
	file >> c2;
	file >> c3;

	if (c1 != '\xef' || c2 != '\xbb' || c3 != '\xbf')
		return false;

	std::string line;
	std::getline(file, line);

	// expected format:
	// <section-key-path>:<type>\t<value>endl

	std::string::size_type pos= line.find(':');

	//

	return false;
}
