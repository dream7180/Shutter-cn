/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_CAPTIONWINDOW_H__3F0C293A_18E1_45A9_88F7_D7718133156C__INCLUDED_)
#define AFX_CAPTIONWINDOW_H__3F0C293A_18E1_45A9_88F7_D7718133156C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaptionWindow.h : header file
//
class SnapFrame;
class SnapView;
class ColorConfiguration;
#include "../ToolBarWnd.h"


/////////////////////////////////////////////////////////////////////////////
// CaptionWindow window

class CaptionWindow : public CWnd
{
// Construction
public:
	CaptionWindow();

// Attributes
public:
	static int GetHeight();

	//static bool IsCaptionBig()	{ return big_; }

	void ModifyToolbar(bool wnd_maximized);

	// tab and text colors if different from the default ones
	void SetTabColors(const ColorConfiguration& colors);

	// modify colors of static image lists used by all caption windows to draw title
	//static void ReinitializeImageLists(COLORREF active_color, COLORREF inactive_color);

	// normal bottom edge shade (false), or a faint one (true)
	void SetBottomEdge(bool faint);

	bool HasMaximizeButton() const;

// Operations
public:
	bool Create(CWnd /*SnapView*/ * parent, const TCHAR* title);

	void SetPosition(const CRect& rect);

	void Activate(bool active);

	void SetFrame(SnapFrame* frame)		{ frame_ = frame; }

	void SetTitle(const TCHAR* title);

	void AddBand(CWnd* toolbar, CWnd* owner, std::pair<int, int> pairMinMaxWidth, bool resizable);

	void ResetBandsWidth(std::pair<int, int> pairMinMaxWidth);
	void ResetBandsWidth();

	//	void SetToolbarOwner(CWnd* parent);

	// remove close btn
	void HideCloseButton();
	// remove maximize btn
	void HideMaximizeButton();

	// set large caption bitmap
	//static void SetLargeCaption(bool large)		{ big_ = large; }

	// set proper bmp for toolbar (close, maximize, restore buttons)
	//void ChangeHeight(bool big);

	// erase background
	void EraseBackground(CDC& dc, const CRect& rect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CaptionWindow)
	//}}AFX_VIRTUAL

// Implementation
public:
	//virtual ~CaptionWindow();

	// Generated message map functions
protected:
	//{{AFX_MSG(CaptionWindow)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
	afx_msg void OnRButtonDown(UINT flags, CPoint point);
	afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnChevron();
	//}}AFX_MSG
//	void OnSmallIcons();
//	void OnLargeIcons();
	DECLARE_MESSAGE_MAP()
private:
	static CString wnd_class_;
	//static CFont bold_fnt_;

	ToolBarWnd	tool_bar_wnd_;
	ToolBarWnd	chevron_wnd_;
	bool		no_close_btn_;
	bool		no_maximize_btn_;
	CBitmap		toolbar1_bmp_;
	CBitmap		toolbar2_bmp_;
	bool		active_;
	bool		dragging_;
	CRect		trace_rect_;
	SnapFrame*	frame_;
	SnapView*	parent_;
	CPoint		start_;
	bool		block_dragging_;
	CWnd*		hosted_bar_;
	CWnd*		hosted_bar_owner_;
	bool		hosted_bar_resizable_;
	std::pair<int, int> pair_hosted_bar_min_max_width_;
	CRect		hosted_bar_rect_;
	bool		show_context_menu_;
	bool		faint_bottom_edge_;
	bool		maximized_wnd_toolbar_;

	// (re)initialize static image lists used by all caption windows
	//static void InitializeImageLists(COLORREF active_color, COLORREF inactive_color);
	void	DrawCaptionBar(CDC* dc, CRect& rect, const TCHAR* title, bool is_active);
	void	DrawTab(CDC* dc, int image, const CRect& rect, int text_width, COLORREF base_color);
	int		GetTollBarWidth();
	static int CreateImgList(CImageList& img_list_tab, int tab_bmp_id, int shadow_bmp_id, int parts, COLORREF active_color, COLORREF inactive_color);
	void	PositionHostedBar();
	CRect	GetHostedBarRect();
	void	CreateChevron();
	void	SetChevronPos();
	void	Resize();
	CRect	GetTextRect(CDC* dc, const CRect& rect, const TCHAR* title, CSize& text_size);
	//void	OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu);
	void	OnContextMenu(CWnd* wnd, CPoint point);
	void	CreateToolbar();
	void	SetCursor();
	void	DrawCaptionGradient(CDC* dc, const CRect& rect);

	//static bool big_;		// big caption or normal one

	//CImageList& GetTabImg();
	//CImageList& GetTabEnd();
	//int GetTabPartWidth();
	//int GetTabEndWidth();
	//int GetMinCaptionWidth();

	static int initialized_;

	//static CImageList img_list_tab_;
	//static CImageList img_list_tab_end_;
	//static int tab_part_width_;
	//static int tab_end_width_;
	//static int min_caption_width_;

	//static const int tab_part_divider_;

	//static CImageList img_list_tab_big_;
	//static CImageList img_list_tab_end_big_;
	//static int tab_part_width_big_;
	//static int tab_end_width_big_;
	//static int min_caption_width_big_;
	//static COLORREF static_active_caption_color;
	//static COLORREF static_inactive_caption_color;

	LRESULT OnPrint(WPARAM hdc, LPARAM flags);

	// local image lists used only when color style differs from the global one
	//CImageList tab_imglist_;
	//CImageList tab_end_imglist_;
	//bool use_local_colors_;
	COLORREF active_caption_color_;
	COLORREF inactive_caption_color_;
	bool use_inactive_caption_colors_;
	COLORREF active_caption_text_color_;
	COLORREF inactive_caption_text_color_;
	COLORREF bar_color_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAPTIONWINDOW_H__3F0C293A_18E1_45A9_88F7_D7718133156C__INCLUDED_)
