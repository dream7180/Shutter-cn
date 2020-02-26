/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CatalogFile.h"
#include "Database/Database.h"
#include "CatalogImgRecord.h"
#include "StringConversions.h"


static int const CATALOG_DB_VERSION= 1;


int CatalogFile::GetSupportedVersion() const
{
	return CATALOG_DB_VERSION;
}


struct CatalogFile::Impl
{
	Impl() : catalog_(false)
	{
		cur_record_.Clear();
	}

	void Open(const String& path)
	{
		try
		{
			if (!catalog_.Open(path, CATALOG_DB_VERSION, true))
				throw String(_T("Cannot open catalog file: ") + path);

			std::vector<uint8> header;
			if (!catalog_.ReadNext(header))
				throw String(_T("Cannot read catalog header: ") + path);

			header_.Read(header);
		}
		catch (int)
		{
			throw String(_T("Error reading catalog file: " + path));
		}
	}

	bool ReadNext(uint64& offset)
	{
		try
		{
			offset = 0;

			cur_record_.Clear();

			if (!catalog_.IsOpen())
				return false;

			std::vector<uint8> record;
			if (!catalog_.ReadNext(record, &offset))
				return false;

			cur_record_.Construct(record);
		}
		catch (int)
		{
			throw String(_T("Error reading catalog record"));
		}

		return true;
	}

	bool Read(uint64 offset, CatalogImgRecord& img)
	{
		try
		{
			img.Clear();

			if (!catalog_.IsOpen())
				return false;

			std::vector<uint8> record;
			catalog_.Read(offset, record);

			img.Construct(record);
		}
		catch (int)
		{
			throw String(_T("Error reading catalog record"));
		}

		return true;
	}

	CatalogHeader header_;
	Database catalog_;
	CatalogImgRecord cur_record_;
	String path_;
};



CatalogFile::CatalogFile(String path) : pImpl_(new Impl)
{
	pImpl_->path_ = path;
	pImpl_->Open(path);
}


CatalogFile::~CatalogFile()
{}


bool CatalogFile::NextRecord(uint64& offset)
{
	return pImpl_->ReadNext(offset);
}


//const CatalogImgRecord& CatalogFile::Image() const
//{
//	return pImpl_->cur_record_;
//}


CatalogHeader::DirPtr CatalogFile::GetRoot() const
{
	return pImpl_->header_.directory_;
}


String CatalogFile::GetTitle() const
{
#ifdef _UNICODE
	return pImpl_->header_.title_;
#else
	string s;
	WideStringToMultiByte(pImpl_->header_.title_, s);
	return s;
#endif
}

String CatalogFile::GetDescription() const
{
#ifdef _UNICODE
	return pImpl_->header_.description_;
#else
	string s;
	WideStringToMultiByte(pImpl_->header_.description_, s);
	return s;
#endif
}

uint32 CatalogFile::GetImageCount() const
{
	return pImpl_->header_.img_count_;
}


bool CatalogFile::ReadRecord(uint64 offset, CatalogImgRecord& img)
{
	return pImpl_->Read(offset, img);
}


String CatalogFile::GetPath() const
{
	return pImpl_->path_;
}
