/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "DialogChild.h"
#include "ToolbarWnd.h"
#include "ScriptEditCtrl.h"
class PhotoTagsCollection;


// EditFilterDlg dialog

class EditFilterDlg : public DialogChild
{
public:
	EditFilterDlg(CWnd* parent, const PhotoInfo& example, const PhotoTagsCollection& collection);
	virtual ~EditFilterDlg();

// Dialog Data
	enum { IDD = IDD_CUSTOM_FILTER };

	String filter_rule_;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	void OnVerifyExpr();
	bool VerifyExpr(const CString& expr, String* err_message);
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnViewAttribs(CPoint pos);
	void OnViewTags(CPoint pos);
	void InsertText(const TCHAR* text);
	bool ReadExpr(String& filter_rule);
	void OnExprSetFocus();

	const PhotoInfo& example_;
	const PhotoTagsCollection& collection_;
	ScriptEditCtrl expression_;
	CEdit info_text_;
	//CFont expr_font_;
	ToolBarWnd insert_;
};
