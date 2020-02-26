/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "NEFReader.h"
#include "file.h"
#include "data.h"
#include <float.h>
#include <math.h>
//#include <boost/algorithm/string/trim.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static void TrimSpaces(std::string& str)
{
	if (str.empty())
		return;

	std::string::size_type i= 0;
	while (str[i] == _T(' '))
		++i;

	std::string::size_type j= str.length() - 1;
	while (j > i && str[j] == _T(' '))
		--j;

	if (i > 0 || j < str.length() - 1)
		str = str.substr(i, j - i + 1);
}


NEFReader::NEFReader()
{
	raw_width_ = width_ = height_ = 0;
	iwidth_ = iheight_ = 0;
	compression_ = 0;
	data_offset_ = 0;
	orientation_ = 0;
	tiff_samples_ = 0;
	nikon_curve_offset_ = 0;
	leaf_ = 0;
	bitbuf_ = 0;
	vbits_ = 0;
	maximum_ = 0;
	filters_ = left_margin_ = 0;
	shrink_ = 0;
	image_ = 0;
	four_color_rgb_ = half_size_ = false;
	colors_ = 0;
	camera_blue_ = camera_red_ = 0.0;
	use_coeff_ = false;
	use_camera_wb_ = false;
	use_auto_wb_ = true;
	pre_mul_[0] = pre_mul_[1] = pre_mul_[2] = pre_mul_[3] = 0.0;
	jpeg_data_offset_ = jpeg_data_length_ = 0;
	bits_per_sample_ = 0;

	second_decode_ = free_decode_ = first_decode_;
}


NEFReader::~NEFReader()
{
}


void NEFReader::ParseMakerNote(FileStream& nef)
{
	char buf[10];
	nef.Read(buf, 10);
	uint32 base= 0;

	if (memcmp(buf, "Nikon", 6) == 0)
	{

//	if (buf[6] < 2)		// version
//		return;

		base = nef.RPosition();

		uint16 order= nef_.GetUInt16();
		if (order != 'MM' && order != 'II')
			return;

//		nef.SetByteOrder(start[10] == 'M');

		if (nef.GetUInt16() != 0x2a)	// has to be 0x2a (this is IFD header)
			return;

		nef.RPosition(nef.GetUInt32() - 8);
	}
	else
		nef.RPosition(-10);

	uint16 entries= nef.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return;
	}

	for (int i= 0; i < entries; ++i)
	{
		uint16 tag= nef.GetUInt16();
		Data data(nef, base);
		Offset temp= nef.RPosition();

		switch (tag)
		{
		case 0x0c:	// old Nikons
			if (data.Components() == 4)
			{
				uint32 buf[4];
				data.ReadLongs(buf, 4);
				camera_red_ = buf[0];
				camera_red_ /= buf[1];
				camera_blue_ = buf[2];
				camera_blue_ /= buf[3];
			}
			break;

		case 0x8c:	// contrast curve
			{
				int size= data.Components() - 2;
				if (size > 0)
				{
					contrast_curve_.resize(size);
					nef.RPosition(base, data.GetData() + 2, false);
					nef.Read(&contrast_curve_.front(), size);
				}
			}
			break;

		case 0x96:
			nikon_curve_offset_ = base + data.GetData() + 2;
			break;

		case 0x97:	// R&B (camera white balance)
			{
				nef.RPosition(base, data.GetData(), false);

				if (model_ == "NIKON D100")
				{
					nef.RPosition(72);
					camera_red_ = nef.GetUInt16() / 256.0;
					camera_blue_ = nef.GetUInt16() / 256.0;
				}
				else if (model_ == "NIKON D2H")
				{
					nef.RPosition(10);
					camera_red_  = nef.GetUInt16();
					camera_red_ /= nef.GetUInt16();
					camera_blue_ = nef.GetUInt16();
					camera_blue_ /= nef.GetUInt16();
				}
				else if (model_ == "NIKON D70")
				{
					nef.RPosition(20);
					camera_red_  = nef.GetUInt16();
					camera_red_ /= nef.GetUInt16();
					camera_blue_ = nef.GetUInt16();
					camera_blue_ /= nef.GetUInt16();
				}
			}
			break;

		default:
			break;
		}

		nef.RPosition(temp, FileStream::beg);
	}

}


void NEFReader::ParseExif(FileStream& nef, uint32 base)
{
	uint16 entries= nef.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return;
	}

	for (int i= 0; i < entries; ++i)
	{
		uint16 tag= nef.GetUInt16();
		Data data(nef, 0); //ifd_offset);
		Offset temp= nef.RPosition();

TRACE(_T("exf: %x (= %d)\n"), int(tag), int(data.GetData()));

		switch (tag)
		{
		case 0x927c:
			nef.RPosition(data.GetData(), FileStream::beg);
			ParseMakerNote(nef);
			break;

		default:
			break;
		}

		nef.RPosition(temp, FileStream::beg);
	}
}


void NEFReader::ParseIFD(FileStream& nef, uint32 ifd_offset)
{
	uint16 entries= nef.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return;
	}

	for (int i= 0; i < entries; ++i)
	{
		uint16 tag= nef.GetUInt16();
		Data data(nef, 0); //ifd_offset);

TRACE(_T("tag: %x (= %d)\n"), int(tag), int(data.GetData()));

		Offset temp= nef.RPosition();

		switch (tag)
		{
		case 0xfe:		// sub file type
			// if (data.GetData() & 1)
			//		this is thumbnail;
			// else
			//		main img;
			break;

		case 0x100:		// img width
			raw_width_ = width_ = data.GetData();
			break;

		case 0x101:		// img height
			height_ = data.GetData();
			break;

		case 0x103:		// compression
			compression_ = data.GetData();
			break;

		case 0x10f:		// make
			make_ = data.AsAnsiString();
			TrimSpaces(make_);
			break;

		case 0x110:		// model
			model_ = data.AsAnsiString();
			TrimSpaces(model_);
			break;

		case 0x111:		// StripOffset
			data.ReadLongs(&data_offset_, 1);
			break;

		case 0x112:		// orientation
			orientation_ = data.GetData();
			break;

		case 0x115:		// SamplesPerPixel
			tiff_samples_ = data.GetData();
			break;

		case 0x102:		// TIFFTAG_BITSPERSAMPLE		258	/* bits per channel (sample) */
			bits_per_sample_ = data.GetData();
			break;

		case 0x14a:		// SubIFD
			// scan it...
			{
				uint32 len= data.Components();
				uint32 offset[8];
				data.ReadLongs(offset, std::min<uint32>(len, array_count(offset)));
				for (uint32 i= 0; i < len; ++i)
				{
					nef.RPosition(offset[i], FileStream::beg);
TRACE(_T("-->subifd\n"));
					ParseIFD(nef, nef.RPosition());
TRACE(_T("<--subifd\n"));
				}
			}
			break;

		case 0x201:		// JPEGInterchangeFormat
			break;

		case 0x828E:	// CFAPattern
			{
				uint8 cfa_pat[16];
				uint8 cfa_pc[] = { 0, 1, 2, 3 };
				uint32 plen= std::min<uint32>(16, data.Components());
				data.ReadUChar(cfa_pat, plen);
				colors_ = 0;
				int cfa= 0;
				for (int i= 0; i < plen; ++i)
				{
					colors_ += !(cfa & (1 << cfa_pat[i]));
					cfa |= 1 << cfa_pat[i];
				}
				if (cfa == 070) memcpy (cfa_pc, "\003\004\005", 3);	/* CMY */
				if (cfa == 072) memcpy (cfa_pc, "\005\003\004\001", 4);	/* GMCY */

				uint8 tab[256];
				for (int c= 0; c < colors_; ++c)
					tab[cfa_pc[c]] = c;
				for (int i= 16; i--; )
					filters_ = filters_ << 2 | tab[cfa_pat[i % plen]];
			}
			break;

		case 0x8769:	// EXIF tag
			nef.RPosition(data.GetData(), FileStream::beg);
			ParseExif(nef, data.GetData());
			break;

		case 0xC61D:	// WhiteLevel
			maximum_ = data.GetData();
			break;
		}

		nef.RPosition(temp, FileStream::beg);
	}

}


ImageStat NEFReader::Open(const TCHAR* file_path)
{
	nef_.Close();

	if (!nef_.Open(file_path))
		return IS_OPEN_ERR;

	if (nef_.GetLength32() < 200)	// too short?
		return IS_FMT_NOT_SUPPORTED;

	uint16 order= nef_.GetUInt16();
	if (order == 'MM')
		nef_.SetByteOrder(true);
	else if (order == 'II')
		nef_.SetByteOrder(false);
	else
		return IS_FMT_NOT_SUPPORTED;

	if (nef_.GetUInt16() != 42)		// TIFF magic number
		return IS_FMT_NOT_SUPPORTED;

	for (;;)
	{
		uint32 offset= nef_.GetUInt32();		// IFD offset
		if (offset == 0)
			break;

		nef_.RPosition(offset, FileStream::beg);	// go to the IFD

		ParseIFD(nef_, offset);

		//if (uint32 exif_offset= ParseIFD(nef_, offset))
		//{
		//	ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF

		//	// for NEF IFD offset is seems to be 0
		//	uint32 ifd_offset= 0;
		//	//ScanExif(filename, ifs, ifd_offset, *this, exif, false);

		//	break;
		//}
	}

	if (width_ == 0 || height_ == 0 || data_offset_ == 0 || filters_ == 0)
		return IS_FMT_NOT_SUPPORTED;

	if (compression_ == 34713 && !is_compressed())
	{
		// go figure... a bit of correction for Capture NX treated raws
		width_ += 3;
		raw_width_ = width_ + 3;
	}

	return IS_OK;
}


#define FC(row, col) \
	(filters_ >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define BAYER(row, col) \
	image_[((row) >> shrink_) * iwidth_ + ((col) >> shrink_)][FC(row, col)]


/*
   Construct a decode tree according the specification in *source.
   The first 16 bytes specify how many codes should be 1-bit, 2-bit
   3-bit, etc.  Bytes after that are the leaf values.

   For example, if the source is

    { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
      0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

   then the code is

	00		0x04
	010		0x03
	011		0x05
	100		0x06
	101		0x02
	1100		0x07
	1101		0x01
	11100		0x08
	11101		0x09
	11110		0x00
	111110		0x0a
	1111110		0x0b
	1111111		0xff
 */
void NEFReader::make_decoder(const uint8* source, int level)
{
	if (level == 0)
		leaf_ = 0;

	decode* cur= free_decode_++;

	if (free_decode_ > first_decode_+2048) {
		//fprintf (stderr, "%s: decoder table overflow\n", ifname);
		//longjmp (failure, 2);
		throw 1;
	}

	int i= 0, next= 0;

	while (i <= leaf_ && next < 16)
		i += source[next++];

	if (i > leaf_)
	{
		if (level < next)
		{
			cur->branch[0] = free_decode_;
			make_decoder(source, level + 1);
			cur->branch[1] = free_decode_;
			make_decoder(source, level + 1);
		}
		else
			cur->leaf = source[16 + leaf_++];
	}
//	return (uchar *) source + 16 + leaf;
}

/*
   Figure out if a NEF file is compressed.  These fancy heuristics
   are only needed for the D100, thanks to a bug in some cameras
   that tags all images as "compressed".
 */
bool NEFReader::is_compressed()
{
	if (compression_ != 34713)
		return false;

	if (model_ != "NIKON D100")
		return true;

	nef_.RPosition(data_offset_, FileStream::beg);

	const int TEST= 256;
	uint8 test[TEST];
	nef_.Read(test, TEST);

	for (int i= 15; i < TEST; i += 16)
		if (test[i])
			return true;

	return false;
}


// getbits(-1) initializes the buffer
// getbits(n) where 0 <= n <= 25 returns an n-bit integer

uint32 NEFReader::getbits(int nbits)
{
//	static uint32 bitbuf= 0;
//	static int vbits=0;
//	unsigned c, ret;

	uint32 ret= 0;
	const int LONG_BIT= 8 * sizeof (uint32);

	if (nbits == 0) return 0;
	if (nbits == -1)
		ret = bitbuf_ = vbits_ = 0;
	else
	{
		ret = bitbuf_ << (LONG_BIT - vbits_) >> (LONG_BIT - nbits);
		vbits_ -= nbits;
	}

	while (vbits_ < LONG_BIT - 7)
	{
		if (nef_.RPosition() < nef_.GetLength32())
		{
		uint32 c= nef_.GetUInt8(); // fgetc(ifp);
		bitbuf_ = (bitbuf_ << 8) + c;
//		if (c == 0xff && zero_after_ff)
//			fgetc(ifp);
		vbits_ += 8;
		}
		else	// EOF
		{
			bitbuf_ = (bitbuf_ << 8) + 0;
			vbits_ += 8;
		}
	}

	return ret;
}


int NEFReader::ljpeg_diff(decode* dindex)
{
	while (dindex->branch[0])
		dindex = dindex->branch[getbits(1)];

	int len= dindex->leaf;
	int diff= getbits(len);

	if ((diff & (1 << (len-1))) == 0)
		diff -= (1 << len) - 1;

	return diff;
}


void NEFReader::eight_bit_load_raw()
{
  //uchar *pixel;
  //unsigned row, col, val, lblack=0;

  //pixel = (uchar *) calloc (raw_width, sizeof *pixel);
  //merror (pixel, "eight_bit_load_raw()");
  //fseek (ifp, top_margin*raw_width, SEEK_CUR);
  //for (row=0; row < height; row++) {
  //  if (fread (pixel, 1, raw_width, ifp) < raw_width) derror();
  //  for (col=0; col < raw_width; col++) {
  //    val = curve[pixel[col]];
  //    if ((unsigned) (col-left_margin) < width)
  //      BAYER(row,col-left_margin) = val;
  //    else lblack += val;
  //  }
  //}
  //free (pixel);
  //if (raw_width > width+1)
  //  black = lblack / ((raw_width - width) * height);
  //if (!strncmp(model,"DC2",3))
  //  black = 0;
  //maximum = curve[0xff];
}


int NEFReader::LoadRaw()
{
	int iiiiiii= tiff_samples_;
//	int irow, row, col, i;
	int maximum= 0;

	nef_.RPosition(data_offset_, FileStream::beg);
	getbits(-1);

	for (int irow= 0; irow < height_; irow++)
	{
		int row = irow;
/*		if (!model_.empty() && model_[0] == 'E')
		{
			row = irow * 2 % height + irow / (height/2);
			if (row == 1 && atoi(model+1) < 5000)
			{
				fseek(ifp, 0, SEEK_END);
				fseek(ifp, ftell(ifp)/2, SEEK_SET);
				getbits(-1);
			}
		} */
		for (int col= 0; col < raw_width_; col++)
		{
			int i= getbits(12);

			if ((unsigned) (col - left_margin_) < width_)
				BAYER(row, col - left_margin_) = i;

			if (i > maximum)
				maximum = i;

			if (compression_ > 32768 && (col % 10) == 9)
				getbits(8);
		}
	}

	return maximum;
}


int NEFReader::LoadCompressedRaw()
{
	static const uint8 nikon_tree[]=
	{
		0,1,5,1,1,1,1,1,1,2,0,0,0,0,0,0,
		5,4,3,6,2,7,1,0,8,9,11,10,12
	};
//	int csize, row, col, i, diff;
//	ushort vpred[4], hpred[2], *curve;

//	init_decoder();
	second_decode_ = free_decode_ = first_decode_;
	make_decoder(nikon_tree, 0);

	if (nikon_curve_offset_ == 0)
		throw 1;

	nef_.RPosition(nikon_curve_offset_, FileStream::beg);

	uint16 vpred[4]= { nef_.GetUInt16(), nef_.GetUInt16(), nef_.GetUInt16(), nef_.GetUInt16() };
//	read_shorts (vpred, 4);
	uint32 csize= nef_.GetUInt16();
//	csize = get2();
	std::vector<uint16> curve(csize);
//	curve = calloc (csize, sizeof *curve);
//	merror (curve, "nikon_compressed_load_raw()");
//	read_shorts (curve, csize);
	for (size_t i= 0; i < csize; ++i)
		nef_ >> curve[i];

	nef_.RPosition(data_offset_, FileStream::beg);
//	fseek (ifp, data_offset, SEEK_SET);
	getbits(-1);

	uint16 hpred[2]= { 0, 0 };

	for (int row= 0; row < height_; ++row)
		for (int col= 0; col < raw_width_; ++col)
		{
			int diff= ljpeg_diff(first_decode_);
			if (col < 2)
			{
				int i= 2 * (row & 1) + (col & 1);
				vpred[i] += diff;
				hpred[col] = vpred[i];
			}
			else
				hpred[col & 1] += diff;

			if (size_t(col - left_margin_) >= width_)
				continue;

			diff = hpred[col & 1];
			if (diff >= csize)
				diff = csize - 1;

			BAYER(row, col - left_margin_) = curve[diff];
		}

	return curve[csize - 1];
}


#define FORC4 for (int c=0; c < colors_; c++)

/* This algorithm is officially called:

   "Interpolation using a Threshold-based variable number of gradients"

   described in http://www-ise.stanford.edu/~tingchen/algodep/vargra.html

   I've extended the basic idea to work with non-Bayer filter arrays.
   Gradients are numbered clockwise from NW=0 to W=7. */

void NEFReader::vng_interpolate(bool quick_interpolate)
{
	static const signed char terms[]=
	{
		-2,-2,+0,-1,0,0x01, -2,-2,+0,+0,1,0x01, -2,-1,-1,+0,0,0x01,
		-2,-1,+0,-1,0,0x02, -2,-1,+0,+0,0,0x03, -2,-1,+0,+1,1,0x01,
		-2,+0,+0,-1,0,0x06, -2,+0,+0,+0,1,0x02, -2,+0,+0,+1,0,0x03,
		-2,+1,-1,+0,0,0x04, -2,+1,+0,-1,1,0x04, -2,+1,+0,+0,0,0x06,
		-2,+1,+0,+1,0,0x02, -2,+2,+0,+0,1,0x04, -2,+2,+0,+1,0,0x04,
		-1,-2,-1,+0,0,int8(0x80), -1,-2,+0,-1,0,0x01, -1,-2,+1,-1,0,0x01,
		-1,-2,+1,+0,1,0x01, -1,-1,-1,+1,0,int8(0x88), -1,-1,+1,-2,0,0x40,
		-1,-1,+1,-1,0,0x22, -1,-1,+1,+0,0,0x33, -1,-1,+1,+1,1,0x11,
		-1,+0,-1,+2,0,0x08, -1,+0,+0,-1,0,0x44, -1,+0,+0,+1,0,0x11,
		-1,+0,+1,-2,1,0x40, -1,+0,+1,-1,0,0x66, -1,+0,+1,+0,1,0x22,
		-1,+0,+1,+1,0,0x33, -1,+0,+1,+2,1,0x10, -1,+1,+1,-1,1,0x44,
		-1,+1,+1,+0,0,0x66, -1,+1,+1,+1,0,0x22, -1,+1,+1,+2,0,0x10,
		-1,+2,+0,+1,0,0x04, -1,+2,+1,+0,1,0x04, -1,+2,+1,+1,0,0x04,
		+0,-2,+0,+0,1,int8(0x80), +0,-1,+0,+1,1,int8(0x88), +0,-1,+1,-2,0,0x40,
		+0,-1,+1,+0,0,0x11, +0,-1,+2,-2,0,0x40, +0,-1,+2,-1,0,0x20,
		+0,-1,+2,+0,0,0x30, +0,-1,+2,+1,1,0x10, +0,+0,+0,+2,1,0x08,
		+0,+0,+2,-2,1,0x40, +0,+0,+2,-1,0,0x60, +0,+0,+2,+0,1,0x20,
		+0,+0,+2,+1,0,0x30, +0,+0,+2,+2,1,0x10, +0,+1,+1,+0,0,0x44,
		+0,+1,+1,+2,0,0x10, +0,+1,+2,-1,1,0x40, +0,+1,+2,+0,0,0x60,
		+0,+1,+2,+1,0,0x20, +0,+1,+2,+2,0,0x10, +1,-2,+1,+0,0,int8(0x80),
		+1,-1,+1,+1,0,int8(0x88), +1,+0,+1,+2,0,0x08, +1,+0,+2,-1,0,0x40,
		+1,+0,+2,+1,0,0x10
	};

	static const signed char chood[]= { -1,-1, -1,0, -1,+1, 0,+1, +1,+1, +1,0, +1,-1, 0,-1 };

//	const signed char* cp;
//	uint16 (*brow[5])[4], *pix;
//	int code[8][2][320], *ip, gval[8], gmin, gmax, sum[4];
//	int row, col, shift, x, y, x1, x2, y1, y2, t, weight, grads, color, diag;
//	int g, diff, thold, num, c;

	int code[8][2][320];
	int sum[4];

	for (int row= 0; row < 8; row++)		/* Precalculate for bilinear */
	{
		for (int col= 1; col < 3; col++)
		{
			int* ip= code[row][col & 1];
			memset(sum, 0, sizeof sum);
			for (int y= -1; y <= 1; y++)
				for (int x= -1; x <= 1; x++)
				{
					int shift= (y==0) + (x==0);
					if (shift == 2) continue;
					int color = FC(row+y,col+x);
					*ip++ = (width_ * y + x) * 4 + color;
					*ip++ = shift;
					*ip++ = color;
					sum[color] += 1 << shift;
				}
				FORC4
					if (c != FC(row,col))
					{
						*ip++ = c;
						*ip++ = sum[c];
					}
		}
	}

	for (int row= 1; row < height_ - 1; row++)		/* Do bilinear interpolation */
	{
		for (int col= 1; col < width_ - 1; col++)
		{
			uint16* pix= image_[row * width_ + col];
			int* ip= code[row & 7][col & 1];
			memset(sum, 0, sizeof sum);
			for (int g= 8; g--; )
			{
				int diff= pix[*ip++];
				diff <<= *ip++;
				sum[*ip++] += diff;
			}
			for (int g= colors_; --g; )
			{
				int c= *ip++;
				pix[c] = sum[c] / *ip++;
			}
		}
	}

	if (quick_interpolate)
		return;
#if 0
	for (int row= 0; row < 8; row++)		/* Precalculate for VNG */
	{
		for (int col= 0; col < 2; col++)
		{
			ip = code[row][col];
			const signed char* cp= terms;
			for (int t= 0; t < 64; t++)
			{
				int y1 = *cp++;
				int x1 = *cp++;
				int y2 = *cp++;
				int x2 = *cp++;
				int weight = *cp++;
				int grads = *cp++;
				int color = FC(row+y1,col+x1);
				if (FC(row+y2,col+x2) != color) continue;
				int diag = (FC(row, col + 1) == color && FC(row + 1, col) == color) ? 2 : 1;
				if (abs(y1-y2) == diag && abs(x1-x2) == diag) continue;
				*ip++ = (y1 * width_ + x1) * 4 + color;
				*ip++ = (y2 * width_ + x2) * 4 + color;
				*ip++ = weight;
				for (int g= 0; g < 8; g++)
					if (grads & 1 << g)
						*ip++ = g;
				*ip++ = -1;
			}

			*ip++ = INT_MAX;

			cp = chood;
			for (int g= 0; g < 8; g++)
			{
				int y= *cp++;
				int x= *cp++;
				*ip++ = (y * width_ + x) * 4;
				int color = FC(row,col);
				if (FC(row+y, col+x) != color && FC(row+y*2, col+x*2) == color)
					*ip++ = (y * width_ + x) * 8 + color;
				else
					*ip++ = 0;
			}
		}
	}

	brow[4] = calloc (width_ * 3, sizeof **brow);
	merror (brow[4], "vng_interpolate()");

	for (int row= 0; row < 3; row++)
		brow[row] = brow[4] + row * width_;

	for (int row= 2; row < height_ - 2; row++)		/* Do VNG interpolation */
	{
		for (int col= 2; col < width_ - 2; col++)
		{
			pix = image_[row * width_ + col];
			ip = code[row & 7][col & 1];
			memset (gval, 0, sizeof gval);
			int g= 0;
			while ((g = ip[0]) != INT_MAX)		/* Calculate gradients */
			{
				int diff= pix[g] - pix[ip[1]];
				num = diff >> 31;
				gval[ip[3]] += (diff = ((diff ^ num) - num) << ip[2]);
				ip += 5;
				if ((g = ip[-1]) == -1) continue;
				gval[g] += diff;
				while ((g = *ip++) != -1)
					gval[g] += diff;
			}
			ip++;
			gmin = gmax = gval[0];			/* Choose a threshold */
			for (int g= 1; g < 8; g++)
			{
				if (gmin > gval[g]) gmin = gval[g];
				if (gmax < gval[g]) gmax = gval[g];
			}
			if (gmax == 0)
			{
				memcpy (brow[2][col], pix, sizeof *image);
				continue;
			}
			thold = gmin + (gmax >> 1);
			memset(sum, 0, sizeof sum);
			color = FC(row,col);
			for (int num= 0, g= 0; g < 8; g++, ip+=2)		/* Average the neighbors */
			{
				if (gval[g] <= thold)
				{
					FORC4
						if (c == color && ip[1])
							sum[c] += (pix[c] + pix[ip[1]]) >> 1;
						else
							sum[c] += pix[ip[0] + c];
					num++;
				}
			}
			FORC4					/* Save to buffer */
			{
				t = pix[color];
				if (c != color)
				{
					t += (sum[c] - sum[color]) / num;
					if (t < 0) t = 0;
					if (t > clip_max) t = clip_max;
				}
				brow[2][col][c] = t;
			}
		}

		if (row > 3)				/* Write buffer to image */
			memcpy(image[(row-2)*width+2], brow[0]+2, (width-4)*sizeof *image);

		for (int g=0; g < 4; g++)
			brow[(g-1) & 3] = brow[g];
	}

	memcpy(image[(row-2)*width+2], brow[0]+2, (width-4)*sizeof *image);
	memcpy(image[(row-1)*width+2], brow[1]+2, (width-4)*sizeof *image);
	free(brow[4]);
#endif
}


ImageStat NEFReader::ReadNextLine(Dib& dib, const uint8* gamma_table)
{
	image_buf_.resize(iheight_ * iwidth_ * sizeof *image_ / sizeof image_[0][0]);

	image_ = reinterpret_cast<Quad*>(&image_buf_.front());

	if (is_compressed())
		maximum_ = LoadCompressedRaw();
	else
		maximum_ = LoadRaw();

	// scale colors to 8-bit color applying contrast curve (or gamma curve)

	ScaleColors(0, maximum_, iwidth_, iheight_);

	// interpolation

	if (shrink_ == 0)
		vng_interpolate(true);

	// convert to RGB

	ConvertToRGB(maximum_, iwidth_, iheight_);

	dib.Create(iwidth_, iheight_, 24);

	if (contrast_curve_.empty())
	{
		contrast_curve_.resize(maximum_ + 1);
		for (size_t i= 0; i <= maximum_; ++i)
		{
			float x= static_cast<float>(i);
			x /= maximum_;
			contrast_curve_[i] = static_cast<uint8>(0xff * pow(x, 0.45f));
		}
	}

	const size_t len= contrast_curve_.size();
	bool apply_curve= true;
	for (uint32 y= 0; y < iheight_ && apply_curve; ++y)
	{
		BYTE* line= dib.LineBuffer(y);
		uint32 offset= y * iwidth_;

		for (uint32 x= 0; x < iwidth_; ++x)
		{
			if (image_[offset + x][2] >= len ||
				image_[offset + x][1] >= len ||
				image_[offset + x][0] >= len)
			{
				apply_curve = false;
				break;
			}
		}
	}

	for (uint32 y= 0; y < iheight_; ++y)
	{
		BYTE* line= dib.LineBuffer(y);
		uint32 offset= y * iwidth_;

		if (apply_curve)
			for (uint32 x= 0; x < iwidth_; ++x)
			{
				*line++ = contrast_curve_[image_[offset + x][2]];
				*line++ = contrast_curve_[image_[offset + x][1]];
				*line++ = contrast_curve_[image_[offset + x][0]];
			}
		else
			for (uint32 x= 0; x < iwidth_; ++x)
			{
				*line++ = image_[offset + x][2];
				*line++ = image_[offset + x][1];
				*line++ = image_[offset + x][0];
			}
	}

	return IS_OK;
}


void NEFReader::dng_coeff(double cc[4][4], double cm[4][3], double xyz[3])
{
	static const double rgb_xyz[3][3]=		/* RGB from XYZ */
	{
		{  3.240479, -1.537150, -0.498535 },
		{ -0.969256,  1.875992,  0.041556 },
		{  0.055648, -0.204043,  1.057311 }
	};
#if 0
	static const double xyz_rgb[3][3] = {		/* XYZ from RGB */
		{ 0.412453, 0.357580, 0.180423 },
		{ 0.212671, 0.715160, 0.072169 },
		{ 0.019334, 0.119193, 0.950227 } };
#endif
	double cam_xyz[4][3], xyz_cam[3][4], invert[3][6];

	memset(cam_xyz, 0, sizeof cam_xyz);
	for (int i= 0; i < colors_; i++)
		for (int j= 0; j < 3; j++)
			for (int k= 0; k < colors_; k++)
				cam_xyz[i][j] += cc[i][k] * cm[k][j] * xyz[j];

	for (int i= 0; i < colors_; i++)
	{
		double num= 0.0;
		for (int j= 0; j < 3; j++)
			num += cam_xyz[i][j];
		for (int j=0; j < 3; j++)
			cam_xyz[i][j] /= num;
		pre_mul_[i] = static_cast<float>(1.0 / num);
	}

	for (int i= 0; i < 3; i++)
	{
		for (int j= 0; j < 6; j++)
			invert[i][j] = j == i+3;
		for (int j= 0; j < 3; j++)
			for (int k= 0; k < colors_; k++)
				invert[i][j] += cam_xyz[k][i] * cam_xyz[k][j];
	}
	for (int i= 0; i < 3; i++)
	{
		double num= invert[i][i];
		for (int j= 0; j < 6; j++)		/* Normalize row i */
			invert[i][j] /= num;
		for (int k= 0; k < 3; k++)		/* Subtract it from other rows */
		{
			if (k == i)
				continue;
			num = invert[k][i];
			for (int j= 0; j < 6; j++)
				invert[k][j] -= invert[i][j] * num;
		}
	}
	memset(xyz_cam, 0, sizeof xyz_cam);
	for (int i= 0; i < 3; i++)
		for (int j= 0; j < colors_; j++)
			for (int k= 0; k < 3; k++)
				xyz_cam[i][j] += invert[i][k+3] * cam_xyz[j][k];

	memset(coeff_, 0, sizeof coeff_);
	for (int i= 0; i < 3; i++)
		for (int j= 0; j < colors_; j++)
			for (int k=0; k < 3; k++)
				coeff_[i][j] += static_cast<float>(rgb_xyz[i][k] * xyz_cam[k][j]);

	double num= 0.0;
	for (int j= 0; j < colors_; j++)
		num += coeff_[1][j];
	for (int i= 0; i < 3; i++)
		for (int j= 0; j < colors_; j++)
			coeff_[i][j] /= static_cast<float>(num);

	use_coeff_ = true;
}


//   Thanks to Adobe for providing these excellent CAM -> XYZ matrices!
void NEFReader::adobe_coeff()
{
	static const struct
	{
		const char* prefix;
		short trans[12];
	} table[]=
	{
		{ "NIKON D100",	{ 5915,-949,-778,-7516,15364,2282,-1228,1337,6404 } },
		{ "NIKON D1H",	{ 7577,-2166,-926,-7454,15592,1934,-2377,2808,8606 } },
		{ "NIKON D1X",	{ 7620,-2173,-966,-7604,15843,1805,-2356,2811,8439 } },
		{ "NIKON D1",	{ 7559,-2130,-965,-7611,15713,1972,-2478,3042,8290 } },
		{ "NIKON D2H",	{ 5710,-901,-615,-8594,16617,2024,-2975,4120,6830 } },
		{ "NIKON D70",	{ 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
		{ "NIKON E995",	// copied from E5000
		{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "NIKON E2500",	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "NIKON E4500",	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "NIKON E5000",	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
		{ "NIKON E5400",	{ 9349,-2987,-1001,-7919,15766,2266,-2098,2680,6839 } },
		{ "NIKON E5700",	{ -5368,11478,2368,5537,-113,3148,-4969,10021,5782,778,9028,211 } },
		{ "NIKON E8400",	{ 7842,-2320,-992,-8154,15718,2599,-1098,1342,7560 } },
		{ "NIKON E8700",	{ 8489,-2583,-1036,-8051,15583,2643,-1307,1407,7354 } },
		{ "NIKON E8800",	{ 7971,-2314,-913,-8451,15762,2894,-1442,1520,7610 } },
	};

	double cc[4][4], cm[4][3], xyz[] = { 1,1,1 };

	for (int i= 0; i < 4; i++)
		for (int j= 0; j < 4; j++)
			cc[i][j] = i == j;

	//string name= make_ + " " + model_;

	const int size= sizeof table / sizeof table[0];

	for (int i= 0; i < size; i++)
		if (model_ == table[i].prefix)
		{
			for (int j= 0; j < 12; j++)
				cm[0][j] = table[i].trans[j];
			dng_coeff(cc, cm, xyz);
			break;
		}
}


ImageStat NEFReader::PrepareReading(int reduction_factor)
{
	half_size_ = reduction_factor > 1;
	four_color_rgb_ = half_size_;

	if (!use_coeff_)
		adobe_coeff();

	if (four_color_rgb_ && filters_ && colors_ == 3)
	{
		for (int i= 0; i < 32; i += 4)
		{
			if ((filters_ >> i & 15) == 9)
				filters_ |= 2 << i;
			if ((filters_ >> i & 15) == 6)
				filters_ |= 8 << i;
		}

		colors_++;

		pre_mul_[3] = pre_mul_[1];

		if (use_coeff_)
			for (int i= 0; i < 3; ++i)
				coeff_[i][3] = coeff_[i][1] /= 2.0f;
	}

	shrink_ = half_size_ && filters_;
	iheight_ = (height_ + shrink_) >> shrink_;
	iwidth_  = (width_  + shrink_) >> shrink_;

//	scan_line_ = 0;
//	strip_index_ = 0;
//	buffer_ = TIFFLib::_TIFFmalloc(TIFFLib::TIFFStripSize(tiff_));

	return IS_OK;
}


void NEFReader::ScaleColors(int black, int maximum, uint32 width, uint32 height)
{
	int min[4], max[4], count[4];
	double sum[4]= { 0.0, 0.0, 0.0, 0.0 };
	uint16 white[8][8];

	memset(&white, 0, sizeof white);

	maximum -= black;
	if (use_auto_wb_ || (use_camera_wb_ && camera_red_ == -1))
	{
		FORC4 min[c] = INT_MAX;
		FORC4 max[c] = count[c] = 0;
		for (int row= 0; row < height; row++)
			for (int col= 0; col < width; col++)
				FORC4
				{
					int val = image_[row*width+col][c];
					if (!val) continue;
					if (min[c] > val) min[c] = val;
					if (max[c] < val) max[c] = val;
					val -= black;
					if (val > maximum-25) continue;
					if (val < 0) val = 0;
					sum[c] += val;
					count[c]++;
				}
				FORC4 pre_mul_[c] = static_cast<float>(count[c] / sum[c]);
	}

	if (use_camera_wb_ && camera_red_ != -1)
	{
		int val= 0;
		FORC4 count[c] = 0;
		for (int row= 0; row < 8; row++)
			for (int col= 0; col < 8; col++)
			{
				int c = FC(row,col);
				if ((val = white[row][col] - black) > 0)
					sum[c] += val;
				count[c]++;
			}

		val = 1;
		FORC4 if (sum[c] == 0) val = 0;
		if (val)
			FORC4 pre_mul_[c] = static_cast<float>(count[c] / sum[c]);
		else if (camera_red_ && camera_blue_)
		{
			pre_mul_[0] = static_cast<float>(camera_red_);
			pre_mul_[2] = static_cast<float>(camera_blue_);
			pre_mul_[1] = pre_mul_[3] = 1.0f;
		}
		else
		{ /*fprintf (stderr, "%s: Cannot use camera white balance.\n", ifname);*/ }
	}

	//if (!use_coeff_)
	//{
	//	pre_mul_[0] *= red_scale;
	//	pre_mul_[2] *= blue_scale;
	//}

	float dmin= FLT_MAX;
	FORC4 if (dmin > pre_mul_[c])
		dmin = pre_mul_[c];
	FORC4 pre_mul_[c] /= dmin;

	//  while (maximum << shift < 0x8000) shift++;
	//  FORC4 pre_mul[c] *= 1 << shift;
	//  maximum <<= shift;

	//if (write_fun != write_ppm || bright < 1) {
	//	maximum *= bright;
	//	if (maximum > 0xffff)
	//		maximum = 0xffff;
	//	FORC4 pre_mul[c] *= bright;
	//}
	//if (verbose) {
	//	fprintf (stderr, "Scaling with black=%d, pre_mul[] =", black);
	//	FORC4 fprintf (stderr, " %f", pre_mul[c]);
	//	fputc ('\n', stderr);
	//}
//	clip_max = clip_color ? maximum : 0xffff;
	uint32 clip_max = maximum;

	for (int row= 0; row < height; row++)
		for (int col= 0; col < width; col++)
			for (int c= 0; c < colors_; c++)
			{
				int val = image_[row*width+col][c];
				if (!val) continue;
				val -= black;
				val = static_cast<int>(val * pre_mul_[c]);
				if (val < 0) val = 0;
				if (val > clip_max) val = clip_max;
				image_[row*width+col][c] = val;
			}
}


void NEFReader::ConvertToRGB(int clip_max, uint32 width, uint32 height)
{
	int trim= 0;
	int c= 0;
//	int row, col, r, g, c=0;
//	ushort *img;
	float rgb[3];

//	if (document_mode)
//		colors = 1;

	memset(histogram_, 0, sizeof histogram_);

	for (int row= trim; row < height-trim; row++)
		for (int col= trim; col < width-trim; col++)
		{
			uint16* img= image_[row*width+col];
//			if (document_mode)
//				c = FC(row,col);
			if (colors_ == 4 && !use_coeff_)	/* Recombine the greens */
				img[1] = (img[1] + img[3]) >> 1;
			if (colors_ == 1)			/* RGB from grayscale */
				for (int r= 0; r < 3; r++)
					rgb[r] = img[c];
			else if (use_coeff_)
			{		/* RGB via coeff[][] */
				for (int r= 0; r < 3; r++)
				{
					rgb[r] = 0.0f;
					for (int g= 0; g < colors_; g++)
						rgb[r] += img[g] * coeff_[r][g];
				}
			}
			else				/* RGB from RGB (easy) */
				goto norgb;

			for (int r= 0; r < 3; r++)
			{
				if (rgb[r] < 0)        rgb[r] = 0.0;
				if (rgb[r] > clip_max) rgb[r] = static_cast<float>(clip_max);
				img[r] = static_cast<uint16>(rgb[r]);
			}
norgb:
			for (int r= 0; r < 3; r++)
				histogram_[r][img[r] >> 3]++;
		}
}


void NEFReader::Close()
{
	nef_.Close();
}
