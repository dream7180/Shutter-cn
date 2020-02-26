/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

namespace {

	struct Block
	{
		Block(bool& b) : b_(b)
		{
			b_ = true;
		}

		~Block()
		{
			b_ = false;
		}

		bool& b_;
	};

}
