/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Path.h"


class FileSend //: CDocument
{
public:
	FileSend();
	virtual ~FileSend();

	void SendFile(const TCHAR* filename);

	bool IsSendCmdAvailable();

	void SendFiles(const std::vector<Path>& files, const std::vector<String>& names, const String* msg= 0, const char* subject= 0, const char* dest_addr= 0);

private:
	void OnUpdateFileSendMail(CCmdUI* cmd_ui);
	void FileSendMail(const TCHAR* file);
};
