/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CopyMoveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CopyMoveDlg.h"
#include "FolderSelect.h"
#include "RString.h"
#include "FolderListCtrl.h"
#include "ListViewCtrl.h"
#include "ItemIdList.h"
#include "RecentPaths.h"
#include "PathEdit.h"
#include "ShellFolder.h"
#include "ToolBarWnd.h"
#include "WhistlerLook.h"
#include "CatchAll.h"
#include "BalloonMsg.h"
#include <shlwapi.h>
#include "Path.h"
#include <map>
#include "SeparatorWnd.h"
#include "ResizeWnd.h"
#include "clamp.h"
#include "Profile.h"
#include "Color.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const int MAX_RECENT_PATHS= 30;
const int MIN_RECENT_PATHS_PANEL_SIZE= 20;

enum { ID_GO_UP= 1000, ID_LIST_VIEW, ID_ICON_VIEW };

/////////////////////////////////////////////////////////////////////////////
// CopyMoveDlg dialog

struct CopyMoveDlg::Impl : ListViewCtrlNotifications, ResizeWnd
{
	Impl() : resize_(false), self_(0), recent_paths_panel_right_(0), ctrl_shift_(0)
	{
		store_ctrl_shift_.Register(L"CopyMoveDlg", L"CtrlShift", ctrl_shift_);
		ctrl_shift_ = store_ctrl_shift_;
	}
	~Impl()
	{
		store_ctrl_shift_ = ctrl_shift_;
	}

	CopyMoveDlg* self_;
	FolderListCtrl list_;
	ListViewCtrl recent_;
	CImageList sys_img_list_;
	PathVector recent_paths_;
	PathVector current_paths_;
	CPathEdit edit_path_;
	bool copy_operation_;
	ToolBarWnd toolbar_;
	std::vector<float> color_shades_;
	FolderPathPtr current_photo_path_;	// photos were loaded using this path
	SeparatorWnd resize_;
	int recent_paths_panel_right_;
	int ctrl_shift_;	// when splitter is used it resizes left and right part of the window; this is shift from original location
	Profile<int> store_ctrl_shift_;

	void FolderChanged(CShellFolderPtr folder);

	// ListViewCtrlNotifications interface implementation:
	virtual void ItemClicked(ListViewCtrl& ctrl, int index, size_t param);
	virtual int GetItemCheckState(ListViewCtrl& ctrl, int index, size_t param);
	virtual void ItemColors(ListViewCtrl& ctrl, int index, size_t param, COLORREF& rgb_text, COLORREF& rgb_backgnd);
	virtual CString GetTooltipText(ListViewCtrl& ctrl, int index, size_t param);
	virtual bool IsItemEnabled(ListViewCtrl& ctrl, int index, size_t param);
	virtual bool FilterItem(ListViewCtrl& ctrl, int index, size_t param, bool group, const String& label, bool& filter_in);
	virtual void DrawItemBackground(ListViewCtrl& ctrl, int index, size_t param, CDC& dc, CRect rect);

	// ResizeWnd notifications:
	virtual int GetPaneHeight();
	virtual void ResizePane(int width);
	//virtual void Resizing(bool start);

	void ResizePaneBy(int shift);
};


CopyMoveDlg::CopyMoveDlg(bool copy, const TCHAR* dest_path, CWnd* parent, FolderPathPtr cur_path)
	: DialogChild(CopyMoveDlg::IDD, parent), impl_(*new Impl)
{
	impl_.self_ = this;
	impl_.copy_operation_ = copy;
	impl_.current_photo_path_ = cur_path;

//	label_ = copy ? _T("Copy") : _T("Move");
//	label_ += _T(" selected images to this folder:");
	label_ = RString(copy ? IDS_COPY_FILES : IDS_MOVE_FILES);
	path_ = dest_path;

	SHFILEINFO info;
	if (DWORD_PTR image_list= ::SHGetFileInfo(_T(""), NULL, &info, sizeof info, SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX))
		impl_.sys_img_list_.Attach(reinterpret_cast<HIMAGELIST>(image_list));

	if (WhistlerLook::IsAvailable())
	{
		const static float shades[]= { -30.0f, 50.0f, 40.0f, 20.0f, 5.0f };
		impl_.color_shades_.assign(shades, shades + array_count(shades));
	}
}


CopyMoveDlg::~CopyMoveDlg()
{
	impl_.sys_img_list_.Detach();
	delete &impl_;
}


void CopyMoveDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CopyMoveDlg)
	DDX_Control(DX, IDOK, btn_ok_);
	DDX_Control(DX, IDC_PATH, impl_.edit_path_);
	DDX_Text(DX, IDC_LABEL, label_);
	DDX_Text(DX, IDC_PATH, path_);
	//}}AFX_DATA_MAP
	DDX_Control(DX, IDC_RECENT_PATHS, impl_.recent_);
	DDX_Control(DX, IDC_LIST, impl_.list_);
	DDX_Control(DX, IDC_TOOLBAR, impl_.toolbar_);
}


BEGIN_MESSAGE_MAP(CopyMoveDlg, DialogChild)
	ON_COMMAND(IDC_BROWSE, OnBrowse)
	ON_COMMAND(ID_GO_UP, OnGoLevelUp)
	ON_COMMAND(ID_LIST_VIEW, OnListView)
	ON_COMMAND(ID_ICON_VIEW, OnIconView)
	ON_WM_ERASEBKGND()
	ON_EN_CHANGE(IDC_PATH, OnPathEditChanged)
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CopyMoveDlg message handlers

BOOL CopyMoveDlg::OnInitDialog()
{
	try
	{
		return InitDialog();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


const TCHAR* RECENT_PATHS_SECTION= _T("CopyMoveDlg");


BOOL CopyMoveDlg::InitDialog()
{
	DialogChild::OnInitDialog();

	impl_.resize_.SubclassDlgItem(IDC_RESIZE, this);
	impl_.resize_.SetCallbacks(&impl_);
	impl_.resize_.DrawBar(false);

	SubclassHelpBtn(impl_.copy_operation_ ? _T("ToolCopy.htm") : _T("ToolMove.htm"));

	SetWindowText(impl_.copy_operation_ ? _T("复制照片") : _T("移动照片"));
	btn_ok_.SetWindowText(impl_.copy_operation_ ? _T("复制") : _T("移动"));

	COLORREF dark= ::CalcShade(::GetSysColor(COLOR_3DFACE), -21.5f);
	impl_.recent_.SetBackgndColor(dark);
	impl_.recent_.SetMenuLikeSelection(true);
	impl_.recent_.SetItemSpace(1.4f);
	impl_.recent_.SetImageList(&impl_.sys_img_list_);
	impl_.recent_.SetReceiver(&impl_);
	impl_.recent_.EnableToolTips();

	impl_.list_.SetFolderChangedCallback(boost::bind(&Impl::FolderChanged, &impl_, _1));

	impl_.toolbar_.SetOnIdleUpdateState(false);
	const int commands[]= { ID_GO_UP, ID_LIST_VIEW, ID_ICON_VIEW };
	impl_.toolbar_.AddButtons("P|GG", commands, IDB_FOLDER_LIST_VIEW_TB, IDS_FOLDER_LIST_VIEW_TB);
	impl_.toolbar_.CheckButton(ID_LIST_VIEW, true);

	// read recent paths
	RecentPaths(impl_.recent_paths_, false, RECENT_PATHS_SECTION, MAX_RECENT_PATHS);

//	impl_.current_paths_ = impl_.recent_paths_;

	ItemIdList cur= impl_.current_photo_path_->GetPIDL();
	AddCommonPaths(impl_.current_paths_, &cur);

	// append all recent paths
	AppendPaths(impl_.current_paths_, impl_.recent_paths_);

	const size_t count= impl_.current_paths_.size();
	std::map<CString, int> dup;		// detect identical folder names

	for (size_t i= 0; i < count; ++i)
		dup[impl_.current_paths_[i]->GetName()]++;

	for (size_t i= 0; i < count; ++i)
	{
		CString folder= impl_.current_paths_[i]->GetName();
		if (dup[folder] > 1)
		{
			CString path= impl_.current_paths_[i]->GetPath();
			if (!path.IsEmpty())
				folder = path;
		}

		impl_.recent_.AddItem(folder, impl_.current_paths_[i]->GetIconIndex(), 0, 0);
	}

	ItemIdListPtr start;

	// set the most recent path
	if (!impl_.recent_paths_.empty())
		start = impl_.recent_paths_.back();
	else if (!impl_.current_paths_.empty())
		start = impl_.current_paths_.front();

	if (start)
	{
		// make sure we have a valid folder to start

		Path path= start->GetPath();

		if (!path.empty())	// 'my computer' has no path
		{
			bool reset= false;

			while (!path.empty() && !path.IsFolder())	// folder does not exist?
			{
				path = path.GetDir();
				reset = true;
			}

			if (!path.empty() && reset)
				start.reset(new ItemIdList(path.c_str()));
		}

		impl_.list_.SetPath(*start);
	}

	WINDOWPLACEMENT wp;
	if (impl_.recent_.GetWindowPlacement(&wp))
	{
		impl_.recent_paths_panel_right_ = wp.rcNormalPosition.right;
		SetRightSide(impl_.recent_paths_panel_right_, dark, impl_.color_shades_);
	}

	BuildResizingMap();
	SetWndResizing(IDC_RECENT_PATHS, DlgAutoResize::RESIZE_V, DlgAutoResize::SHIFT_RESIZES);
	SetWndResizing(IDC_PATH, DlgAutoResize::RESIZE_H, DlgAutoResize::SHIFT);
	SetWndResizing(IDC_LABEL, DlgAutoResize::RESIZE_H, DlgAutoResize::SHIFT);
	SetWndResizing(IDC_TOOLBAR, DlgAutoResize::NONE, DlgAutoResize::SHIFT);
	SetWndResizing(IDC_BROWSE, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE, DlgAutoResize::SHIFT_LEFT);
	SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V, DlgAutoResize::SHIFT);
	SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	SetWndResizing(IDOK, DlgAutoResize::MOVE);
	SetWndResizing(IDC_RESIZE, DlgAutoResize::RESIZE_V, DlgAutoResize::SHIFT);

	CRect rect(0,0,0,0);
	GetWindowRect(rect);
	SetMinimalDlgSize(rect.Size());

	impl_.ResizePaneBy(impl_.ctrl_shift_);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


int CopyMoveDlg::Impl::GetPaneHeight()
{
	if (recent_.m_hWnd)
	{
		WINDOWPLACEMENT wp;
		if (recent_.GetWindowPlacement(&wp))
			return wp.rcNormalPosition.right;
	}
	return 0;
}


void CopyMoveDlg::Impl::ResizePane(int width)
{
	if (recent_.m_hWnd && self_ != 0)
	{
		width = clamp(width, MIN_RECENT_PATHS_PANEL_SIZE, 2 * recent_paths_panel_right_);
		ctrl_shift_ = width - recent_paths_panel_right_;
		ResizePaneBy(ctrl_shift_);
	}
}


void CopyMoveDlg::Impl::ResizePaneBy(int shift)
{
	if (recent_.m_hWnd && self_ != 0)
	{
		self_->SetControlsShift(CSize(shift, 0));

		WINDOWPLACEMENT wp;
		if (recent_.GetWindowPlacement(&wp))
			self_->SetRightSide(wp.rcNormalPosition.right);

		self_->Invalidate();
	}
}

void CopyMoveDlg::OnGoLevelUp()	{ impl_.list_.GoLevelUp(); }
void CopyMoveDlg::OnListView()		{ impl_.list_.SetListView(); }
void CopyMoveDlg::OnIconView()		{ impl_.list_.SetIconView(); }


void CopyMoveDlg::Impl::FolderChanged(CShellFolderPtr folder)
{
	TCHAR path[MAX_PATH]= { 0 };
	ItemIdList idlPath;
	if (folder->GetPath(idlPath))
		idlPath.GetPath(path);
	edit_path_.SetWindowText(path);
}


void CopyMoveDlg::Impl::ItemClicked(ListViewCtrl& ctrl, int index, size_t param)
{
	if (static_cast<size_t>(index) < current_paths_.size())
		list_.SetPath(*current_paths_[index]);
}

CString CopyMoveDlg::Impl::GetTooltipText(ListViewCtrl& ctrl, int index, size_t param)
{
	if (param < current_paths_.size())
		return current_paths_[param]->GetPath();
	else
		return CString();
}

int CopyMoveDlg::Impl::GetItemCheckState(ListViewCtrl& ctrl, int index, size_t)	{ return false; }
void CopyMoveDlg::Impl::ItemColors(ListViewCtrl& ctrl, int index, size_t param, COLORREF& rgb_text, COLORREF& rgb_backgnd) {}
bool CopyMoveDlg::Impl::IsItemEnabled(ListViewCtrl& ctrl, int index, size_t param) { return true; }
bool CopyMoveDlg::Impl::FilterItem(ListViewCtrl& ctrl, int index, size_t param, bool group, const String& label, bool& filter_in) { return false; }
void CopyMoveDlg::Impl::DrawItemBackground(ListViewCtrl& ctrl, int index, size_t param, CDC& dc, CRect rect) {}


void CopyMoveDlg::OnBrowse()
{
	UpdateData();

	CFolderSelect fs(this);
	ItemIdList folder(path_);
	if (!fs.DoSelect(_T("选择目标文件夹"), folder))
		return;
//	CString path= fs.DoSelectPath(_T("Select destination folder")/*RString(IDS_SELECT_DEST)*/, path_);

//	if (path.IsEmpty())
//		return;

	path_ = folder.GetPath();
	impl_.list_.SetPath(folder);

//	UpdateData(false);
}


BOOL CopyMoveDlg::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	int right= 0;

	if (impl_.recent_.m_hWnd)
	{
		WINDOWPLACEMENT wp;
		if (impl_.recent_.GetWindowPlacement(&wp))
			right = wp.rcNormalPosition.right;
	}

	COLORREF gray= ::GetSysColor(COLOR_3DFACE);
	for (int i= 0; i < impl_.color_shades_.size(); ++i)
		dc->FillSolidRect(right++, 0, 1, rect.Height(), ::CalcShade(gray, impl_.color_shades_[i]));

	dc->FillSolidRect(right, 0, rect.Width() - right, rect.Height(), gray);

	return true;
}


void CopyMoveDlg::OnOK()
{
	if (impl_.edit_path_.m_hWnd == 0)
		return;

	try
	{
		// validate path

		//TODO: suppress zip "folders"

		CString str;
		impl_.edit_path_.GetWindowText(str);

		TCHAR path[MAX_PATH];
		if (!::PathCanonicalize(path, str))
		{
			// error
			new BalloonMsg(&impl_.edit_path_, _T("路径无效"), _T("目标路径不可用."), BalloonMsg::IERROR);
			return;
		}

		if (!::PathIsDirectory(path))
		{
			// check parent dir
			TCHAR buf[MAX_PATH];
			_tcscpy(buf, path);

			::PathRemoveFileSpec(buf);

			if (!::PathIsDirectory(buf))
			{
				new BalloonMsg(&impl_.edit_path_, _T("路径无效"), _T("目标路径不可用."), BalloonMsg::IERROR);
				return;
			}

			// create this last folder
			if (!::CreateDirectory(path, 0))
			{
				// error
				new BalloonMsg(&impl_.edit_path_, _T("创建文件夹失败"), _T("指定的文件夹未能创建."),
					BalloonMsg::IERROR);
				return;
			}
		}


		DialogChild::OnOK();

		path_ = path;

		// add path to the recent paths
		InsertUniquePath(impl_.recent_paths_, ItemIdList(path), true);
/* test
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\dane"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\dane\\web"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\dane\\grid"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\dane\\grid\\web"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\dane\\do_zgrywu"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\cougar"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\data91"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\filmy"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\logs"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\lots"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\osts"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\recipes"), true);
InsertUniquePath(impl_.recent_paths_, ItemIdList(L"c:\\review"), true);
*/
		// write paths
		RecentPaths(impl_.recent_paths_, true, RECENT_PATHS_SECTION, MAX_RECENT_PATHS);
	}
	CATCH_ALL
}


void CopyMoveDlg::OnPathEditChanged()
{
	if (btn_ok_.m_hWnd && impl_.edit_path_.m_hWnd)
		btn_ok_.EnableWindow(impl_.edit_path_.GetWindowTextLength() > 0);
}
