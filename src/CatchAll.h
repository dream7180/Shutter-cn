/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _catch_all_
#define _catch_all_

#include "JPEGException.h"
#include "Exception.h"
#include "ErrorDlg.h"
extern CString CurCallStackInfo();
std::string CurrentCallStackInfo(CONTEXT& ctx);
//void DisplayErrorDialog(CWnd* parent, const wchar_t* title, const wchar_t* message, const std::string& callstack);
//void DisplayErrorDialog(const wchar_t* caption, CWnd* parent, const wchar_t* title, const wchar_t* message);

#define CURRENT_CALL_STACK_DUMP	\
	CONTEXT ctx; \
	memset(&ctx, 0, sizeof ctx); \
	::RtlCaptureContext(&ctx); \
	std::string callstack= CurrentCallStackInfo(ctx);


#undef CATCH_ALL	// MFC

#define CATCH_ALL_KNOWN_W(wnd)	\
	catch (CException* ex) \
	{ \
		{ \
			CURRENT_CALL_STACK_DUMP \
			TCHAR error_message[512]; \
			if (ex->GetErrorMessage(error_message, array_count(error_message))) \
				DisplayErrorDialog((wnd), L"MFC exception encountered", error_message, callstack); \
			else \
				DisplayErrorDialog((wnd), L"MFC exception encountered", 0, callstack); \
		} \
		ex->Delete(); \
	} \
	catch (JPEGException& ex)	/* jpeg decoding error? */ \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), L"JPEG library error", ex.GetMessage(), callstack); \
	} \
	catch (std::exception& ex) \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), L"Standard library exception encountered", CString(ex.what()), callstack); \
	} \
	catch (std::wstring& str) \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), str.c_str(), 0, callstack); \
	} \
	catch (std::string& str) \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), CString(str.c_str()), 0, callstack); \
	} \
	catch (const wchar_t* str) \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), str, 0, callstack); \
	} \
	catch (const char* str) \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), CString(str), 0, callstack); \
	} \
	catch (UserException& ex) \
	{ \
		DisplayErrorDialog(ex.GetCaption(), (wnd), ex.GetTitle(), ex.GetDescription(), false); \
	} \
	catch (Exception& ex) \
	{ \
		DisplayErrorDialog((wnd), ex.GetTitle(), ex.GetDescription(), ex.GetCallStack()); \
	}

#define CATCH_UNKNOWN_W(wnd)	\
	catch (...) \
	{ \
		CURRENT_CALL_STACK_DUMP \
		DisplayErrorDialog((wnd), L"Unexpected exception encountered.", 0, callstack); \
	}



#ifdef _DEBUG	//=========================================================================

// no generic catch in debug mode

#define CATCH_ALL		CATCH_ALL_KNOWN_W(0)

#define CATCH_ALL_W(wnd)	CATCH_ALL_KNOWN_W(wnd)

#else	// RELEASE =================================================================================

#define CATCH_ALL_W(wnd)	\
	CATCH_ALL_KNOWN_W(wnd) \
	CATCH_UNKNOWN_W(wnd)

#define CATCH_ALL		CATCH_ALL_W(0)

#endif	// ==============================================================================



#define CATCH_FILE_GUARDS		\
	catch (FileGuardException& ex) \
	{ \
		ex->ReportError(); \
	}

#endif // _catch_all_
