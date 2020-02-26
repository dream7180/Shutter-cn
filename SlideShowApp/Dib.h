// Dib.h: interface for the CDib class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIB_H__BF29F0E1_CEAC_4E07_ACA4_AD81F2E6E807__INCLUDED_)
#define AFX_DIB_H__BF29F0E1_CEAC_4E07_ACA4_AD81F2E6E807__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CDib
{
public:
	CDib();
	CDib(int nWidth, int nHeight, int nBitsPerPixel);
	virtual ~CDib();

	void Create(int nWidth, int nHeight, int nBitsPerPixel);

	bool Load(int nBmpRsrcId);

	int GetWidth() const			{ return m_nWidth; }
	int GetHeight() const			{ return m_nHeight; }
	int GetLineBytes() const		{ return m_nLineWidth; }
	int GetBytesPerLine() const		{ return m_nLineWidth; }
	int GetColorComponents() const	{ return m_nBitsPerPixel / 8; }
	int GetBitsPerPixel() const		{ return m_nBitsPerPixel; }

	const BYTE* GetBuffer() const	{ return m_pDataBuff; }

	BYTE* LineBuffer(int nLine)
	{ return size_t(nLine) < size_t(m_nHeight) ? m_pDataBuff + (m_nHeight - nLine - 1) * m_nLineWidth : 0; }

	const BYTE* LineBuffer(int nLine) const
	{ return size_t(nLine) < size_t(m_nHeight) ? m_pDataBuff + (m_nHeight - nLine - 1) * m_nLineWidth : 0; }

	void DrawImg(HDC hDC, const MRect& rectDest, COLORREF rgbBack);
	void Draw(HDC hDC, const MRect& rect);

	void Save(const TCHAR* pcszOutputFile);

	static int ColorTableSize(int nBitsPerPixel);

private:
	MBitmap m_bmpDib;
	int m_nWidth;
	int m_nHeight;
	int m_nLineWidth;
	int m_nBitsPerPixel;
	BYTE* m_pDataBuff;

	BITMAPINFO m_BmpInfo;
	RGBQUAD m_vColorTable[255];	// this vector must follow BITMAPINFO struct

//	HDRAWDIB m_hDD;		// for DrawDib API

	CDib& operator = (const CDib&);
	CDib(const CDib&);
};

#endif // !defined(AFX_DIB_H__BF29F0E1_CEAC_4E07_ACA4_AD81F2E6E807__INCLUDED_)
