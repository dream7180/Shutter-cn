/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ChildFrm.cpp : implementation of the BrowserFrame class
//

#include "stdafx.h"
#include "resource.h"
#include "BrowserFrame.h"
#include "PhotoInfo.h"
#include "ItemIdList.h"
#include "MenuFolders.h"
#include "Config.h"
#include "BalloonMsg.h"
#include "CatchAll.h"
#include "ProfileVector.h"
#include "PreviewPane.h"
#include "TaskBarView.h"
#include "OptionsDlg.h"
#include "PaneNotification.h"
#include "ManagePaneLayouts.h"
#include "MenuFolders.h"
#include "TimeLimit.h"
#include "FolderPathHelpers.h"
#include "WhistlerLook.h"
#include "CustomColumnsDlg.h"
#include "HeaderDialog.h"
#include "TagsCommonCode.h"
#include "UIElements.h"
#include "Color.h"

#ifdef _DEBUG
#define new		new(__FILE__, __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void BuildCatalog(const TCHAR* path);

/////////////////////////////////////////////////////////////////////////////
// BrowserFrame

IMPLEMENT_DYNCREATE(BrowserFrame, MainFrame)

BEGIN_MESSAGE_MAP(BrowserFrame, MainFrame)
	//{{AFX_MSG_MAP(BrowserFrame)
	ON_COMMAND(ID_INFO_BAR, OnInfoBar)
	ON_UPDATE_COMMAND_UI(ID_INFO_BAR, OnUpdateInfoBar)
	ON_COMMAND(ID_FOLDERS, OnFolders)
	ON_UPDATE_COMMAND_UI(ID_FOLDERS, OnUpdateFolders)
	ON_WM_DESTROY()
	ON_COMMAND(ID_PREVIEW, OnPreview)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW, OnUpdatePreview)
	ON_COMMAND(ID_BROWSER, OnBrowser)
	ON_UPDATE_COMMAND_UI(ID_BROWSER, OnUpdateBrowser)
	ON_UPDATE_COMMAND_UI(ID_ENTER, OnUpdateEnter)
	ON_UPDATE_COMMAND_UI(ID_NEXT_PANE, OnUpdateNextPane)
	ON_UPDATE_COMMAND_UI(ID_PREV_PANE, OnUpdateNextPane)	// same as next
	ON_COMMAND(ID_NEXT_PANE, OnNextPane)
	ON_COMMAND(ID_PREV_PANE, OnPreviousPane)
	ON_COMMAND(ID_TAGS_BAR, OnTagsBar)
	ON_UPDATE_COMMAND_UI(ID_TAGS_BAR, OnUpdateTagsBar)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
	ON_COMMAND(ID_VIEW_ADDRESSBAR, OnViewAddressbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ADDRESSBAR, OnUpdateViewAddressbar)
	ON_COMMAND(ID_VIEW_FILTERBAR, OnViewFilterbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILTERBAR, OnUpdateViewFilterbar)
	ON_COMMAND(ID_VIEW_TOOLS, OnViewTools)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLS, OnUpdateViewTools)
	ON_COMMAND(ID_PATH_TYPED, OnPathTyped)
	ON_COMMAND(ID_ADDRESS_BAR, OnAddressBar)
	ON_UPDATE_COMMAND_UI(ID_ADDRESS_BAR, OnUpdateAddressBar)
	ON_COMMAND(ID_PATH_CANCELLED, OnPathCancelled)
	ON_COMMAND(ID_FIND_BOX, OnFindBox)
	ON_COMMAND(ID_FILE_MASK, OnFileMask)
	ON_UPDATE_COMMAND_UI(ID_FILE_MASK, OnUpdateFileMask)
	ON_COMMAND(ID_OPTIONS, OnOptions)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS, OnUpdateOptions)
	ON_UPDATE_COMMAND_UI(ID_PANE_MENU, OnUpdatePaneMenu)
	ON_COMMAND(ID_PANE_ZOOM, OnPaneZoom)
	ON_UPDATE_COMMAND_UI(ID_PANE_ZOOM, OnUpdatePaneZoom)
	ON_WM_CREATE()
	ON_COMMAND(ID_PANES_MANAGE_LAYOUTS, OnPanesManageLayouts)
	ON_UPDATE_COMMAND_UI(ID_PANES_MANAGE_LAYOUTS, OnUpdatePanesManageLayouts)
	ON_COMMAND(ID_PANES_STORE_LAYOUT, OnPanesStoreLayout)
	ON_UPDATE_COMMAND_UI(ID_PANES_STORE_LAYOUT, OnUpdatePanesStoreLayout)
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_CONTROLBAR_LAST, OnTbDropDown)
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_CONTROLBAR_LAST - 1, OnTbDropDown)
	ON_UPDATE_COMMAND_UI_RANGE(ID_BROWSE_FOLDER_1, ID_BROWSE_FOLDER_1 + MAX_FAVORITE_FOLDERS - 1, OnUpdateFavoriteFolder)
	ON_COMMAND_RANGE(ID_BROWSE_FOLDER_1, ID_BROWSE_FOLDER_1 + MAX_FAVORITE_FOLDERS - 1, OnFavoriteFolder)
	ON_COMMAND(ID_FOLDER_LIST, OnFolderList)
	ON_COMMAND_RANGE(ID_MASK_JPEG, ID_MASK_JPEG + MAX_FILE_MASKS - 1, OnFileMask)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MASK_JPEG, ID_MASK_JPEG + MAX_FILE_MASKS - 1, OnUpdateFileMaskRng)
	ON_COMMAND_RANGE(ID_PANES_LAYOUT_00, ID_PANES_LAYOUT_99, OnRestorePaneLayout)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PANES_LAYOUT_00, ID_PANES_LAYOUT_99, OnUpdateRestorePaneLayout)
	ON_COMMAND(ID_PANE_MENU, OnPanePopupMenu)
	ON_WM_MEASUREITEM()
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_ORDER, ID_SORTING_ORDER + 999, EnableSortingOption)
	ON_COMMAND_RANGE(ID_SORTING_ORDER, ID_SORTING_ORDER + 999, OnSortingOption)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_USER+1234, OnCloseApp)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RECENT_PATH_FIRST, ID_RECENT_PATH_LAST, EnableRecentFolder)
	ON_COMMAND(ID_BUILD_IMG_CATALOG, OnBuildImgCatalog)
	ON_COMMAND(ID_DEFINE_CUSTOM_COLUMNS, OnDefineCustomColumns)
	ON_UPDATE_COMMAND_UI(ID_DEFINE_CUSTOM_COLUMNS, OnUpdateDefineCustomColumns)
	ON_WM_SIZE()
	ON_COMMAND(ID_FILTER_PHOTOS, OnFilterPhotos)
	ON_COMMAND(ID_CANCEL_FILTER, OnCancelFilter)
	ON_COMMAND(ID_MASK_ALL, OnLoadAllTypes)
	ON_COMMAND(ID_MASK_JPEG_ONLY, OnLoadJpegOnly)
	ON_COMMAND(ID_MASK_RAW_ONLY, OnLoadRawOnly)
	ON_COMMAND(ID_TOOLBAR_HORIZONTAL, OnToolbarHorizontal)
	ON_COMMAND(ID_TOOLBAR_VERTICAL, OnToolbarVertical)
END_MESSAGE_MAP()


namespace {
	const TCHAR REGISTRY_SECTION_BROWSER[]	= _T("Browser\\View %d");
	const TCHAR REGISTRY_SECTION_CAMERA[]	= _T("Browser\\Camera");
	const TCHAR REG_FOLDERS_WIDTH[]			= _T("FoldersWidth");
	const TCHAR REG_SHOW_FOLDERS[]			= _T("ShowFolders");
	const TCHAR REG_SHOW_TASKS[]			= _T("ShowTasks");
	const TCHAR REG_INFOBAR_WIDTH[]			= _T("InfoBarWidth");
	const TCHAR REG_SHOW_INFOBAR[]			= _T("ShowInfoBar");
	const TCHAR REG_VIEW_SETTINGS[]			= _T("Settings");
	const TCHAR REG_LAST_FAVORITE[]			= _T("LastFavorite");
	const TCHAR REG_LAST_PATH[]				= _T("LastPath");
	const TCHAR REG_REBAR_LAYOUT[]			= _T("ReBarLayout");
	const TCHAR REG_STATUSBAR_FIELDS[]		= _T("StatusBarFields");
	const TCHAR REG_CUSTOM_COLUMNS[]		= _T("CustomColumns");
	const TCHAR CUSTOM_COLUMN_NAME[]		= _T("Caption");
	const TCHAR CUSTOM_COLUMN_EXPR[]		= _T("Expression");
}


/////////////////////////////////////////////////////////////////////////////
// BrowserFrame construction/destruction

BrowserFrame::BrowserFrame() : exif_view_wnd_(columns_)
{
	reg_section_.Format(REGISTRY_SECTION_BROWSER, 0);
	last_folder_ = -1;
	initialized_ = false;
	save_settings_ = false;

	Tags::OpenTags(Tags::GetTagsPathName().c_str(), Tags::GetTagCollection());

	toolbar_size_ = CSize(0, 0);

	exif_view_wnd_.ConnectReloadProgress(boost::bind(&BrowserFrame::OnReloadProgress, this, _1, _2));
	exif_view_wnd_.ConnectFilterChanged(boost::bind(&BrowserFrame::OnFilterStateChanged, this, _1));

	addr_box_wnd_.ConnectRunCommand(boost::bind(&BrowserFrame::OnRunCommand, this, _1));
	filter_bar_wnd_.ConnectRunCommand(boost::bind(&BrowserFrame::OnRunCommand, this, _1));

	tools_visible_.Register(reg_section_, L"ToolsBarVisible", true);
	tools_horizontal_.Register(reg_section_, L"ToolsBarHorizontal", false);
}


BrowserFrame::~BrowserFrame()
{
	menu_sort_order_popup_.Detach();
	menu_pane_windows_popup_.Detach();
}


void BrowserFrame::OnReloadProgress(ReloadJob::Progress progress, size_t images)
{
	switch (progress)
	{
	case ReloadJob::RELOADING_START:
		addr_box_wnd_.SetScan(true);
		UpdateStatusBar();
		break;

	case ReloadJob::RELOADING_FINISHED:
	case ReloadJob::RELOADING_CANCELLED:
	case ReloadJob::RELOADING_QUIT:
		addr_box_wnd_.SetScan(false);
		UpdateStatusBar();
		break;

	// on idle update takes care of progress (I guess)
//	case ReloadJob::RELOADING_PENDING:
//		break;

	default:
		break;
	};
}


void BrowserFrame::OnFilterStateChanged(bool active)
{
	filter_bar_wnd_.FilterOn(active);
}


void BrowserFrame::OnRunCommand(int cmd)
{
	OnCommand(cmd, 0);
}


namespace {	// pane windows creation

	template <class View>
	struct PaneFactory : PaneConstruction
	{
		PaneFactory(View* pane) : PaneConstruction(pane), pane_(pane)
		{}

		virtual bool CreateWin(CWnd* parent)	{ return pane_->Create(parent); }

		View* pane_;
	};

	template <class View>
	inline PaneFactory<View> GetFactory(View* object)
	{
		return PaneFactory<View>(object);
	}
}


int BrowserFrame::OnCreate(LPCREATESTRUCT create_struct)
{
	// this OnCreate will call OnCreateClient() below and then RecalcLayout()
	if (MainFrame::OnCreate(create_struct) == -1)
		return -1;

	// at this point pane windows are created, layout is read from registry,
	// panes are resized; synch visibility flag now: open panes will have
	// a chance to update themselves
	SendPaneNotification(frame_wnd_, &PaneWnd::SyncVisibilityFlag);

	// once everything is up and running saving settings on destroy makes sense; setting initialized_ flag
	initialized_ = true;
	save_settings_ = true;

	return 0;
}


BOOL BrowserFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* context)
{
	if (!MainFrame::OnCreateClient(lpcs, context))
		return false;

	try
	{
		return CreateWindows();
	}
	CATCH_ALL

	return false;
}


void BrowserFrame::PlaceToolbar()
{
	if (toolbar_.m_hWnd && frame_wnd_.m_hWnd && (toolbar_.GetStyle() & WS_VISIBLE) != 0)
	{
		WINDOWPLACEMENT wp;
		if (frame_wnd_.GetWindowPlacement(&wp))
		{
			if (toolbar_.IsHorizontal())
			{
				int y= wp.rcNormalPosition.bottom;
				int w= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				CSize size(w, toolbar_size_.cy);
				toolbar_.SetWindowPos(nullptr, 2, y + 2, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				int x= wp.rcNormalPosition.right;
				int y= wp.rcNormalPosition.top;
				int h= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
				CSize size(toolbar_size_.cx, h);
				toolbar_.SetWindowPos(nullptr, x + 2, y, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
	}
}


void BrowserFrame::OnSize(UINT type, int cx, int cy)
{
	MainFrame::OnSize(type, cx, cy);
	PlaceToolbar();
}

static const int BOTTOM_GAP= 0;
static const int RIGHT_GAP = 0;
static const int TOP_GAP = Pixels(2);

bool BrowserFrame::CreateWindows()
{
	struct InitData
	{
		const ItemIdList* folder_list;
		bool open_as_root;
	}* init= 0;

	if (init && init->folder_list)
		folder_name_ = init->folder_list->GetName();
	else
		folder_name_.Empty();

	SetColors(g_Settings.AppColors());

	if (!rebar_wnd_.Create(this, false))
		return false;

	//rebar_wnd_.SetBottomMargin(10);	// pixel gap for some breathing space

	// two pixels high border for a little shade below rebar control
	auto gap = SnapView::GetSeparatorThickness();
	m_rectBorder.top = gap.cy + TOP_GAP;
	m_rectBorder.bottom = gap.cy;
	// tool bar
	bool vert_tbar= !tools_horizontal_;
	if (toolbar_.Create(this, AFX_IDW_CONTROLBAR_LAST - 10, 0, vert_tbar))
	{
		toolbar_size_ = toolbar_.GetSize();
//		toolbar_size_ = toolbar_.IsHorizontal() ? size.cy : size.cx;
		toolbar_.ModifyStyle(0, WS_VISIBLE);
		if (tools_visible_)	// reserve space for toolbar in the main frame window
		{
			if (toolbar_.IsHorizontal())
				m_rectBorder.bottom = toolbar_size_.cy + gap.cy;	// add gap at the bottom
			else
				m_rectBorder.right = toolbar_size_.cx + gap.cx;
		}
		else
		{
			toolbar_.ShowWindow(SW_HIDE);
			//m_rectBorder.bottom = gap;
		}
	}
	else
	{ ASSERT(false); }

	if (!menu_bar_wnd_.Create(this, IDR_MAINFRAME))
		return false;

	HMENU first_popup= ::GetSubMenu(menu_bar_wnd_.GetMenu(), 0);
	HMENU favorites= ::GetSubMenu(first_popup, 1);
	if (favorites == 0)
	{
		ASSERT(false);	// wrong popup position
		return false;
	}
	folders_popup_.reset(new CMenuFolders(favorites));

	//======================= MAGIC NUMBERS =========================
	// hardcoded menu positions -------------------------------------
	//
	HMENU third_popup= ::GetSubMenu(menu_bar_wnd_.GetMenu(), 2);
	HMENU pane_windows= ::GetSubMenu(third_popup, 3);
	HMENU sort_order= ::GetSubMenu(third_popup, 20);
	if (sort_order == 0 || pane_windows == 0)
	{
		ASSERT(false);	// wrong popup position
		return false;
	}
	menu_sort_order_popup_.Attach(sort_order);
	menu_pane_windows_popup_.Attach(pane_windows);

	if (!toolbar_wnd_.Create(this, AFX_IDW_CONTROLBAR_LAST, AFX_IDW_CONTROLBAR_LAST - 1, BAND_TOOLBAR))
		return false;

	if (!filter_bar_wnd_.Create(this))
		return false;

	if (!addr_box_wnd_.Create(this))
		return false;

	addr_box_wnd_.SetHistory(*recent_path_list_);

	rebar_wnd_.AddBand(&menu_bar_wnd_, BAND_MENU);

	CRect rect;
	toolbar_wnd_.GetWindowRect(rect);
	rebar_wnd_.AddBand(&toolbar_wnd_, rect.Size(), 0, BAND_TOOLBAR, rect.Width(), true);

	bool underscore= WhistlerLook::IsAvailable(); //g_common_control_lib_version >= PACKVERSION(6,0);
	
	filter_bar_wnd_.GetWindowRect(rect);
	rebar_wnd_.AddBand(&filter_bar_wnd_, CSize(300, rect.Height()), underscore ? _T("Fi&nd") : _T("Find"), BAND_FILTER, 100);

	addr_box_wnd_.GetWindowRect(rect);
	rebar_wnd_.AddBand(&addr_box_wnd_, CSize(300, rect.Height()), underscore ? _T("A&ddress") : _T("Address"), BAND_ADDRESS, 150);

	rebar_wnd_.MaximizeBand(0);

	rebar_wnd_.RestoreLayout(reg_section_, REG_REBAR_LAYOUT);

	CWinApp* app= AfxGetApp();

	// restore custom columns
	CString section= GetRegSection();
	section += '\\';
	section += REG_CUSTOM_COLUMNS;

	CustomColumns custom_cols;
	for (size_t i= 0; i < custom_cols.size(); ++i)
	{
		CustomColumnDef& col= custom_cols[i];

		TCHAR key[200];
		wsprintf(key, _T("%s-%d"), CUSTOM_COLUMN_NAME, static_cast<int>(i));
		col.caption_ = static_cast<const TCHAR*>(app->GetProfileString(section, key, col.caption_.c_str()));
		wsprintf(key, _T("%s-%d"), CUSTOM_COLUMN_EXPR, static_cast<int>(i));
		col.expression_ = static_cast<const TCHAR*>(app->GetProfileString(section, key, col.expression_.c_str()));
	}
	columns_.SetCustomColumns(custom_cols);

	// default create client will create a view if asked for it
	CCreateContext ctx;
	ctx.m_pCurrentDoc = 0;
	ctx.m_pCurrentFrame = this;
	ctx.m_pLastView = 0;
	ctx.m_pNewDocTemplate = 0;
	ctx.m_pNewViewClass = 0;

	frame_wnd_.Create(this, _T("frame"), AFX_IDW_PANE_FIRST);

	if (!WhistlerLook::IsAvailable())
		frame_wnd_.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);

	ctx.m_pCurrentFrame = &frame_wnd_;

	const ITEMIDLIST* idl= 0;
	if (init && init->folder_list)
		idl = *init->folder_list;
	bool root= init ? init->open_as_root : true;

	last_folder_ = app->GetProfileInt(reg_section_, REG_LAST_FAVORITE, -1);
	try
	{
		last_used_path_ = RestoreFolderPath(reg_section_, REG_LAST_PATH);
	}
	catch(...)	// missing catalog or bogus folder are fine
	{
		last_used_path_ = 0;
	}

	folders_wnd_.on_folder_changed_ = boost::bind(&BrowserFrame::FolderSelectedEx, this, _1);
	folders_wnd_.on_recent_folders_ = boost::bind(&BrowserFrame::RecentFolders, this, _1);
	folders_wnd_.on_folder_up_ = boost::bind(&BrowserFrame::FolderUp, this);
	folders_wnd_.on_update_folder_up_ = boost::bind(&BrowserFrame::UpdateFolderUp, this);

	exif_view_wnd_.SetFrame(this);

	static PaneFactory<FolderPane> folders=			GetFactory(&folders_wnd_);
	static PaneFactory<PreviewPane> preview=		GetFactory(&preview_wnd_);
	static PaneFactory<ExifView> mainview=			GetFactory(&exif_view_wnd_);
	static PaneFactory<InfoPane> infobar=			GetFactory(&info_bar_wnd_);
	static PaneFactory<HistogramPane> histogram=	GetFactory(&histogram_wnd_);
	static PaneFactory<TagBarView> tagbar=			GetFactory(&tag_bar_wnd_);

//	PaneLayoutInfo test(folders,		CRect( 0,  0,  20,  60), _T("Folders"),		PANE_NORMAL, -1, EDGE_NONE, EDGE_LEFT);

	static PaneLayoutInfo layout[]=
	{
		// initial layout of pane windows; all sizes are relative (0-100%) and provided as corners: left-top, right-bottom.
		// entire window area has to be covered by panes, without overlaps, but some of them may be hidden
		PaneLayoutInfo(folders,   CRect( 0,  0,  20,  60), L"Folders",   PANE_NORMAL | PANE_NO_CLOSE | PANE_NO_MAXIMIZE, -1, EDGE_NONE, EDGE_LEFT),
		PaneLayoutInfo(tagbar,    CRect( 0, 60,  20,  70), L"Tags",      PANE_NORMAL | PANE_NO_CLOSE | PANE_NO_MAXIMIZE, 0, EDGE_BOTTOM, EDGE_NONE, 200 / 8),
		PaneLayoutInfo(histogram, CRect( 0, 70,  20, 100), L"Histogram", PANE_HIDDEN | PANE_NO_CLOSE, 0, EDGE_BOTTOM, EDGE_NONE, 200 / 8),
		PaneLayoutInfo(preview,   CRect(20,  0,  75,  35), L"Preview",   PANE_NORMAL, -1, EDGE_NONE, EDGE_NONE),
		PaneLayoutInfo(mainview,  CRect(20, 35,  75, 100), L"Images",    PANE_NORMAL | PANE_NO_CLOSE, -1, EDGE_NONE, EDGE_NONE),
		PaneLayoutInfo(infobar,   CRect(75,  0, 100, 100), L"Info ",     PANE_HIDDEN, -1, EDGE_NONE, EDGE_RIGHT, 300 / 8),
	};
	static PaneLayoutInfoArray panes(layout, array_count(layout), _T("PaneLayout"), _T("Current"), _T("Name"), 0);

	FramePageCreateContext fcc(ctx, 0);

	if (!frame_wnd_.CreatePanes(panes, &fcc))
	{
		MessageBox(_T("Pane window creation failed.\nPlease make sure there is enough system resources available."),
			0, MB_OK | MB_ICONERROR);
		return false;
	}

	// send initial update: panes may read registry settings;
	// at this point they are still not resized (window rect is empty) and visibility flag is set to false
	SendPaneNotification(frame_wnd_, &PaneWnd::InitialUpdate);

	if (!GetProfileVector(reg_section_, REG_STATUSBAR_FIELDS, status_bar_wnd_.fields_))
		status_bar_wnd_.DefaultFields();

	CHECK_TIME_LIMIT

	frame_wnd_.EnableWindow();
	frame_wnd_.ShowWindow(SW_SHOW);

//TODO
//	if (SnapView* view= dynamic_cast<SnapView*>(exif_view_wnd_.GetParent()))
//		frame_wnd_.SetActiveSnapView(view);

//	SetActiveView(&exif_view_wnd_);

	return true;
}


BOOL BrowserFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	if (!MainFrame::PreCreateWindow(cs))
		return FALSE;

//	if (WhistlerLook::IsAvailable())
		cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// BrowserFrame diagnostics

#ifdef _DEBUG
void BrowserFrame::AssertValid() const
{
	MainFrame::AssertValid();
}

void BrowserFrame::Dump(CDumpContext& dc) const
{
	MainFrame::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// BrowserFrame message handlers

void BrowserFrame::TogglePane(CWnd& wnd)
{
	if (SnapView* view= dynamic_cast<SnapView*>(wnd.GetParent()))
		frame_wnd_.PaneToggle(view);
}

bool BrowserFrame::IsPaneOpen(CWnd& wnd)
{
	if (SnapView* view= dynamic_cast<SnapView*>(wnd.GetParent()))
		return view->IsPaneOpen();
	ASSERT(false);
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void BrowserFrame::OnInfoBar()						{ TogglePane(info_bar_wnd_); }
void BrowserFrame::OnUpdateInfoBar(CCmdUI* cmd_ui)	{ cmd_ui->SetCheck(IsPaneOpen(info_bar_wnd_)); }

void BrowserFrame::OnFolders()						{ TogglePane(folders_wnd_); }
void BrowserFrame::OnUpdateFolders(CCmdUI* cmd_ui)	{ cmd_ui->SetCheck(IsPaneOpen(folders_wnd_)); }

void BrowserFrame::OnPreview()						{ TogglePane(preview_wnd_); }
void BrowserFrame::OnUpdatePreview(CCmdUI* cmd_ui)	{ cmd_ui->SetCheck(IsPaneOpen(preview_wnd_)); }


///////////////////////////////////////////////////////////////////////////////

// call after selected photo was changed to update info pane
//
void BrowserFrame::PhotoDescriptionChanged(std::wstring& descr)
{
	try
	{
		// send notification to pane windows
		SendPaneNotification(frame_wnd_, &PaneWnd::PhotoDescriptionChanged, descr);
	}
	CATCH_ALL
}

///////////////////////////////////////////////////////////////////////////////

// call after selected photo was changed to update info pane
//
void BrowserFrame::SelectionChanged(PhotoInfoPtr photo)
{
	try
	{
		// send notification to pane windows
		VectPhotoInfo selection;
		if (photo != 0)
			selection.push_back(photo);
		SendPaneNotification(frame_wnd_, &PaneWnd::PhotoSelectionChanged, selection);
	}
	CATCH_ALL
}


void BrowserFrame::SelectionChanged(VectPhotoInfo& selected)
{
	try
	{
		SendPaneNotification(frame_wnd_, &PaneWnd::PhotoSelectionChanged, selected);
	}
	CATCH_ALL
}


void BrowserFrame::RefreshStatusBar()
{
	// update status bar
	SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
}


void BrowserFrame::CurrentChanged(PhotoInfoPtr photo, bool has_selection)
{
	// update status bar
	RefreshStatusBar();
}


void BrowserFrame::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case ID_VIEW_MODE:		// view mode
		OnViewModePopupMenu();
		break;

	case ID_VIEW_SORT:		// sorting
		OnViewSortPopupMenu();
		break;

//	case ID_BROWSER:		// recent folders
//		OnBrowserPopupMenu();
//		break;

	case ID_FOLDER_LIST:	// favorites
		OnFolderListMenu();
		break;

	case ID_FILE_MASK:
		OnFileMaskPopupMenu();
		break;

	case ID_PANE_MENU:
		OnPanePopupMenu();
		break;
	}
}


void BrowserFrame::OnFolderList()
{
	toolbar_wnd_.PressButton(ID_FOLDER_LIST);
	OnFolderListMenu();
	toolbar_wnd_.PressButton(ID_FOLDER_LIST, false);
}


void BrowserFrame::OnFolderListMenu()
{
	try
	{
		if (!toolbar_wnd_.IsWindowVisible())	// if toolbar is hidden then no-op
			return;

		CMenuFolders menu;
		BuildFolderListMenu(menu);

		CRect rect= toolbar_wnd_.GetButtonRect(ID_FOLDER_LIST);
		CPoint pos(rect.left, rect.bottom);

		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);
	}
	CATCH_ALL
}


void BrowserFrame::BuildFolderListMenu(CMenuFolders& menu)
{
	UINT count= menu.GetMenuItemCount();
	while (count-- > 0)
		menu.DeleteMenu(0, MF_BYPOSITION);

	menu.InsertItem(ID_ADD_TO_FAVORITES,  _T("&Add to Favorites..."));
	menu.InsertItem(ID_SET_FAVORITES, _T("&Setup Favorites..."));

	if (favorite_folders_->GetCount() > 0)
	{
		menu.InsertMenu(-1, MF_BYPOSITION | MF_SEPARATOR);
		favorite_folders_->AppendMenuItems(menu);
	}
}


// favorite folder selected: open browser window
//
void BrowserFrame::OnFavoriteFolder(UINT folder_id)
{
	try
	{
		int index= folder_id - ID_BROWSE_FOLDER_1;

		bool shift= ::GetKeyState(VK_SHIFT) < 0;

		if (shift)
			AddToFavorites(index);
		else
			FavoriteFolder(index);
	}
	CATCH_ALL
}


bool BrowserFrame::FavoriteFolder(int index)
{
	if (const ::FavoriteFolder* folder= favorite_folders_->GetFolder(index))
	{
		//last_folder_ = index;

		FolderPathPtr path= ::CreateFolderPath(folder->GetIDL());

		if (path == 0)
		{
			AfxMessageBox(_T("Favorite folder '") + folder->GetDisplayName() + _T("' does not exist."), MB_OK | MB_ICONWARNING);
			return false;
		}

		FolderSelected(path);

		//folders_wnd_.ResetPath(::CreateFolderPath(folder->GetIDL()), folder->OpenAsRoot());
		//RefreshView();

		last_folder_ = index;

		return true;
	}

	return false;
}


void BrowserFrame::OnUpdateFavoriteFolder(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(IsWindowVisible());
}


void BrowserFrame::EnableRecentFolder(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(IsWindowVisible());
}


static void BuildRecentFoldersMenu(CMenu& menu, const CRecentFileList& paths)
{
	CString name;
	CString temp;
	int cmd= ID_RECENT_PATH_FIRST;
	for (int mru= 0; mru < paths.GetSize(); mru++)
	{
		name = const_cast<CRecentFileList&>(paths)[mru]; //.GetDisplayName(name, mru, 0, 0))

		if (name.IsEmpty())
			break;

		// double up any '&' characters so they are not underlined
		LPCTSTR src = name;
		LPTSTR dest = temp.GetBuffer(name.GetLength()*2);
		while (*src != 0)
		{
			if (*src == '&')
				*dest++ = '&';
			if (_istlead(*src))
				*dest++ = *src++;
			*dest++ = *src++;
		}
		*dest = 0;
		temp.ReleaseBuffer();

		// insert mnemonic + the file name
		TCHAR buf[10];
		int item = (mru + 1 /*+ start_*/);

		// number &1 thru &9, then 1&0, then 11 thru ...
		if (item > 10)
			wsprintf(buf, _T("%d "), item);
		else if (item == 10)
			lstrcpy(buf, _T("1&0 "));
		else
			wsprintf(buf, _T("&%d "), item);

		menu.InsertMenu(mru, MF_STRING | MF_BYPOSITION, cmd++, buf + temp);
	}
}


void BrowserFrame::RecentFolders(CPoint pos)
{
	try
	{
		CMenu menu;
		if (!menu.CreatePopupMenu())
			return;

		CMenu* popup= &menu;

		BuildRecentFoldersMenu(menu, *recent_path_list_);

		int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this);

		if (cmd == 0 || recent_path_list_->GetSize() == 0)
			return;

		const CString& str= (*recent_path_list_)[cmd - ID_RECENT_PATH_FIRST];

		//DWORD attrib= ::GetFileAttributes(str);
		//if (attrib == 0-1 || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
		//	return;

		// scan selected folder
		FolderPathPtr path= ::CreateFolderPath(str);
		//ItemIdList idl(str);
		Browser(path, true);
	}
	CATCH_ALL
}


void BrowserFrame::FolderUp()
{
	FolderPathPtr path= exif_view_wnd_.GetCurrentPath();
	if (path)
	{
		FolderPathPtr parent= path->GetParent();
		if (parent)
			Browser(parent, true);
	}
}


bool BrowserFrame::UpdateFolderUp()
{
	FolderPathPtr path= exif_view_wnd_.GetCurrentPath();
	return path && !path->TopLevel();

//	const ItemIdList& path= exif_view_wnd_.GetCurrentPath();
//	return path.GetLength() > 0;
}


void BrowserFrame::OnViewModePopupMenu()
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_VIEW_POPUP_MENU))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);

	CRect rect= toolbar_wnd_.GetButtonRect(ID_VIEW_MODE);
	CPoint pos(rect.left, rect.bottom);

	popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON, pos.x, pos.y, this);
}


void BrowserFrame::OnViewSortPopupMenu()
{
	CRect rect= toolbar_wnd_.GetButtonRect(ID_VIEW_SORT);
	CPoint pos(rect.left, rect.bottom);
	exif_view_wnd_.OnViewSortPopupMenu(pos);
}

///////////////////////////////////////////////////////////////////////////////

// Low level fn: new folder selected; refresh view
//
void BrowserFrame::FolderSelectedEx(FolderPathPtr path)
{
	addr_box_wnd_.SetPath(path);

	if (exif_view_wnd_.PathSelected(path))
	{
		String str= path->GetDisplayPath();
		if (!str.empty())
			recent_path_list_->Add(str.c_str());
		//CString path= idlPath.GetPath();
		//if (!path.IsEmpty())
		//	recent_path_list_->Add(path);
	}
}


// open maximized if this is first MDI window
//
void BrowserFrame::ActivateFrame(int cmd_show)
{
/*	CMDIFrameWnd* frame_wnd = GetMDIFrame();
	ASSERT_VALID(frame_wnd);

	// determine default show command
	if (cmd_show == -1)
	{
		// get maximized state of frame window (previously active child)
		BOOL maximized;
		if (frame_wnd->MDIGetActive(&maximized) == 0)	// no MDI child window yet?
			cmd_show = SW_MAXIMIZE;
	}
*/
	MainFrame::ActivateFrame(cmd_show);
}


// High level fn: new folder selected
//
bool BrowserFrame::FolderSelected(const TCHAR* str_path)
{
	FolderPathPtr path= ::CreateFolderPath(str_path);
	if (path == 0)
		return false;

	return FolderSelected(path);
}


bool BrowserFrame::FolderSelected(FolderPathPtr path)
{
	if (path == 0)
		return false;

	Browser(path, true);

	return true;
}


// re-read photos from the current folder
//
void BrowserFrame::RefreshView()
{
	//ItemIdList idlPath;
	//if (!folders_wnd_.GetCurrentPath(idlPath))
	//	return;

	FolderPathPtr path= folders_wnd_.GetCurrentPath();
	if (path)
		FolderSelectedEx(path);
}


// get text displayed in first status bar pane (instead of 'Ready' string)
CString BrowserFrame::GetStatusReadyText() const
{
	return exif_view_wnd_.GetCurImageInfo();
}


// status pane: number of images or 'scanning' text
CString BrowserFrame::GetStatusPaneText() const
{
	return exif_view_wnd_.GetImagesStat();
}


bool BrowserFrame::IsScanning() const
{
	return exif_view_wnd_.IsScanning();
}


void BrowserFrame::EnableSavingSettings(bool enable/*= true*/)
{
	save_settings_ = enable;
}


void BrowserFrame::OnDestroy()
{
	if (initialized_ && save_settings_)
	{
		try
		{
			SendPaneNotification(frame_wnd_, &PaneWnd::SaveSettings);

			CWinApp* app= AfxGetApp();

			app->WriteProfileInt(reg_section_, REG_LAST_FAVORITE, last_folder_);

			if (FolderPathPtr path= folders_wnd_.GetCurrentPath())
			{
				ItemIdList idlFolder= path->GetPIDL();
				//	if (folders_wnd_.GetCurrentPath(idlFolder))
				idlFolder.Store(reg_section_, REG_LAST_PATH);
			}
			else
			{ ASSERT(false); }

			rebar_wnd_.StoreLayout(reg_section_, REG_REBAR_LAYOUT);

			// status bar fields
			WriteProfileVector(reg_section_, REG_STATUSBAR_FIELDS, status_bar_wnd_.fields_);

			// save custom columns
			CString section= GetRegSection();
			section += '\\';
			section += REG_CUSTOM_COLUMNS;

			const CustomColumns& custom_cols= columns_.GetCustomColumns();
			for (size_t i= 0; i < custom_cols.size(); ++i)
			{
				const CustomColumnDef& col= custom_cols[i];

				TCHAR key[200];
				wsprintf(key, _T("%s-%d"), CUSTOM_COLUMN_NAME, static_cast<int>(i));
				app->WriteProfileString(section, key, col.caption_.c_str());
				wsprintf(key, _T("%s-%d"), CUSTOM_COLUMN_EXPR, static_cast<int>(i));
				app->WriteProfileString(section, key, col.expression_.c_str());
			}
		}
		CATCH_ALL
	}

	MainFrame::OnDestroy();
}


FolderPathPtr BrowserFrame::GetCurrentFolder()
{
	return folders_wnd_.GetCurrentPath();
}


void BrowserFrame::OnBrowser()
{
	Browser();
}

void BrowserFrame::Browser()
{
	ItemIdList idlMyComp(CSIDL_DRIVES);
	FolderPathPtr path= ::CreateFolderPath(idlMyComp);
	Browser(path, false);
}

//void BrowserFrame::Browser(const ItemIdList& idlFolder, bool root_dir)
//{
//	last_folder_ = -1;
//	folders_wnd_.ResetPath(idlFolder, root_dir /*false*/);
//	RefreshView();
//}

void BrowserFrame::Browser(FolderPathPtr path, bool rootDir)
{
	if (path)
	{
		last_folder_ = -1;
		folders_wnd_.ResetPath(path, rootDir);
//		RefreshView();
		FolderSelectedEx(path);
	}
}


void BrowserFrame::OnUpdateBrowser(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


BOOL BrowserFrame::OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info)
{
	if (filter_bar_wnd_.m_hWnd && filter_bar_wnd_.OnCmdMsg(id, code, extra, handler_info))
		return true;

	if (frame_wnd_.m_hWnd && frame_wnd_.OnCmdMsg(id, code, extra, handler_info))
		return true;

	return MainFrame::OnCmdMsg(id, code, extra, handler_info);
}


void BrowserFrame::OnUpdateEnter(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


// display only photos with given text in description (pass null to cancel filtering)
//
void BrowserFrame::FilterPhotos(const TCHAR* text)
{
	if (text == 0 || *text == 0)
		exif_view_wnd_.CancelFiltering();
	else
		exif_view_wnd_.FilterPhotos(text);
}


bool BrowserFrame::IsFilterActive() const
{
	return exif_view_wnd_.IsMainPageFilterActive();
}


void BrowserFrame::OnNextPane()
{
	NextPane(true);
}


void BrowserFrame::OnPreviousPane()
{
	NextPane(false);
}


void BrowserFrame::NextPane(bool next)
{
	HWND focus= ::GetFocus();

	CWnd* panes[]= { &tag_bar_wnd_, &exif_view_wnd_, &folders_wnd_ };

	int index= -1;
	if (tag_bar_wnd_.HasFocus())
		index = 0;
	else if (exif_view_wnd_.GetListCtrl().m_hWnd == focus)
		index = 1;
	else
		index = 2;

	for (int i= 0; i < array_count(panes) - 1; ++i)
	{
		if (next)
			++index;
		else
			index += array_count(panes) - 1;
		index %= array_count(panes);

		if (SnapView* view= dynamic_cast<SnapView*>(panes[index]->GetParent()))
			if (view->IsWindowVisible())
			{
				frame_wnd_.SetActiveSnapView(view);
				break;
			}
	}
}


void BrowserFrame::OnUpdateNextPane(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void BrowserFrame::OnTagsBar()		// toggle tags bar window (it was; now experimental change)
{
	exif_view_wnd_.StartTagging();
}

void BrowserFrame::OnUpdateTagsBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


// apply (or remove) a tag to/from selected photos
//
//bool BrowserFrame::ApplyTagToPhotos(VectPhotoInfo& photos, const String& tag, bool apply, CWnd* parent)
//{
//	CWnd* wnd= parent ? parent : this;
//
//	try
//	{
//		// copy is necessary: photos may be changed indirectly by applying tags
//		VectPhotoInfo selected= photos;
//
//		for (VectPhotoInfo::iterator it= selected.begin(); it != selected.end(); ++it)
//		{
//			if (*it == 0)
//			{
//				ASSERT(false);
//				continue;
//			}
//			PhotoInfo& photo= **it;
//
//			if (apply)
//				photo.GetTags().ApplyTag(tag);
//			else
//				photo.GetTags().RemoveTag(tag);
//
//			// save changes
//			if (g_Settings.save_tags_to_photo_)
//			{
//				//TODO: improve: collect all errors and show them ONCE
//				try
//				{
//					photo.SaveTags();
//				}
//				CATCH_ALL_W(wnd)
//			}
//
//			// TODO
//			// move photos to different group
//			//
//
//			exif_view_wnd_.TagApplied(*it);
//		}
//
//		RefreshStatusBar();
//	}
//	CATCH_ALL_W(wnd)
//
//	return true;
//}


//void BrowserFrame::ApplyRatingToPhotos(VectPhotoInfo& photos, int rating, CWnd* parent)
//{
//	CWnd* wnd= parent ? parent : this;
//
//	try
//	{
//		// copy is necessary: photos may be changed indirectly by applying rating
//		VectPhotoInfo selected= photos;
//
//		for (VectPhotoInfo::iterator it= selected.begin(); it != selected.end(); ++it)
//		{
//			if (*it == 0)
//			{
//				ASSERT(false);
//				continue;
//			}
//			PhotoInfo& photo= **it;
//
//			//TODO: improve: collect all errors and show them ONCE
//			try
//			{
//				photo.SaveRating(rating);
//			}
//			CATCH_ALL_W(wnd)
//
//			exif_view_wnd_.TagApplied(*it);	// borrowing notification for tag change
//		}
//
//		RefreshStatusBar();
//	}
//	CATCH_ALL_W(wnd)
//}


void BrowserFrame::OnDropFiles(HDROP drop_info)
{
	SetActiveWindow();      // activate us first!

	UINT files= ::DragQueryFile(drop_info, (UINT)-1, NULL, 0);

	TCHAR path[_MAX_PATH];
	path[0] = 0;

	//TODO: currently using only first file...

//	for (UINT iFile = 0; iFile < files; iFile++)
	if (files > 0)
		::DragQueryFile(drop_info, 0, path, _MAX_PATH);

	::DragFinish(drop_info);

	if (path[0])
	{
		FolderPathPtr path_ptr= CreateFolderPath(path);
		if (path_ptr)
			FolderSelected(path_ptr);
	}
}


void BrowserFrame::OnViewToolbar()
{
	rebar_wnd_.ShowBand(BAND_TOOLBAR, !rebar_wnd_.IsBandVisible(BAND_TOOLBAR));
}

void BrowserFrame::OnUpdateViewToolbar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(rebar_wnd_.IsBandVisible(BAND_TOOLBAR) ? 1 : 0);
}


void BrowserFrame::OnViewAddressbar()
{
	rebar_wnd_.ShowBand(BAND_ADDRESS, !rebar_wnd_.IsBandVisible(BAND_ADDRESS));
}

void BrowserFrame::OnUpdateViewAddressbar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(rebar_wnd_.IsBandVisible(BAND_ADDRESS) ? 1 : 0);
}


void BrowserFrame::OnViewFilterbar()
{
	rebar_wnd_.ShowBand(BAND_FILTER, !rebar_wnd_.IsBandVisible(BAND_FILTER));
}

void BrowserFrame::OnUpdateViewFilterbar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(rebar_wnd_.IsBandVisible(BAND_FILTER) ? 1 : 0);
}


void BrowserFrame::RepositionTaskBar()
{
	auto gap = SnapView::GetSeparatorThickness();
	m_rectBorder.bottom = gap.cy;

	if (toolbar_.IsWindowVisible())
	{
		tools_visible_ = true;

		if (toolbar_.IsHorizontal())
		{
			m_rectBorder.bottom = toolbar_size_.cy + BOTTOM_GAP;
			m_rectBorder.right = 0;
		}
		else
		{
			m_rectBorder.right = toolbar_size_.cx + RIGHT_GAP;
		}
	}
	else
	{
		tools_visible_ = false;
		m_rectBorder.right = 0;
	}
	RecalcLayout();
	PlaceToolbar();
}


void BrowserFrame::OnViewTools()
{
	toolbar_.ShowWindow(toolbar_.IsWindowVisible() ? SW_HIDE : SW_SHOWNA);
	RepositionTaskBar();
}

void BrowserFrame::OnUpdateViewTools(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(toolbar_.IsWindowVisible() ? 1 : 0);
}


void BrowserFrame::OnPathTyped()
{
	// new path typed in the address bar

	try
	{
		CEdit& edit= addr_box_wnd_.GetEditCtrl();
		{
			CString path;
			edit.GetWindowText(path);

			if (!FolderSelected(path))
			{
				new BalloonMsg(&edit, _T("Wrong Path"), _T("Please type in existing directory."), BalloonMsg::IERROR);
				return;
			}

			// move focus to the folder tree
			folders_wnd_.GetWindow()->SetFocus();
		}
	}
	CATCH_ALL
}

void BrowserFrame::OnPathCancelled()
{
	// user cancelled typing a path--restore focus to the list ctrl

	FolderPathPtr path= folders_wnd_.GetCurrentPath();
	addr_box_wnd_.SetPath(path);
	CEdit& edit= addr_box_wnd_.GetEditCtrl();
	edit.SetSel(edit.GetWindowTextLength(), edit.GetWindowTextLength());

	exif_view_wnd_.GetListCtrl().SetFocus();
}


void BrowserFrame::OnAddressBar()
{
	addr_box_wnd_.SetHistory(*recent_path_list_);
	CEdit& edit= addr_box_wnd_.GetEditCtrl();
	edit.SetSel(0, edit.GetWindowTextLength());
	edit.SetFocus();
}

void BrowserFrame::OnUpdateAddressBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void BrowserFrame::OnFindBox()
{
	rebar_wnd_.ShowBand(BAND_FILTER, true);
	filter_bar_wnd_.Activate();
}


void BrowserFrame::SetRecursiveScan(bool recursive)
{
	exif_view_wnd_.SetRecursiveScan(recursive);
}


void BrowserFrame::SetReadOnlyExif(bool exif_only)
{
	exif_view_wnd_.SetReadOnlyExif(exif_only);
}


BOOL BrowserFrame::PreTranslateMessage(MSG* msg)
{
	if (menu_bar_wnd_.TranslateFrameMessage(msg))
		return true;

	if (MainFrame::PreTranslateMessage(msg))
		return true;

	if (msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE && GetActiveWindow() == this)
	{
		OnCommand(ID_ESCAPE, 0);
		return true;
	}

	return false;
}


void BrowserFrame::OnFileMaskPopupMenu()
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_FILE_MASK))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);

	CRect rect= toolbar_wnd_.GetButtonRect(ID_FILE_MASK);
	CPoint pos(rect.left, rect.bottom);

	popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON, pos.x, pos.y, this);
}


void BrowserFrame::OnFileMask(UINT folder_id)
{
	folder_id -= ID_MASK_JPEG;
	g_Settings.ToggleScanFileType(folder_id);
	g_Settings.Store();
	exif_view_wnd_.ReloadPhotos();
}


void BrowserFrame::OnLoadAllTypes()
{
	g_Settings.SetAllFileTypes(true);
	g_Settings.Store();
	exif_view_wnd_.ReloadPhotos();
}


void BrowserFrame::OnLoadJpegOnly()
{
	g_Settings.SetAllFileTypes(false);
	g_Settings.SetScanFileType(FT_JPEG);
	g_Settings.Store();
	exif_view_wnd_.ReloadPhotos();
}


void BrowserFrame::OnLoadRawOnly()
{
	g_Settings.SetAllFileTypes(false);
	g_Settings.SetScanFileType(FT_CRW);
	g_Settings.SetScanFileType(FT_NEF);
	g_Settings.SetScanFileType(FT_ORF);
	g_Settings.SetScanFileType(FT_DNG);
	g_Settings.SetScanFileType(FT_RAF);
	g_Settings.SetScanFileType(FT_PEF);
	g_Settings.SetScanFileType(FT_ARW);
	g_Settings.SetScanFileType(FT_RW2);
	g_Settings.SetScanFileType(FT_X3F);
	g_Settings.SetScanFileType(FT_SRW);
	g_Settings.Store();
	exif_view_wnd_.ReloadPhotos();
}


void BrowserFrame::OnUpdateFileMaskRng(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(g_Settings.ScanFileType(cmd_ui->m_nID - ID_MASK_JPEG));
}


ExifStatusBar& BrowserFrame::GetStatusBar()
{
	return status_bar_wnd_;
}


///////////////////////////////////////////////////////////////////////////////
	
// show options dlg
void BrowserFrame::OptionsDlg(int page)
{
	try
	{
		boost::function<void (CWnd* parent)> define_columns;
		if (exif_view_wnd_.CurrentItem() != 0)
			define_columns = boost::bind(&BrowserFrame::DefineCustomColumns, this, _1);

		::OptionsDlg dlg(this, exif_view_wnd_.SelectedColumns(), g_Settings.balloon_fields_, define_columns, page, columns_);

		if (dlg.DoModal() != IDOK)
			return;

		//vector<COLORREF> colors= g_Settings.pane_caption_colors_.Colors();
		//CaptionWindow::ReinitializeImageLists(colors[0], colors[1]);
		SetColors(g_Settings.AppColors());

		frame_wnd_.ResetColors(g_Settings.pane_caption_colors_);

		SendPaneNotification(frame_wnd_, &PaneWnd::OptionsChanged, dlg);

		if (dlg.FieldsChanged())	// balloon fields selected?
			g_Settings.balloon_fields_ = dlg.SelectedFields();

		g_Settings.Store();
	}
	CATCH_ALL
}

void BrowserFrame::SetColors(const ApplicationColors& colors)
{
	auto backgnd = colors[AppColors::Background];
	auto text = colors[AppColors::Text];
	auto dim = colors[AppColors::DimText];
	auto edit = colors[AppColors::EditBox];

	status_bar_wnd_.SetColors(backgnd, text, dim);
	addr_box_wnd_.SetColors(backgnd, edit, text);
	filter_bar_wnd_.SetColors(backgnd, edit, text);
	toolbar_wnd_.SetBackgroundColor(backgnd);
	rebar_wnd_.SetBackgroundColor(backgnd);
	rebar_wnd_.SetTextColor(dim);
}

void BrowserFrame::OnFileMask()	{ OptionsDlg(3); }	// file types page
void BrowserFrame::OnUpdateFileMask(CCmdUI* cmd_ui)	{ cmd_ui->Enable(); }

void BrowserFrame::OnOptions()		{ OptionsDlg(-1); }	// last active page as a starting one
void BrowserFrame::OnUpdateOptions(CCmdUI* cmd_ui)		{ cmd_ui->Enable(); }


void BrowserFrame::OnUpdatePaneMenu(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void BrowserFrame::OnPanePopupMenu()
{
	if (!IsWindowVisible())
		return;

	CRect rect= toolbar_wnd_.GetButtonRect(ID_PANE_MENU);
	toolbar_wnd_.PressButton(ID_PANE_MENU);
	CPoint pos(rect.left, rect.bottom);
	frame_wnd_.PanesMenu(pos, ID_PANES_LAYOUT_00);
	toolbar_wnd_.PressButton(ID_PANE_MENU, false);
}


void BrowserFrame::OnPaneZoom()
{
	frame_wnd_.PaneZoom(0);
}

void BrowserFrame::OnUpdatePaneZoom(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void BrowserFrame::OnRestorePaneLayout(UINT pane_layout_id)
{
	frame_wnd_.RestorePaneLayout(pane_layout_id - ID_PANES_LAYOUT_00);
}

void BrowserFrame::OnUpdateRestorePaneLayout(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void BrowserFrame::OnPanesManageLayouts()
{
	ManagePaneLayouts dlg(this, false);
	frame_wnd_.PaneLayoutList(dlg.names_);
	if (dlg.DoModal() == IDOK)
		frame_wnd_.PaneLayoutSync(dlg.names_);
}

void BrowserFrame::OnUpdatePanesManageLayouts(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void BrowserFrame::OnPanesStoreLayout()
{
	// name it & save
	ManagePaneLayouts dlg(this, true);
	frame_wnd_.PaneLayoutList(dlg.names_);
	dlg.name_ = _T("My Favorite Layout");

	if (dlg.DoModal() == IDOK)
	{
		frame_wnd_.PaneLayoutSync(dlg.names_);
		frame_wnd_.AddCurrentLayout(dlg.name_);
	}
}

void BrowserFrame::OnUpdatePanesStoreLayout(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(frame_wnd_.CanSaveLayout());
}


void BrowserFrame::InitialSetActiveView()
{
	if (SnapView* view= dynamic_cast<SnapView*>(exif_view_wnd_.GetParent()))
		frame_wnd_.SetActiveSnapView(view);
}


void BrowserFrame::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
	MainFrame::OnActivate(state, wnd_other, minimized);

	bool active= state == WA_ACTIVE || state == WA_CLICKACTIVE;

	if (active)
		SendPaneNotification(frame_wnd_, &PaneWnd::MainWndActivated);

	frame_wnd_.ActivateView(active);
}


// _AfxFindPopupMenuFromID & OnMeasureItem--verbatim copy from MFC because of non-virtual GetMenu()

static CMenu* AFXAPI x_AfxFindPopupMenuFromID(CMenu* menu, UINT id)
{
	ASSERT_VALID(menu);
	// walk through all items, looking for ID match
	UINT items = menu->GetMenuItemCount();
	for (int iItem = 0; iItem < (int)items; iItem++)
	{
		CMenu* popup = menu->GetSubMenu(iItem);
		if (popup != NULL)
		{
			// recurse to child popup
			popup = x_AfxFindPopupMenuFromID(popup, id);
			// check popups on this popup
			if (popup != NULL)
				return popup;
		}
		else if (menu->GetMenuItemID(iItem) == id)
		{
			// it is a normal item inside our popup
			menu = CMenu::FromHandlePermanent(menu->m_hMenu);
			return menu;
		}
	}
	// not found
	return NULL;
}

// Measure item implementation relies on unique control/menu IDs
void BrowserFrame::OnMeasureItem(int /*id_ctl*/, LPMEASUREITEMSTRUCT measure_item_struct)
{
	if (measure_item_struct->CtlType == ODT_MENU)
	{
		ASSERT(measure_item_struct->CtlID == 0);
		CMenu* menu= 0;

		_AFX_THREAD_STATE* thread_state = AfxGetThreadState(); //_afxThreadState.GetData();
		if (thread_state->m_hTrackingWindow == m_hWnd)
		{
			// start from popup
			menu = CMenu::FromHandle(thread_state->m_hTrackingMenu);
		}
		else
		{
			// start from menubar
			menu = CMenu::FromHandle(GetMenu()); // GetMenu();
		}

		menu = x_AfxFindPopupMenuFromID(menu, measure_item_struct->itemID);
		if (menu != NULL)
			menu->MeasureItem(measure_item_struct);
		else
			TRACE(traceAppMsg, 0, "Warning: unknown WM_MEASUREITEM for menu item 0x%04X.\n",
				measure_item_struct->itemID);
	}
	else
	{
		CWnd* child = GetDescendantWindow(measure_item_struct->CtlID, TRUE);
		if (child != NULL && child->SendChildNotifyLastMsg())
			return;     // eaten by child
	}
	// not handled - do default
	Default();
}


void BrowserFrame::OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu)
{
	if (popup_menu == folders_popup_.get())
		BuildFolderListMenu(*folders_popup_);
	else if (popup_menu == &menu_sort_order_popup_)
		SendPaneNotification(frame_wnd_, &PaneWnd::UpdateSortOrderPopup, menu_sort_order_popup_);
	else if (popup_menu == &menu_pane_windows_popup_)
	{
		UINT count= menu_pane_windows_popup_.GetMenuItemCount();
		while (count-- > 0)
			menu_pane_windows_popup_.DeleteMenu(0, MF_BYPOSITION);
		frame_wnd_.PanesMenu(menu_pane_windows_popup_, ID_PANES_LAYOUT_00);
	}

	MainFrame::OnInitMenuPopup(popup_menu, index, sys_menu);
}


void BrowserFrame::EnableSortingOption(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}

void BrowserFrame::OnSortingOption(UINT sorting_option)
{
	SendPaneNotification(frame_wnd_, &PaneWnd::ChangeSortOrder, sorting_option);
}


void BrowserFrame::OnClose()
{
	// postponed close due to the second msg run loop in mini frame windows; let them finish first
	// if they are active or else they will crash the app
	PostMessage(WM_USER + 1234);
}


LRESULT BrowserFrame::OnCloseApp(WPARAM, LPARAM)
{
	CFrameWnd::OnClose();
	return 0;
}


FolderPathPtr BrowserFrame::GetLastUsedFolder() const
{
	return last_used_path_;
}


void BrowserFrame::OnBuildImgCatalog()	// this cmd invoked from the folder pane
{
	// get currently selected folder in the folder pane (it may not be the one that's currently used)
	CString path= folders_wnd_.GetCurrentPhysPath();
	BuildCatalog(path);
}


void BrowserFrame::OnDefineCustomColumns()
{
	DefineCustomColumns(this);
}


void BrowserFrame::DefineCustomColumns(CWnd* parent)
{
	PhotoInfoPtr photo= exif_view_wnd_.CurrentItem();
	if (photo == 0)
		return;

	try
	{
		CustomColumnsDlg dlg(parent, columns_.GetCustomColumns(), *photo);
		HeaderDialog frame(dlg, _T("TTL"), parent);

		if (frame.DoModal() != IDOK)
			return;

		exif_view_wnd_.SetCustomColumns(dlg.GetColumns());
	}
	CATCH_ALL
}


void BrowserFrame::OnUpdateDefineCustomColumns(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(exif_view_wnd_.CurrentItem() != 0);
}

void BrowserFrame::DrawSeparator(CDC& dc, const CRect& client)
{
	if (rebar_wnd_.m_hWnd == 0 || frame_wnd_.m_hWnd == 0)
		return;

	COLORREF separator = g_Settings.AppColors()[AppColors::Separator];
	COLORREF base = g_Settings.AppColors()[AppColors::Background];

	WINDOWPLACEMENT wp;
	if (rebar_wnd_.GetWindowPlacement(&wp))
	{
		int y= wp.rcNormalPosition.bottom;

		CRect rect(client.left, y, client.right, y + TOP_GAP);
		dc.FillSolidRect(rect, base);
		rect.OffsetRect(0, TOP_GAP);
		//CRect rect(client.left, rect1.bottom, client.right, rect1.bottom + m_rectBorder.top - TOP_GAP);
		rect.bottom = rect.top + m_rectBorder.top - TOP_GAP;
		dc.FillSolidRect(rect, separator);

		//rect.OffsetRect(0, 1);
		//dc.FillSolidRect(rect, CalcShade(base, -2.5f));
		//rect.OffsetRect(0, 1);
		//dc.FillSolidRect(rect, CalcShade(base, -7.0f));
	}

	auto gap = SnapView::GetSeparatorThickness();

	if (frame_wnd_.GetWindowPlacement(&wp))
	{
		// toolbar area
		if (toolbar_.m_hWnd && toolbar_.IsWindowVisible())
		{
			if (toolbar_.IsHorizontal())
			{
				int y = wp.rcNormalPosition.bottom;

				dc.FillSolidRect(client.left, y, client.Width(), m_rectBorder.bottom, base);

				CRect rect = client;
				rect.top = wp.rcNormalPosition.top;

				// horizontal separator (edge)
				dc.FillSolidRect(rect.left, y, rect.Width(), gap.cy, separator);
			}
			else		// vertical toolbar
			{
				CRect rect = wp.rcNormalPosition;

				// vertical separator (edge)
				dc.FillSolidRect(rect.right, rect.top, gap.cx, rect.Height(), separator);

				// horizontal separator
				dc.FillSolidRect(rect.left, rect.bottom, client.Width(), gap.cy, separator);
			}
		}
		else
		{
			CRect rect = wp.rcNormalPosition;

			// horizontal separator (edge) at the bottom
			dc.FillSolidRect(rect.left, rect.bottom, client.Width(), gap.cy, separator);
			//dc.FillSolidRect(client, RGB(0, 255, 0));
		}
	}
}


void BrowserFrame::OnFilterPhotos()
{
	CString text;
	filter_bar_wnd_.GetEditCtrl().GetWindowText(text);
	FilterPhotos(text);
}


void BrowserFrame::OnCancelFilter()
{
	FilterPhotos(nullptr);
}


void BrowserFrame::OnToolbarHorizontal()
{
	SetTaskBarHorzOrientation(true);
}


void BrowserFrame::OnToolbarVertical()
{
	SetTaskBarHorzOrientation(false);
}


void BrowserFrame::SetTaskBarHorzOrientation(bool horz)
{
	toolbar_.SetOrientation(horz);
	toolbar_size_ = toolbar_.GetSize();
	RepositionTaskBar();
	tools_horizontal_ = toolbar_.IsHorizontal();
}
