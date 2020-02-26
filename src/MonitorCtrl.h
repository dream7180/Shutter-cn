/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include <boost/function.hpp>


class MonitorCtrl : public CWnd
{
public:
	MonitorCtrl();

//	bool Create(CWnd* parent, int count, int rating, const boost::function<void (int)>& on_clicked, int id);

	//void SetRating(int rating);

	void SetResizeCallback(const boost::function<void ()>& on_resized);

	//int GetStarCount() const;

	//int GetRating() const;

	void SetAspect(int w, int h);

	CRect GetDisplayArea() const;

protected:
	DECLARE_MESSAGE_MAP()

private:
	//static CImageList star_img_;
	//static CSize star_size_;

	boost::function<void ()> on_resized_;
	//int stars_;
	//int rating_;
	CSize aspect_;
	CRect monitor_rect_;
	CRect display_area_;
	int bezel_thickness_;

	void CalcSize();
	BOOL OnEraseBkgnd(CDC* dc);
	void OnSize(UINT type, int cx, int cy);
	void OnLButtonDown(UINT flags, CPoint point);
	virtual void PreSubclassWindow();
};
