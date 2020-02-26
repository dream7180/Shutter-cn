/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include <boost/noncopyable.hpp>
class ImgProcessingThread;

// helper class: it can clone 'worker_thread' and run multiple threads of execution

class ImgProcessingPool : boost::noncopyable
{
public:
	ImgProcessingPool(std::auto_ptr<ImgProcessingThread> worker_thread, size_t max_threads= 0);

	~ImgProcessingPool();

	size_t GetFileCount() const;
	void SetStatusWnd(CWnd* wnd);

	size_t ThreadCount() const;

	void Start();
	void Quit();

private:
	struct Impl;
	Impl& impl_;
};
