/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_GENHTMLALBUMDLG_H__8BAC25E1_9987_4EA0_B3AB_D32AB96A0095__INCLUDED_)
#define AFX_GENHTMLALBUMDLG_H__8BAC25E1_9987_4EA0_B3AB_D32AB96A0095__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GenHTMLAlbumDlg.h : header file
//
#include "DialogChild.h"
#include "ColorButton.h"
#include "ResizeDlg.h"
#include "PathEdit.h"

/////////////////////////////////////////////////////////////////////////////
// GenHTMLAlbumDlg dialog

class GenHTMLAlbumDlg : public ResizeDlg
{
// Construction
public:
	GenHTMLAlbumDlg(CWnd* parent, double ratio, OrientationOfImages orientation);

// Dialog Data
	//{{AFX_DATA(GenHTMLAlbumDlg)
	enum { IDD = IDD_GEN_HTML };
	CSpinButtonCtrl	thumb_spin2_wnd_;
	CSpinButtonCtrl	thumb_spin1_wnd_;
	CSpinButtonCtrl	grid_spin2_wnd_;
	CSpinButtonCtrl	grid_spin1_wnd_;
	ColorButton	btn_color_text_;
	ColorButton	btn_color_preview_;
	ColorButton	btn_color_page_;
	CStatic	frames_wnd_;
	CStatic	grid_wnd_;
	int		type_;
	int		grid_columns_;
	int		grid_rows_;
	int		thumb_height_;
	int		thumb_width_;
	CString	root_dir_;
	CString	photos_dir_;
	CString	thumbs_dir_;
	//}}AFX_DATA
	COLORREF rgb_page_backgnd_;
	COLORREF rgb_page_text_;
	COLORREF rgb_preview_backgnd_;
	CString page_title_;
	CString page_file_name_;
	int item_text_type_;
	CEdit edit_grid_size_;
	BOOL text_1_;
	BOOL text_2_;
	BOOL text_3_;
	BOOL text_4_;
	BOOL open_in_browser_;

	enum { CUSTOM_COLORS_SIZE= 16 };
	std::vector<COLORREF> custom_colors_;

	const TCHAR* GetRootDir() const			{ return root_dir_; }
	const TCHAR* GetPhotosDir() const		{ return photos_dir_; }
	const TCHAR* GetThumbsDir() const		{ return thumbs_dir_; }
	int GetJPEGQuality() const				{ return compr_level_; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GenHTMLAlbumDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GenHTMLAlbumDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnColorPage();
	afx_msg void OnColorPreview();
	afx_msg void OnColorText();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual int TranslateSizeSlider(int pos);
	void OnGridClicked();
	void OnFramesClicked();
	void OnTypeChange();

private:
	CBitmap grid_bmp_;
	CBitmap frames_bmp_;
	void MapColor(int id, CBitmap& bmp, CStatic& wnd);
	void MapColors();
	void OnSelColor(COLORREF& rgb_color, ColorButton& btn);
	void OnBrowse();

	CPathEdit edit_root_;
	CPathEdit edit_photos_;
	CPathEdit edit_thumbs_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENHTMLALBUMDLG_H__8BAC25E1_9987_4EA0_B3AB_D32AB96A0095__INCLUDED_)
