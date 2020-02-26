// Dib.cpp: implementation of the CDib class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dib.h"
#include "JPEGException.h"
#include <math.h>

//extern HDRAWDIB g_hDrawDib;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDib::CDib()
{
//	m_hDD = 0;
	m_nWidth = m_nHeight = 0;
	m_nLineWidth = 0;
	m_nBitsPerPixel = 0;
	m_pDataBuff = 0;
}

CDib::CDib(int nWidth, int nHeight, int nBitsPerPixel)
{
//	m_hDD = 0;
	Create(nWidth, nHeight, nBitsPerPixel);
}


void CDib::Create(int nWidth, int nHeight, int nBitsPerPixel)
{
	MDC dc;
	dc.CreateDC("DISPLAY", 0, 0, 0);

	int nColorComponents= nBitsPerPixel / 8;
	ASSERT(nColorComponents == 1 || nColorComponents == 3);

	m_nWidth = nWidth; //(nWidth + 3) & ~3;
	m_nHeight = nHeight;
//	m_nLineWidth = m_nWidth * nColorComponents;
	m_nBitsPerPixel = nBitsPerPixel;
	// create a bitmap
	m_BmpInfo.bmiHeader.biSize = sizeof m_BmpInfo.bmiHeader;
	m_BmpInfo.bmiHeader.biWidth = m_nWidth;
	m_BmpInfo.bmiHeader.biHeight = m_nHeight;
	m_BmpInfo.bmiHeader.biPlanes = 1;
	m_BmpInfo.bmiHeader.biBitCount = m_nBitsPerPixel;
	m_BmpInfo.bmiHeader.biCompression = BI_RGB;
	m_BmpInfo.bmiHeader.biSizeImage = 0;
	m_BmpInfo.bmiHeader.biXPelsPerMeter = 0;
	m_BmpInfo.bmiHeader.biYPelsPerMeter = 0;
	m_BmpInfo.bmiHeader.biClrUsed = 0;
	m_BmpInfo.bmiHeader.biClrImportant = 0;

	if (nColorComponents == 1)
		for (int i= 0; i < 256; ++i)
		{
			m_BmpInfo.bmiColors[i].rgbBlue = i;		// initialize color palette with shades of gray
			m_BmpInfo.bmiColors[i].rgbGreen = i;
			m_BmpInfo.bmiColors[i].rgbRed = i;
			m_BmpInfo.bmiColors[i].rgbReserved = 0;
		}

	VOID* pData;
	HBITMAP hDib= ::CreateDIBSection(dc, &m_BmpInfo, DIB_PAL_COLORS, &pData, 0, 0);
	if (hDib == 0 || pData == 0)
		throw new CJPEGException("Out of memory");

	m_pDataBuff = reinterpret_cast<BYTE*>(pData);

	m_bmpDib.Attach(hDib);

	DIBSECTION ds;
	::GetObject(hDib, sizeof ds, &ds);
	m_nLineWidth = ds.dsBm.bmWidthBytes;

	// wipe out
	memset(m_pDataBuff, 0xff, m_nLineWidth * m_nHeight);
}


CDib::~CDib()
{
//	if (m_hDD)
//		::DrawDibClose(m_hDD);
}


static void FillSolidRect(HDC hDC, int x, int y, int cx, int cy, COLORREF clr)
{
	::SetBkColor(hDC, clr);
	MRect rect(x, y, x + cx, y + cy);
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
}


void CDib::Draw(HDC hDC, const MRect& rect)
{
/*	if (g_hDrawDib)
	{
		::DrawDibDraw(g_hDrawDib, *pDC, rect.left, rect.top, rect.Width(), rect.Height(),
			&bi.bmiHeader, m_pDataBuff, 0, 0, m_nWidth, m_nHeight, DDF_HALFTONE);
	}
	else */
	{
		::SetStretchBltMode(hDC, HALFTONE);
		::StretchDIBits(hDC, rect.left, rect.top, rect.Width(), rect.Height(),
			0, 0, m_nWidth, m_nHeight, m_pDataBuff, &m_BmpInfo, DIB_RGB_COLORS, SRCCOPY);
	}
}


void CDib::DrawImg(HDC hDC, const MRect& rectDest, COLORREF rgbBack)
{
	ASSERT(hDC);
/*
	if (m_pBmp.empty())
	{
		pDC->FillSolidRect(rectDest, rgbBack);
		return;
	}

	MSize sizeBmp= m_pBmp->GetSize();
	if (sizeBmp.cx < 1 || sizeBmp.cy < 1)
	{
		ASSERT(false);
		return;
	}
*/
	if (m_nWidth < 1 || m_nHeight < 1)
	{
		::SetBkColor(hDC, rgbBack);
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rectDest, 0, 0, 0);
		return;
	}

	MSize sizeBmp(m_nWidth, m_nHeight);

	MSize sizeDest(rectDest.Width(), rectDest.Height());

	if (sizeDest.cx <= 0 || sizeDest.cy <= 0)
		return;

	double dBmpRatio= double(sizeBmp.cx) / double(sizeBmp.cy);

	double dDestRatio= double(sizeDest.cx) / double(sizeDest.cy);

	double dEpsillon= 0.01;

	// compare bmp ratio and destination ratio; if same bmp will be simply scalled
	if (fabs(dBmpRatio - dDestRatio) < dEpsillon)
	{
		Draw(hDC, rectDest);
	}
	else
	{
		// calc how to rescale bmp to fit into dest rect
		double dScaleW= double(sizeDest.cx) / double(sizeBmp.cx);
		double dScaleH= double(sizeDest.cy) / double(sizeBmp.cy);

		double dScale= min(dScaleW, dScaleH);

		// rescale bmp
		sizeBmp.cx = static_cast<int>(sizeBmp.cx * dScale);
		sizeBmp.cy = static_cast<int>(sizeBmp.cy * dScale);

		ASSERT(sizeBmp.cx <= sizeDest.cx);
		ASSERT(sizeBmp.cy <= sizeDest.cy);
		MSize sizeDiff(sizeDest.cx - sizeBmp.cx, sizeDest.cy - sizeBmp.cy);

		// center rescalled bitmap in destination rect
		int ptPosX= sizeDiff.cx / 2;
		int ptPosY= sizeDiff.cy / 2;

		// fill not covered areas
		if (sizeDiff.cx > sizeDiff.cy)
		{
			FillSolidRect(hDC, rectDest.left, rectDest.top, ptPosX, sizeDest.cy, rgbBack);
			FillSolidRect(hDC, rectDest.left + ptPosX + sizeBmp.cx, rectDest.top, sizeDest.cx - ptPosX - sizeBmp.cx, sizeDest.cy, rgbBack);
		}
		else
		{
			FillSolidRect(hDC, rectDest.left, rectDest.top, sizeDest.cx, ptPosY, rgbBack);
			FillSolidRect(hDC, rectDest.left, rectDest.top + ptPosY + sizeBmp.cy, sizeDest.cx, sizeDest.cy - ptPosY - sizeBmp.cy, rgbBack);
		}

		// finally draw the bitmap
		MRect rect(ptPosX, ptPosY, ptPosX + sizeBmp.cx, ptPosY + sizeBmp.cy);
//		rect.OffsetRect(rectDest.TopLeft());
		rect.left	+= rectDest.left;
		rect.right	+= rectDest.left;
		rect.top	+= rectDest.top;
		rect.bottom	+= rectDest.top;

		Draw(hDC, rect);
	}
}


#if 0
void CDib::Draw(MDC* pDC, MRect rectDest, MRect* prectSrc)
{
	ASSERT(m_bmpDib.m_hObject != 0);

	if (!m_hDD)
		VERIFY(m_hDD = ::DrawDibOpen());

	if (prectSrc == 0)
	{
		pDC->SetStretchBltMode(HALFTONE);
//		::DrawDibDraw(m_hDD, *pDC,
//			rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
//			&m_BmpInfo.bmiHeader, m_pDataBuff,
//			0, 0, m_nWidth, m_nHeight, DDF_HALFTONE);
		::StretchDIBits(*pDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
			0, 0, m_nWidth, m_nHeight, m_pDataBuff, &m_BmpInfo, DIB_RGB_COLORS, SRCCOPY);
	}
	else
	{
		pDC->SetStretchBltMode(HALFTONE);
		::StretchDIBits(*pDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
			prectSrc->left, m_BmpInfo.bmiHeader.biHeight - prectSrc->top - prectSrc->Height() + 1, prectSrc->Width(), prectSrc->Height(), m_pDataBuff, &m_BmpInfo, DIB_RGB_COLORS, SRCCOPY);
/*
		::DrawDibDraw(m_hDD, *pDC,
			rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
			&m_BmpInfo.bmiHeader, m_pDataBuff,
			prectSrc->left, prectSrc->top, prectSrc->Width(), prectSrc->Height(),
			DDF_HALFTONE);
*/
	}
}


void CDib::Draw(MDC* pDC, CPoint ptPos) const
{
	// do not use HALFTONE
	pDC->SetStretchBltMode(COLORONCOLOR);

	::StretchDIBits(*pDC, ptPos.x, ptPos.y, m_nWidth, m_nHeight,
		0, 0, m_nWidth, m_nHeight, m_pDataBuff, &m_BmpInfo, DIB_RGB_COLORS, SRCCOPY);
}


// Resize dib to requested dimensions; center it and fill not covered areas
// with rgbBack color
//
void CDib::Resize(MSize sizeImg, COLORREF rgbBack, bool bUniform)
{
	if (sizeImg.cx < 0)		// negative value means percent
		sizeImg.cx = m_nWidth * (-sizeImg.cx) / 100;
	if (sizeImg.cy < 0)
		sizeImg.cy = m_nHeight * (-sizeImg.cy) / 100;

	int nX= 0;
	int nY= 0;
	int nDestW= sizeImg.cx;
	int nDestH= sizeImg.cy;

	CDib dibCopy(sizeImg.cx, sizeImg.cy, m_nBitsPerPixel);

	MDC dcSrc;
	dcSrc.CreateCompatibleDC(0);
	dcSrc.SelectObject(&m_bmpDib);

	MDC dcDst;
	dcDst.CreateCompatibleDC(0);
	dcDst.SelectObject(&dibCopy.m_bmpDib);

	dcDst.FillSolidRect(0, 0, sizeImg.cx, sizeImg.cy, rgbBack);

	if (bUniform)
	{
		double dRatioImg= double(m_nWidth) / double(m_nHeight);
		if (double(sizeImg.cx) / double(sizeImg.cy) > dRatioImg)
			nDestW = nDestH * dRatioImg;
		else
			nDestH = nDestW / dRatioImg;
		nX = (sizeImg.cx - nDestW) / 2;
		nY = (sizeImg.cy - nDestH) / 2;
	}

	dcDst.SetStretchBltMode(HALFTONE);
	dcDst.StretchBlt(nX, nY, nDestW, nDestH, &dcSrc, 0, 0, m_nWidth, m_nHeight, SRCCOPY);

	m_bmpDib.DeleteObject();
	m_bmpDib.Attach(dibCopy.m_bmpDib.Detach());
	m_pDataBuff	 = dibCopy.m_pDataBuff;
	m_nWidth	 = dibCopy.m_nWidth;
	m_nLineWidth = dibCopy.m_nLineWidth;
	m_nHeight	 = dibCopy.m_nHeight;
	m_BmpInfo	 = dibCopy.m_BmpInfo;
}


// Create device dependent bmp form dib
//
AutoPtr<CBitmap> CDib::DeviceBitmap()
{
	AutoPtr<CBitmap> pBmp(new CBitmap());

	MDC dc;
	dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);

	pBmp->CreateCompatibleBitmap(&dc, m_nWidth, m_nHeight);

	MDC dcDst;
	dcDst.CreateCompatibleDC(0);
	dcDst.SelectObject(pBmp.get());

	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = m_nWidth;
	bi.bmiHeader.biHeight = -m_nHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = m_nBitsPerPixel;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

/*	if (g_hDrawDib)
	{
		::DrawDibDraw(g_hDrawDib, dcDst, 0, 0, m_nWidth, m_nHeight,
			&bi.bmiHeader, m_pDataBuff, 0, 0, m_nWidth, m_nHeight, DDF_HALFTONE);
	}
	else */
	{
		dcDst.SetStretchBltMode(HALFTONE);
		::StretchDIBits(dcDst, 0, 0, m_nWidth, m_nHeight, 0, 0, m_nWidth, m_nHeight, m_pDataBuff, &bi, DIB_RGB_COLORS, SRCCOPY);
	}

	return pBmp;
}


void CDib::Save(const TCHAR* pcszOutputFile)
{
	if (GetColorComponents() != 3)
		AfxThrowFileException(0, 0, pcszOutputFile); //throw new CException();

	CFile out(pcszOutputFile, CFile::modeWrite | CFile::modeCreate);

	BITMAPFILEHEADER bfh;
	bfh.bfType = 'MB';
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof bfh + sizeof BITMAPINFOHEADER;
	bfh.bfSize = bfh.bfOffBits;
	bfh.bfSize += (((m_nWidth * GetColorComponents()) + 3) & ~3) * m_nHeight;

	out.Write(&bfh, sizeof bfh);
	out.Write(&m_BmpInfo.bmiHeader, sizeof m_BmpInfo.bmiHeader);

	int nLineBytes= m_nWidth * GetColorComponents();
	int nLinePadding= (4 - (nLineBytes & 3)) & 3;

	for (int y= m_nHeight - 1; y >= 0; --y)
	{
		const BYTE* pcScanLineData= LineBuffer(y);
		out.Write(pcScanLineData, nLineBytes);
		if (nLinePadding > 0)
		{
			static const BYTE vPadding[4]= { 0, 0, 0, 0 };
			out.Write(vPadding, nLinePadding);
		}
	}
}


bool CDib::Load(int nBmpRsrcId)
{
	HINSTANCE hInst= AfxFindResourceHandle(MAKEINTRESOURCE(nBmpRsrcId), RT_BITMAP);
	HANDLE hDib= ::LoadImage(hInst, MAKEINTRESOURCE(nBmpRsrcId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hDib == 0)
		return false;

	m_bmpDib.Attach(hDib);
	DIBSECTION ds;
	m_bmpDib.GetObject(sizeof ds, &ds);

	m_BmpInfo.bmiHeader = ds.dsBmih;
	m_nWidth = ds.dsBm.bmWidth;
	m_nHeight = ds.dsBm.bmHeight;
	m_nBitsPerPixel = ds.dsBm.bmBitsPixel;
	m_nLineWidth = ds.dsBm.bmWidthBytes;
	m_pDataBuff = reinterpret_cast<BYTE*>(ds.dsBm.bmBits);

	if (m_nBitsPerPixel <= 8)
	{
		MDC dc;
		dc.CreateCompatibleDC(0);
		dc.SelectObject(&m_bmpDib);

		::GetDIBColorTable(dc, 0, ColorTableSize(m_nBitsPerPixel), m_BmpInfo.bmiColors);
	}

	return true;
}
#endif

int CDib::ColorTableSize(int nBitsPerPixel)
{
	int nColors= 0;
	switch (nBitsPerPixel)
	{
	case 8:
		nColors = 256;
		break;
	case 4:
		nColors = 16;
		break;
	case 1:
		nColors = 2;
		break;
	}
	return nColors;
}
