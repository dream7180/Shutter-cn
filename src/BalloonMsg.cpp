/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// BalloonMsg.cpp : implementation file
//

#include "stdafx.h"
#include "BalloonMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BalloonMsg

#ifndef TTS_BALLOON
	#define TTS_BALLOON             0x40
#endif
#ifndef TTM_SETTITLE
	#define TTM_SETTITLEA           (WM_USER + 32)  // wParam = TTI_*, lParam = char* title
	#define TTM_SETTITLEW           (WM_USER + 33)  // wParam = TTI_*, lParam = wchar* title
	#ifdef UNICODE
		#define TTM_SETTITLE            TTM_SETTITLEW
	#else
		#define TTM_SETTITLE            TTM_SETTITLEA
	#endif
#endif
#ifndef TTI_NONE
// ToolTip Icons (Set with TTM_SETTITLE)
	#define TTI_NONE                0
	#define TTI_INFO                1
	#define TTI_WARNING             2
	#define TTI_ERROR               3
#endif


BEGIN_MESSAGE_MAP(BalloonMsg, CToolTipCtrl)
	//{{AFX_MSG_MAP(BalloonMsg)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER, OnWmUser)
END_MESSAGE_MAP()

static const int TIMER_ID= 1;


// Displays balloon tool tip message pointing to the window 'wnd'
//

BalloonMsg::BalloonMsg(CWnd* wnd, const TCHAR* title, const TCHAR* msg, MsgIcon icon, CPoint pos)
{
	if (wnd == 0)
	{
		ASSERT(false);
		delete this;
		return;
	}
	ASSERT(msg && title);
	if (!wnd->IsWindowVisible())	// it only makes sense to display balloon for visible windows
	{
		MessageBox(msg, title);
		delete this;
		return;
	}

	closing_ = false;

	Create(0, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON | TTS_ALWAYSTIP);

	if (m_hWnd == 0)
		return;

	msg_ = msg;
	title_ = title;

	const TCHAR* text= msg_;

	struct OLDTOOLINFO
	{
		UINT cbSize;
		UINT uFlags;
		HWND hwnd;
		UINT uId;
		RECT rect;
		HINSTANCE hinst;
		LPTSTR lpszText;
	} ti;
	// new TOOLINFO with extra reserved field changes the way balloons work rendering them useless...
//	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.uFlags = TTF_TRACK | TTF_IDISHWND;
	ti.hwnd = wnd->m_hWnd;
	ti.uId = 1;
	ti.hinst = NULL;
	ti.lpszText = const_cast<TCHAR*>(text); //LPSTR_TEXTCALLBACK;
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 0;
	ti.rect.bottom = 0;
//	ti.lParam = 0;

	SendMessage(TTM_ADDTOOL, 0, LPARAM(&ti));

	SetMaxTipWidth(300);

//	SetTitle(icon, title_);
	const TCHAR* ttl= title_;
	SendMessage(TTM_SETTITLE, icon, LPARAM(ttl));

	if (pos.x == 0 && pos.y == 0)
	{
		CRect rect;
		wnd->GetWindowRect(rect);
		pos = rect.CenterPoint();
	}

	SendMessage(TTM_TRACKPOSITION, 0, MAKELONG(pos.x, pos.y));

	SendMessage(TTM_TRACKACTIVATE, true, LPARAM(&ti));

	::MessageBeep(MB_OK);

	SetTimer(TIMER_ID, 10000, 0);

	wnd->SetFocus();

	SubclassWnd(*wnd);

	InstallMouseHook();
}


BalloonMsg::~BalloonMsg()
{}


/////////////////////////////////////////////////////////////////////////////
// BalloonMsg message handlers


void BalloonMsg::OnTimer(UINT_PTR id_event)
{
	if (id_event == TIMER_ID)
		Close();
	else
		CToolTipCtrl::OnTimer(id_event);
}


void BalloonMsg::PostNcDestroy()
{}


///////////////////////////////////////////////////////////////////////////////

namespace {

BalloonMsg* g_balloon= 0;		// there is only one active balloon at a time
HWND g_sentry= 0;			// window below balloon
WNDPROC g_pfnOldWindowProc= 0;	// and its wnd proc
HWND g_parent= 0;			// parent window
WNDPROC g_pfnOldWindowProc2= 0;	// and its wnd proc
RECT g_child_window_rect_rect;		// child window rect

static LRESULT CALLBACK SentryWindowProc(
  HWND wnd,      // handle to window
  UINT msg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	WNDPROC pfn= g_pfnOldWindowProc;
//TRACE(L"sentry msg: %x %x %x \n", msg, wParam, lParam);
	switch (msg)
	{
	case WM_MOVE:
	case WM_DESTROY:
	case WM_SIZING:
	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KILLFOCUS:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (g_balloon)
			g_balloon->Close();
		break;

	case WM_ACTIVATE:
		if (HIWORD(wParam) == WA_INACTIVE && g_balloon)
			g_balloon->Close();
		break;

	case WM_PAINT:
		{
			CRect rect;
			::GetWindowRect(wnd, rect);
			if (rect != g_child_window_rect_rect && g_balloon)
				g_balloon->Close();
		}
		break;
	}

	return pfn ? ::CallWindowProc(pfn, wnd, msg, wParam, lParam) : 0;
}

static LRESULT CALLBACK ParentWindowProc(
  HWND wnd,      // handle to window
  UINT msg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	WNDPROC pfn= g_pfnOldWindowProc2;
//TRACE(L"parent msg: %x %x %x \n", msg, wParam, lParam);
	switch (msg)
	{
	case WM_ENTERSIZEMOVE:
	case WM_MOVE:
	case WM_DESTROY:
	case WM_SIZING:
	case WM_CHAR:
	case WM_KILLFOCUS:
		if (g_balloon)
			g_balloon->Close();
		break;
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE && g_balloon)
			g_balloon->Close();
		break;
	}

	return pfn ? ::CallWindowProc(pfn, wnd, msg, wParam, lParam) : 0;
}

} // namespace


void BalloonMsg::Unsubclass()
{
	if (g_sentry)
	{
		if (g_pfnOldWindowProc)
			::SetWindowLongPtr(g_sentry, GWLP_WNDPROC, LONG_PTR(g_pfnOldWindowProc));
		if (g_pfnOldWindowProc2)
			::SetWindowLongPtr(g_parent, GWLP_WNDPROC, LONG_PTR(g_pfnOldWindowProc2));

		g_pfnOldWindowProc = 0;
		g_sentry = 0;
		g_pfnOldWindowProc2 = 0;
		g_parent = 0;
		g_balloon = 0;
	}
}


void BalloonMsg::SubclassWnd(HWND wnd)
{
	if (g_balloon)
		g_balloon->Close();

	Unsubclass();

	::GetWindowRect(wnd, &g_child_window_rect_rect);

	g_sentry = wnd;
	g_balloon = this;
	g_pfnOldWindowProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(wnd, GWLP_WNDPROC, LONG_PTR(SentryWindowProc)));
	g_parent = ::GetParent(wnd);
	if (g_parent)
		g_pfnOldWindowProc2 = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(g_parent, GWLP_WNDPROC, LONG_PTR(ParentWindowProc)));
}


void BalloonMsg::Close()
{
	if (closing_)
		return;

	closing_ = true;

//	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	UninstallMouseHook();

	ShowWindow(SW_HIDE);

	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.uFlags = TTF_TRACK | TTF_IDISHWND;
	ti.hwnd = g_sentry;
	ti.uId = 1;
	ti.hinst = NULL;
	ti.lpszText = 0;
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 0;
	ti.rect.bottom = 0;
	ti.lParam = 0;
	SendMessage(TTM_TRACKACTIVATE, false, LPARAM(&ti));

	HWND wnd= g_sentry;

	Unsubclass();
	KillTimer(1);

	DestroyWindow();

	if (wnd && ::IsWindow(wnd))
	{
		// this multiple invalidate rect takes care of proper
		// redrawing of MSFlexGrid control (at least usually);
		// when the scrollbar is on even that doesn't help
		::InvalidateRect(wnd, 0, true);
		::UpdateWindow(wnd);
		::InvalidateRect(wnd, 0, true);

		::RedrawWindow(wnd, 0, 0, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	}

	delete this;
}


LRESULT BalloonMsg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
//TRACE(L"Ballon msg: %x\n", message);

	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (g_balloon)
		{
			g_balloon->Close();
			return 0;
		}
	}
	
	return CToolTipCtrl::WindowProc(message, wParam, lParam);
}


namespace {

HHOOK g_next_proc= 0;

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (g_next_proc == 0)
		return 0;

	if (code < 0)  // do not process the message
		return CallNextHookEx(g_next_proc, code, wParam, lParam);

	if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN)
		if (g_balloon != 0)
			g_balloon->PostMessage(WM_USER);

	return CallNextHookEx(g_next_proc, code, wParam, lParam);
}

} // namespace


void BalloonMsg::InstallMouseHook()
{
	ASSERT(g_next_proc == 0);
	g_next_proc = ::SetWindowsHookEx(WH_MOUSE, MouseProc, 0, ::GetCurrentThreadId());
}


void BalloonMsg::UninstallMouseHook()
{
	if (g_next_proc)
	{
		::UnhookWindowsHookEx(g_next_proc);
		g_next_proc = 0;
	}
}


LRESULT BalloonMsg::OnWmUser(WPARAM, LPARAM)
{
	if (g_balloon)
		g_balloon->Close();
	return 0;
}
