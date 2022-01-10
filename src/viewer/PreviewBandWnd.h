/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
#include "../AnyPointer.h"
#include "../ColorSets.h"


// PreviewBandWnd

class PreviewBandWnd : public CWnd
{
public:
	PreviewBandWnd();
	virtual ~PreviewBandWnd();

	bool Create(CWnd* parent, bool isLightTable);

	// delete all items
	void RemoveAllItems();

	// reserve space for 'count' items
	void ReserveItems(size_t count);

	// add new item
	void AddItem(CSize size, AnyPointer key);

	// modify item's size; invalidates window, resets scrollbar
	void ModifyItem(CSize size, AnyPointer key);
	// modify given item; window is not invalidated, scrollbar is not reset
	void ModifyItem(size_t index, CSize size, AnyPointer key);

	size_t GetItemCount() const;

	bool IsItemVisible(AnyPointer key) const;

	CRect GetItemRect(AnyPointer key);
	CSize GetItemsSize(AnyPointer key);	// not const due to layout validation

	void EnableSelectionDisp(bool enable);
	void EnableToolTips(bool enable);

	void ResetScrollBar(bool move_pos);

	void ScrollLeft(int speed_up_factor);
	void ScrollRight(int speed_up_factor);

	void SetOrientation(bool horizontal);
	bool IsHorizontalOrientation();

	enum ClickAction { MouseBtnPressed, StartDragDrop, MouseBtnReleased };

	void SetCallBacks(
		const boost::function<void (size_t index, AnyPointer key, ClickAction action)>& item_clicked,
		const boost::function<void (CDC& dc, CRect rect, size_t index, AnyPointer key)>& draw_item,
		const boost::function<String (size_t index, AnyPointer key)>& get_tooltip_text);

	void SetItemDrawCallback(
		const boost::function<void (CDC& dc, CRect rect, size_t index, AnyPointer key)>& draw_item);

	// scroll items so selected one is in the view
	void SelectionVisible(size_t current, bool smooth_scroll);

	void RedrawSelection();
	void RedrawItem(AnyPointer key);

	void SetSelectionColor(COLORREF color);

	// modify brightness of background bitmaps by applying gamma correction
	void SetUIBrightness(double gamma);

	// 100 is normal speed; 0 - no smooth scrolling
	void SetSmoothScrollingSpeed(int speed);

	// set selection (current item)
	void SetCurrent(AnyPointer photo);

	// if true, current item will be kept at the center of preview band
	void KeepCurrentItemCentered(bool enabled);

	// reserve 'vspace' pixels below items (typically used by client code to draw text label)
	void ReserveVerticalSpace(int vspace);


protected:
	DECLARE_MESSAGE_MAP()

private:
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* dc);
	void Paint(CDC& dc);
	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	void OnSize(UINT type, int cx, int cy);
	void OnDestroy();
	//void OnCustDrawScrollBar(UINT id, NMHDR* hdr, LRESULT* result);
	BOOL OnToolTipNotify(UINT id, NMHDR* hdr, LRESULT* result);
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnScroll(UINT scroll_code, UINT pos, int speed);
	bool OnScrollBy(int scroll, int speed, bool repeated);
	void ResetToolTips();
	void ValidateLayout();
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint pos);
	void OnMouseMove(UINT flags, CPoint pos);

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};
