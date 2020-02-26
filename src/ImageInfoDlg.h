/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImageListCtrl.h"
class Dib;


// ImageInfoDlg dialog

class ImageInfoDlg : public CDialog
{
public:
	ImageInfoDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~ImageInfoDlg();

// Dialog Data
	enum { IDD = IDD_IMG_INFO };

	bool Create(CWnd* parent);

	void ShowColors(RGBQUAD original, RGBQUAD new_color, bool hide_colors);

	CImageListCtrl ctrl_image_;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CBitmap eye_drop_bmp_;
	CImageList image_list_;

	DECLARE_MESSAGE_MAP()
};
