/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSFILETYPES_H__CEF78D9B_B6DF_40B9_B59C_2D445F886BDB__INCLUDED_)
#define AFX_OPTIONSFILETYPES_H__CEF78D9B_B6DF_40B9_B59C_2D445F886BDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsFileTypes.h : header file
//
#include "PhotoCtrl.h"
#include "RPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// OptionsFileTypes dialog


class OptionsFileTypes : public RPropertyPage, PhotoCtrlNotification
{
	DECLARE_DYNCREATE(OptionsFileTypes)

// Construction
public:
	OptionsFileTypes();
	~OptionsFileTypes();

// Dialog Data
	//{{AFX_DATA(OptionsFileTypes)
	enum { IDD = IDD_OPTIONS_FILE_TYPES };
	CListCtrl	types_wnd_;
	//}}AFX_DATA
	std::vector<bool> markers_;
	std::vector<bool> no_exif_;
	std::vector<bool> scan_;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(OptionsFileTypes)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(OptionsFileTypes)
	virtual BOOL OnInitDialog();
	afx_msg void OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnListCtrlDraw(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnClickList(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnShowMarker();
	afx_msg void OnUpdateShowMarker(CCmdUI* cmd_ui);
	afx_msg void OnHideMarker();
	afx_msg void OnUpdateHideMarker(CCmdUI* cmd_ui);
	afx_msg void OnDestroy();
	afx_msg void OnShowNoExif();
	afx_msg void OnUpdateShowNoExif(CCmdUI* cmd_ui);
	afx_msg void OnHideNoExif();
	afx_msg void OnUpdateHideNoExif(CCmdUI* cmd_ui);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CImageList img_list_menu_btn_;
	PhotoCtrl example_wnd_;
	CListCtrl border_wnd_;
	PhotoInfoStorage photos_;
	int no_marker_entry_;

	void ShowMarker(bool show);
	void ShowNoExif(bool show);
	BOOL InitDialog();

	virtual bool ShowPhotoMarker(int file_type);
	virtual bool ShowNoExifIndicator(int file_type);
	Dib* RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available);

	void OnSize(UINT type, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSFILETYPES_H__CEF78D9B_B6DF_40B9_B59C_2D445F886BDB__INCLUDED_)
