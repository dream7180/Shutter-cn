// DelayedDirectoryChangeHandler.cpp: implementation of the CDelayedDirectoryChangeHandler2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

/*

#include "DirectoryChanges.h"
#include "DelayedDirectoryChangeHandler.h"
#include <process.h>//for _beginthreadex

#include <shlwapi.h>				 // for PathMatchSpec
#pragma comment( lib, "shlwapi.lib") // function


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UWM_DELAYED_DIRECTORY_NOTIFICATION (WM_APP+1024)


HINSTANCE GetInstanceHandle()
{
	return (HINSTANCE)GetModuleHandle(NULL);
	// ASSERT( AfxGetInstanceHandle() == (HINSTANCE)GetModuleHandle(NULL) ); <-- true for building .exe's 
	//NOTE: In Dll's using shared MFC, AfxGetInstanceHandle() != (HINSTANCE)GetModuleHandle(NULL)...
	//don't know if this is the case for dll's using static MFC
}
static inline bool IsEmptyString(LPCTSTR sz)
{
	return (bool)(sz==NULL || *sz == 0);
}
// *********************************************************
//  PathMatchSpec() requires IE 4.0 or greater on NT...
//  if running on NT 4.0 w/ out IE 4.0, then uses this function instead.
//
//  Based on code by Jack Handy:
//  http://www.codeproject.com/string/wildcmp.asp
//
//  Changed slightly to match the PathMatchSpec signature, be unicode compliant & to ignore case by myself.
//  
// *********************************************************

//#define _TESTING_WILDCMP_ONLY_ 

BOOL STDAPICALLTYPE wildcmp(LPCTSTR string, LPCTSTR wild )
{
	const TCHAR *cp, *mp;
	cp = mp = NULL;
	
	while (*string && *wild != _T('*'))
	{
		if (toupper(*wild) != toupper(*string) && *wild != _T('?'))
		{
			return FALSE;
		}
		wild++;
		string++;
	}
		
	while (*string)
	{
		if (*wild == _T('*'))
		{
			if (!*++wild)
			{
				return TRUE;
			}
			mp = wild;
			cp = string+1;
		} 
		else 
		if (toupper(*wild) == toupper(*string) || *wild == _T('?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == _T('*'))
	{
		wild++;
	}
	return (!*wild)? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//CDirChangeNotification member functions:
//
CDirChangeNotification::CDirChangeNotification(CDelayedDirectoryChangeHandler *	delayed_handler, DWORD partial_path_offset)
:delayed_handler_( delayed_handler )
,file_name1_(NULL)
,file_name2_(NULL)
,error_(0UL)
,partial_path_offset_(partial_path_offset)
{
	ASSERT( delayed_handler );
}

CDirChangeNotification::~CDirChangeNotification()
{
	if( file_name1_ ) free(file_name1_), file_name1_ = NULL;
	if( file_name2_ ) free(file_name2_), file_name2_ = NULL;
}

void CDirChangeNotification::DispatchNotificationFunction()
{
	ASSERT( delayed_handler_ );
	if( delayed_handler_ )
		delayed_handler_->DispatchNotificationFunction( this );
}

void CDirChangeNotification::PostOn_FileAdded(LPCTSTR file_name)
{
	ASSERT( file_name );
	function_to_dispatch_	= on__file_added;
	file_name1_			= _tcsdup( file_name) ;
	//
	// post the message so it'll be dispatch by another thread.
	PostNotification();

}
void CDirChangeNotification::PostOn_FileRemoved(LPCTSTR file_name)
{
	ASSERT( file_name );
	function_to_dispatch_	= on__file_removed;
	file_name1_			= _tcsdup( file_name) ;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
	
}
void CDirChangeNotification::PostOn_FileNameChanged(LPCTSTR old_name, LPCTSTR new_name)
{
	ASSERT( old_name && new_name );

	function_to_dispatch_	= on__file_name_changed;
	file_name1_			= _tcsdup( old_name) ;
	file_name2_			= _tcsdup( new_name) ;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
	
}

void CDirChangeNotification::PostOn_FileModified(LPCTSTR file_name)
{
	ASSERT( file_name );

	function_to_dispatch_	= on__file_modified;
	file_name1_			= _tcsdup( file_name );
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
}

void CDirChangeNotification::PostOn_ReadDirectoryChangesError(DWORD error, LPCTSTR directory_name)
{
	ASSERT( directory_name );

	function_to_dispatch_ = on__read_directory_changes_error;
	error_			  = error;
	file_name1_		  = _tcsdup(directory_name);
	//
	// post the message so it'll be dispatched by the another thread.
	PostNotification();
	
}

void CDirChangeNotification::PostOn_WatchStarted(DWORD error, LPCTSTR directory_name)
{
	ASSERT( directory_name );

	function_to_dispatch_ = on__watch_started;
	error_			  =	error;
	file_name1_		  = _tcsdup(directory_name);

	PostNotification();
}

void CDirChangeNotification::PostOn_WatchStopped(LPCTSTR directory_name)
{
	ASSERT( directory_name );

	function_to_dispatch_ = on__watch_stopped;
	file_name1_		  = _tcsdup(directory_name);

	PostNotification();
}

void CDirChangeNotification::PostNotification()
{
	ASSERT( delayed_handler_ );
	if( delayed_handler_ )
		delayed_handler_->PostNotification( this );
}

static LRESULT CALLBACK DelayedNotificationWndProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//	This is the wndproc for the notification window
//
//	it's here to dispatch the notifications to the client
//
{
	if( message == UWM_DELAYED_DIRECTORY_NOTIFICATION )
	{
TRACE(L"delayed notification arrived\n");
		CDirChangeNotification * notification = reinterpret_cast<CDirChangeNotification*>(lParam);
		ASSERT(  notification );
TRACE(L"delayed notification %p\n", notification);
		if( notification )
		{
			DWORD ex(0);
			__try{
				notification->DispatchNotificationFunction();
			}
			__except(ex = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER){
				//An exception was raised:
				//
				//	Likely cause: there was a problem creating the CDelayedDirectoryChangeHandler::watch_stopped_dispatched_event_ object
				//	and the change handler object was deleted before the notification could be dispatched to this function.
				//
				//  or perhaps, somebody's implementation of an overridden function caused an exception
				TRACE(_T("Following exception occurred: %d -- File: %s Line: %d\n"), ex, _T(__FILE__), __LINE__);
			}
		}
		
		return 0UL;
	}
	else
		return DefWindowProc(wnd,message,wParam,lParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//
//CDelayedNotificationWindow static member vars:
//
long CDelayedNotificationWindow::ref_cnt_ = 0L;
HWND CDelayedNotificationWindow::s_hWnd = NULL;
BOOL CDelayedNotificationWindow::register_window_ = FALSE;
//
//
long CDelayedNotificationWindow::AddRef()//creates window for first time if necessary
{
	if( InterlockedIncrement(&ref_cnt_) == 1
		||	!::IsWindow( s_hWnd ) )
	{
		TRACE(_T("CDelayedNotificationWindow -- Creating the notification window\n"));
		VERIFY( CreateNotificationWindow() );
	}
	return ref_cnt_;
}

long CDelayedNotificationWindow::Release()//destroys window for last time if necessary
{
	long ref_cnt = -1;
	if( (ref_cnt = InterlockedDecrement(&ref_cnt_)) == 0 )
	{
		//no body else using the window so destroy it?
		TRACE(_T("CDelayedNotificationWindow -- Destroying the notification window\n"));
		DestroyWindow( s_hWnd );
		s_hWnd = NULL;
	}
	return ref_cnt;
}
BOOL CDelayedNotificationWindow::RegisterWindowClass(LPCTSTR class_name)
//
//	registers our own window class to use as the hidden notification window.
//
{
	WNDCLASS wc = {0};
	
	wc.style = 0;
	wc.hInstance		= GetInstanceHandle();
	wc.lpszClassName	= class_name;
	wc.hbrBackground	= (HBRUSH)GetStockObject( WHITE_BRUSH );
	wc.lpfnWndProc		= DelayedNotificationWndProc;
	
	ATOM ant = RegisterClass( &wc );
	if( ant == NULL )
	{
		TRACE(_T("CDirChangeNotification::RegisterWindowClass - RegisterClass failed: %d\n"), GetLastError());
	}
	return (BOOL)(ant!= NULL);
	
}

BOOL CDelayedNotificationWindow::CreateNotificationWindow()
//
//	Create the hidden notification windows.
//
{
	TCHAR class_name[] = _T("Delayed_Message_Sender");
	if( !register_window_ )
		register_window_ = RegisterWindowClass(class_name);
	s_hWnd 	= CreateWindowEx(0, class_name, _T("DelayedWnd"),0,0,0,0,0, NULL, 0, 
							GetInstanceHandle(), NULL);
	if( s_hWnd == NULL )
	{
		TRACE(_T("Unable to create notification window! GetLastError(): %d\n"), GetLastError());
		TRACE(_T("File: %s Line: %d\n"), _T(__FILE__), __LINE__);
	}
	
	return (BOOL)(s_hWnd != NULL);
}
void CDelayedNotificationWindow::PostNotification(CDirChangeNotification * notification)
//
//	Posts a message to a window created in the main 
//	thread.
//	The main thread catches this message, and dispatches it in 
//	the context of the main thread.
//
{
	ASSERT( notification );
	ASSERT( s_hWnd );
	ASSERT( ::IsWindow( s_hWnd ) );

	PostMessage(s_hWnd, 
				UWM_DELAYED_DIRECTORY_NOTIFICATION, 
				0, 
				reinterpret_cast<LPARAM>( notification ));

//  if you don't want the notification delayed, 
//  
//	if( false )
//	{
//		notification->DispatchNotificationFunction();
//	}
}

/////////////////////////////////////////////////////////
//	CDelayedNoticationThread
//
long	CDelayedNotificationThread::ref_cnt_ = 0L;
HANDLE	CDelayedNotificationThread::thread_ = NULL;
DWORD	CDelayedNotificationThread::thread_id_ = 0UL;

void CDelayedNotificationThread::PostNotification(CDirChangeNotification * notification)
{
	ASSERT( thread_ != NULL );
	ASSERT( thread_id_ != 0 );

	if(
		!PostThreadMessage(thread_id_, 
						   UWM_DELAYED_DIRECTORY_NOTIFICATION, 
						   0, 
						   reinterpret_cast<LPARAM>(notification))
	  )
	{
		//Note, this can sometimes fail.
		//Will fail if: thread_id_ references a invalid thread id(the thread has died for example)
		// OR will fail if the thread doesn't have a message queue.
		//
		//	This was failing because the thread had not been fully started by the time PostThreadMessage had been called
		//
		//Note: if this fails, it creates a memory leak because
		//the CDirChangeNotification object that was allocated and posted
		//to the thread is actually never going to be dispatched and then deleted.... it's
		//hanging in limbo.....

		//
		//	The fix for this situation was to force the thread that starts
		//	this worker thread to wait until the worker thread was fully started before
		//	continueing.  accomplished w/ an event... also.. posting a message to itself before signalling the 
		//  'spawning' thread that it was started ensured that there was a message pump
		//  associated w/ the worker thread by the time PostThreadMessage was called.
		TRACE(_T("PostThreadMessage() failed while posting to thread id: %d! GetLastError(): %d%s\n"), thread_id_, GetLastError(), GetLastError() == ERROR_INVALID_THREAD_ID? _T("(ERROR_INVALID_THREAD_ID)") : _T(""));
	}
}

bool CDelayedNotificationThread::StartThread()
{
	TRACE(_T("CDelayedNotificationThread::StartThread()\n"));
	ASSERT( thread_ == NULL 
		&&	thread_id_ == 0 );
	thread_ = (HANDLE)_beginthreadex(NULL,0, 
								ThreadFunc, this, 0, (UINT*) &thread_id_);
	if( thread_ )
		WaitForThreadStartup();

	return thread_ == NULL ? false : true;

}

bool CDelayedNotificationThread::StopThread()
{
	TRACE(_T("CDelayedNotificationThread::StopThread()\n"));
	if( thread_ != NULL 
	&&	thread_id_ != 0 )
	{
		PostThreadMessage(thread_id_, WM_QUIT, 0,0);

		WaitForSingleObject(thread_, INFINITE);
		CloseHandle(thread_);
		thread_	 = NULL;
		thread_id_ = 0UL;
		return true;
	}
	return true;//already shutdown
}

UINT __stdcall CDelayedNotificationThread::ThreadFunc(LPVOID lpvThis)
{
	//UNREFERENCED_PARAMETER( lpvThis );
	//
	//	Implements a simple message pump
	//
	CDelayedNotificationThread * this = reinterpret_cast<CDelayedNotificationThread*>(lpvThis);
	ASSERT( this );

	//
	//	Insure that this thread has a message queue by the time another
	//	thread gets control and tries to use PostThreadMessage
	//	problems can happen if someone tries to use PostThreadMessage
	//	in between the time this->SignalThreadStartup() is called,
	//	and the first call to GetMessage();

	::PostMessage(NULL, WM_NULL, 0,0);//if this thread didn't have a message queue before this, it does now.


	//
	//
	//	Signal that this thread has started so that StartThread can continue.
	//
	if( this ) this->SignalThreadStartup();

	TRACE(_T("CDelayedNotificationThread::ThreadFunc() ThreadID: %d -- Starting\n"), GetCurrentThreadId());
	MSG msg;
	do{
		while( GetMessage(&msg, NULL, 0,0) )//note GetMessage() can return -1, but only if i give it a bad HWND.(HWND for another thread for example)..i'm not giving an HWND, so no problemo here.
		{
			if( msg.message == UWM_DELAYED_DIRECTORY_NOTIFICATION )
			{
				CDirChangeNotification * notification = 
								reinterpret_cast<CDirChangeNotification *>( msg.lParam );
				DWORD ex(0UL);

				__try{
				if( notification )
					notification->DispatchNotificationFunction();
				}
				__except(ex = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER){
				//An exception was raised:
				//
				//	Likely causes: 
				//		* There was a problem creating the CDelayedDirectoryChangeHandler::watch_stopped_dispatched_event_ object
				//			and the change handler object was deleted before the notification could be dispatched to this function.
				//
				//		* Somebody's implementation of an overridden virtual function caused an exception
				TRACE(_T("The following exception occurred: %d -- File: %s Line: %d\n"), ex, _T(__FILE__), __LINE__);
				}
			}
			else
			if( msg.message == WM_QUIT )
			{
				break;
			}
		}
	}while( msg.message != WM_QUIT );
	TRACE(_T("CDelayedNotificationThread::ThreadFunc() exiting. ThreadID: %d\n"), GetCurrentThreadId());
	return 0;
}

long CDelayedNotificationThread::AddRef()
{
	if( InterlockedIncrement(&ref_cnt_) == 1 )
	{
		VERIFY( StartThread() );
	}
	return ref_cnt_;
}
long CDelayedNotificationThread::Release()
{
	if( InterlockedDecrement(&ref_cnt_) <= 0 )
	{
		ref_cnt_ = 0;
		VERIFY( StopThread() );
	}
	return ref_cnt_;
}

///////////////////////////////////////////////////////
//static member data for CDelayedDirectoryChangeHandler
HINSTANCE CDelayedDirectoryChangeHandler::shlwapi_dll_ = NULL;//for the PathMatchSpec() function
BOOL CDelayedDirectoryChangeHandler::shlwapi_dll_exists_ = TRUE;
long CDelayedDirectoryChangeHandler::ref_cnt_h_shlwapi_ = 0L;
FUNC_PatternMatchSpec CDelayedDirectoryChangeHandler::fp_pattern_match_spec_ = wildcmp;//default
///////////////////////////////////////////////////////
//construction destruction
CDelayedDirectoryChangeHandler::CDelayedDirectoryChangeHandler(CDirectoryChangeHandler * real_handler, bool app_hasGUI, LPCTSTR include_filter, LPCTSTR exclude_filter, DWORD filter_flags)
: delay_notifier_( NULL )
 ,real_handler_( real_handler )
 ,include_filter_(NULL)
 ,exclude_filter_(NULL)
 ,filter_flags_( filter_flags )
 ,partial_path_offset_( 0UL )
 ,watch_stopped_dispatched_event_(NULL)
 ,num_include_filter_specs_(0)
 ,num_exclude_filter_specs_(0)
{


	ASSERT( real_handler_ ); 

	InitializePathMatchFunc( include_filter, exclude_filter );

	//
	// See that we're 
	//


	watch_stopped_dispatched_event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);//AUTO-RESET, not initially signalled
	ASSERT( watch_stopped_dispatched_event_ );
	
	if( app_hasGUI )
	{
		//
		//	The value true was passed to the CDirectoryChangeWatcher constructor.
		//	It's assumed that your app has a gui, that is, it implements
		//	a message pump.  To delay the notification to another thread,
		//	we'll use a hidden notification window.
		//
		delay_notifier_ = new CDelayedNotificationWindow();
	}
	else
	{
		// The value 'false' was passed to the CDirectoryChangeWatcher constructor.
		//
		// Your app has no message pump... use a class that implements one for you
		// in a worker thread.
		//
		// Notifications will be executed in this worker thread.
		//
		delay_notifier_ = new CDelayedNotificationThread();
	}
}

CDelayedDirectoryChangeHandler::~CDelayedDirectoryChangeHandler()
{
	if( real_handler_ )
		delete real_handler_, real_handler_ = NULL;
	if( delay_notifier_ )
		delete delay_notifier_, delay_notifier_ = NULL;

	if( watch_stopped_dispatched_event_ )
		CloseHandle(watch_stopped_dispatched_event_), watch_stopped_dispatched_event_ = NULL;

	if( include_filter_ ){
		if( num_include_filter_specs_ == 1 )
			free(include_filter_);
		else
		{
			TCHAR ** tmp = (TCHAR**)include_filter_;
			for(int i(0); i < num_include_filter_specs_; ++i)
			{
				free( *tmp++ );
			}
			free( include_filter_ );
		}
		include_filter_ = NULL;
		num_include_filter_specs_;
	}
	if( exclude_filter_ ) {
		if( num_exclude_filter_specs_ == 1 )
			free(exclude_filter_);
		else{
			TCHAR ** tmp = (TCHAR**)exclude_filter_;
			for(int i(0); i < num_exclude_filter_specs_; ++i)
			{
				free( *tmp++ );
			}
			free( exclude_filter_ );
		}
		exclude_filter_ = NULL;
		num_exclude_filter_specs_ = 0;
	}

	UninitializePathMatchFunc();
}

BOOL CDelayedDirectoryChangeHandler::_PathMatchSpec(LPCTSTR path, LPCTSTR pattern)
{
	if( fp_pattern_match_spec_ )
	{
		return fp_pattern_match_spec_(path, pattern);
	}
	ASSERT( FALSE );
	return TRUE;//everything matches.
}

BOOL CDelayedDirectoryChangeHandler::InitializePathMatchFunc(LPCTSTR include_filter, LPCTSTR exclude_filter)
//
//
//	To support the Include and Exclude filters, the PathMatchSpec function is used.
//	PathMatchSpec is only supported on NT4.0 if IE 4.0 is installed.
//
//	for the case where this code is running on NT 4.0 w/out IE 4.0, we use
//	a different function: wildcmp ()
//
//
//	This function attempts to load shlwapi.dll dynamically and find the PathMatchSpec function.
//
//	if the function PathMatchSpec can't be found, the function pointer fp_path_match_spec_ is set to wildcmp.
//
//
//	Note:  wildcmp doesn't support multiple file specs separated by a semi-colon
//	as PathMatchSpec does.... we'll support it by parsing them 
//	when we want to test the filters, we'll call the pattern matching functions multiple times...
//
{

	//
	//	Copy the include/exclude filters if specified...
	//
	//
	if( IsEmptyString(include_filter)
	&&	IsEmptyString(exclude_filter) )
	{
		return TRUE;//both the include && exclude filters aren't specified
					//no need to initialize the pattern matching function....
					//if filters are never used, then
					//one less dll is loaded.
	}

#ifdef _TESTING_WILDCMP_ONLY_
	shlwapi_dll_ = NULL;
	shlwapi_dll_exists_ = FALSE;
	return InitializePatterns(include_filter, exclude_filter);
#endif


	if( shlwapi_dll_ != NULL )
	{
		ASSERT( fp_pattern_match_spec_ != NULL );
		InterlockedIncrement(&ref_cnt_h_shlwapi_);

		return InitializePatterns(include_filter, exclude_filter);
	}
	else{
		if( shlwapi_dll_exists_ == TRUE )//either the dll exists, or we haven't tried loading it yet...
		{
			//The pattern match function hasn't been initialized yet....
			//
		
			shlwapi_dll_ = ::LoadLibrary(_T("Shlwapi.dll"));
			if( shlwapi_dll_ == NULL )
			{
				shlwapi_dll_exists_ = FALSE;//don't try loading this dll again.
				fp_pattern_match_spec_ = wildcmp;//even though it's set buy default, and this code will only get here once, set it just for fun.

				return InitializePatterns(include_filter, exclude_filter);
				
			}
			else
			{
				//Shlwapi.dll was found....check it for PathMatchSpec()
#ifdef UNICODE 
				fp_pattern_match_spec_ = (FUNC_PatternMatchSpec)::GetProcAddress(shlwapi_dll_, "PathMatchSpecW");
#else
				fp_pattern_match_spec_ = (FUNC_PatternMatchSpec)::GetProcAddress(shlwapi_dll_, "PathMatchSpecA");
#endif
	
				if( fp_pattern_match_spec_ != NULL )
				{
					//UsesRealPathMatchSpec() will now return true.
					//we're on NT w/ IE 4.0 or greater...(or Win2k/XP)
					InterlockedIncrement(&ref_cnt_h_shlwapi_);
					return InitializePatterns(include_filter, exclude_filter);
				}
				else
				{
					//we found shlwapi.dll, but it didn't have PathMatchSpec()
					::FreeLibrary( shlwapi_dll_ );
					shlwapi_dll_ = NULL;
					shlwapi_dll_exists_ = FALSE;

					//instead of using PathMatchSpec()
					//we'll use wildcmp()
					fp_pattern_match_spec_ = wildcmp;
					//UsesRealPathMatchSpec() will now return false w/out asserting.

					return InitializePatterns(include_filter, exclude_filter);
				}
			}
			
		}
	}
	return (fp_pattern_match_spec_ != NULL);
}

BOOL CDelayedDirectoryChangeHandler::InitializePatterns(LPCTSTR include_filter, LPCTSTR exclude_filter)
{
	ASSERT( !IsEmptyString(include_filter)   //one of these must have something in it, 
		||  !IsEmptyString(exclude_filter) );//or else this function shouldn't be called.

	if( shlwapi_dll_ != NULL )
	{
		//we're using Shlwapi.dll's PathMatchSpec function....
		//we're running on NT4.0 w/ IE 4.0 installed, or win2k/winXP(or greater)
		//
		//	Copy the include/exclude filters if specified...
		//
		//
		// we're using the real PathMatchSpec() function which
		//	supports multiple pattern specs...(separated by a semi-colon)
		//	so there's only one filter spec as far as my code is concerned.
		//
		if( !IsEmptyString(include_filter) )
		{
			include_filter_ = _tcsdup(include_filter);
			ASSERT( include_filter_ );
			num_include_filter_specs_ = 1;
		}
		if( !IsEmptyString(exclude_filter) )
		{
			exclude_filter_ = _tcsdup(exclude_filter);	
			ASSERT( exclude_filter_ );
			num_exclude_filter_specs_ = 1;
		}	
	}
	else
	{
		//shlwapi.dll isn't on this machine.... can happen on NT4.0 w/ out IE 4.0 installed.
		ASSERT( shlwapi_dll_exists_ == FALSE );

		//
		//	we're using the function wildcmp() instead of PathMatchSpec..
		//
		//	this means that multiple pattern specs aren't supported...
		// in order to support them, we'll tokenize the string into multiple
		// pattern specs and test the string multiple times(once per pattern spec)
		// in order to support multiple patterns.
		//
		//
		//	include_filter_ & exclude_ filter will be used like TCHAR**'s instead of TCHAR*'s
		//

		num_include_filter_specs_ = 0;
		if( !IsEmptyString(include_filter) )
		{
			TCHAR * tmp_filter = _tcsdup(include_filter);
			TCHAR * tok = _tcstok( tmp_filter, _T(";"));
			while( tok )
			{
				num_include_filter_specs_++;
				tok = _tcstok(NULL, _T(";"));
			}
			if( num_include_filter_specs_ == 1 )
				include_filter_ = _tcsdup(include_filter);
			else
			{   //allocate room for pointers .. one for each token...
				include_filter_ = (TCHAR*)malloc( num_include_filter_specs_ * sizeof(TCHAR*));

				free(tmp_filter);
				tmp_filter = _tcsdup(include_filter);
				tok = _tcstok(tmp_filter, _T(";"));
				TCHAR ** tmp = (TCHAR**)include_filter_;
				while(tok)
				{
					*tmp = _tcsdup(tok);
					tmp++;
					tok = _tcstok(NULL, _T(";"));
				}
			}

			free(tmp_filter);
		}

		//
		//	Do the same for the Exclude filter...
		//
		num_exclude_filter_specs_ = 0;
		if( !IsEmptyString(exclude_filter) )
		{
			TCHAR * tmp_filter = _tcsdup(exclude_filter);
			TCHAR * tok = _tcstok( tmp_filter, _T(";"));
			while( tok )
			{
				num_exclude_filter_specs_++;
				tok = _tcstok(NULL, _T(";"));
			}
			if( num_exclude_filter_specs_ == 1 )
				exclude_filter_ = _tcsdup(exclude_filter);
			else
			{   //allocate room for pointers .. one for each token...
				exclude_filter_ = (TCHAR*)malloc( num_exclude_filter_specs_ * sizeof(TCHAR*));

				free(tmp_filter);
				tmp_filter = _tcsdup(exclude_filter);

				tok = _tcstok(tmp_filter, _T(";"));
				TCHAR ** tmp = (TCHAR**)exclude_filter_;
				while(tok)
				{
					*tmp = _tcsdup(tok);
					tmp++;
					tok = _tcstok(NULL, _T(";"));
				}
			}
			free(tmp_filter);
		}

	}

	return (exclude_filter_!= NULL || (include_filter_ != NULL));
}
void CDelayedDirectoryChangeHandler::UninitializePathMatchFunc()
{
	if( shlwapi_dll_exists_ == TRUE 
	&&  shlwapi_dll_ != NULL )
	{
		if( InterlockedDecrement(&ref_cnt_h_shlwapi_) <= 0)
		{
			ref_cnt_h_shlwapi_ = 0;
			FreeLibrary( shlwapi_dll_ );
			shlwapi_dll_ = NULL;
			fp_pattern_match_spec_ = wildcmp;
		}
	}
}

bool CDelayedDirectoryChangeHandler::UsesRealPathMatchSpec() const
//are we using PathMatchSpec() or wildcmp()?
{
	if( shlwapi_dll_ != NULL && fp_pattern_match_spec_ != NULL )
		return true;
	if( shlwapi_dll_ == NULL && fp_pattern_match_spec_ != NULL )
		return false;

	ASSERT( FALSE );//this function was called before InitializePathMatchFunc()
	//oops!
	return false;
}
static inline bool HasTrailingBackslash(const CString & str )
{
	if( str.GetLength() > 0 
	&&	str[ str.GetLength() - 1 ] == _T('\\') )
		return true;
	return false;
}
void CDelayedDirectoryChangeHandler::SetPartialPathOffset(const CString & watched_dir_name)
{
	if( filter_flags_ & CDirectoryChangeWatcher::FILTERS_CHECK_PARTIAL_PATH )
	{
		//set the offset to 
		if( HasTrailingBackslash( watched_dir_name ) )
			partial_path_offset_ = watched_dir_name.GetLength();
		else
			partial_path_offset_ = watched_dir_name.GetLength() + 1;
	}
	else
		partial_path_offset_ = 0;
}

CDirChangeNotification * CDelayedDirectoryChangeHandler::GetNotificationObject()
//
//	Maybe in future I'll keep a pool of these 
//	objects around to increase performance...
//	using objects from a cache will be faster 
//	than allocated and destroying a new one each time.
//	
//  
{
	ASSERT( real_handler_ );
	return new CDirChangeNotification(this, partial_path_offset_);//helps support FILTERS_CHECK_PARTIAL_PATH
}

void CDelayedDirectoryChangeHandler::DisposeOfNotification(CDirChangeNotification * notification)
{
	delete notification;
}

//These functions are called when the directory to watch has had a change made to it
void CDelayedDirectoryChangeHandler::On_FileAdded(const CString & file_name)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p ) p->PostOn_FileAdded( file_name );
}

void CDelayedDirectoryChangeHandler::On_FileRemoved(const CString & file_name)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p ) p->PostOn_FileRemoved( file_name );
}

void CDelayedDirectoryChangeHandler::On_FileModified(const CString & file_name)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p ) p->PostOn_FileModified( file_name );
}

void CDelayedDirectoryChangeHandler::On_FileNameChanged(const CString & old_file_name, const CString & new_file_name)
{
	CDirChangeNotification * p = GetNotificationObject();	
	ASSERT( p );
	if( p ) p->PostOn_FileNameChanged( old_file_name, new_file_name );
}

void CDelayedDirectoryChangeHandler::On_ReadDirectoryChangesError(DWORD error, const CString & directory_name)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p ) p->PostOn_ReadDirectoryChangesError( error, directory_name );
}

void CDelayedDirectoryChangeHandler::On_WatchStarted(DWORD error, const CString & directory_name)
{
	if( !(filter_flags_ & CDirectoryChangeWatcher::FILTERS_NO_WATCHSTART_NOTIFICATION))
	{
		CDirChangeNotification * p = GetNotificationObject();

		if( p ) p->PostOn_WatchStarted(error, directory_name);
	}
}

void CDelayedDirectoryChangeHandler::On_WatchStopped(const CString & directory_name)
{
	if( !(filter_flags_ & CDirectoryChangeWatcher::FILTERS_NO_WATCHSTOP_NOTIFICATION))
	{
		CDirChangeNotification * p = GetNotificationObject();

		if( p ){
			if( watch_stopped_dispatched_event_ )
				::ResetEvent(watch_stopped_dispatched_event_);

			p->PostOn_WatchStopped( directory_name );

			//	Wait that this function has been dispatched to the other thread
			//	before continueing.  This object may be getting deleted
			//	soon after this function returns, and before the function can be
			//	dispatched to the other thread....
			WaitForOnWatchStoppedDispatched();
		}
	}
}


void CDelayedDirectoryChangeHandler::PostNotification(CDirChangeNotification * notification)
{
	if( delay_notifier_ )
		delay_notifier_->PostNotification( notification );
}

inline bool IsNonFilterableEvent( CDirChangeNotification::function_to_dispatch event)
// Helper function
//	For filtering events..... these functions can not be filtered out.
//
{
	if(	event == CDirChangeNotification::on__watch_started 
	||	event == CDirChangeNotification::on__watch_stopped
	||	event == CDirChangeNotification::on__read_directory_changes_error )
	{
		return true;
	}
	else
		return false;
}
DWORD GetPathOffsetBasedOnFilterFlags(CDirChangeNotification * not, DWORD filter_flags)
{
	
	ASSERT( not && filter_flags != 0 );
	//helps support the filter options FILTERS_CHECK_FULL_PATH, FILTERS_CHECK_PARTIAL_PATH, and FILTERS_CHECK_FILE_NAME_ONLY

	DWORD file_name_offset = 0;//offset needed to support FILTERS_CHECK_FULL_PATH
	if( filter_flags & CDirectoryChangeWatcher::FILTERS_CHECK_FILE_NAME_ONLY )
	{
		//set the offset to support FILTERS_CHECK_FILE_NAME_ONLY
		TCHAR * slash  = _tcsrchr(not->file_name1_, _T('\\'));
		if( slash )
			file_name_offset = (++slash - not->file_name1_);

		//
		//	Because file name change notifications take place in the same directory,
		//	the same file_name_offset can be used for the new_file_name(not->file_name2_)
		//	when checking the filter against the new file name.
		//
	}
	else
	if( filter_flags & CDirectoryChangeWatcher::FILTERS_CHECK_PARTIAL_PATH)
	{
		//
		//	partial path offset is the offset 
		//	from the beginning of the file name, 
		//	to the end of the watched directory path...
		//	ie: If you're watching "C:\Temp"
		//		and the file C:\Temp\SubFolder\FileName.txt is changed,
		//		the partial path offset will give you "SubFolder\FileName.txt"
		//		when this is checked against the include/exclude filter.
		//
		file_name_offset = not->partial_path_offset_;
	}
	//else
	//if( filter_flags_ & CDirectoryChangeWatcher::FILTERS_CHECK_FULL_PATH )
	//	file_name_offset = 0;
	
	return file_name_offset;
}

bool CDelayedDirectoryChangeHandler::NotifyClientOfFileChange(CDirChangeNotification * not)
//
//
//	Perform the tests to see if the client wants to be notified of this
//	file change notification.
//
//	Tests performed:
//
//	Event test:		Not all events can be filtered out.
//					On_ReadDirectoryChangesError
//					cannot be filtered out.
//	Filter flags test:  User can specify flags so that no tests are performed....all notifications are sent to the user.
//
//	Filter test:	Test the notification file name against include and exclude filters.
//
//					Only files changes matching the INCLUDE filter will be passed to the client.
//					By not specifying an include filter, all file changes are passed to the client.
//	
//				    Any files matching the EXCLUDE filter will not be passed to the client.
//
//
//					Note: For the file name change event:
//							If the old file name does not pass the tests for the include and exclude filter
//							but the NEW file name does pass the test for the filters, then the client IS notified.
//
//	Client test:	The CDirectoryChangeHandler derived class is given a chance to filter the event by calling
//					CDirectoryChangeHandler::On_FilterNotification()
//
//	RETURN VALUE:
//	If this function returns true, the notification function is called.
//	If it returns false, the notification is ignored.
//			The client is notified by calling CDirectoryChangeHandler's virtual functions On_FileAdded(),On_FileRemoved(), etc.
{
	ASSERT( not );
	ASSERT( real_handler_ );

	//
	//	Some events can't be ignored, or filtered out.
	//

	if( ((filter_flags_ & CDirectoryChangeWatcher::FILTERS_DONT_USE_ANY_FILTER_TESTS) == CDirectoryChangeWatcher::FILTERS_DONT_USE_ANY_FILTER_TESTS)
	||	IsNonFilterableEvent( not->function_to_dispatch_ ) )
	{
		// Either this is a non-filterable event, or we're not using any filters...
		// client is notified of all events..
		return true;
	}

	//
	//	See if user wants to test CDirectoryChangeHandler::On_FilterNotification()
	//	before tests are performed against the file name, and filter specifications
	//
	if( (filter_flags_ & CDirectoryChangeWatcher::FILTERS_TEST_HANDLER_FIRST )//specified that CDirectoryChangeHandler::On_FilterNotification is to be called before any other filter tests
	&&	!(filter_flags_ & CDirectoryChangeWatcher::FILTERS_DONT_USE_HANDLER_FILTER)//and did not specify that this CDirectoryChangeHandler::On_FilterNotification is not to be called..
	&&	!real_handler_->On_FilterNotification(not->function_to_dispatch_, not->file_name1_, not->file_name2_) )
	{
		//
		//	Client specified to test handler first, and it didn't pass the test... don't notify the client.
		//
		return false;
	}
	//else
	//
	//	this file change passed the user test, continue testing
	//	to see if it passes the filter tests.

	DWORD file_name_offset = GetPathOffsetBasedOnFilterFlags(not, filter_flags_);

	//
	//	See if the changed file matches the include or exclude filter
	//	Only allow notifications for included files 
	//	that have not been exluded.
	//
	if(!(filter_flags_ & CDirectoryChangeWatcher::FILTERS_DONT_USE_FILTERS) )
	{
		if( false == IncludeThisNotification(not->file_name1_ + file_name_offset)
		||	true == ExcludeThisNotification(not->file_name1_ + file_name_offset) )
		{
			if( not->function_to_dispatch_ != CDirChangeNotification::on__file_name_changed )
				return false;
			else{
				//Special case for file name change:
				//
				// the old file name didn't pass the include/exclude filter
				// but if the new name passes the include/exclude filter, 
				// we will pass it on to the client...

				if( false == IncludeThisNotification(not->file_name2_ + file_name_offset)
				||	true == ExcludeThisNotification(not->file_name2_ + file_name_offset) )
				{
					// the new file name didn't pass the include/exclude filter test either
					// so don't pass the notification on...
					return false;
				}

			}
		}

	}
	
	//
	//	Finally, let the client determine whether or not it wants this file notification
	//	if this test has not already been performed...
	//

	if( (filter_flags_ & CDirectoryChangeWatcher::FILTERS_TEST_HANDLER_FIRST)
	||	(filter_flags_ & CDirectoryChangeWatcher::FILTERS_DONT_USE_HANDLER_FILTER) )
	{
		//	if we got this far, and this flag was specified, 
		//	it's already passed this test,
		//  or we're not checking it based on the filter flag FILTERS_DONT_USE_HANDLER_FILTER....
		return true;
	}
	else
	{
		if( real_handler_->On_FilterNotification(not->function_to_dispatch_,
											  not->file_name1_,
											  not->file_name2_) )
		{
			return true;
		}
		else
		{
			//else  client's derived CDirectoryChangeHandler class chose 
			//		not to be notified of this file change
			return false;
		}
	}
		
	
}

bool CDelayedDirectoryChangeHandler::IncludeThisNotification(LPCTSTR file_name)
//
//	The Include filter specifies which file names we should allow to notifications
//	for... otherwise these notifications are not dispatched to the client's code.
//
//	Tests the file name to see if it matches a filter specification
//
//	RETURN VALUES:
//		
//		true : notifications for this file are to be included...
//			   notifiy the client by calling the appropriate CDirectoryChangeHandler::On_Filexxx() function.
//		false: this file is not included.... do not notifiy the client...
//
{
	ASSERT( file_name );

	if( include_filter_ == NULL ) // no filter specified, all files pass....
		return true;
	if( num_include_filter_specs_ == 1 )
	{
		return _PathMatchSpec(file_name, include_filter_)? true : false;
	}
	else
	{
		TCHAR ** tmp = (TCHAR**)include_filter_;
		for(int i(0); i < num_include_filter_specs_; ++i )
		{
			if( _PathMatchSpec(file_name, *tmp++) )
				return true;
		}
		return false;
	}

	return false;
}

bool CDelayedDirectoryChangeHandler::ExcludeThisNotification(LPCTSTR file_name)
//
//	Tests the file name to see if it matches a filter specification
//	if this function returns true, it means that this notification
//	is NOT to be passed to the client.... changes to this kind of file
//	are not
//
//	RETURN VALUES:
//		
//		true :   notifications for this file are to be filtered out(EXCLUDED)...
//				 do not notifify the client code.
//		false:   notifications for this file are NOT to be filtered out
//
//
{

	ASSERT( file_name );

	if( exclude_filter_ == NULL ) // no exclude filter... nothing is excluded...
		return false;
	if( num_exclude_filter_specs_ == 1 )
	{
		if( _PathMatchSpec(file_name, exclude_filter_) )
			return true;//exclude this notification...
		return false;
	}
	else
	{
		TCHAR ** tmp = (TCHAR**)exclude_filter_;
		for(int i(0); i < num_exclude_filter_specs_; ++i )
		{
			if( _PathMatchSpec(file_name, *tmp++) )
				return true;//exclude this one...
		}
		return false;//didn't match any exclude filters...don't exclude it
	}

	//if( exclude_filter_ == NULL //no exclude filter specified, not excluding anything....
	//||	!PathMatchSpec(file_name, exclude_filter_) )//or didn't match filter pattern.. this is not excluded...
	//{
	//	return false;
	//}
	//return true;
	
}

void CDelayedDirectoryChangeHandler::DispatchNotificationFunction(CDirChangeNotification * notification)
// ****************************************************
//	This function is called when we want the notification to execute.
// ******************************************************
{
TRACE(L"DispatchNotificationFunction\n");
	ASSERT( real_handler_ );
	ASSERT( notification );
	if( notification && real_handler_ )
	{
		//
		//	Allow the client to ignore the notification
		//
		//
		if( NotifyClientOfFileChange(notification))
		{
TRACE(L"DispatchNotificationFunction step 2\n");
			switch( notification->function_to_dispatch_ )
			{
			case CDirChangeNotification::on__file_added:
				
TRACE(L"DispatchNotificationFunction file added\n");
				real_handler_->On_FileAdded( notification->file_name1_ ); 
				break;
				
			case CDirChangeNotification::on__file_removed:
				
				real_handler_->On_FileRemoved( notification->file_name1_ );
				break;
				
			case CDirChangeNotification::on__file_name_changed:
				
				real_handler_->On_FileNameChanged( notification->file_name1_, notification->file_name2_ );
				break;
				
			case CDirChangeNotification::on__file_modified:
				
				real_handler_->On_FileModified( notification->file_name1_ );
				break;
				
			case CDirChangeNotification::on__read_directory_changes_error:
				
				real_handler_->On_ReadDirectoryChangesError( notification->error_, notification->file_name1_ );
				break;
				
			case CDirChangeNotification::on__watch_started:
		
				real_handler_->On_WatchStarted(notification->error_, notification->file_name1_);
				break;
		
			case CDirChangeNotification::on__watch_stopped:
				
				try{
					//
					//	The exception handler is just in case of the condition described in DirectoryChanges.h
					//	in the comments for On_WatchStopped()
					//
					real_handler_->On_WatchStopped(notification->file_name1_);

				}catch(...){
					MessageBeep( 0xffff );
					MessageBeep( 0xffff );
			#ifdef DEBUG
					MessageBox(NULL,_T("An RTFM Exception was raised in On_WatchStopped() -- see Comments for CDirectoryChangeHandler::On_WatchStopped() in DirectoryChanges.h."), _T("Programmer Note(DEBUG INFO):"), MB_ICONEXCLAMATION | MB_OK);
			#endif
				}
				//
				//	Signal that the On_WatchStopped() function has been dispatched.
				//
				if( watch_stopped_dispatched_event_ )
					SetEvent(watch_stopped_dispatched_event_);
				break;
			case CDirChangeNotification::function_not_defined:
			default:
				break;
			}//end switch()
		}
	}	
	if( notification )						 //		
		DisposeOfNotification(notification);// deletes or releases the notification object from memory/use
											 //
}

BOOL CDelayedDirectoryChangeHandler::WaitForOnWatchStoppedDispatched( )
//
//	When shutting down, real_handler_->On_WatchStopped() will be called.
//	Because it's possible that this object will be deleted before the notification
//	can be dispatched to the other thread, we have to wait until we know that it's been executed
//	before returning control.
//
//	This function signals that the function has been dispatched to the other
//	thread and it will be safe to delete this object once this has returned.
//
{
	ASSERT( watch_stopped_dispatched_event_ );
	DWORD wait = WAIT_FAILED;
	if( watch_stopped_dispatched_event_ )
	{

		if( app_hasGUI_ == false )
		{
			//
			//	The function will be dispatched to another thread...
			//	just wait for the event to be signalled....
			do{
				wait	= WaitForSingleObject(watch_stopped_dispatched_event_, 5000);//wait five seconds
				if( wait != WAIT_OBJECT_0 )
				{
					TRACE(_T("WARNING: Possible Deadlock detected! ThreadID: %d File: %s Line: %d\n"), GetCurrentThreadId(), _T(__FILE__), __LINE__);
				}
			}while( wait != WAIT_OBJECT_0 );
		}
		else
		{
			//
			//	Note to self:  This thread doesn't have a message Q, and therefore can't attach to 
			//	receive messages and process them... MsgWaitForMultipleObjects won't wake up for messages 
			//	unless i attach myself the the other threads input Q....
			//	just use MsgWaitForMultipleObjects() in place of WaitForSingleObject in the places where it's used...
			//
			do{
				wait = MsgWaitForMultipleObjects(1, &watch_stopped_dispatched_event_, 
												   FALSE, 5000, 
												   QS_ALLEVENTS);//wake up for all events, sent messages, posted messages etc.
				switch(wait)
				{
				case WAIT_OBJECT_0:
					{
						//
						// The event has become signalled
						//
						
					}break;
				case WAIT_OBJECT_0 + 1: 
					{
						//
						//	There is a message in this thread's queue, so 
						//	MsgWaitForMultipleObjects returned.
						//	Process those messages, and wait again.

						MSG msg;
						while( PeekMessage(&msg, NULL, 0,0, PM_REMOVE ) )
						{
							if( msg.message != WM_QUIT)
							{
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
							else
							{
								// ****
								//NOTE: putting WM_QUIT back in the Q caused problems. forget about it.
								// ****
								break;
							}
						}
					}break;
				case WAIT_TIMEOUT:
					{
						TRACE(_T("WARNING: Possible Deadlock detected! ThreadID: %d File: %s Line: %d\n"), GetCurrentThreadId(), _T(__FILE__), __LINE__);
					}break;
				}
			}while( wait != WAIT_OBJECT_0 );
			ASSERT( wait == WAIT_OBJECT_0 );
		}

	}
	else
	{
		TRACE(_T("WARNING: Unable to wait for notification that the On_WatchStopped function has been dispatched to another thread.\n"));
		TRACE(_T("An Exception may occur shortly.\n"));
		TRACE(_T("File: %s Line: %d"), _T( __FILE__ ), __LINE__);
	
	}


	return (wait == WAIT_OBJECT_0 );
}

void CDelayedDirectoryChangeHandler::SetChangedDirectoryName(const CString & changed_dir_name)
{
	ASSERT( real_handler_ );
	CDirectoryChangeHandler::SetChangedDirectoryName(changed_dir_name);
	if( real_handler_ )
		real_handler_->SetChangedDirectoryName( changed_dir_name );
}
const CString & CDelayedDirectoryChangeHandler::GetChangedDirectoryName() const
{
	if( real_handler_ )
		return real_handler_->GetChangedDirectoryName();
	return CDirectoryChangeHandler::GetChangedDirectoryName();
}

*/
