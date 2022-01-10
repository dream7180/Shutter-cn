/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MainFrm.h : interface of the MainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__7EF7528D_4A25_492F_AA0A_DB348E206AC0__INCLUDED_)
#define AFX_MAINFRM_H__7EF7528D_4A25_492F_AA0A_DB348E206AC0__INCLUDED_

#include "FavoritesFolders.h"
#include "MultiMonitor.h"
#include "ExifStatusBar.h"
#include "Accelerator.h"
#include "CloseBar.h"
#include "FolderPath.h"
#include "Columns.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class MainFrame : public CFrameWnd
{
	DECLARE_DYNAMIC(MainFrame)
public:
	MainFrame();

// Attributes
public:
	const Accelerator& GetAccelerator() const		{ return accelerator_; }

// Operations
public:
//	void SetLoadingIndicator();
//	void SetLoadingIndicator(int photo_count);
	void UpdateStatusBar();

	String Transfer(const TCHAR* path);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MainFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//virtual void WinHelp(DWORD data, UINT cmd = HELP_CONTEXT);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* context);
	//}}AFX_VIRTUAL
	virtual HACCEL GetDefaultAccelerator();

// Implementation
public:
	virtual ~MainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	ExifStatusBar	status_bar_wnd_;
	CAnimateCtrl	scanning_wnd_;
	bool			scanning_bar_;
//	CReBar       rebar_wnd_;
	AutoPtr<CRecentFileList> recent_path_list_;
	AutoPtr<FavoriteFolders> favorite_folders_;
	// registry settings
	WindowPosition	wnd_pos_;
	Accelerator	accelerator_;
	Columns		columns_;

	enum { MAX_FAVORITE_FOLDERS= 1000, MAX_FILE_MASKS= 100 };
	//enum { MAX_RECENT_PATHS= 20 };

	virtual void GetMessageString(UINT id, CString& message) const;
	virtual void OnUpdateFrameTitle(BOOL add_to_title);
	void OnReBarChildSize(NMHDR* nmhdr, LRESULT* result);

	// get text displayed in status bar pane (no of images or scanning text)
	virtual CString GetStatusPaneText() const = 0;
	// return true is directory scanning is in progress
	virtual bool IsScanning() const = 0;

	// get text displayed in first status bar pane (instead of 'Ready' string)
	virtual CString GetStatusReadyText() const = 0;

	// retrieve current folder
	virtual FolderPathPtr GetCurrentFolder() = 0;

// Generated message map functions
protected:
	//{{AFX_MSG(MainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFavorites();
	afx_msg void OnAddToFavorites();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnUpdateFolderList(CCmdUI* cmd_ui);
	afx_msg void OnTaskTransfer();
	afx_msg void OnUpdateTaskTransfer(CCmdUI* cmd_ui);
	afx_msg void OnSize(UINT type, int cx, int cy);
	//afx_msg void OnHelpWindow();
	//afx_msg void OnUpdateHelpWindow(CCmdUI* cmd_ui);
	afx_msg LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//	void OnUpdateRecentPath(CCmdUI*);
	void OnFolderListMenu();
	void OnFavoriteFolder(UINT folder_id);
	void OnUpdateFavoriteFolder(CCmdUI*);
	void AddToFavorites(int folder_index);
	void OnUpdateStatusPaneImage(CCmdUI* cmd_ui);
	bool ShowScanningBar(bool show);
	CRect GetScanningWndRect();
	virtual void RecalcLayout(BOOL notify= TRUE);
	virtual void DrawSeparator(CDC& dc, const CRect& client);

private:
	CloseBar close_wnd_;			// close btn band (for full screen mode)
	bool full_screen_;
	DWORD style_;
	//CRect window_rect_;				// store window pos while in full screen mode
	WINDOWPLACEMENT normal_state_;

	void WndMode();
	void FullScreen();
	void OnFullScreen();
	void InformTaskBar();
	void ShowCloseBar(bool show);
	void OnUpdateFullScreen(CCmdUI* cmd_ui);
	afx_msg void OnCmdClose();
	afx_msg void OnRestore();

	void AddToFavoritesSystemFolders();

	void OnBuildCatalog();
	void OnMemoryStatus();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__7EF7528D_4A25_492F_AA0A_DB348E206AC0__INCLUDED_)
