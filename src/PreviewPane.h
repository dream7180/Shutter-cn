/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PREVIEWPANE_H__B14FC273_1310_4BAC_826D_F8054BB4C731__INCLUDED_)
#define AFX_PREVIEWPANE_H__B14FC273_1310_4BAC_826D_F8054BB4C731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewPane.h : header file
//
#include "ViewPane.h"
#include "PreviewBar.h"
#include "Pane.h"
#include "PhotoCollection.h"

/////////////////////////////////////////////////////////////////////////////
// PreviewPane window

class PreviewPane : public PaneWnd, ViewPaneNotifications
{
// Construction
public:
	PreviewPane();

// Attributes
public:
	void SetBkColor(COLORREF rgb_back);
	void SetTextColor(COLORREF rgb_text);

//	void SetTransform(ICMTransformPtr transform)	{ transform_ = transform; }

// Operations
public:
	bool Create(CWnd* parent);
	void SliderChanged(int pos);		// notification from zoom slider

	void LoadPhoto(const PhotoInfo& inf, bool force_reload= false);

	void Clear()						{ display_.Reset(); }

	void ResetDescription(const std::wstring& desc)
	{ display_.ResetDescription(desc); display_.Invalidate(); }

	void ResetDescriptionFont()			{ display_.ResetDescriptionFont(); }

	void ResetGamma();

	// set default colors
	void ResetColors();
	// set given colors
	void SetColors(const std::vector<COLORREF>& colors);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PreviewPane)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PreviewPane();
	virtual BOOL IsFrameWnd() const;

	// Generated message map functions
protected:
	//{{AFX_MSG(PreviewPane)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnZoom100();
	afx_msg void OnUpdateZoom100(CCmdUI* cmd_ui);
	afx_msg void OnZoomFit();
	afx_msg void OnUpdateZoomFit(CCmdUI* cmd_ui);
	afx_msg void OnZoomIn();
	afx_msg void OnUpdateZoomIn(CCmdUI* cmd_ui);
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateZoomOut(CCmdUI* cmd_ui);
	afx_msg void OnPreviewOptions();
	afx_msg void OnUpdatePreviewOptions(CCmdUI* cmd_ui);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR id_event);
	afx_msg void OnMagnifierLens();
	afx_msg void OnUpdateMagnifierLens(CCmdUI* cmd_ui);
	afx_msg void OnJpegRotate90CCW();
	afx_msg void OnJpegRotate90CW();
	afx_msg void OnUpdateJpegRotate(CCmdUI* cmd_ui);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	LRESULT OnDblClick(WPARAM, LPARAM);
	void OnContextMenu(CWnd* wnd, CPoint pos);
	void OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu);
	void OnToggleDispDescription();
	void OnUpdateToggleDispDescription(CCmdUI* cmd_ui);
	void OnSetAsWallpaper();
	void OnUpdateSetAsWallpaper(CCmdUI* cmd);

private:
	ViewPane display_;
	PreviewBar bar_wnd_;
	UINT_PTR timer_id_;					// delay timer
	bool show_magnifier_lens_;
	ICMTransform transform_;
	auto_connection metadata_changed_;
	bool rotation_feasible_;

	void Resize();
	void SetSlider();
	void SetColors();
	void RefreshAfterChange(const VectPhotoInfo& photos);
	void RotatePhoto(CWnd* wnd, RotationTransformation transform);

	// notifications
	virtual void OptionsChanged(OptionsDlg& dlg);
	virtual void PaneHidden();
	virtual void CurrentChanged(PhotoInfoPtr photo);
	virtual void CurrentModified(PhotoInfoPtr photo);
	virtual void CaptionHeightChanged(bool big);
	virtual void PhotoDescriptionChanged(std::wstring& descr);
	virtual void UpdatePane();

	// notifications from ViewPane
	virtual void DecodingStarted(ViewPane* view);
	virtual void DecodingFinished(ViewPane* view);
	virtual void DecodingThreadFinished(ViewPane* view);
	virtual void MiddleButtonDown(CPoint pos);
	virtual void AnimationStarts();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWPANE_H__B14FC273_1310_4BAC_826D_F8054BB4C731__INCLUDED_)
