/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RPropertyPage.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RPropertyPage property page


RPropertyPage::RPropertyPage(int dlg_id) : CPropertyPage(dlg_id)
{
	dlg_id_ = dlg_id;
	//{{AFX_DATA_INIT(RPropertyPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_psp.pResource = 0;
	dlg_ = 0;

	if (dlg_id_ != 0)
	{
		const TCHAR* dlg_template= MAKEINTRESOURCE(dlg_id_);

		HINSTANCE inst= AfxFindResourceHandle(dlg_template, RT_DIALOG);
		HRSRC resource= ::FindResourceEx(inst, RT_DIALOG, dlg_template, g_Settings.GetLanguageId());
		if (resource == 0)
		{
			ASSERT(false);	// missing dlg template
			resource = ::FindResource(inst, dlg_template, RT_DIALOG);
		}
		if (resource == 0)
			return;

		dlg_ = ::LoadResource(inst, resource);
		m_psp.pResource = reinterpret_cast<const DLGTEMPLATE*>(::LockResource(dlg_));
		m_psp.dwFlags |= PSP_DLGINDIRECT;
	}
}


RPropertyPage::~RPropertyPage()
{
	if (dlg_ != 0)
	{
		::UnlockResource(dlg_);
		::FreeResource(dlg_);
	}
}


BEGIN_MESSAGE_MAP(RPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(RPropertyPage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RPropertyPage message handlers

void RPropertyPage::OnSize(UINT type, int cx, int cy)
{
	if (type != SIZE_MINIMIZED)
		resize_.Resize();
	CPropertyPage::OnSize(type, cx, cy);
}
