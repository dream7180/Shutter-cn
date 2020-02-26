/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


class ArrowButton : public CButton
{
// Construction
public:
	ArrowButton();

// Attributes
public:
	void DrawLeftArrow(bool left);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ArrowButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ArrowButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(ArrowButton)
	afx_msg void OnPaint();
	afx_msg void OnEnable(BOOL enable);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	bool left_arrow_;
	void PaintIt(int offset, bool enabled);
};
