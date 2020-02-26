/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>


// FancyToolBar

class FancyToolBar : public CWnd
{
public:
	FancyToolBar();
	virtual ~FancyToolBar();

	struct Params
	{
		Params();

		// fancy toolbar expects images to be provided in full saturation (hot) appearance;
		// from those normal look will be derived by desaturating them and dimming
		float desaturate;	// -1.0..1.0 (de)saturation factor; negative desaturates, positive saturates
		float shade;		// dimming (-1.0..+1.0), negative darkens, positive lightens
		int arrow_down_img_id;
		int string_rsrc_id;
		COLORREF text_color;
		COLORREF hot_text_color;
		COLORREF dis_text_color;
	};

	bool Create(CWnd* parent, const char* pattern, const int* cmdIds, int imgId, const Params* params= 0);

	CSize Size() /*const*/;

	void SetCommandCallback(const boost::function<void (int cmd, size_t btn_index)>& fn);

//	void SetBkgndEraseCallback(const boost::function<void (CDC& dc, CSize size)>& fn);

	CRect GetButtonRect(size_t btn_index);
	CRect GetCmdButtonRect(int cmdId);
	void GetRect(int cmdId, CRect& rect);

	void CheckButton(int cmdId, bool checked);
	void ShowButton(int cmdId, bool visible);
	void EnableButton(int cmdId, bool enabled);
	void PressButton(int cmdId, bool pressed= true);
	void HideButton(int cmdId)						{ ShowButton(cmdId, false); }

	enum Options { BEVEL_LOOK, HOT_OVERLAY, SHIFT_BTN };
	void SetOption(Options opt, bool enable);

	void SetPadding(CRect pad);
	void SetPadding(int cx, int cy);

	void RestoreState(const TCHAR* subKey, const TCHAR* valueName);
	void SaveState(const TCHAR* subKey, const TCHAR* valueName);

	void Customize();

	void ResetToolBar(bool bResizeToFit);

	bool ReplaceImageList(int imgId, Params* params= 0);

	size_t GetButtonCount() const;
	int GetButtonId(size_t index) const;

	void SetOnIdleUpdateState(bool enabled);

protected:
	DECLARE_MESSAGE_MAP()

	virtual CString GetToolTip(int cmdId);

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	void OnPaint();
	BOOL OnEraseBkgnd(CDC* pDC);
	void OnMouseMove(UINT flags, CPoint pos);
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint pos);
	LRESULT OnMouseLeave(WPARAM, LPARAM);
	void OnTimer(UINT_PTR eventId);
	void ValidateLayout(CDC* pDC= 0);
	BOOL OnToolTipGetText(UINT uId, NMHDR* pNmHdr, LRESULT* pResult);
	LRESULT OnCheckMouse(WPARAM, LPARAM);

	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	void OnUpdateCmdUI(CWnd* pTarget, bool bDisableIfNoHndler);
};
