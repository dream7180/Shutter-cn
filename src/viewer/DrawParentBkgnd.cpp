/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"


extern void DrawParentBkgnd(CWnd& wnd, CDC& dc)
{
	WINDOWPLACEMENT wp;
	if (wnd.GetWindowPlacement(&wp))
	{
		CPoint org= dc.GetWindowOrg();
		dc.SetWindowOrg(org.x + wp.rcNormalPosition.left, org.y + wp.rcNormalPosition.top);
		if (CWnd* parent= wnd.GetParent())
			parent->PrintClient(&dc, PRF_ERASEBKGND | PRF_CLIENT);
		dc.SetWindowOrg(org);
	}
}
