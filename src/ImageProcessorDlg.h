/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"
#include "DialogChild.h"
#include "PhotoInfoStorage.h"


// ImageProcessorDlg dialog

class ImageProcessorDlg : public DialogChild
{
public:
	ImageProcessorDlg(CWnd* parent, VectPhotoInfo& photos);
	virtual ~ImageProcessorDlg();

// Dialog Data
	enum { IDD = IDD_PROCESSOR };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	virtual void OnOK();
	virtual BOOL OnInitDialog();
//	BOOL OnEraseBkgnd(CDC* dc);
	void OnSize(UINT type, int cx, int cy);

	struct Impl;
	std::auto_ptr<Impl> impl_;
};
