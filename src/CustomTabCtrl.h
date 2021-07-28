#pragma once

/**********************************************************************
**
**	CustomTabCtrl.h : include file
**
**	by Andrzej Markowski June 2004
**
**********************************************************************/

#include <Afxtempl.h>
#include <afxcmn.h>


// hit test
#define CTCHT_ONFIRSTBUTTON		-1
#define CTCHT_ONPREVBUTTON		-2
#define CTCHT_ONNEXTBUTTON		-3
#define CTCHT_ONLASTBUTTON		-4
#define CTCHT_ONCLOSEBUTTON		-5
#define CTCHT_NOWHERE			-6

// CustomTabCtrlItem


#define MAX_LABEL_TEXT				100

struct CTC_NMHDR
{
	NMHDR hdr;
	int	item;
	TCHAR pszText[MAX_LABEL_TEXT+2];
	LPARAM lParam;
	RECT item_rect;
	POINT hit_test;
	bool selected;
	bool highlighted;
};


struct MY_MARGINS
{
	int cxLeftWidth;
	int cxRightWidth;
	int cyTopHeight;
	int cyBottomHeight;
};


class CCustomTabCtrlItem
{
	friend class CustomTabCtrl;
private:
								CCustomTabCtrlItem(CString text, LPARAM lParam);
	void						ComputeRgn(bool on_top);
	void						Draw(CDC& dc, CFont& font, bool selected, bool on_top, bool RTL, int selection_tinge, COLORREF tab_color, COLORREF text_color);
	bool						HitTest(CPoint pt)			{ return (shape_ && rgn_.PtInRegion(pt)) ? TRUE : FALSE; }
	void						GetRegionPoints(const CRect& rc, CPoint* pts, bool on_top) const;
	void						GetDrawPoints(const CRect& rc, CPoint* pts, bool on_top) const;
	void						operator=(const CCustomTabCtrlItem &other);
private:
	CString						text_;
	LPARAM						param_;
	CRect						rect_;
	CRect						text_rect_;
	CRgn						rgn_;			
	BYTE						shape_;
	bool						selected_;
	bool						highlighted_;
	bool						highlight_changed_;
};

// CustomTabCtrl

// styles
#define CTCS_FIXEDWIDTH			1		// Makes all tabs the same width.
#define CTCS_FOURBUTTONS		2		// Four buttons (First, Prev, Next, Last)
#define CTCS_AUTOHIDEBUTTONS	4		// Auto hide buttons
#define CTCS_TOOLTIPS			8		// Tooltips
#define CTCS_MULTIHIGHLIGHT		16		// Multi highlighted items
#define CTCS_EDITLABELS			32		// Allows item text to be edited in place
#define CTCS_DRAGMOVE			64		// Allows move items
#define CTCS_DRAGCOPY			128		// Allows copy items
#define CTCS_CLOSEBUTTON		256     // Close button
#define CTCS_BUTTONSAFTER		512		// Button after items
#define CTCS_TOP				1024    // Location on top
#define CTCS_RIGHT				2048    // Location on right
#define CTCS_LEFT				3072	// Location on left

// notification messages
#define CTCN_CLICK				NM_CLICK
#define CTCN_RCLICK				NM_RCLICK
#define CTCN_DBLCLK				NM_DBLCLK
#define CTCN_RDBLCLK			NM_RDBLCLK
#define CTCN_OUTOFMEMORY		NM_OUTOFMEMORY

#define CTCN_SELCHANGE			NM_FIRST
#define CTCN_HIGHLIGHTCHANGE	NM_FIRST + 1
#define CTCN_ITEMMOVE			NM_FIRST + 2
#define CTCN_ITEMCOPY			NM_FIRST + 3
#define CTCN_LABELUPDATE		NM_FIRST + 4

#define CTCID_FIRSTBUTTON		-1
#define CTCID_PREVBUTTON		-2
#define CTCID_NEXTBUTTON		-3	
#define CTCID_LASTBUTTON		-4
#define CTCID_CLOSEBUTTON		-5
#define CTCID_NOBUTTON			-6

#define CTCID_EDITCTRL			1

#define REPEAT_TIMEOUT			250

// error codes
#define CTCERR_NOERROR					0
#define CTCERR_OUTOFMEMORY				-1
#define CTCERR_INDEXOUTOFRANGE			-2
#define CTCERR_NOEDITLABELSTYLE			-3
#define CTCERR_NOMULTIHIGHLIGHTSTYLE	-4
#define CTCERR_ITEMNOTSELECTED			-5
#define CTCERR_ALREADYINEDITMODE		-6
#define CTCERR_TEXTTOOLONG				-7
#define CTCERR_NOTOOLTIPSSTYLE			-8
#define CTCERR_CREATETOOLTIPFAILED		-9
#define CTCERR_EDITNOTSUPPORTED			-10

// button states
#define BNST_INVISIBLE			0
#define BNST_NORMAL				DNHZS_NORMAL
#define BNST_HOT				DNHZS_HOT
#define BNST_PRESSED			DNHZS_PRESSED

#define CustomTabCtrl_CLASSNAME    _T("CustomTabCtrl")  // Window class name

class CustomTabCtrl : public CWnd
{
public:

	// Construction

	CustomTabCtrl();
	virtual						~CustomTabCtrl();
	bool						Create(UINT dwStyle, const CRect & rect, CWnd * parent_wnd, UINT id);

	// Attributes

	int							GetItemCount() { return static_cast<int>(a_items_.GetSize()); }
	int							GetCurSel() { return item_selected_; }
	int							SetCurSel(int item);
	int							IsItemHighlighted(int item);
	int							HighlightItem(int item, bool highlight);
	int							GetItemData(int item, DWORD& data);
	int							SetItemData(int item, DWORD data);
	int							GetItemText(int item, CString& text);
	int							SetItemText(int item, CString text);
	int							GetItemRect(int item, CRect& rect) const;
	int							SetItemTooltipText(int item, CString text);
	void						SetDragCursors(HCURSOR cursor_move, HCURSOR cursor_copy);
	bool						ModifyStyle(DWORD remove, DWORD add, UINT flags=0);
	bool						ModifyStyleEx(DWORD remove, DWORD add, UINT flags=0);
	void						SetControlFont(/*const LOGFONT& lf, */bool redraw=FALSE);
	//static const LOGFONT&		GetDefaultFont() {return lf_default;}
	bool						IsVertical() { return (GetStyle()&CTCS_TOP && GetStyle()&CTCS_RIGHT) || GetStyle()&CTCS_RIGHT;}
	void SetIdealHeight(int height);
	int GetIdealHeight() const;
	void SetTabColor(COLORREF tab, COLORREF backgnd, COLORREF text_color);

	// Operations

	int							InsertItem(int item, CString text, LPARAM lParam=0);
	int							DeleteItem(int item);
	void						DeleteAllItems();
	int							MoveItem(int item_src, int item_dst);
	int							CopyItem(int item_src, int item_dst);
	int							HitTest(CPoint pt);
	int							EditLabel(int item);

	void BlinkSelectedTab(int blinks);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CustomTabCtrl)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* msg);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CustomTabCtrl)
	afx_msg BOOL				OnEraseBkgnd(CDC* dc);
	afx_msg void				OnLButtonDown(UINT flags, CPoint point);
	afx_msg void				OnRButtonDown(UINT flags, CPoint point);
	afx_msg void				OnLButtonUp(UINT flags, CPoint point);
	afx_msg LRESULT				OnMouseLeave(WPARAM wParam, LPARAM lParam);
//	afx_msg LONG				OnThemeChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void				OnMouseMove(UINT flags, CPoint point);
	afx_msg void				OnPaint();
	afx_msg void				OnSize(UINT type, int cx, int cy);
	afx_msg void				OnLButtonDblClk(UINT flags, CPoint point);
	afx_msg void				OnTimer(UINT_PTR id_event);
	afx_msg void				OnUpdateEdit();
	afx_msg void				OnRButtonDblClk(UINT flags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT				OnSizeParent(WPARAM, LPARAM lParam);
	DECLARE_MESSAGE_MAP()	

private:
	void						RecalcLayout(int recalc_type,int item);
	void						RecalcEditResized(int offset, int item);
	void						RecalcOffset(int offset);
	int							RecalcRectangles();
	bool						RegisterWindowClass();
	int							ProcessLButtonDown(int hit_test, UINT flags, CPoint point);
	int							MoveItem(int item_src, int item_dst, bool mouse_sel);
	int							CopyItem(int item_src, int item_dst, bool mouse_sel);
	int							SetCurSel(int item, bool mouse_sel, bool ctrl_pressed);
	int							HighlightItem(int item, bool mouse_sel, bool ctrl_pressed);
	void						DrawGlyph(CDC& dc, CPoint& pt, int image_ndx, int color_ndx);
	void						DrawBk(CDC& dc, CRect& r, HBITMAP bmp, bool is_image_hor_layout, MY_MARGINS& mrgn, int image_ndx);
	bool						NotifyParent(UINT code, int item, CPoint pt);
	int							EditLabel(int item, bool mouse_sel);
	CRect						GetTabArea() const;
	void DrawCtrl(CDC& dc, int selection_tinge);

private:
	//static LOGFONT				lf_default;
	static BYTE					bits_glyphs_[];
	HCURSOR						cursor_move_;
	HCURSOR						cursor_copy_;
	CFont						font_;
	CFont						font_selected_;
	int							item_selected_;
	int							item_ndx_offset_;
	int							item_drag_dest_;
	int							prev_state_;
	int							next_state_;
	int							first_state_;
	int							last_state_;
	int							close_state_;
	int							button_id_down_;
	DWORD						last_repeat_time_;
	COLORREF					rgb_glyph_[4];
	CBitmap						glyphs_mono_bmp_;
	HBITMAP						bmp_bk_left_spin_;
	HBITMAP						bmp_bk_right_spin_;
	bool						is_left_image_hor_layout_;
	bool						is_right_image_hor_layout_;
	MY_MARGINS					mrgn_left_;
	MY_MARGINS					mrgn_right_;
	CToolTipCtrl				ctrl_tool_tip_;
	CEdit						ctrl_edit_;
	CArray<CCustomTabCtrlItem*, CCustomTabCtrlItem*>	a_items_;
	int divider_height_;
	int top_margin_;
	int bottom_margin_;
	int ideal_height_;
	COLORREF tab_color_;
	COLORREF text_color_;
	COLORREF backgnd_color_;
};
