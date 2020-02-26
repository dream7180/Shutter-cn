/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

template<class T>
T inline clamp(T arg, T min, T max)
{
	if (arg < min)
		return min;
	if (arg > max)
		return max;
	return arg;
}
