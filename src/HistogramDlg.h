/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_HISTOGRAMDLG_H__FD53900D_CD48_413A_8D0D_9CD778595887__INCLUDED_)
#define AFX_HISTOGRAMDLG_H__FD53900D_CD48_413A_8D0D_9CD778595887__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistogramDlg.h : header file
//
#include "HistogramCtrl.h"
#include "HistogramImg.h"
#include "DialogChild.h"
#include "PhotoCache.h"
class Dib;
class PhotoInfo;

/////////////////////////////////////////////////////////////////////////////
// HistogramDlg dialog

class HistogramDlg : public DialogChild
{
// Construction
public:
	HistogramDlg(const PhotoInfo& photo, PhotoCache* cache);

// Dialog Data
	//{{AFX_DATA(HistogramDlg)
	enum { IDD = IDD_HISTOGRAM };
	CStatic	rect_label_wnd_;
	HistogramImg image_wnd_;
	HistogramCtrl hist_wnd_;
	//}}AFX_DATA
	CListBox channels_wnd_;

	bool PhotoLoaded() const	{ return !bmp_.empty(); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HistogramDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(HistogramDlg)
	afx_msg void OnSelChangeChannel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnOpenImg();
	afx_msg void OnLogScale();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
//	bool opened_img_;
	//CSize opened_dlg_size_;
	//CSize closed_dlg_size_;
	PhotoCache* cache_;
	AutoPtr<Dib> bmp_;
	String name_;

	Dib* LoadPhoto(const PhotoInfo& photo);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTOGRAMDLG_H__FD53900D_CD48_413A_8D0D_9CD778595887__INCLUDED_)
