/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// UniqueLetter.cpp: implementation of the UniqueLetter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UniqueLetter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UniqueLetter::UniqueLetter()
{
	Reset();
}


bool UniqueLetter::SelectUniqueLetter(String& text)
{
	int len= text.length();

	for (int i= 0; i < len; ++i)
	{
		int chr= tolower(text[i]) - 'a';

		if (chr >= 0 && chr < MAX)
		{
			if (!letters[chr])
			{
				text.insert(i, _T("&"));
				letters[chr] = true;
				return true;
			}
		}
	}

	return false;
}


void UniqueLetter::Reset()
{
	std::fill(letters, letters + MAX, false);
}


void UniqueLetter::Reserve(char letter)
{
	if (letter >= 'A' && letter <= 'Z')
		letter += 'a' - 'A';

	int index= letter;
	index -= 'a';
	if (index >= 0 && index < MAX)
		letters[index] = true;
}
