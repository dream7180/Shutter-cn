/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSDLG_H__455F6A3C_5B79_4327_B73D_17A51016EC61__INCLUDED_)
#define AFX_OPTIONSDLG_H__455F6A3C_5B79_4327_B73D_17A51016EC61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//
#include "OptionsGeneral.h"
#include "OptionsColumns.h"
#include "OptionsAppearance.h"
#include "OptionsFileTypes.h"
#include "OptionsBalloons.h"
#include "OptionsShortcuts.h"
#include "AdvancedOptions.h"
#include "DlgAutoResize.h"
#include "MultiMonitor.h"
#include "DataExchange.h"

/////////////////////////////////////////////////////////////////////////////
// OptionsDlg

class OptionsDlg : public CPropertySheet
{
// Construction
public:
	OptionsDlg(CWnd* parent_wnd, std::vector<uint16>& columns_idx, std::vector<uint16>& balloon_fields, const boost::function<void (CWnd* parent)>& define_columns, int selectPage, Columns& columns);

// Attributes
	std::vector<uint16>& SelectedColumns()	{ return page_columns_.selected_; }
	bool ColumnsChanged() const				{ return page_columns_.changed_; }

	std::vector<uint16>& SelectedFields()	{ return page_balloons_.selected_; }
	bool FieldsChanged() const				{ return page_balloons_.changed_; }

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OptionsDlg)
	virtual INT_PTR DoModal();
	//}}AFX_VIRTUAL

// Implementation
	virtual ~OptionsDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(OptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnHelp();

private:
	OptionsGeneral		page_general_;
	OptionsColumns		page_columns_;
	OptionsAppearance	page_appearance_;
	OptionsFileTypes	page_file_types_;
	OptionsBalloons		page_balloons_;
	OptionsShortcuts	page_shortcuts_;
	AdvancedOptions		page_advanced_;
	static const int PAGES= 7;
	RPropertyPage*		pages_[PAGES];

	DataExchange dx_;
	DlgAutoResize resize_;
	static int active_page_index_;
	CSize min_size_;
	WindowPosition position_;

	void OnSize(UINT type, int cx, int cy);
	void OnGetMinMaxInfo(MINMAXINFO* mmi);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* result);
	LRESULT OnResizePages(WPARAM wParam, LPARAM lParam);
	void ResizePages();
};


#endif // !defined(AFX_OPTIONSDLG_H__455F6A3C_5B79_4327_B73D_17A51016EC61__INCLUDED_)
