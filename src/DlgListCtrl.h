/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// DlgListCtrl

class DlgListCtrl : public CWnd
{
public:
	DlgListCtrl();
	virtual ~DlgListCtrl();

	bool Create(CWnd* parent);
	bool Create(CWnd* parent, int dlg_id);

	void AddSubDialog(CDialog* dlg, const TCHAR* title= 0, bool expanded= true);
	void AddSubDialog(CDialog* dlg, int image_index, const TCHAR* title, bool expanded);
	void AddSubDialog(CWnd* dlg, int image_index, const TCHAR* title, bool expanded, CSize min_dim_size);

	size_t GetCount() const;

	bool IsSubDialogExpanded(size_t index);
	void ExpandSubDialog(size_t index, bool expand);

	void SetImageList(CImageList* image_list)	{ image_list_ = image_list; }

	void SetLastDlgExtendable(bool enable)		{ extend_last_dlg_to_whole_wnd_ = enable; }

	void SetSingleExpand(bool enable);

	// make sure this 'rect' in a 'dialog' is visible (scroll to it)
	void EnsureVisible(CWnd* dialog, CRect rect);

	void SetRightMargin(int margin);
	void SetLeftMargin(int margin);
	void SetTopMargin(int margin);
	void SetBottomMargin(int margin);

	void DrawCheckboxes(bool enable);

	void SetSubdialogHeight(CWnd* dlg, int height);
	int GetSubdialogHeight(size_t dlg_index) const;

	void SetSubdialogImage(CWnd* dlg, int image_index);

protected:
	DECLARE_MESSAGE_MAP()

	BOOL OnEraseBkgnd(CDC* dc);
	virtual void PreSubclassWindow();

	static CImageList img_list_triangles_;
	CImageList* image_list_;
	bool extend_last_dlg_to_whole_wnd_;

	struct SubDlg
	{
		SubDlg() : dlg_(0), expanded_(false), is_resizable_(false), image_index_(-1) {}
		SubDlg(CWnd* dlg, const TCHAR* title, bool expanded, int image_index, CWnd* parent);

		bool IsExpanded() const		{ return expanded_; }
		long Width() const			{ return dlg_rect_.Width(); }
		long Height() const			{ return dlg_rect_.Height(); }

		bool SetLocation(CPoint start, int available_width, int height);
		void DrawHeader(CDC& dc, CRect rect, int wnd_width, int left_margin, int right_margin, CImageList* image_list, bool draw_checkboxes);
		bool Toggle();
		bool Expand(bool expand);
		void ShowDlg();
		void Hide();
		void DrawArrowAnimation(CDC& dc, CRect rect, bool closing_anim);

		CWnd* dlg_;
		CRect dlg_rect_;
		bool expanded_;
		bool is_resizable_;
		String item_;
		int image_index_;
	private:
		bool Create(CDialog* dlg, CWnd* parent);
		void DrawArrow(CDC& dc, CRect& rect, int image, bool adjust_rect);
	};

	std::vector<SubDlg> dialogs_;

private:
	void OnSize(UINT type, int cx, int cy);
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	bool OnScroll(UINT scroll_code, UINT pos, bool do_scroll= true);
	bool OnScrollBy(CSize scroll_size, bool do_scroll);
	void ResizeSubDialogs(bool shifting_down= false);
	void RegisterWndClass();
	void SetScrollBar();
	void CalcHeaderHeight();
	void SelectFont(CDC& dc);
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint pos);
	int HitText(CPoint pos);
	CRect GetHeaderRect(int index);
	void ToggleSubDialog(int index);
	COLORREF GetBackgndColor();
	CSize GetTotalSize() const;
	BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
	void MoveFocusToVisibleWindow(CWnd* start= 0);
	void OnSetFocus(CWnd* focus);
	UINT OnGetDlgCode();
	CFont _font;

	CSize range_size_;	// combined height of sub dialogs, max width
	int header_height_;
	CSize scroll_bars_size_;		// scrollbars dimensions
	bool single_expand_;
	int right_margin_;
	int left_margin_;
//	bool update_;
	bool draw_checkboxes_;
	int top_margin_;
	int bottom_margin_;

	CPoint GetOffset() const		{ return CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT)); }
};
