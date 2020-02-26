/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(_IMAGE_CTRL_)
#define _IMAGE_CTRL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ImageCtrl window - draw PNG image


class ImageCtrl : public CWnd
{
// Construction
public:
	ImageCtrl();

// Attributes
public:
	void SetImage(int rsrc_id);

// Operations
public:
	bool Create(CWnd* parent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ImageCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ImageCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(ImageCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CImageList image_;
};

#endif // !defined(_IMAGE_CTRL_)
