/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"


// ProcessorCropForm dialog

class ProcessorCropForm : public CDialog
{
public:
	ProcessorCropForm(CWnd* parent = NULL);   // standard constructor
	virtual ~ProcessorCropForm();

// Dialog Data
	enum { IDD = IDD_PROCESSOR_CROP };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
