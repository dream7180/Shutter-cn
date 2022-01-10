/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MainFrm.cpp : implementation of the MainFrame class
//

#include "stdafx.h"
#include "resource.h"
#include "MainFrm.h"
#include "FolderSelect.h"
#include "ItemIdList.h"
#include "MenuFolders.h"
#include "SetupFavoritesDlg.h"
#include "TransferDlg.h"
#include "HeaderDialog.h"
#include "Path.h"
#include "AddFavoriteDlg.h"
#include "CatchAll.h"
#include "MemoryStatus/MemoryStatDlg.h"
#include "UIElements.h"
#include "Config.h"
#include "GetDefaultGuiFont.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PREVIEW_VERSION	0
#define EXPIRY_YEAR 9012
#define EXPIRY_MONTH 4


extern bool g_first_time_up;
extern CString g_user_name;
extern void BuildCatalog(const TCHAR* path);
extern bool g_preview_version= PREVIEW_VERSION != 0;

/////////////////////////////////////////////////////////////////////////////
// MainFrame

IMPLEMENT_DYNAMIC(MainFrame, CFrameWnd)


BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_SET_FAVORITES, OnSetFavorites)
	ON_COMMAND(ID_ADD_TO_FAVORITES, OnAddToFavorites)
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(ID_FOLDER_LIST, OnUpdateFolderList)
	ON_COMMAND(ID_TASK_TRANSFER, OnTaskTransfer)
	ON_UPDATE_COMMAND_UI(ID_TASK_TRANSFER, OnUpdateTaskTransfer)
	ON_WM_SIZE()
	//ON_COMMAND(ID_HELP_WINDOW, OnHelpWindow)
	//ON_UPDATE_COMMAND_UI(ID_HELP_WINDOW, OnUpdateHelpWindow)
	//}}AFX_MSG_MAP
//	ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE1 + MAX_RECENT_PATHS - 1, OnUpdateRecentPath)
	ON_NOTIFY(RBN_CHILDSIZE, AFX_IDW_REBAR, OnReBarChildSize)
	ON_UPDATE_COMMAND_UI(ID_IMAGES_PANE, OnUpdateStatusPaneImage)
	ON_COMMAND(ID_FULL_SCREEN, OnFullScreen)
	ON_UPDATE_COMMAND_UI(ID_FULL_SCREEN, OnUpdateFullScreen)
	ON_COMMAND(SC_CLOSE, OnCmdClose)
	ON_COMMAND(SC_RESTORE, OnRestore)
	ON_COMMAND(ID_BUILD_CATALOG, OnBuildCatalog)
	ON_COMMAND(ID_MEMORY_STATUS, OnMemoryStatus)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

namespace {

	static UINT indicators[] =
	{
		ID_SEPARATOR,           // status line indicator
		ID_IMAGES_PANE
//		ID_INDICATOR_CAPS,
//		ID_INDICATOR_NUM,
//		ID_INDICATOR_SCRL,
	};

	const TCHAR* REGISTRY_SECTION_PATHS= _T("Paths");
	const TCHAR* REGISTRY_SECTION_FAVORITES= _T("Favorites");
	const TCHAR* REGISTRY_ENTRY_PATHS= _T("RecentPaths");
	const TCHAR* REG_RECENT_PATH= _T("%d");
	const TCHAR* REG_CAMERA_PATH= _T("CameraPath");
	const TCHAR* REG_FULL_SCREEN= _T("FullScreen");
	const TCHAR* REGISTRY_SECTION_MAIN_WND= _T("MainWnd");
	const int g_REBAR_GAP= 0;	 // gap below rebar
}


/////////////////////////////////////////////////////////////////////////////
// MainFrame construction/destruction

MainFrame::MainFrame() : wnd_pos_(REGISTRY_SECTION_MAIN_WND), status_bar_wnd_(columns_)
{
	favorite_folders_ = new FavoriteFolders(MAX_FAVORITE_FOLDERS);
	favorite_folders_->RetrieveFolders(REGISTRY_SECTION_FAVORITES);

	recent_path_list_ = new CRecentFileList(0, REGISTRY_ENTRY_PATHS, REG_RECENT_PATH, 20);
	recent_path_list_->ReadList();

	scanning_bar_ = false;

	full_screen_ = AfxGetApp()->GetProfileInt(REGISTRY_SECTION_MAIN_WND, REG_FULL_SCREEN, 0) != 0;
	style_ = 0;
	memset(&normal_state_, 0, sizeof normal_state_);
}


MainFrame::~MainFrame()
{
	favorite_folders_->StoreFolders(REGISTRY_SECTION_FAVORITES);

	// store recently used paths
	recent_path_list_->WriteList();
}

const int TIMER_ID= 123;

int MainFrame::OnCreate(LPCREATESTRUCT create_struct)
{
	// get placement, it will return whatever PreCreateWindow requested
	// then set it; this will trigger coordinate validation
	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
	{
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(&wp);
	}

	if (g_first_time_up)
		AddToFavoritesSystemFolders();

//	m_rectBorder.top = g_REBAR_GAP;
	accelerator_.Load(IDR_MAINFRAME);

	VERIFY(close_wnd_.Create(this));
//	LOG_FILENAME(log); log << "mainwnd: close wnd created\n";
	ShowCloseBar(false);

	if (CFrameWnd::OnCreate(create_struct) == -1)
		return -1;

	if (full_screen_)
		FullScreen();

	SetTimer(TIMER_ID, 3000, 0);

	return 0;
}


BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style |= WS_CLIPCHILDREN;

	CRect rect= wnd_pos_.GetLocation(false);

	cs.x = rect.left;
	cs.y = rect.top;
	cs.cx = rect.Width();
	cs.cy = rect.Height();
//cs.x=-4202;
	if (wnd_pos_.IsMaximized())
		AfxGetApp()->m_nCmdShow = SW_SHOWMAXIMIZED;

	return TRUE;
}


void MainFrame::OnDestroy()
{
	if (full_screen_)
		wnd_pos_.StoreState(normal_state_);
	else
		wnd_pos_.StoreState(*this);	// store current wnd position

	AfxGetApp()->WriteProfileInt(REGISTRY_SECTION_MAIN_WND, REG_FULL_SCREEN, full_screen_ ? 1 : 0);

	CFrameWnd::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// MainFrame diagnostics

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// MainFrame message handlers


//void MainFrame::OnUpdateRecentPath(CCmdUI* cmd_ui)
//{
	//ASSERT(cmd_ui->index_ >= 0 && cmd_ui->index_ < MAX_RECENT_PATHS);
	//cmd_ui->Enable(!(*recent_path_list_)[cmd_ui->index_].IsEmpty());
//}


void MainFrame::OnUpdateFolderList(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


// set favorites folders
//
void MainFrame::OnSetFavorites()
{
	SetupFavoritesDlg dlg(*favorite_folders_, this);

	if (dlg.DoModal() == IDOK)
	{
		favorite_folders_->StoreFolders(REGISTRY_SECTION_FAVORITES);
	}
}


// add new folder to favorites
//
void MainFrame::OnAddToFavorites()
{
	try
	{
		if (favorite_folders_->GetCount() >= MAX_FAVORITE_FOLDERS)
			AfxMessageBox(_T("没有可用的空间."), MB_OK);
		else
			AddToFavorites(-1);
	}
	CATCH_ALL
}


void MainFrame::AddToFavorites(int folder_index)
{
	CFolderSelect dlg(this);
	FavoriteFolder* folder= folder_index >= 0 ? favorite_folders_->GetFolder(folder_index) : 0;

	const TCHAR* prompt= _T("选择收藏夹目录:");

	if (folder)				// existing folder?
	{
		ITEMIDLIST* idl= folder->GetIDL();
		ItemIdList idlFolder(dlg.DoSelect(prompt, idl), false);
		if (idlFolder.IsEmpty())
			return;

		// modify folder
		*folder = idlFolder;
	}
	else if (folder_index < 0)	// new folder
	{
		FolderPathPtr path= GetCurrentFolder();
		if (path == 0)
			return;
		ItemIdList idlFolder= path->GetPIDL();
		if (idlFolder.IsEmpty())
			return;

		AddFavoriteDlg dlg;
		dlg.name_ = idlFolder.GetName();
		dlg.path_ = idlFolder.GetPath();

		if (dlg.DoModal() == IDOK)
		{
			// add current folder to favorites
			folder = favorite_folders_->AddNewFolder(idlFolder, dlg.name_, true);
		}
	}
}


void MainFrame::AddToFavoritesSystemFolders()
{
	ItemIdList idlPictures(CSIDL_MYPICTURES);

	if (idlPictures.IsInitialized())
		favorite_folders_->AddNewFolder(idlPictures);

	ItemIdList idlDocuments(CSIDL_PERSONAL);

	if (idlDocuments.IsInitialized())
		favorite_folders_->AddNewFolder(idlDocuments);
}

///////////////////////////////////////////////////////////////////////////////

void MainFrame::GetMessageString(UINT id, CString& message) const
{
	if (id >= ID_BROWSE_FOLDER_1 && id < ID_BROWSE_FOLDER_1 + MAX_FAVORITE_FOLDERS)
	{
		int index= id - ID_BROWSE_FOLDER_1;
		if (FavoriteFolder* folder= favorite_folders_->GetFolder(index))
		{
			message = _T("浏览文件夹 ");
			message += folder->GetIDL().GetPath();
		}
	}
	else if (id >= ID_RECENT_PATH_FIRST && id <= ID_RECENT_PATH_LAST)
	{
		message = _T("扫描最近使用的文件夹");
	}
	else if (id >= ID_MASK_JPEG && id < ID_MASK_JPEG + MAX_FILE_MASKS)
	{
		message = _T("扫描文件夹时查找的图像类型");
	}
	else if (id >= ID_PANES_LAYOUT_00 && id <= ID_PANES_LAYOUT_99)
	{
		message = _T("恢复本面板窗口");
	}
	else if (id >= AFX_IDW_PANE_FIRST && id <= AFX_IDW_PANE_LAST)
	{
		message = _T("切换面板窗口");
	}
	else if (id >= ID_SORTING_ORDER && id <= ID_SORTING_ORDER + 999)
	{
		message = _T("按此列排序; 再次选择此项将进行反向排序");
	}
	else if (id == AFX_IDS_IDLEMESSAGE)
	{
		// replace 'Ready' message by photo's info
		message = GetStatusReadyText();

		if (message.IsEmpty())
			CFrameWnd::GetMessageString(id, message);
	}
	else
	{
		CString str;
		if (str.LoadString(id))
		{
			int index= str.Find(_T('\n'));
			if (index > 0)
				str = str.Left(index);

			index = str.Find(_T('('));
			if (index > 0)
				message = str.Left(index);
			else
				message = str;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	COLORREF backgnd= g_Settings.AppColors()[AppColors::Background]; // ::GetSysColor(COLOR_3DFACE);

	// this is only visible below close btn in full screen mode
	dc->FillSolidRect(rect, backgnd);

	// close bar separator line--it's safe to draw it even when close bar is invisible
	//CRect line(CPoint(rect.right - CloseBar::CLOSE_BAR_WIDTH, rect.top), CSize(4, 199));
	//DrawVertLineSeparator(*dc, line, &backgnd);

	// give derived class chance to draw separator
	DrawSeparator(*dc, rect);//horizontal top toolbar & vertical task bar 1px seprator

	return true;
}


void MainFrame::DrawSeparator(CDC&, const CRect&)	// NoOp
{}


void MainFrame::OnUpdateFrameTitle(BOOL add_to_title)
{
	CFrameWnd::OnUpdateFrameTitle(add_to_title);

//	if (docking_bar_wnd_.m_hWnd)
//		docking_bar_wnd_.Refresh();
}

///////////////////////////////////////////////////////////////////////////////

// position rebar bands: bottom band (windows bar) should be as wide as rebar itself
//
void MainFrame::OnReBarChildSize(NMHDR* nmhdr, LRESULT* result)
{
	if (NMREBARCHILDSIZE* rbcs= reinterpret_cast<NMREBARCHILDSIZE*>(nmhdr))
	{
		if (rbcs->uBand == 0 - 1)
			return;

		CRect rect;
		GetClientRect(rect);
		int right= rect.Width();

		switch (rbcs->uBand)
		{
		case 0:
			break;

		case 1:
			rbcs->rcChild.right = rbcs->rcBand.right;
			break;

		default:
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void MainFrame::OnUpdateStatusPaneImage(CCmdUI* cmd_ui)
{
	CString stat= GetStatusPaneText();
	cmd_ui->SetText(stat);
	if (scanning_bar_ != IsScanning())
		scanning_bar_ = ShowScanningBar(!scanning_bar_);
}


CRect MainFrame::GetScanningWndRect()
{
	CRect rect(0, 0, 0, 0);
	status_bar_wnd_.GetItemRect(1, rect); // this lousy thing lies when statbar is owner drawn
	if (rect.Width() > 0 && rect.Height() > 0)
	{
		const int PROGRESS_ANIM_WIDTH= 68;
		rect.right -= 20;
		rect.left = rect.right - PROGRESS_ANIM_WIDTH;
		rect.top += rect.Height()/2-4;//3;
		rect.bottom = rect.top + 9;
	}
	return rect;
}


bool MainFrame::ShowScanningBar(bool show)
{
	if (scanning_wnd_.m_hWnd == 0)
	{
		CRect rect= GetScanningWndRect();
		if (rect.Width() > 0 && rect.Height() > 0)
		{
			scanning_wnd_.Create(WS_CHILD, rect, &status_bar_wnd_, 100);
			scanning_wnd_.Open(IDR_PROGRESS_BAR);
		}
		else
			return false;
	}

	if (show)
	{
		scanning_wnd_.Play(0, -1, -1);
		scanning_wnd_.ShowWindow(SW_SHOWNA);
	}
	else
	{
		scanning_wnd_.Stop();
		scanning_wnd_.ShowWindow(SW_HIDE);
	}

	return show;
}


///////////////////////////////////////////////////////////////////////////////


void MainFrame::UpdateStatusBar()
{
	status_bar_wnd_.OnUpdateCmdUI(this, true);
}


BOOL MainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* context)
{
	//ModifyStyle(WS_THICKFRAME, 0);//remove SBARS_SIZEGRIP
	if (!status_bar_wnd_.Create(this, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | CBRS_BOTTOM) ||
		!status_bar_wnd_.SetIndicators(indicators, array_count(indicators)))
	{
		TRACE0("Failed to create status bar\n");
		return false;      // fail to create
	}
	//ModifyStyle(0, WS_THICKFRAME);//recover thickframe after removing SBARS_SIZEGRIP
	status_bar_wnd_.SetPaneStyle(0, SBPS_OWNERDRAW | SBPS_STRETCH | SBPS_NOBORDERS);
	status_bar_wnd_.SetPaneStyle(1, SBPS_OWNERDRAW | SBPS_NOBORDERS);
	LOGFONT lf;
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);
	status_bar_wnd_.SendMessage(WM_SETFONT, WPARAM(hfont));
	//status_bar_wnd_.SendMessage(WM_SETFONT, WPARAM(::GetDefaultGuiHFont()));

	return CFrameWnd::OnCreateClient(lpcs, context);
}


void MainFrame::OnTaskTransfer()
{
	Transfer(0);
}

String MainFrame::Transfer(const TCHAR* path)
{
	CTransferDlg dlgTr(this, path);
	HeaderDialog dlg(dlgTr, _T("转移"), HeaderDialog::IMG_COPY);
	dlg.DoModal();
	return dlgTr.GetDestPath();
}

void MainFrame::OnUpdateTaskTransfer(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void MainFrame::OnSize(UINT type, int cx, int cy)
{
	CFrameWnd::OnSize(type, cx, cy);

	if (scanning_wnd_.m_hWnd)
		scanning_wnd_.MoveWindow(GetScanningWndRect());
}

/*
void MainFrame::OnHelpWindow()
{
	WinHelp(0, 0);
}

void MainFrame::OnUpdateHelpWindow(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


extern void OpenHelp(const TCHAR* initial_page)
{
	TCHAR buff[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH));
	size_t len= _tcslen(buff);
	if (len < 4 || buff[len - 4] != _T('.'))
		return;
	_tcscpy(&buff[len - 3], _T("chm"));

	if (Path(buff).FileExists())
	{
		CWaitCursor wait;
		CString str= buff;
		if (initial_page && *initial_page)
		{
			str += _T("::/");
			str += initial_page;
		}
		::ShellExecute(0, _T("open"), _T("hh.exe"), str, 0, SW_SHOWNORMAL);
	}
	else
	{
		CString msg= _T("帮助文件 ");
		msg += buff;
		msg += _T(" 未能找到.");
		AfxMessageBox(msg, MB_OK | MB_ICONERROR);
	}
}


void MainFrame::WinHelp(DWORD, UINT)
{
	OpenHelp(0);

//	CFrameWnd::WinHelp(data, cmd);
}
*/

HACCEL MainFrame::GetDefaultAccelerator()
{
	return accelerator_.GetHandle();
}


void MainFrame::OnFullScreen()
{
	if (full_screen_)
		WndMode();
	else
		FullScreen();
}


void MainFrame::OnUpdateFullScreen(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(full_screen_ ? 1 : 0);
}


void MainFrame::WndMode()
{
	ShowCloseBar(false);

	full_screen_ = false;

	if (normal_state_.showCmd == SW_SHOWMAXIMIZED)	// was wnd maximized prior to entering full screen mode?
	{
		SetWindowLong(m_hWnd, GWL_STYLE, style_);
		SetWindowPlacement(&normal_state_);
		SetWindowPos(0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	else
	{
		ModifyStyle(WS_MAXIMIZE, style_ & ~WS_MAXIMIZE);
		CRect rect= normal_state_.rcNormalPosition;
		SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	}

	InformTaskBar();
}


void MainFrame::FullScreen()
{
	CRect window_rect(0,0,0,0);
	GetWindowRect(window_rect);

	// full screen rect (of the monitor this window is displayed in)
	CRect screen_rect= ::GetFullScreenRect(window_rect);

	style_ = GetStyle();

	if (!GetWindowPlacement(&normal_state_))
	{
		ASSERT(false);
		return;
	}

	ModifyStyle(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, WS_MAXIMIZE);

	CRect rect= screen_rect;
	::AdjustWindowRect(rect, GetStyle(), false);

	full_screen_ = true;

	SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);

	InformTaskBar();
}


void MainFrame::InformTaskBar()	// help taskbar recognize full-screen window
{
	_COM_SMARTPTR_TYPEDEF(ITaskbarList2, __uuidof(ITaskbarList2));
	ITaskbarList2Ptr task_bar;

	if (SUCCEEDED(task_bar.CreateInstance(CLSID_TaskbarList)))
		task_bar->MarkFullscreenWindow(m_hWnd, full_screen_);
}


void MainFrame::ShowCloseBar(bool show)
{
	if (show)
	{
		close_wnd_.EnableWindow();
		close_wnd_.ShowWindow(SW_SHOWNA);
	}
	else
	{
		close_wnd_.ShowWindow(SW_HIDE);
		close_wnd_.EnableWindow(false);
	}
}


void MainFrame::RecalcLayout(BOOL notify/*= TRUE*/)
{
	CFrameWnd::RecalcLayout(notify);

	if (full_screen_)
	{
		if (CWnd* rebar= GetDlgItem(AFX_IDW_REBAR))
		{
			CRect bar_rect;
			rebar->GetWindowRect(bar_rect);

			CRect rect;
			GetClientRect(rect);

			int width= rect.Width() - CloseBar::CLOSE_BAR_WIDTH;

			rebar->SetWindowPos(0, 0, 0, width, bar_rect.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			close_wnd_.SetWindowPos(0, rect.Width() - CloseBar::CLOSE_BAR_WIDTH + 4, 2, CloseBar::CLOSE_BAR_WIDTH - 4, bar_rect.Height() - 2, SWP_NOZORDER | SWP_NOACTIVATE);

			ShowCloseBar(true);
		}
	}
}


void MainFrame::OnCmdClose()
{
	DestroyWindow();
}


void MainFrame::OnRestore()
{
	if (full_screen_)
		WndMode();
}


void MainFrame::OnBuildCatalog()
{
	try
	{
		BuildCatalog(0);
	}
	CATCH_ALL
}


void MainFrame::OnMemoryStatus()
{
	try
	{
		MemoryStatDlg dlg;
		dlg.DoModal();
	}
	CATCH_ALL
}


LRESULT MainFrame::OnPrintClient(WPARAM hdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(hdc)))
		OnEraseBkgnd(dc);

	return 1;
}
