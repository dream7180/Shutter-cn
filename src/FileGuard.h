/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileGuard.h: interface for the FileGuard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEGUARD_H__01B0E9A8_1BE0_4C8D_98B3_976AB2DBE27A__INCLUDED_)
#define AFX_FILEGUARD_H__01B0E9A8_1BE0_4C8D_98B3_976AB2DBE27A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class FileGuard
{
public:
	FileGuard(bool auto_restore= false);
	FileGuard(const TCHAR* file, bool auto_restore= false);

	~FileGuard();

	// try to back up original file; show msg if copying fails
	bool BackUp(const TCHAR* file, CWnd* parent) throw ();

	// ditto, but throw on failure
	void BackUp(const TCHAR* file);

	// try to restore original file from backup; show msg if restoring fails
	bool Restore(CWnd* parent) throw ();

	void DeleteBackup();

	void AnnulAutoRestore();

	void SetAutoRestore();

private:
	// restore file
	DWORD Restore();

	void GetTempName(CString& name, int unique);

	String original_file_;
	String backup_file_;
	CWnd* parent_;
	bool auto_restore_;
};


//-------------------------------------------------------------------

class FileGuardException : public CException
{
public:
	FileGuardException(DWORD err) : last_error_(err)
	{}

//	virtual void ReportError() = 0;

protected:
//	CWnd* parent_;
	DWORD last_error_;
};


class FileGuardExceptionBackup : public FileGuardException
{
public:
	FileGuardExceptionBackup(const TCHAR* msg, const String& original, DWORD err)
		: FileGuardException(err), message_(msg), file_(original)
	{}

	virtual BOOL GetErrorMessage(LPTSTR error, UINT maxError, PUINT pnHelpContext = NULL);

	void ReportError(CWnd* parent);

private:
	String file_;
	String message_;
};


class FileGuardExceptionRestore : public FileGuardException
{
public:
	FileGuardExceptionRestore(const String& copy, const String& original, DWORD err)
		: FileGuardException(err), original_(original), backup_(copy)
	{}

	void ReportError(CWnd* parent);

private:
	String original_;
	String backup_;
};


#endif // !defined(AFX_FILEGUARD_H__01B0E9A8_1BE0_4C8D_98B3_976AB2DBE27A__INCLUDED_)
