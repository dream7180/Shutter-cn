/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _tasks_h_
#define _tasks_h_

#include "PhotoInfoStorage.h"
#include "Transform.h"
#include <boost/function.hpp>


class CTask
{
	virtual bool Go()= 0;
};

//-----------------------------------------------------------------------------

class CTaskResize : public CTask
{
public:
	CTaskResize(VectPhotoInfo& selected, CWnd* parent) : selected_(selected), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& selected_;
	CWnd* parent_;
};

//-----------------------------------------------------------------------------

class CTaskGenSlideShow : public CTask
{
public:
	CTaskGenSlideShow(VectPhotoInfo& selected) : selected_(selected)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& selected_;
};

//-----------------------------------------------------------------------------

struct ResizePhotoInfo;

class CTaskGenHTMLAlbum : public CTask
{
public:
	CTaskGenHTMLAlbum(VectPhotoInfo& selected, CWnd* parent) : selected_(selected), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& selected_;
	CWnd* parent_;

	bool Generate(CWnd* parent);
	std::string GenerateTable(const std::string& a_template, const std::string& img_template, int grid_cols,
		const std::vector<ResizePhotoInfo>& photos, Path pathThumbs, Path pathPhotos, int item_text_type,
		bool img_text_items[], String& path_to_first_img, String main_page);
	std::string FindText(const std::string& a_template, const char* start, const char* end, std::string& before, std::string& after);
//	std::string RemoveText(const std::string& a_template, const char* start, const char* end);

	void Replace(std::string& text, const char* key, const std::wstring& value);
	void Replace(std::string& text, const char* key, const std::string& value);
	void Replace(std::string& text, const char* key, int value);

	std::string WStringToUTF8(const std::wstring& value);
	std::string WStringToUTF8(const std::string& value)	{ return value; }
};

//-----------------------------------------------------------------------------

#include "Columns.h"

class CTaskExportEXIF
{
public:
	CTaskExportEXIF(VectPhotoInfo& selected, const std::vector<uint16>& sel_columns, const std::vector<int>& col_order, bool all, Columns& columns)
		: selected_(selected), sel_columns_(sel_columns), col_order_(col_order), all_(all), columns_(columns)
	{}

	bool Go(CWnd* parent);

private:
	VectPhotoInfo& selected_;
	const std::vector<int> col_order_;
	const std::vector<uint16> sel_columns_;
	Columns& columns_;
	bool all_;
};

//-----------------------------------------------------------------------------

class CTaskRotatePhotos : public CTask
{
public:
	CTaskRotatePhotos(VectPhotoInfo& selected, bool all, CWnd* parent)
		: selected_(selected), all_(all), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& selected_;
	bool all_;
	CWnd* parent_;

	void RotatePhotos(RotationTransformation transform, bool mirror);
};

//-----------------------------------------------------------------------------

class CTaskPrint : public CTask
{
public:
	CTaskPrint(VectPhotoInfo& selected, const TCHAR* folder_path)
		: selected_(selected), folder_path_(folder_path)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& selected_;
	String folder_path_;
};

//-----------------------------------------------------------------------------

class CTaskPrintThumbnails : public CTask
{
public:
	CTaskPrintThumbnails(VectPhotoInfo& selected, const TCHAR* folder_path)
		: selected_(selected), folder_path_(folder_path)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& selected_;
	String folder_path_;
};

//-----------------------------------------------------------------------------

typedef std::vector<std::pair<VectPhotoInfo::iterator, VectPhotoInfo::iterator>> VectPhotoRanges;


class CTaskCopyTagged : public CTask
{
public:
	CTaskCopyTagged(VectPhotoRanges& tagged, CWnd* parent)
		: tagged_photos_(tagged), parent_(parent)
	{}

	virtual bool Go();

	static String TagNameToFolderName(const String& tag);

private:
	VectPhotoRanges& tagged_photos_;
	CWnd* parent_;
	void CopyTaggedPhotos(VectPhotoRanges& tagged_photos, Path dest_folder, bool create_separate_folders);
};

//-----------------------------------------------------------------------------
#if 0
class CTaskEditDescription : public CTask
{
public:
	typedef boost::function<void (PhotoInfo& photo)> PhotoChangedFn;

	CTaskEditDescription(CWnd* parent, PhotoInfo& photo, VectPhotoInfo& photos,
		VectPhotoInfo& selected, PhotoChangedFn fn= 0)
		: parent_(parent), photo_(photo), photos_(photos), selected_(selected), photo_changed_(fn)
	{}

	virtual bool Go();

private:
	PhotoInfo& photo_;
	VectPhotoInfo& photos_;
	VectPhotoInfo& selected_;
	CWnd* parent_;
	PhotoChangedFn photo_changed_;
};
#endif
//-----------------------------------------------------------------------------

class CTaskImgManipulation : public CTask
{
public:
	CTaskImgManipulation(CWnd* parent, VectPhotoInfo& photos)
		: parent_(parent), photos_(photos)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& photos_;
	CWnd* parent_;
};

//-----------------------------------------------------------------------------

class CTaskDateTimeAdjustment : public CTask
{
public:
	CTaskDateTimeAdjustment(CWnd* parent, VectPhotoInfo& photos)
		: parent_(parent), photos_(photos)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& photos_;
	CWnd* parent_;
};

//-----------------------------------------------------------------------------

class CTaskSendEMail : public CTask
{
public:
	CTaskSendEMail(VectPhotoInfo& photos, CWnd* parent) : photos_(photos), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& photos_;
	CWnd* parent_;

	bool Send();
};

//-----------------------------------------------------------------------------

class CTaskExtractJpegs : public CTask
{
public:
	CTaskExtractJpegs(VectPhotoInfo& photos, CWnd* parent) : photos_(photos), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& photos_;
	CWnd* parent_;

	bool Extract();
};

//-----------------------------------------------------------------------------

class CTaskImageProcessor : public CTask
{
public:
	CTaskImageProcessor(VectPhotoInfo& photos, CWnd* parent) : photos_(photos), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& photos_;
	CWnd* parent_;

	bool Process();
};

//-----------------------------------------------------------------------------

class CTaskImageRename : public CTask
{
public:
	CTaskImageRename(VectPhotoInfo& photos, CWnd* parent) : photos_(photos), parent_(parent)
	{}

	virtual bool Go();

private:
	VectPhotoInfo& photos_;
	CWnd* parent_;

	bool Rename();
};

//-----------------------------------------------------------------------------

// change tag;
// NOTE: photos are passed by value to operate on a copy when notifications are sent!
extern bool ApplyTagToPhotos(VectPhotoInfo photos, String tag, bool apply, CWnd* parent);

// change rating
// NOTE: photos are passed by value to operate on a copy when notifications are sent!
extern bool ApplyRatingToPhotos(VectPhotoInfo photos, int rating, CWnd* parent);


#endif // _tasks_h_
