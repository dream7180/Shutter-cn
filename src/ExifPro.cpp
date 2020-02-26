/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifPro.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "ExifPro.h"
#include "MainFrm.h"
#include "BrowserFrame.h"
#include "ExifProView.h"
#include "Config.h"
#include "BrowserFrame.h"
#include "AboutDlg.h"
#include "CmdLineInfo.h"
#include "Logger.h"
#include "SEException.h"
#include "TimeLimit.h"
#include "ImgDb.h"
#include "FolderPathHelpers.h"
#include "scintilla\Include\Scintilla.h"
#include "UpdateCheck.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void SetImageCache(int image_cache_size);
void SetPhotoCache(int n, size_t memory_limit);

/////////////////////////////////////////////////////////////////////////////

//#pragma data_seg ("shared")
//
//// this doesn't work because of UPX
//static LONG shared_instance_counter= 0;
//
//#pragma data_seg ()
// linker: /SECTION:shared,RWS


/////////////////////////////////////////////////////////////////////////////
// App

BEGIN_MESSAGE_MAP(App, CWinApp)
	//{{AFX_MSG_MAP(App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_UPDATE_CHECK, OnUpdateCheck)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CHECK, DisableItem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// App construction

App::App()
{
//	pfn_auto_complete_fn_ = 0;
	app_event_ = 0;
}

//bool App::whistler_look_= false;
//extern DWORD g_common_control_lib_version= 0;
extern bool g_first_time_up= false;

////////////////////////////////////////////////////////////////////////////

static bool SetRegKey(const TCHAR* key, const TCHAR* value, const TCHAR* valueName= 0)
{
	if (valueName == NULL)
	{
		if (::RegSetValue(HKEY_CLASSES_ROOT, key, REG_SZ, value, lstrlen(value) * sizeof(*value)) != ERROR_SUCCESS)
		{
			TRACE(traceAppMsg, 0, _T("Warning: registration database update failed for key '%s'.\n"), key);
			return false;
		}
		return true;
	}
	else
	{
		HKEY hkey= 0;

		if (::RegCreateKey(HKEY_CLASSES_ROOT, key, &hkey) == ERROR_SUCCESS)
		{
			LONG result = ::RegSetValueEx(hkey, valueName, 0, REG_SZ,
				(CONST BYTE*)value, (lstrlen(value) + 1) * sizeof(*value));

			if (::RegCloseKey(hkey) == ERROR_SUCCESS && result == ERROR_SUCCESS)
				return true;
		}
		TRACE(traceAppMsg, 0, _T("Warning: registration database update failed for key '%s'.\n"), key);
		return false;
	}
}


void InvalidParameterHandler(
	const wchar_t* expression,
	const wchar_t* function,
	const wchar_t* file,
	unsigned int line,
	uintptr_t reserved)
{
	// add logging here...

	// just return, CRT function will then return error code
}


/////////////////////////////////////////////////////////////////////////////
// The one and only App object

App theApp;

/////////////////////////////////////////////////////////////////////////////
// App initialization

BOOL App::InitInstance()
{
	try
	{
		return InitializeInstance();
	}
	catch (...)
	{
		::MessageBox(0, _T("ExifPro initialization failed."), _T("ExifPro"), MB_ICONERROR | MB_OK);
	}
	return false;
}


void ShowMainWnd(BrowserFrame* main, int cmdShow)
{
	main->EnableWindow();
	main->ShowWindow(cmdShow);
	main->UpdateWindow();

	// initially set focus to the image pane
	main->InitialSetActiveView();
}


#if 0
extern string DumpTheStack(CONTEXT* context);

int handle_out_of_memory(size_t size)
{
	try
	{
		CONTEXT ctx;
		memset(&ctx, 0, sizeof ctx);
		__asm
		{
			mov ctx.Ebp, ebp
			lea eax, addr
addr:
			mov ctx.Eip, eax
			mov ctx.Esp, esp
		}

		string stack= DumpTheStack(&ctx);

		::MessageBoxA(0, stack.c_str(), "Out of Memory", MB_OK | MB_ICONERROR);
	}
	catch (...)
	{}

	throw std::bad_alloc("out of memory");
}
#endif


bool App::InitializeInstance()
{
	app_event_ = ::CreateEvent(0, false, true, _T("ExifProSingleStart"));
	DWORD err= ::GetLastError();

	// prevent multiple ExifPro instances from running concurrently
	if ( /*::InterlockedIncrement(&shared_instance_counter) > 1 || */
		err == ERROR_ALREADY_EXISTS)	// due to the UPX shared counter fails; rely on event instead
	{
//		::InterlockedDecrement(&shared_instance_counter);

		// find running ExifPro window
		HWND running= ::FindWindowEx(HWND_DESKTOP, 0, 0, _T("ExifPro"));

		if (running)
		{
			// bring running ExifPro window to front
			::SetForegroundWindow(running);

			// restore window if it's minimized
			if (::IsIconic(running))
				::ShowWindow(running, SW_RESTORE);
		}
		else
		{
			::MessageBox(0,
				_T("There is already an instance of ExifPro running.\n")
				_T("If it doesn't respond you may need to terminate it in the TaskManager."),
				_T("ExifPro"), MB_OK | MB_ICONWARNING);
		}

		return false;
	}

	AfxEnableControlContainer();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(_T("OLE initialization failed. Make sure that the OLE libraries are the correct version."));
		return FALSE;
	}

	INITCOMMONCONTROLSEX ccs;
	ccs.dwSize = sizeof ccs;
	ccs.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_COOL_CLASSES;
	::InitCommonControlsEx(&ccs);

	DWORD GetDllVersion(LPCTSTR dll_name);
	DWORD commonControlLibVersion = GetDllVersion(_T("comctl32.dll"));
	if (commonControlLibVersion < PACKVERSION(4,71))
	{
		AfxMessageBox(_T("ExifPro requires Common Control library\n(comctl32.dll) version 4.71 or higher.\n")
					  _T("Please reinstall ExifPro."), MB_OK | MB_ICONERROR);
		return false;
	}

	_set_invalid_parameter_handler(&InvalidParameterHandler);

//	_set_new_handler(&handle_out_of_memory);
//	_set_new_mode(1);

	FILETIME ft;
	::GetSystemTimeAsFileTime(&ft);

	// try to locate SHAutoComplete function
//	pfn_auto_complete_fn_ = FindSHAutoCompleteFn();

	// install translation fn for SEH to C++ exception
	SEException::Install();

	Scintilla_RegisterClasses(AfxGetInstanceHandle());

	SetRegistryKey(_T("ExifPro"));	// Registry key under which settings are stored.
	BOOL enable= AfxEnableMemoryTracking(FALSE);
	free(const_cast<TCHAR*>(m_pszProfileName));
	m_pszProfileName = _tcsdup(_T("3.x"));
	AfxEnableMemoryTracking(enable);

#ifdef USE_GDI_PLUS
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	// Initialize GDI+hhh
	Gdiplus::GdiplusStartup(&gdi_plus_token_, &gdiplusStartupInput, NULL);
#endif

	// Turn off the "Server Busy" dialog
	if (COleMessageFilter* filter= AfxOleGetMessageFilter())
	{
		filter->EnableBusyDialog(false);
		filter->EnableNotRespondingDialog(false);
		filter->SetMessagePendingDelay(60000); // 60 sec
	}

	Profile<bool> profileFirstTimeUp(_T("ExifPro"), _T("FirstTimeUp"), true);
	g_first_time_up = profileFirstTimeUp;
	profileFirstTimeUp = false;

	g_Settings.Restore();

	// initialize thumbnail image cache structures
	SetImageCache(g_Settings.image_cache_size_);

	// associate *.catalog files with ExifPro
	CString ExifProCatalog= _T("ExifPro.catalog");
	if (SetRegKey(_T(".catalog"), ExifProCatalog))
	{
		VERIFY(SetRegKey(ExifProCatalog, _T("ExifPro Image Catalog")));

		TCHAR path[_MAX_PATH + 16];
		VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), path, _MAX_PATH));
		_tcscat(path, _T(",1"));
		VERIFY(SetRegKey(ExifProCatalog + _T("\\DefaultIcon"), path));

		CString shell= ExifProCatalog + _T("\\shell\\open\\");

		VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), path, _MAX_PATH));
		_tcscat(path, _T(" \"%1\" /dde"));
		VERIFY(SetRegKey(shell + _T("command"), path));

		VERIFY(SetRegKey(shell + _T("ddeexec"), _T("[open(\"%1\")]")));
	}

	LoadStdProfileSettings(16);		// Load standard INI file options (including MRU)

	// create main MDI Frame window
	BrowserFrame* main_frame= new BrowserFrame;
	if (!main_frame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = main_frame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
//	RegisterShellFileTypes(TRUE);

	// check expiry date for beta copy
	CHECK_TIME_LIMIT

	// Parse command line for standard shell commands, DDE, file open
	CmdLineInfo cmdInfo;
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	ParseCommandLine(cmdInfo);

#ifdef _DEBUG
	//CString cmdLine;
	//for (int i= 1; i < __argc; i++)
	//{
	//	const TCHAR* param= __targv[i];
	//	cmdLine += param;
	//	cmdLine += _T('\n');
	//}
	//AfxMessageBox(cmdLine, MB_OK, 0);
#endif

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.

	//main_frame->ShowWindow(m_nCmdShow);
	//main_frame->UpdateWindow();
	main_frame->EnableWindow(false);

	//// initially set focus to the image pane
	//main_frame->InitialSetActiveView();

	if (cmdInfo.scan_sub_folders_ != CmdLineInfo::DEFAULT_SCAN)
		main_frame->SetRecursiveScan(cmdInfo.scan_sub_folders_ == CmdLineInfo::SCAN);

	if (cmdInfo.load_onlyEXIF_ != CmdLineInfo::DEFAULT_EXIF)
		main_frame->SetReadOnlyExif(cmdInfo.load_onlyEXIF_ == CmdLineInfo::ONLY_EXIF);

	if (cmdInfo.log_)
	{
		TCHAR buff[_MAX_PATH + 8];
		VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH));
		_tcscat(buff, _T(".log"));
		Logger::Open(buff);
	}

	bool goToFolder= true;
	bool allowSinglePhotoScan= true;
	bool showWindow= true;
	bool singlePhotoScan= false;

	if (cmdInfo.startTransferTool_)		// transfer files?
	{
		CString path= cmdInfo.m_strFileName;
		path.Trim(_T("\""));
//AfxMessageBox(path, MB_OK);
		// start transfer tool
		String dest= main_frame->Transfer(path);
		if (!dest.empty())	// this is where files were copied
		{
			main_frame->FolderSelected(dest.c_str());	// go there
			goToFolder = false;
			allowSinglePhotoScan = false;
		}
	}

	if (goToFolder && cmdInfo.start_scanning_)
	{
		try
		{
			FolderPathPtr path;
			bool favorite= false;
			if (!cmdInfo.m_strFileName.IsEmpty())	// folder passed to ExifPro.exe?
			{
				// attempt to create an object that encapsulates path to the folder;
				// if cmdInfo.m_strFileName points to a file instead, path to the folder this file in
				// will be used, and file name will be recorded too; this is how ExifPro knows to
				// open this file first in a viewer window (ExifProView does it)

				path = CreateFolderPath(cmdInfo.m_strFileName);//, allowSinglePhotoScan);
				if (path && !path->GetSelectedFile().empty() && allowSinglePhotoScan)
				{
					// if single (image) file has been passed to the ExifPro, turn off recursive
					// scanning of folders to speed viewer up a bit
					if (cmdInfo.scan_sub_folders_ == CmdLineInfo::DEFAULT_SCAN)
						main_frame->SetRecursiveScan(false);
					showWindow = false;
					main_frame->EnableSavingSettings(false);
					singlePhotoScan = true;
				}
			}
			else
			{
				if (main_frame->last_folder_ >= 0)
					favorite = main_frame->FavoriteFolder(main_frame->last_folder_);
				else
					path = main_frame->GetLastUsedFolder();
			}

			if (showWindow)
			{
				ShowMainWnd(main_frame, m_nCmdShow);
				showWindow = false;
			}

			if (path)
			{
				if (!main_frame->FolderSelected(path) && singlePhotoScan)
					return false; // exit now, single photo cannot be opened for some reason...
			}
			else if (!favorite)
				main_frame->Browser();
		}
		catch (...)
		{
			// invalid path encountered
			main_frame->Browser();
		}
	}

	if (showWindow)
		ShowMainWnd(main_frame, m_nCmdShow);

	return TRUE;
}


int App::ExitInstance()
{
#ifdef USE_GDI_PLUS
	// if shut down here, destruction of static GDI+ object will bomb
//	Gdiplus::GdiplusShutdown(gdi_plus_token_);
#endif
//	::CoUninitialize();

	if (::GetDeleteFlagForImageDataBase())
		::DeleteImageDataBase();

	::CloseHandle(app_event_);

	Scintilla_ReleaseResources();

	return CWinApp::ExitInstance();
}


// App command to run the dialog
void App::OnAppAbout()
{
	AboutDlg aboutDlg;
	aboutDlg.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// App message handlers

#include <shlwapi.h>

DWORD GetDllVersion(LPCTSTR dll_name)
{
	DWORD version= 0;

	if (HINSTANCE hinstDll= ::LoadLibrary(dll_name))
	{
		DLLGETVERSIONPROC pfnDllGetVersion= reinterpret_cast<DLLGETVERSIONPROC>(::GetProcAddress(hinstDll, "DllGetVersion"));

		// Because some DLLs might not implement this function, you
		// must test for it explicitly. Depending on the particular 
		// DLL, the lack of a DllGetVersion function can be a useful
		// indicator of the version.

		if (pfnDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi, sizeof dvi);
			dvi.cbSize = sizeof dvi;

			if (SUCCEEDED(pfnDllGetVersion(&dvi)))
				version = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
		}

		::FreeLibrary(hinstDll);
	}

	return version;
}


// if input path starts with . or ". then it'll be replaced by the same directory
// ExifPro.exe is running from (not the current one nor the working one)
//
CString App::RelativeToCurrentDir(const TCHAR* path)
{
//AfxMessageBox(path, MB_OK);

	int index= 0;

	if (path[0] == _T('.'))
		index = 1;
	else if (path[1] == _T('"') && path[0] == _T('.'))
		index = 2;

	CString dir;
	if (index > 0)
	{
		TCHAR prog_path[_MAX_PATH];
		VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), prog_path, _MAX_PATH));
		dir = Path(prog_path).GetDir().c_str();

		if (index == 2)
			dir += _T('"') + dir + (path + index);
		else
			dir += (path + index);
	}
	else
		dir = path;

	dir.Replace(_T("/"), _T("\\"));

//AfxMessageBox(dir, MB_OK);

	return dir;
}


void App::OnUpdateCheck()
{
	UpdateCheck dlg(0);

	dlg.DoModal();
}


namespace Tags { extern void FreeTagCollection(); }

App::~App()
{
	Tags::FreeTagCollection();
}


void App::DisableItem(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(false);
}
