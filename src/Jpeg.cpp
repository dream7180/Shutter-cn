/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Jpeg.cpp: implementation of the CJpeg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jpeg.h"
#include "JPEGDecoder.h"
#include "MemoryDataSource.h"
#include "JPEGException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJpeg::CJpeg()
{
}

CJpeg::~CJpeg()
{
}

// NOTE:
//
// locking is not necessary; no two threads are accessing this jpg data
// simultaneously; PhotoInfo is first created, then populated and appended
// to the storage list only after loading is complete


ImageStat CJpeg::GetDib(Dib& dib) const
{
//	CSingleLock lock(&access_ctrl_, true);

	if (jpeg_data_.empty())
		return IS_OPERATION_NOT_SUPPORTED;

	try
	{
		CMemoryDataSource memsrc(&jpeg_data_.front(), jpeg_data_.size());
		JPEGDecoder dec(memsrc, JDEC_INTEGER_HIQ); //g_Settings.dct_method_);
//dec.DecodeToYCbCr(dib, CSize(0, 0), false, 0.0);
		return dec.DecodeImg(dib);
	}
	catch (const JPEGException&)	// jpeg decoding error?
	{
//		CString msg= _T("Error reading embedded thumbnail image.\n\nFile: ");
//		msg += file;
//		msg += _T("\nDecoder message: ");
//		msg += ex.GetMessage();
//		msg += _T(".\n\nEXIF block is corrupted.");
//		AfxGetMainWnd()->MessageBox(msg, 0, MB_OK | MB_ICONWARNING);
		return IS_DECODING_FAILED;
	}
	catch (...)
	{
		return IS_READ_ERROR;
	}
	return IS_READ_ERROR;
}


bool CJpeg::IsEmpty() const
{
//	CSingleLock lock(&access_ctrl_, true);
	return jpeg_data_.empty();
}


void CJpeg::Empty()
{
//	CSingleLock lock(&access_ctrl_, true);

	if (!jpeg_data_.empty())
	{
		std::vector<uint8> empty;
		jpeg_data_.swap(empty);
	}
}


void CJpeg::SwapBuffer(std::vector<uint8>& buffer)
{
//	CSingleLock lock(&access_ctrl_, true);

	jpeg_data_.swap(buffer);
}


void CJpeg::SetBuffer(const std::vector<uint8>& buffer)
{
	jpeg_data_ = buffer;
}


const uint8* CJpeg::JpegData() const
{
	return jpeg_data_.empty() ? 0 : &jpeg_data_.front();
}


size_t CJpeg::JpegDataSize() const
{
	return jpeg_data_.size();
}
