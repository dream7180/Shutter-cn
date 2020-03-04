/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
// ExifPro.h : main header file for the EXIFPRO application


class App : public CWinApp
{
public:
	App();
	virtual ~App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(App)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(App)
	afx_msg void OnAppAbout();
	//void OnUpdateCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR gdi_plus_token_;

	CString RelativeToCurrentDir(const TCHAR* path);
	bool InitializeInstance();
	void DisableItem(CCmdUI* cmd_ui);

	HANDLE app_event_;
};
