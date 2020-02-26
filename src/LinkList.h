/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

struct LINKLISTITEM
{
	void* value;
	LINKLISTITEM* prev;
	LINKLISTITEM* next;
};


class CLinkList
{
public:
	CLinkList();
	~CLinkList();
	void* Add(void* value);
	void* Next();
	LINKLISTITEM* NextItem();
	void Delete(LINKLISTITEM* item);
	char IsEmpty();
	void DeleteAll();
private:
	LINKLISTITEM* head_;
	LINKLISTITEM* tail_;
	LINKLISTITEM* current_;
};
