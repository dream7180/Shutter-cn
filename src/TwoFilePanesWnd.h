/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class FilePaneWnd;


// TwoFilePanesWnd dialog

class TwoFilePanesWnd : public CDialog
{
public:
	TwoFilePanesWnd(CWnd* parent = NULL);   // standard constructor
	virtual ~TwoFilePanesWnd();

	bool Create(CWnd* parent, double paneSplit= 0.5);

	FilePaneWnd& GetLeftPane();
	FilePaneWnd& GetRightPane();

	void ShowLeftPane(bool show);

	double GetPaneSplitRatio() const;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	void OnSize(UINT, int cx, int cy);
	BOOL OnEraseBkgnd(CDC* dc);
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint pos);
	void OnMouseMove(UINT flags, CPoint pos);
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
};
