/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TransferDlg.h"
#include "Path.h"
#include "FolderSelect.h"
#include "TransferFiles.h"
#include "EnableCtrl.h"
#include "ItemIdList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {
	const TCHAR* REGISTRY_ENTRY_TRANSFER= _T("TransferDlg");
	const TCHAR* REG_OPERATION			= _T("Operation");
	const TCHAR* REG_PATTERN			= _T("RenamePat");
	const TCHAR* REG_FROM				= _T("FromDir");
	const TCHAR* REG_TO					= _T("ToDir");
	const TCHAR* REG_RENAME				= _T("Rename");
	const TCHAR* REG_READ_ONLY			= _T("ReadOnly");
}

/////////////////////////////////////////////////////////////////////////////
// CTransferDlg dialog
#define FLAG_CHR _T('%')


CTransferDlg::CTransferDlg(CWnd* parent /*=NULL*/)
	: CDialogChild(CTransferDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(CTransferDlg)
	pattern_ = _T("");
	from_ = _T("");
	to_ = _T("");
	operation_ = 0;
	rename_ = FALSE;
	read_only_ = FALSE;
	//}}AFX_DATA_INIT
}


void CTransferDlg::DoDataExchange(CDataExchange* DX)
{
	CDialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CTransferDlg)
	DDX_Control(DX, IDC_LABEL_1, label_to_wnd_);
	DDX_Control(DX, IDC_TO_BTN, btn_to_);
	DDX_Control(DX, IDC_TO, edit_to_);
	DDX_Control(DX, IDC_FROM, edit_from_);
	DDX_Control(DX, IDC_PATTERN, edit_pattern_);
	DDX_Control(DX, IDC_EXAMPLE, example_wnd_);
	DDX_Text(DX, IDC_PATTERN, pattern_);
	DDX_Text(DX, IDC_FROM, from_);
	DDX_Text(DX, IDC_TO, to_);
	DDX_Radio(DX, IDC_COPY, operation_);
	DDX_Check(DX, IDC_RENAME, rename_);
	DDX_Check(DX, IDC_READ_ONLY, read_only_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTransferDlg, CDialogChild)
	//{{AFX_MSG_MAP(CTransferDlg)
	ON_EN_CHANGE(IDC_PATTERN, OnChangePattern)
	ON_BN_CLICKED(IDC_FROM_BTN, OnFromBtn)
	ON_BN_CLICKED(IDC_TO_BTN, OnToBtn)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_MOVE, OnMove)
	ON_BN_CLICKED(IDC_BATCH, OnBatch)
	//}}AFX_MSG_MAP
	ON_NOTIFY(TBN_DROPDOWN, IDC_SYMBOLS, OnTbDropDown)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransferDlg message handlers

BOOL CTransferDlg::OnInitDialog()
{
	CWinApp* app= AfxGetApp();
	operation_	= app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_OPERATION, 0);
	pattern_	= app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_PATTERN, _T("%Y-%m-%d_%H-%M-%S"));
	from_		= app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_FROM, _T("d:\\"));
	to_			= app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_TO, _T(""));
	rename_		= app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_RENAME, rename_);
	read_only_		= app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_READ_ONLY, read_only_);

	if (to_.IsEmpty())
	{
		ITEMIDLIST* pidl= 0;
		if (::SHGetSpecialFolderLocation(m_hWnd, CSIDL_MYPICTURES, &pidl) == NOERROR && pidl != 0)
		{
			CItemIdList folder(pidl, false);
			to_ = folder.GetPath();
		}

//		TCHAR buf[MAX_PATH];
//		if (SUCCEEDED(::SHGetFolderPath(*this, CSIDL_MYPICTURES, 0, SHGFP_TYPE_CURRENT, buf)))
//			to_ = buf;
	}

	CDialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolTransfer.htm"));

	symbols_wnd_.SubclassDlgItem(IDC_SYMBOLS, this);
	int cmd_id[]= { IDC_SYMBOLS };
	symbols_wnd_.SetPadding(2, 8);
	symbols_wnd_.AddButtons("v", cmd_id, 0);

	example_wnd_.ModifyStyle(0, SS_ENDELLIPSIS);

	edit_pattern_.FileNameEditing(true);

	OnChangePattern();

	UpdatePatternCtrl();

	OperationChanged();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CTransferDlg::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case IDC_SYMBOLS:		// view mode
		SymbolsPopupMenu();
		break;
	}
}


void CTransferDlg::SymbolsPopupMenu()
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_TRANSFER_SYMBOLS))
		return;

	CMenu* popup= menu.GetSubMenu(0);

	CRect rect;
	symbols_wnd_.GetRect(IDC_SYMBOLS, rect);
	symbols_wnd_.ClientToScreen(rect);

	int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, rect.left, rect.bottom, this);

	if (cmd == 0)
		return;

	switch (cmd)
	{
	case ID_SYMBOL_DATE_TIME:
		edit_pattern_.SetWindowText(_T("%Y-%m-%d_%H-%M-%S"));
		break;

	case ID_SYMBOL_FILE_DATE:
		edit_pattern_.SetWindowText(_T("%f_%Y-%m-%d"));
		break;

	default:
		{
			cmd -= ID_SYMBOL_1;
			static const TCHAR flags[]= _T("YymdHMSfn");

			ASSERT(cmd >= 0 && cmd < _tcslen(flags));
			TCHAR insert[]= { FLAG_CHR, flags[cmd], 0 };

			edit_pattern_.ReplaceSel(insert, true);
		}
		break;
	}
}


extern CString ParsePattern(const TCHAR* filename, COleDateTime date, int serial, const TCHAR* pattern)
{
	ASSERT(pattern && filename);

	CPath file(filename);

	CString out;

	bool flag= false;

	for (const TCHAR* chr= pattern; *chr; ++chr)
	{
		TCHAR chr= *chr;

		if (chr == FLAG_CHR)
		{
			if (!flag)
			{
				flag = true;
				continue;
			}
			flag = false;
		}
		else
		{
			if (flag)
			{
				switch (chr)
				{
				case _T('Y'):
				case _T('y'):
				case _T('m'):
				case _T('d'):
				case _T('H'):
				case _T('M'):
				case _T('S'):
					{
						TCHAR format[]= { _T('%'), chr, 0 };
						out += date.Format(format);
					}
					break;

				case _T('f'):
					{
						out += file.GetFileName().c_str();
					}
					break;

				case _T('n'):
					{
						oStringstream ost;
						ost.fill(_T('0'));
						ost.width(5);
						ost << serial;
						out += ost.str().c_str();
					}
					break;

				case FLAG_CHR:
					out += FLAG_CHR;
					break;

				default:
					out += _T("-");
					break;
				}
				flag = false;
				continue;
			}
		}

		// substitute illegal chars with underscore
		switch (chr)
		{
		case _T('\\'):
		case _T('/'):
		case _T('?'):
		case _T('*'):
		case _T(':'):
		case _T('\'):
		case _T('>'):
		case _T('<'):
		case _T('|'):
			chr = _T('_');
			break;
		default:
			break;
		}

		out += chr;
	}

	CString ext= file.GetExtension().c_str();
	if (!ext.IsEmpty())
		out += _T('.') + ext;

	return out;
}


void CTransferDlg::OnChangePattern()
{
	if (edit_pattern_.m_hWnd == 0)
		return;

	CString text;
	edit_pattern_.GetWindowText(text);

	text = ParsePattern(_T("DSC01234.JPG"), COleDateTime(2002, 12, 31, 23, 59, 58), 123, text);

	if (!text.IsEmpty())
		example_wnd_.SetWindowText(text);
}


void CTransferDlg::OnFromBtn()
{
	OnBrowse(edit_from_, _T("Select Source Folder"));
}

void CTransferDlg::OnToBtn()
{
	OnBrowse(edit_to_, _T("Select Destination Folder"));
}


void CTransferDlg::OnBrowse(CWnd& wnd, const TCHAR* caption)
{
	CString path;
	wnd.GetWindowText(path);

	if (path.IsEmpty())
		path = _T("c:\\");

	CFolderSelect fs(this);
	CString str= fs.DoSelectPath(caption, path);

	if (str.IsEmpty())
		return;

	wnd.SetWindowText(str);
}


void CTransferDlg::OnOK()
{
	if (!UpdateData())
		return;

	CWinApp* app= AfxGetApp();
	app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_OPERATION, operation_);
	app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_PATTERN, pattern_);
	app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_FROM, from_);
	app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_TO, to_);
	app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_RENAME, rename_);
	app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_READ_ONLY, read_only_);

	EndDialog(IDOK);

	// for batch rename dest folder is same as source
	CString& dest_path= operation_ == 2 ? from_ : to_;

	CTransferFiles files(from_, dest_path, !!read_only_, rename_ ? pattern_ : _T(""));

	files.Transfer(operation_ == 0);
}


void CTransferDlg::OnRename()
{
	UpdatePatternCtrl();
}


void CTransferDlg::UpdatePatternCtrl()
{
	bool rename= IsDlgButtonChecked(IDC_RENAME) != 0;

	EnableCtrl(&example_wnd_, rename);
	EnableCtrl(GetDlgItem(IDC_PATTERN_LABEL), rename);
	EnableCtrl(&edit_pattern_, rename);
	EnableCtrl(&symbols_wnd_, rename);
	EnableCtrl(GetDlgItem(IDC_EXAMPLE_LABEL), rename);
}


void CTransferDlg::OnCopy()
{
	OperationChanged();
}

void CTransferDlg::OnMove()
{
	OperationChanged();
}

void CTransferDlg::OnBatch()
{
	OperationChanged();
}


void CTransferDlg::OperationChanged()
{
	if (btn_to_.m_hWnd == 0 || edit_to_.m_hWnd == 0 || label_to_wnd_.m_hWnd == 0)
		return;

	bool batch= IsDlgButtonChecked(IDC_BATCH) != 0;

	edit_to_.EnableWindow(!batch);
	btn_to_.EnableWindow(!batch);
	label_to_wnd_.EnableWindow(!batch);
}
