/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "BMPReader.h"
#include "File.h"
#include "Dib.h"


struct BMPReader::Impl
{
	BITMAPFILEHEADER bfh_;
	BITMAPINFOHEADER header_;
	FileStream fs_;
	Offset clr_table_offset_;
	CSize img_size_;

	Impl()
	{
		memset(&header_, 0, sizeof header_);
		memset(&bfh_, 0, sizeof bfh_);
		clr_table_offset_ = 0;
		img_size_.cx = img_size_.cy = 0;
	}

	~Impl()
	{
	}

	ImageStat Open(const TCHAR* filePath);

	int Read(uint8* buf, int len);

	ImageStat ReadImage(Dib& dib);

	BYTE* GetLineBuffer(Dib& dib, int y);
	RGBTRIPLE* GetLineRgbBuffer(Dib& dib, int y);
};



BMPReader::BMPReader() : pImpl_(new Impl)
{
}

BMPReader::~BMPReader()
{
}


ImageStat BMPReader::Open(const TCHAR* filePath)
{
	try
	{
		return pImpl_->Open(filePath);
	}
	catch (...)
	{
		return IS_OPEN_ERR;
	}
}


int BMPReader::Impl::Read(uint8* buf, int len)
{
	try
	{
		fs_.Read(buf, len);
		return len;
	}
	catch (...)
	{
		return IS_OPEN_ERR;
	}
	return 0;
}


const uint16 BMP_MAGIC_VAL= 0x4d42;		// 'BM'


ImageStat BMPReader::Impl::Open(const TCHAR* filePath)
{
	if (!fs_.Open(filePath))
		return IS_OPEN_ERR;

	fs_.SetByteOrder(false);	// little endian
//	fs_.SetExceptions(false);

	fs_.Read(reinterpret_cast<uint8*>(&bfh_), sizeof bfh_);
	if (bfh_.bfType != BMP_MAGIC_VAL)
		return IS_FMT_NOT_SUPPORTED;

	memset(&header_, 0, sizeof header_);

	uint32 header_size= fs_.GetUInt32();
	fs_.RPosition(-4);

	switch (header_size)
	{
	case sizeof(BITMAPINFOHEADER):
		fs_.Read(reinterpret_cast<uint8*>(&header_), sizeof header_);
		break;

	case sizeof(BITMAPCOREHEADER):
		{
			BITMAPCOREHEADER core;
			fs_.Read(reinterpret_cast<uint8*>(&core), sizeof core);
			header_.biSize = core.bcSize;
			header_.biWidth = core.bcWidth;
			header_.biHeight = core.bcHeight;
			header_.biPlanes = core.bcPlanes;
			header_.biBitCount = core.bcBitCount;
		}
		break;

	case 64: //sizeof(OS2_BMP_HEADER):
		fs_.Read(reinterpret_cast<uint8*>(&header_), header_size);
		break;

	default:
		if (header_size > sizeof(header_))
		{
			fs_.Read(reinterpret_cast<uint8*>(&header_), sizeof(header_));
			fs_.RPosition(int32(header_size - sizeof(header_)));
		}
		break;
	}

	img_size_.cx = abs(header_.biWidth);
	img_size_.cy = abs(header_.biHeight);

	const uint32 MAX_SIZE= 0xffff;

	if (img_size_.cx > MAX_SIZE || img_size_.cy > MAX_SIZE || img_size_.cx == 0 || img_size_.cy == 0)
		return IS_FMT_NOT_SUPPORTED;

	//TODO: add more decoding...
	if (header_.biBitCount < 8)
		return IS_FMT_NOT_SUPPORTED;

	//TODO: ditto
	if (header_.biBitCount <= 8 && (header_.biCompression == BI_RLE8 || header_.biCompression == BI_RLE4))
		return IS_FMT_NOT_SUPPORTED;

	return IS_OK;
}


bool BMPReader::IsSupported() const
{
	return true;
}


CSize BMPReader::GetSize() const
{
	return pImpl_->img_size_;
}


ImageStat BMPReader::ReadImage(Dib& dib)
{
	return pImpl_->ReadImage(dib);
}


BYTE* BMPReader::Impl::GetLineBuffer(Dib& dib, int y)
{
	bool reversed= header_.biHeight < 0;
	return dib.LineBuffer(reversed ? y : img_size_.cy - y - 1);
}


RGBTRIPLE* BMPReader::Impl::GetLineRgbBuffer(Dib& dib, int y)
{
	return reinterpret_cast<RGBTRIPLE*>(GetLineBuffer(dib, y));
}


ImageStat BMPReader::Impl::ReadImage(Dib& dib)
{
	// prepare color map
	RGBQUAD color_map[256];
	bool grayscale= true;

	if (header_.biBitCount < 16)
	{
		size_t palette_entries= 256;
		switch (header_.biBitCount)
		{
		case 1:		palette_entries = 2;	break;
		case 4:		palette_entries = 16;	break;
		case 8:		palette_entries = 256;	break;
		}

		// number of colors used (and recorded) in a palette
		if (header_.biClrUsed > 0 && header_.biClrUsed < palette_entries)
			palette_entries = header_.biClrUsed;

		bool is_old_bmp= header_.biSize == sizeof(BITMAPCOREHEADER);

		if (is_old_bmp)
		{
			// read an old color table (3 byte entries)
			RGBTRIPLE pal[256];
			fs_.Read(reinterpret_cast<uint8*>(pal), static_cast<uint32>(palette_entries * sizeof(pal[0])));

			for (size_t i= 0; i < palette_entries; ++i)
			{
				color_map[i].rgbRed      = pal[i].rgbtRed;
				color_map[i].rgbBlue     = pal[i].rgbtBlue;
				color_map[i].rgbGreen    = pal[i].rgbtGreen;
				color_map[i].rgbReserved = 0;
			}
		}
		else
		{
			fs_.Read(reinterpret_cast<uint8*>(color_map), static_cast<uint32>(palette_entries * sizeof(color_map[0])));
		}

		for (int i= 0; i < palette_entries; ++i)
			if (grayscale && (color_map[i].rgbBlue != color_map[i].rgbGreen || color_map[i].rgbGreen != color_map[i].rgbRed))
			{
				grayscale = false;
				break;
			}
	}
	else
	{
		memset(color_map, 0, sizeof color_map);
		grayscale = false;
	}

	dib.Create(img_size_.cx, img_size_.cy, grayscale ? 8 : 24);

//	bool reversed= header_.biHeight < 0;

	if (bfh_.bfOffBits != 0L)
		fs_.RPosFromBeg(bfh_.bfOffBits);

	switch (header_.biBitCount)
	{
	case 32:
		if (header_.biCompression == BI_BITFIELDS || header_.biCompression == BI_RGB)
		{
			std::vector<RGBQUAD> line(img_size_.cx);

			for (int y= 0; y < img_size_.cy; ++y)
			{
				fs_.Read(reinterpret_cast<uint8*>(&line.front()), line.size() * sizeof(line[0]));
				RGBTRIPLE* p= GetLineRgbBuffer(dib, y);

				for (int x= 0; x < img_size_.cx; ++x)
				{
					p->rgbtRed   = line[x].rgbRed;
					p->rgbtGreen = line[x].rgbGreen;
					p->rgbtBlue  = line[x].rgbBlue;
					++p;
				}
			}
		}
		else
			return IS_FMT_NOT_SUPPORTED;

		break;

	case 24:
		if (header_.biCompression == BI_RGB)
		{
			int pad= (img_size_.cx * 3) & 3;
			if (pad)
				pad = 4 - pad;

			for (int y= 0; y < img_size_.cy; ++y)
			{
				BYTE* p= GetLineBuffer(dib, y);
				fs_.Read(p, img_size_.cx * 3);
				if (pad)
					fs_.RPosition(pad);
			}
		}
		else
			return IS_FMT_NOT_SUPPORTED;

		break;

	case 8:
		if (header_.biCompression == BI_RGB)
		{
			int line_bytes= (img_size_.cx + 3) & ~3;
			std::vector<uint8> line(line_bytes);

			for (int y= 0; y < img_size_.cy; ++y)
			{
				fs_.Read(&line.front(), line.size());

				if (grayscale)
				{
					BYTE* p= reinterpret_cast<BYTE*>(GetLineRgbBuffer(dib, y));
					for (int x= 0; x < img_size_.cx; ++x)
						*p++ = color_map[line[x]].rgbBlue;
				}
				else
				{
					RGBTRIPLE* p= GetLineRgbBuffer(dib, y);
					for (int x= 0; x < img_size_.cx; ++x)
					{
						p->rgbtRed   = color_map[line[x]].rgbRed;
						p->rgbtGreen = color_map[line[x]].rgbGreen;
						p->rgbtBlue  = color_map[line[x]].rgbBlue;
						++p;
					}
				}
			}
		}
		break;

	//TODO
	// case 4, 2, 1:

	default:
		return IS_FMT_NOT_SUPPORTED;
	}

	fs_.Close();

	return IS_OK;
}
