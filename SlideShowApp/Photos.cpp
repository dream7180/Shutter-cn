// Photos.cpp: implementation of the CPhotos class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "assert.h"
#include "Photos.h"
#include "MemoryDataSource.h"
#include "JPEGDecoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPhotos::CPhotos(const BYTE* pcSectionStart, DWORD dwSectionSize)
{
	m_pPhotosHeader = reinterpret_cast<const PhotosHeader*>(pcSectionStart);

	const BYTE* pBeginAddr= m_pPhotosHeader->vJpegData;
	const BYTE* pEndAddr= pcSectionStart + dwSectionSize;

	m_vPhotos = new PhotoInfoPtr[m_pPhotosHeader->dwPhotoCount];

	// find all photo headers and store them inside m_vPhotos

	for (DWORD dwIndex= 0, dwOffset= 0; ; ++dwIndex)
	{
		const CPhotoInfo* pcPhoto= reinterpret_cast<const CPhotoInfo*>(pBeginAddr + dwOffset);

		assert(pcPhoto->m_dwJPEGSize > 0 && pcPhoto->m_vJPEGData[1] == 0xd8);

		m_vPhotos[dwIndex] = pcPhoto;

		if (dwIndex >= m_pPhotosHeader->dwPhotoCount - 1)
			break;

		dwOffset += pcPhoto->m_dwJPEGSize + (sizeof *pcPhoto - sizeof pcPhoto->m_vJPEGData);

		assert((const BYTE*)pcPhoto < pEndAddr);
	}
}


CPhotos::~CPhotos()
{
	if (m_vPhotos)
		delete [] m_vPhotos;
}


AutoPtr<CDib> CPhotos::GetDib(PhotoInfoPtr pPhoto)
{
	try
	{
		CMemoryDataSource src(pPhoto->m_vJPEGData, pPhoto->m_dwJPEGSize);
		CJPEGDecoder dec(src);
		return dec.Decode();
	}
	catch (...)
	{
		::MessageBox(HWND_DESKTOP, "Error extracting photograph.\nThis slide show might be corrupt.", "Slide Show", MB_OK | MB_ICONERROR);
	}
	return 0;
}


AutoPtr<CDib> CPhotos::GetPhoto(UINT uIndex)
{
	if (uIndex >= m_pPhotosHeader->dwPhotoCount)
	{
		assert(false);
		return 0;
	}

	return GetDib(m_vPhotos[uIndex]);
}
