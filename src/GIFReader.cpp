/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "GIFReader.h"
#include "File.h"
#include "Dib.h"
//extern "C" {
#include "giflib\gif_lib.h"
//}


struct GIFReader::Impl
{
	GifFileType* gif_;
	FileStream fs_;
	CSize img_size_;
//	short color_table_flag_;
	//short bkgnd_color_index_;
	//short pixel_aspect_ratio_;
	//short color_resolution_;
	//short color_table_size_;

	Impl()
	{
		gif_ = 0;
	}

	~Impl()
	{
		if (gif_)
			::DGifCloseFile(gif_);
	}

	ImageStat Open(const TCHAR* filePath);

	static int ReadFn(GifFileType* gif, GifByteType* t, int n);
	int Read(uint8* buf, int len);

	ImageStat ReadImage(Dib& dib);
};



GIFReader::GIFReader() : pImpl_(new Impl)
{
}

GIFReader::~GIFReader()
{
}


ImageStat GIFReader::Open(const TCHAR* filePath)
{
	return pImpl_->Open(filePath);
}


int GIFReader::Impl::Read(uint8* buf, int len)
{
	try
	{
		fs_.Read(buf, len);
		return len;
	}
	catch (...)
	{}
	return 0;
}


int GIFReader::Impl::ReadFn(GifFileType* gif, GifByteType* buf, int n)
{
	return static_cast<Impl*>(gif->UserData)->Read(buf, n);
}


ImageStat GIFReader::Impl::Open(const TCHAR* filePath)
{
	if (gif_ != 0)
		return IS_OPEN_ERR;

	if (!fs_.Open(filePath))
		return IS_OPEN_ERR;

	fs_.SetByteOrder(true);	// big endian
	fs_.SetExceptions(false);

	if (gif_ == 0)
		gif_ = ::DGifOpen(this, &GIFReader::Impl::ReadFn);

	if (gif_ == 0)
		return IS_OPEN_ERR;

	img_size_.cx = gif_->SWidth;
	img_size_.cy = gif_->SHeight;

	return IS_OK;
}


bool GIFReader::IsSupported() const
{
	return false;
}


CSize GIFReader::GetSize() const
{
	return pImpl_->img_size_;
}


ImageStat GIFReader::ReadImage(Dib& dib)
{
	return pImpl_->ReadImage(dib);
}


ImageStat GIFReader::Impl::ReadImage(Dib& dib)
{
	if (gif_ == 0)
		return IS_OPEN_ERR;

	if (::DGifSlurp(gif_) != GIF_OK)
		return IS_READ_ERROR;

	if (gif_->SavedImages == 0)
		return IS_READ_ERROR;

	ColorMapObject* colors= gif_->SavedImages->ImageDesc.ColorMap;
	if (colors == 0)
		colors = gif_->SColorMap;
	if (colors == 0)
		return IS_FMT_NOT_SUPPORTED;

	bool grayscale= true;

	// prepare color map
	RGBQUAD color_map[256];
	for (int i= 0; i < 256; ++i)
		if (i < colors->ColorCount)
		{
			color_map[i].rgbBlue = colors->Colors[i].Blue;
			color_map[i].rgbGreen = colors->Colors[i].Green;
			color_map[i].rgbRed = colors->Colors[i].Red;

			if (grayscale && (color_map[i].rgbBlue != color_map[i].rgbGreen ||
				color_map[i].rgbGreen != color_map[i].rgbRed))
				grayscale = false;
		}
		else
		{
			color_map[i].rgbBlue = 0;
			color_map[i].rgbGreen = 0;
			color_map[i].rgbRed = 0;
		}

	int width= gif_->SWidth;
	int height= gif_->SHeight;

	dib.Create(gif_->SWidth, gif_->SHeight, grayscale ? 8 : 24);

	uint8* img_data= gif_->SavedImages->RasterBits;
	if (img_data == 0)
		return IS_FMT_NOT_SUPPORTED;

	std::vector<int> lines(height);

	if (gif_->SavedImages->ImageDesc.Interlace)
	{
		int i= 0;
		for (int line= 0; line < height; line += 8)
			lines[i++] = line;
		for (int line= 4; line < height; line += 8)
			lines[i++] = line;
		for (int line= 2; line < height; line += 4)
			lines[i++] = line;
		for (int line= 1; line < height; line += 2)
			lines[i++] = line;
		ASSERT(i == height);
	}
	else
	{
		for (int i= 0; i < height; ++i)
			lines[i] = i;
	}

	for (int index= 0; index < height; ++index)
	{
		int line= lines[index];
		ASSERT(line < height);
		BYTE* p= dib.LineBuffer(line);

		if (grayscale)
		{
			for (int x= 0; x < width; ++x)
			{
				uint8 pix= *img_data++;

				*p++ = color_map[pix].rgbBlue;
			}
		}
		else
		{
			for (int x= 0; x < width; ++x)
			{
				uint8 pix= *img_data++;

				p[0] = color_map[pix].rgbBlue;
				p[1] = color_map[pix].rgbGreen;
				p[2] = color_map[pix].rgbRed;

				p += 3;
			}
		}
	}

	::DGifCloseFile(gif_);

	gif_ = 0;

	return IS_OK;
}
