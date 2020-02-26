/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "CustomColumns.h"
#include "DialogChild.h"
#include "ToolbarWnd.h"
#include "ScriptEditCtrl.h"


// CustomColumnsDlg dialog

class CustomColumnsDlg : public DialogChild
{
public:
	CustomColumnsDlg(CWnd* parent, const CustomColumns& defs, const PhotoInfo& example);
	virtual ~CustomColumnsDlg();

// Dialog Data
	enum { IDD = IDD_CUSTOM_COLUMNS };

	const CustomColumns& GetColumns() const;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

private:
	CEdit name_;
	ScriptEditCtrl expression_;
//	CEdit expression_;
	CListBox list_;
	CButton show_col_;
	CustomColumns col_defs_;
	size_t current_column_;
	const PhotoInfo& example_;
	bool in_update_;
	CEdit info_text_;
	CFont expr_font_;
	ToolBarWnd insert_;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	void OnColSelChange();
	void ShowCol(int col);
	bool ReadCol(size_t col);
	void OnVerifyExpr();
	void OnResetAll();
	bool VerifyExpr(const CString& expr, String* err_message);
	void OnSize(UINT type, int cx, int cy);
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnExprSetFocus();
};
