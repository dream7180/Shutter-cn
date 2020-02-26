/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern int GetLineHeight(CDC& dc)
{
	TEXTMETRIC tm;
	if (dc.GetTextMetrics(&tm))
	{
		int line_height= tm.tmHeight + /*tm.tmInternalLeading +*/ tm.tmExternalLeading;
		return line_height;
	}
	else
		return 13;
}


extern int GetDefaultLineHeight()
{
	CDC dc;
	dc.CreateCompatibleDC(0);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
	g_Settings.SelectDefaultFont(dc);
	return GetLineHeight(dc);
}
