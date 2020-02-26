/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

enum JpegMarkers
{
	MARK_SOI= 0xffd8,
	MARK_EOI= 0xffd9,
	MARK_APP0= 0xffe0,
	MARK_APP1= 0xffe1,
	MARK_APP2= 0xffe2,
	MARK_APP3= 0xffe3,
	MARK_APP4= 0xffe4,
	MARK_APP5= 0xffe5,
	MARK_APP6= 0xffe6,
	MARK_APP7= 0xffe7,
	MARK_APP8= 0xffe8,
	MARK_APP9= 0xffe9,
	MARK_APP10= 0xffea,
	MARK_APP11= 0xffeb,
	MARK_APP12= 0xffec,
	MARK_APP13= 0xffed,
	MARK_APP14= 0xffee,
	MARK_APP15= 0xffef,

	MARK_COM= 0xfffe,

	MARK_DQT= 0xffdb,
	MARK_DNL= 0xffdc,
	MARK_DRI= 0xffdd
};
