/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Lua.h"
class PhotoInfo;


struct CustomColumnDef
{
	CustomColumnDef() : visible_(false) {}

	String caption_;
	String expression_;
	bool visible_;
};


class CustomColumns
{
public:
	CustomColumns();
	CustomColumns(const CustomColumns& src);
	~CustomColumns();

	CustomColumnDef& operator [] (std::size_t i);
	const CustomColumnDef& operator [] (std::size_t i) const;

	std::size_t size() const	{ return CUST_COLUMNS; }

	const static std::size_t CUST_COLUMNS= 15;

	void assign(const CustomColumns& src);

	enum Result { Err, Number, Text };
	// calculate value for 'column' and 'photo'
	Result CalcValue(size_t column, const PhotoInfo& photo, double& num_result, String& text_result) const;

	// less than operator
	bool Less(size_t column, const PhotoInfo& photo1, const PhotoInfo& photo2) const;

private:
	void Init() const;
	void Defaults();
	CustomColumnDef columns_[CUST_COLUMNS];
	mutable std::auto_ptr<Lua> lua_;
};
