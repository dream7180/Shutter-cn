/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// EditFld.cpp: implementation of the CEditFld class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EditFld.h"
#include "PropertyDlg.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

String CEditFld::class_;
HFONT CEditFld::font_= 0;


CEditFld::CEditFld(CPropField* field) : edit_styles_(0), field_(field)
{}

CEditFld::~CEditFld()
{}


BEGIN_MESSAGE_MAP(CEditFld, CWnd)
	//{{AFX_MSG_MAP(CEditFld)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_EN_KILLFOCUS(100, OnEditKillFocus)
END_MESSAGE_MAP()



bool CEditFld::Create(DWORD styles, const CRect& rect, CWnd* parent, int id)
{
	if (class_.empty())
	{
		class_ = AfxRegisterWndClass(CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW)); //, HBRUSH(COLOR_WINDOW + 1));
	}

	edit_styles_ = styles;

	return !!CWnd::Create(class_.c_str(), 0, WS_CHILD | WS_VISIBLE /*| WS_TABSTOP*/, rect, parent, id);
}


LRESULT CEditFld::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	font_ = HFONT(wParam);
	return 0;
}


void CEditFld::OnPaint()
{
	CPaintDC dc(this); // device context for painting
//dc.FillSolidRect(0, 0, 100, 20, RGB(120,0,0));
	COLORREF rgb_back= GetSysColor(COLOR_WINDOW); //field_->IsReadOnly() ? COLOR_3DFACE : COLOR_WINDOW);

	CMemoryDC memDC(dc, this, rgb_back);

	if (font_)
		::SelectObject(memDC, font_);

	memDC.SetBkColor(rgb_back);

	CRect rect;
	GetClientRect(rect);
	rect.left += 3;
	String str= field_->GetVal();
	UINT format= DT_LEFT | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX;
	if (field_->IsMultiLine())
		format &= ~DT_SINGLELINE;
	memDC.DrawText(str.c_str(), str.length(), rect, format);

	memDC.BitBlt();
}


void CEditFld::EnterEdit()
{
	if (edit_.get() == 0 && !field_->IsReadOnly())
	{
		edit_ = new CEdit;
		CRect rect;
		GetClientRect(rect);
		rect.left += 3;
		edit_->Create(edit_styles_, rect, this, 100);
		edit_->SendMessage(WM_SETFONT, WPARAM(font_), 0);
		original_val_ = field_->GetVal();

		if (field_->IsMultiLine())
		{
			CString val= original_val_.c_str();
			val.Replace(_T("\x0d"), _T("\x0d\0x0a"));
			edit_->SetWindowText(val);
		}
		else
			edit_->SetWindowText(original_val_.c_str());

		edit_->SetLimitText(field_->TextLimit());
		edit_->SetSel(0x7fff, 0x7fff);
		edit_->SetFocus();
	}
}


void CEditFld::OnLButtonDown(UINT flags, CPoint point)
{
	EnterEdit();
}


void CEditFld::OnEditKillFocus()
{
	if (edit_.get() != 0)
	{
		CString val;
		edit_->GetWindowText(val);

		if (field_->IsMultiLine())
			val.Replace(_T("\x0d\0x0a"), _T("\x0d"));

		if (val != original_val_.c_str())
		{
			if (!field_->SetVal(String(val)))
			{
				//
			}
		}

		edit_->DestroyWindow();
		edit_.free();
	}
}


BOOL CEditFld::OnEraseBkgnd(CDC* dc)
{
//	CRect rect;
//	GetClientRect(rect);
//	dc->FillSolidRect(rect, GetSysColor(field_->IsReadOnly() ? COLOR_3DFACE : COLOR_WINDOW));
	return true;
//	return CWnd::OnEraseBkgnd(dc);
}


BOOL CEditFld::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT && !field_->IsReadOnly())
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_IBEAM));
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}
