/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// RadioListBox

class RadioListBox : public CCheckListBox
{
public:
	RadioListBox();
	virtual ~RadioListBox();

	// turn on radio button style (true) or regular check boxes (false)
	void RadioButtons(bool radio);

protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
	void PreDrawItem(LPDRAWITEMSTRUCT draw_item_struct);
	void radioPreDrawItemNonThemed(CDC* dc, DRAWITEMSTRUCT& drawItem, int check, int cyItem);

	BOOL OnCheckChanged();

	bool radio_buttons_;
};
