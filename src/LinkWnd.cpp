/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// LinkWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LinkWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLinkWnd

CLinkWnd::CLinkWnd()
{
	rgb_text_color_ = ::GetSysColor(COLOR_BTNTEXT);
//	rgb_hot_text_color_ = RGB();

	const TCHAR* cls_name= _T("MKLinkWnd");
	HINSTANCE instance= AfxGetResourceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, cls_name, &wndcls))
	{
		// otherwise we need to register a new class
		HCURSOR cursor= AfxGetApp()->LoadStandardCursor(IDC_HAND);
		if (cursor == 0)
			cursor = AfxGetApp()->LoadCursor(IDC_LINK);

		wndcls.style = 0;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = cursor;
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = cls_name;

		AfxRegisterClass(&wndcls);
	}
}

CLinkWnd::~CLinkWnd()
{
}


BEGIN_MESSAGE_MAP(CLinkWnd, CWnd)
	//{{AFX_MSG_MAP(CLinkWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLinkWnd message handlers

bool CLinkWnd::Create(CWnd* parent, CPoint top_left, const TCHAR* display, const TCHAR* URL,
		CFont* font, UINT id/*= ~0u*/)
{
	ASSERT(URL && display);

	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);

	if (underlined_fnt_.m_hObject == 0)
	{
		LOGFONT lf;

		if (font)
			font->GetLogFont(&lf);
		else
		{
			HFONT font= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
			::GetObject(font, sizeof(lf), &lf);
			//lf.lfQuality = ANTIALIASED_QUALITY;
			lf.lfHeight += 1;
			_tcscpy(lf.lfFaceName, _T("Tahoma"));
		}
		lf.lfUnderline = true;
		underlined_fnt_.CreateFontIndirect(&lf);
	}
	dc.SelectObject(&underlined_fnt_);

	CSize size= dc.GetTextExtent(display, _tcslen(display));
	size.cy += 3;

	HCURSOR cursor= AfxGetApp()->LoadStandardCursor(IDC_HAND);
	if (cursor == 0)
		cursor = AfxGetApp()->LoadCursor(IDC_LINK);

	if (!CWnd::Create(AfxRegisterWndClass(0, cursor), display,
		WS_CHILD | WS_VISIBLE, CRect(top_left, size), parent, id))
		return false;

	URL_ = URL;

	return true;
}


void CLinkWnd::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	if (underlined_fnt_.m_hObject == 0)
	{
		LOGFONT lf;

		if (CFont* font= GetFont())
			font->GetLogFont(&lf);
		else
		{
			HFONT hfont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
			::GetObject(hfont, sizeof(lf), &lf);
			lf.lfHeight += 1;
			//lf.lfQuality = ANTIALIASED_QUALITY;
			_tcscpy(lf.lfFaceName, _T("Tahoma"));
		}
		lf.lfUnderline = true;
		underlined_fnt_.CreateFontIndirect(&lf);
	}
}


BOOL CLinkWnd::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	if (CWnd* parent= GetParent())
	{
		CDC background_dc;
		background_dc.CreateCompatibleDC(dc);

		CBitmap background_bmp;
		background_bmp.CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
		background_dc.SelectObject(&background_bmp);

		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);

		// prepare copy of background
		background_dc.SetWindowOrg(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
		parent->Print(&background_dc, PRF_ERASEBKGND | PRF_CLIENT);
		background_dc.SetWindowOrg(0, 0);

		background_dc.SetBkMode(TRANSPARENT);
		background_dc.SetTextColor(rgb_text_color_);
		background_dc.SelectObject(&underlined_fnt_);

		CString text;
		GetWindowText(text);

		background_dc.DrawText(text, rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

		dc->BitBlt(0, 0, rect.Width(), rect.Height(), &background_dc, 0, 0, SRCCOPY);

		background_dc.DeleteDC();
	}
	
	return true;
}


void CLinkWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}


void CLinkWnd::OnLButtonDown(UINT flags, CPoint point)
{
	if (URL_.IsEmpty())
	{
		if (CWnd* wnd= GetParent())
			wnd->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
	else
		::ShellExecute(0, _T("open"), URL_, 0, 0, SW_SHOWNORMAL);
}
