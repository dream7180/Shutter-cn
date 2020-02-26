/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_RESIZEDLG_H__5BE1B698_FF9A_42CA_B70F_FF09D4D019A1__INCLUDED_)
#define AFX_RESIZEDLG_H__5BE1B698_FF9A_42CA_B70F_FF09D4D019A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "RDialog.h"
#include "DialogChild.h"
#include "PathEdit.h"
#include "ResizeOptionsDlg.h"
#include "RecentPaths.h"
#include "ACListWnd.h"

// ResizeDlg.h : header file
//

enum OrientationOfImages
{
	LANDSCAPE_ORIENTATION,
	PORTRAIT_ORIENTATION,
	MIXED_ORIENTATION
};

/////////////////////////////////////////////////////////////////////////////
// ResizeDlg dialog

class ResizeDlg : public DialogChild
{
// Construction
public:
	ResizeDlg(CWnd* parent, double ratio, OrientationOfImages orientation, UINT dlg_id= IDD);

// Dialog Data
	//{{AFX_DATA(ResizeDlg)
	enum { IDD = IDD_RESIZE };
	CStatic	compr_ratio_wnd_;
	CSpinButtonCtrl	spin_compr_level_;
	CEdit	edit_compr_level_;
	CSliderCtrl	slider_size_;
	CSliderCtrl	slider_quality_;
	CButton	btn_jpeg_;
	CButton	btnBMP_;
	CEdit	edit_width_;
	CEdit	edit_height_;
	CEdit	edit_size_percent_;
	CComboBox dest_path_combo_;
	CButton	btn_relative_size_;
	CButton	btn_fixed_size_;
	CStatic	example_wnd_;
	CEdit	edit_suffix_;
	CButton	btn_same_folder_;
	int		size_percent_;
	int		width_;
	int		height_;
	int		size_selection_;
	int		same_dir_;
	int		output_format_;
	CString	suffix_;
	CString	dest_path_;
	int		size_;
	int		quality_;
	int		compr_level_;
	//}}AFX_DATA
	CSize image_size_;
	CString registry_;
	int options_dlg_id_;
	CResizeOptionsDlg dlg_options_;

	const TCHAR* GetDestPath() const	{ return dest_path_; }
	int GetJPEGQuality() const			{ return compr_level_; }


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ResizeDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ResizeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSameDir();
	afx_msg void OnSelectDir();
	afx_msg void OnFixedSize();
	afx_msg void OnPercentageSize();
	afx_msg void OnBrowse();
	afx_msg void OnChangeSuffix();
	afx_msg void OnJpeg();
	afx_msg void OnBmp();
	virtual void OnOK();
	afx_msg void OnChangeHeight();
	afx_msg void OnChangeWidth();
	afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnChangeComprLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	bool Finish();
	virtual int TranslateSizeSlider(int pos);

private:
	void ReadOnlyCtrl(CEdit* wnd, bool read_only);
	void UpdateDirs();
	void UpdateDims();
	void UpdateFmt();
	void UpdateExample();
	void UpdateQuality();
	void UpdateSize();
	const double ratio_;
	OrientationOfImages orientation_;
	bool update_;
	bool force_jpeg_;
	PathVector recent_paths_;
	//AutoCompletePopup auto_complete_;
	std::vector<String> recent_path_strings_;
	CPathEdit dest_path_editbox_;

	void OnOptions();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZEDLG_H__5BE1B698_FF9A_42CA_B70F_FF09D4D019A1__INCLUDED_)
