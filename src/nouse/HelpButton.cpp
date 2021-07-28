/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HelpButton.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "HelpButton.h"
#include "LoadImageList.h"
#include "WhistlerLook.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// HelpButton

HelpButton::HelpButton()
{
}

HelpButton::~HelpButton()
{
}


BEGIN_MESSAGE_MAP(HelpButton, CButton)
END_MESSAGE_MAP()



// HelpButton message handlers


void HelpButton::PreSubclassWindow()
{
	if (WhistlerLook::IsAvailable())
	{
		if (!LoadImageList(image_list_, IDB_HELP_BTN, 15, ::GetSysColor(COLOR_3DFACE)))
		{
			ASSERT(false);	// help btn bmp not found?
			return;
		}

		BUTTON_IMAGELIST bi;
		memset(&bi, 0, sizeof bi);
		bi.himl = image_list_.m_hImageList;
		bi.margin = CRect(0,0,0,0);
		bi.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;

		SetImageList(&bi);
	}
	else
	{
		// traditional bitmap btn

		HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDB_HELP_BTN), RT_BITMAP);
		HANDLE dib= ::LoadImage(inst, MAKEINTRESOURCE(IDB_HELP_BTN), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
		if (dib == 0)
		{
			ASSERT(false);	// help btn bmp not found?
			return;
		}
		bitmap_.Attach(HGDIOBJ(dib));

		ModifyStyle(BS_TEXT, BS_BITMAP | BS_CENTER | BS_VCENTER);

		SetBitmap(bitmap_);
	}
}
