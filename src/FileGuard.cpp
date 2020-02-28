/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileGuard.cpp: implementation of the FileGuard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileGuard.h"
#include "FileErrorDlg.h"
#include "CatchAll.h"
#include "Exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


FileGuard::FileGuard(const TCHAR* file, bool auto_restore/*= false*/) : auto_restore_(auto_restore)
{
	ASSERT(file && *file);

	BackUp(file);
}


FileGuard::FileGuard(bool auto_restore/*= false*/) : parent_(0), auto_restore_(auto_restore)
{
}


FileGuard::~FileGuard()
{
	if (auto_restore_)
		Restore(0);
}


void FileGuard::AnnulAutoRestore()
{
	auto_restore_ = false;
}


void FileGuard::SetAutoRestore()
{
	auto_restore_ = true;
}


void FileGuard::GetTempName(CString& name, int unique)
{
	TCHAR path[MAX_PATH * 2];
	::GetTempPath(MAX_PATH, path);
	name.Format(_T("%s%s_backup_%06x.tmp"), path, AfxGetAppName(), unique);
}


void FileGuard::BackUp(const TCHAR* file)
{
	original_file_ = file;

	CString temp;
	LARGE_INTEGER t;
	int unique= ::GetTickCount() + rand();
	if (!::QueryPerformanceCounter(&t))
		unique = t.LowPart;

	for (int i= 0; i < 20; ++i, unique += rand())
	{
		GetTempName(temp, unique);

		backup_file_ = temp;

		// attempt to create a backup copy of specified file

		if (::CopyFile(file, temp, true))
			return;

		DWORD err= ::GetLastError();

		if (err != ERROR_FILE_EXISTS && err != ERROR_ACCESS_DENIED)
		{
			::DeleteFile(temp);
			backup_file_.erase();
			_com_error error(err);
			const TCHAR* error_text= error.ErrorMessage();

			THROW_EXCEPTION(_T("未能创建图像文件的临时拷贝."), SF(L"创建临时文件的尝试失败.\n" << original_file_ << L"\n" << error_text));
		}
	}

	THROW_EXCEPTION(_T("未能创建图像文件的临时拷贝."), SF("创建临时文件的尝试失败.\n" << original_file_));
}


bool FileGuard::BackUp(const TCHAR* file, CWnd* parent) throw ()
{
	try
	{
		BackUp(file);

		return true;
	}
	CATCH_ALL_W(parent)

	return false;
}


DWORD FileGuard::Restore()
{
	if (backup_file_.empty() || original_file_.empty())
		return 0;

	if (::CopyFile(backup_file_.c_str(), original_file_.c_str(), false) != 0)
	{
		::DeleteFile(backup_file_.c_str());	// ReadOnly backup won't be deleted
		backup_file_.erase();
		original_file_.erase();
		return 0;
	}

	DWORD err= ::GetLastError();
	return err != 0 ? err : ~0;
}


bool FileGuard::Restore(CWnd* parent)
{
	try
	{
		DWORD err= Restore();

		if (err == 0)
			return true;

		// report error
		FileGuardExceptionRestore(backup_file_, original_file_, err).ReportError(parent);
	}
	catch (...)
	{
	}

	return false;
}


void FileGuard::DeleteBackup()
{
	if (!backup_file_.empty())
		::DeleteFile(backup_file_.c_str());	// ReadOnly backup won't be deleted

	backup_file_.erase();
	original_file_.erase();

	auto_restore_ = false;
}


//-----------------------------------------------------------------------------

BOOL FileGuardExceptionBackup::GetErrorMessage(LPTSTR error, UINT maxError, PUINT pnHelpContext/* = NULL*/)
{
	CString msg;
	msg.Format(_T("创建文件备份的尝试\n%s\n已失败, 代码 0x%x"), file_.c_str(), last_error_);
	if (!message_.empty())
	{
		msg += _T("\n");
		msg += message_.c_str();
	}

	if (error && maxError)
	{
		_tcsncpy(error, msg, maxError);
		error[maxError - 1] = 0;
	}

	return true;
}


void FileGuardExceptionBackup::ReportError(CWnd* parent)
{
	try
	{
		CString msg;
		msg.Format(_T("创建文件备份的尝试\n%s\n已失败, 代码 0x%x"), file_.c_str(), last_error_);
		if (!message_.empty())
		{
			msg += _T("\n");
			msg += message_.c_str();
		}

		// it's safe to use null parent_ here:
		parent->MessageBox(msg, 0, MB_OK | MB_ICONWARNING);
	}
	catch (...)
	{}
}


void FileGuardExceptionRestore::ReportError(CWnd* parent)
{
	try
	{
		FileErrorDlg dlg(parent);
		dlg.backup_ = backup_.c_str();
		dlg.original_ = original_.c_str();
		dlg.DoModal();

//		CString msg;
//		msg.Format(_T("An attempt to restore modified file\n%s\nusing backup copy\n%s\nfailed with an error code 0x%x"), , , last_error_);
		// it's safe to use null parent_ here:
//		parent_->MessageBox(msg, 0, MB_OK | MB_ICONWARNING);
	}
	catch (...)
	{}
}
