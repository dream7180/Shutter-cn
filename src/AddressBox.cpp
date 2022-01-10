/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// AddressBox.cpp : implementation file
//

#include "stdafx.h"
#include "AddressBox.h"
#include "ItemIdList.h"
#include "resource.h"
#include "AutoComplete.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AddressBox

AddressBox::AddressBox()
{
	scan_in_progress_ = false;
}

AddressBox::~AddressBox()
{}


bool AddressBox::Create(CWnd* parent)
{
	SHFILEINFO info;
	HIMAGELIST image_list= reinterpret_cast<HIMAGELIST>(::SHGetFileInfo(_T("\0\0\0\0"),
		NULL, &info, sizeof info, SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX));

	SetImageList(image_list);

	if (!EditCombo::Create(parent, IDB_ADDRESS_TOOLBAR, ID_REFRESH, ID_STOP_SCANNING, EditCombo::AUTO_COMPLETE))
		return false;

	SetMargins(CRect(0, 0, Pixels(10), 0));

	GetAutoComplete().HandleUpDownKey(false);
	GetAutoComplete().RegisterTextSelectedFn(boost::bind(&AddressBox::Selected, this));

	ConnectFinishCommand(boost::bind(&AddressBox::EndEdit, this, _1));

	// add SHAutoComplete fn call if it's available
	AutoComplete::TurnOn(GetEditCtrl().m_hWnd);

	return true;
}


void AddressBox::Selected()
{
	Run(ID_PATH_TYPED);
}


// set path to display
//
void AddressBox::SetPath(const ItemIdList& idlPath)
{
	CEdit& edit= GetEditCtrl();
	CString path= idlPath.GetPath();
	edit.SetWindowText(path.IsEmpty() ? idlPath.GetName() : path);
	SetImageIndex(idlPath.GetIconIndex());
}


void AddressBox::SetPath(const TCHAR* path)
{
	CEdit& edit= GetEditCtrl();
	edit.SetWindowText(path != 0 ? path : _T(""));
}


void AddressBox::SetPath(FolderPathPtr path)
{
	if (path)
	{
		ItemIdList pidl= path->GetPIDL();
		SetPath(pidl);
		SetImageIndex(pidl.GetIconIndex());
	}
	else
	{
		SetPath(L"");
		SetImageIndex(-1);
	}
}


// on 'Enter' or 'Esc' send notification to browser frame
//
//void AddressBox::OnEndEdit(NMHDR* nmhdr, LRESULT* result)
//{
//	NMCBEENDEDIT* end_edit= reinterpret_cast<NMCBEENDEDIT*>(nmhdr);
//
//	if (end_edit->iWhy == CBENF_RETURN || end_edit->iWhy == CBENF_ESCAPE)
//		if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(frame_))
//		{
//			if (end_edit->iWhy == CBENF_RETURN)
//			{
//				if (end_edit->szText[0])	// is something typed in?
//					frame->PostMessage(WM_COMMAND, ID_PATH_TYPED);
//			}
//			else
//				frame->PostMessage(WM_COMMAND, ID_PATH_CANCELLED);
//		}
//
//	*result = 0;
//}


void AddressBox::SetHistory(const CRecentFileList& paths)
{
	std::vector<String> history;
	const int count= paths.GetSize();

	history.reserve(count);
	String path_i;
	for (int i= 0; i < count; ++i){
		path_i = String(const_cast<CRecentFileList&>(paths)[i]);
		if(path_i.length()!=0)
			history.push_back(path_i);
	}

	EditCombo::SetHistory(history);
}


void AddressBox::SetScan(bool scan_in_progress)
{
	scan_in_progress_ = scan_in_progress;
	SetState(scan_in_progress ? 1 : 0);
}


void AddressBox::EndEdit(bool ok)
{
	if (ok)
	{
		Run(ID_PATH_TYPED);
	}
	else
	{
		if (scan_in_progress_)
			Run(ID_STOP_SCANNING);
		// restore current path
		Run(ID_PATH_CANCELLED);
	}
}
