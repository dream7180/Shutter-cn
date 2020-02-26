/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifViewReBar.h: interface for the ExifViewReBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXIFVIEWREBAR_H__91D67920_8F72_49F0_B6F3_3F281F1FE9E2__INCLUDED_)
#define AFX_EXIFVIEWREBAR_H__91D67920_8F72_49F0_B6F3_3F281F1FE9E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HorzReBar.h"
#include "ToolBarWnd.h"
#include "SliderCtrlEx.h"


class ExifViewReBar : public CWnd // CHorzReBar
{
public:
	ExifViewReBar();
	virtual ~ExifViewReBar();

	bool Create(CWnd* parent, int thumb_index_range, bool large);

	void SetThumbPos(int pos)			{ thumb_size_wnd_.SetPos(pos); }
	int GetThumbPos() const				{ return thumb_size_wnd_.m_hWnd ? thumb_size_wnd_.GetPos() : 0; }

	void ResetThumbRange(int range)		{ thumb_size_wnd_.SetRange(0, range - 1); }

	bool IsSliderValid() const			{ return thumb_size_wnd_.m_hWnd != 0; }

	CRect GetSortBtnRect(int cmd_id) const;
	CRect GetMainBtnRect(int cmd_id) const;
	CRect GetViewBtnRect(int cmd_id) const;

	std::pair<int, int> GetMinMaxWidth() const	{ return std::make_pair(GetMinTotalBarWidth(), GetTotalBarWidth()); }

	void ShowCancelSortBtn(bool show)	{ tool_bar_sort_wnd_.HideButton(ID_CANCEL_SORT_BY_SIMILARITY, !show); }

	void SetBitmapSize(bool large);

	void ImgSizeLabel(bool thumbs);

	void Invalidate();

	void SetBackgroundColor(COLORREF backgnd);

	DECLARE_MESSAGE_MAP()
private:
	ToolBarWnd toolbar_first_;
	ToolBarWnd tool_bar_view_wnd_;
	ToolBarWnd tool_bar_thumb_wnd_;
	ToolBarWnd tool_bar_group_wnd_;
	ToolBarWnd tool_bar_sort_wnd_;
	//ToolBarWnd toolbar_filter_;
	SliderCtrlEx thumb_size_wnd_;
	CWnd* owner_;
	struct Part
	{
		int label_width;
		int tool_bar_width;
		bool show_label;
		const TCHAR* label;

		Part(int label_width, int tb, const TCHAR* label)
			: label_width(label_width), tool_bar_width(tb), show_label(false), label(label)
		{}
	};
	std::vector<Part> tool_bar_pos_;

	BOOL OnEraseBkgnd(CDC* dc);
	Part PositionBar(ToolBarWnd& tool_bar_wnd, const TCHAR* label);
	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	LRESULT OnTbClicked(WPARAM hwnd, LPARAM code);
	void OnSize(UINT type, int cx, int cy);
	void Resize();
	void Reposition();
	int GetTotalBarWidth() const;		// total width: all toolbars including labels
	int GetMinTotalBarWidth() const;	// only toolbars' widths
	void SetSliderPos();

	enum { ID_THUMB_SIZE= 10002 };
};

#endif // !defined(AFX_EXIFVIEWREBAR_H__91D67920_8F72_49F0_B6F3_3F281F1FE9E2__INCLUDED_)
