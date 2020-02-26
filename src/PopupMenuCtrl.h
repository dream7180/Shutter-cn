/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PopupMenuCtrl.h: interface for the CPopupMenuCtrl class.

#pragma once
#include "ToolBarWnd.h"


class CPopupMenuCtrl : public ToolBarWnd
{
public:
	CPopupMenuCtrl();
	virtual ~CPopupMenuCtrl();

	bool Create(CWnd* parent, int id);
};
