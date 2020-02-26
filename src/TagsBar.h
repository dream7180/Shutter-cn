/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// TagsBar.h : header file

#include "ToolBarWnd.h"
#include "PhotoInfoStorage.h"
#include "ConnectionPoint.h"
#include "TagsBarCommon.h"
#include "MultiMonitor.h"
class BrowserFrame;


class CTagsBar : public CMiniFrameWnd, public ConnectionPointer
{
public:
	CTagsBar(PhotoTagsCollection& TagCollection, ConnectionPointer* link);

// Operations
public:
	bool Create(CWnd* parent, BrowserFrame* frame);

	// invoked after selection has changed
	void PhotoSelected(PhotoInfoPtr photo, bool selection, bool still_loading)
	{ bar_wnd_.PhotoSelected(photo, selection, still_loading); }

	void PhotosSelected(VectPhotoInfo& photos)
	{ bar_wnd_.PhotosSelected(photos); }

	// synchronize tags (buttons) with tag collection
//	void SynchTags(const PhotoTagsCollection& tags, int dummy)
//	{ bar_wnd_.SynchTags(tags, dummy); }

	// load tags from text file
	bool LoadTags(const TCHAR* filename)
	{ return bar_wnd_.LoadTags(filename); }

	// notification
	void PhotoReloadingDone()
	{ bar_wnd_.PhotoReloadingDone(); }

// Implementation
	virtual ~CTagsBar();

protected:
	// Generated message map functions
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* MMI);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateTags(CCmdUI* cmd_ui);
	afx_msg void OnTagSelected(UINT cmd);

private:
	TagsBarCommon bar_wnd_;
	WindowPosition wnd_pos_;

	void Resize();
};
