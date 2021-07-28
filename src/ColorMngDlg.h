/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DlgAutoResize.h"
#include "MultiMonitor.h"
#include "ToolBarWnd.h"
#include "ExtTreeCtrl.h"
#include "ExtTreeNode.h"
#include "ICCScanner.h"
#include "ColorProfile.h"


// ColorMngDlg dialog

class ColorMngDlg : public CDialog
{
public:
	ColorMngDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~ColorMngDlg();

// Dialog Data
	enum { IDD = IDD_ICM_SETUP };

	ICMProfilePtr monitor_viewer_;
	ICMProfilePtr monitor_main_wnd_;
	ICMProfilePtr default_printer_;
	ICMProfilePtr default_image_;
	bool changed_;

protected:
	enum { IDC_GRIP = 50 };

	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
//	afx_msg void OnCustomDraw(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* MMI);
//	afx_msg void OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	void OnSize(UINT type, int cx, int cy);
	void OnDestroy();
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);
	LRESULT OnItemClicked(WPARAM item, LPARAM column);
	LRESULT OnItemChanged(WPARAM item, LPARAM);
	void OnItemClicked(ExtTreeRow* tree_item, HTREEITEM item, int column);
	void InsertItems(ExtTreeNode* node);

	// list of color profiles
	ExtTreeCtrl tree_wnd_;
	CScrollBar grip_wnd_;
	CImageList image_list_;
	DlgAutoResize dlg_resize_map_;
	WindowPosition wnd_pos_;		// registry settings
	ToolBarWnd tool_bar_wnd_;
	//CFont bold_fnt_;

	CExtTreeNodePtr monitors_;
	CExtTreeNodePtr printers_;
	CExtTreeNodePtr images_;

	bool profile_files_ready_;
	std::vector<ICCProfileInfo> icc_files_;

	void ShowProfilesMenu(ExtTreeRow* item, CPoint pos);
	void UpdateInfo(ICMProfilePtr icm);
};
