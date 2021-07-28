/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"
#include "DialogBase.h"
#include "DlgAutoResize.h"
#include <boost/function.hpp>
#include "FilterData.h"
#include "SeparatorWnd.h"
#include "ResizeWnd.h"
#include "StarCtrl.h"
#include "ToolbarWnd.h"
#include "PhotoTagsCollection.h"

enum { DEFAULT_TB_PAD_DX= 8, DEFAULT_TB_PAD_DY= 10 };


class FilterDialog : public DialogBase, protected ResizeWnd
{
public:
	FilterDialog(int id, CWnd* parent);
	virtual ~FilterDialog();

	// callback
	boost::function<void (FilterDialog* dlg)> FilterChanged;

	virtual bool IsFilterActive() const = 0;
	virtual void ClearFilter() = 0;

	void ShowClearAllBtn(bool show);

	bool IsResizable() const;
	virtual void ResizePane(int height);

	virtual int GetFlags() const;
	virtual void SetFlags(int flags);

protected:
//	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnSize(UINT type, int cx, int cy);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClear();

	DECLARE_MESSAGE_MAP()

	DlgAutoResize resize_map_;
	int min_height_;
	SeparatorWnd resize_;
	ToolBarWnd cancel_;

	void NotifyFilterChanged();

	virtual int GetPaneHeight();
};


class CFilterDialog_1 : public FilterDialog
{
public:
	CFilterDialog_1(CWnd* parent = NULL);   // standard constructor

	void SetTags(PhotoTagsCollection& tags);

	void GetCurrentSelection(const PhotoTagsCollection& tags, FilterTags& selection);
	void SetSelection(const PhotoTagsCollection& tags, const FilterTags& selection);

	virtual bool IsFilterActive() const;
	virtual void ClearFilter();

	virtual int GetFlags() const;
	virtual void SetFlags(int flags);

protected:
	enum { IDD = IDD_FILTER_1 };

	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CTreeCtrl tags_;
	HTREEITEM include_;
	HTREEITEM exclude_;
	bool ready_;
	PhotoTagsCollection* collection_;
	auto_connection on_change_;
	ToolBarWnd edit_tags_;
	struct TreeItems
	{
		TreeItems() : include(0), exclude(0)
		{}
		TreeItems(HTREEITEM include, HTREEITEM exclude) : include(include), exclude(exclude)
		{}
		HTREEITEM include, exclude;
	};
	typedef std::map<String, TreeItems> TreeMap;
	TreeMap tree_map_;
	bool handle_clicks_;

	void PopulateTree(const PhotoTagsCollection& tags);
	void UpdateTree(const PhotoTagsCollection& tags);
	void TagsCollectionChanged();
	void OnEditTags();
	void CheckTags(CTreeCtrl& tree, const FilterTags& selection);

	afx_msg void OnAll();
	afx_msg void OnAny();
	afx_msg void OnItemChanged(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnItemClicked(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnItemKeyDown(NMHDR* nmhdr, LRESULT* result);
	LRESULT OnChange(WPARAM, LPARAM);
};


class CFilterDialog_2 : public FilterDialog
{
public:
	CFilterDialog_2(CWnd* parent = NULL);   // standard constructor

	void SetText(const TCHAR* include, const TCHAR* exclude);
	void GetText(String& include, String& exclude) const;

	virtual bool IsFilterActive() const;
	virtual void ClearFilter();

protected:
	enum { IDD = IDD_FILTER_2 };

	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CEdit include_;
	CEdit exclude_;
	bool update_blocked_;
	static const int TIMER_ID= 123;
	void OnTimer(UINT_PTR event);
	void OnTextChanged();
};


class CFilterDialog_3 : public FilterDialog
{
public:
	CFilterDialog_3(CWnd* parent = NULL);   // standard constructor

	String GetRule();// const;
	void SetRule(const String& rule);
	CListCtrl filterrule_;

	virtual bool IsFilterActive() const;
	virtual void ClearFilter();
	CStringArray filter_rules;

protected:
	enum { IDD = IDD_FILTER_3 };

	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnEditRule();
	void UpdateRule(const String& rule);
	void LoadFilterRules(const TCHAR* filename);

	String rule_;
	ToolBarWnd edit_rule_;
	DECLARE_MESSAGE_MAP()
	
private:
	bool m_bHit = false;
	afx_msg void OnItemClicked(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnItemChanged(NMHDR* nmhdr, LRESULT* result);
};


class CFilterDialog_4 : public FilterDialog
{
public:
	CFilterDialog_4(CWnd* parent = NULL);   // standard constructor

	void SetStars(int stars);
	int GetStars() const;

	virtual bool IsFilterActive() const;
	virtual void ClearFilter();

protected:
	enum { IDD = IDD_FILTER_4 };

	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void OnStarsClicked(int stars);
	DECLARE_MESSAGE_MAP()

private:
	StarCtrl stars_;
	CStatic label_;
};
