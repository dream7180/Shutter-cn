/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Path.h"
#include "PhotoInfo.h"
#include "Transform.h"
#include "PhotoAttrAccess.h"
#include "DbOperations.h"
#include "FileStatus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int transform_jpeg(int transform, bool mirror, int x, int y, int width, int height,
						  const TCHAR* input_file, const TCHAR* output_file, int* out_width, int* out_height);

extern int FixRotatedPhoto(int& transf, PhotoInfo& img, const TCHAR* output_file);


int Transform(int transform, bool mirror, int width, int height, Path input_file, CWnd* parent,
			  int* out_width, int* out_height, uint16* thumbnail_orientation, PhotoInfoPtr photo)
{
	try
	{
		const int SIZE= MAX_PATH + 2;
		CString output;
		::GetTempFileName(input_file.GetDir().c_str(), _T("jpg_"), 0, output.GetBuffer(SIZE));
		output.ReleaseBuffer();
		bool fixed_rotation= false;

		if (transform & FIX_ROTATED_THUMBNAIL)
		{
			if (photo)
			{
				if (int err= FixRotatedPhoto(transform, *photo, output))
				{
					::DeleteFile(output);
					return err;
				}
				fixed_rotation = true;
			}
		}
		else if (int err= transform_jpeg(transform, mirror, 0, 0, width, height, input_file.c_str(), output, out_width, out_height))
		{
			::DeleteFile(output);
			return err;
		}

		FileStatus fileStatus;
		GetFileStatus(input_file.c_str(), fileStatus);

		{	// now add PhotoAttrib structure to store orientation (used for thumbnail rotation)

			PhotoAttrAccess access(output, parent);
			if (!access.Open(true) || access.GetAttribPtr() == 0)
			{
				access.Close();
				ASSERT(false);
				::DeleteFile(output);
				return -5;
			}

			int rotation= 0;
			if (transform == ROTATE_90_DEG_CW || transform == CONDITIONAL_ROTATE_90_DEG_CW)
				rotation = 1;
			else if (transform == ROTATE_90_DEG_COUNTERCW || transform == CONDITIONAL_ROTATE_90_DEG_COUNTERCW)
				rotation = -1;
			else if (transform == ROTATE_180_DEG)
				rotation = 2;

			if (fixed_rotation)
				access.GetAttribPtr()->ResetThumbnailOrientation(rotation, mirror);
			else
				access.GetAttribPtr()->SetThumbnailOrientation2(rotation, mirror);

			if (thumbnail_orientation)
				*thumbnail_orientation = access.GetAttribPtr()->orientation_info_;

			// closing photo access here (out of scope)
		}

		// restoring original file status including time stamp
		SetFileStatus(output, fileStatus);

		if (::DeleteFile(input_file.c_str()) == 0)
		{
			::DeleteFile(output);
			return -2;
		}

		// just in case file size has not been changed, invalidate cache record
		MarkRecordAsStale(input_file.c_str());

		return ::MoveFile(output, input_file.c_str()) != 0 ? 0 : -3;
	}
	catch (CException* p)
	{
		//TODO: log
		p->Delete();
	}
	catch (...) //j_common_ptr p)
	{
		//TODO: log
	}
	return -4;
}


int RotatePhoto(PhotoInfo& photo, RotationTransformation transform, bool mirror, CWnd* parent)
{
	int rot_flag= photo.rotation_flag_ & PhotoInfo::RF_MASK;

	int rotation_result= 0;

	RotationTransformation operation= transform;

	if (transform == AUTO_ROTATE)	// auto-rotation?
	{
		// determine necessary rotation
		if (photo.GetOrientation() == PhotoInfo::ORIENT_90CW)
			operation = CONDITIONAL_ROTATE_90_DEG_COUNTERCW;
		else if (photo.GetOrientation() == PhotoInfo::ORIENT_90CCW)
			operation = CONDITIONAL_ROTATE_90_DEG_CW;
	}
	else if (rot_flag != PhotoInfo::RF_INTACT)
	{
		// take non-permanent rotation into account
		if (transform == ROTATE_90_DEG_CW)
		{
			if (rot_flag == PhotoInfo::RF_90CW)
				operation = ROTATE_180_DEG;
			else if (rot_flag == PhotoInfo::RF_90CCW)
				operation = ROTATE_NONE; // do nothing
			else if (rot_flag == PhotoInfo::RF_UPDN)
				operation = ROTATE_90_DEG_COUNTERCW;
		}
		else if (transform == ROTATE_90_DEG_COUNTERCW)
		{
			if (rot_flag == PhotoInfo::RF_90CW)
				operation = ROTATE_NONE; // do nothing
			else if (rot_flag == PhotoInfo::RF_90CCW)
				operation = ROTATE_180_DEG;
			else if (rot_flag == PhotoInfo::RF_UPDN)
				operation = ROTATE_90_DEG_CW;
		}
		else if (transform == ROTATE_180_DEG)
		{
			if (rot_flag == PhotoInfo::RF_90CW)
				operation = ROTATE_90_DEG_CW;
			else if (rot_flag == PhotoInfo::RF_90CCW)
				operation = ROTATE_90_DEG_COUNTERCW;
			else if (rot_flag == PhotoInfo::RF_UPDN)
				operation = ROTATE_NONE; // do nothing
		}
	}

	if (operation != AUTO_ROTATE)	// any rotation necessary?
	{
		int new_width= 0;
		int new_height= 0;
		uint16 thumb_orient= photo.ThumbnailOrientation();
		if (int result= Transform(operation, mirror, photo.GetWidth(), photo.GetHeight(),
			photo.GetOriginalPath().c_str(), parent, &new_width, &new_height, &thumb_orient, &photo))
		{
			if (result != 9999)	// 9999 means file was skipped
				rotation_result = -1;	// error
			else
				rotation_result = 0;	// skip
		}
		else	// file rotated
		{
			rotation_result = 1; // ok
			photo.SetSize(new_width, new_height);
			photo.SetOrientation(1);	// 'normal' orientation
			photo.SetThumbnailOrientation(thumb_orient);
			// reset non-permanent rotation back to zero
			photo.rotation_flag_ = PhotoInfo::RF_INTACT;

			//orient it...
			if ((transform & FIX_ROTATED_THUMBNAIL) == 0)
			{
				// this is all bogus, it's just needed to *get rid* of thumbnail from the cache

				if (operation != ROTATE_180_DEG)
					photo.RotateThumbnail(operation == CONDITIONAL_ROTATE_90_DEG_CW || operation == ROTATE_90_DEG_CW, mirror);
				else
				{
					photo.RotateThumbnail(true, mirror);
					photo.RotateThumbnail(true, false);
				}
			}
		}
	}

	return rotation_result;
}


extern void JPEG_Crop_raw(Path input_file, Path output_file, CRect cropped_img_rect)
{
	int out_width, out_height;

	if (int err= transform_jpeg(JPEG_LOSSLESS_CROP, false,
		cropped_img_rect.left, cropped_img_rect.top, cropped_img_rect.Width(), cropped_img_rect.Height(),
		input_file.c_str(), output_file.c_str(), &out_width, &out_height))
	{
		::DeleteFile(output_file.c_str());

		switch (err)
		{
		case -1:
			throw String(L"Error reading input file.");
		case -2:
			throw String(L"Error writing output file.");
		default:
			ASSERT(false);
			throw String(L"Lossless cropping error.");
		}
	}
}


extern int JPEG_Crop(Path input_file, Path output_file, CRect cropped_img_rect)
{
	try
	{
		int out_width, out_height;

		if (int err= transform_jpeg(JPEG_LOSSLESS_CROP, false,
			cropped_img_rect.left, cropped_img_rect.top, cropped_img_rect.Width(), cropped_img_rect.Height(),
			input_file.c_str(), output_file.c_str(), &out_width, &out_height))
		{
			::DeleteFile(output_file.c_str());
			return err;
		}

		return 0;	// OK
	}
	catch (CException* p)
	{
		//TODO: log
		p->Delete();
	}
	catch (...)
	{
		//TODO: log
	}
	return -4;
}
