/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ErrorDlg.h"
#include "FileSend.h"
#include "CatchAll.h"
#include "StringConversions.h"


// ErrorDlg dialog


ErrorDlg::ErrorDlg(CWnd* parent, bool report_btn) : CDialog(ErrorDlg::IDD, parent)
{
	caption_ = 0;
	title_ = 0;
	message_ = 0;
	callstack_ = 0;
	width_ = collapsed_height_ = expanded_height_ = 0;
	is_small_ = true;
	report_btn_ = report_btn;
}

ErrorDlg::~ErrorDlg()
{}

void ErrorDlg::DoDataExchange(CDataExchange* dx)
{
	DDX_Control(dx, IDC_DETAILS, details_);
	DDX_Control(dx, IDC_MESSAGE, msg_);
	CDialog::DoDataExchange(dx);
}


BEGIN_MESSAGE_MAP(ErrorDlg, CDialog)
	ON_BN_CLICKED(IDC_CALL_STACK, OnDetails)
	ON_BN_CLICKED(IDC_REPORT, OnReport)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


void DisplayErrorDialog(const wchar_t* caption, CWnd* parent, const wchar_t* title, const wchar_t* message, const std::string& callstack, bool report_btn)
{
	ErrorDlg dlg(parent, report_btn);
	dlg.caption_ = caption;
	dlg.title_ = title;
	dlg.message_ = message;
	if (!callstack.empty())
		dlg.callstack_ = &callstack;
	dlg.DoModal();
}

void DisplayErrorDialog(CWnd* parent, const wchar_t* title, const wchar_t* message, const std::string& callstack)
{
	DisplayErrorDialog(0, parent, title, message, callstack, true);
}

void DisplayErrorDialog(CWnd* parent, const wchar_t* title, const wchar_t* message)
{
	DisplayErrorDialog(parent, title, message, std::string());
}

void DisplayErrorDialog(const wchar_t* caption, CWnd* parent, const wchar_t* title, const wchar_t* message, bool report_btn)
{
	DisplayErrorDialog(caption, parent, title, message, std::string(), report_btn);
}


BOOL ErrorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_TITLE, title_);

	if (message_)
		msg_.SetWindowText(message_);
	else
		msg_.ShowWindow(SW_HIDE);

	if (caption_)
		SetWindowText(caption_);

	if (callstack_ == 0 || callstack_->empty())
	{
		if (CWnd* btn= GetDlgItem(IDC_CALL_STACK))
		{
			btn->ShowWindow(SW_HIDE);
			btn->EnableWindow(false);
		}
		if (CWnd* btn= GetDlgItem(IDC_REPORT))
		{
			btn->ShowWindow(SW_HIDE);
			btn->EnableWindow(false);
		}
	}

	if (!report_btn_)
	{
		if (CWnd* btn= GetDlgItem(IDC_REPORT))
		{
			btn->ShowWindow(SW_HIDE);
			btn->EnableWindow(false);
		}
	}

	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
	{
		expanded_height_ = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		width_ = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	}
	else
	{ ASSERT(false); }

	CRect rect(0,0,0,0);
	GetClientRect(rect);
	WINDOWPLACEMENT wp2;
	if (details_.GetWindowPlacement(&wp2))
		collapsed_height_ = expanded_height_ - (rect.Height() - wp2.rcNormalPosition.top);

	SetWindowPos(nullptr, 0, 0, width_, collapsed_height_, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	return true;
}


void ErrorDlg::OnDetails()
{
	int height= is_small_ ? expanded_height_ : collapsed_height_;
	if (is_small_)
	{
		CString str(callstack_->c_str());
		str.Replace(L"\n", L"\r\n");
		SetDlgItemText(IDC_DETAILS, str);
	}
	SetWindowPos(nullptr, 0, 0, width_, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	is_small_ = !is_small_;
	SetDlgItemText(IDC_CALL_STACK, is_small_ ? L"&Details >>" : L"&Details <<");
}


HBRUSH ErrorDlg::OnCtlColor(CDC* dc, CWnd* ctrl, UINT code)
{
	HBRUSH hbr= CDialog::OnCtlColor(dc, ctrl, code);

	if (ctrl != 0 && ctrl->GetDlgCtrlID() == IDC_TITLE)
	{
		if (bold_fnt_.m_hObject == 0)
		{
			LOGFONT lf;
			if (CFont* font= GetFont())
				font->GetLogFont(&lf);
			else
			{
				HFONT font_handle= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
				::GetObject(font_handle, sizeof(lf), &lf);
			}
			lf.lfWeight =  FW_BOLD;
			//lf.lfQuality = ANTIALIASED_QUALITY;
			lf.lfHeight += 1;
			_tcscpy(lf.lfFaceName, _T("Tahoma"));
			bold_fnt_.CreateFontIndirect(&lf);
		}

		dc->SelectObject(&bold_fnt_);
	}
	else if (ctrl != 0 && ctrl->GetDlgCtrlID() == IDC_DETAILS)
	{
		if (mono_fnt_.m_hObject == 0)
		{
			LOGFONT lf;
			if (CFont* font= GetFont())
				font->GetLogFont(&lf);
			else
			{
				HFONT font_handle= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
				::GetObject(font_handle, sizeof(lf), &lf);
			}
			lf.lfPitchAndFamily = FIXED_PITCH;
			wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Lucida Console");
			mono_fnt_.CreateFontIndirect(&lf);

			ctrl->SetFont(&mono_fnt_);
		}

		dc->SelectObject(&mono_fnt_);

		COLORREF color= ::GetSysColor(COLOR_WINDOW);
		if (window_brush_.m_hObject == 0)
			window_brush_.CreateSolidBrush(color);

		hbr = window_brush_;
		dc->SetBkColor(color);
	}

	return hbr;
}

//extern std::string DumpMemoryStatus();
extern void ReportCrashInfo(std::ofstream& fileName, EXCEPTION_POINTERS* exp);

void ErrorDlg::OnReport()
{
	// send error and callstack by e-mail

	char path[MAX_PATH];
	VERIFY(GetTempPathA(MAX_PATH, path) != 0);
	char fname[MAX_PATH];
	VERIFY(GetTempFileNameA(path, "rep", 0, fname) != 0);

	try
	{
		FileSend mail;

		// save report into disk

		std::ofstream report(fname);
		report << "ExifPro error report.\n\n";

		report << "Error: " << TStr2AStr(title_) << '\n';

		ReportCrashInfo(report, 0);

		if (callstack_)
			report << *callstack_;

		report.close();

		std::vector<Path> files(1, Path(CString(fname)));
		std::vector<String> names(1, L"report.txt");

		String msg= L"ExifPro error report. Please describe what you have been doing in ExifPro when this error occurred:\n\n\n";

		mail.SendFiles(files, names, &msg, "Error Report", "errors@exifpro.com");
	}
	CATCH_ALL_W(this)

	::DeleteFileA(fname);
}
