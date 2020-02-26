/*
 * jpegtran.c
 *
 * Copyright (C) 1995-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for JPEG transcoding.
 * It is very similar to cjpeg.c, but provides lossless transcoding between
 * different JPEG file formats.  It also provides some lossless and sort-of-
 * lossless transformations of JPEG data.
 */

// 2000.07.15 modified by M. Kowalski

#include "stdafx.h"
#include "PhotoInfo.h"
#include "Transform.h"
#include "JPEGException.h"
extern "C" {
#ifdef WIN64
	#include "libjpeg-turbo/cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
	#include "libjpeg-turbo/transupp.h"	/* Support routines for jpegtran */
	#include "libjpeg-turbo/jversion.h"	/* for version message */
#else
	#include "jpeg-mmx/cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
	#include "jpeg-mmx/transupp.h"		/* Support routines for jpegtran */
	#include "jpeg-mmx/jversion.h"		/* for version message */
#endif

#ifdef USE_CCOMMAND		/* command-line reader for Macintosh */
#ifdef __MWERKS__
#include <SIOUX.h>              /* Metrowerks needs this */
#include <console.h>		/* ... and this */
#endif
#ifdef THINK_C
#include <console.h>		/* Think declares it here */
#endif
#endif
}

boolean keymatch(char * arg, const char * keyword, int minchars) { return false; }

/*
 * Argument-parsing code.
 * The switch parser is designed to be useful with DOS-style command line
 * syntax, ie, intermixed switches and file names, where only the switches
 * to the left of a given file name affect processing of that file.
 * The main program in this file doesn't actually use this capability...
 */

static const JCOPY_OPTION copyoption= JCOPYOPT_ALL;	/* -copy switch */


LOCAL(void) select_transform(JXFORM_CODE transform, jpeg_transform_info& transformoption)
/* Silly little routine to detect multiple transform options, which we can't handle. */
{
	if (transformoption.transform == JXFORM_NONE || transformoption.transform == transform)
		transformoption.transform = transform;
	else
		throw String(L"Transformation combination is not supported.");
}


LOCAL(int) parse_switches(j_compress_ptr cinfo, int transform, bool mirror, int x, int y, int width, int height, jpeg_transform_info& transformoption)
/* Parse optional switches. */
{
	boolean simple_progressive;
	char * scansarg = NULL;	/* saves -scans parm if any */

	/* Set up default JPEG parameters. */
	simple_progressive = FALSE;
	transformoption.transform = JXFORM_NONE;
	transformoption.trim = FALSE;
	transformoption.force_grayscale = FALSE;
	cinfo->err->trace_level = 0;

	/* Scan command line options, adjust parameters */
	if (transform == JPEG_LOSSLESS_CROP)//JXFORM_CROP)
	{
#ifdef WIN64
		select_transform(JXFORM_NONE, transformoption);
		transformoption.crop = true;
		transformoption.crop_xoffset = x;
		transformoption.crop_yoffset = y;
		transformoption.crop_width = width;
		transformoption.crop_height = height;

		transformoption.crop_height_set = transformoption.crop_width_set = JCROP_POS;
		transformoption.crop_xoffset_set = transformoption.crop_yoffset_set = JCROP_POS;
#else
		select_transform(JXFORM_CROP, transformoption);
		transformoption.crop_x = x;
		transformoption.crop_y = y;
		transformoption.crop_width = width;
		transformoption.crop_height = height;
#endif
		ASSERT(!mirror);
	}
	else if (mirror)
	{
		if (transform == ROTATE_NONE)
			select_transform(JXFORM_FLIP_H, transformoption);
		else if (transform == ROTATE_180_DEG)
			select_transform(JXFORM_FLIP_V, transformoption);
		else if (transform == ROTATE_90_DEG_COUNTERCW || transform == CONDITIONAL_ROTATE_90_DEG_COUNTERCW)
			select_transform(JXFORM_TRANSPOSE, transformoption);
		else if (transform == ROTATE_90_DEG_CW || transform == CONDITIONAL_ROTATE_90_DEG_CW)
			select_transform(JXFORM_TRANSVERSE, transformoption);
		else
		{ ASSERT(false); }
	}
	else if (transform == ROTATE_180_DEG)
		select_transform(JXFORM_ROT_180, transformoption);
	else if (transform == ROTATE_NONE)
		select_transform(JXFORM_NONE, transformoption);
	else if (transform == ROTATE_90_DEG_COUNTERCW || transform == CONDITIONAL_ROTATE_90_DEG_COUNTERCW)
		select_transform(JXFORM_ROT_270, transformoption);
	else if (transform == ROTATE_90_DEG_CW || transform == CONDITIONAL_ROTATE_90_DEG_CW)
		select_transform(JXFORM_ROT_90, transformoption);
	else
	{ ASSERT(false); }

  return 0;
}


METHODDEF(void) jpg_error_fn(j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	jpeg_destroy(cinfo);

	String msg= L"JPEG processing error: ";
	msg += CString(buffer);
	throw msg;
}


JXFORM_CODE RotationCodeToJpegTransformation(RotationTransformation r)
{
	return JXFORM_NONE;
}

struct C_file
{
	C_file() : file_(0)
	{}

	~C_file()
	{
		if (file_)
			fclose(file_);
	}

	bool Open(const TCHAR* fname, const TCHAR* mode)
	{
		ASSERT(file_ == nullptr);
		file_ = _tfopen(fname, mode);
		return file_ != nullptr;
	}

	bool Close()
	{
		if (file_)
		{
			int err= fclose(file_);
			file_ = nullptr;
			return err == 0;
		}
		return false;
	}

	FILE* get() const	{ return file_; }

private:
	FILE* file_;
};

struct C_file_delete
{
	C_file_delete(const TCHAR* fname) : path_(fname), delete_(true)
	{}

	~C_file_delete()
	{
		if (delete_)
			::DeleteFile(path_.c_str());
	}

	void DoNotDelete()
	{
		delete_ = false;
	}

private:
	std::wstring path_;
	bool delete_;
};


extern int transform_jpeg(int transform, bool mirror, int x, int y, int width, int height,
						  const TCHAR* in_file, const TCHAR* out_file, int* out_width, int* out_height)
{
	C_file_delete delete_output(out_file);

	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct jpeg_error_mgr jsrcerr, jdsterr;
//#ifdef PROGRESS_REPORT
//  struct cdjpeg_progress_mgr progress;
//#endif
	jvirt_barray_ptr* src_coef_arrays= 0;
	jvirt_barray_ptr* dst_coef_arrays= 0;
	C_file input_file;
	C_file output_file;

	/* Initialize the JPEG decompression object with default error handling. */
	srcinfo.err = jpeg_std_error(&jsrcerr);
	//MiK
	srcinfo.err->error_exit = jpg_error_fn;
	jpeg_create_decompress(&srcinfo);
	/* Initialize the JPEG compression object with default error handling. */
	dstinfo.err = jpeg_std_error(&jdsterr);
	//MiK
	dstinfo.err->error_exit = jpg_error_fn;
	jpeg_create_compress(&dstinfo);

	/* Now safe to enable signal catcher.
	* Note: we assume only the decompression object will have virtual arrays.
	*/
#ifdef NEED_SIGNAL_CATCHER
	enable_signal_catcher((j_common_ptr) &srcinfo);
#endif

	/* Scan command line to find file names.
	* It is convenient to use just one switch-parsing routine, but the switch
	* values read here are mostly ignored; we will rescan the switches after
	* opening the input file.  Also note that most of the switches affect the
	* destination JPEG object, so we parse into that and then copy over what
	* needs to affects the source too.
	*/
	jpeg_transform_info transformoption;
	memset(&transformoption, 0, sizeof(transformoption));
	transformoption.transform = JXFORM_NONE;

	parse_switches(&dstinfo, transform, mirror, x, y, width, height, transformoption);
	jsrcerr.trace_level = jdsterr.trace_level;
	srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

	if (!input_file.Open(in_file, _T(READ_BINARY)))
		return -1;

	/* Open the output file. */
	if (!output_file.Open(out_file, _T(WRITE_BINARY)))
		return -2;

//#ifdef PROGRESS_REPORT
//  start_progress_monitor((j_common_ptr) &dstinfo, &progress);
//#endif

	/* Specify data source for decompression */
	jpeg_stdio_src(&srcinfo, input_file.get());

	/* Enable saving of extra markers that we want to copy */
	jcopy_markers_setup(&srcinfo, copyoption);

	/* Read file header */
	(void) jpeg_read_header(&srcinfo, TRUE);

	// check image dimensions to determine if rotation should be applied
	if (transform == CONDITIONAL_ROTATE_90_DEG_COUNTERCW || transform == CONDITIONAL_ROTATE_90_DEG_CW)
	{
		if (srcinfo.image_width != width || srcinfo.image_height != height)
		{
			jpeg_destroy(reinterpret_cast<j_common_ptr>(&srcinfo));
			return 9999;	// file skipped
		}
	}

	/* Any space needed by a transform option must be requested before
	* jpeg_read_coefficients so that memory allocation will be done right.
	*/
//#if TRANSFORMS_SUPPORTED
	jtransform_request_workspace(&srcinfo, &transformoption);
//#endif

	/* Read source file as DCT coefficients */
	src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	/* Initialize destination compression parameters from source values */
	jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

	/* Adjust destination parameters if required by transform options;
	* also find out which set of coefficient arrays will hold the output.
	*/
//#if TRANSFORMS_SUPPORTED
	dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);
//#else
 // dst_coef_arrays = src_coef_arrays;
//#endif
	*out_width = dstinfo.image_width;
	*out_height = dstinfo.image_height;

	/* Adjust default compression parameters by re-parsing the options */
	parse_switches(&dstinfo, transform, mirror, x, y, width, height, transformoption);

	/* Specify data destination for compression */
	jpeg_stdio_dest(&dstinfo, output_file.get());

	/* Start compressor (note no image data is actually written here) */
	jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

	/* Copy to the output file any extra markers that we want to preserve */
	jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

	/* Execute image transformation, if any */
//#if TRANSFORMS_SUPPORTED
	jtransform_execute_transformation(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);
//#endif

	/* Finish compression and release memory */
	jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void) jpeg_finish_decompress(&srcinfo);
	jpeg_destroy_decompress(&srcinfo);

  /* Close files, if we opened them */
	input_file.Close();
	if (!output_file.Close())
		return -1;		// err writing output

	delete_output.DoNotDelete();

//#ifdef PROGRESS_REPORT
//  end_progress_monitor((j_common_ptr) &dstinfo);
//#endif

	/* All done. */
//	return jsrcerr.num_warnings + jdsterr.num_warnings ? EXIT_WARNING : EXIT_SUCCESS;
	return 0;
}
