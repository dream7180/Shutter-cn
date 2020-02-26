/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RMenu.cpp: implementation of the RMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RMenu.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RMenu::RMenu()
{
}

RMenu::~RMenu()
{
}


bool RMenu::LoadMenu(UINT id_resource)
{
	const TCHAR* menu_template= MAKEINTRESOURCE(id_resource);

	HINSTANCE inst= AfxFindResourceHandle(menu_template, RT_MENU);
	HRSRC resource= ::FindResourceEx(inst, RT_MENU, menu_template, g_Settings.GetLanguageId());
	if (resource == 0)
	{
		ASSERT(false);	// missing dlg template
		resource = ::FindResource(inst, menu_template, RT_MENU);
	}
	if (resource == 0)
		return false;
	HGLOBAL menu_template_h= ::LoadResource(inst, resource);
	void* menu_template_ptr= ::LockResource(menu_template_h);

	BOOL result= LoadMenuIndirect(menu_template_ptr);

	::UnlockResource(menu_template_h);

	return !!result;
}
