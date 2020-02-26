/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "TagsBarCommon.h"
#include "CloseBar.h"
class PhotoInfo;


class TaggingPopup : public CMiniFrameWnd
{
// Construction
public:
	TaggingPopup(PhotoTagsCollection& tagCollection, const VectPhotoInfo& photos,
		const TagsBarCommon::ApplyTagsFn& apply, const TagsBarCommon::ApplyRatingFn& rate, CPoint left_top, CWnd* parent);

	virtual ~TaggingPopup();

	static bool IsPopupActive();
	static void Refresh(const VectPhotoInfo& photos);

// Overrides
	// ClassWizard generated virtual function overrides
	virtual void PostNcDestroy();
	virtual BOOL ContinueModal();

// Implementation
protected:

	// Generated message map functions
	afx_msg void OnKillFocus(CWnd* new_wnd);
	afx_msg void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	DECLARE_MESSAGE_MAP()
	LRESULT OnFinish(WPARAM, LPARAM);

private:
	// Create modal dialog
	bool Create(CWnd* parent, const VectPhotoInfo& photos);
	BOOL OnEraseBkgnd(CDC* dc);
	void OnClose();
	void RefreshTags(const VectPhotoInfo& photos);

	CPoint left_top_;
	bool run_;
	static TaggingPopup* running_instance_;
	TagsBarCommon tag_wnd_;
	CloseBar close_wnd_;
};
