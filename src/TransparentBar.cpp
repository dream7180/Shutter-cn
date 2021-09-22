/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransparentBar.cpp : implementation file
//

#include "stdafx.h"
#include "ExifPro.h"
#include "TransparentBar.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTransparentBar

CTransparentBar::CTransparentBar()
{
}

CTransparentBar::~CTransparentBar()
{
}


BEGIN_MESSAGE_MAP(CTransparentBar, CToolBarCtrl)
	//{{AFX_MSG_MAP(CTransparentBar)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransparentBar message handlers


bool CTransparentBar::Create(CWnd* parent, UINT id)
{
	int height= GetApp()->IsWhistlerLookAvailable() ? 25 : 23;

	if (!CToolBarCtrl::Create(WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | //TBSTYLE_TRANSPARENT |
		CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER, CRect(0,0,80,height), parent, id))
	{
		ASSERT(false);
		return false;
	}
	
	LOGFONT lf;
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);
	SendMessage(WM_SETFONT, WPARAM(hfont));
	//SendMessage(WM_SETFONT, WPARAM(::GetStockObject(DEFAULT_GUI_FONT)));

	// Add toolbar buttons
	//
	SetButtonStructSize(sizeof(TBBUTTON));
	static const int anCommands[]=
	{
		ID_PHOTO_PREV, ID_PHOTO_NEXT, SC_CLOSE
	};
	const int COUNT= array_count(anCommands);		// no of buttons
	CSize btn_size;
	{
		CBitmap Bmp;
		Bmp.LoadBitmap(IDB_TRANSPARENT_BAR);
		BITMAP bmp;
		Bmp.GetBitmap(&bmp);
		btn_size = CSize(bmp.bmWidth / COUNT, bmp.bmHeight);	// determine single button bitmap size
	}
	SetBitmapSize(btn_size);
	SetButtonSize(btn_size + CSize(8, 7));
	AddBitmap(COUNT, IDB_TRANSPARENT_BAR);
//	RString tb(IDS_MAIN_TOOLBAR);
//	tb += "\n";
//	tb.Replace('\n', '\0');
//	int string= AddStrings(tb);
	CSize padding_size= GetApp()->IsWhistlerLookAvailable() ? CSize(5, 9) : CSize(3, 7);
	SendMessage(TB_SETPADDING, 0, MAKELONG(padding_size.cx, padding_size.cy));
	for (int i= 0; i < COUNT; i++)
	{
		TBBUTTON btn;
		if (anCommands[i] == 0)
		{
			btn.iBitmap = 8;
			btn.idCommand = -1;
			btn.fsState = TBSTATE_ENABLED;
			btn.fsStyle = TBSTYLE_SEP;
			btn.data  = 0;
			btn.iString = 0;
			AddButtons(1, &btn);
		}
		btn.iBitmap = i;
		btn.idCommand = anCommands[i];
		btn.fsState = TBSTATE_ENABLED;
		btn.fsStyle = TBSTYLE_BUTTON; // | TBSTYLE_AUTOSIZE;
//		if (anCommands[i] == ID_RECURSIVE || anCommands[i] == ID_EXIF_ONLY)
//			btn.fsStyle |= TBSTYLE_CHECK;
//		else if (anCommands[i] == ID_VIEW_DETAILS || anCommands[i] == ID_VIEW_THUMBNAILS)
//			btn.fsStyle |= TBSTYLE_CHECKGROUP;
//		if (anCommands[i] == ID_FOLDER_LIST)
//			btn.fsStyle |= BTNS_WHOLEDROPDOWN;
//		if (anCommands[i] == ID_BROWSER || anCommands[i] == ID_COMPOSER || anCommands[i] == ID_READ_CAMERA)
//			btn.fsStyle |= BTNS_DROPDOWN;

		btn.data = 0;

//		if (anCommands[i] == ID_BROWSER || anCommands[i] == ID_COMPOSER ||
//			anCommands[i] == ID_READ_CAMERA || anCommands[i] == ID_FOLDER_LIST)
//			btn.iString = string++;
//		else
			btn.iString = -1;

		AddButtons(1, &btn);
	}
//	SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	return true;
}
