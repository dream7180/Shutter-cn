/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// TagsBarCommon.h : header file
//
#include "ToolBarWnd.h"
#include "PhotoInfoStorage.h"
#include "ListViewCtrl.h"
#include "PhotoTagsCollection.h"
#include "StarCtrl.h"
#include "TopSeparatorCtrl.h"
#include "EditCombo.h"
class ApplicationColors;


enum { MAX_TAGS= 10000 };

extern void ResetPopupMenuTags(CMenu& menu, int first_id, const PhotoTagsCollection& tags);

class CDialogEX : public CDialog
{
public:
	CDialogEX();

	void SetColors(COLORREF text, COLORREF backgnd);

	BOOL OnEraseBkgnd(CDC* dc);

	HBRUSH OnCtlColor(CDC* dc, CWnd* ctrl, UINT code);

private:
	COLORREF text_;
	COLORREF background_;
	CBrush background_brush_;

	DECLARE_MESSAGE_MAP()
};


class TagsBarCommon : public CWnd, ListViewCtrlNotifications
{
public:
	TagsBarCommon(PhotoTagsCollection& tag_collection, const wchar_t* registry_key);

// Attributes
public:
	enum { ID_TOOLBAR= 1000, ID_FILTER= 1001 };

	int left_indent_;
	int top_indent_;

	COLORREF GetBackgndColor() const;
	void SetColors(const ApplicationColors& colors);

	typedef boost::function<bool (VectPhotoInfo& photos, const String& tag, bool apply, CWnd* parent)> ApplyTagsFn;
	typedef boost::function<bool (VectPhotoInfo& photos, int rating, CWnd* parent)> ApplyRatingFn;

	// if true leave space for a toolbar at the top
	bool tool_bar_at_top_;

	ToolBarWnd& GetToolBar()		{ return tool_bar_wnd_; }
	CWnd& GetFilterEditBox();

	CSize GetIdealSize();

	void UseParentBackground(bool enable);

// Operations
public:
	bool Create(CWnd* parent, bool big_toolbar);
	bool Create(CWnd* parent);

	// invoked after selection has changed
	void PhotoSelected(PhotoInfoPtr photo, bool selection, bool still_loading);
	void PhotosSelected(const VectPhotoInfo& photos);

	// synchronize tags (buttons) with tag collection
	//void SynchTags(const PhotoTagsCollection& tags, int);

	// load tags from text file
	bool LoadTags(const TCHAR* filename);

	// notification
	void PhotoReloadingDone();

	// assign tag to selected photos
	void AssignTag(int index);

	// set callbacks that allow tags bar to fire events (to carry on actions: tagging/rating)
	void SetTagCallback(const ApplyTagsFn& fn);
	void SetRatingCallback(const ApplyRatingFn& fn);

	void SetFocus();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TagsBarCommon)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TagsBarCommon();
protected:
	virtual BOOL IsFrameWnd() const;

	// Generated message map functions
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* MMI);
	afx_msg void OnTagsNew();
	afx_msg void OnUpdateTagsNew(CCmdUI* cmd_ui);
	afx_msg void OnTagsManage();
	afx_msg void OnUpdateTagsManage(CCmdUI* cmd_ui);
	afx_msg void OnTagsSave();
	afx_msg void OnUpdateTagsSave(CCmdUI* cmd_ui);
	afx_msg void OnDestroy();
	afx_msg void OnTagsAssign();
	afx_msg void OnUpdateTagsAssign(CCmdUI* cmd_ui);
	afx_msg void OnTagsLoad();
	afx_msg void OnUpdateTagsLoad(CCmdUI* cmd_ui);
	afx_msg void OnUpdateTagsSaveInPhoto(CCmdUI* cmd_ui);
	afx_msg void OnTagsSaveInPhoto();
	afx_msg void OnUpdateTags(CCmdUI* cmd_ui);
	afx_msg void OnTagSelected(UINT cmd);
	afx_msg void OnUpdateTagsOptions(CCmdUI* cmd_ui);
	afx_msg void OnClearRating();
	void OnChangeFilter();
	void OnApplyTags();
	void OnCancelFilter();

	DECLARE_MESSAGE_MAP()

private:
	ToolBarWnd tool_bar_wnd_;
	EditCombo filter_bar_;
	CImageList image_list_;
	CImageList image_list_dis_;
	ListViewCtrl checks_wnd_;
	CDialogEX rating_wnd_;
	CDialogEX status_wnd_;
	CStatic status_label_;
	StarCtrl stars_;
	//CStatic rate_label_;
	TopSeparatorCtrl separator_;
	ToolBarWnd clear_rating_;
	int top_;
	bool in_update_;
	//bool create_big_toolbar_;
	bool create_toolbar_;
	bool create_status_bar_;
	bool create_rating_wnd_;
	bool create_filter_bar_;
	ApplyTagsFn apply_tag_to_photos_;
	ApplyRatingFn apply_rating_to_photos_;
	COLORREF rgb_background_;
	COLORREF edit_background_;
	int extra_tags_begin_;
	bool use_parent_background_;
	const wchar_t* registry_key_;
	String filter_text_;

	void CreateTagsButtons();

	struct TagBtn
	{
		enum TagState { UNSELECTED, SELECTED, UNDETERMINED } state_;
		String name_;

		TagBtn(const String& name, TagState state= UNSELECTED)
		{
			state_ = state;
			name_ = name;
		}

		TagBtn(const TagBtn& tag)
		{
			*this = tag;
		}

		TagBtn& operator = (const TagBtn& tag)
		{
			state_ = tag.state_;
			name_ = tag.name_;
			return *this;
		}
	};


	struct Tags : public std::vector<TagBtn>
	{
		bool AllTagsClear()
		{
			bool change= false;
			for (iterator it= begin(); it != end(); ++it)
				if (it->state_ != TagBtn::UNSELECTED)
					it->state_ = TagBtn::UNSELECTED, change = true;
			return change;
		}
	};

	friend struct Tags;

	Tags tags_;						// tags as buttons
	PhotoTagsCollection& tag_collection_;
	auto_connection on_change_;
	VectPhotoInfo selected_photos_;

	void SyncButtons(ConstPhotoInfoPtr photo);
	void SyncButtons(const VectPhotoInfo& photos);
	bool tags_enabled_;
	enum SyncState { CONT, CHANGED, STOP };
	SyncState SyncButtons(const PhotoInfo& photo, bool ignore_existing_state);
	void SyncStars(const PhotoInfoPtr photos[], size_t count);
	void RedrawItem(int index);
	// when global collection of tags changes, refresh our copy and the UI
	void SynchronizeTags(const PhotoTagsCollection& tags);
	// update status bar (currently selected photo)
	void UpdateStatusBar(bool selection, bool still_loading);
	void AddPhotoSpecificTags(ConstPhotoInfoPtr photo);
	void AddPhotoSpecificTags(const VectPhotoInfo& photos);
	String GetTagItemName(int index, const String& tag) const;
	void TagCollectionChanged();
	void EnableStars(bool enable);
	void StarClicked(int stars);
	void RememberCollapsedGroups();
	String FilterText();
	void OnFilterCmd(bool enter);
	void UpdateFilterButton();
	void OnTagFilter();
	bool IsApplyBtnEnabled();

	void SaveTags(const TCHAR* filename, const PhotoTagsCollection& collection);
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnViewPopupMenu(CWnd* toolbar, CRect btn);
	LRESULT OnPrintClient(WPARAM wdc, LPARAM flags);

	// list view ctrl notifications
	virtual void ItemClicked(ListViewCtrl& ctrl, int index, size_t param);
	virtual int GetItemCheckState(ListViewCtrl& ctrl, int index, size_t param);
	virtual void ItemColors(ListViewCtrl& ctrl, int index, size_t param, COLORREF& rgb_text, COLORREF& rgb_backgnd);
	virtual bool IsItemEnabled(ListViewCtrl& ctrl, int index, size_t param);
	virtual bool FilterItem(ListViewCtrl& ctrl, int index, size_t param, bool group, const String& label, bool& filter_in);
	virtual void DrawItemBackground(ListViewCtrl& ctrl, int index, size_t param, CDC& dc, CRect rect);
};
