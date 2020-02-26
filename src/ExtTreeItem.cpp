/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeItem.cpp: implementation of the ExtTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExtTreeItem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ExtTreeItem::ExtTreeItem()
{
	item_ = 0;
}

ExtTreeItem::~ExtTreeItem()
{}


void ExtTreeItem::GetDisplayText(int /*column*/, CString& rstrBuffer, bool& bold_font) const
{
	rstrBuffer = _T('-');
	bold_font = false;
}


bool ExtTreeItem::IsBold() const
{
	return false;
}


void ExtTreeItem::IntToStr(int value, CString& rstrBuffer) const
{
	if (value >= 0)
		rstrBuffer.Format(_T("%d"), value);
	else
		rstrBuffer = _T("-");
}


bool ExtTreeItem::IsActive() const
{
	return true;
}


int ExtTreeItem::GetImageIndex() const
{
	return 0;
}
