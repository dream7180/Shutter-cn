/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class PhotoInfo;
#include "ToolBarWnd.h"
#include "DialogChild.h"
#include "PhotoInfoStorage.h"
#include <boost/function.hpp>

/////////////////////////////////////////////////////////////////////////////
// CDescriptionDlg dialog

class CDescriptionDlg : public CDialogChild
{
// Construction
public:
	typedef boost::function<void (PhotoInfo& photo)> PhotoChangedFn;

	CDescriptionDlg(CWnd* parent, PhotoInfo& inf, VectPhotoInfo& photos,
		VectPhotoInfo& selected, PhotoChangedFn fn= 0);

// Dialog Data
	//{{AFX_DATA(CDescriptionDlg)
	enum { IDD = IDD_DESCRIPTION };
	CToolBarWnd	tool_bar_wnd_;
	CToolBarWnd	navigation_wnd_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDescriptionDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDescriptionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPrevious();
	afx_msg void OnNext();
	afx_msg void OnChangeDescription();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CString title_;
	VectPhotoInfo& photos_;
	VectPhotoInfo& selected_;
	int current_photo_index_;
	bool unicode_;
	HWND edit_;
	CFont fnd_edit_;
	bool modified_;
	bool multiple_;
	PhotoChangedFn photo_changed_;
	CStatic infoText_;
	CProgressCtrl progressCtrl_;

	void BuildToolbar();
	void OnSymbol(UINT cmd_id);
	void SetDescriptionText(PhotoInfo* photo);
	bool WriteDescription(int index);
	bool Write(PhotoInfo* photo[], const size_t count);
	bool Load(int index);
	BOOL InitDlg();
	void UpdateButtons();
};
