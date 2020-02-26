/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski

____________________________________________________________________________*/

#if !defined(AFX_PHOTOLIST_H__1B797685_20F8_4A89_983A_E437153E9475__INCLUDED_)
#define AFX_PHOTOLIST_H__1B797685_20F8_4A89_983A_E437153E9475__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhotoList.h : header file
//
#include "PhotoInfoStorage.h"


/////////////////////////////////////////////////////////////////////////////
// CPhotoList window

class CPhotoList : public CMenu
{
// Construction
public:
	CPhotoList(const VectPhotoInfo& photos);

// Attributes
public:

// Operations
public:
	int TrackPopupMenu(CWnd* parent, CPoint left_top, int current);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhotoList)
public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT measure_item_struct);
	virtual void DrawItem(LPDRAWITEMSTRUCT draw_item_struct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPhotoList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPhotoList)
	//}}AFX_MSG

private:
	const VectPhotoInfo& photos_;
	CSize img_size_;
	CSize text_size_;
	std::vector<int> items_text_width_;
	int vert_max_;
	int horz_max_;
	int current_photo_index_;

	enum { LEFT_MARGIN= 4, TEXT_IMG_SPACE= 4 };

	void AddPhoto(int index, int data, CDC& dc);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHOTOLIST_H__1B797685_20F8_4A89_983A_E437153E9475__INCLUDED_)
