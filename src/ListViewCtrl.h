/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class ListViewCtrl;


class ListViewCtrlNotifications
{
public:
	virtual void ItemClicked(ListViewCtrl& ctrl, int index, size_t param) = 0;
	virtual int GetItemCheckState(ListViewCtrl& ctrl, int index, size_t param) = 0;
	virtual void ItemColors(ListViewCtrl& ctrl, int index, size_t param, COLORREF& rgb_text, COLORREF& rgb_backgnd) = 0;
	virtual CString GetTooltipText(ListViewCtrl& ctrl, int index, size_t param) { return CString(); }
	virtual bool IsItemEnabled(ListViewCtrl& ctrl, int index, size_t param) = 0;
	virtual bool FilterItem(ListViewCtrl& ctrl, int index, size_t param, bool group, const String& label, bool& filter_in) = 0;
	virtual void DrawItemBackground(ListViewCtrl& ctrl, int index, size_t param, CDC& dc, CRect rect) = 0;
};



// ListViewCtrl

class ListViewCtrl : public CWnd
{
public:
	ListViewCtrl();
	virtual ~ListViewCtrl();

	bool Create(CWnd* parent, CRect rect, UINT id, ListViewCtrlNotifications* receiver);

	void ReserveItems(size_t count);
	int GetItemCount() const;
	int GetVisibleItemCount() const;
	bool HasVisibleItems() const;

	// append new item
	void AddItem(const TCHAR* text, int icon_index, int command_id, size_t param);
	void AddItem(const TCHAR* text);

	// items can be grouped under single node; open new group and add items
	int OpenGroup(const TCHAR* text);
	void CloseGroup();
	// traversing groups:
	int GetFirstGroup() const;
	int GetNextGroup(int index) const;

	// delete from given item to the end
	void DeleteFromItem(int item_index);

	void SetImageList(CImageList* image_list);
	void SetCheckBoxes(bool enable);

	void SetBackgndColor(COLORREF rgb_backgnd);
	void SetBackgndColors(COLORREF rgb_backgnd, COLORREF selection);
	void SetTextColors(COLORREF normal_text, COLORREF selected_text, COLORREF disabled_text);

	// change item's text
	void SetItemText(int item_index, const TCHAR* text);
	const String& GetItemText(int item_index) const;

	void CollapseGroup(int item_index, bool collapsed);
	bool IsGroupCollapsed(int item_index) const;

	bool IsGroup(int item_index) const;
	bool IsVisible(int item_index) const;
	size_t GetUserParam(int item_index) const;

	// space between items
	void SetItemSpace(float times_text_height);
	void SetMenuLikeSelection(bool on);
	void SetReceiver(ListViewCtrlNotifications* receiver);

	CSize GetIdealSize();

	void EnableItemByIndex(size_t index, bool enable);

	// create tooltip ctrl
	void EnableToolTips();
	void SetOnIdleUpdateState(bool enable);
	void UseParentBackground(bool enable);

	enum Mode { List, Tiles };
	void SetMode(Mode mode);

	// apply item filtering (FilterItemOut will be invoked)
	void FilterItems();

private:
	DECLARE_MESSAGE_MAP()

private:
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* dc);
	void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	LRESULT OnMouseLeave(WPARAM, LPARAM);
	void OnMouseMove(UINT flags, CPoint pos);
	void OnSize(UINT type, int cx, int cy);
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	BOOL OnScroll(UINT scroll_code, UINT pos, BOOL do_scroll= TRUE);
	bool OnScrollBy(CSize scroll_size, bool do_scroll);
	virtual void PreSubclassWindow();
	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	void OnUpdateCmdUI(CFrameWnd* target, bool disable_if_no_hndler);
	BOOL OnToolTipNotify(UINT id, NMHDR* hdr, LRESULT* result);
	void SetCursor();

	struct Impl;
	Impl& impl_;
};
