/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "intrusive_ptr.h"

class ColorProfile;

typedef mik::intrusive_ptr<ColorProfile> ColorProfilePtr;

class ColorTransform;

typedef mik::intrusive_ptr<ColorTransform> ColorTransformPtr;

class ColorException;
