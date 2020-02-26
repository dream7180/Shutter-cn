/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Dlg.h : header file
//

#pragma once

extern void AlphaBitBlt(const Dib& src, const CRect& src_rect, int src_alpha, Dib& dst, const CPoint& dst_pos);
extern void AlphaBitBlt(const Dib& src, const CRect& src_rect, Dib& dst, const CPoint& dst_pos);
extern void AlphaBitBlt(const Dib& src1, const Dib& src2, const CRect& src_rect, int src_alpha, Dib& dst, const CPoint& dst_pos);
extern void Blur(Dib& dib, Dib& res);
extern void BlurAlphaChnl(Dib& dib, int from, int width, Dib& res, const int MIDDLE);
extern void BlurAlphaChnl2(Dib& dib, int from, int width, Dib& res, const int MIDDLE);
extern void Bevel(Dib& img, int from, int width, Dib& res);
extern bool UnsharpMask(Dib& dib, Dib& sharp, int threshold, float amount);
