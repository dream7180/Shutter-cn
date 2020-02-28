/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ColorMngDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ColorMngDlg.h"
#include "ProfileVector.h"
#include "ColorProfile.h"
#include "UniqueLetter.h"
#include "MultiMonitor.h"
#include "WhistlerLook.h"
#include "CatchAll.h"
#include "UIElements.h"

namespace {
	const TCHAR* REG_ICC_DLG=		_T("ICC Dialog");
	const TCHAR* REG_COL_WIDTH=		_T("ColumnWidth");
}


void DuplicateAmpersand(String& str)
{
	String::size_type pos= str.find(_T('&'), 0);

	while (pos != String::npos)
	{
		str.insert(pos, _T("&"));
		pos += 2;

		if (pos >= str.length())
			break;

		pos = str.find(_T('&'), pos);
	}
}


Path GetProfilesFolder()
{
	Path dir;

	typedef BOOL (WINAPI *fnGetColorDirectory)(const TCHAR*, TCHAR*, DWORD*);

	if (HMODULE lib= ::LoadLibrary(_T("mscms.dll")))
	{
		fnGetColorDirectory pfnGetColorDirectory= reinterpret_cast<fnGetColorDirectory>(::GetProcAddress(lib,
#ifdef UNICODE
				"GetColorDirectoryW"));
#else
				"GetColorDirectoryA"));
#endif // !UNICODE

		if (pfnGetColorDirectory)
		{
			TCHAR path[2 * MAX_PATH + 2]= { 0 };
			DWORD path_size= 2 * MAX_PATH + 1;	// MSDN says it's in bytes (?)
			if (pfnGetColorDirectory(0, path, &path_size))
				dir = path;
		}

		::FreeLibrary(lib);
	}

	if (dir.empty())
	{
		TCHAR path[MAX_PATH + 2]= { 0 };
		if (::GetSystemDirectory(path, MAX_PATH + 1) > 0)
		{
			dir = path;
			dir += _T("\\spool\\drivers\\color");
		}
	}

	return dir;
}


//int XcmsErrorHandlerFunction(int ErrorCode, const char* error_text)
//{
//	throw make_pair(ErrorCode, error_text);
//}


// ColorMngDlg dialog

ColorMngDlg::ColorMngDlg(CWnd* parent /*=NULL*/)
 : CDialog(ColorMngDlg::IDD, parent), wnd_pos_(REG_ICC_DLG)
{
	profile_files_ready_ = false;
	changed_ = false;
}

ColorMngDlg::~ColorMngDlg()
{}


void ColorMngDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_TREE, tree_wnd_);
	DDX_Control(DX, IDC_TOOLBAR, tool_bar_wnd_);
}


BEGIN_MESSAGE_MAP(ColorMngDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_MESSAGE(ExtTreeCtrl::NOTIFY_ITEM_CLICKED, OnItemClicked)
	ON_MESSAGE(ExtTreeCtrl::NOTIFY_SEL_CHANGED, OnItemChanged)
END_MESSAGE_MAP()


// ColorMngDlg message handlers


BOOL ColorMngDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT(monitor_viewer_);
	ASSERT(monitor_main_wnd_);
	ASSERT(default_image_);

	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDB_ICM_ICONS), RT_BITMAP);
	image_list_.Attach(ImageList_LoadImage(inst, MAKEINTRESOURCE(IDB_ICM_ICONS),
		17, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION));

	tree_wnd_.SetImageList(&image_list_);

	CRect rect;
	GetClientRect(rect);

	CSize bar_size(::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CYHSCROLL));
	rect.left = rect.right - bar_size.cx;
	rect.top = rect.bottom - bar_size.cy;

	if (WhistlerLook::IsAvailable())
		VERIFY(grip_wnd_.Create(SBS_SIZEGRIP | WS_CHILD | WS_VISIBLE, rect, this, IDC_GRIP));

	int commands[]= { ID_NEW, ID_DELETE };
	tool_bar_wnd_.SetOnIdleUpdateState(false);
	tool_bar_wnd_.AddButtons("PP.", commands, IDB_ICM_TOOLBAR, IDS_ICM_TOOLBAR);

	tree_wnd_.InsertColumn(0, _T("项目"), Pixels(140));
	tree_wnd_.InsertColumn(1, _T("色彩配置"), Pixels(160));
	tree_wnd_.InsertColumn(2, _T("渲染意向"), Pixels(100));
	tree_wnd_.InsertColumn(3, _T("启用"), Pixels(70), HDF_CENTER);

	monitors_ = new CExtTreeRootNode(_T("监视器"), 0);
	printers_ = new CExtTreeRootNode(_T("打印机"), 1);
	images_ = new CExtTreeRootNode(_T("图像"), 2);

	new ExtTreeRow(monitors_.get(), monitor_main_wnd_);
	new ExtTreeRow(monitors_.get(), monitor_viewer_);
	if (default_printer_)
		new ExtTreeRow(printers_.get(), default_printer_, ExtTreeRow::OUTPUT | ExtTreeRow::RENDERING_INTENT);
	new ExtTreeRow(images_.get(), default_image_, ExtTreeRow::NONE);

	InsertItems(monitors_.get());
	if (default_printer_)
		InsertItems(printers_.get());
	InsertItems(images_.get());

	dlg_resize_map_.BuildMap(this);

	dlg_resize_map_.SetWndResizing(IDC_TREE, DlgAutoResize::RESIZE);
	dlg_resize_map_.SetWndResizing(IDOK, DlgAutoResize::MOVE);
	dlg_resize_map_.SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	if (WhistlerLook::IsAvailable())
		dlg_resize_map_.SetWndResizing(IDC_GRIP, DlgAutoResize::MOVE);
	dlg_resize_map_.SetWndResizing(IDC_NAME, DlgAutoResize::MOVE_V_RESIZE_H);
	//dlg_resize_map_.SetWndResizing(IDC_DESC, DlgAutoResize::MOVE_V_RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_INFO, DlgAutoResize::MOVE_V_RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_NAME, DlgAutoResize::MOVE_V);
	//dlg_resize_map_.SetWndResizing(IDC_LABEL_DESC, DlgAutoResize::MOVE_V);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_INFO, DlgAutoResize::MOVE_V);

	std::vector<int16> width_array;
	int count= tree_wnd_.header_wnd_.GetItemCount();
	if (GetProfileVector(REG_ICC_DLG, REG_COL_WIDTH, width_array) && width_array.size() == count)
		for (int i= 0; i < count; ++i)
			tree_wnd_.SetColumnWidth(i, width_array[i]);

	SetIcon(AfxGetApp()->LoadIcon(IDI_RAINBOW), false);

	if (wnd_pos_.IsRegEntryPresent())
	{
		CRect rect= wnd_pos_.GetLocation(true);
		SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	GotoDlgCtrl(&tree_wnd_.tree_wnd_);

	//
//	::GetColorDirectory(0, path, MAX_PATH);
/*
cmsSetErrorHandler(&XcmsErrorHandlerFunction);

try {
ColorProfile cp1, cp2;
if (cp1.Open(L"D:\\WINDOWS\\system32\\spool\\drivers\\color\\AdobeRGB1998.icc")
	&& cp2.Open(L"D:\\WINDOWS\\system32\\spool\\drivers\\color\\Trinitron Compatible 9300K G2.2.icm"))
{
	ColorTransform ct;
	static BYTE in[]= { 0x80, 0x80, 0x80, 0x10, 0x10, 0x10, 0xF0, 0xF0, 0xF0 };
	static BYTE out[]= { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	vector<BYTE> in(3 * 2240 * 1680, 0x80);
	vector<BYTE> out(3 * 2240 * 1680, 0);

	if (ct.Create(cp1, TYPE_BGR_8, cp2, TYPE_BGR_8, INTENT_PERCEPTUAL))
		ct.Transform(&in[0], &out[0], in.size() / 3);

	ct.Delete();

	if (ct.Create(cp1, TYPE_BGR_8, cp2, TYPE_BGR_8, INTENT_ABSOLUTE_COLORIMETRIC))
		ct.Transform(in, out, 3);

	int n= 1;
}
} catch (...) {}
*/
	return false;
}


void ColorMngDlg::InsertItems(ExtTreeNode* node)
{
	HTREEITEM node_handle= tree_wnd_.InsertItem(node, TVI_ROOT, 0, false, false);

	ExtTreeNode::TreeRowVector::iterator end= node->end();
	for (ExtTreeNode::TreeRowVector::iterator it= node->begin(); it != end; ++it)
		tree_wnd_.InsertItem(it->get(), node_handle, 0, false, false);

	tree_wnd_.tree_wnd_.Expand(node_handle, TVE_EXPAND);
}


void ColorMngDlg::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED && cx > 0 && cy > 0)
	{
		dlg_resize_map_.Resize();
	}
}


void ColorMngDlg::OnGetMinMaxInfo(MINMAXINFO* MMI)
{
	CDialog::OnGetMinMaxInfo(MMI);

	MMI->ptMinTrackSize.x = 300;
	MMI->ptMinTrackSize.y = 200;
}


void ColorMngDlg::OnDestroy()
{
	if (tree_wnd_.m_hWnd != 0)
		if (CHeaderCtrl* ctrl= &tree_wnd_.header_wnd_)
		{
			int col_count= ctrl->GetItemCount();

/*			vector<INT> col_order;
			col_order.resize(col_count);
			GetColumnOrderArray(&col_order.front(), col_order.size());
			WriteProfileVector(REG_INFO_PANE, REG_COL_WIDTH, col_order); */

			std::vector<int16> width_array;
			width_array.resize(col_count);
			for (int i= 0; i < col_count; ++i)
			{
				width_array[i] = tree_wnd_.GetColumnWidth(i);
				if (width_array[i] <= 0)
				{
					ASSERT(false);
					width_array[i] = 2;
				}
			}
			WriteProfileVector(REG_ICC_DLG, REG_COL_WIDTH, width_array);
		}

	wnd_pos_.StoreState(*this);

	CDialog::OnDestroy();
}


LRESULT ColorMngDlg::OnItemClicked(WPARAM item_param, LPARAM column)
{
	HTREEITEM item_handle= HTREEITEM(item_param);
	DWORD_PTR data= tree_wnd_.GetItemData(item_handle);

	if (ExtTreeRow* item= reinterpret_cast<ExtTreeRow*>(data))
		OnItemClicked(item, item_handle, static_cast<int>(column));

	return 0;
}


void ColorMngDlg::OnItemClicked(ExtTreeRow* tree_item, HTREEITEM item, int column)
{
	if (tree_item->icm_ == 0)
		return;

	if (column == ExtTreeRow::COL_RENDERING)	// rendering intent?
	{
		if (!tree_item->IsPopupMenuColumn(column))
			return;

		tree_wnd_.tree_wnd_.EnsureVisible(item);
		CRect rect= tree_wnd_.GetCellRect(item, static_cast<int>(column), true);

		CMenu menu;
		if (!menu.LoadMenu(IDR_ICM_DLG_POPUP))
			return;

		if (CMenu* popup= menu.GetSubMenu(0))
		{
			int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, rect.left, rect.bottom, this);

			switch (cmd)
			{
			case 1:		tree_item->icm_->rendering_ = ICMProfile::PICTURE;	break;
			case 2:		tree_item->icm_->rendering_ = ICMProfile::GRAPHIC;	break;
			case 3:		tree_item->icm_->rendering_ = ICMProfile::PROOF;	break;
			case 4:		tree_item->icm_->rendering_ = ICMProfile::MATCH;	break;
			default:	return;
			}
			tree_wnd_.RedrawItem(item);

			changed_ = true;
		}
	}
	else if (column == ExtTreeRow::COL_PROFILE)	// color profile?
	{
		tree_wnd_.tree_wnd_.EnsureVisible(item);
		CRect rect= tree_wnd_.GetCellRect(item, static_cast<int>(column), true);

		try
		{
			ShowProfilesMenu(tree_item, CPoint(rect.left, rect.bottom));
		}
		CATCH_ALL

	}
	else if (column == ExtTreeRow::COL_ENABLE_FLAG)	// enable flag?
	{
		tree_item->icm_->enabled_ = !tree_item->icm_->enabled_;
		tree_wnd_.RedrawItem(item);
		changed_ = true;
	}
	else if (column == ExtTreeRow::COL_ITEM_NAME)		// item name?
	{
		if (tree_item->icm_->name_editing_)
		{
			tree_wnd_.tree_wnd_.EnsureVisible(item);
			tree_wnd_.tree_wnd_.UpdateWindow();
			tree_wnd_.tree_wnd_.SetRedraw(false);

			CRect rect= tree_wnd_.GetCellRect(item, static_cast<int>(column), false);

			/*				CEditBox edit_wnd(&tree_wnd_, rect, false, 3);

			if (edit_wnd.Edit(root_node->rule_.code_))
			{
				//TODO: validation
				int code= edit_wnd.GetIntValue();
				if (code < 0 || code > 999)
				{
					CPoint pos= rect.CenterPoint();
					tree_wnd_.ClientToScreen(&pos);
					new BalloonMsg(&tree_wnd_, _T("error msg"), _T("explanation."), BalloonMsg::IERROR, pos);
				}
			}
			else
			{
				code;
				tree_wnd_.RedrawItem(item);
			}
			*/
			tree_wnd_.tree_wnd_.SetRedraw();
		}
	}
}

namespace {

	struct ICCProfileInfoLess
	{
		bool operator () (const ICCProfileInfo& i1, const ICCProfileInfo& i2) const
		{
			return lexicographical_compare(i1.file_.begin(), i1.file_.end(), i2.file_.begin(), i2.file_.end());
		}
	};
}


void ColorMngDlg::ShowProfilesMenu(ExtTreeRow* item, CPoint pos)
{
	ASSERT(item && item->icm_ != 0);

	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	CRect wnd_rect;
	GetWindowRect(wnd_rect);
	CRect monitor_rect= ::GetFullScreenRect(wnd_rect);

	if (!profile_files_ready_)
	{
		CWaitCursor wait;

		Path path= GetProfilesFolder();

		if (!path.empty() && path.IsFolder())
		{
			ICCScanner scanner(icc_files_);
			scanner.ScanDir(path, true);
			sort(icc_files_.begin(), icc_files_.end(), ICCProfileInfoLess());
		}

		profile_files_ready_ = true;
	}

	const UINT_PTR CMD_SELECT_FILE= icc_files_.size() + 1;
	const UINT_PTR CMD_DEFAULT_sRGB= icc_files_.size() + 2;
	const UINT_PTR CMD_LAST= CMD_DEFAULT_sRGB;

	UniqueLetter short_cuts;
	String item_text= _T("选择颜色配置文件...");
	short_cuts.SelectUniqueLetter(item_text);
	menu.AppendMenu(MF_STRING, CMD_SELECT_FILE, item_text.c_str());

	UINT_PTR selected_item= 0;

	item_text = _T("默认 sRGB 颜色配置");
	short_cuts.SelectUniqueLetter(item_text);
	menu.AppendMenu(MF_STRING, CMD_DEFAULT_sRGB, item_text.c_str());
	if (item->icm_->default_s_rgb_)
		selected_item = CMD_DEFAULT_sRGB;

	// (lame) estimation of menu item height
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	LOGFONT lf;
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	//lf.lfQuality = ANTIALIASED_QUALITY;
	CFont _font;
	_font.CreateFontIndirect(&lf);
	dc.SelectObject(&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int line_height= tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
	int item_height= line_height * 11 / 10 + 1;

	if (!icc_files_.empty())
	{
		menu.AppendMenu(MF_STRING | MF_SEPARATOR);

		size_t count= icc_files_.size();
		int menu_height= item_height * 3;
		for (size_t i= 0; i < count; ++i)
		{
			ColorProfilePtr profile= icc_files_[i].profile_;

			if (!item->IsMatchingProfile(profile))
				continue;

			String name= icc_files_[i].file_.GetFileName(); // + _T("  ") + icc_files_[i].profile_->GetDeviceClassName();

			if (item->icm_->profile_path_ == icc_files_[i].file_)	// TODO: lexico.... cmp
				selected_item = i + 1;

			DuplicateAmpersand(name);
			short_cuts.SelectUniqueLetter(name);
			UINT flags= MF_STRING;

			menu_height += item_height;
			if (menu_height > monitor_rect.Height())
			{
				menu_height = 0;
				flags |= MF_MENUBARBREAK;
			}
			menu.AppendMenu(flags, i + 1, name.c_str());
		}
	}

	if (selected_item > 0)
		menu.CheckMenuRadioItem(1, static_cast<UINT>(CMD_LAST), static_cast<UINT>(selected_item), MF_BYCOMMAND);

	int cmd= menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this);

	if (cmd > 0)
	{
		int index= cmd - 1;
		if (index < icc_files_.size())
		{
			// TODO: transform creation
			item->icm_->AssignProfile(icc_files_[index].file_, icc_files_[index].profile_);
		}
		else if (cmd == CMD_DEFAULT_sRGB)	// default rgb?
		{
			item->icm_->AssignDefault_sRGB();
		}
		else
		{
			// select ICM file
			String initial_dir;
			if (item->icm_->profile_path_.empty())
				initial_dir = GetProfilesFolder();
			else
				initial_dir = item->icm_->profile_path_.GetParentDir();

			CFileDialog dlg(true);
			dlg.m_pOFN->lpstrInitialDir = initial_dir.c_str();

			if (dlg.DoModal() != IDOK)
				return;

			CString str= dlg.GetPathName();

			ColorProfilePtr profile= new ColorProfile();
			if (!profile->Open(str))
			{
				MessageBox(_T("选定的文件不能打开或者不是可用的颜色配置."));
				return;
			}
			if (!item->IsMatchingProfile(profile))
			{
				MessageBox(_T("选定的颜色配置类型不匹配当前项目."));
				return;
			}
			item->icm_->AssignProfile(Path(str), profile);
		}

		changed_ = true;

		tree_wnd_.RedrawItem(item->GetHandle());
		UpdateInfo(item->icm_);
	}
}


HBRUSH ColorMngDlg::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr= CDialog::OnCtlColor(dc, wnd, ctl_color);

	int id= wnd ? wnd->GetDlgCtrlID() : 0;

	if (bold_fnt_.m_hObject == 0)
	{
		LOGFONT lf;
		if (CFont* font= GetParent()->GetFont())
			font->GetLogFont(&lf);
		else
		{
			HFONT font_handle= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
			::GetObject(font_handle, sizeof(lf), &lf);
			lf.lfHeight += 1;
			_tcscpy(lf.lfFaceName, _T("Tahoma"));
			//lf.lfQuality = ANTIALIASED_QUALITY;
		}
		lf.lfWeight =  FW_BOLD;
		bold_fnt_.CreateFontIndirect(&lf);
	}

	switch (id)
	{
	case IDC_LABEL_NAME:
//	case IDC_LABEL_DESC:
	case IDC_LABEL_INFO:
		dc->SelectObject(&bold_fnt_);
		break;
	}

	return hbr;
}


LRESULT ColorMngDlg::OnItemChanged(WPARAM item, LPARAM)
{
	if (DWORD_PTR data= tree_wnd_.GetItemData(HTREEITEM(item)))
		if (ExtTreeRow* item= reinterpret_cast<ExtTreeRow*>(data))
			UpdateInfo(item->icm_);

	return 0;
}


void ColorMngDlg::UpdateInfo(ICMProfilePtr icm)
{
	if (icm != 0 && icm->default_s_rgb_)
	{
		SetDlgItemText(IDC_NAME, _T("sRGB"));
		//SetDlgItemText(IDC_DESC, _T(""));
		SetDlgItemText(IDC_INFO, _T("默认 sRGB 颜色配置"));
	}
	else if (icm == 0 || icm->profile_ == 0)
	{
		SetDlgItemText(IDC_NAME, _T(""));
		//SetDlgItemText(IDC_DESC, _T(""));
		SetDlgItemText(IDC_INFO, _T(""));
	}
	else
	{
		SetDlgItemText(IDC_NAME, icm->profile_path_.c_str());
		//SetDlgItemText(IDC_DESC, icm->profile_->GetProductDesc().c_str());
		SetDlgItemText(IDC_INFO, icm->profile_->GetProfileInfo().c_str());
	}
}
