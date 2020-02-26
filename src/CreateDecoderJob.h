/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

class PhotoInfo;
class DecoderJobInterface;

extern DecoderJobInterface* CreateImageDecoderJob(PhotoInfo& photo, CSize size, CWnd* receiver);

extern DecoderJobInterface* CreateThumbnailDecoderJob(PhotoInfo& photo, CSize, CWnd*);
