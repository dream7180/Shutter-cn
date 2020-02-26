/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"
#include "DialogHostCtrl.h"


// ProcessorSharpenForm dialog

class ProcessorSharpenForm : public CDialog
{
public:
	ProcessorSharpenForm(CWnd* parent = NULL);   // standard constructor
	virtual ~ProcessorSharpenForm();

// Dialog Data
	enum { IDD = IDD_PROCESSOR_SHARPEN };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	DialogHostCtrl amount_;
	DialogHostCtrl radius_;
	DialogHostCtrl threshold_;
	virtual BOOL OnInitDialog();
};
