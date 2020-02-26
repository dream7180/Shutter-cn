/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

class ImageDatabase;

extern ImageDatabase& GetImageDataBase(bool read_only, bool open);

extern void SetDeleteFlagForImageDataBase(bool del);
extern bool GetDeleteFlagForImageDataBase();

extern void DeleteImageDataBase();

extern String GetDefaultDbFolder();
extern String GetConfiguredDbFileAndPath();
