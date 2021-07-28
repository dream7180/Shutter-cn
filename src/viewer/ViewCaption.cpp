/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "../resource.h"
#include "ViewCaption.h"
#include "../DrawFields.h"
#include "../MemoryDC.h"
#include "../GetDefaultGuiFont.h"

extern bool LoadPingFromRsrc(LPCTSTR resource_id, Dib& bmp);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(ViewCaption, CWnd)
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


Dib ViewCaption::active_marker_;
//Dib ViewCaption::caption_;


ViewCaption::ViewCaption()
{
	active_ = false;
	text_color_ = RGB(220,220,220);
	label_color_ = RGB(139,139,147);
}


bool ViewCaption::Create(CWnd* parent, int toolbarBmp, const int commands[], int count,
						 const boost::function<void (void)>& on_clicked)
{
	on_clicked_ = on_clicked;

	CDC dc;
	dc.CreateCompatibleDC(0);
	LOGFONT lf;
	/*HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	//lf.lfQuality = ANTIALIASED_QUALITY;
	//lf.lfHeight += 1;
	lf.lfWeight =  FW_BOLD;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));*/
	::GetDefaultGuiBoldFont(lf);
	_font.CreateFontIndirect(&lf);
	dc.SelectObject(&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int h= tm.tmHeight + tm.tmInternalLeading + 2;	// yes, internal leading is already in tmHeight

	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CRect(0,0,0,h), parent, -1))
		return false;

	//if (!caption_.IsValid())
	//	VERIFY(LoadPingFromRsrc(MAKEINTRESOURCE(IDB_VIEW_CAPTION), caption_));
	if (!active_marker_.IsValid())
		VERIFY(LoadPingFromRsrc(MAKEINTRESOURCE(IDB_ACTIVE_VIEW), active_marker_));

	toolbar_.SetOnIdleUpdateState(false);

	FancyToolBar::Params p;
	p.shade = -0.28f;
	std::string btn(count, 'p');
	if (!toolbar_.Create(this, btn.c_str(), commands, toolbarBmp, &p))
		return false;

	toolbar_.SetPadding(CRect(5,4,5,4));
	toolbar_.SetOption(FancyToolBar::HOT_OVERLAY, false);
	toolbar_.SetOwner(parent);

	return true;
}


BOOL ViewCaption::OnEraseBkgnd(CDC* dc)
{
	//if (caption_.IsValid())
	//{
		CRect rect(0,0,0,0);
		GetClientRect(rect);

		MemoryDC mdc(*dc, rect);

		//caption_.Draw(&dc, rect);
		mdc.FillSolidRect(rect, RGB(35,35,35));
		CPoint pos(4, 0);
		pos.y = (rect.Height() - active_marker_.GetHeight()) / 2;
		rect.left = pos.x + active_marker_.GetWidth();
		if (active_)
		{
			//CPoint pos(3, 0);
			//pos.y = (rect.Height() - active_marker_.GetHeight()) / 2;
			active_marker_.Draw(&mdc, pos);
			//rect.left = pos.x + active_marker_.GetWidth();
		}
		//dc.SetBkMode(TRANSPARENT);
		mdc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
		mdc.SelectObject(&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);

		CString text;
		GetWindowText(text);
		// trim spaces
		text.TrimRight(_T(' '));
		text.Insert(0, _T("  "));

		CRect text_rect= rect;
		text_rect.right -= toolbar_.Size().cx;
		if (text_rect.Width() > 0)
			DrawFields::Draw(mdc, text, text_rect, text_color_, label_color_, text_color_);

		mdc.BitBlt();
	//}
	return true;
}


LRESULT ViewCaption::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		OnEraseBkgnd(dc);

	return 0;
}


void ViewCaption::OnSize(UINT type, int cx, int cy)
{
	if (toolbar_.m_hWnd)
	{
		CSize s= toolbar_.Size();
		int x= cx - s.cx;
		int y= (cy - s.cy - 1) / 2;
		toolbar_.SetWindowPos(0, x, y, s.cx, s.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


void ViewCaption::SetActive(bool active)
{
	active_ = active;
	Invalidate();
}


void ViewCaption::OnLButtonDown(UINT flags, CPoint point)
{
	if (on_clicked_)
		on_clicked_();
}
