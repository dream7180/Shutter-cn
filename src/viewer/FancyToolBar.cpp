/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FancyToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "FancyToolBar.h"
#include "../Dib.h"
#include "../resource.h"	// images
#include "../Block.h"
#include "../Color.h"
using namespace std;

extern bool LoadPingFromRsrc(LPCTSTR pcszResourceId, Dib& bmp);
extern void AlphaBitBlt(const Dib& src, const CRect& src_rect, Dib& dst, const CPoint& dst_pos);
extern void AlphaBitBlt(const Dib& src, const CRect& src_rect, int src_alpha, Dib& dst, const CPoint& dst_pos);
extern void AlphaBitBlt(const Dib& src1, const Dib& src2, const CRect& src_rect, int src_alpha, Dib& dst, const CPoint& dst_pos);
extern void Bevel(Dib& img, int from, int width, Dib& res);
extern void DrawParentBkgnd(CWnd& wnd, CDC& dc);
extern void ModifyHueSaturation(Dib& dib, float saturation, float lightness);
extern void ModifyHueSaturation(Dib& dib, const CRect& rect, float saturation, float lightness);
extern void ModifyHueSaturation(Dib& dib, const CRect& rect, float saturation, float lightness, float contrast);
extern void DrawSeparatorBar(Dib& dest, int x, int y, int height, float black_opacity, float white_opacity);


FancyToolBar::Params::Params()
{
	desaturate = -0.50f;
	shade = -0.10f;

	arrow_down_img_id =	IDR_ARROW_DOWN_IMG;

	string_rsrc_id = 0;

	text_color = RGB(0,0,0);
}


// FancyToolBar


struct Image
{
	Image()
	{
		start_ = end_ = 0;
		offset_x_ = offset_y_ = 0;
	}

	int Width() const	{ return end_ - start_; }
	int Pos() const		{ return start_; }
	int OffsetX() const	{ return offset_x_; }
	int OffsetY() const	{ return offset_y_; }

	void Set(unsigned short from, unsigned short size)
	{
		start_ = from;
		end_ = from + size;
	}

	void SetOffsets(short x, short y)
	{
		offset_x_ = x;
		offset_y_ = y;
	}

private:
	unsigned short start_;	// location in the image stri of btn face
	unsigned short end_;	// width of btn face
	short offset_x_;		// offset for drawing; may be negative
	short offset_y_;
};


struct Button
{
	Button()
	{
		from_ = to_ = 0;
		style_ = PUSHBTN;
		id_ = 0;
		visible = true;
		enabled = true;
		show_text = true;
		arrow_pressed = false;
		pressed = false;
		checked = false;
		hot = false;
		hot_fade_ = 0;
	}

	size_t from_, to_;
	int id_;
	String text_;
	Image img_;
	enum Style { PUSHBTN= 0, DOWNARROW, DOWNMENU, CHECKBTN, SEPARATOR } style_;
	struct	// state
	{
		unsigned visible : 1;
		unsigned enabled : 1;
		unsigned show_text : 1;
		unsigned arrow_pressed : 1;
		unsigned pressed : 1;
		unsigned checked : 1;
		unsigned hot : 1;
	};
	int hot_fade_;
};


struct FancyToolBar::Impl
{
	Impl() : layout_valid_(false), btn_padding_(5, 5, 5, 5), tb_size_(0), hot_btn_index_(~0), timer_id_(0),
		pressed_btn_index_(~0), btn_tracking_(false), hot_tracking_disabled_(false), delayed_stop_btn_tracking_(false)
	{
		use_bevel_look_ = false;
		use_hot_tracing_ = use_btn_shifting_ = true;
		cmd_wnd_ = 0;
		on_idle_update_state_ = false;
		normal_btn_count_ = 0;
		text_color_ = 0;
		hot_text_color_ = 0;
		dis_text_color_ = 0;
		text_space_width_ = 0;
		//check_mouse_pos_ = false;
	}

	bool Create(const char* pattern, const int* cmdIds, int imgId, const FancyToolBar::Params& params);
	void RecalcLayout(CDC& dc, const CRect& rect);
	void AddToolTips(CWnd* wnd);
	int GetBtnTextWidth(CDC& dc, const String& text);
	int ArrowSpace() const		{ return 2 + arrow_down_img_.GetWidth(); }
	static const int SEP_WIDTH= 2;
	static const int SEP_SPACE= 3;
	int SepArrowSpace() const	{ return SEP_SPACE + SEP_WIDTH + 2 + arrow_down_img_.GetWidth(); }
	int SepSpace() const		{ return 6 + SEP_WIDTH; }
	void Paint(Dib& dst);	// paint whole toolbar into the bitmap
	void DrawBackgnd(Dib& bmp, CWnd& wnd, CDC& destDC);
	void DrawButton(Dib& dst, int x, int y, const Button& btn, bool use_hot_tracing, bool use_bevel_look);
	CSize Size() const;		// calc size of whole toolbar
	size_t HitTest(CPoint pos) const;
	bool HotTrack(CPoint pos);
	bool HotButtonFading();
	bool EnterBtnTracking(CPoint pos);
	bool StopBtnTracking(CWnd& wnd, CPoint pos, bool cancel= false);
	bool CheckTouchExit();
	bool PressTrack(CPoint pos);
	CRect GetButtonRect(size_t btn_index);
	void SendCommand(size_t btn_idx, bool arrow_hit= false);
	void SetTimer(CWnd& wnd);
	bool OnTimer(CWnd& wnd);
	void DelayedStopTracking();
	bool CheckButton(int cmdId, bool checked);
	bool ShowButton(int cmdId, bool visible);
	bool EnableButton(int cmdId, bool enabled);
	Button* FindButtonByCmd(int cmdId);
	Button* GetButton(size_t index)		{ return index < btn_.size() ? &btn_[index] : 0; }
	bool ReplaceImageList(int imgId, const FancyToolBar::Params& params);
	void ResizeButtons();

	vector<Button> btn_;
	bool layout_valid_;
	CRect btn_padding_;		// button padding (each side)
	Dib images_;			// normal toolbar buttons images (dim)
	Dib hot_images_;		// toolbar buttons images (bright)
	Dib arrow_down_img_;	// image of down pointing arrow
	size_t tb_size_;		// height (for horz tb) or width (for vert one)
	static const size_t NONE_HIT= ~0;
	size_t hot_btn_index_;
	static const int ALPHA_OPAQUE= 255;
	static const int FADE_OUT= 60;
	UINT_PTR timer_id_;
	size_t pressed_btn_index_;
	bool btn_tracking_;
	bool hot_tracking_disabled_;
	boost::function<void (int cmd, size_t btn_index)> on_command_fn_;
	CWnd* cmd_wnd_;
	bool delayed_stop_btn_tracking_;
	CToolTipCtrl tool_tips_;
	bool use_bevel_look_;
	bool use_hot_tracing_;
	bool use_btn_shifting_;
	static const int DISABLED_ALPHA= 100;
	bool on_idle_update_state_;
	size_t normal_btn_count_;
	COLORREF text_color_;
	COLORREF hot_text_color_;
	COLORREF dis_text_color_;
	int text_space_width_;
	//bool check_mouse_pos_;
};


bool ClearAllHot(vector<Button>& btn)
{
	bool changed= false;
	const size_t count= btn.size();
	for (size_t i= 0; i < count; ++i)
		if (btn[i].hot)
		{
			btn[i].hot = false;
			changed = true;
		}

	return changed;
}


Button* FancyToolBar::Impl::FindButtonByCmd(int cmdId)
{
	const size_t count= btn_.size();

	for (size_t i= 0; i < count; ++i)
		if (btn_[i].id_ == cmdId)
			return &btn_[i];

	return 0;
}


bool FancyToolBar::Impl::EnableButton(int cmdId, bool enabled)
{
	if (Button* btn= FindButtonByCmd(cmdId))
	{
		if (static_cast<bool>(btn->enabled) != enabled)
		{
			btn->enabled = enabled;
			btn->hot = false;
			return true;
		}
		return false;
	}

	ASSERT(false);
	return false;
}


bool FancyToolBar::Impl::ShowButton(int cmdId, bool visible)
{
	if (Button* btn= FindButtonByCmd(cmdId))
	{
		if (static_cast<bool>(btn->visible) != visible)
		{
			btn->visible = visible;
			layout_valid_ = false;
			return true;
		}
		return false;
	}

	ASSERT(false);
	return false;
}


bool FancyToolBar::Impl::CheckButton(int cmdId, bool checked)
{
	if (Button* btn= FindButtonByCmd(cmdId))
	{
		if (static_cast<bool>(btn->checked) != checked)
		{
			btn->checked = checked;
			return true;
		}
		return false;
	}

	ASSERT(false);
	return false;
}


void FancyToolBar::Impl::DelayedStopTracking()
{
	delayed_stop_btn_tracking_ = true;
}


bool FancyToolBar::Impl::OnTimer(CWnd& wnd)
{
	bool changed= false;

	if (HotButtonFading())
		changed = true;
	else if (!delayed_stop_btn_tracking_)
	{
		wnd.KillTimer(timer_id_);
		timer_id_ = 0;
	}

	if (delayed_stop_btn_tracking_)
	{
		delayed_stop_btn_tracking_ = false;
		changed = true;
		StopBtnTracking(wnd, CPoint(-1, -1), true);
	}

	return changed;
}

void FancyToolBar::Impl::SetTimer(CWnd& wnd)
{
	if (timer_id_ == 0)
		timer_id_ = wnd.SetTimer(123, 20, 0);
}


CRect FancyToolBar::Impl::GetButtonRect(size_t btn_index)
{
	ASSERT(layout_valid_);

	if (btn_index < btn_.size())
	{
		int left= static_cast<int>(btn_[btn_index].from_);
		int right= static_cast<int>(btn_[btn_index].to_);
		int bottom= Size().cy;
		return CRect(left, 0, right, bottom);
	}
	else
	{
		ASSERT(false);
		return CRect(0,0,0,0);
	}
}


void FancyToolBar::Impl::SendCommand(size_t btn_idx, bool arrow_hit/*= false*/)
{
	if (btn_idx >= btn_.size())
	{
		ASSERT(false);
		return;
	}

	if (on_command_fn_)
		// send cmd notification
		on_command_fn_(btn_[btn_idx].id_, btn_idx);
	else if (cmd_wnd_)
	{
		if (CWnd* wnd= cmd_wnd_->GetOwner())
		{
			//TODO:!!!
			if (btn_[btn_idx].style_ == Button::DOWNARROW ||
				(btn_[btn_idx].style_ == Button::DOWNMENU && arrow_hit))
			{
				NMTOOLBAR nm;
				memset(&nm, 0, sizeof nm);
				nm.hdr.hwndFrom = cmd_wnd_->GetSafeHwnd();
				nm.hdr.idFrom = cmd_wnd_ ? cmd_wnd_->GetDlgCtrlID() : 0;
				nm.hdr.code = TBN_DROPDOWN;
				nm.iItem = btn_[btn_idx].id_;
				nm.rcButton = GetButtonRect(btn_idx);

				wnd->SendMessage(WM_NOTIFY, nm.hdr.idFrom, LPARAM(&nm));
			}
			else
				wnd->SendMessage(WM_COMMAND, btn_[btn_idx].id_);
		}
	}
}


bool FancyToolBar::Impl::StopBtnTracking(CWnd& wnd, CPoint pos, bool cancel/*= false*/)
{
	if (!btn_tracking_)
		return false;

	size_t btn_index= HitTest(pos);
	bool arrow_hit= false;

	if (pressed_btn_index_ < btn_.size())
	{
		Button& b= btn_[pressed_btn_index_];

		if (b.style_ == Button::DOWNMENU)
			arrow_hit = b.arrow_pressed;

		b.pressed = false;
		b.arrow_pressed = false;
	}

	bool hit= btn_index == pressed_btn_index_;

	pressed_btn_index_ = NONE_HIT;

	if (cancel)
		ClearAllHot(btn_);

	if (!cancel && hit && btn_index < btn_.size())
	{
		wnd.PostMessage(WM_USER);	// check mouse position after executing command (mouse may be moved)
		SendCommand(btn_index, arrow_hit);
	}

	// Impl may be already deleted here!

	return true;
}



bool FancyToolBar::Impl::CheckTouchExit()
{
	if (!btn_tracking_)
		return false;

	if (pressed_btn_index_ < btn_.size() && btn_[pressed_btn_index_].style_ == Button::DOWNARROW)
	{
		{
			Block tracking(hot_tracking_disabled_);
			SendCommand(pressed_btn_index_);
		}
//TRACE(L"cancel btn down\n");
//		StopBtnTracking(CPoint(-1, -1), true);
		return true;
	}

	return false;
}


static bool ArrowHit(const Button& btn, CPoint pos, int arr_width)
{
	return pos.x >= btn.to_ - arr_width && pos.x < btn.to_;
}


bool FancyToolBar::Impl::EnterBtnTracking(CPoint pos)
{
	bool changed= false;
	btn_tracking_ = false;

	size_t hit= HitTest(pos);

	// just in case old btn is still remembered...
	if (pressed_btn_index_ < btn_.size()) // && pressed_btn_index_ == hit)
	{
		btn_[pressed_btn_index_].pressed = false;
		btn_[pressed_btn_index_].arrow_pressed = false;

		if (pressed_btn_index_ == hit)
		{
			// pressed down btn clicked again (this can happen for DOWNARROW buttons)
			pressed_btn_index_ = NONE_HIT;
			return true;
		}
		pressed_btn_index_ = NONE_HIT;
	}

	delayed_stop_btn_tracking_ = false;
	pressed_btn_index_ = hit;

	if (ClearAllHot(btn_))
		changed = true;

	if (pressed_btn_index_ < btn_.size())
	{
		btn_tracking_ = true;

		Button& b= btn_[pressed_btn_index_];
		b.pressed = true;
		b.arrow_pressed = false;
		b.hot = true;

		if (b.style_ == Button::DOWNMENU)	// check arrow click in this type of button
		{
			if (ArrowHit(b, pos, SepArrowSpace()))
			{
				b.pressed = false;
				b.arrow_pressed = true;
			}
		}

		changed = true;
	}
	else
		btn_tracking_ = false;

	return changed;
}


bool FancyToolBar::Impl::PressTrack(CPoint pos)
{
	if (!btn_tracking_)
		return false;

	size_t hit= HitTest(pos);
	bool down= pressed_btn_index_ == hit;

	if (pressed_btn_index_ < btn_.size())
	{
		Button& b= btn_[pressed_btn_index_];

		if (b.style_ == Button::DOWNMENU)	// check arrow click in this type of button
		{
			bool arrow_down= ArrowHit(b, pos, SepArrowSpace());

			if (arrow_down == static_cast<bool>(b.arrow_pressed) &&
				down == static_cast<bool>(b.pressed))
				return false;

			b.arrow_pressed = arrow_down;
			if (arrow_down)
				down = false;
		}
		else if (down == static_cast<bool>(b.pressed))
			return false;

		b.pressed = down;
	}

	return true;
}


bool FancyToolBar::Impl::HotButtonFading()
{
	bool changed= false;

	const size_t count= btn_.size();

	for (size_t i= 0; i < count; ++i)
	{
		if (!btn_[i].hot && btn_[i].hot_fade_ > 0)
		{
			btn_[i].hot_fade_ = max<int>(0, btn_[i].hot_fade_ - FADE_OUT);
			changed = true;
		}
	}

	return changed;
}


bool FancyToolBar::Impl::HotTrack(CPoint pos)
{
	if (hot_tracking_disabled_)
		return false;

	size_t hot= HitTest(pos);
	if (hot_btn_index_ == hot)
		return false;	// no changes
//TRACE(L"hot old: %d  new: %d\n", hot_btn_index_, hot);

	if (hot_btn_index_ < btn_.size())
	{
	//	btn_[hot_btn_index_].hot = false;
	}

	ClearAllHot(btn_);

	hot_btn_index_ = hot;

	if (hot_btn_index_ < btn_.size())
	{
		btn_[hot_btn_index_].hot = true;
		btn_[hot_btn_index_].hot_fade_ = ALPHA_OPAQUE;
	}

	return true;
}


size_t FancyToolBar::Impl::HitTest(CPoint pos) const
{
	if (!layout_valid_ || btn_.empty())
		return NONE_HIT;

	CSize s= Size();
	if (pos.x < 0 || pos.x >= s.cx || pos.y < 0 || pos.y >= s.cy)
		return NONE_HIT;

	const size_t count= btn_.size();
	size_t x= pos.x;

	//TODO: binary search
	for (size_t i= 0; i < count; ++i)
	{
		if (x >= btn_[i].from_ && x < btn_[i].to_)
		{
			if (btn_[i].style_ == Button::SEPARATOR || !btn_[i].enabled)
				return NONE_HIT;

			return i;
		}
	}

	return NONE_HIT;
}


static void SelectFont(CDC& dc)
{
	if (CWnd* wnd= dc.GetWindow())
		if (CFont* font= wnd->GetFont())
		{
			dc.SelectObject(font);
			return;
		}
	LOGFONT lf;
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	//lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("tahoma"));
	CFont _font;
	_font.CreateFontIndirect(&lf);
	dc.SelectObject(&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
}


int FancyToolBar::Impl::GetBtnTextWidth(CDC& dc, const String& text)
{
	SelectFont(dc);
	return dc.GetTextExtent(text.c_str(), static_cast<int>(text.length())).cx;
}


void FancyToolBar::Impl::AddToolTips(CWnd* wnd)
{
	{
		const int count= static_cast<int>(btn_.size());

		for (int i= 0; i < count; ++i)
			tool_tips_.DelTool(wnd, i + 1);
	}

	const size_t count= btn_.size();
	for (size_t i= 0; i < count; ++i)
	{
		const Button& btn= btn_[i];
		if (btn.visible && btn.id_ && btn.style_ != Button::SEPARATOR)
		{
			CRect rect= GetButtonRect(i);
			tool_tips_.AddTool(wnd, LPSTR_TEXTCALLBACK, rect, static_cast<int>(i + 1));
		}
	}
}


void FancyToolBar::Impl::RecalcLayout(CDC& dc, const CRect& rect)
{
	int pos= rect.left;
	const size_t count= btn_.size();

	text_space_width_ = GetBtnTextWidth(dc, _T(" "));

	// limitation: horz layout only

	for (size_t i= 0; i < count; ++i)
	{
		Button& btn= btn_[i];

		btn.from_ = pos;

		if (btn.visible)
		{
			int size= btn.img_.Width();

			if (btn.style_ == Button::SEPARATOR)
			{
				; // get sep. size
				size += SepSpace();
			}
			else
			{
				if (btn.show_text && !btn.text_.empty())
					size += text_space_width_ + GetBtnTextWidth(dc, btn.text_) + text_space_width_;

				if (btn.style_ == Button::DOWNARROW)
					size += ArrowSpace();
				else if (btn.style_ == Button::DOWNMENU)
					size += SepArrowSpace();

				if (size > 0)
					size += btn_padding_.left + btn_padding_.right;
			}

			pos += size;
		}

		btn.to_ = pos;
	}
}


// copy image at (from, width) into dst
void DrawImage(Dib& dst, int x, int y, const Dib& images, int from, int width)
{
	CRect src(CPoint(from, 0), CSize(width, images.GetHeight()));
	AlphaBitBlt(images, src, dst, CPoint(x, y));
}

void DrawImage(Dib& dst, int x, int y, const Dib& images, int from, int width, int alpha)
{
	CRect src(CPoint(from, 0), CSize(width, images.GetHeight()));
	AlphaBitBlt(images, src, alpha, dst, CPoint(x, y));
}

void DrawImage(Dib& dst, int x, int y, const Dib& images1, const Dib& images2, int mix_ratio, int from, int width)
{
	CRect src(CPoint(from, 0), CSize(width, images1.GetHeight()));
	AlphaBitBlt(images1, images2, src, mix_ratio, dst, CPoint(x, y));
}


CPoint PreparePressedBtnImg(Dib& images, int from, int width, Dib& dst)
{
	// apply bevel effect
	Bevel(images, from, width, dst);

	int height= images.GetHeight();

	// resulting image is increased in size to make space for highlight/shadow areas;
	// return relative shift of the original image to the beveled one
	return CPoint((width - dst.GetWidth()) / 2, (height - dst.GetHeight()) / 2);
}


static const float DARK= 0.3f;
static const float BRIGHT= 0.15f;


void FancyToolBar::Impl::DrawButton(Dib& dst, int x, int y, const Button& btn, bool use_hot_tracing, bool use_bevel_look)
{
	if (btn.style_ == Button::SEPARATOR)
	{
		// draw separator

		int sep_width= SEP_WIDTH;
		int space= SepSpace();
		int xpos= x + (space - sep_width) / 2 + btn.img_.OffsetX();
		int ypos= y + btn_padding_.top + btn.img_.OffsetY();

		DrawSeparatorBar(dst, xpos, ypos, images_.GetHeight() + btn_padding_.bottom / 2, DARK, BRIGHT);

		return;
	}

	bool draw_hot= btn.hot || btn.hot_fade_ > 0;
	bool draw_arrow= btn.style_ == Button::DOWNARROW; // || btn.style_ == Button::DOWNMENU;
	bool separate_arrow= btn.style_ == Button::DOWNMENU;

	const CSize size(btn.to_ - btn.from_, Size().cy);
	const CRect btn_rect(CPoint(x, y), size);

	bool draw_text= btn.show_text && !btn.text_.empty();

	CDC textDC;

	if (draw_text)
	{
		textDC.CreateCompatibleDC(0);
		textDC.SelectObject(dst.GetBmp());
		SelectFont(textDC);
	}

	if (draw_hot && use_hot_tracing || btn.checked)
	{
		// modify background of hot tracked and/or checked button

		const float hot_sat= 0.12f;
		const float hot_light= 0.15f;
		const float hot_ctr= 1.20f;

		const float check_sat= -0.03f;
		const float check_light= 0.16f;
		const float check_ctr= 1.60f;

		float saturation= 0.0f;
		float lightness= 0.0f;
		float contrast= 1.0f;

		CRect rect= btn_rect;

		if (btn.checked)
		{
			saturation = check_sat;
			lightness = check_light;
			contrast = check_ctr;

			if (!btn.hot)
				rect.DeflateRect(1, 1, 1, 0);	// narrower to prevent from touching neighbor buttons
		}

		if (draw_hot && use_hot_tracing)
		{
			if (!btn.hot)	// fading?
			{
				float factor= static_cast<float>(btn.hot_fade_) / ALPHA_OPAQUE;

				saturation = hot_sat * factor + (1.0f - factor) * saturation;
				lightness = hot_light * factor + (1.0f - factor) * lightness;
				contrast = hot_ctr * factor + (1.0f - factor) * contrast;
			}
			else
			{
				saturation = hot_sat;
				lightness = hot_light;
				contrast = hot_ctr;
			}
		}

		ModifyHueSaturation(dst, rect, saturation, lightness, contrast);
	}

	int x_arr= 0;
	int y_arr= 0;
	int xpos= x + btn_padding_.left + btn.img_.OffsetX();
	int ypos= y + btn_padding_.top + btn.img_.OffsetY();

	if (separate_arrow)
	{
		int sep_width= SEP_WIDTH;
		int space= SepSpace();
		int x_sep= xpos + btn.img_.Width() + SEP_SPACE;
		int y_sep= y;
		int h= tb_size_ + btn_padding_.top + btn_padding_.bottom;

		// separator between btn image and arrow image (for hot buttons only)
		if (btn.hot)
			DrawSeparatorBar(dst, x_sep, y_sep, h, DARK, BRIGHT);

		int arr_height= arrow_down_img_.GetHeight();

		x_arr = x_sep + sep_width + SEP_SPACE;
		y_arr = ypos + (static_cast<int>(tb_size_) - arr_height) / 2;

		draw_arrow = true;
	}
	else if (draw_arrow)
	{
		// draw down pointing arrow

		int arr_height= arrow_down_img_.GetHeight();

		x_arr = xpos + btn.img_.Width() + 2;
		y_arr = ypos + (static_cast<int>(tb_size_) - arr_height) / 2;

		if (draw_text)
		{
			int w= textDC.GetTextExtent(btn.text_.c_str(), static_cast<int>(btn.text_.length())).cx;
			x_arr += w + 5;
		}
	}

	if (btn.pressed || btn.arrow_pressed)
	{
		if (use_btn_shifting_)
		{
			++x_arr; ++y_arr;

			if (btn.pressed)
			{
				++xpos; ++ypos;	// shift pressed btn down and to the left
			}
		}

		if (use_bevel_look)
		{
			if (btn.pressed)
			{
				Dib img;
				CPoint shift= PreparePressedBtnImg(images_, btn.img_.Pos(), btn.img_.Width(), img);
				DrawImage(dst, xpos + shift.x, ypos + shift.y, img, 0, img.GetWidth());
			}

			if (draw_arrow)
			{
				Dib img;
				CPoint shift= PreparePressedBtnImg(arrow_down_img_, 0, arrow_down_img_.GetWidth(), img);
				DrawImage(dst, x_arr + shift.x, y_arr + shift.y, img, 0, img.GetWidth());
			}
		}
	}

	if (draw_hot)
	{
		if (btn.hot)
			DrawImage(dst, xpos, ypos, hot_images_, btn.img_.Pos(), btn.img_.Width());
		else if (use_hot_tracing)
		{
			// fading hot btn
			DrawImage(dst, xpos, ypos, images_, hot_images_, btn.hot_fade_, btn.img_.Pos(), btn.img_.Width());
		}
		else
			DrawImage(dst, xpos, ypos, images_, btn.img_.Pos(), btn.img_.Width());
	}
	else
	{
		if (!btn.enabled)
			DrawImage(dst, xpos, ypos, images_, btn.img_.Pos(), btn.img_.Width(), DISABLED_ALPHA);
		else
			DrawImage(dst, xpos, ypos, images_, btn.img_.Pos(), btn.img_.Width());
	}

	if (draw_arrow)
	{
		// draw down pointing arrow
		DrawImage(dst, x_arr, y_arr, arrow_down_img_, 0, arrow_down_img_.GetWidth());
	}

	if (draw_text)
	{
		//CDC dc;
		//dc.CreateCompatibleDC(0);
		//dc.SelectObject(dst.GetBmp());
		//SelectFont(dc);

		textDC.SetTextColor(btn.hot ? hot_text_color_ : (btn.enabled ? text_color_ : dis_text_color_));
		textDC.SetBkMode(TRANSPARENT);
//		textDC.TextOut(xpos + btn.img_.Width() + text_space_width_, ypos, btn.text_.c_str(), static_cast<int>(btn.text_.length()));
		CRect r(CPoint(xpos + btn.img_.Width() + text_space_width_, ypos), CSize(btn.to_ - btn.from_, tb_size_));
		textDC.DrawText(btn.text_.c_str(), static_cast<int>(btn.text_.length()), r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

//	return static_cast<int>(btn.to_ - btn.from_);
}


void FancyToolBar::Impl::Paint(Dib& dst)	// paint entire toolbar into the bitmap
{
	ASSERT(layout_valid_);

	const size_t count= btn_.size();

	int x= 0, y= 0;

	for (size_t i= 0; i < count; ++i)
	{
		const Button& btn= btn_[i];

		if (!btn.visible)
			continue;

		DrawButton(dst, static_cast<int>(btn.from_) + x, y, btn, use_hot_tracing_, use_bevel_look_);
	}
}


void FancyToolBar::Impl::DrawBackgnd(Dib& bmp, CWnd& wnd, CDC& destDC)
{
	ASSERT(layout_valid_);

	CDC dc;
	dc.CreateCompatibleDC(0);

	dc.SelectObject(bmp.GetBmp());

	::DrawParentBkgnd(wnd, dc);
}


CSize FancyToolBar::Impl::Size() const		// calc size of the whole toolbar
{
	if (!layout_valid_)
	{
		ASSERT(false);
		return CSize(0, 0);
	}

	if (btn_.empty())
		return CSize(0, 0);

	int width= static_cast<int>(btn_.back().to_);
	int height= static_cast<int>(tb_size_ + btn_padding_.top + btn_padding_.bottom);

	return CSize(width, height);
}


bool FancyToolBar::Impl::ReplaceImageList(int imgId, const FancyToolBar::Params& params)
{
	Dib hot;
	if (!::LoadPingFromRsrc(MAKEINTRESOURCE(imgId), hot))
	{
		ASSERT(false);
		return false;
	}

	Dib img;
	// clone hot images
	img.Clone(hot);
	// and desaturate them
	::ModifyHueSaturation(img, params.desaturate, params.shade);

	hot_images_.Swap(hot);
	images_.Swap(img);

	ResizeButtons();

	tb_size_ = images_.GetHeight();

	layout_valid_ = false;

	return true;
}


bool FancyToolBar::Impl::Create(const char* pattern, const int* cmdIds, int imgId, const FancyToolBar::Params& params)
{
	layout_valid_ = false;
	tb_size_ = 0;
	hot_btn_index_ = NONE_HIT;
	pressed_btn_index_ = NONE_HIT;

	if (!::LoadPingFromRsrc(MAKEINTRESOURCE(imgId), hot_images_))
	{
		ASSERT(false);
		return false;
	}

	// clone hot images
	images_.Clone(hot_images_);
	// and desaturate them
	::ModifyHueSaturation(images_, params.desaturate, params.shade);

	::LoadPingFromRsrc(MAKEINTRESOURCE(params.arrow_down_img_id), arrow_down_img_);

	CString labels;
	if (params.string_rsrc_id)
		labels.LoadString(params.string_rsrc_id);

	hot_text_color_ = params.hot_text_color;
	text_color_ = params.text_color; //CalcNewColorDelta(params.text_color, params.desaturate, params.shade);
	dis_text_color_ = params.dis_text_color;

	const char SEP= '|';	// btn separator

	// count how many buttons is defined by the pattern

	size_t count= 0;
	normal_btn_count_ = 0;
	for (const char* p= pattern; *p; ++p, ++count)
		if (*p != SEP)
			++normal_btn_count_;

//	const unsigned short img_width= static_cast<unsigned short>(normal_btn_count_ > 0 ? images_.GetWidth() / normal_btn_count_ : 0);

	const int* cmd= cmdIds;

	btn_.resize(count);

//	unsigned short img_start= 0;
	int substr= 0;

	for (size_t i= 0; i < count; ++i)
	{
		Button& btn= btn_[i];

		if (pattern[i] == SEP)	// separator?
		{
			btn.style_ = Button::SEPARATOR;
			continue;
		}

		btn.id_ = *cmd++;
//		btn.img_.Set(img_start, img_width);

//		img_start += img_width;

//		btn.text_;	TODO

		char pat= pattern[i];
		if (pat >= 'A' && pat <= 'Z')
		{
			pat += 'a' - 'A';

			// add text label

			CString str;
			AfxExtractSubString(str, labels, substr++);

			btn.text_ = str;
		}

		switch (pat)
		{
		case 'p':	btn.style_ = Button::PUSHBTN;	break;
		case 'v':	btn.style_ = Button::DOWNARROW;	break;
		case 'm':	btn.style_ = Button::DOWNMENU;	break;
		case 'x':	btn.style_ = Button::CHECKBTN;	break;

			// no-button case:
		case '.':
			btn.style_ = Button::PUSHBTN;
			btn.visible = false;
			break;
		}
	}

	ResizeButtons();

	tb_size_ = images_.GetHeight();

	return true;
}


void FancyToolBar::Impl::ResizeButtons()
{
	const size_t count= btn_.size();

	if (count == 0)
		return;

	const unsigned short img_width= static_cast<unsigned short>(normal_btn_count_ > 0 ? images_.GetWidth() / normal_btn_count_ : 0);

	unsigned short img_start= 0;

	for (size_t i= 0; i < count; ++i)
	{
		Button& btn= btn_[i];

		if (btn.style_ == Button::SEPARATOR)
		{
			btn.img_.Set(0, SEP_WIDTH);
			btn.img_.SetOffsets(1, -2);
		}
		else
		{
			btn.img_.Set(img_start, img_width);
			img_start += img_width;

	//		btn.text_;	TODO

			switch (btn.style_)
			{
			case Button::PUSHBTN:	break;
			case Button::DOWNARROW:	break;	// arrow down
			case Button::DOWNMENU:	break;	// separator + arrow down
			case Button::CHECKBTN:	break;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////


FancyToolBar::FancyToolBar() : pImpl_(new Impl())
{
}

FancyToolBar::~FancyToolBar()
{
}


BEGIN_MESSAGE_MAP(FancyToolBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_TIMER()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipGetText)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_USER, OnCheckMouse)
END_MESSAGE_MAP()


bool FancyToolBar::Create(CWnd* parent, const char* pattern, const int* cmdIds, int imgId, const FancyToolBar::Params* params)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	if (!pImpl_->tool_tips_.Create(this, WS_POPUP))
		return false;

	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.hwnd   = *this;
	ti.uId    = UINT_PTR(ti.hwnd);
	ti.lpszText = const_cast<TCHAR*>(_T(""));
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.rect   = CRect(0,0,0,0);
	ti.hinst  = NULL;
	ti.lParam = 0;
	pImpl_->tool_tips_.SendMessage(TTM_ADDTOOL, 0, LPARAM(&ti));

	pImpl_->cmd_wnd_ = this;

	return pImpl_->Create(pattern, cmdIds, imgId, params ? *params : Params());
}


void FancyToolBar::ValidateLayout(CDC* pDC)
{
	if (!pImpl_->layout_valid_)
	{
		CRect rect(0,0,0,0);
		if (pDC == 0)
		{
			CClientDC dc(this);
			pImpl_->RecalcLayout(dc, rect);
		}
		else
			pImpl_->RecalcLayout(*pDC, rect);

		pImpl_->layout_valid_ = true;

		pImpl_->AddToolTips(this);
	}
}


CSize FancyToolBar::Size() //const
{
	ValidateLayout();
	return pImpl_->Size();
}


CRect FancyToolBar::GetButtonRect(size_t btn_index)
{
	ValidateLayout();
	return pImpl_->GetButtonRect(btn_index);
}


void FancyToolBar::OnPaint()
{
	CPaintDC dc(this);
}


BOOL FancyToolBar::OnEraseBkgnd(CDC* pDC)
{
	try
	{
		ValidateLayout(pDC);

		CSize s= pImpl_->Size();
		Dib bmp(s.cx, s.cy, 32);

		pImpl_->DrawBackgnd(bmp, *this, *pDC);

		pImpl_->Paint(bmp);

		bmp.Draw(pDC, CPoint(0, 0));

		// erase area to the right of the toolbar
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		int extra_space= rect.Width() - s.cx;
		if (extra_space > 0)
		{
			Dib bmp(extra_space, s.cy, 32);
			pImpl_->DrawBackgnd(bmp, *this, *pDC);
			bmp.Draw(pDC, CPoint(s.cx, 0));
		}
	}
	catch (...)
	{}

	return true;
}


void FancyToolBar::SetCommandCallback(const boost::function<void (int cmd, size_t btn_index)>& fn)
{
	pImpl_->on_command_fn_ = fn;
}


//void FancyToolBar::SetBkgndEraseCallback(const boost::function<void (CDC& dc, CSize size)>& fn)
//{
//	pImpl_->erase_bkgnd_fn_ = fn;
//}


void FancyToolBar::OnMouseMove(UINT flags, CPoint pos)
{
	if (flags & MK_LBUTTON)
	{
		if (pImpl_->PressTrack(pos))
			RedrawWindow();
	}
	else
	{
		if (pImpl_->HotTrack(pos))
			RedrawWindow();

		TRACKMOUSEEVENT et;
		et.cbSize = sizeof et;
		et.dwFlags = TME_LEAVE;
		et.hwndTrack = m_hWnd;
		et.dwHoverTime = 0;

		_TrackMouseEvent(&et);

		pImpl_->SetTimer(*this);
	}
}


void FancyToolBar::OnLButtonDown(UINT flags, CPoint pos)
{
	SetCapture();

	if (pImpl_->EnterBtnTracking(pos))
	{
		RedrawWindow();

		if (pImpl_->CheckTouchExit())
		{
			CPoint pos(0, 0);
			GetCursorPos(&pos);
			ScreenToClient(&pos);
			pImpl_->HotTrack(pos);
			RedrawWindow();
			ReleaseCapture();
			pImpl_->SetTimer(*this);
			pImpl_->DelayedStopTracking();
		}
	}
}


void FancyToolBar::OnLButtonUp(UINT flags, CPoint pos)
{
	ReleaseCapture();

	Invalidate();

	pImpl_->StopBtnTracking(*this, pos);

	// at this point 'this' may be deleted
}


LRESULT FancyToolBar::OnMouseLeave(WPARAM, LPARAM)
{
	if (pImpl_->HotTrack(CPoint(-1, -1)))
		RedrawWindow();

	pImpl_->SetTimer(*this);

	return 0;
}


void FancyToolBar::OnTimer(UINT_PTR eventId)
{
	if (eventId != pImpl_->timer_id_)
	{
		CWnd::OnTimer(eventId);
		return;
	}

	if (pImpl_->OnTimer(*this))
		RedrawWindow();
}


void FancyToolBar::CheckButton(int cmdId, bool checked)
{
	if (pImpl_->CheckButton(cmdId, checked))
		Invalidate();
}


void FancyToolBar::ShowButton(int cmdId, bool visible)
{
	if (pImpl_->ShowButton(cmdId, visible))
		Invalidate();
}


void FancyToolBar::EnableButton(int cmdId, bool enabled)
{
	if (pImpl_->EnableButton(cmdId, enabled))
		Invalidate();
}


CString FancyToolBar::GetToolTip(int cmdId)
{
	CString str;
	str.LoadString(cmdId);
	return str;
}


BOOL FancyToolBar::OnToolTipGetText(UINT id, NMHDR* pNmHdr, LRESULT* pResult)
{
	NMTTDISPINFO* pTTT= reinterpret_cast<NMTTDISPINFO*>(pNmHdr);

	static CString buf;

	if (Button* btn= pImpl_->GetButton(pTTT->hdr.idFrom - 1))
	{
		buf = GetToolTip(btn->id_);
		const TCHAR* p= buf;
		pTTT->lpszText = const_cast<TCHAR*>(p);
	}

	pTTT->szText[0] = 0;
	pTTT->hinst = NULL;

	*pResult = 0;
	return TRUE;
}


void FancyToolBar::SetOption(Options opt, bool enable)
{
	switch (opt)
	{
	case BEVEL_LOOK:	pImpl_->use_bevel_look_ = enable;	break;
	case HOT_OVERLAY:	pImpl_->use_hot_tracing_ = enable;	break;
	case SHIFT_BTN:		pImpl_->use_btn_shifting_ = enable;	break;
	}
}


void FancyToolBar::SetPadding(int cx, int cy)
{
	SetPadding(CRect(cx, cy, cx, cy));
}


void FancyToolBar::SetPadding(CRect pad)
{
	pImpl_->btn_padding_ = pad;
	pImpl_->layout_valid_ = false;
}


void FancyToolBar::RestoreState(const TCHAR* subKey, const TCHAR* valueName)
{}


void FancyToolBar::SaveState(const TCHAR* subKey, const TCHAR* valueName)
{}


void FancyToolBar::Customize()
{}


void FancyToolBar::ResetToolBar(bool bResizeToFit)
{}


bool FancyToolBar::ReplaceImageList(int imgId, Params* params/*= 0*/)
{
	return pImpl_->ReplaceImageList(imgId, params ? *params : Params());
}

///////////////////////////////////////////////////////////////////////////////

LRESULT FancyToolBar::OnCheckMouse(WPARAM, LPARAM)
{
	CPoint pos(0, 0);
	::GetCursorPos(&pos);
	ScreenToClient(&pos);
	OnMouseMove(0, pos);
	return 0;
}


LRESULT FancyToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	if (!pImpl_->on_idle_update_state_)
		return 0L;

	// the toolbar must be visible
	if ((GetStyle() & WS_VISIBLE))
	{
		//CFrameWnd* pTarget = static_cast<CFrameWnd*>(GetOwner());
		CWnd* pTarget = static_cast<CWnd*>(GetOwner());
		if (pTarget == NULL)// || !pTarget->IsFrameWnd())
			pTarget = GetParentFrame();
		if (pTarget != NULL)
			OnUpdateCmdUI(pTarget, !!wParam);
	}
	return 0L;
}


namespace {

// CToolBar idle update through FxCToolCmdUI class

class FxCToolCmdUI : public CCmdUI        // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void FxCToolCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	if (FancyToolBar* pToolBar= dynamic_cast<FancyToolBar*>(m_pOther))
	{
//		ASSERT(m_nIndex < m_nIndexMax);
//		if (!bOn)
//			pToolBar->PressButton(m_nID, false);	// unpress btn when disabled
		pToolBar->EnableButton(m_nID, !!bOn);
	}
}

void FxCToolCmdUI::SetCheck(int nCheck)
{
	ASSERT(nCheck >= 0 && nCheck <= 2); // 0=>off, 1=>on, 2=>indeterminate
	if (FancyToolBar* pToolBar = dynamic_cast<FancyToolBar*>(m_pOther))
	{
		pToolBar->CheckButton(m_nID, nCheck != 0);
	}
//	ASSERT(pToolBar != NULL);
//	dynamiASSERT_KINDOF(FancyToolBar, pToolBar);
//	ASSERT(m_nIndex < m_nIndexMax);

	//if (pToolBar->m_nShiftImageForCheckedBtn != 0 && pToolBar->m_nShiftImageForCheckedBtn == m_nID)
	//{
	//	TBBUTTONINFO tbbi;
	//	tbbi.cbSize = sizeof tbbi;
	//	tbbi.dwMask = TBIF_IMAGE;
	//	tbbi.iImage = nCheck;
	//	pToolBar->SetButtonInfo(m_nID, &tbbi);
	//}
	//else
	//{
	//	TBBUTTONINFO tbbi;
	//	tbbi.cbSize = sizeof tbbi;
	//	tbbi.dwMask = TBIF_STYLE;
	//	// do not set checked state for normal button (non-check buttons)
	//	pToolBar->GetButtonInfo(m_nID, &tbbi);
	//	if ((tbbi.fsStyle & BTNS_CHECK) == 0)
	//		return;
	//	pToolBar->CheckButton(m_nID, nCheck);
	//}
}

void FxCToolCmdUI::SetText(LPCTSTR)
{
	// ignore it
}

} // namespace


void FancyToolBar::OnUpdateCmdUI(CWnd* pTarget, bool bDisableIfNoHndler)
{
	FxCToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = GetButtonCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		// get buttons state
//		TBBUTTON button;
//		GetButton(state.m_nIndex, &button);
//		_GetButton(state.m_nIndex, &button);
		state.m_nID = GetButtonId(state.m_nIndex);

		// ignore separators
		if (state.m_nID != 0)
		{
			// allow reflections
//			if (CWnd::OnCmdMsg(0,
//				MAKELONG((int)CN_UPDATE_COMMAND_UI, WM_COMMAND+WM_REFLECT_BASE),
//				&state, NULL))
//				continue;

			// allow the toolbar itself to have update handlers
//			if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
//				continue;

			// allow the owner to process the update
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

///////////////////////////////////////////////////////////////////////////////

size_t FancyToolBar::GetButtonCount() const
{
	return pImpl_->btn_.size();
}


int FancyToolBar::GetButtonId(size_t index) const
{
	if (index < pImpl_->btn_.size())
		return pImpl_->btn_[index].id_;

	ASSERT(false);
	return 0;
}


void FancyToolBar::SetOnIdleUpdateState(bool enabled)
{
	pImpl_->on_idle_update_state_ = enabled;
}


void FancyToolBar::PressButton(int cmdId, bool pressed)
{
}

void FancyToolBar::GetRect(int cmdId, CRect& rect)
{
	if (Button* btn= pImpl_->FindButtonByCmd(cmdId))
	{
		rect = pImpl_->GetButtonRect(btn - &pImpl_->btn_.front());
	}
	else
	{
		ASSERT(false);
	}
}


CRect FancyToolBar::GetCmdButtonRect(int cmdId)
{
	CRect rect(0,0,0,0);
	GetRect(cmdId, rect);
	return rect;
}
