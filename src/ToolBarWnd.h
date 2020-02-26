/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_TOOLBARWND_H__7B2D6D9C_16E2_4045_AD73_5A7ABA9AFA2A__INCLUDED_)
#define AFX_TOOLBARWND_H__7B2D6D9C_16E2_4045_AD73_5A7ABA9AFA2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ToolBarWnd window

class ToolBarWnd : public CToolBarCtrl
{
// Construction
public:
	ToolBarWnd();

// Attributes
public:
	void SetOnIdleUpdateState(bool enabled);
	void SetPadding(int cx, int cy);	// set button pad (before creation)
	void SetPadding(CSize pad);
	void SetDefaultFontFlag(bool enabled);

// Operations
public:
	bool Create(const char* a_template, int rsrc_id, int str_id, CWnd* parent, int id= -1);
	bool Create(const char* a_template, const int commands[], int rsrc_id, int str_rsrc_id, CWnd* parent, int id= -1, bool vertical= false);

	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* parent_wnd, UINT id)
	{ return CToolBarCtrl::Create(dwStyle, rect, parent_wnd, id); }

	bool AddButtons(const char* a_template, const int commands[], int bmp_id, int str_id= 0, bool vertical= false);

	void ShiftImageForCheckedBtn(int shift_btn_cmd_id)		{ shift_image_for_checked_btn_ = shift_btn_cmd_id; }

	void SetHotImageList(int hot_bmp_id);
	void SetDisabledImageList(int disabled_bmp_id);
	bool CreateDisabledImageList(int bmp_id, float saturation= -0.3f, float lightness= +0.15f, float alpha= 1.0f);

	void SetOwnerDraw(bool owner_draw)		{ owner_draw_ = owner_draw; }

	void SetScrollInfoReceiver(CWnd* wnd)	{ pass_scroll_info_ = wnd; }

	bool DeleteButton(int command_id);

	void SaveState(const TCHAR* sub_key, const TCHAR* value_name);
	void RestoreState(const TCHAR* sub_key, const TCHAR* value_name);

	bool AddButtons(int count, int first_command, const TCHAR* text, BYTE btn_style= BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT);

	void AutoResize();

	virtual void ResetToolBar(bool resize_to_fit);

	void HideButtonIdx(int index, bool hide);

	void Destroy();

	virtual CString GetToolTip(int cmd_id);

	// replace current image list
	enum ImgListFlag { NORMAL, HOT, DISABLED };
	bool ReplaceImageList(int new_bmp_id, int btn_count= 0, ImgListFlag list= NORMAL);

	void SetButtonText(int index, const TCHAR* text);

	void HandleTooltips(bool enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ToolBarWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ToolBarWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(ToolBarWnd)
	afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	void OnUpdateCmdUI(CFrameWnd* target, bool disable_if_no_hndler);
	BOOL OnGetInfoTip(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnQueryInsert(NMHDR * notify_struct, LRESULT* result);
	afx_msg void OnQueryDelete(NMHDR * notify_struct, LRESULT* result);
	afx_msg void OnCustomDraw(NMHDR* nm_hdr, LRESULT* result);
	afx_msg void OnGetDispInfo(NMHDR * notify_struct, LRESULT* result);
	afx_msg void OnGetButtonInfo(NMHDR* notify_struct, LRESULT* result);
	afx_msg void OnBeginAdjust(NMHDR* notify_struct, LRESULT* result);
	afx_msg void OnReset(NMHDR* notify_struct, LRESULT* result);

	CSize LoadImageList(int bmp_id, int btn_count, CImageList& img_list);

	int shift_image_for_checked_btn_;
	bool on_idle_update_state_;
	bool set_default_font_;
	CImageList image_list_;
	CImageList image_list_hot_;
	CImageList image_list_disabled_;
	CSize pad_size_;
	CSize bmp_btn_size_;
	int image_count_;
	bool owner_draw_;
	CWnd* pass_scroll_info_;
	std::vector<TBBUTTON> buttons_;
	bool handle_tooltips_;

	friend class xCToolCmdUI;

	void OnClick(NMHDR* notify_struct, LRESULT* result);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARWND_H__7B2D6D9C_16E2_4045_AD73_5A7ABA9AFA2A__INCLUDED_)
