#include "stdafx.h"
#include "DialogBase.h"
#include "AppColors.h"

DialogBase::DialogBase()
{}

DialogBase::DialogBase(LPCTSTR lpszTemplateName, CWnd* pParentWnd) : CDialog(lpszTemplateName, pParentWnd)
{}

DialogBase::DialogBase(UINT nIDTemplate, CWnd* pParentWnd) : CDialog(nIDTemplate, pParentWnd)
{}


BEGIN_MESSAGE_MAP(DialogBase, CDialog)
	//ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//ON_COMMAND_RANGE(ADD_FILTER, UPDATE_FILTER, OnFilterCommand)
	//ON_EN_CHANGE(IDC_FILTER_NAME, OnFilterNameChanged)
END_MESSAGE_MAP()

BOOL DialogBase::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0, 0, 0, 0);
	GetClientRect(rect);
	if (!rect.IsRectEmpty())
	{
		//COLORREF background = GetAppColors()[AppColors::Background];
		dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
	}

	return true;
}


HBRUSH DialogBase::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr = CDialog::OnCtlColor(dc, wnd, ctl_color);

	COLORREF background = ::GetSysColor(COLOR_3DFACE);//GetAppColors()[AppColors::Background];
	COLORREF text = ::GetSysColor(COLOR_BTNTEXT);

	//	if (wnd && wnd->GetDlgCtrlID() == IDC_EXAMPLE)
	{
		background_brush_.DeleteObject();
		background_brush_.CreateSolidBrush(background);
		hbr = background_brush_;
		dc->SetBkColor(background);
		dc->SetTextColor(text);
		//		dc->SelectObject(&fnd_desc_);
	}

	return hbr;
}
