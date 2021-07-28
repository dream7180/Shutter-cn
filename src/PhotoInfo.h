/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/


#ifndef _photo_info_h
#define _photo_info_h

#include "AutoPtr.h"
#include "SpecificData.h"
#include "Rational.h"
#include "Dib.h"
#include "GPSData.h"
#include "OutputStr.h"
#include "MakerNote.h"
#include "Index.h"
#include "PhotoTags.h"
#include "Path.h"
#include "IPTCRecord.h"
#include "Jpeg.h"
#include "ImageDecoder.h"
#include "ImageStat.h"
#include "ThumbnailSize.h"
#include "ColorProfileForward.h"
#include "transform.h"
#include "XmpData.h"
#include "ExifTags.h"
#include "ExifInfo.h"
#include "DecoderProgress.h"
#include <boost/function/function1.hpp>
struct ExifBlock;
class ImageDatabase;
class ImgLogger;


class PhotoInfo
#ifdef PHOTO_INFO_SMART_PTR
	: public mik::counter_base
#endif
{
public:
	PhotoInfo();
	virtual ~PhotoInfo();

	typedef boost::function<HANDLE (const TCHAR* path)> WriteAccessFn;

	// is rotation feasible?
	virtual bool IsRotationFeasible() const;
	// returns true if photo can be opened for writing
	virtual bool IsFileEditable() const;
	// can edit IPTC info?
	virtual bool CanEditIPTC(int& err_code) const;
	// return instance of decoder
	virtual CImageDecoderPtr GetDecoder() const;
	// create thumbnail image
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	// save tags inside photo
	virtual void SaveTags(const WriteAccessFn& get_write_access);
	// load saved tags
	virtual void LoadTags();
	// rotate photo
	virtual int RotatePhoto(RotationTransformation transform, bool mirror, CWnd* parent);
	// returns true if 'lossless' JPEG crop is supported
	virtual bool IsLosslessJPEGCropPossible() const;
	// extract EXIF info
	virtual bool ReadExifBlock(ExifBlock& exif) const;
	// return the name of a file type
	virtual String GetFileTypeName() const;
	// get physical path
	virtual const Path& GetPhysicalPath() const;
	// display only path
	virtual Path GetDisplayPath() const;
	// save/load metadata
	virtual void SaveMetadata(const XmpData& xmp, const WriteAccessFn& get_write_access) const;
	virtual bool LoadMetadata(XmpData& xmp) const;
	// asign XMP metadata
	void SetMetadata(const XmpData& xmp);
	// get metadata
	bool GetMetadata(XmpData& xmp) const;
	// true if XMP record is present
	bool HasMetadata() const;
	const XmpData* GetMetadata() const;
	// save image rating into the file info XMP struct
	virtual void SaveRating(int rating, const WriteAccessFn& get_write_access);

	Path GetOriginalPath() const;
	void SetPhysicalPath(const String& path);

	// some photos have associated files (like *.XMP metadata, *.THM thumbnail preview with EXIF data)
	virtual Path GetAssociatedFilePath(size_t index) const;

	// scan photo, store entire EXIF block in the exif_data; if generateThumbnails is true build thumbnail
	// from the main image, otherwise use embedded thumbnail image (if available)
	virtual bool Scan(const TCHAR* filename, ExifBlock& exif_data, bool generate_thumbnails, ImgLogger* logger);
	// scan EXIF block from 'exif_data_buffer' memory
	bool Scan(const TCHAR* filename, const std::vector<uint8>& exif_data_buffer, ImgLogger* logger);

	// this is size of an image; if it's rotated (non-permanent rotation) size will reflect that
	CSize GetSize() const;

	// if photo has an embedded preview image in a JPEG format, return offset (in a file) to that preview and its size
	virtual bool GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const;

	void SetPhotoName(const TCHAR* name);
	void SetPhotoName(const String& name);
	void SetExposureTime(Rational et);
	void SetFStop(Rational fn);
	void SetExposureBias(SRational eb);
	void SetExposureProgram(uint16 ep);
	void SetFileSize(uint64 size);
	void SetMakerNote(AutoPtr<MakerNote> maker_note);
	void SetMake(const String& model);
	void SetModel(const String& model);
	void SetMeteringMode(uint16 mm);
	void SetLightSource(uint16 ls);
	void SetFlash(uint16 f);
	void SetFocalLength(Rational fl);
	void SetShutterSpeedValue(SRational ssv);
	void SetApertureValue(Rational av);
	void SetSubjectDistance(Rational sd);
	void SetExifDescription(const std::wstring& desc);
	void SetGpsData(AutoPtr<GPSData> gps);
	void SetVisitedDirId(uint32 dir);
	void SetLensModel(const String& lens);

	//------------------------- file
	const String&	GetName() const;
	String			GetNameAndExt() const;
	uint64			GetFileSize() const;

	Rational		GetExposureTime() const;
	SRational		GetExposureBias() const;
	Rational		GetFStop() const;
	uint32			GetWidth() const;
	uint32			GetHeight() const;
	Rational		GetFocalLength() const;
	double			GetFieldOfViewCrop() const;	// preconfigured value (like 1.5 for Nikon D70)
	uint16			GetExposureProgram() const;
	const String&	GetMake() const;
	const String&	GetModel() const;
	uint16			GetMeteringMode() const;
	uint16			GetLightSource() const;
	uint16			GetFlash() const;
	const std::wstring& GetExifDescription() const;
	uint32			GetVisitedDirId() const;
	const GPSData*	GetGpsData() const;
	Rational		GetSubjectDistance() const;
	const std::wstring& PhotoDescription() const;
	String			GetLensModel() const;

	void		SetWidth(uint32 w);
	void		SetHeight(uint32 h);
	void		SetSize(uint32 w, uint32 h);

private:
	uint32		dir_visited_;
	uint64		file_size_;
	//------------------------- EXIF
	Rational	exposure_time_;
	Rational	f_number_;
	uint16		exposure_program_;
	SRational	exposure_bias_;
	uint16		metering_mode_;
	uint16		light_source_;
	uint16		flash_;
	Rational	focal_length_;
	Rational	aperture_value_;
	SRational	shutter_speed_value_;
	std::wstring description_;		// EXIF_USER_DESC (tag 0x9286)
	Rational	subject_distance_;
	String		lens_model_;
	//------------------------- extra EXIF data
	AutoPtr<MakerNote>	maker_note_;
	AutoPtr<GPSData>	gps_data_;
	//------------------------- embedded ICC Profile
	ColorProfilePtr icc_profile_;

	DateTime	date_time_;			// photo date/time accurate to ms (thanks to EXIF_SubSecTimeOriginal field)
	int			subsecond_;			// field used to store value of EXIF_SubSecTimeOriginal field
	uint32		date_stamp_;		// this is date in days: year * 12 * 31 + month * 31 + day used for sorting
	uint32		width_;
	uint32		height_;
	String		name_;				// file name extracted from the path (sans extension)
	uint16		iso_speed_;			// ISO speed used or (ISO_SPEED_UNKNOWN)
									// even Auto ISO means that *some* ISO speed was used, but when it's not known
									// then (ISO_SPEED_AUTO) may be recorded (if MakerNote data indicates so)
	uint16		color_space_;		// color space value from EXIF block
	uint32		thumbnail_orientation_;
	uint16		orientation_;
	int			cached_effective_orientation_;
	int CalcPhotoOrientation() const;
protected:
	String		make_;
	String		model_;
	bool		is_raw_;

public:
	void		SetThumbnailOrientation(uint16 o);
	void		SetOrientation(uint16 o);
	uint16		ThumbnailOrientation() const;
	uint16		OrientationField() const;

	bool		HasColorProfile() const;
	bool		HasEmbeddedColorProfile() const;
	ColorProfilePtr GetColorProfile() const;
	void		AssignColorProfile(ColorProfilePtr icc);

	//------------------------- synthetic info
	AutoPtr<Dib> bmp_;				// this is not used for photos apart from options dlg and temporarily in ImgScanner (TODO: remove)
	CJpeg		jpeg_thumb_;		// original thumbnail in JPEG format
	bool		portrait_;
	bool		no_std_thumbnail_;
	bool		exif_data_present_;
	ExifInfo	exif_info_;
	//TODO: refactor
	uint32		jpeg_offset_;		// offset to JPEG image (used for Canon Raw)
	uint32		jpeg_size_;			// size in bytes of embedded JPEG image
	//
	bool		lossless_crop_;		// true if 'lossless' JPEG crop is possible
	//-------------------------
	std::wstring photo_desc_;		// Unicode photo description stored in MARK_APP6 (if no APP6 it may come from JPEG COM marker)
	ImageIndex	index_;
	PhotoTags	tags_;
	ImageStat	thumbnail_stat_;	// status of reading thumbnail image
	ImageStat	photo_stat_;		// status of reading whole photo
	enum RotationFlag
	{ RF_INTACT= 0, RF_90CW= 1, RF_UPDN= 2, RF_90CCW= 3, RF_MASK= 0x03, RF_AUTO= 4 };
	uint32		rotation_flag_;		// current rotation state (if user rotated display in a viewer or auto rotated in a viewer)
	//-------------------------
	void SetISOSpeed(uint32 iso);
	void SetAutoISOSpeed();
	void SetColorSpace(uint16 val);
	//-------------------------
	String ExposureTime() const;
	String FNumber() const;
	String ExposureProgram() const;
	String ExposureBias() const;
	String MeteringMode() const;
	String LightSource() const;
	String Flash() const;
	String FocalLength() const;
	String FocalLengthInt() const;
	String FocalLength35mm() const;
	String ISOSpeed() const;
	uint16 GetISOSpeed() const;
	bool IsISOSpeedValid() const;
	String Orientation() const;
	String Size() const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;
	String FileSize() const;
	String Width() const;
	String Height() const;
	double GetFocalLength35mm() const;
	const MakerNote* GetMakerNote() const;

	// image orientation as stored in the EXIF block
	enum ImgOrientation { ORIENT_NO_INFO, ORIENT_UNKNOWN, ORIENT_NORMAL, ORIENT_90CW, ORIENT_90CCW, ORIENT_UPSDN };
	ImgOrientation GetOrientation() const;

	// if true photo was rotated using ExifPro
	bool OrientationAltered() const;

	// sync rotation flag after photo rotation
	void PhotoRotated(bool b90CW);

	// return true if photo is oriented horizontally (landscape) and false if it's in portrait mode
	bool HorizontalOrientation() const;
	int GetPhotoOrientation() const			{ return cached_effective_orientation_; }
	int GetThumbnailOrientation() const;
	bool GetThumbnailMirrorFlip() const;

	void OrientThumbnail(Dib& bmp) const;
	void RotateThumbnail(bool clockwise, bool mirror);

	double ExposureTimeValue() const;
	double FNumberValue() const;

	String ToolTipInfo(bool path= false) const;
	String ToolTipInfo(const std::vector<uint16> selected_fields) const;

	// sets photo description and writes it to a photo
	//bool WriteDescription(const std::wstring& text, CWnd* parent);

	//void DescriptionAscii(string& rstrText);
	void Description(String& text) const;

	// draw thumbnail (bmp_) preserving aspect ratio
	enum { DRAW_FAST= 0, DRAW_HALFTONE= 1, DRAW_DIBDRAW= 2, DRAW_BACKGND= 4, DRAW_SHADOW= 8,
		DRAW_SELECTION= 0x10, DRAW_OUTLINE= 0x20, DRAW_WITH_ICM= 0x40 };
	void Draw(CDC* dc, CRect& dest_rect, COLORREF rgb_back, COLORREF rgb_selection= RGB(0,0,0),
		COLORREF rgb_outline= RGB(0,0,0), COLORREF rgb_text= RGB(0,0,0), COLORREF rgb_text_bk= RGB(0,0,0),
		UINT flags= DRAW_DIBDRAW | DRAW_BACKGND, const String* label= 0);

	String DateTimeStr() const;

	void SetExifInfo(Offset exif_offset, uint32 exif_size, Offset ifd_offset, bool big_endian);

	bool IsExifDataPresent() const		{ return exif_data_present_; }

	PhotoTags& GetTags()				{ return tags_; }
	const PhotoTags& GetTags() const	{ return tags_; }
	bool FindTag(const String& tag)		{ return tags_.FindTag(tag); }

	int GetRating() const				{ return image_rate_; }

	int GetFileTypeIndex() const		{ return file_type_index_; }

	static CSize GetThumbnailSize()		{ return CSize(STD_THUMBNAIL_WIDTH, STD_THUMBNAIL_HEIGHT); }

	// return pointer to temporary dib with thumbnail image
	Dib* GetThumbnail(CSize dest_size= CSize(0, 0));
	void PutThumbnail(Dib* thumbnail, CSize size);

	Dib* IsThumbnailAvailable(CSize dest_size) const;

	// calc size of bitmap (bmp_size) resized uniformly to fit into given area (dest_size) and return result in photo_size
	bool FitToSize(CSize bmp_size, CSize dest_size, CSize& photo_size);

	static void DrawErrIcon(CDC* dc, CRect& dest_rect);

	void SetRecordOffset(uint64 offset)	{ record_offset_ = offset; }

	DateTime GetDateTime() const;
	void SetDateTime(const DateTime& t);

	void SetSubSeconds(int t); // fraction of the second (1/100 s units) offset for time recorded in EXIF data
	int GetSubSeconds() const;

	uint32 GetDateStamp() const;

	bool CanDelete() const;
	bool CanRename() const;

	// no-throw rename function
	void Rename(Path& path, String& name) throw();

	//------------------------- IPTC info
	void SetIPTCInfo(const IPTCRecord& iptc);	// store IPTC in the XMP record (overwrites XMP!)
	bool GetIPTCInfo(IPTCRecord& iptc) const;	// set IPTC record based on the internal XMP metadata

	bool IsSRGB() const;
	bool IsAdobeRGB() const;

	bool IsRaw() const;					// return true for raw formats

protected:
	Path	path_;
	int		file_type_index_;			// index to the Config file types vector
	uint64	record_offset_;				// offset to the record in an image database
	bool	can_delete_;				// can this image be deleted?
	bool	can_rename_;				// file can be renamed?
	std::auto_ptr<XmpData> xmp_;		// metadata
	int		image_rate_;				// copy of XMP field 'xap:Rating'

	static CImageList img_list_errors_;	// no image error status
	bool ReadJpegThumbnail(ImageDatabase& db, CJpeg& jpg) const;

private:
	PhotoInfo(const PhotoInfo& src) { *this = src; }
	PhotoInfo& operator = (const PhotoInfo& inf);

	static void Draw(Dib* bmp, CDC* dc, CRect& dest_rect, COLORREF rgb_back, COLORREF rgb_selection, COLORREF rgb_outline,
		COLORREF rgb_text, COLORREF rgb_text_bk, UINT flags, const String* pstrLabel);

	// debug facilities: simple flag to set when object is no longer supposed to be used
#ifdef _DEBUG
	bool object_is_deleted_;
public:
	bool IsDeleted() const	{ return object_is_deleted_; }
	void SetDeleted()		{ ASSERT(!object_is_deleted_); object_is_deleted_ = true; }
#endif
};


#ifdef _DEBUG
inline bool IsPointerValid(PhotoInfo* p)
{
	if (p == 0)
		return true;
	if (p->IsDeleted())
		return false;
	return true;
}
#endif

#endif	// _photo_info_h
