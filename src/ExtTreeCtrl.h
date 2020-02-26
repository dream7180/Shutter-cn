/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_EXTTREECTRL_H__FC0A16AF_34FD_11D5_8E84_00B0D078DE24__INCLUDED_)
#define AFX_EXTTREECTRL_H__FC0A16AF_34FD_11D5_8E84_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtTreeCtrl.h : header file
//
class ExtTreeItem;
#include "CtrlDraw.h"


/////////////////////////////////////////////////////////////////////////////
// ExtTreeCtrl window

class ExtTreeCtrl : public CTreeCtrl
{
// Construction
public:
	ExtTreeCtrl();

// Attributes
public:
	CTreeCtrl	tree_wnd_;
	CHeaderCtrl	header_wnd_;
	// cell rectangle
	CRect GetCellRect(HTREEITEM item, int column, bool screen);
	// get selected item
	HTREEITEM GetSelectedItem() const;
	// get item data
	DWORD_PTR GetItemData(HTREEITEM item) const;

// Operations
public:
	bool Create(CWnd* parent, CImageList* img_list, UINT id, bool check_boxes);

	// insert column in header ctrl
	void InsertColumn(int col_index, const TCHAR* col_text, int width, UINT format= HDF_LEFT);

	// insert tree item
	HTREEITEM InsertItem(const TCHAR* item, int image, int selected_image,
		HTREEITEM parent= TVI_ROOT, HTREEITEM insert_after= TVI_LAST);

	// insert ext tree item
	HTREEITEM InsertItem(ExtTreeItem* row, HTREEITEM parent, int image, bool bold, bool check_box);

	// set order of columns
	void SetColumnOrderArray(int count, int* order_array);

	// set column width
	void SetColumnWidth(int col, int width);

	// get column width
	int GetColumnWidth(int col);

	// get column order info
	void GetColumnOrderArray(int count, int* order_array);

	// redraw given item
	void RedrawItem(HTREEITEM item);
	void RedrawItem(ExtTreeItem* item);

	// modify item state
	void SetItemBold(HTREEITEM item, bool bold);

	// invalidate tree view
	void Invalidate();

	// set image list
	void SetImageList(CImageList* image_list);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ExtTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ExtTreeCtrl();

	enum { NOTIFY_ITEM_CLICKED= WM_USER + 600, NOTIFY_SPACE_PRESSED, NOTIFY_SEL_CHANGED, NOTIFY_ITEM_DLBCLICKED };

	// Generated message map functions
protected:
	//{{AFX_MSG(ExtTreeCtrl)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDrawTree(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnEndTrack(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnHeaderTrack(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnClickTreeItem(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnKeyDownTreeItem(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnSelChangedTreeItem(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnRClickTreeItem(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnDblClickTreeItem(NMHDR* nmhdr, LRESULT* result);
	void OnClickTreeItem(bool double_clk);
	virtual void PreSubclassWindow();

private:
	enum { CTR_ID_HEADER= 100, CTR_ID_TREE= 101 };

	static CString wnd_class_;	// registered window class
	void RegisterWndClass();
	bool CreateTree();
	void Resize();

	int header_height_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXTTREECTRL_H__FC0A16AF_34FD_11D5_8E84_00B0D078DE24__INCLUDED_)
