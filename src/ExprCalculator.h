/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class PhotoInfo;
class Lua;


class ExprCalculator
{
public:
	ExprCalculator(const TCHAR* expression);
	~ExprCalculator();

	void DefineGlobalVar(const char* name, const char* value);
	void DefineGlobalVar(const char* name, int value);

	bool IsValid(String& err_msg, const PhotoInfo& photo) const;

	bool IsValid(String& err_msg, const PhotoInfo& photo, int max_steps) const;

//	String Calculate(Lua& lua, const PhotoInfo& photo) const;

private:
	struct Impl;
	std::auto_ptr<Impl> impl_;
	std::string expression_;
	std::string raw_expression_;
};
