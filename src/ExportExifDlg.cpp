/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExportExifDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ExportExifDlg.h"
#include "RString.h"
#include "Path.h"
#include "BalloonMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR* REGISTRY_ENTRY_EXPORT= _T("ExportDlg");
static const TCHAR* REG_FILENAME=		   _T("Filename");
static const TCHAR* REG_SEPARATOR=		   _T("Separator");
static const TCHAR* REG_EXP_TAGS=		   _T("ExportTags");

/////////////////////////////////////////////////////////////////////////////
// CExportExifDlg dialog


CExportExifDlg::CExportExifDlg(bool all, CWnd* parent /*=NULL*/)
	: DialogChild(CExportExifDlg::IDD, parent), all_(all)
{
	//{{AFX_DATA_INIT(CExportExifDlg)
	separator_ = _T("");
	out_file_ = _T("");
	expl_ = _T("");
	//}}AFX_DATA_INIT
	tags_ = false;
}


void CExportExifDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CExportExifDlg)
	DDX_Control(DX, IDC_SEPARATOR, separator_wnd_);
	DDX_Control(DX, IDC_OUT_FILE, out_file_wnd_);
	DDX_Text(DX, IDC_SEPARATOR, separator_);
	DDV_MaxChars(DX, separator_, 1);
	DDX_Text(DX, IDC_OUT_FILE, out_file_);
	DDV_MaxChars(DX, out_file_, 260);
	DDX_Text(DX, IDC_EXPL, expl_);
	DDX_Check(DX, IDC_EXP_TAGS, tags_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportExifDlg, DialogChild)
	//{{AFX_MSG_MAP(CExportExifDlg)
	ON_BN_CLICKED(IDC_SELECT_FILE, OnSelectFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportExifDlg message handlers

BOOL CExportExifDlg::OnInitDialog()
{
	separator_ = _T("\t");

	if (all_)
		expl_ = _T("EXIF data from all loaded photographs will be exported to the text file.");
	else
		expl_ = _T("EXIF data from selected photographs will be exported to the text file.");

	out_file_ = AfxGetApp()->GetProfileString(REGISTRY_ENTRY_EXPORT, REG_FILENAME, _T("c:\\ExifFields.txt"));
	separator_ = AfxGetApp()->GetProfileString(REGISTRY_ENTRY_EXPORT, REG_SEPARATOR, _T(";"));
	tags_ = !!AfxGetApp()->GetProfileInt(REGISTRY_ENTRY_EXPORT, REG_EXP_TAGS, 0);

	DialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolExport.htm"));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportExifDlg::OnSelectFile()
{
	CFileDialog dlgFile(false, _T(".txt"), 0,
		OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		RString(IDS_EXPORT_FILTER), 0);

	CString file;
	out_file_wnd_.GetWindowText(file);
	CString dir= Path(file).GetDir().c_str();
	dlgFile.m_ofn.lpstrInitialDir = dir;

	// get output file name
	if (dlgFile.DoModal() != IDOK)
		return;

	out_file_wnd_.SetWindowText(dlgFile.GetPathName());
}


void CExportExifDlg::OnOK()
{
	if (!UpdateData())
		return;

	if (out_file_.IsEmpty())
	{
		new BalloonMsg(&out_file_wnd_, _T("Missing Filename"),
			_T("Please enter name and path of text output file."), BalloonMsg::IERROR);
		return;
	}

	if (separator_.IsEmpty())
	{
		new BalloonMsg(&separator_wnd_, _T("Missing Separator"),
			_T("Please enter fields separator character."), BalloonMsg::IERROR);
		return;
	}

	AfxGetApp()->WriteProfileString(REGISTRY_ENTRY_EXPORT, REG_FILENAME, out_file_);
	AfxGetApp()->WriteProfileString(REGISTRY_ENTRY_EXPORT, REG_SEPARATOR, separator_);
	AfxGetApp()->WriteProfileInt(REGISTRY_ENTRY_EXPORT, REG_EXP_TAGS, tags_);

	EndDialog(IDOK);
}
