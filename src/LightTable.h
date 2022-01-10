/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// LightTable.h: interface for the LightTable class.

#pragma once

#include "DockedPane.h"
#include "PhotoInfoStorage.h"
#include "ToolBarWnd.h"
#include "ColorSets.h"
struct ICMTransform;
class PhotoCache;


class LightTable : public DockedPane, PhotoInfoStorageObserver
{
public:
	LightTable(PhotoInfoStorage& storage, PhotoCache* cache);
	virtual ~LightTable();

	bool HasPhotos() const;
	void GetPhotos(VectPhotoInfo& photos);

	bool HasPhoto(ConstPhotoInfoPtr photo) const;

	void OperationsPopup();
	void TagsPopup();

	void EnableToolTips(bool enable);

	void SelectionVisible(ConstPhotoInfoPtr current, bool center_current);

	void ScrollLeft(int speed_up_factor);
	void ScrollRight(int speed_up_factor);

// Operations
	bool Create(CWnd* parent, UINT id, int width, const boost::function<void (PhotoInfoPtr)>& selected_callback,
		const boost::function<PhotoInfoPtr (const TCHAR* path)>& path_to_photo);

	void SetBackgndColor(COLORREF rgb_backgnd);

	void SetGamma(const ICMTransform& trans);

	void AddPhoto(PhotoInfoPtr photo);
	void RemovePhoto(PhotoInfoPtr photo);
	void PhotoModified(PhotoInfoPtr photo);
	void RemoveAll();

	// store light table files in a text file (paths)
	void Store(const TCHAR* storage_file);

	// restore light table contents from a text file
	void Restore(const VectPhotoInfo& photos, const TCHAR* storage_file);

	void Invalidate();

	void SetScreenAspectRatio(float horzRes, float vertRes);

	void SetUIBrightness(double gamma);

	//{{AFX_VIRTUAL(LightTable)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(LightTable)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnUpdateAddToLightTable(CCmdUI* cmd_ui);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static const int LIGHTTABLE_H = 60;
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	void Paint(CDC& dc);
	BOOL OnEraseBkgnd(CDC* dc);

	void Resize();

	// photo delete notification
	virtual void Deleting(PhotoInfoStorage& storage, const VectPhotoInfo& selected);
	virtual void Deleting(PhotoInfoStorage& storage, PhotoInfoPtr photo);

	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);

	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	int header_h;
};
