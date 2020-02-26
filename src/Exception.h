/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "StringFormatter.h"
#include <boost/shared_ptr.hpp>

// exception class meant for reporting exceptional conditions (unexpected errors);
// it is presented in a status dialog with title, explanation, call stack, and error reporting button

class Exception
{
public:
	Exception(const wchar_t* title, const wchar_t* description, const char* call_stack);
	Exception(const wchar_t* caption, const wchar_t* title, const wchar_t* description, const char* call_stack);

	virtual ~Exception();

	const wchar_t* GetCaption() const;
	const wchar_t* GetTitle() const;
	const wchar_t* GetDescription() const;
	const char* GetCallStack() const;

private:
	struct Impl;
	boost::shared_ptr<Impl> impl_;

	Exception& operator = (const Exception&);
};


// number to string
#define STRINGIZE_HELPER(something)		#something
#define STRINGIZE(something)			STRINGIZE_HELPER(something)

// single string identifying function name and current line number
#define FUNCTION_AND_LINE				__FUNCTION__ " line: " STRINGIZE(__LINE__)


// string formatting helper macro
#define SF(msg)							((StrFormat() << msg).string().c_str())


extern std::string CurrentCallStackInfo(CONTEXT& ctx);

// capture current call stack
#define CURRENT_CALL_STACK_DUMP	\
	CONTEXT ctx; \
	memset(&ctx, 0, sizeof ctx); \
	::RtlCaptureContext(&ctx); \
	std::string callstack= CurrentCallStackInfo(ctx);


#define THROW_EXCEPTION(ttl, msg)		{	\
											CURRENT_CALL_STACK_DUMP	\
											throw Exception((ttl), (msg), ((FUNCTION_AND_LINE "\n\n") + callstack).c_str());	\
										}


// this exception is meant for reporting errors that are expected and accounted for;
// it is presented in a status dialog with title and explanation, but no reporting nor call stack

class UserException : public Exception
{
public:
	UserException(const wchar_t* caption, const wchar_t* title, const wchar_t* description) : Exception(caption, title, description, 0)
	{}
	UserException(const wchar_t* title, const wchar_t* description) : Exception(L"", title, description, 0)
	{}
};
