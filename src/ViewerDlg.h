/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "ConnectionPoint.h"
#include "VectPhotoInfo.h"
class PhotoInfo;
class PhotoInfoStorage;
class ViewerDlgNotifications;
struct ICMTransform;
class Columns;

// ViewerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ViewerDlg dialog

class ViewerDlg : public CFrameWnd, public ConnectionPointer
{
public:
// Construction
	ViewerDlg(ConnectionPointer* link, VectPhotoInfo& photos, PhotoInfoStorage& storage,
		ViewerDlgNotifications* recipient, const Columns& columns);
	~ViewerDlg();

// Attributes
	int CurrentPhoto() const;

	void SetTransform(const ICMTransform& trans);

// Operations
	bool Create(const TCHAR* folder);

	void LoadPhoto(ConstPhotoInfoPtr photo);

	void Clean();		// inform viewer about removing list of photos

	void Synch();		// inform viewer about change in the list of photos

	// notification from main app: some photos were removed/added
	void PhotosListChange();

	void ResetSettings();	// call after settings have changed

	// set colors
	void SetColors(const std::vector<COLORREF>& colors);

	// change brightness of UI bitmap elements (toolbar, preview, light table)
	// using gamma correction 1.0 to 2.0 (1.0 - normal setting)
	void SetUIBrightness(double gamma);

	// default colors
	void ResetColors();

	// notification from list ctrl
	//void TagApplied(PhotoInfoPtr photo);

	// show tags bar
	void TurnTagsBarOn();

	void CustomColumnsRefresh();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewerDlg)
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
private:
	// Generated message map functions
	//{{AFX_MSG(ViewerDlg)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnViewerBar();
	afx_msg void OnFullScreen();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnUpdateFullScreen(CCmdUI* cmd_ui);
	afx_msg void OnUpdateViewerBar(CCmdUI* cmd_ui);
	afx_msg void OnPhotoFirst();
	afx_msg void OnPhotoLast();
	afx_msg void OnPhotoNext();
	afx_msg void OnUpdatePhotoNext(CCmdUI* cmd_ui);
	afx_msg void OnPhotoPrev();
	afx_msg void OnUpdatePhotoPrev(CCmdUI* cmd_ui);
	afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
	afx_msg void OnPhotoList();
	afx_msg void OnZoomIn();
	afx_msg void OnUpdateZoomIn(CCmdUI* cmd_ui);
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateZoomOut(CCmdUI* cmd_ui);
	afx_msg void OnZoomFit();
	afx_msg void OnZoom100();
	afx_msg void OnPhotoDesc();
	afx_msg void OnDestroy();
	afx_msg void OnNcDestroy();
	afx_msg void OnSysCommand(UINT id, LPARAM lParam);
	afx_msg void OnCmdClose();
	afx_msg void OnRestore();
	afx_msg void OnGamma();
	afx_msg void OnUpdateGamma(CCmdUI* cmd_ui);
	afx_msg void OnViewerBalloons();
	afx_msg void OnUpdateViewerBalloons(CCmdUI* cmd_ui);
	afx_msg void OnViewerToolbar();
	afx_msg void OnUpdateViewerToolbar(CCmdUI* cmd_ui);
	void OnViewerInfobar();
	void OnUpdateViewerInfobar(CCmdUI* cmd_ui);
	afx_msg void OnDelete();
	afx_msg void OnUpdateDelete(CCmdUI* cmd_ui);
	afx_msg void OnUpdateZoom100(CCmdUI* cmd_ui);
	afx_msg void OnUpdateZoomFit(CCmdUI* cmd_ui);
	afx_msg void OnStartSlideShow();
	afx_msg void OnUpdateStartSlideShow(CCmdUI* cmd_ui);
	afx_msg void OnStopSlideShow();
	afx_msg void OnUpdateStopSlideShow(CCmdUI* cmd_ui);
	afx_msg void OnTagsBar();
	afx_msg void OnUpdateTagsBar(CCmdUI* cmd_ui);
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnContextMenu(CWnd* wnd, CPoint point);
	afx_msg void OnUpdatePhotoFirst(CCmdUI* cmd_ui);
	afx_msg void OnUpdatePhotoLast(CCmdUI* cmd_ui);
	afx_msg void OnUpdatePhotoList(CCmdUI* cmd_ui);
	afx_msg void OnSlideShowParams();
	afx_msg void OnUpdateSlideShowParams(CCmdUI* cmd_ui);
	afx_msg void OnRotate90CW();
	afx_msg void OnUpdateRotate90CW(CCmdUI* cmd_ui);
	afx_msg void OnRotate90CCW();
	afx_msg void OnUpdateRotate90CCW(CCmdUI* cmd_ui);
	afx_msg void OnJpegAutoRotate();
	afx_msg void OnUpdateJpegAutoRotate(CCmdUI* cmd_ui);
	afx_msg void OnJpegRotate90CCW();
	afx_msg void OnUpdateJpegRotate90CCW(CCmdUI* cmd_ui);
	afx_msg void OnJpegRotate90CW();
	afx_msg void OnUpdateJpegRotate90CW(CCmdUI* cmd_ui);
	afx_msg void OnToggleLightTable();
	afx_msg void OnUpdateToggleLightTable(CCmdUI* cmd_ui);
	afx_msg void OnAddToLightTable();
	afx_msg void OnRemoveFromLightTable();
	afx_msg void OnUpdateAddToLightTable(CCmdUI* cmd_ui);
	afx_msg void OnUpdateRemoveFromLightTable(CCmdUI* cmd_ui);
	afx_msg void OnMagnifierLens();
	afx_msg void OnUpdateMagnifierLens(CCmdUI* cmd_ui);
	void OnUpdateLightTableTags(CCmdUI* cmd_ui);
	//}}AFX_MSG
	void OnUpdateEnableLabels(CCmdUI* cmd_ui);
	void OnChangePreviewLabels(UINT cmd_id);
	DECLARE_MESSAGE_MAP()

	void OnHideLightTable();
	void OnLightTableDeleteAllOthers();
	afx_msg void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnUpdateLightTableOperations(CCmdUI* cmd_ui);
	virtual void RecalcLayout(BOOL notify= TRUE);
	afx_msg void OnLightTableCopy();
	void OnLightTableHTML();
	afx_msg void OnLightTableSelect();
	afx_msg void OnLightTableResize();
	afx_msg void OnLightTableEmpty();
	afx_msg void OnTimer(UINT_PTR id_event);
	afx_msg void OnViewerOptions();
	afx_msg void OnUpdateViewerOptions(CCmdUI* cmd_ui);
	void OnTagSelected(UINT cmd);
	void OnUpdateTags(CCmdUI* cmd_ui);
	afx_msg void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	afx_msg void OnToggleDispDescription();
	afx_msg void OnUpdateToggleDispDescription(CCmdUI* cmd_ui);
	void OnUseScrollBars();
	void OnUpdateUseScrollBars(CCmdUI* cmd_ui);
	void OnHorzPreviewBar();
	void OnVertPreviewBar();
	void OnUpdateHorzPreviewBar(CCmdUI* cmd_ui);
	void OnUpdateVertPreviewBar(CCmdUI* cmd_ui);
	void OnToggleTagsInPreviewBar();
	void OnUpdateToggleTagsInPreviewBar(CCmdUI* cmd_ui);
	LRESULT OnAppCommand(WPARAM w, LPARAM l);
	void OnOpenPhoto();
	void OnToggleSmoothScroll();
	void OnUpdateToggleSmoothScroll(CCmdUI* cmd_ui);
	void OnToggleMultiView();
	void OnUpdateMultiView(CCmdUI* cmd_ui);
	void OnToggleMultiViewLayout();
	void OnClose();
	LRESULT OnCloseApp(WPARAM, LPARAM);
	void OnMoveLeft();
	void OnMoveRight();
	void OnMoveUp();
	void OnMoveDown();
	void OnSwitchViews();
	void OnRateImage(UINT cmd);
	void OnUpdateRates(CCmdUI* cmd_ui);
	void OnTurnMultiPane(UINT cmd);
	void OnUpdateMultiPane(CCmdUI* cmd_ui);
	void OnChangeMultiPaneLayout(UINT cmd);
	void OnUpdateMultiPaneLayout(CCmdUI* cmd_ui);
	void OnActivateViewPane(UINT cmd);
//	void OnUpdateActivateViewPane(CCmdUI* cmd_ui);
	void OnRenamePhoto();
	void OnUpdatePrint(CCmdUI* cmd_ui);
	void OnPrint();
	void OnSetAsWallpaper();
	void OnUpdateSetAsWallpaper(CCmdUI* cmd);
	void OnToggleCenterImage();
	void OnUpdateToggleCenterImage(CCmdUI* cmd);
	void OnMButtonDown(UINT flags, CPoint point);
	void OnHideTagPane();

private:
	//void OnSmallIcons();
	//void OnLargeIcons();
	//void OnUpdateSmallIcons(CCmdUI* cmd_ui);
	//void OnUpdateLargeIcons(CCmdUI* cmd_ui);

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};
