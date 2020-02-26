/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ProcessorSharpenForm.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessorSharpenForm.h"


// ProcessorSharpenForm dialog

ProcessorSharpenForm::ProcessorSharpenForm(CWnd* parent /*=NULL*/)
	: CDialog(ProcessorSharpenForm::IDD, parent), amount_(IDD_SLIDER_EDIT),
		radius_(IDD_SLIDER_EDIT), threshold_(IDD_SLIDER_EDIT)
{

}

ProcessorSharpenForm::~ProcessorSharpenForm()
{
}

void ProcessorSharpenForm::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(ProcessorSharpenForm, CDialog)
END_MESSAGE_MAP()


BOOL ProcessorSharpenForm::OnInitDialog()
{
	CDialog::OnInitDialog();

	amount_.SubclassDlgItem(IDC_AMOUNT, this);
	radius_.SubclassDlgItem(IDC_RADIUS, this);
	threshold_.SubclassDlgItem(IDC_THRESHOLD, this);

	return true;
}
