/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "StdAfx.h"
#include "PropField.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CPropGroup::~CPropGroup()
{
	for (iterator it= begin(); it != end(); ++it)
		delete *it;
}
