// Photos.h: interface for the CPhotos class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOS_H__6AE177C5_3763_49C4_8714_51E5CA7C609B__INCLUDED_)
#define AFX_PHOTOS_H__6AE177C5_3763_49C4_8714_51E5CA7C609B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "PhotoInfo.h"
#include "Dib.h"


class CPhotos
{
public:
	CPhotos(const BYTE* pcSectionStart, DWORD dwSectionSize);

	~CPhotos();

	AutoPtr<CDib> GetPhoto(UINT uIndex);

	int Count() const		{ return m_pPhotosHeader ? m_pPhotosHeader->dwPhotoCount : 0; }

	DWORD Delay() const		{ return m_pPhotosHeader ? m_pPhotosHeader->dwDelayBetweenPhotos : 0; }

	bool StartFullScreen() const	{ return m_pPhotosHeader ? m_pPhotosHeader->dwStartFullScreen != 0 : false; }

	bool RepeatedLoop() const	{ return m_pPhotosHeader ? m_pPhotosHeader->dwReserved1 != 0 : false; }

private:
	struct PhotosHeader
	{
		BYTE vMagic[4];
		DWORD dwPhotoCount;
		DWORD dwDelayBetweenPhotos;
		DWORD dwStartFullScreen;
		DWORD dwReserved1;
		DWORD dwReserved2;
		const BYTE vJpegData[4];
	};

	const PhotosHeader* m_pPhotosHeader;
	PhotoInfoPtr* m_vPhotos;
	AutoPtr<CDib> GetDib(PhotoInfoPtr pPhoto);
};

#endif // !defined(AFX_PHOTOS_H__6AE177C5_3763_49C4_8714_51E5CA7C609B__INCLUDED_)
