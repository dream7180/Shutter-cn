/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImgPage.h"
#include "ImgManipulation.h"
#include "PreviewCtrl.h"


// ImgCropPage dialog

class ImgCropPage : public ImgPage, PreviewCtrlNotifications
{
public:
	ImgCropPage(CPreviewCtrl& preview_wnd, PhotoInfoPtr photo, CWnd* parent = NULL);   // standard constructor
	virtual ~ImgCropPage();

	virtual void Transform(Dib& dib, bool preview);

	virtual void TransformFile(Path& pathPhoto, Path& pathOutput);

	virtual bool IsExclusive(PhotoInfoPtr photo) const;

	virtual void RestoreSettings(const TCHAR* key);
	virtual void StoreSettings(const TCHAR* key);

// Dialog Data
	enum { IDD = IDD_IMG_CROP };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	bool in_update_;
	bool lossless_crop_possible_;
	CPreviewCtrl& preview_wnd_;
	CStatic size_wnd_;
	CComboBox comboRatios_;
	CButton reverseBtn_;
	CSize original_aspect_ratio_;
	int selected_ratio_index_;
	bool reverse_constraints_;

	void OnReset();
	void SetValues();

	void OnLosslessCrop();
	void OnFreeCrop();
	void OnConstrain();
	void OnReverse();

	virtual void SelectionRectChanged(const CRect& rect);
};
