// DirectoryChanges.cpp: implementation of the CDirectoryChangeWatcher and CDirectoryChangeHandler classes.
//
///////////////////////////////////////////////////////////////////
///

//***********************************************************

//  Author:	Wes Jones wesj@hotmail.com
//
//  File:		DirectoryChanges.cpp
//
//  Latest Changes:
//
//		11/22/2001	--	Fixed bug causing file name's to be truncated if
//						longer than 130 characters. Fixed CFileNotifyInformation::GetFileName()
//						Thanks to Edric(uo_edric@hotmail.com) for pointing this out.
//
//						Added code to enable process privileges when CDirectoryChangeWatcher::WatchDirectory() 
//						is first called.	See docuementation API for ReadDirectoryChangesW() for more information of required privileges.
//						
//						  Currently enables these privileges: (11/22/2001)
//							SE_BACKUP_NAME
//							SE_CHANGE_NOTIFY_NAME 
//							SE_RESTORE_NAME(02/09/2002)
//						Implemented w/ helper class CPrivilegeEnabler.
//
//		11/23/2001		Added classes so that all notifications are handled by the
//						same thread that called CDirectoryChangeWatcher::WatchDirectory(),
//						ie: the main thread.
//						CDirectoryChangeHandler::On_Filexxxx() functions are now called in the 
//						context of the main thread instead of the worker thread.
//
//						This is good for two reasons:
//						1: The worker thread spends less time handling each notification.
//							The worker thread simply passes the notification to the main thread,
//							which does the processing.
//							This means that each file change notification is handled as fast as possible
//							and ReadDirectoryChangesW() can be called again to receive more notifications
//							faster.
//
//						2:	This makes it easier to make an ActiveX or ATL object with this class
//							because the events that are fired, fire in the context of the main thread.
//							The fact that I'm using a worker thread w/ this class is totally 
//							transparent to a client user.
//							If I decide to turn this app into an ActiveX or ATL object
//							I don't have to worry about wierd COM rules and multithreading issues,
//							and neither does the client, be the client a VB app, C++ app, Delphi app, or whatever.
//
//						Implemented with CDelayedDirectoryChangeHandler in DirectoryChangeHandler.h/.cpp
//
//		02/06/2002	  Fixed a bug that would cause an application to hang.
//						If ReadDirectoryChangesW was to fail during normal operation,
//						the short story is that the application would hang
//						when it called CDirectoryChangeWatcher::UnwatchDirectory(const CString &)
//
//						One way to reproduce this behavior on the old code
//						is to watch a directory using a UNC path, and then change the IP
//						address of that machine while the watch was running.  Exitting 
//						the app after this would cause it to hang.
//
//						Steps to reproduce it:
//
//						1) Assume that the computer running the code is
//							named 'ThisComputer' and there is a shared folder named 'FolderName'
//
//
//						2) Start a watch on a folder using a UNC path:  ie: \\ThisComputer\FolderName
//						
//						  eg:	CDirectoryChangeWatcher	watcher;
//							 
//								watcher.WatchDirectory(_T("\\\\ThisComputer\\FolderName",/ * other parameters * /)
//							
//						3)  Change the IP address of 'ThisComputer'
//
//							   ** ReadDirectoryChangesW() will fail some point after this.
//						
//		
//						4)  Exit the application... it may hang.
//
//
//						Anyways, that's what the bug fix is for.
//
//
//		02/06/2002	New side effects for CDirectoryChangeHandler::On_ReadDirectoryChangeError()
//
//					If CDirectoryChangeHandler::On_ReadDirectoryChangeError() is ever executed
//					the directory that you are watching will have been unwatched automatically due
//					to the error condition.
//
//					A call to CDirectoryChangeWatcher::IsWatchingDirectory() will fail because the directory
//					is no longer being watched. You'll need to re-watch that directory.
//
//		02/09/2002	Added a parameter to CDirectoryChangeHandler::On_ReadDirectoryChangeError()
//
//					Added the parameter: const CString & directory_name
//					The new signature is now:
//							virtual void CDirectoryChangeHandler::On_ReadDirectoryChangeError(DWORD error, const CString & directory_name);
//
//					This new parameter gives you the name of the directory that the error occurred on.
//
//		04/25/2002  Provided a way to get around the requirement of a message pump.
//					A console app can now use this w/out problems by passing false
//					to the constructor of CDirectoryChangeWatcher. 
//					An app w/ a message pump can also pass false if it so desires w/out problems.
//
//		04/25/2002	Added two new virtual functions to CDirectoryChangeHandler
//
//					Added:
//							On_WatchStarted(DWORD error, const CString & directory_name)
//							On_WatchStopped(const CString & directory_name);
//					See header file for details.
//
//		04/27/2002	Added new function to CDirectoryChangeHandler:
//
//					Added virtual bool On_FilterNotification(DWORD notify_action, LPCTSTR file_name, LPCTSTR new_file_name)
//
//					This function is called before any notification function, and allows the 
//					CDirectoryChangeHandler derived class to ignore file notifications 
//					by performing a programmer defined test.
//					By ignore, i mean that 
//						On_FileAdded(), On_FileRemoved(), On_FileModified(), or On_FileNameChanged()
//						will NOT be called if this function returns false.
//
//					The default implementation always returns true, signifying that ALL notifications
//					are to be called.
//
//
//		04/27/2002  Added new Parameters to CDirectoryChangeWatcher::WatchDirectory()
//
//					The new parameters are: 
//										LPCTSTR include_filter 
//										LPCTSTR exclude_filter
//					Both parameters are defaulted to NULL.
//					Signature is now:
//					CDirectoryChangeWatcher::WatchDirectory(const CString & dir_to_watch, 
//															DWORD changes_to_watch_for, 
//															CDirectoryChangeHandler * change_handler, 
//															BOOL watch_sub_dirs = FALSE,
//															LPCTSTR include_filter = NULL,
//															LPCTSTR exclude_filter = NULL)
//
//		04/27/2002	Added support for include and exclude filters.
//					These filters allow you to receive notifications for just the files you
//					want... ie: you can specify that you only want to receive notifications
//					for changes to "*.txt" files or some other such file filter.
//
//					NOTE: This feature is implemented w/ the PathMatchSpec() api function
//					which is only available on NT4.0 if Internet Explorer 4.0 or above is installed.
//					See MSDN for PathMatchSpec().  Win2000, and XP do not have to worry about it.
//
//					Filter specifications:
//					   Accepts wild card characters * and ?, just as you're used to for the DOS dir command.
//					   It is possible to specify multiple extenstions in the filter by separating each filter spec
//					   with a semi-colon. 
//					   eg: "*.txt;*.tmp;*.log"  <-- this filter specifies all .txt, .tmp, & .log files
//
//					
//
//					Filters are passed as parameters to CDirectoryChangeWatcher::WatchDirectory()
//
//					NOTE: The filters you specify take precedence over CDirectoryChangeHandler::On_FilterNotification().
//						  This means that if the file name does not pass the filters that you specify
//						  when the watch is started, On_FilterNotification() will not be called.
//						  Filter specifications are case insensitive, ie: ".tmp" and ".TMP" are the same
//
//
//					FILTER TYPES:
//							Include Filter:
//										If you specify an include filter, you are specifying that you
//										only want to receive notifications for specific file types.
//										eg: "*.log" means that you only want notifications for changes 
//											to files w/ an exention of ".log". 
//											Changes to ALL OTHER other file types are ignored.
//										An empty, or not specified include filter means that you want
//										notifications for changes of ALL file types.
//							Exclude filter:
//
//										If you specify an exclude filter, you are specifying that
//										you do not wish to receive notifications for a specific type of file or files.
//										eg: "*.tmp" would mean that you do not want any notifications 
//											regarding files that end w/ ".tmp"
//										An empty, or not specified exclude filter means that
//										you do not wish to exclude any file notifications.
//
//
//
//
//
//		
//					
//************************************************************

#include "stdafx.h"

/*

#include "DirectoryChanges.h"
#include "DelayedDirectoryChangeHandler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//
//
//	Fwd Declares & #define's
//
//
//
// Helper classes
class CPrivilegeEnabler;	 //for use w/ enabling process priveledges when this code is first used. It's a singleton.

class CFileNotifyInformation;//helps CDirectoryChangeWatcher class notify CDirectoryChangeHandler class of changes to files. 

class CDelayedDirectoryChangeHandler;	//	Helps all notifications become handled by the main
										//	thread, or a worker thread depending upon a value passed to the 
										//	constructor of CDirectoryChangeWatcher.
										//	
										//	For notifications to fire in the main thread, your app must have a message pump.
										//
										//  The 'main' thread is the one that called CDirectoryChangeWatcher::WatchDirectory()

// Helper functions
static BOOL	EnablePrivilege(LPCTSTR priv_name, BOOL enable = TRUE);
static bool IsDirectory(const CString & path);
/////////////////////////////////////////////////////////////////////
//	Helper functions.
BOOL	EnablePrivilege(LPCTSTR priv_name, BOOL enable) 
//
//	I think this code is from a Jeffrey Richter book...
//
//	Enables user priviledges to be set for this process.
//	
//	Process needs to have access to certain priviledges in order
//	to use the ReadDirectoryChangesW() API.  See documentation.
{    
	BOOL ok = FALSE;    
	// Assume function fails    
	HANDLE token;    
	// Try to open this process's access token    
	if (OpenProcessToken(GetCurrentProcess(), 		
					TOKEN_ADJUST_PRIVILEGES, &token)) 	
	{        
		// privilege        
		TOKEN_PRIVILEGES tp = { 1 };        

		if (LookupPrivilegeValue(NULL, priv_name,  &tp.Privileges[0].Luid))
		{
			tp.Privileges[0].Attributes = enable ?  SE_PRIVILEGE_ENABLED : 0;

			AdjustTokenPrivileges(token, FALSE, &tp, 			      
									sizeof(tp), NULL, NULL);

			ok = (GetLastError() == ERROR_SUCCESS);		
		}
		CloseHandle(token);	
	}	
	return(ok);
}

static bool IsDirectory(const CString & path)
//
//  Returns: bool
//		true if path is a path to a directory
//		false otherwise.
//
{
	DWORD attrib	= GetFileAttributes(path);
	return static_cast<bool>((attrib != 0xffffffff 
							&&	(attrib & FILE_ATTRIBUTE_DIRECTORY)));

		
}
///////////////////////////////////////////////
//Helper class:

class CFileNotifyInformation 
///*******************************
//
//A Class to more easily traverse the FILE_NOTIFY_INFORMATION records returned 
//by ReadDirectoryChangesW().
//
//FILE_NOTIFY_INFORMATION is defined in Winnt.h as: 
//
// typedef struct _FILE_NOTIFY_INFORMATION {
//    DWORD NextEntryOffset;
//	DWORD Action;
//    DWORD FileNameLength;
//    WCHAR FileName[1];
//} FILE_NOTIFY_INFORMATION, *PFILE_NOTIFY_INFORMATION;	
//
//  ReadDirectoryChangesW basically puts x amount of these records in a 
//  buffer that you specify.
//  The FILE_NOTIFY_INFORMATION structure is a 'dynamically sized' structure (size depends on length
//  of the file name (+ sizeof the DWORDs in the struct))
//
//  Because each structure contains an offset to the 'next' file notification
//  it is basically a singly linked list.  This class treats the structure in that way.
//  
//
//  Sample Usage:
//  BYTE Read_Buffer[ 4096 ];
//
//  ...
//  ReadDirectoryChangesW(...Read_Buffer, 4096,...);
//  ...
//
//  CFileNotifyInformation notify_info(Read_Buffer, 4096);
//  do{
//	    switch(notify_info.GetAction())
//		{
//		case xx:
//		    notify_info.GetFileName();
//		}
//
//  while(notify_info.GetNextNotifyInformation());
//  
//********************************
{
public:
	CFileNotifyInformation(BYTE * file_notify_info_buffer, DWORD buff_size)
	: buffer_(file_notify_info_buffer),
	  buffer_size_(buff_size)
	{
		ASSERT(file_notify_info_buffer && buff_size);
		
		current_record_ = (PFILE_NOTIFY_INFORMATION) buffer_;
	}

	
	BOOL GetNextNotifyInformation();
	
	BOOL CopyCurrentRecordToBeginningOfBuffer(OUT DWORD & ref_dwSizeOfCurrentRecord);

	DWORD	GetAction() const;//gets the type of file change notifiation
	CString GetFileName()const;//gets the file name from the FILE_NOTIFY_INFORMATION record
	CString GetFileNameWithPath(const CString & root_path) const;//same as GetFileName() only it prefixes the root_path into the file name

	
protected:
	BYTE * buffer_;//<--all of the FILE_NOTIFY_INFORMATION records 'live' in the buffer this points to...
	DWORD  buffer_size_;
	PFILE_NOTIFY_INFORMATION current_record_;//this points to the current FILE_NOTIFY_INFORMATION record in buffer_
	
};

BOOL CFileNotifyInformation::GetNextNotifyInformation()
///***************
//  Sets the current_record_ to the next FILE_NOTIFY_INFORMATION record.
//
//  Even if this return FALSE, (unless current_record_ is NULL)
//  current_record_ will still point to the last record in the buffer.
//****************
{
	if (current_record_ 
	&&	current_record_->NextEntryOffset != 0UL)//is there another record after this one?
	{
		//set the current record to point to the 'next' record
		PFILE_NOTIFY_INFORMATION old = current_record_;
		current_record_ = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)current_record_ + current_record_->NextEntryOffset);

		ASSERT((DWORD)((BYTE*)current_record_ - buffer_) < buffer_size_);//make sure we haven't gone too far

		if ((DWORD)((BYTE*)current_record_ - buffer_) > buffer_size_)
		{
			//we've gone too far.... this data is hosed.
			//
			// This sometimes happens if the watched directory becomes deleted... remove the FILE_SHARE_DELETE flag when using CreateFile() to get the handle to the directory...
			current_record_ = old;
		}
					
		return (BOOL)(current_record_ != old);
	}
	return FALSE;
}

BOOL CFileNotifyInformation::CopyCurrentRecordToBeginningOfBuffer(OUT DWORD & ref_dwSizeOfCurrentRecord)
//*****************************************
//   Copies the FILE_NOTIFY_INFORMATION record to the beginning of the buffer
//   specified in the constructor.
//
//   The size of the current record is returned in DWORD & size_of_current_record.
//   
//*****************************************
{
	ASSERT(buffer_ && current_record_);
	if (!current_record_) return FALSE;

	BOOL ret_val = TRUE;

	//determine the size of the current record.
	ref_dwSizeOfCurrentRecord = sizeof(FILE_NOTIFY_INFORMATION);
	//subtract out sizeof FILE_NOTIFY_INFORMATION::FileName[1]
	WCHAR FileName[1];//same as is defined for FILE_NOTIFY_INFORMATION::FileName
	UNREFERENCED_PARAMETER(FileName);
	ref_dwSizeOfCurrentRecord -= sizeof(FileName);   
	//and replace it w/ value of FILE_NOTIFY_INFORMATION::FileNameLength
	ref_dwSizeOfCurrentRecord += current_record_->FileNameLength;

	ASSERT((DWORD)((LPBYTE)current_record_ + ref_dwSizeOfCurrentRecord) <= buffer_size_);

	ASSERT((void*)buffer_ != (void*)current_record_);//if this is the case, your buffer is way too small
	if ((void*)buffer_ != (void*) current_record_)
	{//copy the current_record_ to the beginning of buffer_
		
		ASSERT((DWORD)current_record_ > (DWORD)buffer_ + ref_dwSizeOfCurrentRecord);//will it overlap?
		__try{
			memcpy(buffer_, current_record_, ref_dwSizeOfCurrentRecord);
			ret_val = TRUE;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			TRACE(_T("EXCEPTION!  CFileNotifyInformation::CopyCurrentRecordToBeginningOfBuffer() -- probably because bytes overlapped in a call to memcpy()"));
			ret_val = FALSE;
		}
	}
	//else
	//there was only one record in this buffer, and current_record_ is already at the beginning of the buffer
	return ret_val;
}

DWORD CFileNotifyInformation::GetAction() const
{ 
	ASSERT(current_record_);
	if (current_record_)
		return current_record_->Action;
	return 0UL;
}

CString CFileNotifyInformation::GetFileName() const
{
	//
	//BUG FIX:
	//		File Name's longer than 130 characters are truncated
	//
	//		Thanks Edric @ uo_edric@hotmail.com for pointing this out.
	if (current_record_)
	{
		//WCHAR wcFileName[ MAX_PATH + 1] = {0};//L"";
		//memcpy(	wcFileName, 
		//		current_record_->FileName, 
		//		//min(MAX_PATH, current_record_->FileNameLength) <-- buggy line
		//		min<size_t>((MAX_PATH * sizeof(WCHAR)), current_record_->FileNameLength));

		return CString(current_record_->FileName, current_record_->FileNameLength / sizeof(WCHAR));
	}
	return CString();
}		

static inline bool HasTrailingBackslash(const CString & str)
{
	if (str.GetLength() > 0 
	&&	str[ str.GetLength() - 1 ] == _T('\\'))
		return true;
	return false;
}
CString CFileNotifyInformation::GetFileNameWithPath(const CString & root_path) const
{
	CString file_name(root_path);
	//if (file_name.Right(1) != _T("\\"))
	if (!HasTrailingBackslash(root_path))
		file_name += _T("\\");

	file_name += GetFileName();
	return file_name;
}
/////////////////////////////////////////////////////////////////////////////////
class CPrivilegeEnabler
//
//	Enables privileges for this process
//	first time CDirectoryChangeWatcher::WatchDirectory() is called.
//
//	It's a singleton.
//
{
private:
	CPrivilegeEnabler();//ctor
public:
	~CPrivilegeEnabler(){};
	
	static CPrivilegeEnabler & Instance();
	//friend static CPrivilegeEnabler & Instance();
};

CPrivilegeEnabler::CPrivilegeEnabler()
{
	LPCTSTR arPrivelegeNames[]	=	{
										SE_BACKUP_NAME, //	these two are required for the FILE_FLAG_BACKUP_SEMANTICS flag used in the call to 
										SE_RESTORE_NAME,//  CreateFile() to open the directory handle for ReadDirectoryChangesW

										SE_CHANGE_NOTIFY_NAME //just to make sure...it's on by default for all users.
										//<others here as needed>
									};
	for (int i = 0; i < sizeof(arPrivelegeNames) / sizeof(arPrivelegeNames[0]); ++i)
	{
		if (!EnablePrivilege(arPrivelegeNames[i], TRUE))
		{
			TRACE(_T("Unable to enable privilege: %s	--	GetLastError(): %d\n"), arPrivelegeNames[i], GetLastError());
			TRACE(_T("CDirectoryChangeWatcher notifications may not work as intended due to insufficient access rights/process privileges.\n"));
			TRACE(_T("File: %s Line: %d\n"), _T(__FILE__), __LINE__);
		}
	}
}

CPrivilegeEnabler & CPrivilegeEnabler::Instance()
{
	static CPrivilegeEnabler theInstance;//constructs this first time it's called.
	return theInstance;
}
//
//
//
///////////////////////////////////////////////////////////

	
//
//
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDirectoryChangeHandler::CDirectoryChangeHandler()
: ref_cnt_(1),
  dir_change_watcher_(NULL),
  watcher_ref_cnt_(0L)
{
}

CDirectoryChangeHandler::~CDirectoryChangeHandler()
{
	UnwatchDirectory();
}

long CDirectoryChangeHandler::AddRef()
{ 
	return InterlockedIncrement(&ref_cnt_);	
}

long CDirectoryChangeHandler::Release()
{  
	long ref_cnt = -1;
	if ((ref_cnt = InterlockedDecrement(&ref_cnt_)) == 0)
		delete this;
	return ref_cnt;
}
long CDirectoryChangeHandler::CurRefCnt()const 
{ 
	return ref_cnt_;
}

BOOL CDirectoryChangeHandler::UnwatchDirectory()
{
	CSingleLock lock(&cs_watcher_, TRUE);	
	ASSERT(lock.IsLocked());
	
	if (dir_change_watcher_)
		return dir_change_watcher_->UnwatchDirectory(this);
	return TRUE;
}

long  CDirectoryChangeHandler::ReferencesWatcher(CDirectoryChangeWatcher * dir_change_watcher)
{
	ASSERT(dir_change_watcher);
	CSingleLock lock(&cs_watcher_, TRUE);
	if (dir_change_watcher_ 
	&&  dir_change_watcher_ != dir_change_watcher)
	{
		TRACE(_T("CDirectoryChangeHandler...is becoming used by a different CDirectoryChangeWatcher!\n"));
		TRACE(_T("Directories being handled by this object will now be unwatched.\nThis object is now being used to ")
			  _T("handle changes to a directory watched by different CDirectoryChangeWatcher object, probably on a different directory"));
		
		if (UnwatchDirectory())
		{
			dir_change_watcher_ = dir_change_watcher;
			watcher_ref_cnt_ = 1; //when this reaches 0, set dir_change_watcher_ to NULL
			return watcher_ref_cnt_;
		}
		else
		{
			ASSERT(FALSE);//shouldn't get here!
		}
	}
	else
	{
		ASSERT(!dir_change_watcher_ || dir_change_watcher_ == dir_change_watcher);
		
		dir_change_watcher_ = dir_change_watcher;	
		
		if (dir_change_watcher_)
			return InterlockedIncrement(&watcher_ref_cnt_);
		
	}
	return watcher_ref_cnt_;
}

long CDirectoryChangeHandler::ReleaseReferenceToWatcher(CDirectoryChangeWatcher * dir_change_watcher)
{
	ASSERT(dir_change_watcher_ == dir_change_watcher);
	CSingleLock lock(&cs_watcher_, TRUE);
	long ref;
	if ((ref = InterlockedDecrement(&watcher_ref_cnt_)) <= 0L)
	{
		dir_change_watcher_ = NULL; //Setting this to NULL so that this->UnwatchDirectory() which is called in the dtor
									//won't call dir_change_watcher_->UnwatchDirecotry(this).
									//dir_change_watcher_ may point to a destructed object depending on how
									//these classes are being used.
		watcher_ref_cnt_ = 0L;
	}
	return ref;
}

//
//
//	Default implmentations for CDirectoryChangeHandler's virtual functions.
//
//
void CDirectoryChangeHandler::On_FileAdded(const CString & file_name)
{ 
	TRACE(_T("The following file was added: %s\n"), file_name);
}

void CDirectoryChangeHandler::On_FileRemoved(const CString & file_name)
{
	TRACE(_T("The following file was removed: %s\n"), file_name);
}

void CDirectoryChangeHandler::On_FileModified(const CString & file_name)
{
	TRACE(_T("The following file was modified: %s\n"), file_name);
}

void CDirectoryChangeHandler::On_FileNameChanged(const CString & old_file_name, const CString & new_file_name)
{
	TRACE(_T("The file %s was RENAMED to %s\n"), old_file_name, new_file_name);
}
void CDirectoryChangeHandler::On_ReadDirectoryChangesError(DWORD error, const CString & directory_name)
{
	TRACE(_T("WARNING!!!!!\n"));
	TRACE(_T("An error has occurred on a watched directory!\n"));
	TRACE(_T("This directory has become unwatched! -- %s \n"), directory_name);
	TRACE(_T("ReadDirectoryChangesW has failed! %d"), error);
	ASSERT(FALSE);//you should override this function no matter what. an error will occur someday.
}

void CDirectoryChangeHandler::On_WatchStarted(DWORD error, const CString & directory_name)
{	
	if (error == 0)
		TRACE(_T("A watch has begun on the following directory: %s\n"), directory_name);
	else
		TRACE(_T("A watch failed to start on the following directory: (Error: %d) %s\n"),error, directory_name);
}

void CDirectoryChangeHandler::On_WatchStopped(const CString & directory_name)
{
	TRACE(_T("The watch on the following directory has stopped: %s\n"), directory_name);
}

bool CDirectoryChangeHandler::On_FilterNotification(DWORD *notify_action*, LPCTSTR *file_name*, LPCTSTR *new_file_name*)
//
//	bool On_FilterNotification(DWORD notify_action, LPCTSTR file_name, LPCTSTR new_file_name);
//
//	This function gives your class a chance to filter unwanted notifications.
//
//	PARAMETERS: 
//			DWORD	notify_action	-- specifies the event to filter
//			LPCTSTR file_name		-- specifies the name of the file for the event.
//			LPCTSTR new_file_name	-- specifies the new file name of a file that has been renamed.
//
//	RETURN VALUE:
//			return true from this function, and you will receive the notification.
//			return false from this function, and your class will NOT receive the notification.
//
//	Valid values of notify_action:
//		FILE_ACTION_ADDED			-- On_FileAdded() is about to be called.
//		FILE_ACTION_REMOVED			-- On_FileRemoved() is about to be called.
//		FILE_ACTION_MODIFIED		-- On_FileModified() is about to be called.
//		FILE_ACTION_RENAMED_OLD_NAME-- On_FileNameChanged() is about to be call.
//
//	  
//	NOTE:  When the value of notify_action is FILE_ACTION_RENAMED_OLD_NAME,
//			file_name will be the old name of the file, and new_file_name will
//			be the new name of the renamed file.
//
//  The default implementation always returns true, indicating that all notifications will 
//	be sent.
//	
{
	return true;
}

void CDirectoryChangeHandler::SetChangedDirectoryName(const CString & changed_dir_name)
{
	changed_directory_name_ = changed_dir_name;
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

CDirectoryChangeWatcher::CDirectoryChangeWatcher(bool app_hasGUI *= true*, DWORD filter_flags*=FILTERS_CHECK_FILE_NAME_ONLY*)
: comp_port_(NULL)
 ,thread_(NULL)
 ,thread_id_(0UL)
 ,app_hasGUI_(app_hasGUI)
 ,filter_flags_(filter_flags == 0? FILTERS_DEFAULT_BEHAVIOR : filter_flags)
{
	//NOTE:  
	//	The app_hasGUI variable indicates that you have a message pump associated
	//	with the main thread(or the thread that first calls CDirectoryChangeWatcher::WatchDirectory()).
	//	Directory change notifications are dispatched to your main thread.
	//	
	//	If your app doesn't have a gui, then pass false.  Doing so causes a worker thread
	//	to be created that implements a message pump where it dispatches/executes the notifications.
	//  It's ok to pass false even if your app does have a GUI.
	//	Passing false is required for Console applications, or applications without a message pump.
	//	Note that notifications are fired in a worker thread.
	//

	//NOTE:
	//
	//
}

CDirectoryChangeWatcher::~CDirectoryChangeWatcher()
{

	UnwatchAllDirectories();

	if (comp_port_)
	{
		CloseHandle(comp_port_);
		comp_port_ = NULL;
	}
}

DWORD CDirectoryChangeWatcher::SetFilterFlags(DWORD filter_flags)
//
//	SetFilterFlags()
//	
//	sets filter behavior for directories watched AFTER this function has been called.
//
//
//
{
	DWORD old = filter_flags_;
	filter_flags_ = filter_flags;
	if (filter_flags_ == 0)
		filter_flags_ = FILTERS_DEFAULT_BEHAVIOR;//the default.
	return old;
}

BOOL CDirectoryChangeWatcher::IsWatchingDirectory(const CString & dir_name)const
///*********************************************
//  Determines whether or not a directory is being watched
//
//  be carefull that you have the same name path name, including the backslash
//  as was used in the call to WatchDirectory().
//
//	  ie:	
//			"C:\\Temp"
//		is different than
//			"C:\\Temp\\"
//**********************************************
{
	CSingleLock lock(const_cast<CCriticalSection*>(&cs_dir_watch_info_), TRUE);
	ASSERT(lock.IsLocked());
	int i;
	if (GetDirWatchInfo(dir_name, i))
		return TRUE;
	return FALSE;
}

int	CDirectoryChangeWatcher::NumWatchedDirectories()const
{
	CSingleLock lock(const_cast<CCriticalSection*>(&cs_dir_watch_info_), TRUE);
	ASSERT(lock.IsLocked());
	int cnt(0),max = directories_to_watch_.GetSize();
	for (int i(0); i < max; ++i)
	{
		if (directories_to_watch_[i] != NULL)//array may contain NULL elements.
			cnt++;
	}

	return cnt;
}

DWORD CDirectoryChangeWatcher::WatchDirectory(const CString & dir_to_watch, 
									   DWORD changes_to_watch_for, 
									   CDirectoryChangeHandler * change_handler,
									   BOOL watch_sub_dirs *=FALSE*,
									   LPCTSTR include_filter *=NULL*,
									   LPCTSTR exclude_filter *=NULL*
									  )
///*************************************************************
//FUNCTION:	WatchDirectory(const CString & dir_to_watch,   --the name of the directory to watch
//						   DWORD changes_to_watch_for, --the changes to watch for see dsk docs..for ReadDirectoryChangesW
//						   CDirectoryChangeHandler * change_handler -- handles changes in specified directory
//						   BOOL watch_sub_dirs      --specified to watch sub directories of the directory that you want to watch
//						  )
//
//PARAMETERS:
//		const CString & dir_to_watch -- specifies the path of the directory to watch.
//		DWORD changes_to_watch_for	-- specifies flags to be passed to ReadDirectoryChangesW()
//		CDirectoryChangeHandler *	-- ptr to an object which will handle notifications of file changes.
//		BOOL watch_sub_dirs			-- specifies to watch subdirectories.
//		LPCTSTR include_filter		-- A file pattern string for files that you wish to receive notifications
//									   for. See Remarks.
//		LPCTSTR exclude_filter		-- A file pattern string for files that you do not wish to receive notifications for. See Remarks
//
//	Starts watching the specified directory(and optionally subdirectories) for the specified changes
//
//	When specified changes take place the appropriate CDirectoryChangeHandler::On_Filexxx() function is called.
//
//	changes_to_watch_for can be a combination of the following flags, and changes map out to the 
//	following functions:
//	FILE_NOTIFY_CHANGE_FILE_NAME    -- CDirectoryChangeHandler::On_FileAdded()
//									   CDirectoryChangeHandler::On_FileNameChanged, 
//									   CDirectoryChangeHandler::On_FileRemoved
//	FILE_NOTIFY_CHANGE_DIR_NAME     -- CDirectoryChangeHandler::On_FileNameAdded(), 
//									   CDirectoryChangeHandler::On_FileRemoved
//	FILE_NOTIFY_CHANGE_ATTRIBUTES   -- CDirectoryChangeHandler::On_FileModified
//	FILE_NOTIFY_CHANGE_SIZE         -- CDirectoryChangeHandler::On_FileModified
//	FILE_NOTIFY_CHANGE_LAST_WRITE   -- CDirectoryChangeHandler::On_FileModified
//	FILE_NOTIFY_CHANGE_LAST_ACCESS  -- CDirectoryChangeHandler::On_FileModified
//	FILE_NOTIFY_CHANGE_CREATION     -- CDirectoryChangeHandler::On_FileModified
//	FILE_NOTIFY_CHANGE_SECURITY     -- CDirectoryChangeHandler::On_FileModified?
//	
//
//	Returns ERROR_SUCCESS if the directory will be watched, 
//	or a windows error code if the directory couldn't be watched.
//	The error code will most likely be related to a call to CreateFile(), or 
//	from the initial call to ReadDirectoryChangesW().  It's also possible to get an
//	error code related to being unable to create an io completion port or being unable 
//	to start the worker thread.
//
//	This function will fail if the directory to be watched resides on a 
//	computer that is not a Windows NT/2000/XP machine.
//
//
//	You can only have one watch specified at a time for any particular directory.
//	Calling this function with the same directory name will cause the directory to be 
//	unwatched, and then watched again(w/ the new parameters that have been passed in).  
//
//**************************************************************
{
	ASSERT(changes_to_watch_for != 0);

	if (dir_to_watch.IsEmpty()
	||  changes_to_watch_for == 0 
	||  change_handler == NULL)
	{
		TRACE(_T("ERROR: You've passed invalid parameters to CDirectoryChangeWatcher::WatchDirectory()\n"));
		::SetLastError(ERROR_INVALID_PARAMETER);
		return ERROR_INVALID_PARAMETER;
	}

	
	//double check that it's really a directory
	if (!IsDirectory(dir_to_watch))
	{
		TRACE(_T("ERROR: CDirectoryChangeWatcher::WatchDirectory() -- %s is not a directory!\n"), dir_to_watch);
		::SetLastError(ERROR_BAD_PATHNAME);
		return ERROR_BAD_PATHNAME;
	}

	//double check that this directory is not already being watched....
	//if it is, then unwatch it before restarting it...
	if (IsWatchingDirectory(dir_to_watch))
	{
		VERIFY(
			UnwatchDirectory(dir_to_watch) 
			);
	}
	//
	//
	//	Reference this singleton so that privileges for this process are enabled 
	//	so that it has required permissions to use the ReadDirectoryChangesW API, etc.
	//
	CPrivilegeEnabler::Instance();
	//
	//open the directory to watch....
	HANDLE dir = CreateFile(dir_to_watch, 
								FILE_LIST_DIRECTORY, 
								FILE_SHARE_READ | FILE_SHARE_WRITE ,//| FILE_SHARE_DELETE, <-- removing FILE_SHARE_DELETE prevents the user or someone else from renaming or deleting the watched directory. This is a good thing to prevent.
								NULL, //security attributes
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | //<- the required priviliges for this flag are: SE_BACKUP_NAME and SE_RESTORE_NAME.  CPrivilegeEnabler takes care of that.
                                FILE_FLAG_OVERLAPPED, //OVERLAPPED!
								NULL);
	if (dir == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		TRACE(_T("CDirectoryChangeWatcher::WatchDirectory() -- Couldn't open directory for monitoring. %d\n"), error);
		::SetLastError(error);//who knows if TRACE will cause GetLastError() to return success...probably won't, but set it manually just for fun.
		return error;
	}
	//opened the dir!
	
	CDirWatchInfo * dir_info = new CDirWatchInfo(dir, dir_to_watch, change_handler, changes_to_watch_for, watch_sub_dirs, app_hasGUI_, include_filter, exclude_filter, filter_flags_);
	if (!dir_info)
	{
		TRACE(_T("WARNING: Couldn't allocate a new CDirWatchInfo() object --- File: %s Line: %d\n"), _T(__FILE__), __LINE__);
		CloseHandle(dir);
		::SetLastError(ERROR_OUTOFMEMORY);
		return ERROR_OUTOFMEMORY;
	}
	
	//create a IO completion port/or associate this key with
	//the existing IO completion port
	comp_port_ = CreateIoCompletionPort(dir, 
										comp_port_, //if comp_port_ is NULL, dir is associated with a NEW completion port,
													 //if comp_port_ is NON-NULL, dir is associated with the existing completion port that the handle comp_port_ references
										(DWORD)dir_info, //the completion 'key'... this ptr is returned from GetQueuedCompletionStatus() when one of the events in the changes_to_watch_for filter takes place
										0);
	if (comp_port_ == NULL)
	{
		TRACE(_T("ERROR -- Unable to create I/O Completion port! GetLastError(): %d File: %s Line: %d"), GetLastError(), _T(__FILE__), __LINE__);
		DWORD error = GetLastError();
		dir_info->DeleteSelf(NULL);
		::SetLastError(error);//who knows what the last error will be after i call dir_info->DeleteSelf(), so set it just to make sure
		return error;
	}
	else
	{//completion port created/directory associated w/ it successfully

		//if the thread isn't running start it....
		//when the thread starts, it will call ReadDirectoryChangesW and wait 
		//for changes to take place
		if (thread_ == NULL)
		{
			//start the thread
			CWinThread * thread = AfxBeginThread(MonitorDirectoryChanges, this);
			if (!thread)
			{//couldn't create the thread!
				TRACE(_T("CDirectoryChangeWatcher::WatchDirectory()-- AfxBeginThread failed!\n"));
				dir_info->DeleteSelf(NULL);
				return (GetLastError() == ERROR_SUCCESS)? ERROR_MAX_THRDS_REACHED : GetLastError();
			}
			else
			{
				thread_	 = thread->thread_;
				thread_id_ = thread->thread_id_;
				thread->auto_delete_ = TRUE;//thread is deleted when thread ends....it's TRUE by default(for CWinThread ptrs returned by AfxBeginThread(threadproc, void*)), but just makin sure.
				
			}
		}
		if (thread_ != NULL)
		{//thread is running, 
			//signal the thread to issue the initial call to
			//ReadDirectoryChangesW()
		   DWORD started = dir_info->StartMonitor(comp_port_);

		   if (started != ERROR_SUCCESS)
		   {//there was a problem!
			   TRACE(_T("Unable to watch directory: %s -- GetLastError(): %d\n"), started);
			   dir_info->DeleteSelf(NULL);
				::SetLastError(started);//I think this'll set the Err object in a VB app.....
			   return started;
		   }
		   else
		   {//ReadDirectoryChangesW was successfull!
				//add the directory info to the first empty slot in the array

				//associate the change_handler with this object
				change_handler->ReferencesWatcher(this);//reference is removed when directory is unwatched.
				//CDirWatchInfo::DeleteSelf() must now be called w/ this CDirectoryChangeWatcher pointer becuse
				//of a reference count

				//save the CDirWatchInfo* so I'll be able to use it later.
				VERIFY(AddToWatchInfo(dir_info));
				SetLastError(started);
				return started;
		   }

		}
		else
		{
			ASSERT(FALSE);//control path shouldn't get here
			::SetLastError(ERROR_MAX_THRDS_REACHED);
			return ERROR_MAX_THRDS_REACHED;
		}
		
	}
	ASSERT(FALSE);//shouldn't get here.
}

BOOL CDirectoryChangeWatcher::UnwatchAllDirectories()
{
	
	//unwatch all of the watched directories
	//delete all of the CDirWatchInfo objects,
	//kill off the worker thread
	if (thread_ != NULL)
	{
		ASSERT(comp_port_ != NULL);
		
		CSingleLock lock(&cs_dir_watch_info_, TRUE);
		ASSERT(lock.IsLocked());

		//Unwatch each of the watched directories
		//and delete the CDirWatchInfo associated w/ that directory...
		const int max = directories_to_watch_.GetSize();
		for (int i = 0; i < max; ++i)
			if (CDirWatchInfo* dirInfo = directories_to_watch_[i])
			{
				VERIFY(dirInfo->UnwatchDirectory(comp_port_));

				//directories_to_watch_.SetAt(i, NULL)	;
				//dir_info->DeleteSelf(this);
			}

		//kill off the thread
		PostQueuedCompletionStatus(comp_port_, 0, 0, NULL);//The thread will catch this and exit the thread
		//wait for it to exit
		WaitForSingleObject(thread_, INFINITE);
		//CloseHandle(thread_);//Since thread was started w/ AfxBeginThread() this handle is closed automatically, closing it again will raise an exception

		for (int i = 0; i < max; ++i)
			if (CDirWatchInfo* dirInfo= directories_to_watch_[i])
			{
			//	VERIFY(dir_info->UnwatchDirectory(comp_port_));

				directories_to_watch_.SetAt(i, NULL);
				dirInfo->DeleteSelf(this);
			}

		directories_to_watch_.RemoveAll();

		thread_ = NULL;
		thread_id_ = 0UL;

		//close the completion port...
		CloseHandle(comp_port_);
		comp_port_ = NULL;

		return TRUE;
	}
	else
	{
#ifdef _DEBUG
		//make sure that there aren't any 
		//CDirWatchInfo objects laying around... they should have all been destroyed 
		//and removed from the array directories_to_watch_
		if (directories_to_watch_.GetSize() > 0)
		{
			for (int i = 0; i < directories_to_watch_.GetSize(); ++i)
			{
				ASSERT(directories_to_watch_[i] == NULL);
			}
		}
#endif
	}
	return FALSE;
}

BOOL CDirectoryChangeWatcher::UnwatchDirectory(const CString & dir_to_stop_watching)
///***************************************************************
//FUNCTION:	UnwatchDirectory(const CString & dir_to_stop_watching -- if this directory is being watched, the watch is stopped
//
//****************************************************************
{
	BOOL ret_val = FALSE;



	if (comp_port_ != NULL)//The io completion port must be open
	{
		ASSERT(!dir_to_stop_watching.IsEmpty());
		
		CSingleLock lock(&cs_dir_watch_info_, TRUE);
		ASSERT(lock.IsLocked());	
		int idx = -1;
		CDirWatchInfo * dir_info = GetDirWatchInfo(dir_to_stop_watching, idx);
		if (dir_info != NULL
		&&	idx != -1)
		{

			//stop watching this directory
			VERIFY(dir_info->UnwatchDirectory(comp_port_));

			//cleanup the object used to watch the directory
			directories_to_watch_.SetAt(idx, NULL);
			dir_info->DeleteSelf(this);
			ret_val = TRUE;
		}
	}

	return ret_val;
}

BOOL CDirectoryChangeWatcher::UnwatchDirectory(CDirectoryChangeHandler * change_handler)
///************************************
//
//  This function is called from the dtor of CDirectoryChangeHandler automatically,
//  but may also be called by a programmer because it's public...
//  
//  A single CDirectoryChangeHandler may be used for any number of watched directories.
//
//  Unwatch any directories that may be using this 
//  CDirectoryChangeHandler * change_handler to handle changes to a watched directory...
//  
//  The CDirWatchInfo::change_handler_ member of objects in the directories_to_watch_
//  array will == change_handler if that handler is being used to handle changes to a directory....
//************************************
{
	ASSERT(change_handler);

	CSingleLock lock(&cs_dir_watch_info_, TRUE);
	
	ASSERT(lock.IsLocked());
	
	int unwatched = 0;
	int idx = -1;
	CDirWatchInfo * dir_info;
	//
	//	go through and unwatch any directory that is 
	//	that is using this change_handler as it's file change notification handler.
	//
	while((dir_info = GetDirWatchInfo(change_handler, idx)) != NULL)
	{
		VERIFY(dir_info->UnwatchDirectory(comp_port_));

		unwatched++;
		directories_to_watch_.SetAt(idx, NULL);
		dir_info->DeleteSelf(this);	
	}
	return (BOOL)(unwatched != 0);
}

BOOL CDirectoryChangeWatcher::UnwatchDirectoryBecauseOfError(CDirWatchInfo * watch_info)
//
//	Called in the worker thread in the case that ReadDirectoryChangesW() fails
//	during normal operation. One way to force this to happen is to watch a folder
//	using a UNC path and changing that computer's IP address.
//	
{
	ASSERT(watch_info);
	ASSERT(thread_id_ == GetCurrentThreadId());//this should be called from the worker thread only.
	BOOL ret_val = FALSE;
	if (watch_info)
	{
		CSingleLock lock(&cs_dir_watch_info_, TRUE);
		
		ASSERT(lock.IsLocked());
		int idx = -1;
		if (GetDirWatchInfo(watch_info, idx) == watch_info)
		{
			// we are actually watching this....

			//
			//	Remove this CDirWatchInfo object from the list of watched directories.
			//
			directories_to_watch_.SetAt(idx, NULL);//mark the space as free for the next watch...

			//
			//	and delete it...
			//

			watch_info->DeleteSelf(this);
		}

	}
	return ret_val;
}

int	CDirectoryChangeWatcher::AddToWatchInfo(CDirectoryChangeWatcher::CDirWatchInfo * watch_info)
//
//	
//	To add the CDirWatchInfo  * to an array.
//	The array is used to keep track of which directories 
//	are being watched.
//
//	Add the ptr to the first non-null slot in the array.
{
	CSingleLock lock(&cs_dir_watch_info_, TRUE);
	ASSERT(lock.IsLocked());
	
	//first try to add it to the first empty slot in directories_to_watch_
	int max = directories_to_watch_.GetSize();
	int i= 0;
	for (i = 0; i < max; ++i)
	{
		if (directories_to_watch_[i] == NULL)
		{
			directories_to_watch_[i] = watch_info;
			break;
		}
	}

	if (i == max)
	{
		// there where no empty slots, add it to the end of the array
		try
		{
			i = directories_to_watch_.Add(watch_info);
		}
		catch(CMemoryException * e)
		{
			e->ReportError();
			e->Delete();//??? delete this? I thought CMemoryException objects where pre allocated in mfc? -- sample code in msdn does, so will i
			i = -1;
		}
	}

	return (BOOL)(i != -1);
}

//
//	functions for retrieving the directory watch info based on different parameters
//
CDirectoryChangeWatcher::CDirWatchInfo * CDirectoryChangeWatcher::GetDirWatchInfo(const CString & dir_name, int & ref_nIdx)const
{
	if (dir_name.IsEmpty())// can't be watching a directory if it's you don't pass in the name of it...
		return FALSE;		  //
	
	CSingleLock lock(const_cast<CCriticalSection*>(&cs_dir_watch_info_), TRUE);

	int max = directories_to_watch_.GetSize();
	CDirWatchInfo * p = NULL;
	for (int i = 0; i < max; ++i)
	{
		if ((p = directories_to_watch_[i]) != NULL
		&&	p->dir_name_.CompareNoCase(dir_name) == 0)
		{
			ref_nIdx = i;
			return p;
		}
	}
			
	return NULL;//NOT FOUND
}

CDirectoryChangeWatcher::CDirWatchInfo * CDirectoryChangeWatcher::GetDirWatchInfo(CDirectoryChangeWatcher::CDirWatchInfo * watch_info, int & ref_nIdx)const
{
	ASSERT(watch_info != NULL);

	CSingleLock lock(const_cast<CCriticalSection*>(&cs_dir_watch_info_), TRUE);
	int i(0), max = directories_to_watch_.GetSize();
	CDirWatchInfo * p;
	for (; i < max; ++i)
	{
		if ((p = directories_to_watch_[i]) != NULL
			&& p == watch_info)
		{
			ref_nIdx = i;
			return p;
		}
	}
	return NULL;//NOT FOUND
}

CDirectoryChangeWatcher::CDirWatchInfo * CDirectoryChangeWatcher::GetDirWatchInfo(CDirectoryChangeHandler * change_handler, int & ref_nIdx)const
{
	ASSERT(change_handler != NULL);
	CSingleLock lock(const_cast<CCriticalSection*>(&cs_dir_watch_info_), TRUE);
	int i(0),max = directories_to_watch_.GetSize();
	CDirWatchInfo * p;
	for (; i < max; ++i)
	{
		if ((p = directories_to_watch_[i]) != NULL
		&&	p->GetRealChangeHandler() == change_handler)
		{
			ref_nIdx = i;
			return p;
		}
	}
	return NULL;//NOT FOUND
}

long CDirectoryChangeWatcher::ReleaseReferenceToWatcher(CDirectoryChangeHandler * change_handler)
{
	ASSERT(change_handler);
	return change_handler->ReleaseReferenceToWatcher(this);
}

CDirectoryChangeWatcher::CDirWatchInfo::CDirWatchInfo(HANDLE dir, 
													  const CString & directory_name, 
													  CDirectoryChangeHandler * change_handler,
													  DWORD change_filter, 
													  BOOL watch_sub_dir,
													  bool app_hasGUI,
													  LPCTSTR include_filter,
													  LPCTSTR exclude_filter,
													  DWORD filter_flags)
 :	change_handler_(NULL), 
	dir_(dir),
	change_filter_(change_filter),
	watch_sub_dir_(watch_sub_dir),
	dir_name_(directory_name),
	buf_length_(0),
	read_dir_error_(ERROR_SUCCESS),
	start_stop_event_(FALSE, TRUE), //NOT SIGNALLED, MANUAL RESET
	running_state_(RUNNING_STATE_NOT_SET)
{ 
	
	ASSERT(dir != INVALID_HANDLE_VALUE 
		&& !directory_name.IsEmpty());
	
	//
	//	This object 'decorates' the change_handler passed in
	//	so that notifications fire in the context a thread other than
	//	CDirectoryChangeWatcher::MonitorDirectoryChanges()
	//
	//	Supports the include and exclude filters
	//
	//
	change_handler_ = new CDelayedDirectoryChangeHandler(change_handler, app_hasGUI, include_filter, exclude_filter, filter_flags);
	if (change_handler_) 
		change_handler_->SetPartialPathOffset(dir_name_);//to support FILTERS_CHECK_PARTIAL_PATH..this won't change for the duration of the watch, so set it once... HERE!
	ASSERT(change_handler_);

	ASSERT(GetChangeHandler());
	ASSERT(GetRealChangeHandler());
	if (GetRealChangeHandler())
		GetRealChangeHandler()->AddRef();
	
	memset(&overlapped_, 0, sizeof(overlapped_));
	//memset(buffer_, 0, sizeof(buffer_));
}

CDirectoryChangeWatcher::CDirWatchInfo::~CDirWatchInfo()
{
	if (GetChangeHandler())
	{//If this call to CDirectoryChangeHandler::Release() causes change_handler_ to delete itself,
		//the dtor for CDirectoryChangeHandler will call CDirectoryChangeWatcher::UnwatchDirectory(CDirectoryChangeHandler *),
		//which will make try to delete this object again.
		//if change_handler_ is NULL, it won't try to delete this object again...
		CDirectoryChangeHandler * tmp = SetRealDirectoryChangeHandler(NULL);
		if (tmp)
			tmp->Release();
		else{
			ASSERT(FALSE);
		}
	}
	
	CloseDirectoryHandle();

	delete change_handler_;
	change_handler_ = NULL;

	TRACE(L"CDirWatchInfo has been deleted (%p)\n", this);
}

void CDirectoryChangeWatcher::CDirWatchInfo::DeleteSelf(CDirectoryChangeWatcher * watcher)
//
//	There's a reason for this function!
//
//	the dtor is private to enforce that it is used.
//
//
//	watcher can be NULL only if CDirecotryChangeHandler::ReferencesWatcher() has NOT been called.
//	ie: in certain sections of WatchDirectory() it's ok to pass this w/ NULL, but no where else.
//
{
	//ASSERT(watcher != NULL);


	ASSERT(GetRealChangeHandler());
	if (watcher)
	{
	//
	//
	//	Before this object is deleted, the CDirectoryChangeHandler object
	//	needs to release it's reference count to the CDirectoryChangeWatcher object.
	//	I might forget to do this since I getting rid of CDirWatchInfo objects
	//	in more than one place...hence the reason for this function.
	//
		watcher->ReleaseReferenceToWatcher(GetRealChangeHandler());
	}
	
	delete this;
}

CDelayedDirectoryChangeHandler* CDirectoryChangeWatcher::CDirWatchInfo::GetChangeHandler() const 
{ 
	return change_handler_; 
}

CDirectoryChangeHandler * CDirectoryChangeWatcher::CDirWatchInfo::GetRealChangeHandler() const
//
//	The 'real' change handler is the CDirectoryChangeHandler object 
//	passed to CDirectoryChangeWatcher::WatchDirectory() -- this is the object
//	that really handles the changes.
//
{	
	ASSERT(change_handler_); 
	return change_handler_->GetRealChangeHandler(); 
}

CDirectoryChangeHandler * CDirectoryChangeWatcher::CDirWatchInfo::SetRealDirectoryChangeHandler(CDirectoryChangeHandler * change_handler)
//
//	Allows you to switch out, at run time, which object really handles the change notifications.
//
{
	CDirectoryChangeHandler * old = GetRealChangeHandler();
	change_handler_->GetRealChangeHandler() = change_handler; 
	return old;
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::CloseDirectoryHandle()
//
//	Closes the directory handle that was opened in CDirectoryChangeWatcher::WatchDirecotry()
//
//
{
	BOOL b = TRUE;
	if (dir_ != INVALID_HANDLE_VALUE)
	{
		b = CloseHandle(dir_);
		dir_ = INVALID_HANDLE_VALUE;
	}
	return b;
}

DWORD CDirectoryChangeWatcher::CDirWatchInfo::StartMonitor(HANDLE comp_port)
///*********************************************
//  Sets the running state of the object to perform the initial call to ReadDirectoryChangesW()
//  , wakes up the thread waiting on GetQueuedCompletionStatus()
//  and waits for an event to be set before returning....
//
//  The return value is either ERROR_SUCCESS if ReadDirectoryChangesW is successfull,
//  or is the value of GetLastError() for when ReadDirectoryChangesW() failed.
//**********************************************
{
	ASSERT(comp_port);

	//Guard the properties of this object 
	VERIFY(LockProperties());
	

		
	running_state_ = RUNNING_STATE_START_MONITORING;//set the state member to indicate that the object is to START monitoring the specified directory
	PostQueuedCompletionStatus(comp_port, sizeof(this), (DWORD)this, &overlapped_);//make the thread waiting on GetQueuedCompletionStatus() wake up

	VERIFY(UnlockProperties());//unlock this object so that the thread can get at them...

	//wait for signal that the initial call 
	//to ReadDirectoryChanges has been made
	DWORD wait = 0;
	do{
		wait = WaitForSingleObject(start_stop_event_, 10 * 1000);
		if (wait != WAIT_OBJECT_0)
		{
			//
			//	shouldn't ever see this one....but just in case you do, notify me of the problem wesj@hotmail.com.
			//
			TRACE(_T("WARNING! Possible lockup detected. FILE: %s Line: %d\n"), _T(__FILE__), __LINE__);
		}
	} while(wait != WAIT_OBJECT_0);

	ASSERT(wait == WAIT_OBJECT_0);
	start_stop_event_.ResetEvent();
	
	return read_dir_error_;//This value is set in the worker thread when it first calls ReadDirectoryChangesW().
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::UnwatchDirectory(HANDLE comp_port)
///*******************************************
//
//    Sets the running state of the object to stop monitoring a directory,
//	Causes the worker thread to wake up and to stop monitoring the dierctory
//	
//********************************************
{
	ASSERT(comp_port);
	//
	// Signal that the worker thread is to stop watching the directory
	//
	if (SignalShutdown(comp_port))
	{
		//and wait for the thread to signal that it has shutdown
		return WaitForShutdown();

	}
	return FALSE;
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::SignalShutdown(HANDLE comp_port)
//added to fix a bug -- this will be called normally by UnwatchDirectory(HANDLE)
//						and abnormally by the worker thread in the case that ReadDirectoryChangesW fails -- see code.
//
//	Signals the worker thread(via the I/O completion port) that it is to stop watching the 
//	directory for this object, and then returns.
//
{
	BOOL ret_val = FALSE;
	ASSERT(comp_port);
	ASSERT(dir_ != INVALID_HANDLE_VALUE);
	//Lock the properties so that they aren't modified by another thread
	VERIFY(LockProperties()); //unlikey to fail...
		
	//set the state member to indicate that the object is to stop monitoring the 
	//directory that this CDirWatchInfo is responsible for...
	running_state_ = CDirectoryChangeWatcher::CDirWatchInfo::RUNNING_STATE_STOP;
	//put this object in the I/O completion port... GetQueuedCompletionStatus() will return it inside the worker thread.
	ret_val = PostQueuedCompletionStatus(comp_port, sizeof(CDirWatchInfo*), (DWORD)this, &overlapped_);

	if (!ret_val)
	{
		TRACE(_T("PostQueuedCompletionStatus() failed! GetLastError(): %d\n"), GetLastError());
	}
	VERIFY(UnlockProperties());
	
	return ret_val;
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::WaitForShutdown()
//
//	This is to be called some time after SignalShutdown().
//	
//
{
	ASSERT_VALID(&start_stop_event_);
	
	//Wait for the Worker thread to indicate that the watch has been stopped
	MSG quitMsg = {0};

	DWORD wait;
	bool WM_quit_received = false;
	do{
		wait	= MsgWaitForMultipleObjects(1, &start_stop_event_.m_hObject, FALSE, 5000, QS_ALLINPUT);//wait five seconds
		switch(wait)
		{
		case WAIT_OBJECT_0:
			//handle became signalled!
			break;
		case WAIT_OBJECT_0 + 1:
			{
				//This thread awoke due to sent/posted message
				//process the message Q
				//
				//	There is a message in this thread's queue, so 
				//	MsgWaitForMultipleObjects returned.
				//	Process those messages, and wait again.

				MSG msg;
				while (PeekMessage(&msg, NULL, 0,0, PM_REMOVE)) 
				{
					if (msg.message != WM_QUIT)
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else
					{
						quitMsg = msg;
						///****
						//This appears to be causing quite a lot of pain, to quote Mustafa.

						////it's the WM_QUIT message, put it back in the Q and
						//// exit this function
						//PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
						//WM_quit_received = true;

						//****
						////WM_quit_received = true;
						break;
					}
				}
		
				//if (quitMsg.message)
				//	PostMessage(quitMsg.hwnd, quitMsg.message, quitMsg.wParam, quitMsg.lParam);
			}
			break;

		case WAIT_TIMEOUT:
			{
				TRACE(_T("WARNING: Possible Deadlock detected! ThreadID: %d File: %s Line: %d\n"), GetCurrentThreadId(), _T(__FILE__), __LINE__);
			}break;
		}//end switch(wait)
	}while(wait != WAIT_OBJECT_0 && !WM_quit_received);
		
	
	if (quitMsg.message)
		PostMessage(quitMsg.hwnd, quitMsg.message, quitMsg.wParam, quitMsg.lParam);
	
	ASSERT(wait == WAIT_OBJECT_0 || WM_quit_received);

	start_stop_event_.ResetEvent();
	
	return (BOOL) (wait == WAIT_OBJECT_0 || WM_quit_received);
}


UINT CDirectoryChangeWatcher::MonitorDirectoryChanges(LPVOID lpvThis)
///********************************************
//   The worker thread function which monitors directory changes....
//********************************************
{
	TRACE(_T("CDirectoryChangeWatcher::MonitorDirectoryChanges() started.\n"));
    DWORD numBytes;

    CDirWatchInfo * pdi;
    LPOVERLAPPED overlapped;
    
	CDirectoryChangeWatcher * this = reinterpret_cast<CDirectoryChangeWatcher*>(lpvThis);
	ASSERT(this);

	this->On_ThreadInitialize();


    do
    {
        // Retrieve the directory info for this directory
        // through the io port's completion key
        if (!GetQueuedCompletionStatus(this->comp_port_,
                                   &numBytes,
                                   (LPDWORD) &pdi,//<-- completion Key
                                   &overlapped,
                                   INFINITE))
		{//The io completion request failed...
		 //probably because the handle to the directory that
		 //was used in a call to ReadDirectoryChangesW has been closed.

			//
			//	calling pdi->CloseDirectoryHandle() will cause GetQueuedCompletionStatus() to return false.
			//  
			//
			if (!pdi 
			|| (pdi && AfxIsValidAddress(pdi, sizeof(CDirectoryChangeWatcher::CDirWatchInfo)))
					 &&  pdi->dir_ != INVALID_HANDLE_VALUE //the directory handle is still open! (we expect this when after we close the directory handle)
			 )
			{
#ifdef _DEBUG
			TRACE(_T("GetQueuedCompletionStatus() returned FALSE\nGetLastError(): %d Completion Key: %p lpOverlapped: %p\n"), GetLastError(), pdi, overlapped);
			MessageBeep(static_cast<UINT>(-1));
#endif
			}
		}
		
		if (pdi)//pdi will be null if I call PostQueuedCompletionStatus(comp_port_, 0,0,NULL);
        {
			//
			//	The following check to AfxIsValidAddress() should go off in the case
			//	that I have deleted this CDirWatchInfo object, but it was still in 
			//	"in the Queue" of the i/o completion port from a previous overlapped operation.
			//
			ASSERT(AfxIsValidAddress(pdi, 
					sizeof(CDirectoryChangeWatcher::CDirWatchInfo)));
			///***********************************
			//The CDirWatchInfo::running_state_ is pretty much the only member
			//of CDirWatchInfo that can be modified from the other thread.
			//The functions StartMonitor() and UnwatchDirecotry() are the functions that 
			//can modify that variable.

			//So that I'm sure that I'm getting the right value, 
			//I'm using a critical section to guard against another thread modyfying it when I want
			//to read it...
			//
			//************************************
			bool object_should_be_ok = true;
			try{
			    VERIFY(pdi->LockProperties());//don't give the main thread a chance to change this object
			}
			catch(...){
				//any sort of exception here indicates I've
				//got a hosed object.
				TRACE(_T("CDirectoryChangeWatcher::MonitorDirectoryChanges() -- pdi->LockProperties() raised an exception!\n"));
				object_should_be_ok = false;
			}
			if (object_should_be_ok)
			{
										    //while we're working with this object...

				CDirWatchInfo::running_state Run_State = pdi->running_state_ ;
				
				VERIFY(pdi->UnlockProperties());//let another thread back at the properties...
		//		/***********************************
		//		 Unlock it so that there isn't a DEADLOCK if 
		//		 somebody tries to call a function which will 
		//		 cause CDirWatchInfo::UnwatchDirectory() to be called
		//		 from within the context of this thread (eg: a function called because of
		//		 the handler for one of the CDirectoryChangeHandler::On_Filexxx() functions)
		//
		//		************************************
				
				ASSERT(pdi->GetChangeHandler());
				switch(Run_State)
				{
				case CDirWatchInfo::RUNNING_STATE_START_MONITORING:
					{
						//Issue the initial call to ReadDirectoryChangesW()
						
						if (!ReadDirectoryChangesW(pdi->dir_,
											pdi->buffer_,//<--FILE_NOTIFY_INFORMATION records are put into this buffer
											READ_DIR_CHANGE_BUFFER_SIZE,
											pdi->watch_sub_dir_,
											pdi->change_filter_,
											&pdi->buf_length_,//this var not set when using asynchronous mechanisms...
											&pdi->overlapped_,
											NULL))//no completion routine!
						{
							pdi->read_dir_error_ = GetLastError();
							if (pdi->GetChangeHandler())
								pdi->GetChangeHandler()->On_WatchStarted(pdi->read_dir_error_, pdi->dir_name_);
						}
						else
						{//read directory changes was successful!
						 //allow it to run normally
							pdi->running_state_ = CDirWatchInfo::RUNNING_STATE_NORMAL;
							pdi->read_dir_error_ = ERROR_SUCCESS;
							if (pdi->GetChangeHandler())
								pdi->GetChangeHandler()->On_WatchStarted(ERROR_SUCCESS, pdi->dir_name_);
						}
						pdi->start_stop_event_.SetEvent();//signall that the ReadDirectoryChangesW has been called
														 //check CDirWatchInfo::read_dir_error_ to see whether or not ReadDirectoryChangesW succeeded...

						//
						//	note that pdi->read_dir_error_ is the value returned by WatchDirectory()
						//
						
		
					}break;
				case CDirWatchInfo::RUNNING_STATE_STOP:
					{
						//We want to shut down the monitoring of the directory
						//that pdi is managing...
						
						if (pdi->dir_ != INVALID_HANDLE_VALUE)
						{
						 //Since I've previously called ReadDirectoryChangesW() asynchronously, I am waiting
						 //for it to return via GetQueuedCompletionStatus().  When I close the
						 //handle that ReadDirectoryChangesW() is waiting on, it will
						 //cause GetQueuedCompletionStatus() to return again with this pdi object....
						 // Close the handle, and then wait for the call to GetQueuedCompletionStatus()
						 //to return again by breaking out of the switch, and letting GetQueuedCompletionStatus()
						 //get called again
							pdi->CloseDirectoryHandle();
							pdi->running_state_ = CDirWatchInfo::RUNNING_STATE_STOP_STEP2;//back up step...GetQueuedCompletionStatus() will still need to return from the last time that ReadDirectoryChangesW() was called.....

						 //
						 //	The watch has been stopped, tell the client about it
						 //						if (pdi->GetChangeHandler())
							pdi->GetChangeHandler()->On_WatchStopped(pdi->dir_name_);
						}
						else
						{
							//either we weren't watching this direcotry in the first place,
							//or we've already stopped monitoring it....
							pdi->start_stop_event_.SetEvent();//set the event that ReadDirectoryChangesW has returned and no further calls to it will be made...
						}
						
					
					}break;
				case CDirWatchInfo::RUNNING_STATE_STOP_STEP2:
					{

						//GetQueuedCompletionStatus() has returned from the last
						//time that ReadDirectoryChangesW was called...
						//Using CloseHandle() on the directory handle used by
						//ReadDirectoryChangesW will cause it to return via GetQueuedCompletionStatus()....
						if (pdi->dir_ == INVALID_HANDLE_VALUE)
							pdi->start_stop_event_.SetEvent();//signal that no further calls to ReadDirectoryChangesW will be made
															 //and this pdi can be deleted 
						else
						{//for some reason, the handle is still open..
												
							pdi->CloseDirectoryHandle();

							//wait for GetQueuedCompletionStatus() to return this pdi object again


						}
		
					}break;
															
				case CDirWatchInfo::RUNNING_STATE_NORMAL:
					{
						
						if (pdi->GetChangeHandler())
							pdi->GetChangeHandler()->SetChangedDirectoryName(pdi->dir_name_);
		
						DWORD read_buffer__offset = 0UL;

						//process the FILE_NOTIFY_INFORMATION records:
						CFileNotifyInformation notify_info((LPBYTE)pdi->buffer_, READ_DIR_CHANGE_BUFFER_SIZE);

						this->ProcessChangeNotifications(notify_info, pdi, read_buffer__offset);
		

						//	Changes have been processed,
						//	Reissue the watch command
						//
						if (!ReadDirectoryChangesW(pdi->dir_,
											  pdi->buffer_ + read_buffer__offset,//<--FILE_NOTIFY_INFORMATION records are put into this buffer 
								              READ_DIR_CHANGE_BUFFER_SIZE - read_buffer__offset,
											  pdi->watch_sub_dir_,
										      pdi->change_filter_,
											  &pdi->buf_length_,//this var not set when using asynchronous mechanisms...
											&pdi->overlapped_,
											NULL))//no completion routine!
						{
							//
							//	NOTE:  
							//		In this case the thread will not wake up for 
							//		this pdi object because it is no longer associated w/
							//		the I/O completion port...there will be no more outstanding calls to ReadDirectoryChangesW
							//		so I have to skip the normal shutdown routines(normal being what happens when CDirectoryChangeWatcher::UnwatchDirectory() is called.
							//		and close this up, & cause it to be freed.
							//
							TRACE(_T("WARNING: ReadDirectoryChangesW has failed during normal operations...failed on directory: %s\n"), pdi->dir_name_);

							ASSERT(this);
							//
							//	To help insure that this has been unwatched by the time
							//	the main thread processes the On_ReadDirectoryChangesError() notification
							//	bump the thread priority up temporarily.  The reason this works is because the notification
							//	is really posted to another thread's message queue,...by setting this thread's priority
							//	to highest, this thread will get to shutdown the watch by the time the other thread has a chance
							//	to handle it. *note* not technically guaranteed 100% to be the case, but in practice it'll work.
							int old_thread_priority = GetThreadPriority(GetCurrentThread());
							SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);							

							//
							//	Notify the client object....(a CDirectoryChangeHandler derived class)
							//
							try{
								pdi->read_dir_error_ = GetLastError();
								pdi->GetChangeHandler()->On_ReadDirectoryChangesError(pdi->read_dir_error_, pdi->dir_name_);


								//Do the shutdown
								this->UnwatchDirectoryBecauseOfError(pdi);
								//pdi = NULL; <-- DO NOT set this to NULL, it will cause this worker thread to exit.
								//pdi is INVALID at this point!!
							}
							catch(...)
							{
								//just in case of exception, this thread will be set back to 
								//normal priority.
							}
							//
							//	Set the thread priority back to normal.
							//
							SetThreadPriority(GetCurrentThread(), old_thread_priority);
													
						}
						else
						{//success, continue as normal
							pdi->read_dir_error_ = ERROR_SUCCESS;
						}
					}break;
				default:
					TRACE(_T("MonitorDirectoryChanges() -- how did I get here?\n"));
					break;//how did I get here?
				}//end switch(pdi->running_state_)
		
		
		
			}//end if (object_should_be_ok)
        }//end if (pdi)
    } while(pdi);

	this->On_ThreadExit();
	TRACE(_T("CDirectory	ChangeWatcher::MonitorDirectoryChanges() finished.\n"));
	return 0; //thread is ending
}

void CDirectoryChangeWatcher::ProcessChangeNotifications(IN CFileNotifyInformation & notify_info, 
														 IN CDirectoryChangeWatcher::CDirWatchInfo * pdi,
														 OUT DWORD & ref_dwReadBuffer_Offset//used in case ...see case for FILE_ACTION_RENAMED_OLD_NAME
														)
/////////////////////////////////////////////////////////////
//
//	Processes the change notifications and dispatches the handling of the 
//	notifications to the CDirectoryChangeHandler object passed to WatchDirectory()
//
/////////////////////////////////////////////////////////////
{
	//
	//	Sanity check...
	//	this function should only be called by the worker thread.
	//	
	ASSERT(thread_id_ == GetCurrentThreadId());

	//	Validate parameters...
	//	
	ASSERT(pdi);
	ASSERT(AfxIsValidAddress(pdi, sizeof(CDirectoryChangeWatcher::CDirWatchInfo)));

	if (!pdi || !AfxIsValidAddress(pdi, sizeof(CDirectoryChangeWatcher::CDirWatchInfo)))
	{
		TRACE(_T("Invalid arguments to CDirectoryChangeWatcher::ProcessChangeNotifications() -- pdi is invalid!\n"));
		TRACE(_T("File: %s Line: %d"), _T(__FILE__), __LINE__);
		return;
	}



	DWORD last_action = 0;
	ref_dwReadBuffer_Offset = 0UL;
	

	CDirectoryChangeHandler * change_handler = pdi->GetChangeHandler();
	//CDelayedDirectoryChangeHandler * change_handler = pdi->GetChangeHandler();
	ASSERT(change_handler);
	ASSERT(AfxIsValidAddress(change_handler, sizeof(CDirectoryChangeHandler)));
	//ASSERT(AfxIsValidAddress(change_handler, sizeof(CDelayedDirectoryChangeHandler)));
	if (!change_handler)
	{
		TRACE(_T("CDirectoryChangeWatcher::ProcessChangeNotifications() Unable to continue, pdi->GetChangeHandler() returned NULL!\n"));
		TRACE(_T("File: %s  Line: %d\n"), _T(__FILE__), __LINE__);
		return;
	}


	//
	//	go through and process the notifications contained in the
	//	CFileChangeNotification object(CFileChangeNotification is a wrapper for the FILE_NOTIFY_INFORMATION structure
	//									returned by ReadDirectoryChangesW)
	//
    do
	{
		//The FileName member of the FILE_NOTIFY_INFORMATION
		//structure contains the NAME of the file RELATIVE to the 
		//directory that is being watched...
		//ie, if watching C:\Temp and the file C:\Temp\MyFile.txt is changed,
		//the file name will be "MyFile.txt"
		//If watching C:\Temp, AND you're also watching subdirectories
		//and the file C:\Temp\OtherFolder\MyOtherFile.txt is modified,
		//the file name will be OtherFolder\MyOtherFile.txt

		//The CDirectoryChangeHandler::On_Filexxx() functions will receive the name of the file
		//which includes the full path to the directory being watched
		
		
		//	
		//	See what the change was
		//
		TRACE(L"dir/file notification: %x\n", int(notify_info.GetAction()));

		switch (notify_info.GetAction())
		{
		case FILE_ACTION_ADDED:		// a file was added!
	
			change_handler->On_FileAdded(notify_info.GetFileNameWithPath(pdi->dir_name_)); break;

		case FILE_ACTION_REMOVED:	//a file was removed
		
			change_handler->On_FileRemoved(notify_info.GetFileNameWithPath(pdi->dir_name_)); break;

		case FILE_ACTION_MODIFIED:
			//a file was changed
			//pdi->change_handler_->On_FileModified(last_file_name); break;
			change_handler->On_FileModified(notify_info.GetFileNameWithPath(pdi->dir_name_)); break;

		case FILE_ACTION_RENAMED_OLD_NAME:
			{//a file name has changed, and this is the OLD name
			 //This record is followed by another one w/
			 //the action set to FILE_ACTION_RENAMED_NEW_NAME (contains the new name of the file

				CString old_file_name = notify_info.GetFileNameWithPath(pdi->dir_name_);

				
				if (notify_info.GetNextNotifyInformation())
				{//there is another PFILE_NOTIFY_INFORMATION record following the one we're working on now...
				 //it will be the record for the FILE_ACTION_RENAMED_NEW_NAME record
			

					ASSERT(notify_info.GetAction() == FILE_ACTION_RENAMED_NEW_NAME);//making sure that the next record after the OLD_NAME record is the NEW_NAME record

					//get the new file name
					CString new_file_name = notify_info.GetFileNameWithPath(pdi->dir_name_);

					change_handler->On_FileNameChanged(old_file_name, new_file_name);
				}
				else
				{
					//this OLD_NAME was the last record returned by ReadDirectoryChangesW
					//I will have to call ReadDirectoryChangesW again so that I will get 
					//the record for FILE_ACTION_RENAMED_NEW_NAME

					//Adjust an offset so that when I call ReadDirectoryChangesW again,
					//the FILE_NOTIFY_INFORMATION will be placed after 
					//the record that we are currently working on.

					///***************
					//Let's say that 200 files all had their names changed at about the same time
					//There will be 400 FILE_NOTIFY_INFORMATION records (one for OLD_NAME and one for NEW_NAME for EACH file which had it's name changed)
					//that ReadDirectoryChangesW will have to report to
					//me.   There might not be enough room in the buffer
					//and the last record that we DID get was an OLD_NAME record,
					//I will need to call ReadDirectoryChangesW again so that I will get the NEW_NAME 
					//record.    This way I'll always have to old_file_name and new_file_name to pass
					//to CDirectoryChangeHandler::On_FileRenamed().

				 //  After ReadDirecotryChangesW has filled out our buffer with
				 //  FILE_NOTIFY_INFORMATION records,
				 //  our read buffer would look something like this:
					//																	 End Of Buffer
					//																		  |
					//																		 \-/	
					//|_________________________________________________________________________
					//|																		  |
					//|file1 OLD name record|file1 NEW name record|...|fileX+1 OLD_name record| |(the record we want would be here, but we've ran out of room, so we adjust an offset and call ReadDirecotryChangesW again to get it) 
					//|_________________________________________________________________________|

					//Since the record I need is still waiting to be returned to me,
					//and I need the current 'OLD_NAME' record,
					//I'm copying the current FILE_NOTIFY_INFORMATION record 
					//to the beginning of the buffer used by ReadDirectoryChangesW()
					//and I adjust the offset into the read buffer so the the NEW_NAME record
					//will be placed into the buffer after the OLD_NAME record now at the beginning of the buffer.

					//Before we call ReadDirecotryChangesW again,
					//modify the buffer to contain the current OLD_NAME record...

					//|_______________________________________________________
					//|														|
					//|fileX old name record(saved)|<this is now garbage>.....|
					//|_______________________________________________________|
					//						 	 /-\
					//							  |
					//						 Offset for Read
					//Re-issue the watch command to get the rest of the records...

					//ReadDirectoryChangesW(..., buffer + (an Offset),

					//After GetQueuedCompletionStatus() returns, 
					//our buffer will look like this:

					//|__________________________________________________________________________________________________________
					//|																										   |
					//|fileX old name record(saved)|fileX new name record(the record we've been waiting for)| <other records>... |
					//|__________________________________________________________________________________________________________|

					//Then I'll be able to know that a file name was changed
					//and I will have the OLD and the NEW name of the file to pass to CDirectoryChangeHandler::On_FileNameChanged

					//****************
					//NOTE that this case has never happened to me in my testing
					//so I can only hope that the code works correctly.
					//It would be a good idea to set a breakpoint on this line of code:
					VERIFY(notify_info.CopyCurrentRecordToBeginningOfBuffer(ref_dwReadBuffer_Offset));
					

				}
				break;
			}
		case FILE_ACTION_RENAMED_NEW_NAME:
			{
				//This should have been handled in FILE_ACTION_RENAMED_OLD_NAME
				ASSERT(last_action == FILE_ACTION_RENAMED_OLD_NAME);
				ASSERT(FALSE);//this shouldn't get here
			}
		
		default:
			TRACE(_T("CDirectoryChangeWatcher::ProcessChangeNotifications() -- unknown FILE_ACTION_ value! : %d\n"), notify_info.GetAction());
			break;//unknown action
		}

		last_action = notify_info.GetAction();
		
    
	} while(notify_info.GetNextNotifyInformation());
}

*/
