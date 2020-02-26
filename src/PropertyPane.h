/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class AutoCompletePopup;
#include <boost/ptr_container/ptr_vector.hpp>
#include "DlgAutoResize.h"

namespace Property
{

struct Field;


class Pane : public CDialog
{
public:
	Pane(AutoCompletePopup& auto_complete, boost::ptr_vector<Field>& fields);
	Pane(AutoCompletePopup& auto_complete, boost::ptr_vector<Field>& fields, int dlg_id);

	enum { EDIT_ID= 12000 };
	enum { POPUP_ID= 14000 };

	DlgAutoResize resize_;
	bool modified_;

	virtual void OnOK()	{}
	virtual void OnCancel()	{}
	virtual BOOL OnInitDialog();

	Field& GetField(size_t index);
	size_t FieldCount() const;

	DECLARE_MESSAGE_MAP()

	void OnSize(UINT type, int cx, int cy);

private:
	void OnEditChange(UINT id);
	void OnFocusSet(UINT id);
	void OnFocusKill(UINT id);
	HBRUSH OnCtlColor(CDC* dc, CWnd* ctrl, UINT code);
	void OnAutoCompleteDropDown(UINT code);
	void OnFocusSetNotification(UINT code, NMHDR* hdr, LRESULT* res);
	void OnChangeNotification(UINT code, NMHDR* hdr, LRESULT* res);

	CBrush current_;
	AutoCompletePopup& auto_complete_;
	boost::ptr_vector<Field>& fields_;
};


} // namespace
