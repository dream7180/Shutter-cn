/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
// InfoPane.h : header file
//
#include "ToolBarWnd.h"
#include "InfoDisplay.h"
#include "Profile.h"
#include "Pane.h"
class PhotoInfo;
#include "InfoPaneBar.h"
#include "OutputStr.h"



class InfoPane : public PaneWnd
{
// Construction
public:
	InfoPane();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

	// display photo info
	void UpdateInfo(PhotoInfoPtr photo, bool force= false);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(InfoPane)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~InfoPane();

	virtual BOOL IsFrameWnd() const;

	virtual void CurrentChanged(PhotoInfoPtr photo);
	//virtual void CaptionHeightChanged(bool big);
	virtual void CurrentModified(PhotoInfoPtr photo);

	// Generated message map functions
protected:
	//{{AFX_MSG(InfoPane)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnRawInfo();
	afx_msg void OnUpdateRawInfo(CCmdUI* cmd_ui);
	afx_msg void OnCopyInfo();
	afx_msg void OnUpdateCopyInfo(CCmdUI* cmd_ui);
	afx_msg void OnExportExif();
	afx_msg void OnUpdateExportExif(CCmdUI* cmd_ui);
	afx_msg void OnTogglePreview();
	afx_msg void OnUpdateTogglePreview(CCmdUI* cmd_ui);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);

private:
	static CString wnd_class_;
	ToolBarWnd tool_bar_wnd_;
	bool raw_data_;
	bool hide_unknown_;
	InfoDisplay disp_wnd_;
	InfoPaneBar bar_;
	std::vector<int> selected_lines_;
	void Filter(const CString& text, bool hide_unknown);

	void OnCalcSize(NMHDR* hdr, LRESULT* result);
	void ResetText();
	String GetText() const;
	void GetText(int line, int col, TCHAR* text, int max_len);

	void Resize();
	bool img_preview_;
	OutputStr file_info_;
	OutputStr* info_;
	PhotoInfoPtr photo_;
	CRect GetPreviewRect();

	Profile<bool> profile_show_preview_;
	Profile<bool> profile_raw_info_;
	Profile<bool> profile_hide_unknown_;

	void GetNameText(int line, TCHAR* text, int max_len);
	void GetValueText(int line, TCHAR* text, int max_len);
	void GetLineText(int line, int col, TCHAR* text, int max_len);

	virtual void OptionsChanged(OptionsDlg& dlg);
	void SetColors();
};
