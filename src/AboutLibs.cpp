/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// AboutLibs.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AboutLibs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CAboutLibs dialog

CAboutLibs::CAboutLibs(CWnd* parent /*=NULL*/) : CDialog(CAboutLibs::IDD, parent)
{
}

CAboutLibs::~CAboutLibs()
{
}

void CAboutLibs::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CAboutLibs, CDialog)
END_MESSAGE_MAP()


// CAboutLibs message handlers
