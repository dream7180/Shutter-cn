/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "VectPhotoInfo.h"
#include "PropertyDlg.h"
struct XmpData;


class EditFileInfo
{
public:
	typedef boost::function<void (PhotoInfo& photo)> PhotoChangedFn;

	EditFileInfo(VectPhotoInfo& all_photos, VectPhotoInfo& selected_photos, CWnd* parent, const PhotoChangedFn& fn);

	bool DoModal();

private:
	PhotoChangedFn photo_changed_;
	VectPhotoInfo& selected_photos_;
	VectPhotoInfo& all_photos_;
	VectPhotoInfo* photos_;
	size_t current_photo_;
	CWnd* parent_wnd_;
	bool multi_edit_;

	void PopupMenuInit(CMenu* popup, int cmd_id);

	bool FileInfoCallback(CPropertyDlg* dlg, CPropertyDlg::Action action, PhotoInfoPtr photo, const XmpData& xmp, bool modified, int index);
};


extern void SaveMetadata(PhotoInfo& photo, const XmpData& xmp, bool notify, const PhotoInfo::WriteAccessFn& fn);
