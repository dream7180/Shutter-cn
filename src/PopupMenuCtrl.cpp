/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "PopupMenuCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CPopupMenuCtrl::CPopupMenuCtrl()
{
}

CPopupMenuCtrl::~CPopupMenuCtrl()
{
}


bool CPopupMenuCtrl::Create(CWnd* parent, int id)
{
	int cmd[]= { id };	// double duty of id: toolbar's id, and cmd id
	SetPadding(1, 6);
	return ToolBarWnd::Create("p", cmd, IDR_AUTO_COMPLETE, IDS_EMPTY, parent, id);
}
