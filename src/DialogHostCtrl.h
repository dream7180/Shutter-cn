/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"



class DialogHostCtrl : public CWnd
{
public:
	DialogHostCtrl(UINT dialog_id);
	virtual ~DialogHostCtrl();

// Dialog Data

protected:
	DECLARE_MESSAGE_MAP()

private:
	UINT dialog_id_;
	CDialog dialog_;
	virtual void PreSubclassWindow();
	afx_msg int OnCreate(CREATESTRUCT* createStruct);
	void OnSize(UINT type, int cx, int cy);
};
