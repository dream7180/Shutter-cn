/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "ImgPage.h"
#include "Path.h"


class RenamePage : public ImgPage
{
public:
	RenamePage(int dlg_id) : ImgPage(dlg_id, 0, false)
	{}

	enum State { Start, Next, End };

	virtual bool GenerateFileName(State state, const PhotoInfo* input, Path* output_name) = 0;

	virtual bool ShowFullPath() = 0;

	virtual bool UpdateData() = 0;
};
