/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef _WIN64

// MMX version of alpha blending for 8-bit RGB images
//
// alpha			- 0..256 (lowest bit is ignored though)
// img1 & img2	- images to blend
// output			- blending result
// ...				- image sizes
// prefetch		- prefetch source bitmaps?
// non_temporal_writing_flag	- non-temporal writing of result bmp?

void MMX_AlphaBlendRGB(int alpha, BYTE* img1, BYTE* img2, BYTE* output, DWORD width, DWORD height, DWORD line_length, bool prefetch, bool non_temporal_writing_flag)
{
	if (alpha > 256)
		alpha = 256;
	else if (alpha < 0)
		alpha = 0;

	if (img1 == 0 || img2 == 0 || output == 0)
		return;

	if (width == 0 || height == 0 || line_length == 0)
		return;

	// check the padding bytes (sanity check only)
	{
		DWORD padding= line_length - width * 3;
		if (padding > 16)
		{
			ASSERT(false);
			return;
		}
	}

	// 4-byte alignment expected or performance will be hit
	ASSERT((line_length & 3) == 0);

	// compose the quadruple alpha value; half the alpha to limit it to 0..128 range
	__int64 n64Alpha= (alpha / 2) & 0xffff;
	n64Alpha |= (n64Alpha << 0x10) | (n64Alpha << 0x20) | (n64Alpha << 0x30);

	// each step goes through 8 bytes
	DWORD Q_words_per_line= line_length / 8;

	// if 1 then last pixels in a row have to be processed separately
	DWORD odd= (line_length / 4) & 1;

	// prefetch counter (cache line: 64 bytes)
	DWORD counter= line_length / 64;

	DWORD do_prefetch= prefetch;
	DWORD non_temporal_writing= non_temporal_writing_flag;

	__asm
	{
		// set height counter
		mov			ebx, height
		test		ebx, ebx
		jz			finished

		// do the source prefetch?
		mov			eax, do_prefetch
		test		eax, eax
		jz			end_prefetch

prefetch_line:

		// prefetch input data

		mov			edi, img1
		mov			esi, img2

		mov			ecx, counter
		test		ecx, ecx
		jz			end_prefetch

next_line:
		//prefetcht0	[edi]
		//prefetcht0	[esi]
		mov			eax, [edi]
		mov			eax, [esi]

		add			edi, 64
		add			esi, 64

		dec			ecx
		jnz			next_line

end_prefetch:

		// alpha blend pixels in the current row

		// load the image and output pointers into the registers

		mov			edi, img1
		mov			esi, img2
		mov			eax, output

		// load alpha value
		movq		mm7, n64Alpha

		// zero register (for unpacking)
		pxor		mm6, mm6

		// set width counter
		mov			ecx, Q_words_per_line
		test		ecx, ecx
		jz			test_odd

		// MMX register usage:

		//	mm0:	img1 pixels
		//	mm1:	img2 pixels
		//	mm2:	img1
		//	mm3:	img2
		//	mm4:	result pixels
		//	mm5:	ditto
		//	mm6:	zero for unpacking
		//	mm7:	alpha value

		// calculate: output = alpha * img1 + (128 - alpha) * img2
		// which is equivalent to:
		// output = alpha * (img1 - img2) / 128 + img2

		// note: alpha is limited to 0..128 due to the (img1 - img2) expression
		// spanning -255..255 range and pmullw overflowing 16 bits with 8 bit alpha

		mov			edx, non_temporal_writing
		test		edx, edx
		jz			do_blend2

		// ----- version with non-temporal writing -----------
do_blend:
		movd		mm0, [edi]			// load img1 pixels
		movd		mm1, [esi]			// load img2 pixels
		movd		mm2, 4[edi]			// load img1 pixels
		movd		mm3, 4[esi]			// load img2 pixels

		punpcklbw	mm0, mm6			// unpack pixels (low, bytes to words)
		punpcklbw	mm1, mm6			// unpack pixels
		punpcklbw	mm2, mm6			// unpack pixels
		punpcklbw	mm3, mm6			// unpack pixels

		movq		mm4, mm0			// delta
		movq		mm5, mm2			// delta

		psubw		mm4, mm1			// (img1 - img2)
		psubw		mm5, mm3			// (img1 - img2)

		// stalls here?
		pmullw		mm4, mm7			// alpha * delta
		add			edi, 8
		pmullw		mm5, mm7			// alpha * delta
		add			esi, 8

		psraw		mm4, 7				// divide by 128 (arithmetical)
		psraw		mm5, 7				// divide by 128 (arithmetical)

		paddw		mm4, mm1			// + img2
		paddw		mm5, mm3			// + img2

		packuswb	mm4, mm5			// pack words to bytes with unsigned saturation
										// (even though no overflow is ever arising in this algo)
		movntq		[eax], mm4			// write back the new value (non-temporal data; skipping cache)

		add			eax, 8				// next pixel

		dec			ecx
		jnz			do_blend			// loop
		jz			test_odd

		//-----------------------------------------------------------------------------------------
		// same version but with normal writing rather than non-temporal
do_blend2:
		movd		mm0, [edi]			// load img1 pixels
		movd		mm1, [esi]			// load img2 pixels
		movd		mm2, 4[edi]			// load img1 pixels
		movd		mm3, 4[esi]			// load img2 pixels

		punpcklbw	mm0, mm6			// unpack pixels (low, bytes to words)
		punpcklbw	mm1, mm6			// unpack pixels
		punpcklbw	mm2, mm6			// unpack pixels
		punpcklbw	mm3, mm6			// unpack pixels

		movq		mm4, mm0			// delta
		movq		mm5, mm2			// delta

		psubw		mm4, mm1			// (img1 - img2)
		psubw		mm5, mm3			// (img1 - img2)

		pmullw		mm4, mm7			// alpha * delta
		add			edi, 8
		pmullw		mm5, mm7			// alpha * delta
		add			esi, 8

		psraw		mm4, 7				// divide by 128 (arithmetical)
		psraw		mm5, 7				// divide by 128 (arithmetical)

		paddw		mm4, mm1			// + img2
		paddw		mm5, mm3			// + img2

		packuswb	mm4, mm5			// pack words to bytes with unsigned saturation
										// (even though no overflow is ever arising in this algo)
		movq		[eax], mm4			// write back the new value

		add			eax, 8				// next pixel

		dec			ecx
		jnz			do_blend2			// loop

test_odd:
		test		odd, 1
		jz			done

		// process last pixels in a scan line

		movd		mm0, [edi]			// load img1 pixels
		movd		mm1, [esi]			// load img2 pixels

		punpcklbw	mm0, mm6			// unpack pixels (low, bytes to words)
		punpcklbw	mm1, mm6			// unpack pixels

		movq		mm4, mm0			// delta
		psubw		mm4, mm1			// (img1 - img2)

		pmullw		mm4, mm7			// alpha * delta
		add			edi, 4
		psraw		mm4, 7				// divide by 128 (arithmetical)
		add			esi, 4

		paddw		mm4, mm1			// + img2
		packuswb	mm4, mm4			// pack words to bytes with unsigned saturation

		movd		[eax], mm4			// write back the new value
		add			eax, 4				// next pixel
done:

		// proceed to the next line

		mov			eax, line_length

		add			img1, eax
		add			img2, eax
		add			output, eax

		dec			ebx
		jnz			prefetch_line

finished:

		emms

		// serializing after storing non-temporal data
		sfence;
	}
}

#else

	// TODO: impl. alpha blending in x64

#endif
