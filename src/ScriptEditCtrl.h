/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "boost\noncopyable.hpp"

class ScriptEditCtrl : boost::noncopyable
{
public:
	ScriptEditCtrl();
	~ScriptEditCtrl();

	CStringW GetText();
	void GetWindowText(CString& text);
	void SetWindowText(const TCHAR* text);
	int GetWindowTextLength();

	void SetSel(int from, int end, bool no_scroll= false);
	void ReplaceSel(const TCHAR* new_text, bool can_undo= false);
	void SetFont(CFont* font);

	// colorize "text" & "number" variables
	void EnableInputAttribsColors(bool enable);

	CWnd* GetWnd();
	operator CWnd& ();
	HWND m_hWnd;
private:
	struct Impl;
	std::auto_ptr<Impl> impl_;
};
