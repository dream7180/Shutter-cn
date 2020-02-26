/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// CAboutLibs dialog

class CAboutLibs : public CDialog
{
public:
	CAboutLibs(CWnd* parent = NULL);   // standard constructor
	virtual ~CAboutLibs();

// Dialog Data
	enum { IDD = IDD_ABOUT_LIBS };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
