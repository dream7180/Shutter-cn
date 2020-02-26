/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

/*

// DelayedDirectoryChangeHandler.h: interface for the CDelayedDirectoryChangeHandler2 class.
//
//////////////////////////////////////////////////////////////////////
//
//	You needn't worry about the classes in this file.
//	they are implementation classes used to help CDirectoryChangeWatcher work.
//
//

#if !defined(AFX_DELAYEDDIRECTORYCHANGEHANDLER_H__F20EC22B_1C79_403E_B43C_938F95723D45__INCLUDED_)
#define AFX_DELAYEDDIRECTORYCHANGEHANDLER_H__F20EC22B_1C79_403E_B43C_938F95723D45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//classes declrared in other files:
class CDirectoryChangeWatcher;
class CDirectoryChangeHandler;
//classes declared in this file:
class CDirChangeNotification;
class CDelayedDirectoryChangeHandler;

class CDelayedNotificationWindow;
class CDelayedNotificationThread;

// *******************************************************************
	//The classes in this file implement methods to ensure that file change 
	//notifications are fired in a thread other than the worker thread used
	//by CDirectoryChangeWatcher.

	//Dispatching the notifications in to a different thread improves the performance
	//of CDirectoryChangeWatcher so that it can process more notifications faster
	//and notifications aren't 'lost'.


	//There are two methods of dispatching functions to another thread.

	//	1)  One is to use the message pump associated w/ the main thread by posting notifications
	//		to a hidden window. This is implemented w/ the class CDelayedNotificationWindow.

	//	2)	The other is to create a worker thread that implements a message pump.  This is 
	//		implemented w/ the class CDelayedNotificationThread.


	//If your app uses a GUI then it has a already has message pump.  
	//You can make sure that CDelayedNotificationWindow is used in this case.
	//The advantage here being that there is one less worker thread used in your program.

	//If your app is a command line app or otherwise doesn't have a GUI,
	//then you will want to make sure that you are using the CDelayedNotificationThread
	//to dispatch notifications to another thread.

	//This is determined by a flag passed is passed to the constructor of CDirecotryChangeWatcher

//********************************************************************

class CDelayedNotifier
//
//	Abstract base class for ensuring notifications are fired in a thread 
//
//
{
public:
	virtual ~CDelayedNotifier(){}
	virtual void PostNotification(CDirChangeNotification * notification) = 0;

};

class CDelayedNotificationWindow : public CDelayedNotifier
//
//	A class that implements a
//	there will always be only one of the actual windows 
//	in existance. 
//
{
public:
		CDelayedNotificationWindow(){  AddRef(); }
		virtual ~CDelayedNotificationWindow(){ Release(); }
		

		void PostNotification(CDirChangeNotification * notification);
private:
		long AddRef();		//	the window handle is reference counted
		long Release();		//

		static long ref_cnt_;
		static HWND s_hWnd; //there's only one window no matter how many instances of this class there are.... this means that all notifications are handled by the same thread.
		static BOOL register_window_;
		BOOL RegisterWindowClass(LPCTSTR class_name);
		BOOL CreateNotificationWindow();
};

class CDelayedNotificationThread : public CDelayedNotifier
//
//	Class that implements a worker thread w/ a message pump.
//	CDirectoryChangeWatcher posts notifications to this thread, where they are dispatched.
//	This thread executes CDirectoryChangeHandler notifications.
//
{
public:
	CDelayedNotificationThread()
		:thread_start_event_(NULL)
	{ 
		thread_start_event_ = CreateEvent(NULL,FALSE,FALSE,NULL);
		ASSERT( thread_start_event_ );
		AddRef(); 
	}
	virtual ~CDelayedNotificationThread()
	{ 
		Release(); 
		if( thread_start_event_ )
			CloseHandle(thread_start_event_), thread_start_event_ = NULL;
	}

	void PostNotification(CDirChangeNotification * notification);

private:
	long AddRef();					// The thread handle is reference
	long Release();					// counted so that only one thread is used
									// so that there's only one worker thread(performing this functino)
	static long		ref_cnt_;		// no matter how many directories are being watched
	static HANDLE	thread_;		//	
	static DWORD	thread_id_;	//  
										
	static UINT __stdcall ThreadFunc(LPVOID lpvThis);

	bool StartThread();
	bool StopThread();

	BOOL WaitForThreadStartup(){ return WaitForSingleObject(thread_start_event_, INFINITE) == WAIT_OBJECT_0; };
	BOOL SignalThreadStartup(){ return SetEvent( thread_start_event_ ) ; }

	HANDLE thread_start_event_;//signals that the worker thread has started. this fixes a bug condition.
		
};


class CDirChangeNotification
//
//	 A class to help dispatch the change notifications to the main thread.
//
//	 This class holds the data in memory until the notification can be dispatched.(ie: this is the time between when the notification is posted, and the clients notification code is called).
//
//
{
private:
	CDirChangeNotification();//not implemented
public:
	explicit CDirChangeNotification(CDelayedDirectoryChangeHandler * delayed_handler, DWORD partial_path_offset);
	~CDirChangeNotification();

	//
	//
	void PostOn_FileAdded(LPCTSTR file_name);
	void PostOn_FileRemoved(LPCTSTR file_name);
	void PostOn_FileNameChanged(LPCTSTR old_name, LPCTSTR new_name);
	void PostOn_FileModified(LPCTSTR file_name);
	void PostOn_ReadDirectoryChangesError(DWORD error, LPCTSTR directory_name);
	void PostOn_WatchStarted(DWORD error, LPCTSTR directory_name);
	void PostOn_WatchStopped(LPCTSTR directory_name);

	void DispatchNotificationFunction();


	enum function_to_dispatch{	function_not_defined = -1,
								on__file_added		= FILE_ACTION_ADDED, 
								on__file_removed		= FILE_ACTION_REMOVED, 
								on__file_modified	= FILE_ACTION_MODIFIED,
								on__file_name_changed	= FILE_ACTION_RENAMED_OLD_NAME,
								on__read_directory_changes_error,
								on__watch_started,
								on__watch_stopped
	};	
protected:
	void PostNotification();
	
private:
	friend class CDelayedDirectoryChangeHandler;
	CDelayedDirectoryChangeHandler * delayed_handler_;

	//
	//	Members to help implement DispatchNotificationFunction
	//
	//

	function_to_dispatch function_to_dispatch_;
	//Notification Data:
	TCHAR *	file_name1_;//<-- is the file_name parameter to On_FileAdded(),On_FileRemoved,On_FileModified(), and is old_file_name to On_FileNameChanged(). Is also directory_name to On_ReadDirectoryChangesError(), On_WatchStarted(), and On_WatchStopped()
	TCHAR *	file_name2_;//<-- is the new_file_name parameter to On_FileNameChanged()
	DWORD error_;	  //<-- is the error parameter to On_WatchStarted(), and On_ReadDirectoryChangesError()
	//

	DWORD partial_path_offset_;//helps support FILTERS_CHECK_PARTIAL_PATH...not passed to any functions other than may be used during tests in CDelayedDirectoryChangeHandler::NotifyClientOfFileChange()


	friend class CDirChangeNotification;
	friend class CDirectoryChangeWatcher;
	friend DWORD GetPathOffsetBasedOnFilterFlags(CDirChangeNotification*,DWORD);//a friend function
};


//////////////////////////////////////////////////////////////////////////
//
//	This class makes it so that a file change notification is executed in the
//	context of the main thread, and not the worker thread.
//
//
//	It works by creating a hidden window.  When it receieves a notification
//	via one of the On_Filexxx() functions, a message is posted to this window.
//	when the message is handled, the notification is fired again in the context
//	of the main thread, or whichever thread that called CDirectoryChangeWatcher::WatchDirectory()
//
//
/////////////////////////////////////////////////////////////////////////////
//	Note this code wants to use PathMatchSpec()
//	which is only supported on WINNT 4.0 w/ Internet Explorer 4.0 and above.
//	PathMatchSpec is fully supported on Win2000/XP.
//
//	For the case of WINNT 4.0 w/out IE 4.0, we'll use a simpler function.
//	some functionality is lost, but such is the price.
//

typedef BOOL (STDAPICALLTYPE * FUNC_PatternMatchSpec)(LPCTSTR file, LPCTSTR spec);

class CDelayedDirectoryChangeHandler : public CDirectoryChangeHandler
//
//	Decorates an instance of a CDirectoryChangeHandler object.
//	Intercepts notification function calls and posts them to 
//	another thread through a method implemented by a class derived from 
//	CDelayedNotifier
//	
//
//	This class implements dispatching the notifications to a thread
//	other than CDirectoryChangeWatcher::MonitorDirectoryChanges()
//
//	Also supports the include and exclude filters for each directory
//
{
private:
	CDelayedDirectoryChangeHandler();//not implemented.
public:
	CDelayedDirectoryChangeHandler( CDirectoryChangeHandler * real_handler, bool app_hasGUI, LPCTSTR include_filter, LPCTSTR exclude_filter, DWORD filter_flags);
	virtual ~CDelayedDirectoryChangeHandler();

	
	CDirectoryChangeHandler * GetRealChangeHandler()const { return real_handler_; }
	CDirectoryChangeHandler * & GetRealChangeHandler(){ return real_handler_; }//FYI: PCLint will give a warning that this exposes a private/protected member& defeats encapsulation.  

	void PostNotification(CDirChangeNotification * notification);
	void DispatchNotificationFunction(CDirChangeNotification * notification);


protected:
	//These functions are called when the directory to watch has had a change made to it
	void On_FileAdded(const CString & file_name);
	void On_FileRemoved(const CString & file_name);
	void On_FileModified(const CString & file_name);
	void On_FileNameChanged(const CString & old_file_name, const CString & new_file_name);
	void On_ReadDirectoryChangesError(DWORD error, const CString & directory_name);

	void On_WatchStarted(DWORD error, const CString & directory_name);
	void On_WatchStopped(const CString & directory_name);
	

	void SetChangedDirectoryName(const CString & changed_dir_name);
	const CString & GetChangedDirectoryName()const;

	BOOL WaitForOnWatchStoppedDispatched();//see comments in .cpp


	bool NotifyClientOfFileChange(CDirChangeNotification * not);

	bool IncludeThisNotification(LPCTSTR file_name);	//	based on file name.
	bool ExcludeThisNotification(LPCTSTR file_name);	//	Allows us to filter notifications
														//
	
	

	CDirChangeNotification * GetNotificationObject();
	void DisposeOfNotification(CDirChangeNotification * notification);

	CDelayedNotifier * delay_notifier_;
	CDirectoryChangeHandler * real_handler_;	

						// app_hasGUI_: 
						//   This flag, if set to true, indicates that the app has a message
	bool app_hasGUI_;	//	 pump, and that functions are dispatched to the main thread.
						//   Otherwise, functions are dispatched to a separate worker thread.
						//
	DWORD filter_flags_;

	DWORD partial_path_offset_; //helps support FILTERS_CHECK_PARTIAL_PATH
	void SetPartialPathOffset(const CString & watched_dir_name);

	friend class CDirectoryChangeWatcher;
//	friend class CDirectoryChangeWatcher::CDirWatchInfo;

private:
	HANDLE watch_stopped_dispatched_event_;//supports WaitForOnWatchStoppedDispatched()

	TCHAR * include_filter_;		//	Supports the include
	TCHAR * exclude_filter_;		//	& exclude filters

	//
	//	Load PathMatchSpec dynamically because it's only supported if IE 4.0 or greater is
	//	installed.
	static HMODULE shlwapi_dll_;//for the PathMatchSpec() function
	static BOOL shlwapi_dll_exists_;//if on NT4.0 w/out IE 4.0 or greater, this'll be false
	static long ref_cnt_h_shlwapi_;
	static FUNC_PatternMatchSpec fp_pattern_match_spec_;

	BOOL _PathMatchSpec(LPCTSTR path, LPCTSTR pattern);
	BOOL InitializePathMatchFunc(LPCTSTR include_filter, LPCTSTR exclude_filter);
	BOOL InitializePatterns(LPCTSTR include_filter, LPCTSTR exclude_filter);
	void UninitializePathMatchFunc();

	bool UsesRealPathMatchSpec() const;//are we using PathMatchSpec() or wildcmp()?

	//note: if the PathMatchSpec function isn't found, wildcmp() is used instead.
	//
	//	to support multiple file specs separated by a semi-colon,
	//	the include and exclude filters that are passed into the 
	//	the constructor are parsed into separate strings
	//	which are all checked in a loop.
	//
	int num_include_filter_specs_;
	int num_exclude_filter_specs_;


};




#endif // !defined(AFX_DELAYEDDIRECTORYCHANGEHANDLER_H__F20EC22B_1C79_403E_B43C_938F95723D45__INCLUDED_)

*/
