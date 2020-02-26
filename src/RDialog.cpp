/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RDialog.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RDialog dialog


RDialog::RDialog(int dlg_id, CWnd* parent/*= 0*/) : CDialog(dlg_id, parent)
{
//	m_lpszTemplateName = 0;
	dlg_id_ = dlg_id;
	//{{AFX_DATA_INIT(RDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(RDialog, CDialog)
	//{{AFX_MSG_MAP(RDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RDialog message handlers

INT_PTR RDialog::DoModal()
{
	m_lpszTemplateName = 0;	// suppress loading dlg template in CDialog::DoModal()
	m_hDialogTemplate = 0;

	const TCHAR* dlg_template= MAKEINTRESOURCE(dlg_id_);
	if (dlg_id_ != 0)
	{
		HINSTANCE inst= AfxFindResourceHandle(dlg_template, RT_DIALOG);
		HRSRC resource= ::FindResourceEx(inst, RT_DIALOG, dlg_template, g_Settings.GetLanguageId());
		if (resource == 0)
		{
			ASSERT(false);	// missing dlg template
			resource = ::FindResource(inst, dlg_template, RT_DIALOG);
		}
		if (resource == 0)
			return -1;
		m_hDialogTemplate = LoadResource(inst, resource);
	}

	INT_PTR ret= CDialog::DoModal();

	// free dlg resources
	if (m_hDialogTemplate != NULL)
	{
		UnlockResource(m_hDialogTemplate);
		FreeResource(m_hDialogTemplate);
		m_hDialogTemplate = 0;
	}

	return ret;
}
/*
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_hDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE dialog_template = m_hDialogTemplate;
	HGLOBAL dialog_template = m_hDialogTemplate;
	HINSTANCE inst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		inst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC resource = ::FindResource(inst, m_lpszTemplateName, RT_DIALOG);
		dialog_template = LoadResource(inst, resource);
	}
	if (dialog_template != NULL)
		dialog_template = (LPCDLGTEMPLATE)LockResource(dialog_template);

	// return -1 in case of failure to load the dialog template resource
	if (dialog_template == NULL)
		return -1;

	// disable parent (before creating dialog)
	HWND wnd_parent = PreModal();
	AfxUnhookWindowCreate();
	BOOL enable_parent = FALSE;
	if (wnd_parent != NULL && ::IsWindowEnabled(wnd_parent))
	{
		::EnableWindow(wnd_parent, FALSE);
		enable_parent = TRUE;
	}

	TRY
	{
		// create modeless dialog
		AfxHookWindowCreate(this);
		if (CreateDlgIndirect(dialog_template,
						CWnd::FromHandle(wnd_parent), inst))
		{
			if (flags_ & WF_CONTINUEMODAL)
			{
				// enter modal loop
				DWORD flags = MLF_SHOWONIDLE;
				if (GetStyle() & DS_NOIDLEMSG)
					flags |= MLF_NOIDLEMSG;
				VERIFY(RunModalLoop(flags) == modal_result_);
			}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		modal_result_ = -1;
	}
	END_CATCH_ALL

	if (enable_parent)
		::EnableWindow(wnd_parent, TRUE);
	if (wnd_parent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(wnd_parent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(dialog_template);
	if (m_lpszTemplateName != NULL)
		FreeResource(dialog_template);

	return modal_result_;
}
*/


BOOL RDialog::OnInitDialog()
{
	// this assignment is needed for executing dialog RT_DLGINIT resource
	m_lpszTemplateName = MAKEINTRESOURCE(dlg_id_);
	BOOL ret= CDialog::OnInitDialog();
	m_lpszTemplateName = 0;
	return ret;
}
