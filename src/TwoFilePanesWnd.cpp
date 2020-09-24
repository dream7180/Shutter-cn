/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TwoFilePanesWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TwoFilePanesWnd.h"
#include "FilePaneWnd.h"
#include "Color.h"
#include "EnableCtrl.h"

extern void DrawSeparatorRivets(CDC& dc, CRect rect, bool horizontal);
extern void DrawGrip(CDC* dc, const CRect& rect, COLORREF rgb_light, COLORREF rgb_dark);


struct TwoFilePanesWnd::Impl
{
	FilePaneWnd left_;
	FilePaneWnd right_;
	bool resizing_;
	double pane_split_;
	int leftWidth_;
	int rightWidth_;
	CPoint startPoint_;
	int initialWidth_;

	Impl();
	~Impl();

	void Init(CWnd* parent);
	void Resize(CWnd* parent);
	void Resizing(CWnd* parent, int leftWidth);
	void SetCursor(CWnd* parent);
	bool InSepArea(CWnd* parent);
	void AdjustMinWidth(CRect& left, CRect& right);
	bool LeftPaneVisible();
};



TwoFilePanesWnd::TwoFilePanesWnd(CWnd* parent /*=NULL*/)
	: CDialog(IDD_TWO_FILE_PANES, parent), pImpl_(new Impl)
{
}

TwoFilePanesWnd::~TwoFilePanesWnd()
{
}


void TwoFilePanesWnd::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(TwoFilePanesWnd, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()



TwoFilePanesWnd::Impl::Impl()
{
	resizing_ = false;
	pane_split_ = 0.5;
	leftWidth_ = rightWidth_ = 0;
	startPoint_.x = startPoint_.y = 0;
	initialWidth_ = 0;
}


TwoFilePanesWnd::Impl::~Impl()
{
}


bool TwoFilePanesWnd::Create(CWnd* parent, double paneSplit)
{
	if (paneSplit > 0.0 && paneSplit < 1.0)
		pImpl_->pane_split_ = paneSplit;
	else
	{ ASSERT(false); }	// bogus split ratio

	if (!CDialog::Create(IDD_TWO_FILE_PANES, parent))
		return false;

	return true;
}


BOOL TwoFilePanesWnd::OnInitDialog()
{
	CDialog::OnInitDialog();

	pImpl_->Init(this);

	return true;
}


void TwoFilePanesWnd::Impl::Init(CWnd* parent)
{
	left_.Create(parent, _T("从:"), false, true, true);
	right_.Create(parent, _T("到:"), true, false, true);

	Resize(parent);
}


const int SEP= 6;


BOOL TwoFilePanesWnd::OnEraseBkgnd(CDC* dc)
{
	if (dc)
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);

		COLORREF color= ::GetSysColor(COLOR_3DFACE);
		dc->FillSolidRect(rect, color);

		if (pImpl_->LeftPaneVisible())
		{
			int x= pImpl_->leftWidth_ + 1;
			CRect grip(x, rect.top, x, rect.bottom);

			DrawSeparatorRivets(*dc, grip, false);
/*
			CRect grip(0, 0, 2, 2);
			int x= pImpl_->leftWidth_ + 1;
			grip.OffsetRect(x, (rect.bottom - rect.top) / 2 - 20);

			COLORREF rgb_light= CalcShade(color, -30.0f);
			COLORREF rgb_dark= CalcShade(color, 80.0f);

			for (int i= 0; i < 8; ++i)
			{
				DrawGrip(dc, grip, rgb_light, rgb_dark);
				grip.OffsetRect(3, 3);
				DrawGrip(dc, grip, rgb_light, rgb_dark);
				grip.OffsetRect(-3, 3);
			}
			DrawGrip(dc, grip, rgb_light, rgb_dark); */
		}
	}

	return true;
}


void TwoFilePanesWnd::Impl::Resize(CWnd* parent)
{
	if (left_.m_hWnd == 0 || right_.m_hWnd == 0)
		return;

	CRect rect(0,0,0,0);
	parent->GetClientRect(rect);

	if (rect.Width() <= SEP || rect.Height() <= 0)
		return;

	if (LeftPaneVisible())
	{
		CRect left= rect;
		CRect right= rect;

		ASSERT(pane_split_ > 0.0 && pane_split_ < 1.0);
		left.right = right.left = static_cast<int>(pane_split_ * rect.Width() + 0.5);
		right.left += SEP;

		AdjustMinWidth(left, right);

		leftWidth_ = left.Width();
		rightWidth_ = right.Width();

		left_.MoveWindow(left);
		right_.MoveWindow(right);
	}
	else
		right_.MoveWindow(rect);	// left wnd is hidden
}


bool TwoFilePanesWnd::Impl::LeftPaneVisible()
{
	return (left_.GetStyle() & WS_VISIBLE) != 0;
}


void TwoFilePanesWnd::Impl::Resizing(CWnd* parent, int leftWidth)
{
	if (left_.m_hWnd == 0 || right_.m_hWnd == 0)
		return;

	CRect rect(0,0,0,0);
	parent->GetClientRect(rect);

	if (rect.Width() <= SEP || rect.Height() <= 0)
		return;

	CRect left= rect;
	CRect right= rect;

	right.left = left.right = left.left + leftWidth;
	right.left += SEP;

	AdjustMinWidth(left, right);

	int w= rect.Width();
	if (w > 0)
		pane_split_ = left.Width() / double(w);

	leftWidth_ = left.Width();
	rightWidth_ = right.Width();

	left_.MoveWindow(left);
	right_.MoveWindow(right);

	parent->Invalidate();
	parent->UpdateWindow();

	left_.UpdateWindow();
	right_.UpdateWindow();
}


void TwoFilePanesWnd::Impl::AdjustMinWidth(CRect& left, CRect& right)
{
	// risky assumption about min width that fits in a current window
	if (left.Width() < left_.GetMinWidth())
	{
		right.left = left.right = left.left + left_.GetMinWidth();
		right.left += SEP;
	}
	else if (right.Width() < right_.GetMinWidth())
	{
		left.right = right.left = right.right - right_.GetMinWidth();
		left.right -= SEP;
	}
}


void TwoFilePanesWnd::OnSize(UINT, int cx, int cy)
{
	Invalidate();
	pImpl_->Resize(this);
}


void TwoFilePanesWnd::OnOK()
{}

void TwoFilePanesWnd::OnCancel()
{
	if (CWnd* wnd= GetParent())
		wnd->SendMessage(WM_COMMAND, IDCANCEL, 0);
}


bool TwoFilePanesWnd::Impl::InSepArea(CWnd* parent)
{
	CRect rect(0,0,0,0);
	parent->GetClientRect(rect);

	CPoint pos(0, 0);
	::GetCursorPos(&pos);
	parent->ScreenToClient(&pos);

	return rect.PtInRect(pos) && pos.x >= leftWidth_ && pos.x <= leftWidth_ + SEP;
}


void TwoFilePanesWnd::OnLButtonDown(UINT flags, CPoint pos)
{
	if (pImpl_->InSepArea(this) && pImpl_->LeftPaneVisible())
	{
		SetCapture();
		pImpl_->startPoint_ = pos;
		pImpl_->resizing_ = true;
		pImpl_->initialWidth_ = pImpl_->leftWidth_;
		pImpl_->SetCursor(this);
	}
}


void TwoFilePanesWnd::OnLButtonUp(UINT flags, CPoint pos)
{
	if (pImpl_->resizing_)
	{
		ReleaseCapture();
		pImpl_->resizing_ = false;
		pImpl_->SetCursor(this);
	}
}


void TwoFilePanesWnd::OnMouseMove(UINT flags, CPoint pos)
{
	if ((flags & MK_LBUTTON) && pImpl_->resizing_)
	{
		CSize delta= pos - pImpl_->startPoint_;
		pImpl_->Resizing(this, pImpl_->initialWidth_ + delta.cx);
	}
}


BOOL TwoFilePanesWnd::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		pImpl_->SetCursor(this);
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}


void TwoFilePanesWnd::Impl::SetCursor(CWnd* parent)
{
	if (resizing_ || InSepArea(parent))
		::SetCursor(AfxGetApp()->LoadCursor(IDC_HORZ_RESIZE));
	else
		::SetCursor(::LoadCursor(0, IDC_ARROW));
}


FilePaneWnd& TwoFilePanesWnd::GetLeftPane()
{
	return pImpl_->left_;
}


FilePaneWnd& TwoFilePanesWnd::GetRightPane()
{
	return pImpl_->right_;
}


void TwoFilePanesWnd::ShowLeftPane(bool show)
{
	EnableCtrl(&pImpl_->left_, show);

	Invalidate();
	pImpl_->Resize(this);
}


double TwoFilePanesWnd::GetPaneSplitRatio() const
{
	return pImpl_->pane_split_;
}
