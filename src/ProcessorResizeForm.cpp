/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ProcessorResizeForm.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessorResizeForm.h"


// ProcessorResizeForm dialog

ProcessorResizeForm::ProcessorResizeForm(CWnd* parent /*=NULL*/)
	: CDialog(ProcessorResizeForm::IDD, parent)
{

}

ProcessorResizeForm::~ProcessorResizeForm()
{
}

void ProcessorResizeForm::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(ProcessorResizeForm, CDialog)
END_MESSAGE_MAP()


void ProcessorResizeForm::Create(CWnd* parent)
{
	CDialog::Create(IDD, parent);
}

// ProcessorResizeForm message handlers
