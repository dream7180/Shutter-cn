/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ColorProfile.h"
#include "Dib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void DibColorTransform(const Dib& dibIn, Dib& dibOut, ColorProfilePtr input_profile, ColorProfilePtr display_profile,
					   int rendering_intent, CRect* prectPart/*= 0*/)
{
	ASSERT(dibIn.GetBufferSize() == dibOut.GetBufferSize());

	if (input_profile && display_profile && dibIn.GetColorComponents() == 3)
	{
		ColorTransform trans;

		trans.Create(*input_profile, TYPE_BGR_8, *display_profile, TYPE_BGR_8, rendering_intent);

		if (prectPart == 0)
		{
			for (size_t line= 0; line < dibIn.GetHeight(); ++line)
			{
				const BYTE* buf= dibIn.LinePixelBuffer(line, 0);
				trans.Transform(buf, dibOut.LinePixelBuffer(line, 0), dibIn.GetWidth());
			}
		}
		else
		{
			// use part of the bitmap only
			for (int y= prectPart->top; y < prectPart->bottom; ++y)
			{
				// dib is upside-down internally, but decoding routine starts from
				// the top of a bitmap while decoding, so let's reverse 'y'
				int line= y;//dibIn.GetHeight() - y - 1;

				int x= prectPart->left;
				const BYTE* begin= dibIn.LinePixelBuffer(line, x);
				size_t size= dibIn.LinePixelBuffer(line, prectPart->right) - begin;
				size /= dibIn.GetColorComponents();

				trans.Transform(begin, dibOut.LinePixelBuffer(line, x), static_cast<unsigned int>(size));
			}
		}
	}
	else
		dibOut.Clone(dibIn);
}


AutoPtr<Dib> DibColorTransform(const Dib& dib, ColorProfilePtr input_profile, ColorProfilePtr display_profile,
								int rendering_intent, CRect* prectPart/*= 0*/)
{
	AutoPtr<Dib> dib_ptr= new Dib();

	dib_ptr->Create(dib.GetWidth(), dib.GetHeight(), dib.GetBitsPerPixel());

	DibColorTransform(dib, *dib_ptr, input_profile, display_profile, rendering_intent, prectPart);

	return dib_ptr;
}


void DrawBitmap(const Dib& dib, CDC* dc, const CRect& dest_rect, CRect* prectSrc, DibDispMethod method,
				ColorProfilePtr input_profile, ColorProfilePtr display_profile, int rendering_intent)
{
	if (input_profile && display_profile)
	{
		// correct colors
		try
		{
			AutoPtr<Dib> copy= DibColorTransform(dib, input_profile, display_profile, rendering_intent, 0);

			copy->DrawBitmap(dc, dest_rect, prectSrc, method);

			return;
		}
		catch (...)
		{
		}
	}

	// normal drawing

	dib.DrawBitmap(dc, dest_rect, prectSrc, method);
}
