/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"

// ErrorDlg dialog

class ErrorDlg : public CDialog
{
//	DECLARE_DYNAMIC(ErrorDlg)

public:
	ErrorDlg(CWnd* parent, bool report_btn);
	virtual ~ErrorDlg();

	const wchar_t* caption_;
	const wchar_t* title_;
	const wchar_t* message_;
	const std::string* callstack_;
// Dialog Data
	enum { IDD = IDD_ERROR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnDetails();
	void OnReport();
	HBRUSH OnCtlColor(CDC* dc, CWnd* ctrl, UINT code);
	//CFont bold_fnt_;
	CFont mono_fnt_;
	CBrush window_brush_;
	int expanded_height_;
	int collapsed_height_;
	int width_;
	CEdit details_;
	bool is_small_;
	CStatic msg_;
	bool report_btn_;

	DECLARE_MESSAGE_MAP()
};


void DisplayErrorDialog(CWnd* parent, const wchar_t* title, const wchar_t* message, const std::string& callstack);
void DisplayErrorDialog(CWnd* parent, const wchar_t* title, const wchar_t* message);
void DisplayErrorDialog(const wchar_t* caption, CWnd* parent, const wchar_t* title, const wchar_t* message, bool report_btn);
