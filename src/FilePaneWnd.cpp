/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FilePaneWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FilePaneWnd.h"
#include "FileListCtrl.h"
#include "FolderSelect.h"
#include "ToolBarWnd.h"
#include "CatchAll.h"
#include "DlgAutoResize.h"
#include "CtrlDraw.h"
#include "DibDC.h"
#include "Dib.h"
#include "DrawBmpEffects.h"
#include "Color.h"
#include "PathEdit.h"
#include "ShellItem.h"
#include "ItemIdList.h"
#include "FolderSelect.h"
#include "MenuFolders.h"
#include "BalloonMsg.h"
#include <shlwapi.h>
#include "RecentPaths.h"


enum { ID_GO_UP= 1000, ID_LIST_VIEW, ID_ICON_VIEW };
enum { IDD = IDD_FILE_PANE };
enum { MSG_UPDATE_FOLDERS= WM_USER + 1234 };
const int MAX_RECENT_PATHS= 20;

struct FilePaneWnd::Impl
{
	FileListCtrl list_;
	ToolBarWnd toolbar_;
	ToolBarWnd popup_;
	CPathEdit address_;
	CStatusBar status_bar_;
	CStatic label_;
	CFont bold_fnt_;
	CBrush bkgnd_brush_;
	CBrush light_bkgnd_brush_;
	bool showMask_;
	bool showNew_;
	CString addr_label_;
	bool includeFiles_;
	PathVector recent_paths_;
	PathVector current_paths_;
	String paths_reg_section_;
	// file filter-in callback
	FileFilterCallback includeFileItem_;
	// let the user set file spec in this callback:
	boost::function<bool (CPoint pos)> selectFileTypes_;
	// new folder btn clicked:
	boost::function<void ()> createNewFolder_;
	DlgAutoResize auto_resize_;
	int min_width_;
	bool dialog_ready_;
	CString last_path_;		// detect path change on kill focus
	ItemIdListPtr currentPath_;
	ToolBarWnd filter_tb_;

	Impl();
	~Impl();

	void Init(CWnd* parent, bool showNew, bool showMask);
	void FolderPopup(CWnd* parent);
	void MaskPopup(CWnd* parent);
	void ResizeEditBox();
	void Browse(CWnd* parent);
	void FolderChanged(ShellFolderPtr folder);
	void BuildPathMenu(CMenuFolders& menu, const TCHAR* section);
	void SetFileMask(const FileFilterCallback& includeFileItem, const String& btnMaskText);
	void SetInfo();
	bool ValidatePathField(bool createDir, CString& path);
	void GoTo(const TCHAR* path);
};


// FilePaneWnd dialog

FilePaneWnd::FilePaneWnd(CWnd* parent /*=NULL*/)
  : CDialog(IDD, parent), pImpl_(new Impl)
{
}


FilePaneWnd::~FilePaneWnd()
{
}


void FilePaneWnd::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(FilePaneWnd, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
//	ON_WM_CTLCOLOR()
	ON_NOTIFY(TBN_DROPDOWN, IDC_POPUP, OnTbDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_TOOLBAR, OnTbDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_TOOLBAR2, OnTbDropDown)
	ON_EN_SETFOCUS(IDC_ADDRESS, OnAddressSetFocus)
	ON_EN_KILLFOCUS(IDC_ADDRESS, OnAddressKillFocus)
	ON_COMMAND(ID_GO_UP, OnGoLevelUp)
	ON_COMMAND(ID_LIST_VIEW, OnListView)
	ON_COMMAND(ID_ICON_VIEW, OnIconView)
	ON_MESSAGE(MSG_UPDATE_FOLDERS, OnUpdateFolders)
	ON_COMMAND(ID_NEW, OnNewFolder)
END_MESSAGE_MAP()


// FilePaneWnd message handlers

FilePaneWnd::Impl::Impl()
{
	showMask_ = showNew_ = false;
	includeFiles_ = true;
	min_width_ = 100;
	dialog_ready_ = false;
}

FilePaneWnd::Impl::~Impl()
{}


bool FilePaneWnd::Create(CWnd* parent, const TCHAR* label, bool showNew, bool showMask, bool includeFiles)
{
	pImpl_->showNew_ = showNew;
	pImpl_->showMask_ = showMask;
	pImpl_->addr_label_ = label;
	pImpl_->includeFiles_ = includeFiles;

	if (!CDialog::Create(IDD, parent))
		return false;

	pImpl_->dialog_ready_ = true;

	return true;
}


BOOL FilePaneWnd::OnInitDialog()
{
	CDialog::OnInitDialog();

	pImpl_->Init(this, pImpl_->showNew_, pImpl_->showMask_);

	return false;
}


int FilePaneWnd::OnCreate(CREATESTRUCT* cs)
{
	CDialog::OnCreate(cs);
	return 0;
}


void FilePaneWnd::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case IDC_POPUP:		// popup
		pImpl_->FolderPopup(this);
		break;

	case ID_MASK:
		break;

	case IDC_FILTER:
		pImpl_->MaskPopup(this);
		break;
	}
}


void FilePaneWnd::Impl::Init(CWnd* parent, bool showNew, bool showMask)
{
	VERIFY(address_.SubclassDlgItem(IDC_ADDRESS, parent));
	VERIFY(popup_.SubclassDlgItem(IDC_POPUP, parent));
	VERIFY(toolbar_.SubclassDlgItem(IDC_TOOLBAR, parent));
	VERIFY(filter_tb_.SubclassDlgItem(IDC_TOOLBAR2, parent));
	VERIFY(list_.SubclassDlgItem(IDC_LIST, parent));
	VERIFY(status_bar_.SubclassDlgItem(IDC_STATUS_BAR, parent));
	VERIFY(label_.SubclassDlgItem(IDC_LABEL, parent));

	WINDOWPLACEMENT wp;
	if (parent->GetWindowPlacement(&wp))
		min_width_ = wp.rcNormalPosition.right - wp.rcNormalPosition.left;

	if (bold_fnt_.m_hObject == 0)
	{
		LOGFONT lf;
		if (CFont* font= parent->GetFont())
			font->GetLogFont(&lf);
		else
		{
			HFONT hfont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
			::GetObject(hfont, sizeof(lf), &lf);
			//lf.lfQuality = ANTIALIASED_QUALITY;
			lf.lfHeight += 1;
			_tcscpy(lf.lfFaceName, _T("Tahoma"));
		}
		lf.lfWeight =  FW_BOLD;
		bold_fnt_.CreateFontIndirect(&lf);
	}

	toolbar_.SetOnIdleUpdateState(false);
	const int commands[]= { ID_GO_UP, ID_MASK, ID_NEW, ID_LIST_VIEW, ID_ICON_VIEW, ID_DETAILED_VIEW };

	char* pattern= showNew ? "P|:P|gg." : "P|::gg.";
	//if (!showNew)
	//	pattern[3] = '.';
	//if (!showMask)
	//	pattern[2] = ':';

	toolbar_.AddButtons(pattern, commands, IDR_FILE_PANE_TB, IDS_FILE_PANE_TB);
	toolbar_.CheckButton(ID_LIST_VIEW, true);

	popup_.SetOnIdleUpdateState(false);
	{
		const int command= IDC_POPUP;
		popup_.AddButtons("v", &command, IDB_FOLDER_POPUP);
	}

	filter_tb_.SetOnIdleUpdateState(false);
	{
		const int command= IDC_FILTER;
		filter_tb_.AddButtons("V", &command, IDR_FILE_PANE_FILTER_TB, IDS_ALL_FILES);
//		filter_tb_.SetButton
	}

	int width[]= { -1 };
	status_bar_.SendMessage(SB_SETPARTS, array_count(width), LPARAM(&width));

	label_.SetWindowText(addr_label_);
	ResizeEditBox();

	list_.SetFolderChangedCallback(boost::bind(&Impl::FolderChanged, this, _1));

	auto_resize_.BuildMap(parent);

	auto_resize_.SetWndResizing(IDC_ADDRESS, DlgAutoResize::RESIZE_H);
	auto_resize_.SetWndResizing(IDC_POPUP, DlgAutoResize::MOVE_H);
	auto_resize_.SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE);
	auto_resize_.SetWndResizing(IDC_STATUS_BAR, DlgAutoResize::MOVE_V_RESIZE_H);
	auto_resize_.SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_V);
	//auto_resize_.SetWndResizing(IDC_NEW_ONLY, DlgAutoResize::MOVE);
	auto_resize_.SetWndResizing(IDC_TOOLBAR2, DlgAutoResize::MOVE_V_RESIZE_H);

	auto_resize_.Resize();

	SetInfo();
}


void FilePaneWnd::Impl::FolderChanged(ShellFolderPtr folder)
{
	TCHAR path[MAX_PATH]= { 0 };
	ItemIdListPtr iidpath= ItemIdListPtr(new ItemIdList);
	if (folder->GetPath(*iidpath))
		iidpath->GetPath(path);
	address_.SetWindowText(path);
	address_.SetSel(0x7fff, 0x7fff);
	last_path_ = path;
	currentPath_ = iidpath;
	SetInfo();
}


void FilePaneWnd::Impl::ResizeEditBox()
{
	WINDOWPLACEMENT wp;
	if (label_.GetWindowPlacement(&wp))
	{
		CString str;
		label_.GetWindowText(str);

		CClientDC dc(&label_);
		CFont* old= dc.SelectObject(&bold_fnt_);

		CRect r= wp.rcNormalPosition;
		int w= dc.GetTextExtent(str).cx + 3;
		int x= r.left + w;

		dc.SelectObject(old);

		label_.SetWindowPos(0, 0, 0, w, r.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		if (address_.GetWindowPlacement(&wp))
		{
			CRect r= wp.rcNormalPosition;
			address_.SetWindowPos(0, x, r.top, r.right - x, r.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}


void FilePaneWnd::Impl::BuildPathMenu(CMenuFolders& menu, const TCHAR* section)
{
	if (section == 0 || *section == 0)
		return;

	if (current_paths_.empty())
	{
//		if (recent_paths_.empty())		// read recent paths
//			RecentPaths(recent_paths_, false, section);

		AddCommonPaths(current_paths_, 0);

		// append all recent paths
		AppendPaths(current_paths_, recent_paths_);
	}

	menu.InsertItem(ID_BROWSE, _T("&Browse..."));
	menu.AppendMenu(MF_SEPARATOR);

	const size_t count= current_paths_.size();
	for (size_t i= 0; i < count; ++i)
		menu.InsertItem(*current_paths_[i], static_cast<UINT>(ID_RECENT_PATH_FIRST + i), current_paths_[i]->GetName());
}


void FilePaneWnd::Impl::MaskPopup(CWnd* parent)
{
	try
	{
		CRect rect(0,0,0,0);
		filter_tb_.GetRect(IDC_FILTER, rect);
		filter_tb_.ClientToScreen(rect);

		CPoint pos(rect.left, rect.bottom);

		if (selectFileTypes_)
			selectFileTypes_(pos);

		//
	}
	CATCH_ALL_W(parent)
}


void FilePaneWnd::Impl::FolderPopup(CWnd* parent)
{
	try
	{
		CMenuFolders menu;
//CMenu menu;
//		if (!menu.LoadMenu(IDR_FILE_PANE_POPUP))
//			return;

		CMenu* popup= &menu;//menu.GetSubMenu(0);

		//TODO
//		BuildRecentFoldersMenu(menu, *recent_path_list_);
		BuildPathMenu(menu, paths_reg_section_.c_str());

		CRect rect(0,0,0,0);
		popup_.GetRect(IDC_POPUP, rect);

		//parent-
		popup_.ClientToScreen(rect);

		int cmd= popup->TrackPopupMenu(TPM_RIGHTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			rect.right, rect.bottom, parent);

		if (cmd == 0) // || recent_path_list_->GetSize() == 0)
			return;

		if (cmd == ID_BROWSE)
		{
			Browse(parent);
		}
		else if (cmd >= ID_RECENT_PATH_FIRST)
		{
			cmd -= ID_RECENT_PATH_FIRST;

			currentPath_ = current_paths_[cmd];

			last_path_ = currentPath_->GetPath();
			address_.SetWindowText(last_path_);
			list_.SetPath(*currentPath_, includeFiles_, includeFileItem_);

			SetInfo();
		}

//		const CString& str= (*recent_path_list_)[cmd - ID_RECENT_PATH_FIRST];

		//DWORD attrib= ::GetFileAttributes(str);
		//if (attrib == 0-1 || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
		//	return;

		// scan selected folder
//		ItemIdList idl(str);
//		Browser(idl, true);
	}
	CATCH_ALL_W(parent)
}


void FilePaneWnd::OnSize(UINT, int cx, int cy)
{
	Invalidate();
	pImpl_->auto_resize_.Resize();
}


// pane background color
const COLORREF BLUE= ::CalcShade(::GetSysColor(COLOR_3DFACE), -0.0f);// RGB(53, 104, 204);
// title opacity
const float TITLE_OPACITY= 0.70f;
const float PANE_OPACITY= 0.26f;


BOOL FilePaneWnd::OnEraseBkgnd(CDC* dc)
{
	if (dc == 0)
		return false;

	try
	{
		WINDOWPLACEMENT wp1, wp2;
		if (pImpl_->list_.GetWindowPlacement(&wp1) && pImpl_->status_bar_.GetWindowPlacement(&wp2))
		{
			CRect border_rect= wp1.rcNormalPosition;
			border_rect |= wp2.rcNormalPosition;
			border_rect.InflateRect(1, 1);

			WINDOWPLACEMENT wp;
			pImpl_->address_.GetWindowPlacement(&wp);

			CRect rect= border_rect;
			rect |= wp.rcNormalPosition;

			CRect client_rect(0,0,0,0);
			GetClientRect(client_rect);

			if (client_rect.IsRectEmpty())
				return true;

			const int MARGIN= 6;

			rect.bottom = client_rect.bottom - MARGIN;

			COLORREF bkgnd= ::GetSysColor(COLOR_3DFACE);

			DibDC dc(*dc, client_rect, bkgnd);

			CRect r= rect;
			r.top++;

			const int H= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top - 3;

			int top= r.top;
			r.top = top + H + MARGIN;
			::DrawGlowEffect(dc.GetDib(), r, BLUE, PANE_OPACITY, MARGIN, false);

			dc.FillSolidRect(r, ::CalcNewColor(bkgnd, BLUE, PANE_OPACITY));

			r.bottom = r.top - MARGIN;
			r.top = top + 1;
			::DrawGlowEffect(dc.GetDib(), r, BLUE, TITLE_OPACITY, MARGIN, false);

			dc.FillSolidRect(r, ::CalcNewColor(bkgnd, BLUE, TITLE_OPACITY));

			CtrlDraw::DrawBorder(dc, border_rect);

			dc.BitBlt();
		}
	}
	catch (...)
	{}

	return true;
}


HBRUSH FilePaneWnd::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr= CDialog::OnCtlColor(dc, wnd, ctl_color);

	int id= wnd ? wnd->GetDlgCtrlID() : 0;

	if (pImpl_->bkgnd_brush_.m_hObject == 0)
	{
		COLORREF bkgnd= ::GetSysColor(COLOR_3DFACE);
		pImpl_->bkgnd_brush_.CreateSolidBrush(::CalcNewColor(bkgnd, BLUE, TITLE_OPACITY));
	}
	if (pImpl_->light_bkgnd_brush_.m_hObject == 0)
	{
		COLORREF bkgnd= ::GetSysColor(COLOR_3DFACE);
		pImpl_->light_bkgnd_brush_.CreateSolidBrush(::CalcNewColor(bkgnd, BLUE, PANE_OPACITY));
	}

	switch (id)
	{
	case IDC_LABEL:
		dc->SelectObject(&pImpl_->bold_fnt_);
		dc->SetTextColor(RGB(255,255,255));
		{
			COLORREF bkgnd= ::GetSysColor(COLOR_3DFACE);
			bkgnd = ::CalcNewColor(bkgnd, BLUE, TITLE_OPACITY);
			dc->SetBkColor(bkgnd);
		}
		return pImpl_->bkgnd_brush_;

	case IDC_LABEL_2:
	//case IDC_NEW_ONLY:
		//dc->SelectObject(&pImpl_->bold_fnt_);
		//dc->SetTextColor(RGB(255,255,255));
		{
			COLORREF bkgnd= ::GetSysColor(COLOR_3DFACE);
			bkgnd = ::CalcNewColor(bkgnd, BLUE, PANE_OPACITY);
			dc->SetBkColor(bkgnd);
		}
		return pImpl_->light_bkgnd_brush_;
	}

	return hbr;
}


void FilePaneWnd::OnOK()
{}

void FilePaneWnd::OnCancel()
{
	if (CWnd* wnd= GetParent())
		wnd->SendMessage(WM_COMMAND, IDCANCEL, 0);
}


void FilePaneWnd::Impl::Browse(CWnd* parent)
{
	CString path;
	address_.GetWindowText(path);

	CFolderSelect fs(parent);
	currentPath_ = ItemIdListPtr(new ItemIdList(path));
	if (!fs.DoSelect(_T("Select folder"), *currentPath_))
		return;

	last_path_ = currentPath_->GetPath();
	address_.SetWindowText(last_path_);
	list_.SetPath(*currentPath_, includeFiles_, includeFileItem_);

	SetInfo();
}


void FilePaneWnd::Impl::SetInfo()
{
	const int count= list_.GetItemCount();

	CString str;
	if (count <= 0)
		str = _T("No Items");
	else if (count == 1)
		str = _T("1 Item");
	else
		str.Format(_T("%d Items"), count);

	const TCHAR* p= str;
	status_bar_.SendMessage(SB_SETTEXT, SBT_NOBORDERS, LPARAM(p));
}


void FilePaneWnd::SetFileMask(const FileFilterCallback& includeFileItem, const String& btnMaskText)
{
	pImpl_->SetFileMask(includeFileItem, btnMaskText);
}


void FilePaneWnd::Impl::SetFileMask(const FileFilterCallback& includeFileItem, const String& btnMaskText)
{
	includeFileItem_ = includeFileItem;

	if (currentPath_.get())
	{
		list_.SetPath(*currentPath_, includeFiles_, includeFileItem);
		SetInfo();
	}

	filter_tb_.SetButtonText(IDC_FILTER, btnMaskText.c_str());
}


void FilePaneWnd::OnAddressSetFocus()
{
	if (!pImpl_->dialog_ready_)
		return;

	pImpl_->address_.GetWindowText(pImpl_->last_path_);
}


void FilePaneWnd::OnAddressKillFocus()
{
	if (!pImpl_->dialog_ready_)
		return;

	CString path;
	pImpl_->address_.GetWindowText(path);

	if (path != pImpl_->last_path_)
	{
		// delay info about leaving address field, so we don't update folders when OK is hit
		PostMessage(MSG_UPDATE_FOLDERS);
	}
}

void FilePaneWnd::OnGoLevelUp()		{ pImpl_->list_.GoLevelUp(); }
void FilePaneWnd::OnListView()		{ pImpl_->list_.SetListView(); }
void FilePaneWnd::OnIconView()		{ pImpl_->list_.SetIconView(); }


void FilePaneWnd::SetPathsRegistrySection(const TCHAR* section)
{
	pImpl_->paths_reg_section_ = section;

	// initialize recent paths
	if (pImpl_->recent_paths_.empty())		// read recent paths
		RecentPaths(pImpl_->recent_paths_, false, section, MAX_RECENT_PATHS);
}


void FilePaneWnd::SetFileTypeSelect(const boost::function<bool (CPoint pos)>& selectFileTypes)
{
	pImpl_->selectFileTypes_ = selectFileTypes;
}


void FilePaneWnd::SetNewFolderCmd(const boost::function<void ()>& createNewFolderDlg)
{
	pImpl_->createNewFolder_ = createNewFolderDlg;
}


int FilePaneWnd::GetMinWidth() const
{
	return pImpl_->min_width_;
}


LRESULT FilePaneWnd::OnUpdateFolders(WPARAM, LPARAM)
{
	CString path;
	if (pImpl_->ValidatePathField(false, path))
	{
		//pImpl_->address_.GetWindowText(path);
		pImpl_->GoTo(path);
	}
	return 0;
}


bool FilePaneWnd::Impl::ValidatePathField(bool createDir, CString& outPath)
{
	CString str;
	address_.GetWindowText(str);

	TCHAR path[MAX_PATH];
	if (str.IsEmpty() || !::PathCanonicalize(path, str))
	{
		// error
		new BalloonMsg(&address_, _T("Invalid Path"), _T("Destination path is not valid."), BalloonMsg::IERROR);
		return false;
	}

	if (!::PathIsDirectory(path))
	{
		// check parent dir
		TCHAR buf[MAX_PATH];
		_tcscpy(buf, path);

		::PathRemoveFileSpec(buf);

		if (!::PathIsDirectory(buf))
		{
			new BalloonMsg(&address_, _T("Invalid Path"), _T("Destination path is not valid."), BalloonMsg::IERROR);
			return false;
		}

		if (createDir)
		{
			// create this last folder
			if (!::CreateDirectory(path, 0))
			{
				// error
				new BalloonMsg(&address_, _T("Cannot Create Folder"), _T("Specified folder cannot be created."),
					BalloonMsg::IERROR);
				return false;
			}
		}
		else
			return false;
	}

	outPath = path;

	return true;
}


void FilePaneWnd::Impl::GoTo(const TCHAR* path)
{
	currentPath_ = ItemIdListPtr(new ItemIdList(path));
	list_.SetPath(*currentPath_, includeFiles_, includeFileItem_);

	SetInfo();
}


void FilePaneWnd::SetPath(const TCHAR* path)
{
	pImpl_->address_.SetWindowText(path);
	pImpl_->last_path_ = path;
	pImpl_->GoTo(path);
}


CString FilePaneWnd::GetPath(bool validate)
{
	if (validate)
	{
		CString path;
		if (!pImpl_->ValidatePathField(true, path))
			return _T("");

		return path;
	}

	CString path;
	pImpl_->address_.GetWindowText(path);

	return path;
}


void FilePaneWnd::GetFiles(std::vector<FileInfo>& files)
{
	pImpl_->list_.GetFiles(files);
}


void FilePaneWnd::AddRecentPath(const TCHAR* path)
{
	// add path to the recent paths
	InsertUniquePath(pImpl_->recent_paths_, ItemIdList(path), true);
}


void FilePaneWnd::StoreRecentPaths()
{
	ASSERT(!pImpl_->paths_reg_section_.empty());

	// write paths
	RecentPaths(pImpl_->recent_paths_, true, pImpl_->paths_reg_section_.c_str(), MAX_RECENT_PATHS);
}


void FilePaneWnd::OnNewFolder()
{
	if (pImpl_->createNewFolder_)
		pImpl_->createNewFolder_();
}


void FilePaneWnd::Refresh()
{
	if (pImpl_->currentPath_.get())
	{
		pImpl_->list_.SetPath(*pImpl_->currentPath_, pImpl_->includeFiles_, pImpl_->includeFileItem_);
		pImpl_->SetInfo();
	}
}
