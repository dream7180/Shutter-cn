/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class GridCtrl;


class GridCtrlNotification
{
public:
	enum Flags
	{
		NONE= 0,
		DRAW_BTN_DOWN= 1,
		DRAW_BTN_RIGHT= 2,
		DRAW_ELLIPSIS= 4
	};

	virtual void GetCellText(GridCtrl& ctrl, size_t row, size_t col, CString& text) = 0;
	virtual void CellTextChanged(GridCtrl& ctrl, size_t row, size_t col, const CString& text) = 0;
	virtual void Delete(GridCtrl& ctrl, size_t row, size_t col) = 0;
	virtual bool StartEditing(GridCtrl& ctrl, size_t row, size_t col) { return true; }
	virtual UINT GetItemFlags(GridCtrl& ctrl, size_t row, size_t col) { return NONE; }
	virtual void PostPaintCell(GridCtrl& ctrl, size_t row, size_t col, CDC& dc) {}
};


class CGridEditBox : public CEdit
{
public:

	DECLARE_MESSAGE_MAP()
private:
	void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	UINT OnGetDlgCode();
	void OnChar(UINT chr, UINT rep_cnt, UINT flags);
};


class GridCtrl : public CListCtrl
{
public:
	GridCtrl(bool show_add_neww_item= true);
	virtual ~GridCtrl();

	void SetHost(GridCtrlNotification* host)		{ host_ = host; }

	void SetItemCount(size_t count);

	DECLARE_MESSAGE_MAP()
private:
	BOOL OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	void OnCustomDraw(NMHDR* nm_hdr, LRESULT* result);
	virtual void PreSubclassWindow();
	afx_msg void OnNMClick(NMHDR *nmhdr, LRESULT *result);
	void EnterEdit(int row, int col);
	void EnterEdit(int row, int col, const TCHAR* text);
	void OnEditKillFocus();
	LRESULT OnFinishEdit(WPARAM key, LPARAM);
	void FinishEditing(bool ok);
	void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	void OnChar(UINT chr, UINT rep_cnt, UINT flags);
	void EnsureColVisible(int col);
	UINT OnGetDlgCode();

	enum Dir { GO_LEFT, GO_RIGHT, GO_UP, GO_DOWN, GO_NEXT };
	void GoTo(Dir dir, bool edit);

	enum { EDIT_ID= 100 };
	CGridEditBox edit_box_;
	CImageList image_list_;
	GridCtrlNotification* host_;
	CString buffer_;
	int edit_row_;
	int edit_col_;
	int sel_column_;
	bool show_add_new_item_;
	CImageList menu_down_btn_;
	CImageList menu_right_btn_;

	enum { TEXT_LIMIT= 100 };

	void InstallMouseHook();
	void UninstallMouseHook();
};
