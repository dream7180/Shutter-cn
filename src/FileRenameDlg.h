/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "PathEdit.h"
#include <boost/function.hpp>
class Dib;


// FileRenameDlg dialog

class FileRenameDlg : public CDialog
{
public:
	typedef boost::function<void (const String& name)> Callback;

	FileRenameDlg(const String& name, const String& path, const Dib* img, const Callback& fn, CWnd* parent);
	virtual ~FileRenameDlg();

// Dialog Data
	enum { IDD = IDD_FILE_RENAME };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CPathEdit name_;
	CStatic path_;
	CButton ok_;
	String filePath_;
	String fileName_;
	const Dib* image_;
	CRect rectImage_;
	Callback action_;
	CStatic warning_icon_;

	afx_msg void OnNameChange();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
};
