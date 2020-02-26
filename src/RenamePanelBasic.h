/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "RenamePage.h"
#include "PathEdit.h"
#include "ToolBarWnd.h"
#include "afxwin.h"


// RenamePanelBasic dialog

class RenamePanelBasic : public RenamePage
{
public:
	RenamePanelBasic();
	virtual ~RenamePanelBasic();

// Dialog Data
	enum { IDD = IDD_RENAME_BASIC };

	CString GetPattern();
	int GetStartNumber();

	CString pattern_;
	int starting_number_;
	int trim_left_;
	int trim_right_;
	int letter_case_;
	BOOL replace_case_sensitive_;
	CString search_text_;
	CString replace_text_;
	int extension_case_;

	virtual bool GenerateFileName(State state, const PhotoInfo* input, Path* output_name);
	virtual bool ShowFullPath();
	virtual bool UpdateData();

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	virtual void Transform(Dib& dib, bool preview) {}
	virtual BOOL OnInitDialog();

	CPathEdit pattern_edit_;
	ToolBarWnd symbols_tb_;
	CEdit number_;
	void SymbolsPopupMenu();
	CEdit info_;
	CEdit trim_left_edit_;
	CEdit trim_right_edit_;
	CPathEdit search_edit_;
	CPathEdit replace_edit_;

	void OnPatternChange();
	void OnParamValueChange();
	void OnSearchTextChange();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT flags);
};
