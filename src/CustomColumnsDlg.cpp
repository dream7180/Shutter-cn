/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CustomColumnsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CustomColumnsDlg.h"
#include "BalloonMsg.h"
#include "Block.h"
#include "CatchAll.h"
#include "ExprCalculator.h"
#include "StringConversions.h"
#include "Attributes.h"


// CustomColumnsDlg dialog

CustomColumnsDlg::CustomColumnsDlg(CWnd* parent, const CustomColumns& defs, const PhotoInfo& example)
	: DialogChild(CustomColumnsDlg::IDD, parent), col_defs_(defs), example_(example)
{
	current_column_ = 0;
	in_update_ = false;
}


CustomColumnsDlg::~CustomColumnsDlg()
{}


void CustomColumnsDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	DDX_Control(DX, IDC_CAPTION, name_);
	DDX_Control(DX, IDC_EXPR, expression_);
	DDX_Control(DX, IDC_LIST, list_);
	DDX_Control(DX, IDC_SHOW, show_col_);
	DDX_Control(DX, IDC_LABEL_2, info_text_);
	DDX_Control(DX, IDC_TOOLBAR, insert_);
}


BEGIN_MESSAGE_MAP(CustomColumnsDlg, DialogChild)
	ON_LBN_SELCHANGE(IDC_LIST, &CustomColumnsDlg::OnColSelChange)
	ON_BN_CLICKED(IDC_VERIFY, &CustomColumnsDlg::OnVerifyExpr)
	ON_BN_CLICKED(IDC_RESET, &CustomColumnsDlg::OnResetAll)
//	ON_WM_SIZE()
	ON_NOTIFY(TBN_DROPDOWN, IDC_TOOLBAR, OnTbDropDown)
	ON_EN_SETFOCUS(IDC_EXPR, OnExprSetFocus)
END_MESSAGE_MAP()


void CustomColumnsDlg::OnExprSetFocus()
{
	if (expression_.m_hWnd)
	{
		int len= expression_.GetWindowTextLength();
		expression_.SetSel(-1, 0, true);
	}
}


const int INSERT_ATTRIB= 10;
const int INSERT_TAG= 11;
const int INSERT_XMP= 12;
const int INSERT_FILE= 13;


BOOL CustomColumnsDlg::OnInitDialog()
{
	try
	{
		DialogChild::OnInitDialog();

		BuildResizingMap();

		SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE_V);
		SetWndResizing(IDOK, DlgAutoResize::MOVE);
		SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
		SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_EXPR, DlgAutoResize::RESIZE);
		SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_LABEL_3, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_STATUS, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_VERIFY, DlgAutoResize::MOVE);
		//SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);

		//SubclassHelpBtn(_T("CustomColumns.htm"));

		int cmds[]= { INSERT_ATTRIB, /*INSERT_TAG,*/ INSERT_XMP, INSERT_FILE };
		insert_.SetPadding(8, 14);
		insert_.AddButtons("VVV", cmds, 0, IDS_INSERT_TEXT2);
		insert_.SetOnIdleUpdateState(false);
		//insert_.EnableButton(INSERT_TAG, collection_.GetCount() > 0);

		const int tab_step= 45;
		info_text_.SetTabStops(tab_step);
		info_text_.SetWindowText(GetImgAttribsHelpString(IAH_BASIC_INFO).c_str());

		for (size_t i= 0; i < col_defs_.size(); ++i)
			list_.AddString(col_defs_[i].caption_.c_str());

		list_.SetSel(0);
		list_.SetItemHeight(0, list_.GetItemHeight(0) * 13 / 10);

		// some reasonable text length limits

		const int LIMIT= 100;
		name_.LimitText(LIMIT);

		CreateScriptEditingFont(expr_font_);

		const int MAX_EXPR= 65000;
		//expression_.LimitText(MAX_EXPR);
		expression_.SetFont(&expr_font_);

		ShowCol(0);

		return TRUE;  // return TRUE unless you set the focus to a control
		// EXCEPTION: OCX Property Pages should return FALSE
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


void CustomColumnsDlg::OnColSelChange()
{
	if (in_update_)
		return;

	Block b(in_update_);

	if (list_.m_hWnd)
	{
		int sel= list_.GetCurSel();

		if (ReadCol(current_column_))
			ShowCol(sel);
		else
			list_.SetCurSel(static_cast<int>(current_column_));
	}
}


void CustomColumnsDlg::ShowCol(int col)
{
	if (col >= 0)
	{
		name_.SetWindowText(col_defs_[col].caption_.c_str());
		expression_.SetWindowText(col_defs_[col].expression_.c_str());
		show_col_.SetCheck(col_defs_[col].visible_);

		current_column_ = col;
	}
}


bool CustomColumnsDlg::ReadCol(size_t col)
{
	try
	{
		CString caption;
		name_.GetWindowText(caption);
		if (caption.IsEmpty())
		{
			new BalloonMsg(&name_, _T("需要列名称"),
				_T("请输入列标题, 不能为空."), BalloonMsg::IERROR);
			return false;
		}

		CString expr;
		expression_.GetWindowText(expr);

		String err_msg;
		if (!VerifyExpr(expr, &err_msg))
		{
			SetDlgItemText(IDC_STATUS, (_T("错误: ") + err_msg).c_str());
			new BalloonMsg(expression_.GetWnd(), _T("无效的表达式"), err_msg.c_str(), BalloonMsg::IERROR);
			return false;
		}

		col_defs_[col].caption_ = static_cast<const TCHAR*>(caption);
		col_defs_[col].expression_ = expr;
		col_defs_[col].visible_ = show_col_.GetCheck() != 0;

		CString str;
		list_.GetText(static_cast<int>(col), str);
		if (str != caption)
		{
			list_.DeleteString(static_cast<int>(col));
			list_.InsertString(static_cast<int>(col), caption);
		}

		return true;
	}
	CATCH_ALL

	return false;
}


void CustomColumnsDlg::OnVerifyExpr()
{
	try
	{
		CString expr;
		expression_.GetWindowText(expr);

		String err_msg;
		if (VerifyExpr(expr, &err_msg))
			SetDlgItemText(IDC_STATUS, (_T("结果: ") + err_msg).c_str());
		else
			SetDlgItemText(IDC_STATUS, (_T("错误: ") + err_msg).c_str());
	}
	CATCH_ALL
}


bool CustomColumnsDlg::VerifyExpr(const CString& expr, String* err_message)
{

	if (expr.IsEmpty())
	{
		if (err_message)
			*err_message = _T("请输入有效的表达式, 不能为空.");

		return false;
	}

	ExprCalculator calc(expr);
	String err_msg;

	bool valid= calc.IsValid(err_msg, example_);

	if (err_message)
		*err_message = err_msg;

	return valid;
}


void CustomColumnsDlg::OnOK()
{
	Block b(in_update_);

	if (ReadCol(current_column_))
		EndDialog(IDOK);
}


void CustomColumnsDlg::OnCancel()
{
	EndDialog(IDCANCEL);
}


const CustomColumns& CustomColumnsDlg::GetColumns() const
{
	return col_defs_;
}


void CustomColumnsDlg::OnSize(UINT type, int cx, int cy)
{
	DialogChild::OnSize(type, cx, cy);

	Resize();
}


void CustomColumnsDlg::OnResetAll()
{
	col_defs_.assign(CustomColumns());

	Block b(in_update_);

	list_.ResetContent();
	for (size_t i= 0; i < col_defs_.size(); ++i)
		list_.AddString(col_defs_[i].caption_.c_str());

	ShowCol(0);
}


void CustomColumnsDlg::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	CPoint pos(info->rcButton.left, info->rcButton.bottom);
	insert_.ClientToScreen(&pos);

	switch (info->iItem)
	{
	case INSERT_ATTRIB:
		if (const TCHAR* text= ImgAttribsPopup(this, pos))
			expression_.ReplaceSel(text, true);
		break;

	case INSERT_TAG:
		// OnViewTags(pos);
		break;

	case INSERT_XMP:
		if (const TCHAR* text= ImgMetaAttribsPopup(this, pos))
			expression_.ReplaceSel(text, true);
		break;

	case INSERT_FILE:
		if (const TCHAR* text= ImgFileAttribsPopup(this, pos))
			expression_.ReplaceSel(text, true);
		break;
	}
}
