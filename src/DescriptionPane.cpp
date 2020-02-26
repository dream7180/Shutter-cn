/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DescriptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DescriptionPane.h"
#include "Config.h"
#include "CatchAll.h"
#include "EnableCtrl.h"
#include "DlgListCtrl.h"
#include "PropertyField.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Property;


DescriptionPane::DescriptionPane(AutoCompletePopup& auto_complete, boost::ptr_vector<Field>& fields)
 : Pane(auto_complete, fields, DescriptionPane::IDD), edit_(*(new CEdit))
{
	ASSERT(!fields.empty());
	fields[0].edit = std::auto_ptr<CEdit>(&edit_);	// field takes ownership of CEdit object

	text_ = 0;
//	unicode_system_ = ::GetVersion() < DWORD(0x80000000);
//	edit_ = 0;
//	is_modified_ = false;
}


void DescriptionPane::DoDataExchange(CDataExchange* DX)
{
	Pane::DoDataExchange(DX);
	DDX_Control(DX, IDC_TOOLBAR, toolBar_);
}


BEGIN_MESSAGE_MAP(DescriptionPane, Pane)
//	ON_EN_CHANGE(Pane::EDIT_ID, OnChangeDescription)
	ON_COMMAND_RANGE(ID_INSERT_CHAR_00, ID_INSERT_CHAR_19, OnSymbol)
	ON_WM_SIZE()
//	ON_CONTROL(EN_SETFOCUS, Pane::EDIT_ID, OnFocusSet)
END_MESSAGE_MAP()


void DescriptionPane::Create(CWnd* parent)
{
	m_pParentWnd = parent;
	if (!CDialog::Create(IDD, parent) || m_hWnd == 0)
		throw std::exception("DescriptionPane window creation failed");
}


void DescriptionPane::OnSize(UINT type, int cx, int cy)
{
	Pane::OnSize(type, cx, cy);
	resize_.Resize();
}


BOOL DescriptionPane::OnInitDialog()
{
	try
	{
		return InitDlg();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


BOOL DescriptionPane::InitDlg()
{
	if (!edit_.SubclassDlgItem(IDC_DESCRIPTION, this))
	{
		EndDialog(IDCANCEL);
		return true;
	}

	edit_.SetDlgCtrlID(Pane::EDIT_ID);

	LOGFONT lf= g_Settings.description_font_;
	//lf.lfHeight = -18;
	fndEdit_.CreateFontIndirect(&lf);

	edit_.SetFont(&fndEdit_);
	//::SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(fndEdit_.GetSafeHandle()), 0);
//	::SendMessageW(edit_, EM_SETLIMITTEXT, PhotoAttr::MAX_DESC_LEN, 0);

	Pane::OnInitDialog();

	BuildToolbar();

	resize_.BuildMap(this);

	resize_.SetWndResizing(Pane::EDIT_ID, DlgAutoResize::RESIZE);
//	resize_.SetWndResizing(IDC_TOOLBAR, DlgAutoResize::MOVE_H);

	if (text_)
		edit_.SetWindowText(text_->c_str());

	return false;  // return TRUE unless you set the focus to a control
}


void DescriptionPane::OnOK()
{}

void DescriptionPane::OnCancel()
{}


void DescriptionPane::BuildToolbar()
{
	static const int commands[]=
	{
		ID_INSERT_CHAR_00, ID_INSERT_CHAR_01, ID_INSERT_CHAR_02, ID_INSERT_CHAR_03,
		ID_INSERT_CHAR_04, ID_INSERT_CHAR_05, ID_INSERT_CHAR_06, ID_INSERT_CHAR_07,
		ID_INSERT_CHAR_08, ID_INSERT_CHAR_09, ID_INSERT_CHAR_10, ID_INSERT_CHAR_11,
		ID_INSERT_CHAR_12, ID_INSERT_CHAR_13, ID_INSERT_CHAR_14, ID_INSERT_CHAR_15,
		ID_INSERT_CHAR_16, ID_INSERT_CHAR_17, ID_INSERT_CHAR_18, ID_INSERT_CHAR_19
	};

	toolBar_.SetPadding(4, 5);
#ifdef UNICODE
	toolBar_.AddButtons("pppppppppppppppppppp", commands, IDB_CHARACTERS);
#else
	toolBar_.AddButtons("pppppppppp..........", commands, IDB_CHARACTERS);
#endif
	toolBar_.SetRows(array_count(commands) / 4, false, 0);
}


void DescriptionPane::OnSymbol(UINT cmdId)
{
	cmdId -= ID_INSERT_CHAR_00;

	ASSERT(cmdId < 20);

	TCHAR text[2]= { 0, 0 };

#ifdef UNICODE
	static const TCHAR symbols[]= _T("\xBD\xBC\xA9\xAE\xB1\xB0\xB2\xB3\xBB\xAB\x2018\x2019\x201C\x201D\x201A\x201E\x2013\x2014\x2122\x2116");
#else
	static const TCHAR symbols[]= _T("\xBD\xBC\xA9\xAE\xB1\xB0\xB2\xB3\xBB\xAB\x2e\x2e\x2e\x2e\x2e\x2e\x2e\x2e\x2e\x2e");
#endif

	text[0] = symbols[cmdId];

	edit_.ReplaceSel(text, true);
//	::SendMessage(edit_, EM_REPLACESEL, true, LPARAM(text));
}


//void DescriptionPane::OnChangeDescription()
//{
//	is_modified_ = true;
//}


//void DescriptionPane::OnFocusSet()
//{
//	if (DlgListCtrl* parent= dynamic_cast<DlgListCtrl*>(GetParent()))
//		if (CWnd* ctrl= GetDlgItem(Pane::EDIT_ID))
//		{
//			WINDOWPLACEMENT wp;
//			if (ctrl->GetWindowPlacement(&wp))
//				parent->EnsureVisible(this, wp.rcNormalPosition);
//		}
//}


//void DescriptionPane::ReadText()
//{
//	if (text_)
//	{
//		CString text;
//		edit_.GetWindowText(text);
//		*text_ = text;
//	}
//}
//
//
//void DescriptionPane::Reset()
//{
//	if (text_ && edit_.m_hWnd)
//		edit_.SetWindowText(text_->c_str());
//
//	is_modified_ = false;
//}
//
//
//bool DescriptionPane::IsModified() const
//{
//	return is_modified_;
//}
