/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class PhotoTagsCollection;
#include "FilterData.h"
#include "DialogBase.h"


class FilterOperations
{
public:
	virtual void FilterParamsChanged() = 0;		// notification: filter params have been modified
	//virtual void NameChanged() = 0;			// filter name changed
	//virtual void StoreFilter() = 0;			// store current filter
	//virtual void DeleteFilter() = 0;		// delete current filter
	//virtual void UpdateFilter() = 0;		// update current filter
};


// FilterPanelDlg dialog

class FilterPanelDlg : public DialogBase
{
public:
	FilterPanelDlg();//CWnd* parent);
	virtual ~FilterPanelDlg();

	bool Create(CWnd* parent, PhotoTagsCollection& tags, FilterOperations* operations);

	void GetCurrentFilter(FilterData& filter) const;
	void SetFilter(const FilterData& filter, bool custom);

	//String GetFilterName() const;
	//void ShowNameInUseErr();

	size_t GetPanelCount() const;

	// UI settings
	void GetPanelUISettings(size_t panel, int& height, bool& expanded, int& flags);
	void SetPanelUISettings(size_t panel, int height, bool expanded, int flags);

// Dialog Data
	enum { IDD = IDD_FILTER_PANEL };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	void OnFilterCommand(UINT cmd);
	void OnFilterNameChanged();
	DECLARE_MESSAGE_MAP()

private:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	void OnSize(UINT type, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);

	struct Impl;
	std::auto_ptr<Impl> impl_;
};
