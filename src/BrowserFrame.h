/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ChildFrm.h : interface of the BrowserFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__9FFD901D_57FE_42A0_9AB0_33DF38CAF347__INCLUDED_)
#define AFX_CHILDFRM_H__9FFD901D_57FE_42A0_9AB0_33DF38CAF347__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BrowserToolbar.h"
#include "InfoPane.h"
#include "FolderPane.h"
#include "HorzReBar.h"
#include "AddressBox.h"
#include "FilterBar.h"
#include "TagsBar.h"
#include "MainFrm.h"
#include "MenuBar.h"
#include "SnapFrame/SnapFrame.h"
#include "PreviewPane.h"
#include "ExifProView.h"
#include "HistogramPane.h"
#include "TagBarView.h"
#include "FolderPath.h"
#include "TaskToolbar.h"
class ExifView;
class PhotoInfo;
class ExifStatusBar;
class CMenuFolders;
class ApplicationColors;


class BrowserFrame : public MainFrame
{
	DECLARE_DYNCREATE(BrowserFrame)
public:
	BrowserFrame();

// Attributes
public:
	virtual CString GetStatusPaneText() const;
	virtual bool IsScanning() const;

	const TCHAR* GetRegSection()		{ return reg_section_; }

	// get text displayed in first status bar pane (instead of 'Ready' string)
	virtual CString GetStatusReadyText() const;

	// returns true if photos are filtered
	bool IsFilterActive() const;

	void SetRecursiveScan(bool recursive);

	void SetReadOnlyExif(bool exif_only);

	ExifStatusBar& GetStatusBar();

	HMENU GetMenu()						{ return menu_bar_wnd_.GetMenu(); }

// Operations
public:
	// call after selected photo(s) has changed (update tags window)
	void SelectionChanged(PhotoInfoPtr photo);
	void SelectionChanged(VectPhotoInfo& selected);

	// call after current photo has changed (update status bar)
	void CurrentChanged(PhotoInfoPtr photo, bool has_selection);

	// new folder selected; refresh view
	void FolderSelectedEx(FolderPathPtr path);
	bool FolderSelected(const TCHAR* path);
	bool FolderSelected(FolderPathPtr path);

	// re-read photos from current folder
	void RefreshView();

	// display only photos with given text in description (pass null to cancel filtering)
	void FilterPhotos(const TCHAR* text);

	// select favorite folder
	bool FavoriteFolder(int index);

	// display browser
	void Browser();
	void Browser(FolderPathPtr path, bool rootDir);

	void RefreshStatusBar();

	// open options dlg
	void OptionsDlg(int page);

	void InitialSetActiveView();

	// description has been changed
	void PhotoDescriptionChanged(std::wstring& descr);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BrowserFrame)
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* context);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int cmd_show = -1);
	virtual BOOL OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info);
	virtual BOOL PreTranslateMessage(MSG* msg);
	//}}AFX_VIRTUAL

	int last_folder_;				// last selected favorite folder or (-1)

	FolderPathPtr GetLastUsedFolder() const;

	void EnableSavingSettings(bool enable= true);

// Implementation
public:
	virtual ~BrowserFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual FolderPathPtr GetCurrentFolder();

protected:
	HorzReBar		rebar_wnd_;
	CMenuBar		menu_bar_wnd_;
	BrowserToolbar	toolbar_wnd_;
	AddressBox		addr_box_wnd_;
	FilterBar		filter_bar_wnd_;
	InfoPane		info_bar_wnd_;
	FolderPane		folders_wnd_;
	PreviewPane		preview_wnd_;
	ExifView		exif_view_wnd_;
	HistogramPane	histogram_wnd_;
	TagBarView		tag_bar_wnd_;
	CString			folder_name_;
	CString			reg_section_;
	enum { BAND_MENU= 5, BAND_TOOLBAR, BAND_FILTER, BAND_ADDRESS };
	SnapFrame		frame_wnd_;

// Generated message map functions
protected:
	//{{AFX_MSG(BrowserFrame)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnInfoBar();
	afx_msg void OnUpdateInfoBar(CCmdUI* cmd_ui);
	afx_msg void OnFolders();
	afx_msg void OnUpdateFolders(CCmdUI* cmd_ui);
	afx_msg void OnDestroy();
	afx_msg void OnPreview();
	afx_msg void OnUpdatePreview(CCmdUI* cmd_ui);
	afx_msg void OnBrowser();
	afx_msg void OnUpdateBrowser(CCmdUI* cmd_ui);
	afx_msg void OnUpdateEnter(CCmdUI* cmd_ui);
	afx_msg void OnUpdateNextPane(CCmdUI* cmd_ui);
	afx_msg void OnNextPane();
	void OnPreviousPane();
	afx_msg void OnTagsBar();
	afx_msg void OnUpdateTagsBar(CCmdUI* cmd_ui);
	afx_msg void OnDropFiles(HDROP drop_info);
	afx_msg void OnViewToolbar();
	afx_msg void OnUpdateViewToolbar(CCmdUI* cmd_ui);
	afx_msg void OnViewTools();
	afx_msg void OnUpdateViewTools(CCmdUI* cmd_ui);
	afx_msg void OnViewAddressbar();
	afx_msg void OnUpdateViewAddressbar(CCmdUI* cmd_ui);
	afx_msg void OnViewFilterbar();
	afx_msg void OnUpdateViewFilterbar(CCmdUI* cmd_ui);
	afx_msg void OnPathTyped();
	afx_msg void OnAddressBar();
	afx_msg void OnUpdateAddressBar(CCmdUI* cmd_ui);
	afx_msg void OnPathCancelled();
	afx_msg void OnFindBox();
	afx_msg void OnFileMask();
	afx_msg void OnUpdateFileMask(CCmdUI* cmd_ui);
	afx_msg void OnOptions();
	afx_msg void OnUpdateOptions(CCmdUI* cmd_ui);
	afx_msg void OnUpdatePaneMenu(CCmdUI* cmd_ui);
	afx_msg void OnPaneZoom();
	afx_msg void OnUpdatePaneZoom(CCmdUI* cmd_ui);
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnPanesManageLayouts();
	afx_msg void OnUpdatePanesManageLayouts(CCmdUI* cmd_ui);
	afx_msg void OnPanesStoreLayout();
	afx_msg void OnUpdatePanesStoreLayout(CCmdUI* cmd_ui);
	afx_msg void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	afx_msg void OnSize(UINT type, int cx, int cy);
	//}}AFX_MSG
//	afx_msg void OnUpdateViewStyles(CCmdUI* cmd_ui);
//	afx_msg void OnViewStyle(UINT command_id);
	DECLARE_MESSAGE_MAP()
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnFolderList();
	void OnFolderListMenu();
	//void OnBrowserPopupMenu();
	void OnFavoriteFolder(UINT folder_id);
	void OnUpdateFavoriteFolder(CCmdUI* cmd_ui);
	void OnPanePopupMenu();
	void OnDefineCustomColumns();
	void OnUpdateDefineCustomColumns(CCmdUI* cmd_ui);
	void OnFilterPhotos();
	void OnCancelFilter();
	void OnLoadAllTypes();
	void OnLoadJpegOnly();
	void OnLoadRawOnly();

private:
//	void EnableDockingEx(DWORD dock_style);
	void OnViewModePopupMenu();
	void OnViewSortPopupMenu();
	void OnFileMaskPopupMenu();
	void OnFileMask(UINT folder_id);
	void OnUpdateFileMaskRng(CCmdUI* cmd_ui);
	void OnRestorePaneLayout(UINT pane_layout_id);
	void OnUpdateRestorePaneLayout(CCmdUI* cmd_ui);
	void RecentFolders(CPoint pt);
	void EnableRecentFolder(CCmdUI* cmd_ui);
	void FolderUp();
	bool UpdateFolderUp();
	void DefineCustomColumns(CWnd* parent);

//	static DWORD dwxDockBarMap[4][2];
	virtual void DrawSeparator(CDC& dc, const CRect& client);

	void TogglePane(CWnd& wnd);
	bool IsPaneOpen(CWnd& wnd);

	bool CreateWindows();
	void BuildFolderListMenu(CMenuFolders& menu);

	std::auto_ptr<CMenuFolders> folders_popup_;
	CMenu menu_sort_order_popup_;
	CMenu menu_pane_windows_popup_;
	afx_msg void OnMeasureItem(int id_ctl, LPMEASUREITEMSTRUCT measure_item_struct);
	void OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu);

	afx_msg void EnableSortingOption(CCmdUI* cmd_ui);
	void OnSortingOption(UINT sorting_option);

	void OnClose();
	LRESULT OnCloseApp(WPARAM, LPARAM);

	void NextPane(bool next);

	FolderPathPtr last_used_path_;
	bool initialized_;
	bool save_settings_;

	TaskToolbar toolbar_;
	CSize toolbar_size_;
	Profile<bool> tools_visible_;
	Profile<bool> tools_horizontal_;
	void PlaceToolbar();

	void OnBuildImgCatalog();	// this cmd invoked from the folder pane

	void OnReloadProgress(ReloadJob::Progress progress, size_t images);
	void OnRunCommand(int cmd);
	void OnFilterStateChanged(bool active);

	void OnToolbarHorizontal();
	void OnToolbarVertical();
	void SetTaskBarHorzOrientation(bool horz);
	void RepositionTaskBar();
	void SetColors(const ApplicationColors& colors);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__9FFD901D_57FE_42A0_9AB0_33DF38CAF347__INCLUDED_)
