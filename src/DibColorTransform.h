/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/



extern AutoPtr<Dib> DibColorTransform(const Dib& dib, ColorProfilePtr input_profile, ColorProfilePtr display_profile,
									   int rendering_intent, CRect* prectPart= 0);


extern void DrawBitmap(const Dib& dib, CDC* dc, const CRect& dest_rect, CRect* prectSrc, DibDispMethod method,
				ColorProfilePtr input_profile, ColorProfilePtr display_profile, int rendering_intent, CRect* prectPart= 0);

