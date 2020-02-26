/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DialogHostCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "DialogHostCtrl.h"


static const TCHAR* const WND_CLASS= _T("DialogHostCtrlMiK");

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = 0;//CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}

// DialogHostCtrl dialog

DialogHostCtrl::DialogHostCtrl(UINT dialog_id) : dialog_id_(dialog_id)
{
	RegisterWndClass(WND_CLASS);
}

DialogHostCtrl::~DialogHostCtrl()
{
}


BEGIN_MESSAGE_MAP(DialogHostCtrl, CWnd)
//	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// DialogHostCtrl message handlers

//int DialogHostCtrl::OnCreate(CREATESTRUCT* createStruct)
//{
//	CWnd::OnCreate(createStruct);
//
//	ModifyStyleEx(0, WS_EX_CONTROLPARENT, 0);
//
//	if (!dialog_.Create(dialog_id_, this))
//		return -1;
//
//	CRect rect(0,0,0,0);
//	GetClientRect(rect);
//	dialog_.MoveWindow(rect);
//
//	return 0;
//}


void DialogHostCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyleEx(0, WS_EX_CONTROLPARENT, 0);

	if (dialog_.Create(dialog_id_, this))
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		dialog_.MoveWindow(rect);

//		if (IsWindowVisible())
			dialog_.ShowWindow(SW_SHOWNA);
	}
}


void DialogHostCtrl::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED && dialog_.m_hWnd)
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		dialog_.MoveWindow(rect);
	}
}
