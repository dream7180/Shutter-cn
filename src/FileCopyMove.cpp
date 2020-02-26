/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "FileCopyMove.h"
#include "CatchAll.h"
#include "resource.h"

#if 0
void FileOperation(const VectPhotoInfo& photos, bool copy, String destPath, CWnd* parent)
{
	if (photos.empty())
		return false;

	try
	{
		Profile<String> profileDestPath(REGISTRY_ENTRY_EXIF, _T("DestPath"), _T("c:\\"));
		String dest_path = profileDestPath;
//		FileOperDlg dlg(copy, dest_path.c_str(), this);
		CopyMoveDlg dlg(copy, destPath.c_str(), parent);
		HeaderDialog dlgHdr(dlg, copy ? _T("Copy") : _T("Move"), copy ? HeaderDialog::IMG_COPY : HeaderDialog::IMG_MOVE, parent);
		if (dlgHdr.DoModal() != IDOK)
			return;

		destPath = dlg.DestPath();
		profileDestPath = dest_path;
		dest_path += _T('\0');

		String src;
		src.reserve(photos.front()->path_.length() * photos.size());

		for (VectPhotoInfo::const_iterator it= photos.begin(); it != photos.end(); ++it)
			src += (*it)->path_ + _T('\0');

		src += _T('\0');

		SHFILEOPSTRUCT op;
		memset(&op, 0, sizeof op);
		op.hwnd   = parent ? parent->m_hWnd : m_hWnd;
		op.func  = copy ? FO_COPY : FO_MOVE;
		op.from  = src.data();
		op.to    = dest_path.data();
		op.flags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
		op.any_operations_aborted = false;
		op.name_mappings = 0;
		op.progress_title = 0;
		if (::SHFileOperation(&op) == 0)
		{
			return true;
			//if (!copy)
			//{
			//	VectPhotoInfo selected;
			//	GetSelectedPhotos(selected);
			//	if (photos != selected)
			//		SelectPhotos(photos);
			//	RemoveSelected(photos);
			//}
		}
	}
	CATCH_ALL

	return false;
}
#endif
