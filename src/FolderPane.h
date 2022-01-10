/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "FolderView.h"
///#include "TaskBarView.h"
#include "Pane.h"
#include "ToolBarWnd.h"
#include "FolderPath.h"
//#include "TopSeparatorCtrl.h"


// FolderPane window (folder tree)

class FolderPane : public PaneWnd
{
// Construction
public:
	FolderPane();

// Attributes
public:
	//bool GetCurrentPath(ItemIdList& idlPath) const;
	FolderPathPtr GetCurrentPath() const;

	// get current path (only to the physical folder)
	CString GetCurrentPhysPath() const;

	CWnd* GetWindow()		{ return &tree_wnd_; }

// Operations
public:
	bool Create(CWnd* parent);

	bool Create2(CWnd* parent, UINT id, FolderPathPtr path, bool root, int width);

//	void ResetPath(const ITEMIDLIST* folder_list, bool root);
	void ResetPath(FolderPathPtr path, bool root);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FolderPane)
	//}}AFX_VIRTUAL

	boost::function<void (FolderPathPtr)> on_folder_changed_;
	boost::function<void (CPoint pos)> on_recent_folders_;
	boost::function<void (void)> on_folder_up_;
	boost::function<bool (void)> on_update_folder_up_;

// Implementation
public:
	virtual ~FolderPane();

	// Generated message map functions
protected:
	//{{AFX_MSG(FolderPane)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR id_event);
	afx_msg void OnFolderRefresh();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	void OnFolderUp();
	void OnUpdateFolderUp(CCmdUI* cmd_ui);
	void OnUpdateRecentFolders(CCmdUI* cmd_ui);
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnNewFolder();
	void OnExploreFolder();
	DECLARE_MESSAGE_MAP()

private:
	enum { TREE_WINDOW_ID= 1234 };
	FolderView tree_wnd_;
	ToolBarWnd tool_bar_wnd_;
	//TopSeparatorCtrl separator_;
	UINT_PTR timer_id_;

	LRESULT OnSelChanged(WPARAM, LPARAM path);
	void OnSelChanged(NMHDR* notify_struct, LRESULT*);
	void SelChangeNotify();
	void RecentFolders();
	virtual void CaptionHeightChanged(bool big);
	virtual void ActivatePane(bool active);
	virtual BOOL IsFrameWnd() const;
	virtual void OptionsChanged(OptionsDlg& dlg);

//	class TreeDropTarget; // : public COleDropTarget

	std::auto_ptr<COleDropTarget> drop_target_;
	void SetColors();
};
