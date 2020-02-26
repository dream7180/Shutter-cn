/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageListCtrl.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CImageListCtrl window

class CImageListCtrl : public CWnd
{
// Construction
public:
	CImageListCtrl();

// Attributes
public:
	void SetImageList(CImageList* image_list);

	void SelectImages(const std::vector<int>& images);

	void SetImageSpace(int space);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CImageListCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CImageList* image_list_;
	std::vector<int> images_;
	int extra_space_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
