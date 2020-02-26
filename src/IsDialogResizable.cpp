/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DlgTemplateEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


bool IsDialogResizable(CDialog* dlg)
{
	struct xCDialog : public CDialog
	{
		LPCTSTR Id() const	{ return m_lpszTemplateName; }
	};

	//CDialog* dlg= dlg;
	LPCTSTR template_name= static_cast<xCDialog*>(dlg)->Id();
	ASSERT(template_name != NULL);

	HINSTANCE inst= AfxFindResourceHandle(template_name, RT_DIALOG);
	HRSRC resource= ::FindResource(inst, template_name, RT_DIALOG);
	HGLOBAL dialog_template= LoadResource(inst, resource);

	if (dialog_template == NULL)
	{
		ASSERT(false);
		return false;
	}

	DWORD size= ::SizeofResource(inst, resource);
	void* dialog_template_ptr= ::LockResource(dialog_template);

	std::vector<BYTE> a_template;
	a_template.resize(size);
	DLGTEMPLATEEX* dlg_template= reinterpret_cast<DLGTEMPLATEEX*>(&a_template.front());
	memcpy(dlg_template, dialog_template_ptr, size);

	ASSERT(dlg_template->dlgVer == 1 && dlg_template->signature == 0xffff);

	// unlock/free resources as necessary
	if (/*m_lpszTemplateName != NULL || m_*/ dialog_template_ptr != NULL)
	{
		UnlockResource(dialog_template);
//	if (m_lpszTemplateName != NULL)
		FreeResource(dialog_template);
	}

	return (dlg_template->style & WS_THICKFRAME) != 0;
}
