/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SlideShowGenerator.cpp: implementation of the CSlideShowGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SlideShowGenerator.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSlideShowGenerator::CSlideShowGenerator()
 : slide_show_section_(0), file_size_(0)
{}

CSlideShowGenerator::~CSlideShowGenerator()
{
	Delete();
}


bool CSlideShowGenerator::WriteSlideShow(const TCHAR* filename, int delay, bool full_screen, bool repeatedLoop)
{
	if (file_.m_hFile != CFile::hFileNull)
	{
		ASSERT(false);
		return false;
	}

	CFileException ex;
	if (!file_.Open(filename, CFile::modeReadWrite | CFile::modeCreate, &ex))
	{
		ex.ReportError();
		return false;
	}

	// copy binary app to this file

	HINSTANCE inst= AfxGetResourceHandle();
	HRSRC resource= ::FindResource(inst, MAKEINTRESOURCE(IDR_SLIDESHOWAPP), _T("SlideShowApp"));
	HGLOBAL app= ::LoadResource(inst, resource);
	if (app == 0)
		return false;

	DWORD size= ::SizeofResource(inst, resource);
	void* app_ptr= ::LockResource(app);

	file_.Write(app_ptr, size);

	try
	{
		CreateDataSection(delay, full_screen, repeatedLoop);
	}
	catch (...)
	{
		return false;
	}

	return true;
}


void CSlideShowGenerator::CreateDataSection(int delay, bool full_screen, bool repeatedLoop)
{
	// file length
	DWORD size= static_cast<DWORD>(file_.SeekToEnd());

	// back to beginning
	file_.SeekToBegin();

	// read all executable file headers
	IMAGE_DOS_HEADER dh;
	file_.Read(&dh, sizeof dh);

	LONG NT_header_offset= dh.e_lfanew;
	file_.Seek(NT_header_offset, CFile::begin);

	IMAGE_NT_HEADERS nth;
	file_.Read(&nth, sizeof nth);

	// this is real offset in a SlideShow.app currently; from NT header to the first section
	int directory= NT_header_offset + 0xf8; // static_cast<DWORD>(file_.GetPosition());

	// skip all used section entries, go to last but one section
	//file_.Seek((nth.FileHeader.NumberOfSections - 1) * sizeof IMAGE_SECTION_HEADER, CFile::current);
	file_.Seek(directory + (nth.FileHeader.NumberOfSections - 1) * sizeof IMAGE_SECTION_HEADER, CFile::begin);

	// read last section info
	IMAGE_SECTION_HEADER ish;
	file_.Read(&ish, sizeof ish);

	// prepare new section header
	IMAGE_SECTION_HEADER shImgData;
	memcpy(shImgData.Name, "SlidShow", 8);	// name max 8 bytes long

	struct SlidesHeader
	{
		char magic[4];
		DWORD slide_count;
		DWORD delay_between_photos;
		DWORD start_full_screen;
		DWORD extra_data1;
		DWORD extra_data2;
	} header;

	// header of new section
	shImgData.Misc.VirtualSize = sizeof header;
	// its address (virtual)
	DWORD addr= (ish.VirtualAddress + ish.SizeOfRawData + 0xfff) & ~0xfff;
	shImgData.VirtualAddress = addr;
	shImgData.SizeOfRawData = shImgData.Misc.VirtualSize;
	// new section data at the end of file
	shImgData.PointerToRawData = size;
	shImgData.PointerToRelocations = 0;
	shImgData.PointerToLinenumbers = 0;
	shImgData.NumberOfRelocations = 0;
	shImgData.NumberOfLinenumbers = 0;
	// read-only initialized data
	shImgData.Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;// | IMAGE_SCN_MEM_DISCARDABLE;
	// add an entry at the end
	slide_show_section_ = directory + nth.FileHeader.NumberOfSections * sizeof IMAGE_SECTION_HEADER;
	file_.Seek(slide_show_section_, CFile::begin);
	file_.Write(&shImgData, sizeof shImgData);

	file_.Seek(NT_header_offset, CFile::begin);
	nth.FileHeader.NumberOfSections++;
	file_.Write(&nth, sizeof nth);

	// this header is to be written in newly added section data
	header.magic[0] = '\xfb';
	header.magic[1] = '\x05';
	header.magic[2] = '\xdc';
	header.magic[3] = '\x1d';

	header.slide_count = 0;
	header.delay_between_photos = delay;
	header.start_full_screen = full_screen;
	header.extra_data1 = repeatedLoop;	// TODO: slide show options
	header.extra_data2 = 0;

	file_.SeekToEnd();
	file_.Write(&header, sizeof header);
}


bool CSlideShowGenerator::AddJpegSlide(const BYTE* jpeg_data, int jpeg_size, const SlideData* data/*= 0*/)
{
	if (jpeg_data == 0 || jpeg_size == 0)
	{
		ASSERT(false);
		return false;
	}

	// append JPEG photo at the end of slide show app
	DWORD end= static_cast<DWORD>(file_.SeekToEnd());
	// record photo's size
	file_.Write(&jpeg_size, sizeof jpeg_size);
	// record slide data
	SlideData temp;
	if (data == 0)
		data = &temp;
	file_.Write(data, sizeof *data);
	// record photo
	file_.Write(jpeg_data, jpeg_size);

	// go to slide show section
	file_.Seek(slide_show_section_, CFile::begin);
	IMAGE_SECTION_HEADER ish;
	if (file_.Read(&ish, sizeof ish) != sizeof ish)
		return false;

	// modify section size
	DWORD written= static_cast<DWORD>(file_.SeekToEnd()) - end;
	ish.Misc.VirtualSize += written;
	ish.SizeOfRawData += written;

	file_.Seek(slide_show_section_, CFile::begin);
	file_.Write(&ish, sizeof ish);

	// go to the section data
	int offset= ish.PointerToRawData + sizeof DWORD;
	file_.Seek(offset, CFile::begin);

	DWORD count= 0;
	file_.Read(&count, sizeof count);

	// increase photos counter
	count++;

	file_.Seek(offset, CFile::begin);
	file_.Write(&count, sizeof count);

	return true;
}


void CSlideShowGenerator::Close()
{
	if (file_.m_hFile == CFile::hFileNull)
		return;

	file_.Close();
}


bool CSlideShowGenerator::Finish()
{
	if (file_.m_hFile == CFile::hFileNull)
		return false;

	try
	{
		AlignDataSection();

		CString path= file_.GetFilePath();

		file_size_ = static_cast<DWORD>(file_.GetLength());

		file_.Close();

		path_ = path;

		return true;
	}
	catch (CException* ex)
	{
		ex->ReportError();
	}
	catch (...)
	{
		AfxMessageBox(_T("创建幻灯片失败."), MB_OK | MB_ICONERROR);
	}

	return false;
}


void CSlideShowGenerator::Delete()
{
	if (file_.m_hFile == CFile::hFileNull)
		return;

	CString path= file_.GetFilePath();
	file_.Close();
	if (!path.IsEmpty())
		::DeleteFile(path);
}


bool CSlideShowGenerator::AlignDataSection()
{
	// go to SlidShow section
	file_.Seek(slide_show_section_, CFile::begin);
	IMAGE_SECTION_HEADER sh;
	if (file_.Read(&sh, sizeof sh) != sizeof sh)
		return false;

	// align data size to 4K boundaries
	if (int align= 0x1000 - (sh.SizeOfRawData & 0xfff))
	{
		BYTE empty[4 * 1024];
		memset(empty, 0, sizeof empty);
		file_.SeekToEnd();
		file_.Write(empty, align);

		sh.SizeOfRawData += align;
		sh.Misc.VirtualSize += align;
		file_.Seek(slide_show_section_, CFile::begin);
		file_.Write(&sh, sizeof sh);
	}

	int data_size= sh.SizeOfRawData;

	file_.SeekToBegin();
	IMAGE_DOS_HEADER dh;
	file_.Read(&dh, sizeof dh);

	int NT_header_offset= dh.e_lfanew;
	file_.Seek(NT_header_offset, CFile::begin);
	IMAGE_NT_HEADERS nth;
	file_.Read(&nth, sizeof nth);
	// increase the size of image by an amount of all added JPEG-s
	nth.OptionalHeader.SizeOfImage += data_size;
	nth.OptionalHeader.SizeOfInitializedData += data_size;
	file_.Seek(NT_header_offset, CFile::begin);
	file_.Write(&nth, sizeof nth);

	return true;
}


void CSlideShowGenerator::LaunchSlideShow()
{
	if (!path_.IsEmpty())
	{
		TCHAR cmd_line[MAX_PATH]= { 0 };
		PROCESS_INFORMATION pi;
		memset(&pi, 0, sizeof pi);
		STARTUPINFO si;
		memset(&si, 0, sizeof si);

		if (::CreateProcess(path_, cmd_line, 0, 0, false, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi))
		{
			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);
		}
	}
}
