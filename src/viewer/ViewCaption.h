/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "FancyToolBar.h"
#include "../Dib.h"
#include <boost/function.hpp>


class ViewCaption : public CWnd
{
public:
	ViewCaption();

	bool Create(CWnd* parent, int toolbarBmp, const int commands[], int count,
		const boost::function<void (void)>& on_clicked);

	void SetActive(bool active);

	COLORREF text_color_;
	COLORREF label_color_;

protected:
	DECLARE_MESSAGE_MAP()

private:
	FancyToolBar toolbar_;
	bool active_;
	boost::function<void (void)> on_clicked_;
	static Dib active_marker_;
	static Dib caption_;

	BOOL OnEraseBkgnd(CDC* dc);
	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	void OnSize(UINT type, int cx, int cy);
	void OnLButtonDown(UINT flags, CPoint point);
	CFont _font;
};
