/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"


// ProcessorResizeForm dialog

class ProcessorResizeForm : public CDialog
{
public:
	ProcessorResizeForm(CWnd* parent = NULL);   // standard constructor
	virtual ~ProcessorResizeForm();

	void Create(CWnd* parent);

// Dialog Data
	enum { IDD = IDD_PROCESSOR_RESIZE };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
