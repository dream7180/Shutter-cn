/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "StdFile.h"
#include <fcntl.h>
#include <io.h>


StdFile::StdFile()
{
	file_ = 0;
	m_bCloseOnDelete = true;
}


StdFile::~StdFile()
{
	AFX_BEGIN_DESTRUCTOR	// don't throw here

	if (file_)
		Close();

	AFX_END_DESTRUCTOR
}


BOOL StdFile::Open(LPCTSTR file_name, UINT open_flags, CFileException* exception)
{
	ASSERT(exception == NULL || AfxIsValidAddress(exception, sizeof(CFileException)));
	ASSERT(file_name != NULL);
	ASSERT(AfxIsValidString(file_name));

	if (file_name == NULL)
		return false;

	file_ = NULL;
	if (!CFile::Open(file_name, (open_flags & ~typeText), exception))
		return false;

	ASSERT(m_hFile != hFileNull);
	ASSERT(m_bCloseOnDelete);

	char mode[4]; // C-runtime open string
	int mode_idx = 0;

	// determine read/write mode depending on CFile mode
	if (open_flags & modeCreate)
	{
		if (open_flags & modeNoTruncate)
			mode[mode_idx++] = 'a';
		else
			mode[mode_idx++] = 'w';
	}
	else if (open_flags & modeWrite)
		mode[mode_idx++] = 'a';
	else
		mode[mode_idx++] = 'r';

	// add '+' if necessary (when read/write modes mismatched)
	if (mode[0] == 'r' && (open_flags & modeReadWrite) || mode[0] != 'r' && !(open_flags & modeWrite))
	{
		// current mode mismatched, need to add '+' to fix
		mode[mode_idx++] = '+';
	}

	// will be inverted if not necessary
	int flags = _O_RDONLY | _O_TEXT;
	if (open_flags & (modeWrite | modeReadWrite))
		flags ^= _O_RDONLY;

	if (open_flags & typeBinary)
		mode[mode_idx++] = 'b', flags ^= _O_TEXT;
	else
		mode[mode_idx++] = 't';

	mode[mode_idx] = '\0';

	// open a C-runtime low-level file handle
	int handle= _open_osfhandle(reinterpret_cast<UINT_PTR>(m_hFile), flags);

	// open a C-runtime stream from that handle
	if (handle != -1)
		file_ = _fdopen(handle, mode);

	if (file_ == 0)
	{
		// an error somewhere along the way...
		if (exception != 0)
		{
			exception->m_lOsError = _doserrno;
			exception->m_cause = CFileException::OsErrorToException(_doserrno);
		}

		CFile::Abort(); // close m_hFile
		return false;
	}

	return true;
}


const CString& StdFile::FileName() const
{
	return m_strFileName;
}


void StdFile::Close()
{
	int err= 0;

	if (file_)
		err = fclose(file_);

	file_ = 0;
	m_bCloseOnDelete = FALSE;
	m_hFile = hFileNull;

	if (err != 0)
		AfxThrowFileException(CFileException::diskFull, _doserrno, FileName());
}


void StdFile::Write(const void* buffer, UINT count)
{
	ASSERT(file_ != 0);
	ASSERT(AfxIsValidAddress(buffer, count, false));

	if (buffer == 0)
		AfxThrowInvalidArgException();

	if (fwrite(buffer, sizeof(BYTE), count, file_) != count)
		AfxThrowFileException(CFileException::genericException, _doserrno, FileName());
}


UINT StdFile::Read(void* buffer, UINT count)
{
	ASSERT(file_ != 0);

	if (count == 0)
		return 0;

	ASSERT(AfxIsValidAddress(buffer, count));

	if (buffer == 0)
		AfxThrowInvalidArgException();

	UINT read= static_cast<UINT>(fread(buffer, sizeof(BYTE), count, file_));

	if (read == 0 && !feof(file_))
		AfxThrowFileException(CFileException::genericException, _doserrno, FileName());

	if (ferror(file_))
	{
		Afx_clearerr_s(file_);
		AfxThrowFileException(CFileException::genericException, _doserrno, FileName());
	}

	return read;
}


ULONGLONG StdFile::Seek(LONGLONG offset, UINT from)
{
	ASSERT(file_ != 0);

	if (_fseeki64(file_, offset, from) != 0)
		AfxThrowFileException(CFileException::badSeek, _doserrno, FileName());

	return _ftelli64(file_);
}


ULONGLONG StdFile::SeekToEnd()
{
	return Seek(0, CFile::end);
}


void StdFile::SeekToBegin()
{
	Seek(0, CFile::begin);
}


ULONGLONG StdFile::GetPosition() const
{
	ASSERT(file_ != 0);

	LONGLONG pos= _ftelli64(file_);
	if (pos == -1)
		AfxThrowFileException(CFileException::invalidFile, _doserrno, FileName());

	return pos;
}


bool StdFile::IsOpen() const
{
	return file_ != 0;
}
