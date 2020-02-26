/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// LineSeparatorCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LineSeparatorCtrl.h"
#include "MemoryDC.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const TCHAR* const WND_CLASS= _T("LineSeparatorCtrlMiK");

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = AfxGetApp()->LoadCursor(IDC_VERT_RESIZE);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}


LineSeparatorCtrl::LineSeparatorCtrl()
{
	RegisterWndClass(WND_CLASS);
}


LineSeparatorCtrl::~LineSeparatorCtrl()
{}


BEGIN_MESSAGE_MAP(LineSeparatorCtrl, CWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// LineSeparatorCtrl message handlers


bool LineSeparatorCtrl::Create(CWnd* parent)
{
//	const TCHAR* class_name= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW,
//		AfxGetApp()->LoadCursor(IDC_VERT_RESIZE));

	if (!CWnd::Create(WND_CLASS, _T(""), WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	return true;
}


BOOL LineSeparatorCtrl::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	DrawLineSeparator(*dc, rect);
	return true;
}
