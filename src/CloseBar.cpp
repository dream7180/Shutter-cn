/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CloseBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CloseBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CloseBar

bool CloseBar::Create(CWnd* parent, bool close_and_restore/*= true*/)
{
	SetOnIdleUpdateState(false);

	const int commands[]=
	{
		SC_RESTORE, SC_CLOSE
	};
	//if (!ToolBarWnd::Create(close_and_restore ? "pp" : ".p", commands, IDR_CLOSEBAR, 0, parent))
	if (!ToolBarWnd::Create(close_and_restore ? "pp" : ".p", commands, IDB_CLOSE_TB, 0, parent))
		return false;

	//SetHotImageList(IDB_CLOSE_BAR_HOT);

	return true;
}


CString CloseBar::GetToolTip(int cmd_id)
{
	/*CString tip;

	if (cmd_id == SC_RESTORE)
		tip.LoadString(IDS_RESTOR_WND);
	else if (cmd_id == SC_CLOSE)
		tip.LoadString(IDS_CLOSE_WND);
	else
		return ToolBarWnd::GetToolTip(cmd_id);

	return tip;*/
	return _T("");
}



/*
	int height= GetApp()->IsWhistlerLookAvailable() ? 20 : 18;

	if (!ToolBarWnd::Create(WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT |
		CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER, CRect(0,0,42,height), parent, id))
	{
		ASSERT(false);
		return false;
	}

	SendMessage(WM_SETFONT, WPARAM(::GetStockObject(DEFAULT_GUI_FONT)));

	// Add toolbar buttons
	//
	SetButtonStructSize(sizeof(TBBUTTON));
	static const int anCommands[]=
	{
		SC_RESTORE, SC_CLOSE
	};
	const int COUNT= array_count(anCommands);		// no of buttons
	CSize btn_size;
	{
		CBitmap Bmp;
		Bmp.LoadBitmap(IDR_CLOSEBAR);
		BITMAP bmp;
		Bmp.GetBitmap(&bmp);
		btn_size = CSize(bmp.bmWidth / COUNT, bmp.bmHeight);	// determine single button bitmap size
	}
	SetBitmapSize(btn_size);
	SetButtonSize(btn_size + CSize(8, 7));
	AddBitmap(COUNT, IDR_CLOSEBAR);
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
} */
