/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>


// SimpleReBar

class SimpleReBar : public CWnd
{
public:
	SimpleReBar();
	virtual ~SimpleReBar();

	bool Create(CWnd* parent, CWnd* band1, CWnd* band2,
		const boost::function<void ()>& resize_callback,
		const boost::function<void (CDC& dc, int id, CRect band, bool in_line)>& erase_callback);

	int GetHeight();

	void ShowBand(size_t index, bool visible);
	bool IsBandVisible(size_t index) const;

	void RestoreLayout(const String& layout);
	String StoreLayout();
//	bool RestoreLayout(const TCHAR* section, const TCHAR* key);
//	bool StoreLayout(const TCHAR* section, const TCHAR* key);

	void BandResized(CWnd& band_wnd, CSize size);

	void SetGripperColors(COLORREF light, COLORREF dark);

protected:
	DECLARE_MESSAGE_MAP()

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	void OnPaint();
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint pos);
	void OnLButtonDblClk(UINT flags, CPoint pos);
	void OnMouseMove(UINT flags, CPoint pos);
	void OnSize(UINT type, int cx, int cy);
	BOOL OnEraseBkgnd(CDC* dc);
	void Paint(CDC& dc);
	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	BOOL OnSetCursor(CWnd* wnd, UINT hitTest, UINT message);
	void SetCursor();
};
