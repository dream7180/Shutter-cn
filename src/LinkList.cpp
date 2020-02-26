/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "LinkList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/********************************************************/
/*						CLinkList						*/
/********************************************************/
CLinkList::CLinkList()
{
	head_ = 0;
	tail_ = 0;
	current_ = 0;
}
/********************************************************/
/*						~CLinkList						*/
/********************************************************/
CLinkList::~CLinkList()
{
	DeleteAll();
}
/********************************************************/
/*						Add								*/
/********************************************************/
void* CLinkList::Add(void* value)
{
	LINKLISTITEM* item = new LINKLISTITEM;

	if(!item)
		return 0;

	item->value = value;
	item->prev = tail_;
	item->next = 0;

	if(!head_)
		head_ = item;

	if(tail_)
		tail_->next = item;

	tail_ = item;
	return item;
}
/********************************************************/
/*						Next							*/
/********************************************************/
void* CLinkList::Next()
{
	if(NextItem())
		return current_->value;

	return 0;
}
/********************************************************/
/*						IsEmpty							*/
/********************************************************/
char CLinkList::IsEmpty()
{
	return !head_;
}
/********************************************************/
/*						Next(item)						*/
/********************************************************/
LINKLISTITEM* CLinkList::NextItem()
{
	current_ = (current_) ? current_->next : head_;
	return current_;
}
/********************************************************/
/*						Delete							*/
/********************************************************/
void CLinkList::Delete(LINKLISTITEM* item)
{
	if(!item)
		return;

	if(head_ == item)
		if(head_ = head_->next)
			head_->prev = 0;

	if(tail_ == item)
		if(tail_ = tail_->prev)
			tail_->next = 0;

	if(current_ == item)
		current_ = current_->next;

	if(item->prev)
		item->prev->next = item->next;

	if(item->next)
		item->next->prev = item->prev;

	delete item;
}
/********************************************************/
/*						DeleteAll						*/
/********************************************************/
void CLinkList::DeleteAll()
{
	while (head_)
	{
		current_ = head_->next;
		delete head_;
		head_ = current_;
	}
	
	tail_ = 0;
	current_ = 0;
}
