/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

/* CircularFifo.h 
* Not any company's property but Public-Domain
* Do with source-code as you will. No requirement to keep this
* header if need to use it/change it/ or do whatever with it
*
* Note that there is No guarantee that this code will work 
* and I take no responsibility for this code and any problems you
* might get if using it. The code is highly platform dependent!
*
* Code & platform dependent issues with it was originally 
* published at http://www.kjellkod.cc/threadsafecircularqueue
* 2009-11-02
* @author Kjell Hedström, hedstrom@kjellkod.cc */

#ifndef CIRCULARFIFO_H_
#define CIRCULARFIFO_H_

/** Circular Fifo (a.k.a. Circular Buffer) 
* Thread safe for one reader, and one writer */
template<typename Element, unsigned int Size>
class CircularFifo {
public:
	enum { CAPACITY = Size + 1 };

	CircularFifo() : tail(0), head(0){}
	virtual ~CircularFifo() {}

	bool push(Element& item);
	bool pop(Element& item);

	bool is_empty() const;
	bool is_full() const;

	void clear();

private:
	volatile unsigned int tail; // input index
	Element array[CAPACITY];
	volatile unsigned int head; // output index

	unsigned int increment(unsigned int idx) const;
};


/** Producer only: Adds item to the circular queue. 
* If queue is full at 'push' operation no update/overwrite
* will happen, it is up to the caller to handle this case
*
* \param item copy by reference the input item
* \return whether operation was successful or not */
template<typename Element, unsigned int Size>
bool CircularFifo<Element, Size>::push(Element& item)
{
	int nextTail = increment(tail);
	if (nextTail != head)
	{
		array[tail] = item;
		tail = nextTail;
		return true;
	}

	// queue was full
	return false;
}


/** Consumer only: Removes and returns item from the queue
* If queue is empty at 'pop' operation no retrieve will happen
* It is up to the caller to handle this case
*
* \param item return by reference the wanted item
* \return whether operation was successful or not */
template<typename Element, unsigned int Size>
bool CircularFifo<Element, Size>::pop(Element& item)
{
	if (head == tail)
		return false;  // empty queue

	item = array[head];
	  array[head] = Element(); // mik: empty smart ptr
	head = increment(head);
	return true;
}


/** Useful for testinng and Consumer check of status
* Remember that the 'empty' status can change quickly
* as the Producer adds more items.
*
* \return true if circular buffer is empty */
template<typename Element, unsigned int Size>
bool CircularFifo<Element, Size>::is_empty() const
{
	return head == tail;
}


/** Useful for testing and Producer check of status
* Remember that the 'full' status can change quickly
* as the Consumer catches up.
*
* \return true if circular buffer is full.  */
template<typename Element, unsigned int Size>
bool CircularFifo<Element, Size>::is_full() const
{
	int tailCheck = (tail + 1) % CAPACITY;
	return tailCheck == head;
}

/** Increment helper function for index of the circular queue
* index is inremented or wrapped
*
*  \param idx_ the index to the incremented/wrapped
*  \return new value for the index */
template<typename Element, unsigned int Size>
unsigned int CircularFifo<Element, Size>::increment(unsigned int idx) const
{
	// increment or wrap
	// =================
	//    index++;
	//    if (index == array.lenght) -> index = 0;
	//
	//or as written below:   
	//    index = (index + 1) % array.length
	idx = (idx + 1) % CAPACITY;
	return idx;
}


template<typename Element, unsigned int Size>
void CircularFifo<Element, Size>::clear()
{
	head = tail;
}


#endif /* CIRCULARFIFO_H_ */
