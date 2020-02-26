/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class Dib;
#include "DlgAutoResize.h"
#include "HelpButton.h"
#include "PhotoInfo.h"
#include "PhotoInfoPtr.h"
#include "HeaderDlg.h"


class DialogChild : public CDialog
{
// Construction
public:
	DialogChild(UINT id_template, CWnd* parent= NULL);   // standard constructor

	int GetId() const			{ return LOWORD(m_lpszTemplateName); }
// Dialog Data
	//{{AFX_DATA(DialogChild)
	enum { IDD = 0 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void ShowImage(Dib* image);
	void ShowImage(PhotoInfoPtr photo);

	virtual void Resize();

	void SetFooterDlg(CDialog* dlg);

	void ShowFooterDlg(bool show);

	void SubclassHelpBtn(const TCHAR* help_page);

	bool IsResizable() const;

	void SetBigTitle(const TCHAR* title);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogChild)
	//}}AFX_VIRTUAL

// Implementation
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL ContinueModal();
	virtual BOOL OnInitDialog();

	virtual CString GetDialogTitle();

	// access to some services provided by parent window
	void SetResizeCallback(HeaderDlg* parent);

protected:
	void EndDialog(int result);

	void OnHelpBtn();

	void BuildResizingMap();
	void SetWndResizing(int id, DlgAutoResize::ResizeFlag flags);
	void SetWndResizing(int id, DlgAutoResize::ResizeFlag flags, UINT half_flags);
	void SetControlsShift(CSize shift);

	HelpButton btn_help_;		// help btn
	CString help_page_;

	CScrollBar& GetGripWnd()	{ return grip_wnd_; }

	void Resize(const CRect& rect);

	void SetRightSide(int width, COLORREF color, const std::vector<float>& shades);
	void SetRightSide(int width);

	void SetMinimalDlgSize(CSize minimal);

private:
	void CreateGripWnd();

	bool resizable_dialog_;
	DlgAutoResize dlg_resize_map_;

	enum { IDC_GRIP = 50 };
	CScrollBar grip_wnd_;

	HeaderDlg* parent_;

	void OnWindowPosChanged(WINDOWPOS* wp);

	// Generated message map functions
	//{{AFX_MSG(DialogChild)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
