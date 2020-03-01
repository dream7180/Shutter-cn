/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SetupFavoritesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SetupFavoritesDlg.h"
#include "FavoritesFolders.h"
#include "FolderSelect.h"
#include "BalloonMsg.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void ResizeColumnsToFit(CListCtrl& list_wnd)
{
	if (list_wnd.m_hWnd == 0)
		return;

	CHeaderCtrl* header= list_wnd.GetHeaderCtrl();
	if (header == 0)
		return;

	int count= header->GetItemCount();

	CRect union_rect(0,0,0,0), rect;
	for (int i= 0; i < count; ++i)
		if (header->GetItemRect(i, rect))
			union_rect |= rect;

	CRect cl_rect;
	list_wnd.GetClientRect(cl_rect);

//	if (union_rect.Width() > cl_rect.Width() && union_rect.Width() > 0)
	{
		// resize columns to fit in a window
		for (int i= 0; i < count; ++i)
		{
			int width= list_wnd.GetColumnWidth(i);
			width = std::max(width * cl_rect.Width() / union_rect.Width(), 2);
			list_wnd.SetColumnWidth(i, width);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// SetupFavoritesDlg dialog


SetupFavoritesDlg::SetupFavoritesDlg(FavoriteFolders& folders, CWnd* parent /*=NULL*/)
	: folders_(folders), CDialog(SetupFavoritesDlg::IDD, parent),
	  wnd_pos_(_T("SetupFavorites")), min_size_(0, 0)
{
	//{{AFX_DATA_INIT(SetupFavoritesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SetupFavoritesDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(SetupFavoritesDlg)
	DDX_Control(DX, IDC_LIST, list_wnd_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SetupFavoritesDlg, CDialog)
	//{{AFX_MSG_MAP(SetupFavoritesDlg)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnGetDispInfo)
	ON_BN_CLICKED(ID_SET, OnSetFolder)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblClkList)
	ON_BN_CLICKED(ID_DELETE, OnDelete)
	ON_BN_CLICKED(ID_RENAME, OnRename)
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnCustDraw)
//	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST, OnEndLabelEdit)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST, OnBeginLabelEdit)
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY_RANGE(HDN_ENDTRACK, 0, 0xffff, OnEndTrack)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SetupFavoritesDlg message handlers

BOOL SetupFavoritesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);

	SHFILEINFO info;
	DWORD_PTR image_list= ::SHGetFileInfo(_T(""), NULL, &info, sizeof info,
		SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX);

	if (image_list)
		list_wnd_.SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, LPARAM(image_list));

	list_wnd_.InsertColumn(0, _T("文件夹"), LVCFMT_LEFT, Pixels(190), 0);
	list_wnd_.InsertColumn(1, _T("路径"), LVCFMT_LEFT, Pixels(240), 1);
//	list_wnd_.InsertColumn(2, _T("Root"), LVCFMT_LEFT, 50, 2);

//	list_wnd_.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	bool selected= true;

	for (int index= 0; index < folders_.GetCount(); ++index)
	{
		const FavoriteFolder* folder= folders_.GetFolder(index);

		LVITEM li;
		li.mask			= LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
		li.iItem		= index;
		li.iSubItem		= 0;
		li.state		= selected ? LVIS_SELECTED | LVIS_FOCUSED : 0; // INDEXTOOVERLAYMASK(1);
		selected = false;
		li.stateMask	= LVIS_SELECTED | LVIS_FOCUSED; //LVIS_OVERLAYMASK;
		li.pszText		= LPSTR_TEXTCALLBACK;
		li.cchTextMax	= 0;
		li.iImage		= folder->GetIconIndex();
		li.lParam		= reinterpret_cast<LPARAM>(folder);
		li.iIndent		= 0;
		list_wnd_.InsertItem(&li);
	}

//	GotoDlgCtrl(&list_wnd_);
//	return false;

	dlg_resize_map_.BuildMap(this);
	dlg_resize_map_.SetWndResizing(ID_RENAME, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(ID_SET, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(ID_DELETE, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDOK, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE);

	CRect rect(0,0,0,0);
	GetWindowRect(rect);
	min_size_ = rect.Size();

	if (wnd_pos_.IsRegEntryPresent())
	{
		CRect rect= wnd_pos_.GetLocation(true);
		SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	ResizeColumnsToFit(list_wnd_);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void SetupFavoritesDlg::OnOK()
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}


void SetupFavoritesDlg::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info = reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	if ((disp_info->item.mask & LVIF_TEXT) == 0 || disp_info->item.cchTextMax == 0)
		return;

	const FavoriteFolder* folder= GetFolder(disp_info->item.lParam);
	if (folder == 0)
		return;

	CString text;
	switch (disp_info->item.iSubItem)
	{
	case 0:
		text = folder->GetDisplayName();
		break;

	case 1:
		text = folder->GetDisplayPath();
		break;

	case 2:
		text = _T("     ");
//		text = folder.open_as_root_ ? _T("Yes") : _T("No");
		break;
	}

	_tcsncpy(disp_info->item.pszText, text, disp_info->item.cchTextMax);
	disp_info->item.pszText[disp_info->item.cchTextMax - 1] = '\0';
}


void SetupFavoritesDlg::OnSetFolder()
{
	if (FavoriteFolder* folder= GetSelectedItem())
		SelectFolder(*folder);
}


void SetupFavoritesDlg::OnDblClkList(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;

	if (FavoriteFolder* folder= GetSelectedItem())
		SelectFolder(*folder);
}


// current selection
//
FavoriteFolder* SetupFavoritesDlg::GetSelectedItem()
{
	int index= GetSelectedItemIndex();
	if (index < 0)
		return 0;
	return GetFolder(list_wnd_.GetItemData(index));
}


int SetupFavoritesDlg::GetSelectedItemIndex()
{
	POSITION pos= list_wnd_.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return -1;
	return list_wnd_.GetNextSelectedItem(pos);
}


void SetupFavoritesDlg::SelectFolder(FavoriteFolder& folder)
{
	CFolderSelect fs(this);

	if (fs.DoSelect(_T("选择收藏的文件夹"), folder.GetIDL()))
	{
		list_wnd_.Invalidate();		// redraw changed folder
	}
}


void SetupFavoritesDlg::OnRename()
{
	int index= GetSelectedItemIndex();
	if (index >= 0)
	{
		list_wnd_.SetFocus();
		list_wnd_.EditLabel(index);
	}
	else
	{
		new BalloonMsg(&list_wnd_, _T("请选择文件夹"),
			_T("请选择文件夹以重命名."), BalloonMsg::IERROR);
	}
}


void SetupFavoritesDlg::OnDelete()
{
	int index= GetSelectedItemIndex();
	if (index >= 0)
	{
		if (folders_.Remove(index))
		{
			list_wnd_.DeleteItem(index);
			list_wnd_.Invalidate();		// redraw
		}
	}
	else
	{
		new BalloonMsg(&list_wnd_, _T("请选择文件夹"),
			_T("请选择要从收藏夹移除的文件夹."), BalloonMsg::IERROR);
	}
}


#if 0	// custom draw check box
void SetupFavoritesDlg::OnCustDraw(NMHDR* nm_hdr, LRESULT* result)
{
	NMLVCUSTOMDRAW* NM_custom_draw= reinterpret_cast<NMLVCUSTOMDRAW*>(nm_hdr);
	*result = CDRF_DODEFAULT;

	switch (NM_custom_draw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*result = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		*result = CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		*result |= CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_SUBITEM | CDDS_ITEMPOSTPAINT:
		{
			if (NM_custom_draw->iSubItem != 2)
				break;

			// subitem 'Root': draw checkbox

			int item= NM_custom_draw->nmcd.dwItemSpec;
			CRect rect(0,0,0,0);
			if (list_wnd_.GetSubItemRect(item, NM_custom_draw->iSubItem, LVIR_LABEL, rect))
			{
				CDC* dc= CDC::FromHandle(NM_custom_draw->nmcd.hdc);
				int save= dc->SaveDC();
//				UINT state= list_wnd_.GetItemState(item, -1);

				// draw check box
				rect.DeflateRect(2, 2);
				const FavoriteFolder* folder= GetFolder(NM_custom_draw->nmcd.iteml_param);
				dc->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT |
					(folder->OpenAsRoot() ? DFCS_CHECKED : 0));

				dc->RestoreDC(save);
				*result = CDRF_SKIPDEFAULT;
			}
		}
		break;
	}
}

void SetupFavoritesDlg::OnClickList(NMHDR* nm_hdr, LRESULT* result)
{
	*result = 0;

	int index= GetSelectedItemIndex();
	if (index < 0)
		return;

	CRect rect(0,0,0,0);
	if (!list_wnd_.GetSubItemRect(index, 2, LVIR_LABEL, rect))
		return;

	CPoint pos;
	::GetCursorPos(&pos);
	list_wnd_.ScreenToClient(&pos);

	if (rect.PtInRect(pos))
	{
		FavoriteFolder* folder= GetFolder(list_wnd_.GetItemData(index));
		if (folder == 0)
			return;
		folder->SetOpenAsRoot(!folder->OpenAsRoot());
		list_wnd_.RedrawItems(index, index);
	}
}
#endif


void SetupFavoritesDlg::OnEndLabelEdit(NMHDR* nmhdr, LRESULT* result)
{
	NMLVDISPINFO* info= reinterpret_cast<NMLVDISPINFO*>(nmhdr);
	*result = 0;

	if (info->item.pszText == 0 || *info->item.pszText == 0)
		return;

	int index= info->item.iItem;
	if (index >= 0 && index < folders_.GetCount())
	{
		if (FavoriteFolder* folder= folders_.GetFolder(index))
		{
			folder->SetDisplayName(info->item.pszText);
			*result = 1;
		}
	}
}


void SetupFavoritesDlg::OnBeginLabelEdit(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;

	if (CEdit* edit= list_wnd_.GetEditControl())
		edit->LimitText(NAME_MAX_LENGTH);
}


void SetupFavoritesDlg::OnDestroy()
{
	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
		wnd_pos_.StoreState(wp);

	CDialog::OnDestroy();
}


void SetupFavoritesDlg::OnGetMinMaxInfo(MINMAXINFO* MMI)
{
	CDialog::OnGetMinMaxInfo(MMI);

	MMI->ptMinTrackSize.x = min_size_.cx;
	MMI->ptMinTrackSize.y = min_size_.cy;
}


void SetupFavoritesDlg::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);
	dlg_resize_map_.Resize();
	ResizeColumnsToFit(list_wnd_);
}


// column resizing finished
//
void SetupFavoritesDlg::OnEndTrack(UINT, NMHDR* /*nmhdr*/, LRESULT* result)
{
//	tree_wnd_.Invalidate();
//	*result = ResizeColumnsToFit(list_wnd_) ? 1 : 0;
}
