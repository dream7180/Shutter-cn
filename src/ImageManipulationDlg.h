/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DialogChild.h"
#include "ToolBarWnd.h"
#include "ImgPage.h"
#include "Dib.h"
#include "PreviewCtrl.h"
#include "VectPhotoInfo.h"
#include "ImageInfoDlg.h"
#include "Path.h"
#include "ImageSaveOptions.h"
#include "PhotoCache.h"
#include "ImgBatchModeDlg.h"
#include "Profile.h"
#include <boost/ptr_container/ptr_vector.hpp>


// ImageManipulationDlg dialog

class ImageManipulationDlg : public DialogChild, ImgPageNotifications, PreviewCtrlNotifications
{
public:
	ImageManipulationDlg(CWnd* parent, VectPhotoInfo& photos, PhotoCache* cache);   // standard constructor
	virtual ~ImageManipulationDlg();

// Dialog Data
	enum { IDD = IDD_IMG_MANIPULATION };

private:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	bool InitDialog();
	DECLARE_MESSAGE_MAP()
	virtual void Resize();
	virtual void OnOK();
	void OnDestroy();
	afx_msg void OnSelChangeTab(NMHDR* nmhdr, LRESULT* result);
	bool SelectOutputFileName(const Path& pathPhoto, Path& file_name);
	afx_msg void OnOptions();
	BOOL OnEraseBkgnd(CDC* dc);

	ToolBarWnd tool_bar_wnd_;
	ToolBarWnd next_prev_wnd_;
	CTabCtrl tab_wnd_;
	CPreviewCtrl original_wnd_;
	CPreviewCtrl modified_wnd_;
	CPreviewCtrl crop_wnd_;
	ImageInfoDlg dlg_info_;
	ImageSaveOptions dlg_options_;
	CStatic label_wnd_;
	ImgBatchModeDlg dlg_batch_mode_;
	bool show_batch_mode_params_;

	boost::ptr_vector<ImgPage> tab_dlg_;
	ImgPage* cur_dlg_;

	ImgPage* ShowDlg(ImgPage* dlg);
	void PositionTabDlg(ImgPage* dlg);
	void ShowDlg();
	CImageList image_list_;

	PhotoCache* cache_;
	VectPhotoInfo& photos_;
	Dib dib_original_;
	Dib dib_current_;

	// adjustments can be applied in any order; it is remembered here:
	std::vector<int> effect_stack_;
	void ApplyEffect(size_t index, bool reset);

	virtual void ParamChanged(ImgPage* wnd, bool reset);
	virtual void MouseMoved(CWnd* preview, int x, int y, bool outside);

	Profile<int> profile_dest_folder_sel_;
	Profile<CString> profile_dest_folder_;
	Profile<CString> profile_file_suffix_;
	Profile<int> last_selected_tab_;
};
