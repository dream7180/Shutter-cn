/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSGENERAL_H__9E46E756_FF96_4829_BCEE_38FF28C0282D__INCLUDED_)
#define AFX_OPTIONSGENERAL_H__9E46E756_FF96_4829_BCEE_38FF28C0282D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsGeneral.h : header file
//
#include "RPropertyPage.h"
#include "PathEdit.h"
#include "ICMProfile.h"
#include "GridCtrl.h"
#include "LinkWnd.h"
#include "DibDispMethod.h"
#include "JPEGDecoderMethod.h"
#include "GenThumbMode.h"

/////////////////////////////////////////////////////////////////////////////
// OptionsGeneral dialog

class OptionsGeneral : public RPropertyPage, GridCtrlNotification
{
//	DECLARE_DYNCREATE(OptionsGeneral)

// Construction
public:
	OptionsGeneral();
	~OptionsGeneral();

// Dialog Data
	//{{AFX_DATA(OptionsGeneral)
	enum { IDD = IDD_OPTIONS_GENERAL };
	CStatic	resolution_wnd_;
	BOOL	correct_aspect_ratio_;
	CString	open_photo_app_;
	CString open_raw_photo_app_;
	//}}AFX_DATA
	float	horz_resolution_;
	float	vert_resolution_;
	int		image_cache_size_;
//	JpegDecoderMethod dct_method_;
//	DibDispMethod display_method_;
//	bool	preload_;
//	bool	save_tags_;
//	bool	show_thumb_warning_;
//	uint32	db_file_length_limit_mb_;
//	int		image_blending_;
//	GenThumbMode generate_thumbs_;
	ICMProfilePtr monitor_viewer_;
	ICMProfilePtr monitor_main_wnd_;
	ICMProfilePtr default_printer_;
	ICMProfilePtr default_image_;
	bool	profiles_changed_;
	CPathEdit edit_path_photo_app_;
	CPathEdit edit_path_raw_photo_app_;
	GridCtrl foV_crop_wnd_;
	//CLinkWnd advanced_wnd_;
//	bool	read_thumbs_from_db_;
//	bool	delete_cache_file_;
//	bool	allow_magnifying_above100_;
//	bool	close_app_;
//	int		smooth_scroll_speed_;
//	String	db_cache_path_;
//	int		sharpening_;

	std::vector<std::pair<String, String>> data_;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(OptionsGeneral)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(OptionsGeneral)
	afx_msg void OnResolution();
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaPosGammaSpin(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnChangeGamma();
	afx_msg void OnOpenApp();
	afx_msg void OnOpenRawApp();
	afx_msg void OnAdvanced();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnICMSetup();
	void UpdateRes();

private:
	//CGrayscaleWnd grayscale_wnd_;
	// notifications from the grid ctrl
	virtual void GetCellText(GridCtrl& ctrl, size_t row, size_t col, CString& text);
	virtual void CellTextChanged(GridCtrl& ctrl, size_t row, size_t col, const CString& text);
	virtual void Delete(GridCtrl& ctrl, size_t row, size_t col);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSGENERAL_H__9E46E756_FF96_4829_BCEE_38FF28C0282D__INCLUDED_)
