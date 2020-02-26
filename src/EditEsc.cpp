/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// EditEsc.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "EditEsc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EditEsc

EditEsc::EditEsc()
{
}

EditEsc::~EditEsc()
{
}


BEGIN_MESSAGE_MAP(EditEsc, CEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditEsc message handlers

void EditEsc::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE || chr == VK_RETURN)
	{
		if (!end_event_.empty())
			end_event_(chr);
	}
	else
		CEdit::OnKeyDown(chr, rep_cnt, flags);
}


void EditEsc::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE || chr == VK_RETURN)
	{
		// no-op
		// this is to suppress 'ding' sound
	}
	else
		CEdit::OnChar(chr, rep_cnt, flags);
}


slot_connection EditEsc::ConnectEndEditKeyPress(EndEditKeyPress::slot_function_type fn)
{
	return end_event_.connect(fn);
}
