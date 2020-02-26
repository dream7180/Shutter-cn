/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Using Semaphores: Multithreaded Producer/Consumer
// By Joseph M. Newcomer
// http://www.codeproject.com/KB/threads/semaphores.aspx

// MiK: modifications; shared_ptr, deque, CSingleLock

#include <boost/shared_ptr.hpp>
#include <deque>


template<class T>
class Queue
{
public:
	typedef boost::shared_ptr<T> Element;

	//----------------
	// Queue
	//----------------

	Queue(UINT limit)
	{
		handles[SemaphoreIndex] = ::CreateSemaphore(NULL,  // no security attributes
			0,     // initial count
			limit, // max count
			NULL); // anonymous

		handles[StopperIndex] = ::CreateEvent(NULL,  // no security attributes
			TRUE,  // manual reset
			FALSE, // initially non-signaled
			NULL); // anonymous

		ASSERT(handles[SemaphoreIndex]);
		ASSERT(handles[StopperIndex]);
	}

	//----------------
	// ~Queue
	//----------------

	~Queue()
	{
		::CloseHandle(handles[SemaphoreIndex]);
		::CloseHandle(handles[StopperIndex]);
	}

	//----------------
	// AddTail
	//----------------

	bool AddTail(Element p)
	{
		CSingleLock lock(&lock_, true);
		queue_.push_back(p);
		bool result= !!::ReleaseSemaphore(handles[SemaphoreIndex], 1, NULL);
		if (!result)
		{ /* failed */
			// caller can use ::GetLastError to determine what went wrong
			queue_.pop_back();
		} /* failed */
		return result;
	}

	//----------------
	// RemoveHead
	//----------------

	Element RemoveHead()
	{
		switch (::WaitForMultipleObjects(2, handles, FALSE, INFINITE))
		{ /* decode */
		case StopperIndex: // shut down thread
			return Element();	// empty element to signal exit

		case SemaphoreIndex: // semaphore
			{
				CSingleLock lock(&lock_, true);
				Element result= queue_.front();
				queue_.pop_front();
				return result;
			}

		case WAIT_TIMEOUT: // not implemented
		default:
			ASSERT(FALSE); // impossible condition
			return Element();
		} /* decode */
	}

	//----------------
	// shutdown
	//----------------

	void shutdown()
	{
		::SetEvent(handles[StopperIndex]);
	}

protected:
	enum { StopperIndex, SemaphoreIndex };
	HANDLE handles[2];
	CCriticalSection lock_;
	std::deque<Element> queue_;
};
