/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "StarCtrl.h"


// PropertyFormStar dialog

class PropertyFormStar : public CDialog
{
public:
	PropertyFormStar(CWnd* parent = NULL);   // standard constructor
	virtual ~PropertyFormStar();

	bool Create(CWnd* parent, String* init);

	void Reset(String* init);

	String Read() const;

	StarCtrl star_;
	CSpinButtonCtrl spin_;

	virtual void OnOK() {}
	virtual void OnCancel() {}

// Dialog Data
	enum { IDD = IDD_PROPERTY_STARS };

	bool IsModified() const;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void OnNumChange();
	void OnFocusSet();
	void RatingChanged(int stars);

	bool ready_;
	bool modified_;

	DECLARE_MESSAGE_MAP()
};
