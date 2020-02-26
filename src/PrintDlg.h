/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "DialogChild.h"
#include "ToolBarWnd.h"
#include "PrinterList.h"
#include "PrintPreviewWnd.h"
#include "PrintEngine.h"
#include "PagerCtrl.h"
#include "Profile.h"
#include "GlobalAlloc.h"
#include "PrnPageRangeDlg.h"
#include "PrnThumbsOptionsDlg.h"
#include "PrnPhotoOptionsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// PrintDlg dialog

#undef PrintDlg		// Commdlg.h


class PrintDlg : public DialogChild
{
// Construction
public:
	PrintDlg(VectPhotoInfo& selected, bool thumbnails, const TCHAR* folder_path);

	~PrintDlg();

// Dialog Data
	//{{AFX_DATA(PrintDlg)
	enum { IDD = IDD_PRINT };
	CStatic	page_label_wnd_;
	CStatic	layout_info_wnd_;
	CEdit	edit_margin_;
	CSliderCtrl	margin_slider_wnd_;
	CSpinButtonCtrl	margin_spin_wnd_;
	CListCtrl	type_wnd_;
	CEdit	printers_wnd_;
	ToolBarWnd	pages_wnd_;
	ToolBarWnd	switch_wnd_;
	ToolBarWnd	units_wnd_;
	//}}AFX_DATA
	CString folder_path_;
	CStatic	size_label_wnd_;
	bool profiles_changed_;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PrintDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PrintDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPrinterSettings();
	virtual void OnOK();
	afx_msg void OnPageSetup();
	afx_msg void OnColorSetup();
	afx_msg void OnTypeSelected(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnShowMargins();
	afx_msg void OnUpdateShowMargins(CCmdUI* cmd_ui);
	afx_msg void OnShowPrintArea();
	afx_msg void OnUpdateShowPrintArea(CCmdUI* cmd_ui);
	afx_msg void OnFooter();
	afx_msg void OnDestroy();
	afx_msg void OnChangePagesBox();
	afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnDeltaPosMarginSpin(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnChangeMargin();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual BOOL ContinueModal();
	virtual BOOL IsFrameWnd() const;
	void OnSelChangePrinters();
	void OnChangeFooterText();

private:
	PrintEngine* print_;	// current engine
	std::auto_ptr<PrintEngine> print_photos_;
	std::auto_ptr<PrintEngine> print_thumbnails_;
	PrinterList printers_;
	CImageList img_list_layouts_;
	PrintPreviewWnd preview_wnd_;
	static CPrintDialog dlg_print_;
	CPageSetupDialog dlg_page_setup_;
	VectPhotoInfo& selected_;
	CRect margins_rect_;
	PagerCtrlEx pager_wnd_;
	enum { PAGER_ID= 59999 };
//	GlobalFixAlloc<PRINTDLGEX> print_dlg_ex_;
	bool thumbnails_;		// thumbnails versus photos print dialog
	ToolBarWnd margin_select_wnd_;
	PrnPageRangeDlg dlg_range_;
	PrnThumbsOptionsDlg dlg_thumbs_options_;
	PrnPhotoOptionsDlg dlg_photo_options_;
	bool use_metric_units_for_info_;

	CSize GetPageSize(HGLOBAL dev_mode);
	CRect GetPrintableArea(CDC& print_dc);
	bool UpdatePreviewParams();
	void SetPageToolBar(int page_count);
	void OnSelectPage(UINT cmd);
	void OnUpdateSelectPage(CCmdUI* cmd_ui);
	int GetSelectedType();
	void SelectPrinter(int printer_index);
	void SelectPrinterInCombo(HGLOBAL dev_names);
	void ResetNumberOfPages();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnTbUnitsDown(NMHDR* nmhdr, LRESULT* result);
	void SetTypeLayoutInfo();
	BOOL InitDialog();

	void SelectMarginButton(int index);
	void SetMargin(int index);
	double SetMargin(double val);
	double MarginIndexToAbsValue(int index);
	double GetMargin();
	void MarginChanged();
	void UpdateSliderPos(double val);
	void UpdateSliderPos(int lo_metric);
	int GetSelectedMargin(int selected);
	LRESULT OnItemsNumberChanged(WPARAM items, LPARAM);
	LRESULT OnNumberOfCopiesChanged(WPARAM copies, LPARAM);
	LRESULT OnFontChanged(WPARAM, LPARAM);
	LRESULT OnPrintOptionChanged(WPARAM opt, LPARAM);
	LRESULT OnZoomChanged(WPARAM zoom, LPARAM);
	void UpdatePageAndImageSizes();
	virtual void Resize();
	void OnShowImgSpace();
	void OnUpdateShowImgSpace(CCmdUI* cmd_ui);

	enum Type { FULL_PAGE, TWO_PER_PAGE, THREE_PER_PAGE, FOUR_PER_PAGE, WALLET_PRINTS_LARGE, WALLET_PRINTS, THUMBNAILS };
	void SelectNewType(Type layout);
	Type layout_type_;

	Profile<bool> profile_show_prinatable_area_;
	Profile<bool> profile_show_margins_;
	Profile<bool> profile_show_img_space_;
	Profile<int> profile_layout_type_;
	Profile<int> profile_cur_page_;
	Profile<int> profile_cur_page_thumb_;
	Profile<int> profile_page_range_;
	Profile<bool> profile_print_footer_;
	Profile<bool> profile_print_footer_text_;
	Profile<CString> profile_selected_pages_;
	Profile<int> profile_margin_type_;
	Profile<CRect> profile_margins_;
	Profile<CRect> profile_margins_thumbs_;
	Profile<int> profile_thumbs_items_across_;
	Profile<int> profile_photo_copies_;
	Profile<bool> profile_info_units_;
	Profile<CString> profile_footer_text_;
	Profile<LOGFONT> profile_font_;
	Profile<int> profile_print_option_;
	Profile<int> profile_zoom_;

	enum MarginType { ALL_MARGINS, LEFT_MARGIN, RIGHT_MARGIN, TOP_MARGIN, BOTTOM_MARGIN } selected_margin_;
	enum { RANGE= 100 };
	bool metric_;
};
