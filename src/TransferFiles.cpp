/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransferFiles.cpp: implementation of the CTransferFiles class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransferFiles.h"
#include "Path.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CTransferFiles::CTransferFiles(bool read_only, bool clearArchiveAttr)
 : read_only_(read_only), clearSrcArchiveAttr_(clearArchiveAttr)
{}


CTransferFiles::~CTransferFiles()
{}


bool CTransferFiles::Transfer(bool copy, const std::vector<String>& srcFiles, const std::vector<String>& destFiles)
{
	if (!PrepareFiles(srcFiles, destFiles))
		return false;

	SHFILEOPSTRUCT op;
	op.hwnd						= *AfxGetMainWnd();
	op.wFunc					= copy ? FO_COPY : FO_MOVE;
	op.pFrom					= src_files_.data();
	op.pTo						= dest_files_.data();
	op.fFlags					= FOF_ALLOWUNDO | FOF_WANTNUKEWARNING | FOF_MULTIDESTFILES;
	op.fAnyOperationsAborted	= false;
	op.hNameMappings			= 0;
	op.lpszProgressTitle		= 0;

	if (::SHFileOperation(&op) == 0)
	{
		// set read only
		if (read_only_)
		{
			CWaitCursor wait;

			for (std::vector<String>::const_iterator it= destFiles.begin(); it != destFiles.end(); ++it)
			{
				DWORD attrib= ::GetFileAttributes(it->c_str());
				if (attrib != INVALID_FILE_ATTRIBUTES)
					::SetFileAttributes(it->c_str(), attrib | FILE_ATTRIBUTE_READONLY);
			}
		}

		// clear 'archive' attrib of source files to mark them as already read
		if (clearSrcArchiveAttr_)
		{
			CWaitCursor wait;

			for (std::vector<String>::const_iterator it= srcFiles.begin(); it != srcFiles.end(); ++it)
			{
				DWORD attrib= ::GetFileAttributes(it->c_str());
				if (attrib != INVALID_FILE_ATTRIBUTES)
					::SetFileAttributes(it->c_str(), attrib & ~FILE_ATTRIBUTE_ARCHIVE);
			}
		}
	}
#if 0	// DO NOT DELETE PARTIALLY COPIED FILES (they might have existed there before)
	else	// action cancelled--remove copied files (if any)
	{
		if (op.any_operations_aborted && copy)	// remove only if copied (leave moved files)
		{
			for (vector<SrcDest>::iterator it= files_.begin(); it != files_.end(); ++it)
				::DeleteFile(it->dest_.c_str());
		}
	}
#endif

	return false;
}


bool CTransferFiles::PrepareFiles(const std::vector<String>& srcFiles, const std::vector<String>& destFiles)
{
	src_files_.erase();
	dest_files_.erase();

	if (srcFiles.empty() || srcFiles.size() != destFiles.size())
	{
		ASSERT(false);
		return false;
	}
//	if (from_.empty())
//		return false;
/*
	const size_t count= srcFiles.size();
	files_.resize(count)
	for (size_t i= 0; i < count; ++i)
	{
		const Path& src= srcFiles[i];
		files_[i].src_ = src;

		Path dest= to_;
		dest.AppendDirSeparator();

		if (!pattern_.IsEmpty())	// rename file?
		{
			FILETIME ftm;
			VERIFY(find.GetLastWriteTime(&ftm));
			dest += ParsePattern(path.GetFileName().c_str(), ftm, ++serial_, pattern_);
			String ext= path.GetExtension();
			if (!ext.empty())
			{
				dest += _T('.');
				dest += ext;
			}
		}
		else	// same file name
		{
			dest += path.GetFileNameAndExt();
		}
	}
//	bool scan_subdirs= false;

//	if (!ScanDir(from_, scan_subdirs))
//		return false;

	// sort files
	std::sort(files_.begin(), files_.end(), CmpDest());

	// find duplicate destination names
	int count= files_.size();
	for (int index= 0; index < count - 1; )
	{
		int next= index + 1;

		// case insensitive comparision
		while (_tcsicmp(files_[index].dest_.c_str(), files_[next].dest_.c_str()) == 0)
		{
			TCHAR num[16];
			num[0] = _T('_');
			_itot(next - index, num + 1, 10);
			files_[next].dest_.AppendToFileName(num);
			++next;
			if (next == count)
				break;
		}

		index = next;
	}
*/
	const size_t count= srcFiles.size();
	for (size_t i= 0; i < count; ++i)
//	for (vector<SrcDest>::iterator it= files_.begin(); it != files_.end(); ++it)
	{
		src_files_ += srcFiles[i] + _T('\0');
		dest_files_ += destFiles[i] + _T('\0');
	}

	src_files_ += _T('\0');
	dest_files_ += _T('\0');

	return true;
}

/*
bool CTransferFiles::FileScan(Path path, int64 file_length, const CFileFind& find, int)
{
	if ((include_.empty() || path.MatchFileSpec(include_.c_str())) &&
		(exclude_.empty() || !path.MatchFileSpec(exclude_.c_str())))

	//if (path.MatchExtension(_T("jpg")) || path.MatchExtension(_T("jpeg")) ||
	//	path.MatchExtension(_T("jpe")) || path.MatchExtension(_T("thm")) ||
	//	path.MatchExtension(_T("nef")) || path.MatchExtension(_T("crw")) ||
	//	path.MatchExtension(_T("cr2")) || path.MatchExtension(_T("tif")))
	{
		Path dest= to_;
		dest.AppendDirSeparator();

		if (!pattern_.IsEmpty())	// rename file?
		{
			FILETIME ftm;
			VERIFY(find.GetLastWriteTime(&ftm));
			dest += ParsePattern(path.GetFileName().c_str(), ftm, ++serial_, pattern_);
			String ext= path.GetExtension();
			if (!ext.empty())
			{
				dest += _T('.');
				dest += ext;
			}
		}
		else	// same file name
		{
			dest += path.GetFileNameAndExt();
		}

		files_.push_back(SrcDest(path, dest));
	}

	return true;
} */
