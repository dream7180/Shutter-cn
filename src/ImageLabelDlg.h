/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ColumnTree.h"
class Columns;


// ImageLabelDlg dialog

class ImageLabelDlg : public CDialog
{
public:
	ImageLabelDlg(const std::vector<uint16>& thumbs, const std::vector<uint16>& previews, CWnd* parent, Columns& columns);
	virtual ~ImageLabelDlg();

// Dialog Data
	enum { IDD = IDD_IMAGE_LABEL };

	void GetSelection(int index, std::vector<uint16>& fields);

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CTabCtrl tab_ctrl_;
	ColumnTree tree_1_;
	ColumnTree tree_2_;
	CImageList imageList_;
	const std::vector<uint16>& thumbs_;
	const std::vector<uint16>& previews_;

	virtual BOOL OnInitDialog();
	afx_msg void OnTabChanged(NMHDR* nmhdr, LRESULT* result);
};
