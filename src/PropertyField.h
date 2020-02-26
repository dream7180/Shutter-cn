/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "EditHistory.h"
#include "PopupMenuCtrl.h"

namespace Property
{

// admittedly this implementation stinks and is to be redone; get rid of enum & hide public members

struct Field
{
	const TCHAR* name;
	int lines;
	String data;
	CStatic label;
	std::auto_ptr<CWnd> edit;
	CPopupMenuCtrl popup_btn_;	// button to popup auto complete history
	enum Type { EDITBOX, SEPARATOR, DATE_TIME, STARS } type;
	String* text;
//	IAutoCompletePtr auto_complete_;
//	IEnumStringPtr edit_history_;
	std::auto_ptr<EditHistory> history_;

	Field(const TCHAR* name, String* text) : name(name), lines(1), type(EDITBOX), text(text) //, history_(0)
	{}

	Field(const TCHAR* name, int lines, String* text) : name(name), lines(lines), type(EDITBOX), text(text) //, history_(0)
	{}

	Field(const TCHAR* name, Type type, String* text) : name(name), lines(lines), type(type), text(text) //, history_(0)
	{}

	Field() : name(0), lines(1), type(SEPARATOR), text(0) //, history_(0)
	{}

	~Field()
	{}

	void AddCurTextToHistory();

	void SaveHistory();

	const std::vector<String>* GetHistory();

	void ShowAutoCompleteBtn(bool show);
};


} // namespace
