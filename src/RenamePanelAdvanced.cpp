/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RenamePanelAdvanced.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RenamePanelAdvanced.h"
#include "CatchAll.h"
#include "PhotoInfo.h"
#include "Attributes.h"
#include "Block.h"
#include "BalloonMsg.h"
#include "ExprCalculator.h"
#include "StringConversions.h"
#include "boost/algorithm/string/replace.hpp"

extern void CreateScriptEditingFont(CFont& font);
extern void PassParams(Lua& lua, const PhotoInfo& photo);
extern void DefRoundFn(Lua& lua);
extern void DefGlobalImgTable(Lua& lua);
extern void InitializeLua(Lua& lua);
extern String ReplaceIllegalChars(const String& text, TCHAR replacement_char);

const int NAME_MAX_LENGTH= 100;		// max length for a rule name
const int MAX_SCRIPT_STEPS= 10000;	// to avoid endless loops

// RenamePanelAdvanced dialog

RenamePanelAdvanced::RenamePanelAdvanced(const SmartPtr example_photo, const std::vector<RenameRule>& rule_defs)
 : RenamePage(RenamePanelAdvanced::IDD), example_(example_photo), rule_defs_(rule_defs)
{
	in_update_ = false;
	current_rule_ = 0;
	starting_number_ = 1;
	//input_text_ = _T("");
	expression_.EnableInputAttribsColors(true);
}


RenamePanelAdvanced::~RenamePanelAdvanced()
{}


void RenamePanelAdvanced::DoDataExchange(CDataExchange* DX)
{
	ImgPage::DoDataExchange(DX);
	DDX_Control(DX, IDC_EXPR, expression_);
	DDX_Control(DX, IDC_TOOLBAR, insert_);
	DDX_Control(DX, IDC_LABEL_2, info_text_);
	//DDX_Control(DX, IDC_LIST, list_);
	//DDX_Control(DX, IDC_NAME, name_);
	DDX_Control(DX, IDC_TEXT, text_);
	DDX_Control(DX, IDC_COUNTER, counter_);
	DDX_Control(DX, IDC_LIST2, list2_);
	DDX_Control(DX, IDC_RENAME, rename_);
}


BEGIN_MESSAGE_MAP(RenamePanelAdvanced, ImgPage)
	ON_EN_SETFOCUS(IDC_EXPR, OnExprSetFocus)
//	ON_LBN_SELCHANGE(IDC_LIST, &RenamePanelAdvanced::OnRuleSelChange)
//	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST2, &RenamePanelAdvanced::OnSelectionChanging)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &RenamePanelAdvanced::OnSelectionChanged)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST2, &RenamePanelAdvanced::OnBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST2, &RenamePanelAdvanced::OnRuleNameChanged)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST2, &RenamePanelAdvanced::OnGetDispInfo)
	ON_BN_CLICKED(IDC_VERIFY, &RenamePanelAdvanced::OnVerifyExpr)
	ON_NOTIFY(TBN_DROPDOWN, IDC_TOOLBAR, OnTbDropDown)
	ON_COMMAND(ID_RENAME, OnRenameRule)
	ON_BN_CLICKED(IDC_FILE_ONLY, &RenamePanelAdvanced::OnParamChanged)
	ON_BN_CLICKED(IDC_FULL_PATH, &RenamePanelAdvanced::OnParamChanged)
	ON_EN_CHANGE(IDC_TEXT, &RenamePanelAdvanced::OnParamChanged)
	ON_EN_CHANGE(IDC_COUNTER, &RenamePanelAdvanced::OnParamChanged)
END_MESSAGE_MAP()


void RenamePanelAdvanced::OnParamChanged()
{
	ParamChanged();
}


void RenamePanelAdvanced::OnRenameRule()
{
	list2_.SetFocus();
	list2_.EditLabel(current_rule_);
}


void RenamePanelAdvanced::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	size_t item= disp_info->item.iItem;

	if (item >= rule_defs_.size())
		return;

	rule_defs_[item];

	if ((disp_info->item.mask & LVIF_TEXT))
	{
		disp_info->item.pszText[0] = _T('\0');

		switch (disp_info->item.iSubItem)
		{
		case 0:	// name?
			_tcsncpy(disp_info->item.pszText, rule_defs_[item].name_.c_str(), disp_info->item.cchTextMax);
			break;
		}
	}

	//if ((disp_info->item.mask & LVIF_IMAGE))
	//{
	//	if (disp_info->item.iSubItem == 0)
	//		disp_info->item.iImage = folder->GetIconIndex(false);
	//}

	return;
}


void RenamePanelAdvanced::OnBeginLabelEdit(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;

	if (CEdit* edit= list2_.GetEditControl())
		edit->LimitText(NAME_MAX_LENGTH);
}


void RenamePanelAdvanced::OnRuleNameChanged(NMHDR* nmhdr, LRESULT* result)
{
	NMLVDISPINFO* info= reinterpret_cast<NMLVDISPINFO*>(nmhdr);
	*result = 0;

	if (info->item.pszText == 0 || *info->item.pszText == 0)
		return;

	int index= info->item.iItem;
	if (index >= 0 && index < rule_defs_.size())
	{
		rule_defs_[index].name_ = info->item.pszText;
		*result = 1;
	}
}


//void RenamePanelAdvanced::OnSelectionChanging(NMHDR* nmhdr, LRESULT* result)
//{
//	NMLISTVIEW* lv= reinterpret_cast<NMLISTVIEW*>(nmhdr);
//
//	if (list2_.m_hWnd && (lv->uChanged & LVIF_STATE) != 0)
//	{
//		// item becomes unselected?
//		if ((lv->uNewState & LVIS_SELECTED) == 0 && (lv->uOldState & LVIS_SELECTED) != 0)
//		{
//			int item= lv->iItem;
//
//			if (static_cast<size_t>(item) < rule_defs_.size())
//			{
//				if (ReadRule(current_rule_))	// is current rule OK?
//				{
//					ShowRule(item, false);		// switch to the new one
//					ParamChanged();
//				}
//				else
//				{
//					*result = 1;	// cancel changes, current rule is invalid
//					return;
//				}
//			}
//		}
//		else if ((lv->uNewState & LVIS_SELECTED) && (lv->uOldState & LVIS_SELECTED) == 0)
//		{
//			int item= lv->iItem;
//
//			if (static_cast<size_t>(item) < rule_defs_.size())
//			{
//				if (!ReadRule(current_rule_, true))	// is current rule invalid?
//				{
//					*result = 1;	// cancel changes
//					return;
//				}
//			}
//		}
//	}
//
//	*result = 0;	// carry on changes
//}


void RenamePanelAdvanced::OnSelectionChanged(NMHDR* nmhdr, LRESULT* result)
{
	if (in_update_)
		return;

	if (list2_.m_hWnd)
	{
		POSITION pos= list2_.GetFirstSelectedItemPosition();
		int item= list2_.GetNextSelectedItem(pos);

		if (static_cast<size_t>(item) < rule_defs_.size())
		{
			if (ReadRule(current_rule_))
			{
				ShowRule(item, false);
				ParamChanged();
			}
			else
			{
				Block b(in_update_);
				list2_.SetItemState(current_rule_, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
	}
}


void RenamePanelAdvanced::OnExprSetFocus()
{
	if (expression_.m_hWnd)
	{
		int len= expression_.GetWindowTextLength();
		expression_.SetSel(-1, 0);//, true);
	}
}


const int INSERT_FILE= 10;
const int INSERT_ATTRIB= 11;
const int INSERT_XMP= 12;


BOOL RenamePanelAdvanced::OnInitDialog()
{
	try
	{
		ImgPage::OnInitDialog();

		CreateScriptEditingFont(expr_font_);

		const int MAX_EXPR= 65000;
//		expression_.LimitText(MAX_EXPR);
		expression_.SetFont(&expr_font_);
		expression_.SetWindowText(rename_rule_.c_str());

		dlg_resize_map_.BuildMap(this);

		dlg_resize_map_.SetWndResizing(IDC_LIST2, DlgAutoResize::RESIZE_V);
		//dlg_resize_map_.SetWndResizing(IDC_NAME, DlgAutoResize::RESIZE_H);
		dlg_resize_map_.SetWndResizing(IDC_RENAME, DlgAutoResize::MOVE_V);
		dlg_resize_map_.SetWndResizing(IDC_TEXT, DlgAutoResize::RESIZE_H);
		dlg_resize_map_.SetWndResizing(IDC_LABEL_4, DlgAutoResize::MOVE_H);
		dlg_resize_map_.SetWndResizing(IDC_COUNTER, DlgAutoResize::MOVE_H);
		dlg_resize_map_.SetWndResizing(IDC_SEPARATOR, DlgAutoResize::RESIZE_H);
		dlg_resize_map_.SetWndResizing(IDC_EXPR, DlgAutoResize::RESIZE);
		dlg_resize_map_.SetWndResizing(IDC_LABEL_3, DlgAutoResize::MOVE_V);
		dlg_resize_map_.SetWndResizing(IDC_STATUS, DlgAutoResize::MOVE_V_RESIZE_H);
		dlg_resize_map_.SetWndResizing(IDC_VERIFY, DlgAutoResize::MOVE);
		dlg_resize_map_.SetWndResizing(IDC_FILE_ONLY, DlgAutoResize::MOVE_V);
		dlg_resize_map_.SetWndResizing(IDC_FULL_PATH, DlgAutoResize::MOVE_V);
		dlg_resize_map_.SetWndResizing(IDC_LABEL_5, DlgAutoResize::MOVE_V);
		dlg_resize_map_.SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V);
		dlg_resize_map_.SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_V_RESIZE_H);

		int cmds[]= { INSERT_FILE, INSERT_ATTRIB, INSERT_XMP };
		insert_.SetPadding(8, 14);
		insert_.AddButtons("VVV", cmds, 0, IDS_INSERT_TEXT3);
		insert_.SetOnIdleUpdateState(false);

		CRect client(0,0,0,0);
		list2_.GetClientRect(client);
		list2_.InsertColumn(0, _T("名称"), LVCFMT_LEFT, client.Width() - 1);
		for (size_t i= 0; i < rule_defs_.size(); ++i)
			list2_.InsertItem(static_cast<int>(i), rule_defs_[i].name_.c_str());

		int rename[]= { ID_RENAME };
		rename_.SetPadding(8, 11);
		rename_.AddButtons("p", rename, IDB_EDIT, 0);
		rename_.SetOnIdleUpdateState(false);

		//list_.SetSel(0);
		//list_.SetItemHeight(0, list_.GetItemHeight(0) * 13 / 10);

		// some reasonable text length limits

		const int LIMIT= 100;
		//name_.LimitText(LIMIT);
		text_.LimitText(LIMIT);
		counter_.LimitText(LIMIT);

		const int tab_step= 45;
		info_text_.SetTabStops(tab_step);
		info_text_.SetWindowText(GetImgAttribsHelpString(IAH_BASIC_INFO | IAH_FILE_INFO | IAH_TAG_INFO).c_str());

		SetDlgItemText(IDC_TEXT, input_text_);
		SetDlgItemInt(IDC_COUNTER, starting_number_);

		ShowRule(current_rule_, true);

		return true;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void RenamePanelAdvanced::OnCancel()
{}

void RenamePanelAdvanced::OnOK()
{}


void RenamePanelAdvanced::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	CPoint pos(info->rcButton.left, info->rcButton.bottom);
	insert_.ClientToScreen(&pos);

	switch (info->iItem)
	{
	case INSERT_FILE:
		{
			if (const TCHAR* text= ImgFileAttribsPopup(this, pos))
				expression_.ReplaceSel(text, true);
		}
		break;

	case INSERT_ATTRIB:
		{
			if (const TCHAR* text= ImgAttribsPopup(this, pos))
				expression_.ReplaceSel(text, true);
		}
		break;

	case INSERT_XMP:
		{
			if (const TCHAR* text= ImgMetaAttribsPopup(this, pos))
				expression_.ReplaceSel(text, true);
		}
		break;
	}
}


//void RenamePanelAdvanced::OnRuleSelChange()
//{
//	if (in_update_)
//		return;
//
//	Block b(in_update_);
//
//	if (list_.m_hWnd)
//	{
//		int sel= list_.GetCurSel();
//
//		if (ReadRule(current_rule_))
//		{
//			ShowRule(sel);
//		}
//		else
//			list_.SetCurSel(static_cast<int>(current_rule_));
//	}
//}


void RenamePanelAdvanced::ShowRule(int rule, bool update_selection)
{
	Block b(in_update_);

	if (rule >= 0 && rule < rule_defs_.size())
	{
		//name_.SetWindowText(rule_defs_[rule].name_.c_str());
		expression_.SetWindowText(rule_defs_[rule].expression_.c_str());

		CheckRadioButton(IDC_FILE_ONLY, IDC_FULL_PATH, rule_defs_[rule].rule_outputs_file_name_only_ ? IDC_FILE_ONLY : IDC_FULL_PATH);

		current_rule_ = rule;

		//if (list_.m_hWnd)
		//	list_.SetCurSel(rule);

		if (list2_.m_hWnd && update_selection)
			list2_.SetItemState(rule, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}


bool RenamePanelAdvanced::ReadRule(size_t rule, bool silent)
{
	try
	{
		//CString caption;
		//name_.GetWindowText(caption);
		//if (caption.IsEmpty())
		//{
		//	new BalloonMsg(&name_, _T("Rule Name Expected"),
		//		_T("Please enter rename rule name, it cannot be empty."), BalloonMsg::IERROR);
		//	return false;
		//}

		// rename dlg doesn't support full path:
		bool file_name_only= true; // GetCheckedRadioButton(IDC_FILE_ONLY, IDC_FULL_PATH) == IDC_FILE_ONLY;

		CString expr;
		expression_.GetWindowText(expr);

		String err_msg;
		if (!VerifyExpr(expr, &err_msg))
		{
			if (!silent)
			{
				SetDlgItemText(IDC_STATUS, (_T("错误: ") + err_msg).c_str());
				new BalloonMsg(expression_.GetWnd(), _T("无效表达式"), err_msg.c_str(), BalloonMsg::IERROR);
			}
			return false;
		}

		//rule_defs_[rule].name_ = static_cast<const TCHAR*>(caption);
		rule_defs_[rule].expression_ = expr;
		rule_defs_[rule].rule_outputs_file_name_only_ = file_name_only;

//		CString str;
//		list_.GetText(static_cast<int>(rule), str);
		//if (str != caption)
		//{
		//	list_.DeleteString(static_cast<int>(rule));
		//	list_.InsertString(static_cast<int>(rule), caption);
		//}

		return true;
	}
	CATCH_ALL

	return false;
}


void RenamePanelAdvanced::OnVerifyExpr()
{
	VerifyCurrentExpr();

	ParamChanged();
}


bool RenamePanelAdvanced::VerifyCurrentExpr()
{
	bool ok= false;
	try
	{
		CString expr;
		expression_.GetWindowText(expr);

		String err_msg;
		if (VerifyExpr(expr, &err_msg))
		{
			SetDlgItemText(IDC_STATUS, (_T("结果: ") + err_msg).c_str());
			ok = true;
		}
		else
			SetDlgItemText(IDC_STATUS, (_T("错误: ") + err_msg).c_str());
	}
	CATCH_ALL

	return ok;
}


bool RenamePanelAdvanced::VerifyExpr(const CString& expr, String* err_message)
{
	if (expr.IsEmpty())
	{
		if (err_message)
			*err_message = _T("请输入有效的表达式, 不能为空.");

		return false;
	}

	GetDlgItemText(IDC_TEXT, input_text_);
	starting_number_ = GetDlgItemInt(IDC_COUNTER);

	ExprCalculator calc(expr);
	calc.DefineGlobalVar("text", WStr2UTF8(input_text_).c_str());
	calc.DefineGlobalVar("number", starting_number_);

	String err_msg;

	bool valid= calc.IsValid(err_msg, *example_, MAX_SCRIPT_STEPS);

	if (err_message)
		*err_message = err_msg;

	return valid;
}


bool RenamePanelAdvanced::UpdateData()
{
	Block b(in_update_);

	GetDlgItemText(IDC_TEXT, input_text_);
	starting_number_ = GetDlgItemInt(IDC_COUNTER);

	if (ReadRule(current_rule_))
		return true;

	return false;
}


void FixDirSeparators(String& path)
{
	boost::algorithm::replace_all(path, L"/", L"\\");
}


bool RenamePanelAdvanced::GenerateFileName(State state, const PhotoInfo* input, Path* output_name)
{
	if (state == Start)
	{
		lua_.reset(new Lua());

		InitializeLua(*lua_);

		DefRoundFn(*lua_);
		DefGlobalImgTable(*lua_);

		CString rule;
		expression_.GetWindowText(rule);

		String err_msg;
		if (!VerifyExpr(rule, &err_msg))
			return false;

		// todo: text, number
		lua_->PushValue(WStr2UTF8(input_text_).c_str());
		lua_->SetGlobal("text");
		lua_->PushValue(starting_number_);
		lua_->SetGlobal("number");
//	calc.DefineGlobalVar("text", WStr2UTF8(input_text_).c_str());
//	calc.DefineGlobalVar("number", starting_number_);

		std::string expr;
		TStringToAnsiString(rule, expr);
		std::ostringstream ost;
		ost << "function rename_function_(img, file) " + expr + "\r\nend";

		const char* msg= 0;
		if (lua_->LoadBuffer(ost.str().c_str(), msg))
		{
			lua_->Call(0);
			lua_->PopTop();
		}
		else
			return false;
	}
	else if (state == Next)
	{
		PassParams(*lua_, *input);

		lua_->GetGlobal("rename_function_");
		lua_->GetGlobal("img");
		lua_->GetGlobal("file");

		lua_->Call(2, MAX_SCRIPT_STEPS, 0);

		double num_result= 0.0;
		String text_result;
		if (lua_->GetTop(num_result))
			text_result = boost::lexical_cast<String>(num_result);
		else
			AnsiStringToTString(lua_->GetTop(), text_result);

		lua_->PopTop();

		bool file_name_only= true; // GetCheckedRadioButton(IDC_FILE_ONLY, IDC_FULL_PATH) == IDC_FILE_ONLY;

		if (file_name_only)
			text_result = ReplaceIllegalChars(text_result, _T('-'));
		else
			FixDirSeparators(text_result);

		*output_name = input->GetPhysicalPath();

		if (file_name_only)
			output_name->ReplaceFileNameExt(text_result.c_str());
		else
			*output_name = text_result; //->ReplaceFileNameExt(text_result.c_str());
	}
	else if (state == End)
	{
		lua_.reset();
	}

	return true;
}


bool RenamePanelAdvanced::ShowFullPath()
{
	bool file_name_only= true; // GetCheckedRadioButton(IDC_FILE_ONLY, IDC_FULL_PATH) == IDC_FILE_ONLY;
	// show full path, if script is expected to return full path
	return !file_name_only;
}
