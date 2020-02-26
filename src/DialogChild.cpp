/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DialogChild.cpp : implementation file
//

#include "stdafx.h"
#include "DialogChild.h"
#include "resource.h"
#include "IsDialogResizable.h"
#include "WhistlerLook.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DialogChild dialog


DialogChild::DialogChild(UINT id_template, CWnd* parent /*= NULL*/)
	: CDialog(id_template, parent), parent_(0)
{
	//{{AFX_DATA_INIT(DialogChild)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	resizable_dialog_ = ::IsDialogResizable(this);
}



BEGIN_MESSAGE_MAP(DialogChild, CDialog)
	//{{AFX_MSG_MAP(DialogChild)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_HELP_BTN, &DialogChild::OnHelpBtn)
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()


bool DialogChild::IsResizable() const
{
	return resizable_dialog_;
}

/////////////////////////////////////////////////////////////////////////////
// DialogChild message handlers

void DialogChild::OnOK()
{
	if (!UpdateData())
		return;

	EndDialog(IDOK);
}

void DialogChild::OnCancel()
{
	EndDialog(IDCANCEL);
}


void DialogChild::EndDialog(int result)
{
	if (CDialog* parent= dynamic_cast<CDialog*>(GetParent()))
		parent->EndDialog(result);
}


void DialogChild::ShowImage(Dib* image)
{
	if (parent_)
		parent_->SetExtraImage(image);
}


void DialogChild::ShowImage(PhotoInfoPtr photo)
{
	if (parent_)
		parent_->SetExtraImage(photo);
}


void DialogChild::SetBigTitle(const TCHAR* title)
{
	if (parent_)
		parent_->SetBigTitle(title);
}


void DialogChild::Resize()
{
	dlg_resize_map_.Resize();
}


void DialogChild::Resize(const CRect& rect)
{
	dlg_resize_map_.Resize(rect);
}


void DialogChild::SetFooterDlg(CDialog* dlg)
{
	if (parent_)
		parent_->SetFooterDlg(dlg);
}


void DialogChild::ShowFooterDlg(bool show)
{
	if (parent_)
		parent_->ShowFooterDlg(show);
}


void DialogChild::SetRightSide(int width, COLORREF color, const std::vector<float>& shades)
{
	if (parent_)
		parent_->SetRightSide(width, color, shades);
}

void DialogChild::SetRightSide(int width)
{
	if (parent_)
		parent_->SetRightSide(width);
}


void DialogChild::SubclassHelpBtn(const TCHAR* help_page)
{
	if (btn_help_.m_hWnd == 0)
		VERIFY(btn_help_.SubclassDlgItem(IDC_HELP_BTN, this));
	btn_help_.SetWindowText(_T(""));
	help_page_ = help_page;
}


void DialogChild::OnHelpBtn()
{
	extern void OpenHelp(const TCHAR* initial_page);

	if (!help_page_.IsEmpty())
		OpenHelp(help_page_);
}


// HACK: this is the means of tapping into the modal loop to handle F1 key down message
//
BOOL DialogChild::ContinueModal()
{
	if (!CDialog::ContinueModal())
		return false;

	if (MSG *msg = AfxGetCurrentMessage())
	{
		if (!help_page_.IsEmpty() && msg->message == WM_KEYDOWN && msg->wParam == VK_F1)
			OnHelpBtn();
	}

	return true;
}


CString DialogChild::GetDialogTitle()
{
	CString title;
	GetWindowText(title);
	return title;
}


void DialogChild::CreateGripWnd()
{
	// create grip ctrl
	CRect rect;
	GetClientRect(rect);

	CSize bar_size(::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CYHSCROLL));
	rect.left = rect.right - bar_size.cx;
	rect.top = rect.bottom - bar_size.cy;
	VERIFY(grip_wnd_.Create(SBS_SIZEGRIP | WS_CHILD | WS_VISIBLE, rect, this, IDC_GRIP));

	grip_wnd_.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);
}


BOOL DialogChild::OnInitDialog()
{
	return CDialog::OnInitDialog();
}


void DialogChild::SetWndResizing(int id, DlgAutoResize::ResizeFlag flags)
{
	dlg_resize_map_.SetWndResizing(id, flags);
}

void DialogChild::SetWndResizing(int id, DlgAutoResize::ResizeFlag flags, UINT half_flags)
{
	dlg_resize_map_.SetWndResizing(id, flags, half_flags);
}

void DialogChild::SetControlsShift(CSize shift)
{
	dlg_resize_map_.SetOffset(shift);
	if (parent_)
		parent_->Resize();
}

void DialogChild::BuildResizingMap()
{
	bool has_grip= resizable_dialog_ && WhistlerLook::IsAvailable();

	if (has_grip)
		CreateGripWnd();	// create grip ctrl

	dlg_resize_map_.BuildMap(this);

	if (has_grip)
		dlg_resize_map_.SetWndResizing(IDC_GRIP, DlgAutoResize::MOVE);
}


void DialogChild::OnWindowPosChanged(WINDOWPOS* wp)
{
	CDialog::OnWindowPosChanged(wp);

	if (grip_wnd_.m_hWnd)
		if (CWnd* wnd= GetParent())
			grip_wnd_.EnableWindow(wnd->GetStyle() & WS_MAXIMIZE ? false : true);
}


void DialogChild::SetMinimalDlgSize(CSize minimal)
{
	if (parent_)
		parent_->SetMinimalDlgSize(minimal);
}


void DialogChild::SetResizeCallback(HeaderDlg* parent)
{
	parent_ = parent;
}
