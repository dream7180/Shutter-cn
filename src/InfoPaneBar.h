/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "EditCombo.h"
#include <boost/function.hpp>


// InfoPaneBar dialog

class InfoPaneBar : public CDialog
{
public:
	InfoPaneBar();
	virtual ~InfoPaneBar();

	bool Create(CWnd* parent, const boost::function<void (const CString& filter, bool hide_unknown)>& filter, bool hideUnknown);

// Dialog Data
	enum { IDD = IDD_INFO_BAR };

	bool IsUnknownHidden();
	CString FilterText();

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	EditCombo filter_bar_;
	boost::function<void (const CString& filter, bool hide_unknown)> filter_;

	afx_msg void OnChangeFilter();
	afx_msg void OnClickedHide();
	void OnCancelQuickFilter();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);
	CBrush background_brush_;

	virtual void OnOK();
	virtual void OnCancel();
};
