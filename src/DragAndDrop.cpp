/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DragAndDrop.h"
#include "PhotoInfoStorage.h"
#include "CatchAll.h"

void ConcatenatedPath(const VectPhotoInfo& photos, String& paths);


int DragAndDrop(VectPhotoInfo& selected, CWnd* frame)
{
TRACE(_T("DoDragDrop\n"));
	if (selected.empty())
		return DROPEFFECT_NONE;

	// collect all paths (concatenate)
	String src;
	{
		CWaitCursor wait;
		ConcatenatedPath(selected, src);
		if (src.empty())
			return DROPEFFECT_NONE;
	}

	// prepare clipboard CF_HDROP data
	DROPFILES df;
	df.pFiles = sizeof df;
	df.pt.x = df.pt.y = 0;
	df.fNC = 0;
	df.fWide = sizeof TCHAR == 1 ? 0 : 1;

	size_t len= src.size() * sizeof TCHAR;

	class GlobalMem
	{
	public:
		GlobalMem(size_t size)
		{
			mem_ = ::GlobalAlloc(GHND, size);
		}

		~GlobalMem()
		{
			if (mem_)	// unlock it?
				::GlobalFree(mem_);
		}

		void* Lock()
		{
			return mem_ ? ::GlobalLock(mem_) : 0;
		}

		HGLOBAL Handle() const		{ return mem_; }

	private:
		HGLOBAL mem_;
	};

	GlobalMem mem(sizeof(df) + len);
	void* data= mem.Lock();
	if (data == 0)
		THROW_EXCEPTION(L"Drag & drop failed.", L"Drag and drop cannot be completed due to the lack of memory");

	memcpy(data, &df, sizeof df);
	memcpy((char*)data + sizeof df, src.data(), len);

	// prepare OLE data source
	COleDataSource ds;
	FORMATETC fmt;
	fmt.cfFormat = CF_HDROP;
	fmt.ptd = 0;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;
	ds.CacheGlobalData(fmt.cfFormat, mem.Handle(), &fmt);

	// proceed with drag & drop

	class Drop : public COleDropSource
	{
	public:
		Drop()
		{
			// setup drag/drop sensitivity rect
			m_bDragStarted = FALSE;

			CPoint cursor;
			GetCursorPos(&cursor);
			m_rectStartDrag.SetRect(cursor.x, cursor.y, cursor.x, cursor.y);
			m_rectStartDrag.InflateRect(COleDropSource::nDragMinDist, COleDropSource::nDragMinDist);
		}

		void StartDrag(const CRect& first_item_rect, CPoint offset)
		{
//			CRect wnd_rect;
//			wnd->GetWindowRect(wnd_rect);
//			CPoint cursor;
//			GetCursorPos(&cursor);
//			CPoint start(20, 10);
//			wnd->ClientToScreen(&start);
//			start = wnd_rect.TopLeft() - start;

			img_list_photos_.BeginDrag(0, offset);

			img_list_photos_.DragEnter(0, first_item_rect.TopLeft());
		}

		void EndDrag()
		{
			img_list_photos_.DragLeave(0);
		}

		virtual SCODE QueryContinueDrag(BOOL escape_pressed, DWORD key_state)
		{
			CPoint cursor;
			GetCursorPos(&cursor);
			img_list_photos_.DragMove(cursor);
//			TRACE(L"query (%d, %d)\n", cursor.x, cursor.y);
			return COleDropSource::QueryContinueDrag(escape_pressed, key_state);
		}

		virtual SCODE GiveFeedback(DROPEFFECT dropEffect)
		{
			TRACE(_T("feedback %x\n"), int(dropEffect));
			return COleDropSource::GiveFeedback(dropEffect);
		}

		CImageList img_list_photos_;	// image of photos being dragged
	} dropSource;

	ASSERT_VALID(&dropSource);

//	CreatePaneImage(dropSource.img_list_photos_, photo_ctrl_wnd_.GetImageSize(), selected.front());
	bool reenable_drag_and_drop= false;
	if (frame->GetExStyle() & WS_EX_ACCEPTFILES)
	{
		reenable_drag_and_drop = true;
		frame->DragAcceptFiles(false);
	}

	// before calling OLE drag/drop code, wait for mouse to move outside
	//  the rectangle
	ASSERT_VALID(AfxGetMainWnd());
	HRESULT result= DRAGDROP_S_CANCEL;
	DWORD result_effect = DROPEFFECT_NONE;
	if (dropSource.OnBeginDrag(AfxGetMainWnd()))
	{
		//CPoint cursor;
		//GetCursorPos(&cursor);
		//ClientToScreen(&cursor);
		//CRect rect= photo_ctrl_wnd_.GetItemRect(selected.front(), true);
//		dropSource.StartDrag(rect, rect.TopLeft() - cursor);
		// call global OLE api to do the drag drop
		LPDATAOBJECT data_object = (LPDATAOBJECT)ds.GetInterface(&IID_IDataObject);
		LPDROPSOURCE drop_source = (LPDROPSOURCE)dropSource.GetInterface(&IID_IDropSource);
		result = ::DoDragDrop(data_object, drop_source, DROPEFFECT_COPY | DROPEFFECT_MOVE, &result_effect);
//		dropSource.EndDrag();
	}

	if (reenable_drag_and_drop)
		frame->DragAcceptFiles(true);

//	::GlobalFree(data);

	// files moved?
	return result == DRAGDROP_S_DROP ? result_effect : DROPEFFECT_NONE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////


class PhotoDropTarget : public COleDropTarget
{
public:
	PhotoDropTarget(CWnd& wnd, bool copy_oper, const boost::function<DROPEFFECT (PhotoDrop::DropAction action, const TCHAR* files)>& callback)
		: wnd_(wnd), can_accept_(false), /*drop_target_(0),*/ copy_operation_(copy_oper), callback_(callback)
	{}

	virtual ~PhotoDropTarget()
	{}

	virtual DROPEFFECT OnDragEnter(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
//	virtual BOOL OnDrop(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual void OnDragLeave(CWnd* wnd);
	virtual DROPEFFECT OnDragScroll(CWnd* wnd, DWORD key_state, CPoint point);

private:
	CWnd& wnd_;
	bool can_accept_;
	bool copy_operation_;
//	HTREEITEM drop_target_;
	boost::function<DROPEFFECT (PhotoDrop::DropAction action, const TCHAR* files)> callback_;
	std::vector<TCHAR> dropped_files_;

//	DECLARE_DYNAMIC(PhotoDropTarget)
};

//IMPLEMENT_DYNAMIC(PhotoDropTarget, COleDropTarget)


void GetData(COleDataObject* data_object, std::vector<TCHAR>& dropped_files)
{
	dropped_files.clear();

	if (data_object == 0) // || !can_accept_) // || drop_target_ == 0)
		return;

	STGMEDIUM medium;
	memset(&medium, 0, sizeof medium);

	FORMATETC fmt;
	fmt.cfFormat = CF_HDROP;
	fmt.ptd = 0;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;

	if (data_object->GetData(CF_HDROP, &medium, &fmt) && medium.tymed == TYMED_HGLOBAL && medium.hGlobal != 0)
	{
		struct lock
		{
			lock(HGLOBAL mem) : mem(mem)
			{
				data = ::GlobalLock(mem);
			}

			~lock()
			{
				::GlobalUnlock(mem);
			}

			void* data;
			HGLOBAL mem;

		} mem(medium.hGlobal);

		if (void* data= mem.data)
		{
			DROPFILES* df= reinterpret_cast<DROPFILES*>(data);
			BYTE* files= reinterpret_cast<BYTE*>(data) + df->pFiles;

//			String dest_path= wnd_.GetPhysicalPath(drop_target_);

			if (df->fWide)
			{
#ifdef _UNICODE
				const wchar_t* src= reinterpret_cast<wchar_t*>(files);
				dropped_files.resize(wcslen(src) + 1);
				wcscpy(&dropped_files.front(), src);
//				dest_path.push_back(0);
//				op.to    = dest_path.data();
#else
//				wstring dest;
//				::MultiByteToWideString(dest_path, dest, CP_ACP);
//				dest.push_back(0);
//				op.to    = dest.data();
#endif
				//op.flags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
				//op.any_operations_aborted = false;
				//op.name_mappings = 0;
				//op.progress_title = 0;
				//::SHFileOperationW(&op);
			}
			else
			{
				//SHFILEOPSTRUCTA op;
				//memset(&op, 0, sizeof op);
				//op.hwnd   = wnd_.m_hWnd;
				//op.func  = copy_operation_ ? FO_COPY : FO_MOVE;
				//op.from  = reinterpret_cast<char*>(files);

//				const char* src= reinterpret_cast<char*>(files);

#ifdef _UNICODE
				std::string dest;
//				::WideStringToMultiByte(dest_path.c_str(), dest, CP_ACP);
//				dest.push_back(0);
//				op.to    = dest.data();
#else
//				dest_path.push_back(0);
//				op.to    = dest_path.data();
				const char* src= reinterpret_cast<char*>(files);
				dropped_files.resize(strlen(src) + 1);
				strcpy(&dropped_files.front(), src);
#endif
				//op.flags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
				//op.any_operations_aborted = false;
				//op.name_mappings = 0;
				//op.progress_title = 0;
				//::SHFileOperationA(&op);
			}

//			effect = copy_operation_ ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		}
	}
}



DROPEFFECT PhotoDropTarget::OnDragEnter(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point)
{
	try
	{
		COleDropTarget::OnDragEnter(wnd, data_object, key_state, point);
		//wnd_.SelectDropTarget(0);

		can_accept_ = data_object && data_object->IsDataAvailable(CF_HDROP);

		if (can_accept_)
			GetData(data_object, dropped_files_);
		else
			dropped_files_.clear();

		if (callback_(PhotoDrop::Enter, dropped_files_.empty() ? 0 : &dropped_files_.front()) == DROPEFFECT_NONE)
		{
			can_accept_ = false;
			swap(dropped_files_, std::vector<TCHAR>());
		}
	}
	CATCH_ALL_W(&wnd_)

	return DROPEFFECT_NONE;
}


void PhotoDropTarget::OnDragLeave(CWnd* wnd)
{
	try
	{
		COleDropTarget::OnDragLeave(wnd);

		callback_(PhotoDrop::Leave, dropped_files_.empty() ? 0 : &dropped_files_.front());

		swap(dropped_files_, std::vector<TCHAR>());
	}
	CATCH_ALL_W(&wnd_)

	//wnd_.SelectDropTarget(0);
//	return DROPEFFECT_NONE;
}


DROPEFFECT PhotoDropTarget::OnDropEx(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
	DROPEFFECT effect= DROPEFFECT_NONE;
	try
	{
		//wnd_.SelectDropTarget(0);

		if (data_object == 0 || !can_accept_ || /*drop_target_ == 0 ||*/ dropped_files_.empty())
			return effect;

		effect = callback_(PhotoDrop::Drop, dropped_files_.empty() ? 0 : &dropped_files_.front());
	}
	CATCH_ALL_W(&wnd_)

	return effect;
}


// scroll tree content as items dragged are close to the top or bottom of the window
DROPEFFECT PhotoDropTarget::OnDragScroll(CWnd* wnd, DWORD key_state, CPoint point)
{
	DROPEFFECT effect= DROPEFFECT_NONE;
	//TODO
/*
	CRect rect(0,0,0,0);
	wnd_.GetClientRect(rect);

	CPoint p= point;
//	wnd_.ScreenToClient(&p);
	if (!rect.PtInRect(p))
		return effect;

	const int MARGIN= max<int>(wnd_.GetItemHeight() / 2, 6);
	bool scroll_up= true;

	if (p.y < rect.top + MARGIN)
		scroll_up = true;
	else if (p.y > rect.bottom - MARGIN)
		scroll_up = false;
	else
		return effect;

	if (wnd_.GetStyle() & WS_VSCROLL)
	{
		const DWORD SLEEP= 50;

		if (scroll_up)
		{
			if (HTREEITEM first= wnd_.GetFirstVisibleItem())
				if (HTREEITEM prev= wnd_.GetPrevVisibleItem(first))
				{
					wnd_.EnsureVisible(prev);
					wnd_.UpdateWindow();
					::Sleep(SLEEP);
					effect = DROPEFFECT_SCROLL;
				}
		}
		else
		{
			if (HTREEITEM first= wnd_.GetFirstVisibleItem())
			{
				HTREEITEM item= first;
				UINT count= wnd_.GetVisibleCount();
				for (UINT i= 0; i < count; ++i)
				{
					item = wnd_.GetNextVisibleItem(item);
					if (item == 0)
						break;
				}

				if (item)
				{
					wnd_.EnsureVisible(item);
					wnd_.UpdateWindow();
					::Sleep(SLEEP);
					effect = DROPEFFECT_SCROLL;
				}
			}
		}

	}
*/
	return effect;
}


DROPEFFECT PhotoDropTarget::OnDragOver(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point)
{
	DROPEFFECT effect= DROPEFFECT_NONE;

	try
	{
		COleDropTarget::OnDragOver(wnd, data_object, key_state, point);

		if (wnd != &wnd_ || !can_accept_)
			return DROPEFFECT_NONE;

		effect = callback_(PhotoDrop::DragOver, dropped_files_.empty() ? 0 : &dropped_files_.front());
/*
	UINT flags= 0;
	HTREEITEM item= wnd_.HitTest(point, &flags);

	if (item != 0 && (TVHT_ONITEM & flags) && wnd_.IsPhysicalPath(item))
	{
		drop_target_ = item;

		wnd_.SelectDropTarget(item);

		copy_operation_ = !!(key_state & MK_CONTROL);

		return copy_operation_ ? DROPEFFECT_COPY : DROPEFFECT_MOVE;

	}
	else
	{
		drop_target_ = 0;

		wnd_.SelectDropTarget(0);
	}

	if (item != 0 && (TVHT_ONITEMBUTTON & flags))
		wnd_.Expand(item, TVE_EXPAND);

	return DROPEFFECT_NONE; */

	}
	CATCH_ALL_W(&wnd_)

	return effect;
}


namespace PhotoDrop
{

COleDropTarget* CreatePhotoDropTarget(CWnd& wnd, bool copy_oper, const boost::function<DROPEFFECT (DropAction action, const TCHAR* files)>& callback)
{
	return new PhotoDropTarget(wnd, copy_oper, callback);
}


}
