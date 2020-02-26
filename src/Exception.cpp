/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Exception.h"

struct Exception::Impl //: mik::counter_base
{
	Impl(const std::wstring& caption, const std::wstring& title, const std::wstring& description, const std::string& call_stack)
		: counter(0), caption(caption), title(title), description(description), call_stack(call_stack)
	{}

	int counter;
	std::wstring caption;
	std::wstring title;
	std::wstring description;
	std::string call_stack;
};


Exception::Exception(const wchar_t* caption, const wchar_t* title, const wchar_t* description, const char* call_stack)
 : impl_(new Impl(caption, title, description, call_stack))
{}

Exception::Exception(const wchar_t* title, const wchar_t* description, const char* call_stack)
 : impl_(new Impl(L"", title, description, call_stack))
{}

Exception::~Exception()
{}


const wchar_t* Exception::GetCaption() const
{
	return impl_->caption.c_str();
}

const wchar_t* Exception::GetTitle() const
{
	return impl_->title.c_str();
}

const wchar_t* Exception::GetDescription() const
{
	return impl_->description.c_str();
}

const char* Exception::GetCallStack() const
{
	return impl_->call_stack.c_str();
}
