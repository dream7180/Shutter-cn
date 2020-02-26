/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// popup menu with image attributes; returns selected text or null
const TCHAR* ImgAttribsPopup(CWnd* wnd, CPoint pos);

// get help text for script methods and tables
enum ImgAttributeHelp { IAH_BASIC_INFO= 0, IAH_FILE_INFO= 1, IAH_TAG_INFO= 2 };
String GetImgAttribsHelpString(unsigned int include_help);

// common script editing font
void CreateScriptEditingFont(CFont& font);

// popup menu with image metadata attributes; returns selected text or null
const TCHAR* ImgMetaAttribsPopup(CWnd* wnd, CPoint pos);

// popup menu with file name attributes; returns selected text or null
const TCHAR* ImgFileAttribsPopup(CWnd* wnd, CPoint pos);

// attribute names
const TCHAR* GetImageAttribName(size_t index);
const TCHAR* GetMetaAttribName(size_t index);
const TCHAR* GetFileAttribName(size_t index);
