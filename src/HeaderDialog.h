/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// HeaderDialog.h : header file
//
class DialogChild;
class Dib;
#include "PhotoInfoPtr.h"
#include "MultiMonitor.h"
#include "HeaderDlg.h"


class HeaderDialog : public CDialog, public HeaderDlg
{
// Construction
public:
	enum Image
	{
		IMG_COPY= 0, IMG_SLIDE_SHOW, IMG_RESIZE, IMG_HTML, IMG_EXIF, IMG_ROTATE, IMG_IPTC, IMG_PRINT_THUMBS,
		IMG_COPY_TAGGED, IMG_MOVE, IMG_PENCIL, IMG_PRINT, IMG_HTML2, IMG_ADJUST, IMG_HISTOGRAM, IMG_DATE_TIME,
		IMG_EMAIL, IMG_ALBUM, IMG_EXTRACT
	};

	HeaderDialog(DialogChild& dlg, const TCHAR* title, Image image, CWnd* parent= 0);

	HeaderDialog(DialogChild& dlg, const TCHAR* title, CWnd* parent);

// Dialog Data
	//{{AFX_DATA(HeaderDialog)
//	enum { IDD = IDD_HEADER_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	void SetTitleDuringInit(bool set)		{ set_window_title_ = set; }

private:
	// implementation of HeaderDlg
	virtual void SetExtraImage(Dib* image);
	virtual void SetExtraImage(PhotoInfoPtr photo);

	virtual void SetFooterDlg(CDialog* dlg);
	virtual void ShowFooterDlg(bool show);

	// different color for a right side of a dialog
	virtual void SetRightSide(int width, COLORREF color, const std::vector<float>& shades);
	virtual void SetRightSide(int width);

	virtual void SetBigTitle(const TCHAR* title);

	virtual void SetMinimalDlgSize(CSize minimal);

	virtual void Resize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HeaderDialog)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	virtual BOOL ContinueModal();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(HeaderDialog)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* MMI);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();

	DialogChild* dlg_;
	CImageList image_list_;
	CFont font_;
	CString title_;
	Image image_;
	Dib* extra_image_;
	PhotoInfoPtr photo_image_;
	bool resizable_dlg_;
	CRect child_location_rect_;
	bool child_dlg_ready_;
	WindowPosition wnd_pos_;	// registry settings
	CDialog* footer_dlg_;		// footer dlg (if any)
	bool footer_visible_;		// footer visibility
	bool set_window_title_;
	COLORREF right_side_color_;
	int right_side_;
	std::vector<float> right_side_shades_;
	CSize minimal_size_;
	bool has_header_;

	CRect GetImgRect() const;

private:
//	bool IsResizable();
//	bool IsFooterVisible();
	int GetFooterHeight();
	static const int SEPARATOR_H= 1;	// space for footer separator
	int HeaderHeight() const;

	void Init();
};
