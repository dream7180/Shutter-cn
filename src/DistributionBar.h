/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ToolBarWnd.h"
#include "PhotoInfoStorage.h"


// DistributionBar

class DistributionBar : public CWnd
{
public:
	DistributionBar();
	virtual ~DistributionBar();

	bool Create(CWnd* parent);

	int GetHeight() const;

	void BuildHistogram(const VectPhotoInfo& visible_photos, const VectPhotoInfo& histogram);

	void SetFilterCallback(const boost::function<void (DateTime from, DateTime to, DateTime hist_from, DateTime hist_to)>& callback);
	void SetCancelCallback(const boost::function<void ()>& callback, const boost::function<bool ()>& is_filter_active);

	void ClearSelection();

	void ClearHistory();

	// draw a tick pointing to the 'time' on a timeline; pass empty DateTime() to remove it
	void SetTick(DateTime time);

private:
	struct Impl;

	std::auto_ptr<Impl> impl_;

	BOOL OnEraseBkgnd(CDC* dc);
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void OnRButtonDown(UINT flags, CPoint pos);
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	void SetCursor();
	void OnSize(UINT type, int cx, int cy);
	void OnBack();
	void OnNext();
	void OnCancelFilter();
	void OnUpdateBack(CCmdUI* cmd_ui);
	void OnUpdateNext(CCmdUI* cmd_ui);
	void OnUpdateCancelFilter(CCmdUI* cmd_ui);
	virtual BOOL IsFrameWnd() const;

	DECLARE_MESSAGE_MAP()
};
