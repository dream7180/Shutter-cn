/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _prop_field_h_
#define _prop_field_h_


class CPropField
{
public:
	CPropField(const TCHAR* name_string, bool read_only= false, bool multi_line= false)
		: name_(name_string), read_only_(read_only), multi_line_(multi_line), text_limit_(2000)
	{}
	virtual ~CPropField()
	{}

	String virtual GetVal() const = 0;
	bool virtual SetVal(const String& val)= 0;

	const TCHAR* GetString() const	{ return name_.c_str(); }

	bool IsReadOnly() const			{ return read_only_; }

	bool IsMultiLine() const		{ return multi_line_; }

	int TextLimit() const			{ return text_limit_; }
	void SetTextLimit(int limit)	{ text_limit_ = limit; }

private:
	String name_;
	bool read_only_;
	bool multi_line_;
	int text_limit_;
};


class CPropGroup : public std::vector<CPropField*>
{
public:
	CPropGroup()
	{}

	CPropGroup(const TCHAR* name) : name_(name)
	{}

	void SetName(const TCHAR* name)	{ name_ = name; }
	const TCHAR* GetName() const		{ return name_.c_str(); }

	~CPropGroup();

private:
	String name_;
};



#endif // _prop_field_h_
