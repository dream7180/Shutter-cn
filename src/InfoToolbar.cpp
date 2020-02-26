/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// InfoToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "ExifPro.h"
#include "InfoToolbar.h"
#include "RString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoToolbar

CInfoToolbar::CInfoToolbar()
{
}

CInfoToolbar::~CInfoToolbar()
{
}


BEGIN_MESSAGE_MAP(CInfoToolbar, CHorzReBar)
	//{{AFX_MSG_MAP(CInfoToolbar)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoToolbar message handlers


bool CInfoToolbar::Create(CWnd* parent, UINT id)
{
	if (!CHorzReBar::Create(parent, false, RBS_FIXEDORDER, AFX_IDW_REBAR))
		return false;

	static const int anCommands[]=
	{
		ID_COPY_INFO, ID_RAW_INFO, ID_TASK_EXPORT, ID_TOGGLE_PREVIEW
	};

	tool_bar_wnd_.SetPadding(8, 10);

	if (!tool_bar_wnd_.Create("PXPx", anCommands, IDB_INFO_TOOLBAR, IDS_INFO_TOOLBAR, parent, id))
		return false;

	AddFixedBand(&tool_bar_wnd_, 1);

	return true;
}
