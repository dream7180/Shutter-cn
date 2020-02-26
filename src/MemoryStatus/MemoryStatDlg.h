/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "../DlgAutoResize.h"
#include "../Dib.h"

// MemoryStatDlg dialog

class MemoryStatDlg : public CDialog
{
public:
	MemoryStatDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~MemoryStatDlg();

// Dialog Data
	enum { IDD = IDD_MEMORY_STAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnSize(UINT type, int cx, int cy);
	void OnExpand();
	void OnShowPicture();
	BOOL OnEraseBkgnd(CDC* pDC);
	void Refresh();

	DECLARE_MESSAGE_MAP()

	DlgAutoResize resize_;
	bool expand_blocks_;
	CFont font_;
	CWnd listBox_;
	CWnd virtLabel_;
	HMODULE PSAPI_module_;
	Dib dib_;
	bool showing_picture_;
};
