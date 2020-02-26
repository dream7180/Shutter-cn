/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _transform_h_
#define _transform_h_
#include "PhotoInfoPtr.h"

class Path;

extern int Transform(int transform, bool mirror, int width, int height, Path input_file, CWnd* parent,
					 int* out_width, int* out_height, uint16* thumbnail_orientation, PhotoInfoPtr photo);


enum RotationTransformation
{
	// unconditional rotation
	ROTATE_NONE= 0,
	ROTATE_90_DEG_COUNTERCW= 1,
	ROTATE_90_DEG_CW= 2,
	// conditional rotation: if photo is in portrait mode already no rotation will be applied
	CONDITIONAL_ROTATE_90_DEG_COUNTERCW= 3,
	CONDITIONAL_ROTATE_90_DEG_CW= 4,

	ROTATE_180_DEG= 6,		// = JXFORM_ROT_180
	JPEG_LOSSLESS_CROP= 8,

	// auto rotation: according to orientation field info
	AUTO_ROTATE= 0x7f,
	FIX_ROTATED_THUMBNAIL= 0x80,
};


namespace jpeg {

// copy from transupp.h in jpeglib

//enum JXFORM_CODE
//{
//	JXFORM_NONE,		/* no transformation */
//	JXFORM_FLIP_H,		/* horizontal flip */
//	JXFORM_FLIP_V,		/* vertical flip */
//	JXFORM_TRANSPOSE,	/* transpose across UL-to-LR axis */
//	JXFORM_TRANSVERSE,	/* transpose across UR-to-LL axis */
//	JXFORM_ROT_90,		/* 90-degree clockwise rotation */
//	JXFORM_ROT_180,		/* 180-degree rotation */
//	JXFORM_ROT_270		/* 270-degree clockwise (or 90 ccw) */
//};

}

//jpeg::JXFORM_CODE TransformCode(RotationTransformation rotation, bool horz_flip, bool vert_flip);


extern int RotatePhoto(PhotoInfo& photo, RotationTransformation transform, bool mirror, CWnd* parent);

// 'lossless' JPEG crop
extern int JPEG_Crop(Path inputFile, Path outputFile, CRect cropped_img_rect);
extern void JPEG_Crop_raw(Path inputFile, Path outputFile, CRect cropped_img_rect);

#endif // _transform_h_
