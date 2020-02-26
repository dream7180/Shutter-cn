/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000 Michael Kowalski
____________________________________________________________________________*/

#ifndef _Img_Scanner_h_
#define _Img_Scanner_h_
#pragma once

#include "PhotoInfoStorage.h"
#include "ImageScanner.h"
#include "CircularFifo.h"

//#define BOOST_THREAD_NO_LIB 1
//#define BOOST_DATE_TIME_NO_LIB 1
//#include <boost/date_time.hpp>
//#include <boost/thread/thread.hpp>

class ImageDatabase;
class CatalogScanner;


// physical directory scanner looking for images

class ImgScanner : public ImageScanner
{
public:
	ImgScanner(const TCHAR* path, const TCHAR* selectedFile= 0);

	virtual ~ImgScanner();

	virtual bool Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store);

	virtual void CancelScan();

	SmartPhotoPtr SingleFileScan(Path path, uint32 dir_visited);

private:
	SmartPhotoPtr FileScan(Path path, uint64 file_length, const FILETIME& file_time, uint32 dir_visited, bool scanSubdirs, bool selectedFile);

	bool FileScanAndAppend(Path path, uint64 file_length, const CFileFind& find, uint32 dir_visited, bool scanSubdirs, bool selectedFile);

	uint32 visited_dir_id_;
	Path dir_to_scan_;
	String selected_file_;
	bool cancel_;
	uint32 dir_visited_;
	PhotoInfoStorage* photos_;

	void ReadImage(SmartPhotoPtr& info, const Path& path, uint64 file_length, uint32 dir_visited, bool generateThumbnails, bool selectedFile);
	//void ReadFileTimeStamp(PhotoInfo& photo, const Path& path);

	bool ScanDir(Path path, bool scan_subdirs, const String* selectedFile);

	void ScanCatalog(const Path& path, uint32& dirVisited, bool scanSubdirs);
	void MessageBox(const TCHAR* msg, int = 0, int = 0);

//	void Notify();

	void ForwardNotification(ImageScanner* scanner);

	void ProcessingThread();

	struct File
	{
		File(Path path, uint64 file_length, const CFileFind& find, uint32 dir_visited) : path(path)
		{
			this->file_length = file_length;
			find.GetLastWriteTime(&file_time);
			this->dir_visited = dir_visited;
		}

		Path path;
		uint64 file_length;
		FILETIME file_time;
		uint32 dir_visited;
	};

	typedef boost::shared_ptr<File> FilePtr;


	// single producer, multiple consumer queue;
	// thread safe (hopefully), and exception safe, when queue_ operations
	// do not throw (pop and push doesn't allocate memory);
	// event, semaphore, critical section, and CircularFifo follow RIIA, and
	// the queue is either fully constructed or exception occurs during construction
	// original source: http://www.codeproject.com/KB/threads/semaphores.aspx
	class xQueue
	{
		enum { SIZE= 
#ifdef _WIN64
			10000
#else
			1000
#endif
		};
	public:
		xQueue() :
		  event_(FALSE,		// initially non-signaled
				 TRUE),		// manual reset
		  semaphore_(0, SIZE)
		{
			shutdown_ = false;
			handles[SemaphoreIndex] = semaphore_;
			handles[StopperIndex] = event_;
		}

		// single thread producer adds new element for further processing;
		// if queue is full, it has to wait (sleep), and retry later;
		// to signal end of data, producer adds empty elements (one for each consumer thread)
		bool add_tail(FilePtr p)
		{ 
			if (!queue_.push(p))
				return false;	// queue full

			BOOL result= semaphore_.Unlock(1); // release semaphore
			if (!result)
			{ // failed
				// caller can use ::GetLastError to determine what went wrong
				// note: typically we won't hit this condition, when queue_ becomes
				// full its push() fails, so semaphor has no chance to hit the max,
				// but after shutdown() we may come here, because it empties the queue
				// but leaves sempaphore intact; there's no way to reset semaphore
				CSingleLock l(&lock_, true);
				FilePtr dummy;
				queue_.pop(dummy);
				return false;
			}

			return true; // ok
		}

		// multithreaded consumers retrieve data to process; they call this method and
		// it waits for data if it's not available yet; empty element on return signals
		// to the consumer that it should terminate (no more data available)
		FilePtr remove_head()
		{
			switch (::WaitForMultipleObjects(2, handles, FALSE, INFINITE))
			{
			case StopperIndex:   // shut down thread
				return FilePtr();	// return 'null' to make consumer thread exit (this doesn't throw)

			case SemaphoreIndex: // semaphore
				{
					FilePtr result;	// doesn't throw
					{
						// lock it, one consumer thread at a time
						CSingleLock l(&lock_, true);
						//  copy smart ptr, no throw
						VERIFY(queue_.pop(result));
					}
					return result;	// return result, doesn't throw
				}

			case WAIT_TIMEOUT: // not implemented
			default:
				ASSERT(false); // impossible condition
				return FilePtr();
			}
		}

		// call to stop handing out data to consumer threads; instead they will be served
		// empty data which acts as the 'end of job & terminate' marker;
		// data processing will be interrupted without completion (queue will be emptied)
		void shutdown()
		{
			shutdown_ = true;
			event_.SetEvent();
			queue_.clear();		// empty it, so null pointers can be addedd that signal end in ImgScanner::Scan()
		}

		bool is_shut_down() const
		{
			return shutdown_;
		}

	protected:
		enum { StopperIndex, SemaphoreIndex };
		HANDLE handles[2];
		CEvent event_;
		CSemaphore semaphore_;
		CCriticalSection lock_;
		CircularFifo<FilePtr, SIZE> queue_;
		volatile bool shutdown_;
	};

	void QueueFileEntry(FilePtr p);

	// thread pool of image processors
	boost::thread_group processor_;
	xQueue wait_list_;
};


#endif	// _Img_Scanner_h_
