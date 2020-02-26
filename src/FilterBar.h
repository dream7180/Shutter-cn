/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "EditCombo.h"

/////////////////////////////////////////////////////////////////////////////
// FilterBar window

class FilterBar : public EditCombo
{
// Construction
public:
	FilterBar();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

	void Activate();

	void FilterOn(bool on);

// Implementation
public:
	virtual ~FilterBar();

protected:
	afx_msg void OnDestroy();

private:
	std::vector<String> history_;

	void Find();
	LRESULT OnFindMsg(WPARAM, LPARAM);

	enum { IDC_TEXT_COMBO= 100, IDC_FILTER_TOOLBAR };

	void EndEdit(bool ok);
};
