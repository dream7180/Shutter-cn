/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// UniqueLetter.h: interface for the UniqueLetter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNIQUELETTER_H__C8B9ECB6_CA88_4A4B_9316_D44E89A0783F__INCLUDED_)
#define AFX_UNIQUELETTER_H__C8B9ECB6_CA88_4A4B_9316_D44E89A0783F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// this class is used to find unique shortcuts for given series of strings

class UniqueLetter
{
public:
	UniqueLetter();

	// insert '&' char before shortcut letter
	bool SelectUniqueLetter(String& text);

	void Reset();

	void Reserve(char letter);

private:
	enum { MAX= 'z' - 'a' + 1 };
	bool letters[MAX];		// array of used letter shortcuts
};

#endif // !defined(AFX_UNIQUELETTER_H__C8B9ECB6_CA88_4A4B_9316_D44E89A0783F__INCLUDED_)
