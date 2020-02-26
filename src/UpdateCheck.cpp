/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// UpdateCheck.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "UpdateCheck.h"
#include <afxinet.h>


UpdateCheck::UpdateCheck(CWnd* parent) : CDialog(UpdateCheck::IDD, parent)
{
}

UpdateCheck::~UpdateCheck()
{
}


void UpdateCheck::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(UpdateCheck, CDialog)
END_MESSAGE_MAP()


void Check()
{
	try
	{
		CInternetSession session(L"ExifPro", PRE_CONFIG_INTERNET_ACCESS, 0, 0, 0);

		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 3);
		session.SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000);
		session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);

		if (CHttpConnection* connection= session.GetHttpConnection(L"www.exifpro.com", INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 80, 0, 0))
		{
			std::auto_ptr<CHttpConnection> c(connection);

			if (CHttpFile* file= connection->OpenRequest(L"GET", L"version.dat"))
			{
				std::auto_ptr<CHttpFile> f(file);

				//file->AddRequestHeaders(L"Accept: text/data\r\n", HTTP_ADDREQ_FLAG_ADD_IF_NEW);
				file->SendRequest();

				DWORD status= 0;
				file->QueryInfoStatusCode(status);

				CString url= file->GetFileURL();
				ULONGLONG len= file->GetLength();

				char buf[64]= { 0 };
				file->Read(buf, std::min<UINT>(static_cast<UINT>(len), static_cast<UINT>(array_count(buf)) - 1));
			}
		}
	}
	catch (CInternetException* ex)
	{
		ex->ReportError();
	}
}


void UpdateCheck::OnOK()
{
	Check();
}
