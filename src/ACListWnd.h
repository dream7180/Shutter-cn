#pragma once
#include <boost/function.hpp>

/*********************************************************************
*
* AutoCompletePopup
* Copyright (c) 2002 by Andreas Kapust
* All rights reserved.
* info@akinstaller.de
*
* MiK: massive cleanup
*
*********************************************************************/


class AutoCompletePopup : public CWnd
{
public:
	AutoCompletePopup();

	bool Create();

	void ControlEditBox(CEdit* ctrl, const std::vector<String>* history);

	CEdit* CurrentEditCtrl() const;

	void ShowList();

	bool EnsureVisible(int item, bool wait_);
	bool SelectItem(int item);
	int FindString(int start_after, LPCTSTR string);
	int FindStringExact(int index_start, LPCTSTR find);
	int SelectString(LPCTSTR string);

	void RegisterTextRemoveFn(const boost::function<void (const String& text)>& remove);

	virtual ~AutoCompletePopup();

	void AlignToParent(bool enable);
	void AutoPopup(bool enable);
	void AutoUnhook(bool enable);
	void HandleUpDownKey(bool enable);

	void RegisterTextSelectedFn(const boost::function<void ()>& fn);

protected:
	//{{AFX_MSG(AutoCompletePopup)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnNcPaint();
	afx_msg void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	afx_msg void OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnActivateApp(BOOL active, DWORD task);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnRButtonDown(UINT flags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	afx_msg void OnShowWindow(BOOL show, UINT status);
	afx_msg void OnNcLButtonDown(UINT hit_test, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnTimer(UINT_PTR event_id);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* MMI);
	//}}AFX_MSG
	void OnSizing(UINT type, RECT* rect);
	DECLARE_MESSAGE_MAP()

private:
	enum { IDTimerInstall= 10 };
	std::vector<String> search_list_;
	std::vector<String*> display_list_;
	CScrollBar vert_scrollbar_;
	CRect parent_rect_;
	CSize last_size_;
	CFont* font_dc;
	CFont fontDC;//, boldFontDC;
	CEdit* edit_ctrl_;
	LOGFONT logfont;
	bool in_update_;
	CString user_text_;
	boost::function<void (const String& text)> remove_text_;
	boost::function<void ()> text_selected_;
	bool align_to_parent_;
	bool auto_popup_;
	bool auto_unhook_;
	bool handle_up_down_key_;

	UINT_PTR id_timer_;
	int top_index_;
	int cur_item_;
	int item_height_;
	int visible_items_;
	int selected_item_;

	int HitTest(CPoint point);
	void SetScroller();
	void SetProp();
	long ScrollBarWidth();
	void InvalidateAndScroll();
	void SortList(std::vector<String>& list_);
	static int CompareString(const void* p1, const void* p2);
	bool HandleKey(WPARAM chr);
	static LRESULT CtrlWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT ParentWindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LONG_PTR old_wnd_proc_;
	LONG_PTR old_parent_wnd_proc_;
	LONG_PTR parent_old_wnd_proc_;
	void Hook(CEdit* edit);
	void Unhook();
	void EditChange();
	void SetNextString(int key);
	void Init();
	const String* GetString() const;
	void SelectAndHide();
	void DrawItem(CDC* dc, long item_, long width);
	void ShowString(const std::vector<String>& strings, int item);
	void ShowAll();
	void PopupList(bool resetCurSelection);
};
