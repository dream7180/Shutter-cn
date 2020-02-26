/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "ToolBarWnd.h"
#include <boost/function.hpp>
#include "DlgAutoResize.h"
#include "PropertyPane.h"


class DescriptionPane : public Property::Pane
{
// Construction
public:
	DescriptionPane(AutoCompletePopup& auto_complete, boost::ptr_vector<Property::Field>& fields);

	void Create(CWnd* parent);

// Dialog Data
	enum { IDD = IDD_DESCRIPTION_PANE };

	String* text_;	// description in/out

//	void ReadText();

//	void Reset();

//	bool IsModified() const;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	void OnChangeDescription();
	void OnSize(UINT type, int cx, int cy);
	void OnFocusSet();
	DECLARE_MESSAGE_MAP()

private:
	CEdit& edit_;
	CFont fndEdit_;
	bool is_modified_;
//	DlgAutoResize resize_;
	ToolBarWnd	toolBar_;

	void BuildToolbar();
	void OnSymbol(UINT cmd_id);
	BOOL InitDlg();
	void UpdateButtons();
};
