/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "CatalogDlg.h"
#include "HeaderDialog.h"
#include "PhotoInfo.h"


extern void BuildCatalog(const TCHAR* path)
{
	CWnd* parent= AfxGetMainWnd();

	CatalogDlg dlg(parent, path);
	HeaderDialog dlgHdr(dlg, _T("Catalog Images"), HeaderDialog::IMG_ALBUM);
	if (dlgHdr.DoModal() != IDOK)
		return;

	//

}
