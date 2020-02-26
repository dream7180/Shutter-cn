/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "HelpButton.h"
#include "RPropertyPage.h"
#include "DibDispMethod.h"
#include "GenThumbMode.h"

// AdvancedOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AdvancedOptions dialog

class AdvancedOptions : public RPropertyPage
{
// Construction
public:
	AdvancedOptions();

// Dialog Data
	enum { IDD = IDD_OPTIONS_ADVANCED };

	int		image_blending_;
	CStatic	db_size_label_wnd_;
	CSliderCtrl	db_size_slider_wnd_;
	CStatic	ram_wnd_;
	CSliderCtrl	cache_slider_wnd_;
	int		dct_method_;
	int		display_method_int_;
	int		cache_size_;
	BOOL	save_tags_;
	BOOL	show_warning_;
	BOOL	preload_;
	int		db_size_;
	int		generate_thumbs_int_;
	int		thumb_access_;
	bool	delete_cache_file_;
	BOOL	allow_magnifying_above100_;
	BOOL	allow_zoom_to_fill_;
	int		percent_of_image_to_hide_;
	BOOL	close_app_;
	int		smooth_scroll_;
	String	db_path_;
	int		sharpening_;
	CSliderCtrl	sharpening_slider_;
	DibDispMethod display_method_;
	uint32	db_file_length_limit_mb_;
	GenThumbMode generate_thumbs_;

	static int db_sizes_[];

	void SetDbFileLimit(uint32 db_file_length_limit_mb);
	uint32 GetDbFileLimit() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AdvancedOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetRAMLabel();
	void SetDbSizeLabel();

	// Generated message map functions
	//{{AFX_MSG(AdvancedOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnRestoreDefaults();
	afx_msg void OnClearCache();
	void OnHelpBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CButton btn_clear_;
	CButton set_db_path_;
	CEdit db_path_ctrl_;
//	HelpButton btn_help_;
	CSliderCtrl smooth_scroll_slider_;

	void OnSetDbPath();
};
