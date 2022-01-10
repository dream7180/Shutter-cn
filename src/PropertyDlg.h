/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "resource.h"
#include "DialogChild.h"
//#include "IPTCRecord.h"
#include "XmpData.h"
#include <boost/function.hpp>
class PhotoInfo;


class CPropertyDlg : public DialogChild
{
// Construction
public:
	enum Action { Cancel, Save, Previous, Next, Load, SaveTemplate, LoadTemplate, HasPrevious, HasNext };

	typedef boost::function<bool (CPropertyDlg* dlg, Action action, PhotoInfoPtr photo, const XmpData& data, bool modified, int index)> Callback;
	typedef boost::function<void (CMenu* popup, int cmd_id)> InitMenuCallback;

	CPropertyDlg(const Callback& callback, const InitMenuCallback& init_menu, CWnd* parent, size_t count_of_images,
		bool enable_next_prev, const TCHAR* registry_section);

	// next photo loaded, XmpData changed; refresh dialog with new data
	void PhotoLoaded(PhotoInfoPtr photo, const XmpData& data);

	// populate dialog with new XMP data
	void SetXmp(const XmpData& data, bool clear_modified_flag);

	void EndDialog(int code);

	void SaveHistory();

	virtual ~CPropertyDlg();

// Implementation
protected:
	enum { IDD = IDD_IPTC };
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	BOOL OnEraseBkgnd(CDC* dc);
	void OnSize(UINT type, int cx, int cy);
	void OnDestroy();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	void OnNext();
	void OnPrev();
	void OnSaveTemplate();
	void OnLoadTemplate();
	void OnLoadTemplateFile(UINT id);
	void onTemplatePopup();
	//afx_msg void OnUpdateTmpOptions(CCmdUI* cmd_ui);
	//void OnTbDropDown(CWnd* parent, int cmd, size_t btn_index);
	DECLARE_MESSAGE_MAP()

private:
	virtual void OnCancel();
	bool InitDlg();

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};
