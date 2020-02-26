/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgProcessingThread.h: interface for the ImgProcessingThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Path.h"
class ImageDecoder;
class PhotoInfo;
class PhotoQueue;


enum ImgProcessingThreadMsg
{
	IPM_PHOTO_STATUS= WM_USER + 995,
	IPM_EXCEPTION= WM_USER + 996,
	IPM_NEXT_OPERATION= WM_USER + 997,
	IPM_NEXT_PHOTO= WM_USER + 998,
	IPM_PROGRESS= WM_USER + 999,
	IPM_RESERVED= WM_USER + 1000
};

static const int IPM_COMPLETION_STATUS_OK= -1;
static const int IPM_COMPLETION_STATUS_ERR= -2;


class ImgProcessingThread
{
public:
	ImgProcessingThread(size_t fileCount);

	void Start();
	void Start(PhotoQueue* index);
	void Quit();

	// process one image
	virtual void Process(size_t index) = 0;

	// asynchronous break: stop request
	virtual void Break();

	// count of files to process
	size_t GetFileCount() const			{ return file_count_; }

	virtual String GetSourceFileName(size_t index) const = 0;
	virtual String GetDestFileName(size_t index) const = 0;

	void SetStatusWnd(CWnd* status_wnd)	{ status_wnd_ = status_wnd; }

	virtual ~ImgProcessingThread();

	virtual ImgProcessingThread* Clone() = 0;

	bool StopProcessing() const			{ return break_; }

protected:
	// helper progress intercepting callback functions to be registered by derived classes

	// decoder progress
	bool LinesDecoded(int lines_ready, int img_height, bool finished);
	// encoder progress
	bool LinesEncoded(int lines_ready, int img_height, bool finished);

	void SetOperationLabel(int str_rsrc_id);

	void SendPhotoStatus(int status) const;

private:
	bool break_;
	HANDLE thread_handle_;
	CWinThread* thread_;
	bool pending_;
	CWnd* status_wnd_;
	size_t file_count_;
	size_t cur_file_index_;
	PhotoQueue* index_;

	static UINT AFX_CDECL WorkerProc(LPVOID param);
	void Exec();
};
