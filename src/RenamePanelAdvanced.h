/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "RenamePage.h"
#include "ToolbarWnd.h"
#include "RenameRule.h"
#include "SamplePhoto.h"
#include "Lua.h"
#include "ScriptEditCtrl.h"


// RenamePanelAdvanced dialog

class RenamePanelAdvanced : public RenamePage
{
public:
	RenamePanelAdvanced(const SmartPtr example_photo, const std::vector<RenameRule>& rule_defs);
	virtual ~RenamePanelAdvanced();

// Dialog Data
	enum { IDD = IDD_RENAME_ADVANCED };

	virtual bool GenerateFileName(State state, const PhotoInfo* input, Path* output_name);
	virtual bool ShowFullPath();

	const std::vector<RenameRule>& Rules() const	{ return rule_defs_; }

	virtual bool UpdateData();

	int current_rule_;
	CString input_text_;
	int starting_number_;
//	boost::function<void(void)> refresh_results_;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	virtual void Transform(Dib& dib, bool preview) {}
	virtual BOOL OnInitDialog();
	void OnExprSetFocus();
	virtual void OnCancel();
	virtual void OnOK();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void ShowRule(int rule, bool update_selection);
	bool ReadRule(size_t rule, bool silent= false);
	void OnRuleSelChange();
	void OnVerifyExpr();
	bool VerifyExpr(const CString& expr, String* err_message);
	bool VerifyCurrentExpr();
	void OnSelectionChanged(NMHDR* nmhdr, LRESULT* result);
	void OnSelectionChanging(NMHDR* nmhdr, LRESULT* result);
	void OnRuleNameChanged(NMHDR* nmhdr, LRESULT* result);
	void OnBeginLabelEdit(NMHDR* nmhdr, LRESULT* result);
	void OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	void OnRenameRule();
	void OnParamChanged();

	ScriptEditCtrl expression_;
	String rename_rule_;
	CFont expr_font_;
	ToolBarWnd insert_;
	CEdit info_text_;
	bool in_update_;
	//CEdit name_;
	CEdit text_;
	CEdit counter_;
	std::vector<RenameRule> rule_defs_;
	const SmartPtr example_;
	CListCtrl list2_;
	std::auto_ptr<Lua> lua_;
	ToolBarWnd rename_;
};
