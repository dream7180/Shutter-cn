/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// ProcessingProgressDlg.h : header file
//
#include "RDialog.h"
#include "ResizingThread.h"
#include "ImgProcessingThread.h"
#include "ImgProcessingPool.h"
class CSlideShowGenerator;
#include "FileSend.h"
#include "EstimatedTime.h"


/////////////////////////////////////////////////////////////////////////////
// ProcessingProgressDlg dialog

class ProcessingProgressDlg : public RDialog
{
// Construction
public:
	enum Flags { AUTO_CLOSE = 1, INPUT_OUTPUT = 2, OUTPUT_ONLY = 4 };

	ProcessingProgressDlg(CWnd* parent, ImgProcessingPool& worker_threads, const TCHAR* dialog_title, const TCHAR* operation, UINT flags);

// Dialog Data
	//{{AFX_DATA(ProcessingProgressDlg)
	enum { IDD = IDD_RESIZING };
	CStatic	envelope_wnd_;
	CStatic	run_slide_show_wnd_;
	CButton	btn_send_;
	CStatic	photo_wnd_;
	CButton	btn_run_;
	CProgressCtrl	progress_wnd_;
	//}}AFX_DATA
	CStatic	time_left_;

	//todo: remove
	void SetSlideShow(CSlideShowGenerator* slide_show) { slide_show_ = slide_show; }

// Overrides
	//{{AFX_VIRTUAL(ProcessingProgressDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ProcessingProgressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnRun();
	afx_msg void OnSend();
	//}}AFX_MSG
	afx_msg LRESULT OnException(WPARAM indexFile, LPARAM message);
	afx_msg LRESULT OnProgress(WPARAM progress_percent, LPARAM operation);
	afx_msg LRESULT OnNextPhoto(WPARAM photo_index, LPARAM);
	afx_msg LRESULT OnNextOperation(WPARAM operation, LPARAM);
	DECLARE_MESSAGE_MAP()
	CString title_;
	CString operation_;

private:
	void Init(const TCHAR* dialog_title, const TCHAR* operation, UINT flags);
	void NextPhoto(int photo, bool after);
	void Progress(int progress_percent);
	void EnableSlideShowBtn();

	std::auto_ptr<ImgProcessingThread> worker_thread_;
	ImgProcessingPool& pool_;
	int threads_done_;
	int current_photo_;
	size_t file_count_;
	CSlideShowGenerator* slide_show_;
	CWnd* GetWnd()		{ return this; }
	void SetProgressBar(int current_photo, int progress);
	FileSend file_send_;
	bool auto_close_;
	bool output_only_;
	EstimatedTime est_time_left_;
	DWORD last_update_;
	CButton	exit_;
};
