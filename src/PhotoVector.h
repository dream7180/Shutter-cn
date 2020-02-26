/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoVector.h: interface for the CPhotoVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOVECTOR_H__5DD40863_6166_4DB6_A17D_BBA5D11A5D6D__INCLUDED_)
#define AFX_PHOTOVECTOR_H__5DD40863_6166_4DB6_A17D_BBA5D11A5D6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class PhotoInfoStorage;


class CPhotoVector
{
public:
	CPhotoVector(PhotoInfoStorage& pvPhotos, CListCtrl& list_wnd);
	virtual ~CPhotoVector();

	// count of photos
	int GetCount() const;

	// return photo (in correct order--current list ctrl order)
	PhotoInfo& GetPhoto(int index) const;
	PhotoInfo& operator [] (int index) const		{ return GetPhoto(index); }

private:
//	vector<PhotoInfo>& photos_;
	PhotoInfoStorage& photos_;
	CListCtrl& list_wnd_;

};

#endif // !defined(AFX_PHOTOVECTOR_H__5DD40863_6166_4DB6_A17D_BBA5D11A5D6D__INCLUDED_)
