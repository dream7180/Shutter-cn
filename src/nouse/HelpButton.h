/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// HelpButton

class HelpButton : public CButton
{
public:
	HelpButton();
	virtual ~HelpButton();

protected:
	DECLARE_MESSAGE_MAP()

	virtual void PreSubclassWindow();

private:
	CImageList image_list_;
	CBitmap bitmap_;
};
