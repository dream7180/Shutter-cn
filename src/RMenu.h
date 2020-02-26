/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RMenu.h: interface for the RMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RMENU_H__F7309DF2_6BC9_455B_813B_DBD9677E40BA__INCLUDED_)
#define AFX_RMENU_H__F7309DF2_6BC9_455B_813B_DBD9677E40BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class RMenu : public CMenu
{
public:
	RMenu();
	virtual ~RMenu();

	bool LoadMenu(UINT id_resource);
};

#endif // !defined(AFX_RMENU_H__F7309DF2_6BC9_455B_813B_DBD9677E40BA__INCLUDED_)
