/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class SnapFrame;


// ManagePaneLayouts dialog

class ManagePaneLayouts : public CDialog
{
public:
	ManagePaneLayouts(CWnd* parent, bool add_new);
	virtual ~ManagePaneLayouts();

// Dialog Data
	//{{AFX_DATA(ManagePaneLayouts)
	enum { IDD = IDD_MANAGE_PANE_LAYOUTS };
	CButton	btn_ok_;
	CEdit	edit_name_;
	CListBox list_wnd_;
	CButton btn_delete_;
	CString	name_;
	//}}AFX_DATA

	// list of pane layouts
	std::vector<std::pair<String, int>> names_;

protected:
	//{{AFX_MSG(ManagePaneLayouts)
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	afx_msg void OnDelete();
	afx_msg void OnSelChangeList();
	afx_msg void OnChangeName();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	void CheckDelButton();
	bool FindLayout(const TCHAR* name);

	// if true dlg used to add new layout
	bool adding_new_;
};
