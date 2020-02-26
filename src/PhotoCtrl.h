/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(_PHOTOCTRL_H_INCLUDED_)
#define _PHOTOCTRL_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

/////////////////////////////////////////////////////////////////////////////
// PhotoCtrl.h : header file
//
// PhotoCtrl window
//

#include "PhotoInfo.h"
#include "PhotoInfoStorage.h"
#include "HorzReBar.h"
#include "CoolScrollBar.h"
class MemoryDC;
class PhotoCtrlNotification;



class PhotoCtrl : public CWnd
{
// Construction
public:
	PhotoCtrl();

// Attributes
public:
	COLORREF GetBkColor();

	int GetItemCount() const			{ return groups_.GetItemCount(); }

	CHeaderCtrl* GetHeaderCtrl()		{ return &header_wnd_; }

	// get/set columns order
	void GetColumnOrderArray(std::vector<int>& col_order);
	void SetColumnOrderArray(const std::vector<int>& col_order);
	// get column width
	int GetColumnWidth(int col) const;

	enum Icons { NO_ICON= -1, FILM_ROLL= 0, LABEL, SIMILARITY, BOOK, STAR, CALENDAR };

	int GetSortColumn() const			{ return sort_by_column_; }
	void SetSortColumn(int col);

	int GetSelectedCount() const		{ return select_.selected_count_; }

	void GetSelectedItems(VectPhotoInfo& selected)	{ CollectSelected(selected); }

	PhotoInfoPtr GetCurrentItem() const	{ return current_item_ ? current_item_.Item()->photo_ : 0; }
	void SetCurrentItem(PhotoInfoPtr photo, bool select_current, bool scroll= true, bool notify= false);

	bool ClickAndSelectFlag() const		{ return select_.click_and_select_; }
	void ClickAndSelect(bool select)	{ select_.click_and_select_ = select; }

	bool StickySelectionFlag() const	{ return sticky_selection_; }
	void StickySelection(bool sticky)	{ sticky_selection_ = sticky; }

	CRect GetItemRect(PhotoInfoPtr photo, bool bounding_box) const;

	// current image size
	CSize GetImageSize(PhotoInfoPtr photo= 0) const;

	// set/get all colors
	void SetColors(const std::vector<COLORREF>& colors);
	void GetColors(std::vector<COLORREF>& colors) const;
	// reset colors based on system colors and hardcoded values
	void ResetColors();

	// color indices (in a vector of colors for Get/SetColor)
	enum Colors { C_BACKGND= 0, C_TEXT, C_SELECTION, C_SEL_TEXT, C_DISABLED_TEXT, C_TAG_BACKGND, C_TAG_TEXT, C_SORT, C_SEPARATOR, C_DIM_TEXT, C_EDIT_BACKGND, C_MAX_COLORS };

	bool GetHalftoneDrawing() const		{ return halftone_drawing_; }
	void SetHalftoneDrawing(bool on);

	bool ShowingItemLabel() const		{ return show_label_; }
	void ShowItemLabel(bool show);

	bool IsBalloonInfoEnabled() const	{ return ballon_info_enabled_; }
	void EnableBalloonInfo(bool enable);

	bool ShowingTagText() const			{ return show_tags_; }
	void ShowTagText(bool show);

	void SetItemsAcross(int items);

	bool IsItemVisible(PhotoInfoPtr photo) const;

	// get requested item
	enum ItemGet { FIRST_VISIBLE, LAST_VISIBLE, NEXT_ITEM, PREV_ITEM };
	PhotoInfoPtr GetItem(ItemGet get, PhotoInfoPtr item= 0);

	PhotoInfoPtr GetFirstItem(int groupId);

	// function to call to block internal updates when before inserting items
	void EnableUpdate(bool enable);

	// if there is no current item PhotoCtrl will set it to the first item being added;
	// call this method to enable/disable this behavior
	void EnableSettingCurrentItem(bool enable);

	void SetTagFont(const LOGFONT& font);

	bool HasGroup(int group_id) const;

	// returns true if given item is present in photo control
	bool HasItem(PhotoInfoPtr item) const;

// Operations
public:
	bool Create(CWnd* parent, PhotoCtrlNotification* host, UINT id);

	// different display modes:
	//
	// THUMBNAILS
	// DETAILS - tiny preview; rows and columns with many fields
	// TILES - preview + few lines of info on the side
	// PREVIEWS - big preview images (decoded on the fly)
	// LIST - tiny preview and a label; very tight display
	// MOSAIC - not completely implemented; tightly packed images, image wall
	enum Mode { THUMBNAILS= 1, DETAILS, TILES, PREVIEWS, LIST, MOSAIC };
	void ChangeMode(Mode mode);

	void RemoveAll();
	void RemoveSelected();
	// remove all groups (but not items)
	void RemoveGroups();
	// remove one item (if 'notify' is true CurrentItemChanged will be called)
	bool RemoveItem(PhotoInfoPtr photo, bool notify);

	// add items creating the new group (if necessary): this is update/replace operation!
//NOTE: if group exists items are not added, but *replace* current group's items (this to be fixed)
	void AddItems(VectPhotoInfo& photos, const String& group_name, Icons group_icon, int group_id);
	void AddItems(VectPhotoInfo::iterator begin, VectPhotoInfo::iterator end, const String& group_name, Icons group_icon, int group_id);
	void AddItems(PhotoInfoPtr* begin, PhotoInfoPtr* end, const String& group_name, Icons group_icon, int group_id, bool);
	// add items to the existing group
//	void AddItems(VectPhotoInfo& photos, int group_id);
	// add items; create one hidden group
	//void AddItems(VectPhotoInfo& photos);

	// insert new item into the group
	void InsertItem(PhotoInfoPtr photo, size_t index, const String& group_name, Icons group_icon, int group_id);

	// move item from one group into another
	void MoveItem(PhotoInfoPtr photo, const String& group_name, Icons group_icon, int group_id);

	// selecting images
	enum Selection { ALL, NONE, INVERT, TOGGLE_CURRENT };
	bool SelectItems(Selection sel, bool notify= true);
	bool SelectItem(PhotoInfoPtr photo, bool notify= true);
	bool SelectItems(const VectPhotoInfo& photos, bool notify= true);

	// set new image size for thumbnail view mode
	void SetImageSize(int width);

	// insert/delete column in header ctrl
	int InsertColumn(int col_index, const TCHAR* column_heading, int format, int width, int sub_item= -1);
	void DeleteColumn(int col_index);
	void UpdateColumnHeading(int col_index, const TCHAR* heading);

	// set width of column in detailed view
	void SetColumnWidth(int col, int width);

	// redraw given item
	void RedrawItem(PhotoInfoPtr photo);

	// when item's orientation changes location of all items is to be reset
	void ResetItemsLocation();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* msg);

// Implementation
public:
	virtual ~PhotoCtrl();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnRButtonDown(UINT flags, CPoint point);
	afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT flags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	afx_msg void OnTimer(UINT_PTR id_event);
	afx_msg void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	afx_msg void OnKeyUp(UINT chr, UINT rep_cnt, UINT flags);
	afx_msg void OnSetFocus(CWnd* old_wnd);
	afx_msg void OnKillFocus(CWnd* new_wnd);
	afx_msg void OnChar(UINT chr, UINT rep_cnt, UINT flags);

	DECLARE_MESSAGE_MAP()

private:
	void OnMouseButtonDown(UINT flags, CPoint pos, bool left);
	void OnMouseButtonUp(UINT flags, CPoint pos, bool left);
	void StopSelecting(bool cancel, bool select_current, bool extend_sel_key, bool toggle_sel_key);

	class Group;

	//=============================================================================================
	class Item
	{
	public:
		Item(PhotoInfoPtr photo) : photo_(photo), group_(nullptr)
		{ ASSERT(photo); }

		Item(const Item& item) : photo_(item.photo_), group_(nullptr), flags_(item.flags_)
		{}

		Item& operator = (const Item& item)
		{
			photo_ = item.photo_;
			flags_ = item.flags_;
			group_ = item.group_;
			return *this;
		}

		// draw single item at given location using attributes from 'ctrl'
		void DrawItem(CDC& dc, CRect rect, PhotoCtrl& ctrl, bool is_current, bool show_marker, bool no_exif_marker);
		// draw 'no EXIF' indicator in the right bottom corner of rect
		void DrawNoExifIndicator(CDC& dc, CRect rect);
		// draw marker using 'img_list_indicators_'
		void DrawTypeIndicator(CDC& dc, CRect rect, int index);

		bool IsSelected() const				{ return !!flags_.selected; }
		bool IsTempSelected() const			{ return !!flags_.temp_selected; }

		bool SetSelection(bool selected)
		{
			bool change= static_cast<char>(selected) != flags_.selected;
			flags_.temp_selected = flags_.selected = selected ? 1 : 0;
			return change;
		}

		void ToggleSelection()				{ flags_.temp_selected = flags_.selected = !flags_.selected; }

		void SynchTempSelection()			{ flags_.temp_selected = flags_.selected; }

		bool TempSelectionToPermanent()
		{
			if (flags_.selected == flags_.temp_selected)
				false;
			flags_.selected = flags_.temp_selected;
			return true;
		}

		void SetTempSelection(bool select)	{ flags_.temp_selected = select ? 1 : 0; }

		bool HorizontalOrientation() const	{ return photo_->HorizontalOrientation(); }

		Group* GetGroup() const				{ return group_; }

		void SetGroup(Group* group)			{ group_ = group; }

		PhotoInfoPtr photo_;
		Group* group_;
		struct Flags
		{
			Flags() : selected(0), temp_selected(0) {}
			char selected;
			char temp_selected;
		} flags_;
	};

	friend class Item;

	//=============================================================================================
	class ItemVector : public std::vector<Item*>
	{
		enum { MIN_RESERVE= 10 };
	public:
		ItemVector() { reserve(MIN_RESERVE); }

		ItemVector(/*ItemList*/std::list<Item>::iterator begin, std::list<Item>/*ItemList*/::iterator end)
		{
			reserve(MAX(static_cast<size_t>(MIN_RESERVE), static_cast<size_t>(distance(begin, end))));
			for (std::list<Item>/*ItemList*/::iterator it= begin; it != end; ++it)
				push_back(&(*it));
		}

		explicit ItemVector(const std::vector<Item*>& items) : std::vector<Item*>(items)
		{}

		explicit ItemVector(Item* item) : std::vector<Item*>(1, item)
		{
			reserve(MIN_RESERVE);	// inefficient
		}

		// assign 'group' to all items
		void AssignGroup(Group* group);
	};

	friend class ItemVector;

	//=============================================================================================
	class ItemList : std::list<Item>
	{
	public:
		ItemList() { /*lookup_.max_load_factor(0.8f);*/ }

		ItemList(VectPhotoInfo::iterator begin, VectPhotoInfo::iterator end)
		{
			assign(begin, end);
			//lookup_.max_load_factor(0.8f);
			SyncSorted();
		}

		// add new item(s) to the list; keep sorted_ sorted
		Item* Insert(PhotoInfoPtr photo);
		void Insert(PhotoInfoPtr* begin, PhotoInfoPtr* end);

		// add new elements
		void Splice(list<Item>& items_list);

		// reserve space for extra elements
		void ReserveSpace(const size_t count);

		// last element
		Item* Back()						{ return &back(); }

		// find item (binary search)
		Item* FastFind(PhotoInfoPtr photo);
		const Item* FastFind(PhotoInfoPtr photo) const;

		// remove given item
		bool Remove(PhotoInfoPtr photo);

		// remove all items
		void RemoveAll()					{ clear(); sorted_.clear(); /*lookup_.clear();*/ }

		// remove all selected items
		void RemoveSelected();

		// read-only access
		const_iterator Begin() const		{ return list<Item>::begin(); }
		const_iterator End() const			{ return list<Item>::end(); }

		using list<Item>::const_iterator;

	private:
		// keep sorted_ in sync with list
		void SyncSorted();

		ItemVector sorted_;
		//std::unordered_map<PhotoInfo*, Item*> lookup_;
	};

	friend class ItemList;

	//=============================================================================================
	class Group
	{
	public:
		Group(const String& name, int id, Icons icon, Item* item);
		Group(const String& name, int id, Icons icon, std::vector<Item*>& items);

		~Group();

		const CRect& GetLocation() const	{ return location_rect_; }
		CRect GetItemRect(int index, bool bounding_box= true) const;
		static int GetMaxHeaderHeight();

		int GetItemTop() const;		// top of first item (this is also bottom of header)

		int HorzItems() const				{ return horz_items_; }

		int GetItemCount() const			{ return static_cast<int>(items_.size()); }
		int GetId() const					{ return id_; }
		
		Item* GetItem(int index)			{ return index >= 0 && index < items_.size() ? items_[index] : 0; }

		int SetLocation(CPoint left_top, int field_width, int horz_items, CSize item_size, int label_height,
			const CRect& margins_rect, bool separator, bool same_height);

		void Draw(PhotoCtrl& ctrl, CDC& paint_dc, MemoryDC& item_dc, MemoryDC& header_dc);
		void DrawSelection(PhotoCtrl& ctrl, CDC &dc, Item* item);

		enum GroupElements { NONE= 0, GROUP_LABEL, GROUP_ICON, GROUP_ITEM, GROUP_SEPARATOR, GROUP_HEADER };
		GroupElements HitTest(CPoint pos) const;

		Item* FindItem(PhotoCtrl& ctrl, CPoint pos);
		//Item* FindItem(PhotoInfoPtr photo);
		int FindItemIndex(Item* item);
		int FindRow(int y_pos) const;

		Item* FindFirstItemInRect(const CRect& rect, int current_index, bool full_row= true);
		Item* FindLastItemInRect(const CRect& rect, int current_index, bool full_row= true);

		void AddItem(Item* item);
		bool RemoveItem(Item* item);
		std::pair<bool, ItemVector> UpdateItems(ItemList& items_list, PhotoInfoPtr* begin, PhotoInfoPtr* end);
		bool RemoveSelected();
		void InsertItem(ItemList& items_list, PhotoInfoPtr photo, size_t index);

		bool SelectAll(bool select, bool toggle_if_all_same);
		bool SelectItems(Item* first_item, bool select_to_end);
		bool SelectItems(Item* item1, Item* item2);	// select from item1 to item2 (swap 1 & 2 if necessary)
		void ToggleSelection();
		void SynchTempSelection();
		bool TempSelectionToPermanent();
		void SelectTemporarily(PhotoCtrl& ctrl, CDC& dc, const CRect& selection_rect, bool toggle);
		void CollectSelected(VectPhotoInfo& selected);

		void Reserve(size_t space)			// allocate space for 'space' elements
		{
			items_.reserve(space);
//			sorted_.reserve(space);
		}

	private:
		String name_;
		int id_;
		ItemVector items_;		// group elements in order set by user
//		ItemVector sorted_;		// group elements sorted by photos pointers values for quick access
		enum Flags { NORMAL= 0x0, COLLAPSED= 0x1, HIDDEN= 0x2, SEPARATOR= 0x100, NO_HEADER= 0x200 };
		unsigned flags_;
		CRect location_rect_;		// group location
		CRect label_rect_;
		CSize item_size_;
		int horz_items_;
		CRect margins_rect_;		// item's margins
		Icons icon_;
		std::vector<int> row_heights_;	// accumulated rows heights

		void SyncSorted();			// keep sorted_ in sync with items_
		ItemVector ResetItems(ItemList& items_list, PhotoInfoPtr* begin, PhotoInfoPtr* end);
		bool ItemsOrientedHorizontally(int first_item, int count);
		int GetRowPosition(int row) const;		// row's Y position
		int GetRowHeight(int row) const;		// row's height

		// fixed sizes
		enum { HEADER_HEIGHT= 30, SEPARATOR_HEIGHT= 8 };

		Group(const Group&);
		Group& operator = (const Group&);
	};

	friend class Group;

	//=============================================================================================
	class ItemGroupPair : public std::pair<Group*, Item*>
	{
		typedef std::pair<Group*, Item*> ig_pair;
	public:
		ItemGroupPair() {}
		ItemGroupPair(Group* group, Item* item) : ig_pair(group, item) {}
		ItemGroupPair(const ItemGroupPair& igp) : ig_pair(igp) {}

		ItemGroupPair& operator = (const ItemGroupPair& igp)
		{
			first = igp.first;
			second = igp.second;
			return *this;
		}

		Group* Group() const			{ return first; }
		Item* Item() const				{ return second; }

		operator bool () const			{ return first != 0 && second != 0; }

		void ToggleSelection() const	{ Item()->ToggleSelection(); }

		void DrawSelection(PhotoCtrl& ctrl, CDC &dc) const
		{
			Group()->DrawSelection(ctrl, dc, Item());
		}

		void Clear()					{ first = 0; second = 0; }

		void SetItem(second_type  item)	{ second = item; }

		void Set(first_type group, second_type item)
		{
			first = group;
			second = item;
		}

		CRect GetItemRect(bool bounding_box)
		{
			return Group()->GetItemRect(Group()->FindItemIndex(Item()), bounding_box);
		}
	};

	//=============================================================================================
	class GroupVector : private std::vector<Group*>
	{
	public:
		GroupVector()	{ reserve(32); }
		~GroupVector()	{ RemoveAll(); }

		Group* Insert(std::auto_ptr<Group> group);
		void Remove(Group* group);

		int SetLocation(PhotoCtrl& ctrl, CPoint start, int width, int horz_items, CSize item_size, int label_height, bool same_height, int height_limit);

		void Draw(PhotoCtrl& ctrl, CDC& paint_dc, MemoryDC& item_dc, MemoryDC& header_dc);

		void RemoveAll();
		Group* RemoveItem(PhotoCtrl* ctrl, Item* item, Group* skip_group= nullptr);
		void RemoveSelected();

		Group* HitTest(CPoint pos) const;

		Item* FindItem(PhotoCtrl& ctrl, CPoint pos);
		ItemGroupPair FindItemGroup(PhotoCtrl& ctrl, CPoint pos);
		//ItemGroupPair FindItem(PhotoInfoPtr photo);

		Group* FindFirstGroup();
		Group* FindLastGroup();
		Group* FindNextGroup(Group* group);
		Group* FindPrevGroup(Group* group);
		int FindGroupIndex(Group* group);

		Group* FindFirstGroupInRect(const CRect& rect);
		Group* FindLastGroupInRect(const CRect& rect);

		Group* Get(int index) const	{ ASSERT(index >= 0 && index < size()); return (*this)[index]; }

		bool SelectAll(bool select, bool toggle_if_all_same);
		void ToggleSelection();
		void SynchTempSelection();
		bool TempSelectionToPermanent();
		void SelectTemporarily(PhotoCtrl& ctrl, CDC& dc, const CRect& selection_rect, bool toggle);
		void CollectSelected(VectPhotoInfo& selected);

		int GetItemCount() const;

		Group* Find(int group_id);

		using std::vector<Group*>::empty;
	};

	friend class GroupVector;

	//=============================================================================================

	GroupVector groups_;

	// list of items in PhotoCtrl along with sorted vector for fast access based on PhotoInfo pointer
	ItemList items_list_;

	// current item (has focus frame)
	ItemGroupPair current_item_;

	// anchor item for selection extending (Shift-click)
	ItemGroupPair anchor_item_;

	// current display mode
	Mode mode_;

	// sticky selection (if true) or fragile (std. Windows' selection)
	bool sticky_selection_;

	// total size of one item (includes item's margins, selection outline, text label rect)
	CSize item_size_;
	// rectangle of item's text label (relative to item's rect)
	CRect label_rect_;
	// rectangle of item's image (relative to item's rect)
	CRect image_rect_;
	// no of items horizontally
	int horz_items_;
	// group's icons
	static CImageList img_list_group_icons_;
	// photo indicators (like no-exif)
	static CImageList img_list_indicators_;
	// size of thumbnails (for thumbnail view mode)
	CSize img_size_;
	// sorting order indicator bitmaps
	static CBitmap ascending_bmp_;
	static CBitmap descending_bmp_;

	// in preview mode this variable controls how many items is to be placed across the window
	int items_across_;

	// those two sizes store values passed to scrollbars; used to avoid unnecessarily reseting scrollbars
	CSize page_size_;
	CSize max_size_;

	// if true halftone drawing method is used
	bool halftone_drawing_;

	// colors
	COLORREF rgb_bk_color_;
	COLORREF rgb_text_color_;
	COLORREF rgb_sel_color_;
	COLORREF rgb_sort_color_;
	COLORREF rgb_sel_text_color_;
	COLORREF rgb_separator_;
	COLORREF rgb_tag_bkgnd_;
	COLORREF rgb_tag_text_;
	COLORREF rgb_tile_dim_text_color_;
	// font (for labels and detailed mode)
	CFont default_fnt_;
	// font for group names (bold & underlined)
	CFont group_fnt_;
	// font for tags
	CFont tags_fnt_;
	// size of "M" in default font
	CSize M_size_;
	// item's margins
	CRect margins_rect_;
	// 'y' coordinates of the last used screen line
	int bottom_;
	// if true items manes will be drawn
	bool show_label_;
	// if true tag text (keywords) will be draw over the photo
	bool show_tags_;

	// header ctrl for detailed view mode -----------------
	CHeaderCtrl header_wnd_;
	enum { ID_HEADER= 100 };
	int header_height_;	// header ctrl height in detailed mode
	int top_margin_;		// position where window area starts (below header)
	struct ColumnInfo
	{
		ColumnInfo() : width(0), order(0), sub_item(0), format(0) {}
		ColumnInfo(int sub_item, int width, int order, int format)
			: width(width), order(order), sub_item(sub_item), format(format) {}
		ColumnInfo(const ColumnInfo& col)
		{ *this = col; }

		ColumnInfo& operator = (const ColumnInfo& col)
		{
			width = col.width;
			order = col.order;
			sub_item = col.sub_item;
			format = col.format;
			return *this;
		}

		int width;
		int order;
		int sub_item;
		int format;
	};
	std::vector<ColumnInfo> header_columns_;
	int ColumnsTotalWidth();
	void ResetHorzScrollBar();
	void SynchColumnsOrder();
	void SynchColumnsInfo();
	// header ctrl notifications
	void OnHeaderItemChanging(NMHDR* nm_hdr, LRESULT* result);
	void OnHeaderItemDblClick(NMHDR* nm_hdr, LRESULT* result);
	void OnHeaderItemClick(NMHDR* nm_hdr, LRESULT* result);
	void OnHeaderItemEndDrag(NMHDR* nm_hdr, LRESULT* result);
	void OnHeaderRClick(NMHDR* nm_hdr, LRESULT* result);
	LRESULT OnHeaderItemEndDrag2(WPARAM, LPARAM);
	int sort_by_column_;		// sorting column indicator

	// width of an item in preview mode (for small & big client rect--with & without scrollbars)
	std::pair<int, int> CalcItemWidth(int items_across);

	// mouse select ---------------------------------------
	struct MouseSelect
	{
		bool selecting_;
		bool lasso_;
		CPoint start_;				// selection starting point
		bool drag_and_drop_;			// drag & drop operation initiated?
		CSize scroll_amount_size_;		// amount of pixel to scroll during lasso-selecting outside of the screen
		int selected_count_;			// amount of selected items
		CRect selection_rect_;
		bool click_and_select_;
		bool go_to_done_;

		void Stop()
		{
			selecting_ = false;
			lasso_ = false;
			drag_and_drop_ = false;
			selection_rect_.SetRectEmpty();
			scroll_amount_size_ = CSize(0, 0);
		}

		MouseSelect()
		{
			selecting_ = false;
			lasso_ = false;
			start_ = CPoint(0,0);
			drag_and_drop_ = false;
			scroll_amount_size_ = CSize(0,0);
			selected_count_ = 0;
			click_and_select_ = true;
			selection_rect_.SetRectEmpty();
			go_to_done_ = false;
		}
	} select_;

	// notifications are sent here
	PhotoCtrlNotification* host_;

	// hardcoded values
	enum
	{
		THUMB_SIZE= 160,
		MARGIN_LEFT= 3, MARGIN_RIGHT= 3,
		MARGIN_TOP= 4, MARGIN_BOTTOM= 2,
		ITEM_X_MARGIN= 5,	// horz margin for text in detailed mode view
		SCROLL_TIMER_ID= 1234
	};

	///////////////////////////////////////////////////////////////////////////

	CPoint GetScrollOffset() const;
	void   SetScrollOffset(CPoint scroll)	{ SetScrollPos(SB_HORZ, scroll.x); SetScrollPos(SB_VERT, scroll.y); }

	void   PrepareDC(CDC& dc)				{ dc.SetViewportOrg(-GetScrollOffset()); }

	// find all items currently wisible in whole client area
	void FindVisibleItems(int& first, int& last);

	// find item's location
	CRect GetItemRect(int index);

	// draw one item
//	void DrawItem(Item& item, CDC& dc, CRect rect);

	// set display mode
	void SetMode(Mode mode);

	// set new font
	void SetFont(LOGFONT normal, LOGFONT bold);

	// resize
	void Resize();

	// set scrollbars
	void ResetScrollSize();

	// get client rect (excluding header ctrl if present) and shifted if scrolled
	CRect GetFieldRect(bool shifted= true) const;

	// scrolling support
	void OnScroll(UINT scroll_code, UINT pos);
	BOOL OnScrollBy(CSize scroll_size, BOOL do_scroll);

	// resize helper
	void ResetMode(Mode mode, bool resetLocations);

	// set groups' location
	bool SetLocation(CSize item_size, CSize item_big_size= CSize(0, 0));
	int SetLocation(const CRect& rect, int horz_items, CSize item_size, int height_limit);	// helper fn for SetLocation
	// helper fn for SetLocation
	CSize TryLocation(const CRect& rect, CSize item_size, int& width, int& height, int& horz_items, int& bottom, int height_limit);

	// set scroll info if it has changed
	void SetScrollBar(int bar, SCROLLINFO& si);

	// do we have any groups or is ctrl empty
	bool IsEmpty() const					{ return groups_.empty(); }

	// find group at 'pos'
	Group* HitTest(CPoint pos)				{ return groups_.HitTest(pos); }

	// make sure temporary selection matches real one
	void SynchTempSelection()				{ groups_.SynchTempSelection(); }

	// copy temporary selection to permanent selection
	bool TempSelectionToPermanent()			{ return groups_.TempSelectionToPermanent(); }

	// find item at 'pos'
	Item* FindItem(CPoint pos)				{ return groups_.FindItem(*this, pos); }

	// find item containing photos
	ItemGroupPair FindItem(PhotoInfoPtr photo);

	// find group & item at 'pos'
	ItemGroupPair FindItemGroup(CPoint pos)	{ return groups_.FindItemGroup(*this, pos); }

	// helper functions
	void RemoveItem(Item* item, bool notify);
	void RemoveGroup(Group* group);

	// send selection changed notification
	void SelectionChanged();

	// drawing dotted lasso selection rect (at stored selection_rect_ position)
	void DrawSelectionRect(CDC& dc, bool erasing);

	// select correct cursor (arrow or pointing hand for group labels)
	void SetCursor();

	// move current item
	void GoTo(ItemGroupPair igp, bool select_current, bool extend_sel_key, bool toggle_sel_key, bool notify= true, bool scroll= true);
	enum Direction { GO_LEFT, GO_RIGHT, GO_UP, GO_DOWN, GO_TOP, GO_BOTTOM, GO_PG_UP, GO_PG_DOWN };
	void GoTo(Direction dir, bool select_current, bool extend_sel_key, bool toggle_sel_key);

	// extend current selection from anchor item to the current item
	bool SelectionExtend(ItemGroupPair anchor_item, ItemGroupPair current_item, bool desel_current);

	// find next adjacent item
	ItemGroupPair FindNextItem(ItemGroupPair igp, Direction dir);

	// find next/prev page helper
	ItemGroupPair FindPage(ItemGroupPair igp, bool next_page);

	// perform drag & drop operation
	void DoDragDrop();

	// start scroccling if user is lasso-selecting images outside of the window
	void LassoSelectScroll();

	// change temporary selection for items inside rect
	void SelectTemporarily(CDC& dc, const CRect& selection_rect, bool toggle)
	{ groups_.SelectTemporarily(*this, dc, selection_rect, toggle); }

	// collect all selected photos
	void CollectSelected(VectPhotoInfo& selected);

	// scroll window's content if needed to make sure item 'igp' is visible
	void EnsureVisible(ItemGroupPair igp);

	// set sorting indicator
	void SetSortingIndicator(int column_sort);

	// update/add items to the existing group
	void UpdateItems(PhotoInfoPtr* begin, PhotoInfoPtr* end, Group* group);

	// insert new item into the existing group
	void InsertItem(PhotoInfoPtr photo, size_t index, Group* group);

	// invalidate window's rect at group location
	void InvalidateGroup(Group* group);

	// reselect images in lasso area (after Control key pressed--selectio toggle)
	void LassoReselect(bool toggle);

	// move header ctrl window to the right place (takes scroll pos into account)
	void SetHeader();

	// helper fn: collects Items corresponding to the range of photos in a vector; those
	// items are first searched for in a local storage (items_list_), if not found
	// they are created as 'items_list' elements
	std::vector<Item*> CollectItems(PhotoInfoPtr* begin, PhotoInfoPtr* end, std::list<Item>& items_list, std::vector<Item*>& items);

	// helper fn: move current item to unselected item
	void MoveCurrentToUnselected();

	// helper fn: get size and position of tiny image in first column (detailed view mode)
	static CRect GetFirstColImgRect(const CRect& text_rect);

	// call host to find out is photo marker is to be displayed for a current item
	bool ShowPhotoMarker(Item* item);
	// call host to find out is 'no EXIF' indicator is to be displayed for a current item
	bool ShowNoExifIndicator(Item* item);

	// helper fn to test rect visibility
	BOOL IsRectVisible(CDC& dc, CRect rect) const;

	// helper fn to find first/last visible item
	PhotoInfoPtr FindVisibleItem(bool first);

	// helper fn to isolate calculation: item's rect -> img rect
	CRect GetImageRect(const CRect& item_rect, PhotoInfoPtr photo, bool bmp_outline) const;

	// helper
	int GetLabelHeight() const;

	// helper fn
	CSize SetItemsDimensions(Mode mode, bool bigItem);

	// helper fn: calc client rect with and without scrollbars
	void GetClientRects(CRect& small_rect, CRect& big_rect);

	// redraw given area
	void Draw(CDC& dc, const CRect& rect);

	void OnDestroy();

	friend struct OredTempSelToPerm;
	friend struct PointYInGroup;
	friend struct OredItemTempSelToPerm;
	friend struct AccCountOfItems;
	friend struct MatchingGroupId;
	friend struct MatchingPhotoInfo;
	friend struct GroupIdLess;
	friend struct AccColumnWidth;
	friend struct ItemsByPhotos;
	friend struct SameItem;

	// tooltip support
	virtual INT_PTR OnToolHitTest(CPoint pos, TOOLINFO* it) const;
	BOOL OnToolTipNotify(UINT id, NMHDR* nmhdr, LRESULT* result);
	void FilterToolTipMessage(MSG* msg);
	void RemoveToolTips();
	CToolTipCtrl tool_tip_wnd_;
	CWnd* last_hit_;		// last window to own tooltip
	INT_PTR last_hit_code_;	// last hittest code
	TOOLINFO last_info_;	// last TOOLINFO structure
	bool ballon_info_enabled_;
	bool enable_updating_;
	int enable_set_current_counter_;
	CoolScrollBar scroll_bar_;
};

/////////////////////////////////////////////////////////////////////////////


class PhotoCtrlNotification
{
public:
	virtual void ItemClicked(PhotoInfoPtr photo)								{}
	virtual void ItemDblClicked(PhotoInfoPtr photo)								{}
	virtual void SelectionChanged(VectPhotoInfo& selected_photos)				{}
	virtual void DoDragDrop(VectPhotoInfo& selected_photos)						{}
	virtual void CurrentItemChanged(PhotoInfoPtr photo)							{}
	virtual void GetItemText(PhotoInfoPtr photo, int column, String& buffer)	{}
	virtual void SortByColumn(int col_index, bool secondary_key)				{}
	virtual void ColumnRClick(int col_index)									{}
	virtual void ContextMenu(CPoint mouse, bool mouse_click, int group_id, int group_elem) {}
	virtual void KeyDown(UINT chr, UINT rep_cnt, UINT flags)					{}
	virtual bool ShowPhotoMarker(int file_type)									{ return false; }
	virtual bool ShowNoExifIndicator(int file_type)								{ return false; }
	virtual String GetToolTipText(PhotoInfoPtr photo)							{ return String(); }
	virtual String GetItemLabel(PhotoInfoPtr photo, CDC& dc, int label_space)	{ return String(); }
	virtual int GetLabelLines(PhotoCtrl::Mode mode) const						{ return 1; }
	virtual Dib* RequestImage(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available) { return 0; }
	virtual Dib* RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available) { return 0; }
	virtual AutoPtr<Dib> CorrectImageColors(PhotoInfoPtr photo, Dib* bmp)		{ return 0; }
	enum Shift { UP, DOWN, TOP, BOTTOM };
	virtual void ContentScrolled(Shift dir)										{}
	virtual bool OnChar(UINT chr, UINT rep_cnt, UINT flags)						{ return false; }
};


struct BlockPhotoCtrlUpdates
{
	BlockPhotoCtrlUpdates(PhotoCtrl& ctrl) : ctrl_(ctrl)
	{
		ctrl_.EnableUpdate(false);
		reenable_ = true;
	}

	void Release()
	{
		if (reenable_)
			ctrl_.EnableUpdate(true);
		reenable_ = false;
	}

	~BlockPhotoCtrlUpdates()
	{
		if (reenable_)
		{
			try
			{
				ctrl_.EnableUpdate(true);
			}
			catch (...)
			{}
		}
	}

private:
	PhotoCtrl& ctrl_;
	bool reenable_;
};


#endif // !defined(_PHOTOCTRL_H_INCLUDED_)
