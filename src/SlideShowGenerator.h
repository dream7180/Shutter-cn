/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SlideShowGenerator.h: interface for the CSlideShowGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SLIDESHOWGENERATOR_H__EE4DD161_255B_4A1E_99C9_FB2923D87DAF__INCLUDED_)
#define AFX_SLIDESHOWGENERATOR_H__EE4DD161_255B_4A1E_99C9_FB2923D87DAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSlideShowGenerator
{
public:
	CSlideShowGenerator();
	virtual ~CSlideShowGenerator();

	// write template app to create new slide show
	bool WriteSlideShow(const TCHAR* filename, int delay, bool full_screen, bool repeatedLoop);

	struct SlideData
	{
		SlideData() : reserved1(0), reserved2(0), reserved3(0), reserved4(0)
		{}
		DWORD reserved1;
		DWORD reserved2;
		DWORD reserved3;
		DWORD reserved4;
	};

	// append JPEG photo to the slide show
	bool AddJpegSlide(const BYTE* jpeg_data, int jpeg_size, const SlideData* data= 0);

	// align data section and close file
	bool Finish();

	void Delete();

	// get path to the generated slide show app
	CString GetSlideShowApp() const		{ return path_; }

	// get slide show app's size in bytes
	DWORD GetSlideShowSize() const		{ return file_size_; }

	// start slide show app
	void LaunchSlideShow();

private:
	CFile file_;	// output file
	DWORD slide_show_section_;
	CString path_;	// ready slide show app
	DWORD file_size_;

	void Close();
	void CreateDataSection(int delay, bool full_screen, bool repeatedLoop);
	bool AlignDataSection();
};

#endif // !defined(AFX_SLIDESHOWGENERATOR_H__EE4DD161_255B_4A1E_99C9_FB2923D87DAF__INCLUDED_)
