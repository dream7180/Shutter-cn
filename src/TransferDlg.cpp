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
#include "TwoFilePanesWnd.h"
#include "FilePaneWnd.h"
#include "CatchAll.h"
#include <boost/bind.hpp>
#include "PNGImage.h"
#include "TransferOptionsDlg.h"
#include "RString.h"
#include "ToolBarWnd.h"
#include "PathEdit.h"
#include <ShLwApi.h>
#include "NewFolderDlg.h"
#include "BalloonMsg.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void AppendToFileName(String& fname, const TCHAR* name_suffix);


namespace {
	enum { OPERATION_COPY= 0, OPERATION_MOVE= 1, OPERATION_RENAME= 2 };
	const TCHAR FLAG_CHR= _T('%');
	const TCHAR SEP_CHR= _T(':');
	const TCHAR BEGIN_CHR= _T('{');
	const TCHAR END_CHR= _T('}');

	const TCHAR* REGISTRY_ENTRY_TRANSFER = _T("TransferDlg");

	const TCHAR* REG_OPERATION	= _T("Operation");
	const TCHAR* REG_PATTERN	= _T("RenamePat");
	const TCHAR* REG_FROM		= _T("FromDir");
	const TCHAR* REG_TO			= _T("ToDir");
	const TCHAR* REG_RENAME		= _T("Rename");
	const TCHAR* REG_READ_ONLY	= _T("ReadOnly");
	const TCHAR* REG_CLEAR_ARCHIVE= _T("ClearArchiveAttrib");
	const TCHAR* REG_FILTER_SRC	= _T("SrcFilter");
	const TCHAR* REG_FILTER_DEST= _T("DestFilter");
	const TCHAR* REG_PANE_SIZE	= _T("PaneSize");
}


struct FilterTypes
{
	FilterTypes()
	{
		showNewOnly_ = false;
		jpeg_ = raw_ = video_ = rest_ = true;
	}
	String inclFileSpec_;
	String exclFileSpec_;
	bool showNewOnly_;
	// visible files (types)
	bool jpeg_;
	bool raw_;
	bool video_;
	bool rest_;
};


void StoreFilter(const FilterTypes& f, CWinApp* app, const TCHAR* key, const TCHAR* base_name)
{
	CString name= base_name;
	app->WriteProfileString(key, name + _T("_I"), f.inclFileSpec_.c_str());
	app->WriteProfileString(key, name + _T("_E"), f.exclFileSpec_.c_str());

	TCHAR filter[]=
	{
		f.showNewOnly_ ? '1' : '0',
		f.jpeg_ ? '1' : '0',
		f.raw_ ? '1' : '0',
		f.video_ ? '1' : '0',
		f.rest_ ? '1' : '0',
		0
	};
	app->WriteProfileString(key, name + _T("_F"), filter);
}


void RestoreFilter(FilterTypes& f, CWinApp* app, const TCHAR* key, const TCHAR* base_name)
{
	CString name= base_name;
	f.inclFileSpec_ = app->GetProfileString(key, name + _T("_I"), f.inclFileSpec_.c_str());
	f.exclFileSpec_ = app->GetProfileString(key, name + _T("_E"), f.exclFileSpec_.c_str());

	CString filter= app->GetProfileString(key, name + _T("_F"), _T(""));
	if (filter.GetLength() == 5)
	{
		f.showNewOnly_ = filter[0] != '0';
		f.jpeg_  = filter[1] != '0';
		f.raw_   = filter[2] != '0';
		f.video_ = filter[3] != '0';
		f.rest_  = filter[4] != '0';
	}
}


struct CTransferDlg::Impl
{
	CWnd* parent_;
	TwoFilePanesWnd panes_dlg_;
	FilterTypes srcFilter_;		// left pane
	FilterTypes destFilter_;	// right pane
	CTransferOptionsDlg options_;
	CButton btn_ok_;
	CBitmap oper_bmp_[3];
	CStatic placeholders_[3];
	CButton btn_oper_[3];
	CFont bold_font_;
	CFont normal_font_;
	ToolBarWnd symbols_tb_;
	CString pattern_;
	BOOL rename_files_;
	CPathEdit pattern_edit_;
	CStatic	example_wnd_;
	CStatic rename_label_;
	CButton rename_checkbox_;
	int operation_btn_;
	bool rename_old_state;
	String use_src_path_;
	String used_dest_path_;

	Impl()
	{
		parent_ = 0;
		operation_btn_ = OPERATION_COPY;
		rename_old_state = false;
		rename_files_ = false;
	}
	~Impl()
	{}

	void ResetFileSpecs(FilterTypes& filter);
	bool FilterFiles(const TCHAR* path, FilterTypes* filter);
	bool FileTypePopup(CPoint pos, FilePaneWnd* pane, FilterTypes* filter);
	void SymbolsPopupMenu();
	void UpdatePatternCtrl();
	void SetFilter(FilePaneWnd& pane, FilterTypes& filter);
	void PreparePaths(const std::vector<FileInfo>& src, std::vector<String>& dest, std::vector<String>& source,
		const TCHAR* destFolder, const TCHAR* rename_pattern);
	void CreateNewFolder(FilePaneWnd* pane);
	void Refresh();
};


/////////////////////////////////////////////////////////////////////////////
// CTransferDlg dialog


CTransferDlg::CTransferDlg(CWnd* parent, const TCHAR* src_path)
	: DialogChild(CTransferDlg::IDD, parent), pImpl_(new Impl)
{
	pImpl_->parent_ = this;
	if (src_path && *src_path)
	{
		TCHAR buf[MAX_PATH];
		if (::PathCanonicalize(buf, src_path))
			pImpl_->use_src_path_ = buf;
	}
}


CTransferDlg::~CTransferDlg()
{}


void CTransferDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//DDX_Control(DX, IDC_TOOLBAR, pImpl_->toolbar_);
	DDX_Control(DX, IDOK, pImpl_->btn_ok_);
	DDX_Control(DX, IDC_COPY, pImpl_->btn_oper_[0]);
	DDX_Control(DX, IDC_MOVE, pImpl_->btn_oper_[1]);
	DDX_Control(DX, IDC_BATCH, pImpl_->btn_oper_[2]);
	DDX_Control(DX, IDC_OPER_1, pImpl_->placeholders_[0]);
	DDX_Control(DX, IDC_OPER_2, pImpl_->placeholders_[1]);
	DDX_Control(DX, IDC_OPER_3, pImpl_->placeholders_[2]);
	DDX_Radio(DX, IDC_COPY, pImpl_->operation_btn_);

	DDX_Control(DX, IDC_PATTERN, pImpl_->pattern_edit_);
	DDX_Control(DX, IDC_EXAMPLE, pImpl_->example_wnd_);
	DDX_Text(DX, IDC_PATTERN, pImpl_->pattern_);
	DDX_Check(DX, IDC_RENAME, pImpl_->rename_files_);

	DDX_Control(DX, IDC_LABEL3, pImpl_->rename_label_);
	DDX_Control(DX, IDC_RENAME, pImpl_->rename_checkbox_);
}


BEGIN_MESSAGE_MAP(CTransferDlg, DialogChild)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_MOVE, OnMove)
	ON_BN_CLICKED(IDC_BATCH, OnBatch)
	ON_STN_CLICKED(IDC_OPER_1, OnCopy)
	ON_STN_CLICKED(IDC_OPER_2, OnMove)
	ON_STN_CLICKED(IDC_OPER_3, OnBatch)
//	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	ON_EN_CHANGE(IDC_PATTERN, OnChangePattern)
	ON_NOTIFY(TBN_DROPDOWN, IDC_SYMBOLS, OnTbDropDown)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransferDlg message handlers

BOOL CTransferDlg::OnInitDialog()
{
	try
	{
		return InitDlg();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


static Path FindValidPath(Path path)
{
	// make sure we have a valid folder to start

	while (!path.empty() && !path.IsFolder())	// folder does not exist?
		path = path.GetDir();

	if (path.empty())
		path = _T("C:\\");

	return path;
}


BOOL CTransferDlg::InitDlg()
{
	CWinApp* app= AfxGetApp();

	pImpl_->operation_btn_ = app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_OPERATION, OPERATION_COPY);
	pImpl_->pattern_ = app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_PATTERN, _T("%Y-%m-%d_%H-%M-%S"));
	Path from_path= pImpl_->use_src_path_.empty() ?
		app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_FROM, _T("c:\\")) : pImpl_->use_src_path_.c_str();
	Path to_path= app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_TO, _T(""));
	pImpl_->rename_files_ = app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_RENAME, pImpl_->rename_files_);
	pImpl_->rename_old_state = !!pImpl_->rename_files_;
	pImpl_->options_.makeReadOnly_ = app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_READ_ONLY, false);
	pImpl_->options_.clearArchiveAttr_ = app->GetProfileInt(REGISTRY_ENTRY_TRANSFER, REG_CLEAR_ARCHIVE, true);
	pImpl_->srcFilter_.showNewOnly_ = true;
	RestoreFilter(pImpl_->srcFilter_, app, REGISTRY_ENTRY_TRANSFER, REG_FILTER_SRC);
	RestoreFilter(pImpl_->destFilter_, app, REGISTRY_ENTRY_TRANSFER, REG_FILTER_DEST);

	if (to_path.empty())
	{
		ITEMIDLIST* pidl= 0;
		if (::SHGetSpecialFolderLocation(m_hWnd, CSIDL_MYPICTURES, &pidl) == NOERROR && pidl != 0)
		{
			ItemIdList folder(pidl, false);
			to_path.assign(folder.GetPath());
		}

//		TCHAR buf[MAX_PATH];
//		if (SUCCEEDED(::SHGetFolderPath(*this, CSIDL_MYPICTURES, 0, SHGFP_TYPE_CURRENT, buf)))
//			to_ = buf;
	}
	else
		to_path = FindValidPath(to_path);

	from_path = FindValidPath(from_path);


	DialogChild::OnInitDialog();

	if (!pImpl_->options_.Create(GetParent()))
	{
		EndDialog(IDCANCEL);
		return true;
	}

	//pImpl_->toolbar_.Create("RRR|X", cmds, IDB_TRANSFER_TB, IDS_TRANSFER_TB);

	if (pImpl_->bold_font_.m_hObject == 0)
	{
		LOGFONT lf;
		if (CFont* font= /*GetParent()->*/GetFont())
			font->GetLogFont(&lf);
		else
		{
			HFONT hfont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
			::GetObject(hfont, sizeof(lf), &lf);
			//lf.lfQuality = ANTIALIASED_QUALITY;
			lf.lfHeight += 1;
			_tcscpy(lf.lfFaceName, _T("Tahoma"));
		}
		pImpl_->normal_font_.CreateFontIndirect(&lf);
		lf.lfWeight = FW_BOLD;
		pImpl_->bold_font_.CreateFontIndirect(&lf);
	}

	SetFooterDlg(&pImpl_->options_);

	//VERIFY(LoadImageList(pImpl_->icons_, IDR_TRANSFER_ICONS, 40, ::GetSysColor(COLOR_3DFACE)));
	PNGImage img;
	img.AlphaToColor(::GetSysColor(COLOR_3DFACE));
	static const int bmps[]= { IDR_TRANSFER_OPER_1, IDR_TRANSFER_OPER_2, IDR_TRANSFER_OPER_3 };
	for (int i= 0; i < array_count(bmps); ++i)
	{
		img.Load(bmps[i], pImpl_->oper_bmp_[i]);
		pImpl_->placeholders_[i].SetBitmap(pImpl_->oper_bmp_[i]);
	}

	SubclassHelpBtn(_T("ToolTransfer.htm"));

	pImpl_->symbols_tb_.SubclassDlgItem(IDC_SYMBOLS, this);
	int cmd_id[]= { IDC_SYMBOLS };
	pImpl_->symbols_tb_.SetPadding(2, 9);
	pImpl_->symbols_tb_.AddButtons("v", cmd_id, 0);

	pImpl_->example_wnd_.ModifyStyle(0, SS_ENDELLIPSIS);

	// disable all illegal file name chars but ':' needed for pattern
	pImpl_->pattern_edit_.SetIllegalChars(_T("/\\*?|><\""));

	TCHAR* end= 0;
	double paneSplit= _tcstod(app->GetProfileString(REGISTRY_ENTRY_TRANSFER, REG_PANE_SIZE, _T("0.5")), &end);

	if (!pImpl_->panes_dlg_.Create(this, paneSplit))
		throw _T("Creation of file pane dialog failed.");

	{
		CWnd* frame= GetDlgItem(IDC_PANES);
		WINDOWPLACEMENT wp;
		frame->GetWindowPlacement(&wp);
		frame->DestroyWindow();
		pImpl_->panes_dlg_.MoveWindow(&wp.rcNormalPosition);
		pImpl_->panes_dlg_.SetDlgCtrlID(IDC_PANES);
	}

	FilePaneWnd& left= pImpl_->panes_dlg_.GetLeftPane();
	FilePaneWnd& right= pImpl_->panes_dlg_.GetRightPane();

	left.SetPathsRegistrySection(_T("TransferDlgSrcPaths"));
	// use the same recent paths as copy/move dlg uses
	const TCHAR* RECENT_PATHS_SECTION= _T("CopyMoveDlg");
	right.SetPathsRegistrySection(RECENT_PATHS_SECTION);

	left.SetFileTypeSelect(
		boost::bind(&CTransferDlg::Impl::FileTypePopup, pImpl_.get(), _1, &left, &pImpl_->srcFilter_));

	right.SetFileTypeSelect(
		boost::bind(&CTransferDlg::Impl::FileTypePopup, pImpl_.get(), _1, &right, &pImpl_->destFilter_));

	pImpl_->SetFilter(left, pImpl_->srcFilter_);
	pImpl_->SetFilter(right, pImpl_->destFilter_);

	left.SetPath(from_path.c_str());
	right.SetPath(to_path.c_str());

	right.SetNewFolderCmd(
		boost::bind(&CTransferDlg::Impl::CreateNewFolder, pImpl_.get(), &right));

	OnChangePattern();

	pImpl_->UpdatePatternCtrl();

	OperationChanged();

	BuildResizingMap();

	//SetWndResizing(IDC_COPY, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	//SetWndResizing(IDC_MOVE, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	//SetWndResizing(IDC_BATCH, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);

	//SetWndResizing(IDC_LABEL, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	//SetWndResizing(IDC_OPER_1, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	//SetWndResizing(IDC_OPER_2, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	//SetWndResizing(IDC_OPER_3, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);

	SetWndResizing(IDC_PATTERN, DlgAutoResize::RESIZE_H, DlgAutoResize::HALF_RESIZE_H);
	SetWndResizing(IDC_SYMBOLS, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	SetWndResizing(IDC_EXAMPLE_LABEL, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	SetWndResizing(IDC_EXAMPLE, DlgAutoResize::MOVE_H_RESIZE_H, DlgAutoResize::HALF_MOVE_H | DlgAutoResize::HALF_RESIZE_H);

	SetWndResizing(IDC_PANES, DlgAutoResize::RESIZE);
	SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
	SetWndResizing(IDC_OPTIONS, DlgAutoResize::MOVE_V);
	SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	SetWndResizing(IDOK, DlgAutoResize::MOVE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


String Int(int value, int width= 2)
{
	oStringstream ost;
	ost.width(width);
	ost.fill(L'0');
	ost << value;
	return ost.str();
}


extern CString ParsePattern(const TCHAR* file_name, DateTime date, int serial, const TCHAR* pattern)
{
	ASSERT(pattern && file_name);

	Path filename(file_name);

	String out;

	bool flag= false;
	auto ymd= date.date().year_month_day();
	auto time= date.time_of_day();

	for (const TCHAR* chr= pattern; *chr; ++chr)
	{
		TCHAR c= *chr;

		if (c == FLAG_CHR)
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
				switch (c)
				{
				case _T('Y'):
					out += Int(ymd.year, 4);
					break;
				case _T('y'):
					out += Int(ymd.year % 100);
					break;
				case _T('m'):
					out += Int(ymd.month);
					break;
				case _T('d'):
					out += Int(ymd.day);
					break;

				case _T('H'):
					out += Int(time.hours());
					break;
				case _T('M'):
					out += Int(time.minutes());
					break;
				case _T('S'):
					out += Int(time.seconds());
					break;

				case _T('f'):
					{
						String name= filename.GetFileName();

						if (chr[1] == BEGIN_CHR)
						{
							TCHAR* end= 0;
							int from= _tcstol(&chr[2], &end, 10) - 1;
							size_t length= 0;
							if (end > &chr[2])
							{
								if (*end == SEP_CHR)
									length = _tcstol(end + 1, &end, 10);

								if (*end == END_CHR)
									end++;

								chr = end - 1;
							}

							if (length == 0)
								length = name.size() - from;

							if (from >= 0 && from < name.size() && from + length <= name.size())
								name = name.substr(from, length);
						}

						out += name.c_str();
					}
					break;

				case _T('n'):
					{
						int length= 5;
						if (chr[1] == BEGIN_CHR)
						{
							TCHAR* end= 0;
							int len= _tcstol(&chr[2], &end, 10);
							if (len > 0 && len < 10)
								length = len;

							if (*end == SEP_CHR)
								serial += _tcstol(end + 1, &end, 10) - 1;

							if (*end == END_CHR)
								end++;

							chr = end - 1;
						}
						oStringstream ost;
						ost.fill(_T('0'));
						ost.width(length);
						int mask= 10;
						for (int i= 1; i < length; ++i)
							mask *= 10;
						ost << (serial % mask);
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
		switch (c)
		{
		case _T('\\'):
		case _T('/'):
		case _T('?'):
		case _T('*'):
		case _T(':'):
		case _T('\"'):
		case _T('>'):
		case _T('<'):
		case _T('|'):
			c = _T('_');
			break;
		default:
			break;
		}

		out += c;
	}

	CString ext= filename.GetExtension().c_str();
	if (!ext.IsEmpty())
		out += _T('.') + ext;

	return CString(out.c_str());
}

//void CTransferDlg::OnFromBtn()
//{
//	OnBrowse(edit_from_, _T("Select Source Folder"));
//}
//
//void CTransferDlg::OnToBtn()
//{
//	OnBrowse(edit_to_, _T("Select Destination Folder"));
//}


//void CTransferDlg::OnBrowse(CWnd& wnd, const TCHAR* caption)
//{
//	CString path;
//	wnd.GetWindowText(path);
//
//	if (path.IsEmpty())
//		path = _T("c:\\");
//
//	CFolderSelect fs(this);
//	CString str= fs.DoSelectPath(caption, path);
//
//	if (str.IsEmpty())
//		return;
//
//	wnd.SetWindowText(str);
//}


struct cmp_file_name
{
	bool operator () (const FileInfo& f1, const FileInfo& f2) const
	{
		return _tcsicmp(f1.path_.c_str(), f2.path_.c_str()) < 0;
	}
};


void CTransferDlg::OnOK()
{
	try
	{
		if (!UpdateData() || !pImpl_->options_.UpdateData())
			return;

		FilePaneWnd& left= pImpl_->panes_dlg_.GetLeftPane();
		FilePaneWnd& right= pImpl_->panes_dlg_.GetRightPane();

		bool rename= pImpl_->operation_btn_ == OPERATION_RENAME;

		CString from;
		if (!rename)
		{
			from = left.GetPath(true);
			if (from.IsEmpty())
				return;
		}

		CString to= right.GetPath(true);
		if (to.IsEmpty())
			return;

		if (!rename)
		{
			left.AddRecentPath(from);
			left.StoreRecentPaths();
		}

		right.AddRecentPath(to);
		right.StoreRecentPaths();

		CWinApp* app= AfxGetApp();
		app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_OPERATION, pImpl_->operation_btn_);
		app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_PATTERN, pImpl_->pattern_);
		if (!rename)
			app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_FROM, from);
		app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_TO, to);
		app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_RENAME, pImpl_->rename_files_);
		app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_READ_ONLY, pImpl_->options_.makeReadOnly_);
		app->WriteProfileInt(REGISTRY_ENTRY_TRANSFER, REG_CLEAR_ARCHIVE, pImpl_->options_.clearArchiveAttr_);
		StoreFilter(pImpl_->srcFilter_, app, REGISTRY_ENTRY_TRANSFER, REG_FILTER_SRC);
		StoreFilter(pImpl_->destFilter_, app, REGISTRY_ENTRY_TRANSFER, REG_FILTER_DEST);
		CString paneSplit;
		paneSplit.Format(_T("%.18f"), pImpl_->panes_dlg_.GetPaneSplitRatio());
		app->WriteProfileString(REGISTRY_ENTRY_TRANSFER, REG_PANE_SIZE, paneSplit);

		EndDialog(IDOK);

		if (CWnd* wnd= AfxGetMainWnd())
			wnd->UpdateWindow();

		std::vector<FileInfo> srcFiles;
		{
			CWaitCursor wait;
			// for batch rename source and dest are at the same folder
			if (rename)
				right.GetFiles(srcFiles);
			else
				left.GetFiles(srcFiles);
		}

		if (!srcFiles.empty())
		{
			// sort files by name (important for numbering them)
			std::sort(srcFiles.begin(), srcFiles.end(), cmp_file_name());

			std::vector<String> destFiles;
			std::vector<String> sourceFiles;
			pImpl_->PreparePaths(srcFiles, destFiles, sourceFiles, to, pImpl_->rename_files_ ? pImpl_->pattern_ : _T(""));

			bool copyOp= pImpl_->operation_btn_ == OPERATION_COPY;

			CTransferFiles files(!!pImpl_->options_.makeReadOnly_, !!pImpl_->options_.clearArchiveAttr_ && copyOp);

			files.Transfer(copyOp, sourceFiles, destFiles);
		}

		pImpl_->used_dest_path_ = to;
	}
	CATCH_ALL
}


void CTransferDlg::OnCopy()
{
	CheckRadioButton(IDC_COPY, IDC_BATCH, IDC_COPY);
	OperationChanged();
}

void CTransferDlg::OnMove()
{
	CheckRadioButton(IDC_COPY, IDC_BATCH, IDC_MOVE);
	OperationChanged();
}

void CTransferDlg::OnBatch()
{
	CheckRadioButton(IDC_COPY, IDC_BATCH, IDC_BATCH);
	OperationChanged();
}


void CTransferDlg::OperationChanged()
{
	if (pImpl_->btn_ok_.m_hWnd == 0)
		return;

	bool batch= IsDlgButtonChecked(IDC_BATCH) != 0;
	bool copy= IsDlgButtonChecked(IDC_COPY) != 0;
	bool move= !batch && !copy;

	pImpl_->panes_dlg_.ShowLeftPane(!batch);

	CString label;
	label.LoadString(batch ? IDS_RENAME_BTN : IDS_TRANSFER_BTN);
	pImpl_->btn_ok_.SetWindowText(label);
	label.Replace(_T("&"), _T(""));
	SetBigTitle(label);

	CheckDlgButton(IDC_RENAME, batch ? true : pImpl_->rename_old_state);
	EnableCtrl(GetDlgItem(IDC_RENAME), !batch, false);
	pImpl_->UpdatePatternCtrl();

	pImpl_->btn_oper_[0].SetFont(copy ? &pImpl_->bold_font_ : &pImpl_->normal_font_);
	pImpl_->btn_oper_[1].SetFont(move ? &pImpl_->bold_font_ : &pImpl_->normal_font_);
	pImpl_->btn_oper_[2].SetFont(batch ? &pImpl_->bold_font_ : &pImpl_->normal_font_);

	pImpl_->rename_label_.ShowWindow(batch ? SW_SHOWNA : SW_HIDE);
	pImpl_->rename_checkbox_.ShowWindow(!batch ? SW_SHOWNA : SW_HIDE);

	//if (batch
	//edit_to_.EnableWindow(!batch);
	//btn_to_.EnableWindow(!batch);
	//label_to_wnd_.EnableWindow(!batch);
}


//BOOL CTransferDlg::OnEraseBkgnd(CDC* dc)
//{
//	if (dc)
//	{
//		CRect rect(0,0,0,0);
//		GetClientRect(rect);
//
//		COLORREF color= ::GetSysColor(COLOR_3DFACE);
//
//		// exclude file panes
//
//		WINDOWPLACEMENT wp;
//		VERIFY(pImpl_->panes_dlg_.GetWindowPlacement(&wp));
//
//		CRect r= rect;
//		r.bottom = wp.rcNormalPosition.top;
//		dc->FillSolidRect(r, color);
//
//		r = rect;
//		r.top = wp.rcNormalPosition.bottom;
//		dc->FillSolidRect(r, color);
//
//		r = rect;
//		r.top = wp.rcNormalPosition.top;
//		r.bottom = wp.rcNormalPosition.bottom;
//
//		r.right = wp.rcNormalPosition.left;
//		dc->FillSolidRect(r, color);
//
//		r.right = rect.right;
//		r.left = wp.rcNormalPosition.right;
//		dc->FillSolidRect(r, color);
//	}
//
//	return true;
//}


bool CTransferDlg::Impl::FileTypePopup(CPoint pos, FilePaneWnd* pane, FilterTypes* filter)
{
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_FILE_PANE_POPUP));

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		popup->CheckMenuItem(ID_JPEG_FILES, MF_BYCOMMAND | (filter->jpeg_ ? MF_CHECKED : MF_UNCHECKED));
		popup->CheckMenuItem(ID_RAW_FILES, MF_BYCOMMAND | (filter->raw_ ? MF_CHECKED : MF_UNCHECKED));
		popup->CheckMenuItem(ID_VIDEO_FILES, MF_BYCOMMAND | (filter->video_ ? MF_CHECKED : MF_UNCHECKED));
		popup->CheckMenuItem(ID_MISCELLANEOUS_FILES, MF_BYCOMMAND | (filter->rest_ ? MF_CHECKED : MF_UNCHECKED));
		popup->CheckMenuItem(ID_ONLY_NEW_FILES, MF_BYCOMMAND | (filter->showNewOnly_ ? MF_CHECKED : MF_UNCHECKED));

		if (pane == &panes_dlg_.GetRightPane())
		{
			// dest pane has no 'show new only' option
			popup->DeleteMenu(ID_ONLY_NEW_FILES, MF_BYCOMMAND);
			// delete separator
			popup->DeleteMenu(popup->GetMenuItemCount() - 1, MF_BYPOSITION);
		}

		int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pos.x, pos.y, parent_);

		switch (cmd)
		{
		case ID_ALL_FILE_TYPES:
			filter->jpeg_ = filter->raw_ = filter->video_ = filter->rest_ = true;
			break;

		case ID_JPEG_FILES:
			filter->jpeg_ = !filter->jpeg_;
			break;

		case ID_RAW_FILES:
			filter->raw_ = !filter->raw_;
			break;

		case ID_VIDEO_FILES:
			filter->video_ = !filter->video_;
			break;

		case ID_MISCELLANEOUS_FILES:
			filter->rest_ = !filter->rest_;
			break;

		case ID_ONLY_NEW_FILES:
			filter->showNewOnly_ = !filter->showNewOnly_;
			break;

		default:
			return false;
		}

		if (!filter->jpeg_ && !filter->raw_ && !filter->video_ && !filter->rest_)
			filter->rest_ = true;

		SetFilter(*pane, *filter);
	}

	return true;
}


void CTransferDlg::Impl::SetFilter(FilePaneWnd& pane, FilterTypes& filter)
{
	String label;

	if (filter.jpeg_ && filter.raw_ && filter.video_ && filter.rest_)
		label = filter.showNewOnly_ ?_T("所有新文件") : _T("所有文件类型");
	else
	{
		bool comma= false;

		if (filter.showNewOnly_)
			label = _T("New ");

		if (filter.jpeg_)
			label += _T("JPEG"), comma = true;

		if (filter.raw_)
		{
			if (comma) label += _T(", ");
			label += _T("Raw"), comma = true;
		}

		if (filter.video_)
		{
			if (comma) label += _T(", ");
			label += _T("Video"), comma = true;
		}

		if (filter.rest_)
		{
			if (comma) label += _T(", ");
			label += _T("Other"), comma = true;
		}

		label += _T(" Files");
	}

	ResetFileSpecs(filter);

	pane.SetFileMask(boost::bind(&CTransferDlg::Impl::FilterFiles, this, _1, &filter), label);
}


bool CTransferDlg::Impl::FilterFiles(const TCHAR* path, FilterTypes* filter)
{
	ASSERT(filter);

	if (!filter->inclFileSpec_.empty() && !::PathMatchSpec(path, filter->inclFileSpec_.c_str()))
		return false;

	if (!filter->exclFileSpec_.empty() && ::PathMatchSpec(path, filter->exclFileSpec_.c_str()))
		return false;

	if (filter->showNewOnly_)	// this file info check takes lots of time...
	{
		WIN32_FIND_DATA fileInfo;
		HANDLE find= ::FindFirstFile(path, &fileInfo);
		if (find == INVALID_HANDLE_VALUE)
			return false;

		VERIFY(FindClose(find));
		DWORD attrib= fileInfo.dwFileAttributes;

		// new files only? (show only those with ARCHIVE attrib set)
		if (filter->showNewOnly_ && (attrib & FILE_ATTRIBUTE_ARCHIVE) == 0)
			return false;
	}

	return true;
}


void CTransferDlg::Impl::ResetFileSpecs(FilterTypes& filter)
{
	filter.exclFileSpec_ = _T("");

	//TODO: those masks should be configurable
	const TCHAR* jpg= _T("*.jpg;*.jpeg;*.jpe;*.jp2;*.j2k;*.j2000");
	const TCHAR* raw= _T("*.crw;*.cr2;*.nef;*.thm;*.orf;*.tif;*.tiff;*.dng;*.raf;*.mrw;");
	const TCHAR* vid= _T("*.avi;*.mov;");

	if (filter.rest_)
	{
		filter.inclFileSpec_ = _T("*.*");

		if (!filter.jpeg_)
			filter.exclFileSpec_ += jpg;
		if (!filter.raw_)
			filter.exclFileSpec_ += raw;
		if (!filter.video_)
			filter.exclFileSpec_ += vid;
	}
	else
	{
		filter.inclFileSpec_ = _T("");

		if (filter.jpeg_)
			filter.inclFileSpec_ += jpg;
		if (filter.raw_)
			filter.inclFileSpec_ += raw;
		if (filter.video_)
			filter.inclFileSpec_ += vid;
	}
}


void CTransferDlg::OnOptions()
{
	if (pImpl_->options_.m_hWnd == 0)
		return;

	bool show= pImpl_->options_.GetStyle() & WS_VISIBLE ? false : true;

	if (GetGripWnd().m_hWnd)
		GetGripWnd().ShowWindow(show ? SW_HIDE : SW_SHOWNA);

	ShowFooterDlg(show);

	SetDlgItemText(IDC_OPTIONS, RString(show ? IDS_CLOSE_SAVING_OPTIONS : IDS_SAVING_OPTIONS));
}


void CTransferDlg::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case IDC_SYMBOLS:
		pImpl_->SymbolsPopupMenu();
		break;
	}
}


extern void RenameSymbolsPopupMenu(CPoint pos, CWnd* parent, CEdit& pattern_edit_box)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_TRANSFER_SYMBOLS))
		return;

	CMenu* popup= menu.GetSubMenu(0);

	int cmd= popup->TrackPopupMenu(TPM_RIGHTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, parent);

	if (cmd == 0)
		return;

	switch (cmd)
	{
	case ID_SYMBOL_DATE_TIME:
		pattern_edit_box.SetWindowText(_T("%Y-%m-%d_%H-%M-%S"));
		break;

	case ID_SYMBOL_FILE_DATE:
		pattern_edit_box.SetWindowText(_T("%f_%Y-%m-%d"));
		break;

	case ID_SYMBOL_9:
		pattern_edit_box.ReplaceSel(_T("%f{3}"), true);
		break;

	case ID_SYMBOL_10:
		pattern_edit_box.ReplaceSel(_T("%f{3:5}"), true);
		break;

	case ID_SYMBOL_11:
		pattern_edit_box.ReplaceSel(_T("%n"), true);
		break;

	case ID_SYMBOL_12:
		pattern_edit_box.ReplaceSel(_T("%n{3}"), true);
		break;

	case ID_SYMBOL_13:
		pattern_edit_box.ReplaceSel(_T("%n{3:001}"), true);
		break;

	default:
		{
			cmd -= ID_SYMBOL_1;
			static const TCHAR flags[]= _T("YymdHMSf");

			ASSERT(cmd >= 0 && cmd < _tcslen(flags));
			TCHAR insert[]= { FLAG_CHR, flags[cmd], 0 };

			pattern_edit_box.ReplaceSel(insert, true);
		}
		break;
	}
}


void CTransferDlg::Impl::SymbolsPopupMenu()
{
	CRect rect;
	symbols_tb_.GetRect(IDC_SYMBOLS, rect);
	symbols_tb_.ClientToScreen(rect);

	RenameSymbolsPopupMenu(CPoint(rect.right, rect.bottom), parent_, pattern_edit_);
}


void CTransferDlg::OnChangePattern()
{
	if (pImpl_->pattern_edit_.m_hWnd == 0)
		return;

	CString text;
	pImpl_->pattern_edit_.GetWindowText(text);

	text = ParsePattern(_T("DSC01234.JPG"), ToDateTime(2012, 12, 31, 23, 59, 58), 123, text);

	if (!text.IsEmpty())
		pImpl_->example_wnd_.SetWindowText(text);
}


void CTransferDlg::OnRename()
{
	pImpl_->rename_old_state = IsDlgButtonChecked(IDC_RENAME) != 0;
	pImpl_->UpdatePatternCtrl();
}


void CTransferDlg::Impl::UpdatePatternCtrl()
{
	bool rename= parent_->IsDlgButtonChecked(IDC_RENAME) != 0;

//	EnableCtrl(parent_->GetDlgItem(IDC_PATTERN_LABEL), rename, false);
	EnableCtrl(&pattern_edit_, rename, false);
	EnableCtrl(&symbols_tb_, rename);
	EnableCtrl(parent_->GetDlgItem(IDC_EXAMPLE_LABEL), rename);
	EnableCtrl(&example_wnd_, rename);
}


struct CmpFiles
{
	bool operator () (const String* d1, const String* d2)
	{
		return *d1 < *d2;
	}
};


void CTransferDlg::Impl::PreparePaths(const std::vector<FileInfo>& src, std::vector<String>& dest,
									 std::vector<String>& source, const TCHAR* destFolder, const TCHAR* rename_pattern)
{
	const size_t count= src.size();
	dest.resize(count);
	source.resize(count);

	Path destPath= destFolder;
	destPath.AppendDirSeparator();

	std::vector<String*> sorted;
	sorted.resize(count);

	int serial= 0;

	for (size_t i= 0; i < count; ++i)
	{
		const FileInfo& f= src[i];
		Path srcFile(f.path_);

		String destFile= destPath;

		if (rename_pattern != 0 && *rename_pattern != 0)	// rename file?
		{
			FILETIME localTime;
			if (!::FileTimeToLocalFileTime(&f.modified_, &localTime))
			{
				ASSERT(false);
				localTime.dwLowDateTime = 0;
				localTime.dwHighDateTime = 0;
			}
			SYSTEMTIME sysTime;
			if (!::FileTimeToSystemTime(&localTime, &sysTime))
			{
				ASSERT(false);
				memset(&sysTime, 0, sizeof sysTime);
			}

			destFile += ParsePattern(srcFile.GetFileName().c_str(), SytemTimeToDateTime(sysTime), ++serial, rename_pattern);
			String ext= srcFile.GetExtension();
			if (!ext.empty())
			{
				destFile += _T('.');
				destFile += ext;
			}
		}
		else	// same file name
		{
			destFile += srcFile.GetFileNameAndExt();
		}

		dest[i] = destFile;
		source[i] = srcFile;
	}

	for (size_t i= 0; i < count; ++i)
		sorted[i] = &dest[i];

	// sort dest files
	std::sort(sorted.begin(), sorted.end(), CmpFiles());

	// find duplicate destination names and modify them by appending number
	if (count > 0)
		for (int index= 0; index < count - 1; )
		{
			int next= index + 1;

			// case insensitive comparision
			while (_tcsicmp(sorted[index]->c_str(), sorted[next]->c_str()) == 0)
			{
				TCHAR num[16];
				num[0] = _T('_');
				_itot(next - index, num + 1, 10);
				AppendToFileName(*sorted[next], num);
				++next;
				if (next == count)
					break;
			}

			index = next;
		}
}


String CTransferDlg::GetDestPath() const
{
	return pImpl_->used_dest_path_;
}


void CTransferDlg::Impl::CreateNewFolder(FilePaneWnd* pane)
{
	String path= pane->GetPath(true);
	if (path.empty())
		return;

	NewFolderDlg dlg(pane);

	CTime time= CTime::GetCurrentTime();
	dlg.folderName_ = time.Format(_T("%Y-%m-%d"));

	dlg.SetCreateFolderCallback(boost::bind(&CreateFolderHelperFn, _1, _2, &path));

	if (dlg.DoModal() != IDOK)
		return;

	pane->SetPath(path.c_str());
}


// Tap into the modal loop to handle some key down messages
//
BOOL CTransferDlg::ContinueModal()
{
	if (!DialogChild::ContinueModal())
		return false;

	if (MSG *msg = AfxGetCurrentMessage())
	{
		if (msg->message == WM_KEYDOWN)
		{
			try
			{
				switch (msg->wParam)
				{
				case VK_F5:
					pImpl_->Refresh();
					break;
				}
			}
			CATCH_ALL
		}
	}

	return true;
}


void CTransferDlg::Impl::Refresh()
{
	panes_dlg_.GetLeftPane().Refresh();
	panes_dlg_.GetRightPane().Refresh();
}
