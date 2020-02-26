/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MakerNote.h: interface for the MakerNote class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAKERNOTE_H__BFAF8506_BD1D_4E28_98AC_10A5EB7BA544__INCLUDED_)
#define AFX_MAKERNOTE_H__BFAF8506_BD1D_4E28_98AC_10A5EB7BA544__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Data.h"
#include "SpecificData.h"
class PhotoInfo;
class OutputStr;


class MakerNote
{
public:
	virtual ~MakerNote();
	virtual String TagName(uint16 tag)= 0;
	virtual String TagValue(uint16 tag, const Data& val)= 0;
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);

	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual int CompareNotes(int index, const MakerNote& note) const = 0;
	virtual bool IsDataPresent(int index) const;
	virtual bool LessNote(int index, const MakerNote& note) const = 0;

	int Compare(int index, const MakerNote& maker_note) const;

	bool Less(int index, const MakerNote& maker_note) const;

	// maker note may contain info not present in EXIF block (like ISO speed); complete
	// any missing values in PhotoInfo based on the maker note info
	virtual void CompletePhotoInfo(PhotoInfo& photo) const;

protected:
	void RecordTag(const TCHAR* name, const TCHAR* value, OutputStr& output);
	void RecordTag(uint32 tag, const TCHAR* name, const TCHAR* value, OutputStr& output);
	void RecordInfo(uint32 tag, const TCHAR* name, const Data& val, OutputStr& output);
};



class NikonNote : public MakerNote
{
public:
	NikonNote(bool new_format= true);

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
	virtual void CompletePhotoInfo(PhotoInfo& photo) const;
private:
	bool new_format_;
	NikonData data_;
	const TCHAR* FlashCompensation(int8 fc);
};



class CanonNote : public MakerNote
{
public:
	CanonNote(const String& model);

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
	virtual void CompletePhotoInfo(PhotoInfo& photo) const;
private:
	void ReportTag1Info(const Data& val, OutputStr& rstrOutput, CanonData& inf);
	void ReportTag4Info(const Data& val, OutputStr& rstrOutput, CanonData& inf);
	void ReportTagFInfo(const Data& val, OutputStr& rstrOutput);
	void ReportTagFInfoEOS10D(const Data& val, OutputStr& output);
	void ReportTagFInfoEOS_1D(const Data& val, OutputStr& output);

	CanonData data_;
	enum Model { GENERIC, EOS_D30_60, EOS_10D, EOS_1D, EOS_1DS, EOS_300D } model_;
};



class FujiNote : public MakerNote
{
public:
	FujiNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
private:
	FujiData data_;
};



class OlympusNote : public MakerNote
{
public:
	OlympusNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
private:
	OlympusData data_;
};



class CasioNote : public MakerNote
{
public:
	CasioNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
private:
	CasioData data_;
};


class Casio2Note : public MakerNote
{
public:
	Casio2Note();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
	virtual void CompletePhotoInfo(PhotoInfo& photo) const;
private:
	Casio2Data data_;
};


class PentaxNote : public MakerNote
{
public:
	PentaxNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;
	virtual void CompletePhotoInfo(PhotoInfo& photo) const;

private:
	int iso_;
};


class SonyNote : public MakerNote
{
public:
	SonyNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;

private:
};


class SanyoNote : public MakerNote
{
public:
	SanyoNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;

private:
};


class PanasonicNote : public MakerNote
{
public:
	PanasonicNote();

	virtual String TagName(uint16 tag);
	virtual String TagValue(uint16 tag, const Data& val);
	virtual void RecordInfo(uint16 tag, const Data& val, OutputStr& output);
	virtual String& GetInfo(int index, String& rstrOut) const;
	virtual bool IsDataPresent(int index) const;
	virtual int CompareNotes(int index, const MakerNote& note) const;
	virtual bool LessNote(int index, const MakerNote& note) const;

private:
};


#endif // !defined(AFX_MAKERNOTE_H__BFAF8506_BD1D_4E28_98AC_10A5EB7BA544__INCLUDED_)
