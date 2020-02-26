/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CopyPhoto.h"
#include "PhotoInfo.h"
#include "PhotoFactory.h"
#include "ExifBlock.h"
#include "FileTypeIndex.h"
#include "PhotoInfoPtr.h"
extern void PostProcessTime(PhotoInfoPtr info);
extern void ReadFileTimeStamp(PhotoInfo& photo, const Path& path);
extern void SetAutoRotationFlag(PhotoInfoPtr photo);


CopyPhoto::CopyPhoto()
{
}

CopyPhoto::~CopyPhoto()
{
}

#if 0

void ReadImage(PhotoInfoPtr photo, const Path& file_path, int64 fileLength, int dirVisited)
{
	photo->SetPhysicalPath(file_path);

	ExifBlock exif;
	bool generateThumbnails= false;
	bool exif= photo->Scan(file_path.c_str(), exif, generateThumbnails);	// scan image to find EXIF data

//bool PhotoInfo::Scan(const TCHAR* filename, const vector<uint8>& exif_data_buffer)

	if (!exif || photo->tm_date_.GetTime() == 0)
	{

		ReadFileTimeStamp(*photo, file_path);
	}

	photo->dir_visited_ = static_cast<uint32>(dirVisited);
	photo->file_size_ = fileLength;
	photo->name_ = file_path.GetFileName();
	photo->exif_data_present_ = exif;

	SetAutoRotationFlag(photo);

	//if (Dib* thumbnail= info->bmp_.get())
	//{
	//	if (!found_in_db)
	//	{
	//		//TODO ConvertRGB2YCbCr()
	//		info->index_.CalcHistogram(*info->bmp_);
	//	}
	//}

	//info->bmp_ = 0;

//	EliminateNonPrintableChars(photo->photo_desc_);

	PostProcessTime(photo);
}


void CopyPhoto::Copy(const Path& file_path, const TCHAR* dest_folder, const TCHAR* rename_pattern)
{
	DWORD start= ::GetTickCount();

	CFile file(file_path.c_str(), CFile::modeRead | CFile::shareDenyWrite);// | CFile::osSequentialScan);

	const uint64 length= file.SeekToEnd();

	if (length == 0)
		return;		// do not create empty copies

	Progress(0, length);	// call progress after opening file

	file.SeekToBegin();

	auto_ptr<PhotoInfo> photo;
	PhotoFactory::CreateFn create= 0;
	int type_id= 0;
	if (GetPhotoFactory().MatchPhotoType(file_path.GetExtension(), create, type_id) && type_id != FT_CATALOG)
		photo.reset(create());

	// if there is 'photo' object available try to decode EXIF block
	if (photo.get())
	{
//		FileStream str;
//		VERIFY(str.Open(buffer));

		ReadImage(photo.get(), file_path, length, 0);
	}

	// create destination name
	Path dest= CreateDestPath(file_path, length, photo.get());

	// copy source file to the destination

	const size_t CHUNK= 0x10000;	// 64 KB
	vector<uint8> buffer(CHUNK, 0);

	size_t block= static_cast<size_t>(min<uint64>(CHUNK, length));

	if (file.Read(&buffer.front(), block) != block)
		throw 11111;

}

#endif
