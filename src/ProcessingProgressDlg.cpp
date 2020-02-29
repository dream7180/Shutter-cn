/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ProcessingProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ProcessingProgressDlg.h"
#include "RString.h"
#include "SlideShowGenerator.h"
#include "CatchAll.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ProcessingProgressDlg dialog


ProcessingProgressDlg::ProcessingProgressDlg(CWnd* parent, ImgProcessingPool& worker_threads,
	const TCHAR* dialog_title, const TCHAR* operation, UINT flags)
 : RDialog(ProcessingProgressDlg::IDD, parent), slide_show_(0), pool_(worker_threads)
{
	Init(dialog_title, operation, flags);

	file_count_ = worker_threads.GetFileCount();
	worker_threads.SetStatusWnd(this);
	threads_done_ = static_cast<int>(worker_threads.ThreadCount());
}


void ProcessingProgressDlg::Init(const TCHAR* dialog_title, const TCHAR* operation, UINT flags)
{
	current_photo_ = -1;
	threads_done_ = 0;
	auto_close_ = !!(flags & AUTO_CLOSE);
	title_ = dialog_title;
	if (operation)
		operation_ = operation;
	output_only_ = !!(flags & OUTPUT_ONLY);
	last_update_ = 0;
}


void ProcessingProgressDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(ProcessingProgressDlg)
	DDX_Control(DX, IDC_ENVELOPE, envelope_wnd_);
	DDX_Control(DX, IDC_RUN_SLIDESHOW, run_slide_show_wnd_);
	DDX_Control(DX, IDC_SEND, btn_send_);
	DDX_Control(DX, IDC_PHOTO, photo_wnd_);
	DDX_Control(DX, IDC_TIME, time_left_);
	DDX_Control(DX, IDC_RUN, btn_run_);
	DDX_Control(DX, IDC_PROGRESS, progress_wnd_);
	DDX_Control(DX, IDCANCEL, exit_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ProcessingProgressDlg, CDialog)
	//{{AFX_MSG_MAP(ProcessingProgressDlg)
	ON_BN_CLICKED(IDC_RUN, OnRun)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	//}}AFX_MSG_MAP
	ON_MESSAGE(IPM_NEXT_OPERATION, OnNextOperation)
	ON_MESSAGE(IPM_NEXT_PHOTO, OnNextPhoto)
	ON_MESSAGE(IPM_PROGRESS, OnProgress)
	ON_MESSAGE(IPM_EXCEPTION, OnException)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ProcessingProgressDlg message handlers

BOOL ProcessingProgressDlg::OnInitDialog()
{
	try
	{
		RDialog::OnInitDialog();

		if (output_only_)
		{
			// input photo is the output; hide 'output' fields
			//img_output_wnd_.ShowWindow(SW_HIDE);
			//img_output_wnd_.ShowWindow(SW_HIDE);
			//output_wnd_.ShowWindow(SW_HIDE);
			//if (CWnd* wnd= GetDlgItem(IDC_LABEL_2))
			//	wnd->ShowWindow(SW_HIDE);
		}

//	if (slide_show_)
//		SetWindowText(_T("Slide Show Generation Progress"));

		SetWindowText(title_);
		if (!operation_.IsEmpty())
			SetDlgItemText(IDC_LABEL_1, operation_);

		progress_wnd_.SetRange32(0, static_cast<int>(file_count_ * 100));

		est_time_left_.Start();
		last_update_ = ::GetTickCount();
		time_left_.SetWindowText(L"计算中...");
		SetProgressBar(0, 0);

		photo_wnd_.SetWindowText(L"-");

		pool_.Start();

		return true;
	}
	CATCH_ALL

	// exception captured; close progress dlg
	EndDialog(IDCANCEL);

	return TRUE;
}


void ProcessingProgressDlg::NextPhoto(int photo, bool after)
{
	if (photo >= 0)
	{
		if (after)
			++current_photo_;
	}
	else if (photo < 0)		// thread finished or interrupted?
	{
		threads_done_--;

		if (threads_done_ <= 0)	// all done?
		{
			progress_wnd_.ShowWindow(SW_HIDE);
			time_left_.SetWindowText(L"--:--:--");
			photo_wnd_.SetWindowText(RString(IDS_RESIZE_ALL_PROCESSED));
			//SetDlgItemText(IDC_OPERATION, RString(IDS_RESIZE_COMPLETED));
			GetDlgItem(IDCANCEL)->SetDlgCtrlID(IDOK);
			GetDlgItem(IDC_LABEL_1)->ShowWindow(SW_HIDE);

			if (slide_show_)
				EnableSlideShowBtn();
			else if (auto_close_)
				PostMessage(WM_COMMAND, IDOK);
			else
				SetDlgItemText(IDOK, L"OK");

			return;
		}
	}

	if (current_photo_ >= 0 && current_photo_ < file_count_)
	{
		SetProgressBar(current_photo_, 0);

		CString photo;
		photo.Format(_T("%d of %d"), static_cast<int>(current_photo_ + 1), static_cast<int>(file_count_));
		photo_wnd_.SetWindowText(photo);

		if (current_photo_ >= pool_.ThreadCount())
		{
			DWORD tick= ::GetTickCount();
			if (tick - last_update_ >= 500)	// refresh every 0.5 s at most
			{
				CString time= est_time_left_.TimeLeft(current_photo_, file_count_);
				time_left_.SetWindowText(time);
				last_update_ = tick;
			}
		}
	}
}


void ProcessingProgressDlg::Progress(int progress_percent)
{}


void ProcessingProgressDlg::EnableSlideShowBtn()
{
	if (!slide_show_)
		return;

//	exit_.EnableWindow(false);
//	exit_.UpdateWindow();

	if (!slide_show_->Finish())	// generate slide show app
	{
		photo_wnd_.SetWindowText(_T("写入幻灯片失败."));
	}
	else
	{
		SetDlgItemText(IDC_LABEL_3, L"幻灯片:");
		CString written;
		written.Format(_T("%d KB"),
			/*static_cast<const TCHAR*>(slide_show_->GetSlideShowApp()),*/ static_cast<int>(slide_show_->GetSlideShowSize() / 1024));
		photo_wnd_.SetWindowText(written);
		btn_run_.EnableWindow();
		btn_run_.ShowWindow(SW_SHOWNA);
		btn_send_.EnableWindow(file_send_.IsSendCmdAvailable());
		btn_send_.ShowWindow(SW_SHOWNA);
		envelope_wnd_.ShowWindow(SW_SHOWNA);
		run_slide_show_wnd_.ShowWindow(SW_SHOWNA);
	}

//	exit_.EnableWindow();
	SetDlgItemText(IDOK, L"OK");
}


LRESULT ProcessingProgressDlg::OnNextPhoto(WPARAM photo_index, LPARAM after)
{
	int photo= static_cast<int>(photo_index);
	NextPhoto(photo, !!after);
	return 0;

//	current_photo_ = static_cast<int>(photo_index);
/*
	if (current_photo_ == -1)	// process finished?
	{
		SetDlgItemText(IDC_PHOTO, RString(IDS_RESIZE_ALL_PROCESSED));
		CString written;
		if (slide_show_)
		{
			if (!slide_show_->Finish())	// generate slide show app
				EndDialog(IDCANCEL);

			written.Format(_T("%s (%d KB)"),
				static_cast<const TCHAR*>(slide_show_->GetSlideShowApp()), static_cast<int>(slide_show_->GetSlideShowSize() / 1024));
			btn_run_.EnableWindow();
			btn_run_.ShowWindow(SW_SHOWNA);
			btn_send_.EnableWindow(file_send_.IsSendCmdAvailable());
			btn_send_.ShowWindow(SW_SHOWNA);
			envelope_wnd_.ShowWindow(SW_SHOWNA);
			run_slide_show_wnd_.ShowWindow(SW_SHOWNA);
		}
		else
			written.Format(IDS_RESIZE_FILES_WRITTEN, static_cast<int>(file_count_));
		//SetDlgItemText(IDC_OUTPUT, written);
		SetDlgItemText(IDC_OPERATION, RString(IDS_RESIZE_COMPLETED));
		progress_wnd_.ShowWindow(SW_HIDE);
		SetDlgItemText(IDCANCEL, RString(IDS_RESIZE_DONE));
		GetDlgItem(IDCANCEL)->SetDlgCtrlID(IDOK);
		GetDlgItem(IDC_LABEL_1)->ShowWindow(SW_HIDE);

		if (auto_close_)
			PostMessage(WM_COMMAND, IDOK);

		return 0;
	}
	else if (current_photo_ == -2)	// process interrupted?
	{
		SetDlgItemText(IDC_PHOTO, RString(IDS_RESIZE_NOT_PROCESSED));
		ostringstream ost;
		//SetDlgItemText(IDC_OUTPUT, _T(""));
		SetDlgItemText(IDC_OPERATION, RString(IDS_RESIZE_NOT_COMPLETED));
		progress_wnd_.ShowWindow(SW_HIDE);
		SetDlgItemText(IDCANCEL, RString(IDS_RESIZE_CANCEL));
		GetDlgItem(IDC_LABEL_1)->ShowWindow(SW_HIDE);
		if (slide_show_)
			slide_show_->Delete();
		return 0;
	}

	if (current_photo_ >= 0 && current_photo_ < file_count_ && worker_thread_.get() != 0)
	{
		SetDlgItemText(IDC_PHOTO, worker_thread_->GetSourceFileName(current_photo_).c_str());
		if (slide_show_)
		{
			CString output;
			output.Format(_T("%d/%d"), static_cast<int>(current_photo_ + 1), static_cast<int>(file_count_));
			//SetDlgItemText(IDC_OUTPUT, output);
		}
		else
			//SetDlgItemText(IDC_OUTPUT, worker_thread_->GetDestFileName(current_photo_).c_str());
		SetProgressBar(current_photo_, 0);
	}
	else
	{
		ASSERT(false);
	} */
	return 0;
}


LRESULT ProcessingProgressDlg::OnNextOperation(WPARAM operation, LPARAM)
{
	//RString oper(static_cast<int>(operation));
	//SetDlgItemText(IDC_OPERATION, oper);
	return 0;
}


LRESULT ProcessingProgressDlg::OnProgress(WPARAM progressPercent, LPARAM operation)
{
	Progress(static_cast<int>(progressPercent));
	return 0;

//	SetProgressBar(current_photo_, static_cast<int>(progressPercent));

}


void ProcessingProgressDlg::SetProgressBar(int current_photo, int progress)
{
	ASSERT(current_photo >= 0);
	progress_wnd_.SetPos(current_photo * 100 + progress);
}


void ProcessingProgressDlg::OnCancel()
{
	pool_.Quit();
	EndDialog(IDCANCEL);
}


void ProcessingProgressDlg::OnRun()
{
	if (slide_show_)
	{
		EndDialog(IDCANCEL);
		slide_show_->LaunchSlideShow();
	}
}


void ProcessingProgressDlg::OnSend()
{
	if (slide_show_)
	{
		EndDialog(IDCANCEL);
		CString path= slide_show_->GetSlideShowApp();
		if (!path.IsEmpty())
			file_send_.SendFile(path);
	}
}


LRESULT ProcessingProgressDlg::OnException(WPARAM index_file, LPARAM message)
{
	if (message)
	{
		std::auto_ptr<String> s(reinterpret_cast<String*>(message));
		//todo: error dialog, more exception details
//		AfxMessageBox(s->c_str(), MB_OK | MB_ICONERROR);
		::DisplayErrorDialog(this, L"文件处理失败.", s->c_str());
	}

	return 0;
}
