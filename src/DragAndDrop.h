/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
#include "VectPhotoInfo.h"


extern int DragAndDrop(VectPhotoInfo& selected, CWnd* frame);

namespace PhotoDrop
{

	enum DropAction
	{
		Enter, Leave, Drop, Scroll, DragOver
	};

	COleDropTarget* CreatePhotoDropTarget(CWnd& wnd, bool copy_oper, const boost::function<DROPEFFECT (DropAction action, const TCHAR* files)>& callback);

}
