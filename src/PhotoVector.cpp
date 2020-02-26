/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoVector.cpp: implementation of the CPhotoVector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoInfoStorage.h"
#include "PhotoVector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPhotoVector::CPhotoVector(PhotoInfoStorage& photos, CListCtrl& list_wnd)
 : photos_(photos), list_wnd_(list_wnd)
{}

CPhotoVector::~CPhotoVector()
{}


// count of photos
//
int CPhotoVector::GetCount() const
{
	return list_wnd_.GetItemCount();
}


// return photo (in correct order--current list ctrl order)
//
PhotoInfo& CPhotoVector::GetPhoto(int index) const
{
	if (index < 0 || index >= GetCount())
	{
		ASSERT(false);
		throw 2;
	}
	PhotoInfoPtr photo= reinterpret_cast<PhotoInfoPtr>(list_wnd_.GetItemData(index));
	if (photo == 0)
	{
		ASSERT(false);
		throw 3;
	}
	return *photo;
/*	int photo_no= list_wnd_.GetItemData(index);
	if (photo_no < 0 || photo_no >= GetCount())
	{
		ASSERT(false);
		throw 3;
	}
	return photos_[photo_no]; */
}
