/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// very simple set class; a hybrid vector-hash container that guarantees unique elements to be available in the
// order they have been added and provides fast test for element presence;
// note that elements are kept duplicated (once in a vector and once in a hash)
// apart from that elements can be optionally grouped, that is ranges of tags can be assigned a label (grouped)


template <class T>
class set
{
public:
	set() : current_group_(0) {}

	size_t size() const		{ return vect_.size(); }

	bool empty() const		{ return vect_.empty(); }

	bool push_back(const T& el)
	{
		if (hash_.insert(el).second)
		{
			janitor oper(hash_, el);
			if (current_group_ == 0)
				open_new_group(T());
			vect_.push_back(el);
			current_group_->end_ = vect_.size();
			oper.commit();
			return true;
		}
		return false;	// dups rejected
	}

	T& operator [] (size_t index)				{ return vect_[index]; }
	const T& operator [] (size_t index) const	{ return vect_[index]; }

	// test for presence; does not add the element
	bool operator [] (const T& el) const		{ return hash_.find(el) != hash_.end(); }

	void reserve(size_t count)					{ vect_.reserve(count); }

	void clear()								{ vect_.clear(); hash_.clear(); groups_.clear(); current_group_ = 0; }

	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;

	iterator begin()							{ return vect_.begin(); }
	iterator end()								{ return vect_.end(); }

	const_iterator begin() const				{ return vect_.begin(); }
	const_iterator end() const					{ return vect_.end(); }

	bool operator == (const set& s)
	{
		if (vect_.size() != s.vect_.size())		return false;
		if (vect_.empty())		return true;
		// compare elements one by one
		return mismatch(vect_.begin(), vect_.end(), s.vect_.begin()).first == vect_.end() &&
			s.groups_ == groups_;
	}

	void open_new_group(const T& name)
	{
		close_group();
		groups_.push_back(group(name, vect_.size()));
		current_group_ = &groups_.back();
	}

	void close_group()
	{
		current_group_ = 0;
	}

	class group
	{
	public:
		group(const T& name, size_t start) : name_(name)
		{
			begin_ = end_ = start;
		}

		const T& name() const
		{
			return name_;
		}

		size_t begin() const		{ return begin_; }
		size_t end() const			{ return end_; }
		std::pair<size_t, size_t> span() const	{ return std::make_pair(begin_, end_); }

		bool operator == (const group& g) const
		{
			return name_ == g.name_ && begin_ == g.begin_ && end_ == g.end_;
		}

	private:
		T name_;
		size_t begin_;
		size_t end_;

		friend class ::set;
	};

	typedef typename std::vector<group>::const_iterator group_iterator;

	group_iterator begin_group() const	{ return groups_.begin(); }
	group_iterator end_group() const	{ return groups_.end(); }

	const group& get_group(size_t index) const	{ return groups_.at(index); }

private:
	std::vector<T> vect_;
	std::unordered_set<T> hash_;
	std::vector<group> groups_;
	group* current_group_;

	struct janitor
	{
		janitor(std::unordered_set<T>& hash, const T& el) : hash_(hash), el_(el), undo_(true)
		{}

		void commit()	{ undo_ = false; }

		~janitor()		{ if (undo_) hash_.erase(el_); }

	private:
		std::unordered_set<T>& hash_;
		const T& el_;
		bool undo_;
	};
};
