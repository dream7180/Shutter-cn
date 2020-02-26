/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifStatusBar.h: interface for the ExifStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXIFSTATUSBAR_H__C4FA1AD6_83C1_46E8_9B85_8ED370C205BB__INCLUDED_)
#define AFX_EXIFSTATUSBAR_H__C4FA1AD6_83C1_46E8_9B85_8ED370C205BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Columns;

struct CExifStatusBarNotifications
{
	virtual void FieldSelectionChanged() = 0;
	virtual void PopupMenu(bool displayed) = 0;
	virtual void StatusPaneClick(const CRect& rect) = 0;
};


class ExifStatusBar : public CStatusBar
{
public:
	ExifStatusBar(const Columns& columns);
	virtual ~ExifStatusBar();

	virtual void DrawItem(DRAWITEMSTRUCT*);

	void SetRecipient(CExifStatusBarNotifications* recipient)
	{
		recipient_ = recipient;
	}

	std::vector<uint16> fields_;

	void DefaultFields();

	void SetColors(COLORREF background, COLORREF text, COLORREF dim_text);

protected:
	//{{AFX_MSG(ExifStatusBar)
	afx_msg void OnContextMenu(CWnd* wnd, CPoint point);
	afx_msg int  OnCreate(LPCREATESTRUCT create_struct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	BOOL OnEraseBkgnd(CDC* dc);

	CExifStatusBarNotifications* recipient_;

	void OptionsPopup(CPoint pos);
	void OnLButtonDown(UINT flags, CPoint pos);

private:
	CImageList funnel_icon_;
	const Columns& columns_;
	COLORREF background_;
	COLORREF text_color_;
	COLORREF dim_text_;
};

#endif // !defined(AFX_EXIFSTATUSBAR_H__C4FA1AD6_83C1_46E8_9B85_8ED370C205BB__INCLUDED_)
