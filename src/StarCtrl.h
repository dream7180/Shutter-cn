/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>


class StarCtrl : public CWnd
{
public:
	StarCtrl();
	virtual ~StarCtrl();

	bool Create(CWnd* parent, int count, int rating, const boost::function<void (int)>& on_clicked, int id);

	void SetRating(int rating);

	void SetClickCallback(const boost::function<void (int)>& on_clicked);

	int GetStarCount() const;

	int GetRating() const;

	void SetBackgndColor(COLORREF c);

protected:
	DECLARE_MESSAGE_MAP()

private:
	static CSize star_size_;
	boost::function<void (int)> on_clicked_;
	int stars_;
	int rating_;
	int hot_stars_;
	UINT_PTR timer_id_;
	static boost::scoped_ptr<Gdiplus::Bitmap> stars_bmp_;
	static int counter_;
	std::vector<int> angles_;
	COLORREF background_;

	BOOL OnEraseBkgnd(CDC* dc);
	void OnSize(UINT type, int cx, int cy);
	void OnLButtonDown(UINT flags, CPoint point);
	int FindStar(CPoint point) const;
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	void SetCursor(CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	LRESULT OnMouseLeave(WPARAM, LPARAM);
	void OnTimer(UINT_PTR event_id);
	virtual void PreSubclassWindow();
};
