#pragma once

class DialogBase : public CDialog
{
public:
	DialogBase();
	explicit DialogBase(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	explicit DialogBase(UINT nIDTemplate, CWnd* pParentWnd = NULL);

protected:
	DECLARE_MESSAGE_MAP()

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);

private:
	CBrush background_brush_;
};
