/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// replacement for CStdioFile capable of using files > 2GB

// currently buggy and seems to be broken beyond repair;
// does fread/frite even work together with _fseeki64/_ftelli64 correctly?
// fread(s) seem to be using old buffer content


class StdFile : public CFile
{
public:
	StdFile();
	virtual ~StdFile();

	virtual BOOL Open(LPCTSTR file_name, UINT open_flags, CFileException* error= 0);
	virtual void Close();

	virtual void Write(const void* buffer, UINT count);
	virtual UINT Read(void* buffer, UINT count);

	virtual ULONGLONG Seek(LONGLONG offset, UINT from);

	ULONGLONG SeekToEnd();
	void SeekToBegin();

	virtual ULONGLONG GetPosition() const;

	bool IsOpen() const;

private:
	FILE* file_;
	const CString& FileName() const;
};
