/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransferFiles.h: interface for the CTransferFiles class.

#pragma once
#include "Path.h"


class CTransferFiles
{
public:
	CTransferFiles(bool read_only, bool clearArchiveAttr);
	virtual ~CTransferFiles();

	bool Transfer(bool copy, const std::vector<String>& srcFiles, const std::vector<String>& destFiles);

private:
	bool read_only_;
	bool clearSrcArchiveAttr_;
	String src_files_;
	String dest_files_;

	bool PrepareFiles(const std::vector<String>& srcFiles, const std::vector<String>& destFiles);
};
