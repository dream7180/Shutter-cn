/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

namespace CtrlDraw
{
	void DrawCheckBox(CDC& dc, CRect rect, int state);

	void DrawRadioBox(CDC& dc, CRect rect, int state);

	CSize GetCheckBoxSize(CWnd* wnd);

	CSize GetRadioBtnSize(CWnd* wnd);

	void DrawBorder(CDC& dc, CRect rect);

	void DrawComboBorder(CDC& dc, CRect rect, bool hot);

//	bool DrawListItem(CDC& dc, CRect rect, int state);

	void DrawExpandBox(CDC& dc, const CRect& rect, bool collapsed, bool hot);

//private:
	//static CImageList image_list_;
	//void DrawCtrl(CDC& dc, CRect rect, int state, bool check_box);
	//CSize GetCtrlSize(CWnd* wnd, int part, int state) const;

	bool DrawThemedElement(HDC hdc, const CRect& rect, const wchar_t* wnd_class, int part, int state, int exclude_interior);
};
