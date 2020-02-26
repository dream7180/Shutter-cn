/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#undef ERROR	// windows' p.o.s.


// simple automaton analyzing selection of numbers or number ranges


class ParseRange
{
public:
	ParseRange(const TCHAR* text) : text_(text)
	{}

	enum Status { OK, UNEXPECTED_CHAR, EXPECTED_NUM, NUM_RANGE_ERR, OUT_OF_MEM, WRONG_RANGE };

	Status Parse(std::vector<std::pair<int, int>>& ranges);

	String GetErrMessage(Status stat);

private:
	const TCHAR* text_;

	enum State { START, HAS_NUMBER, HAS_SEPARATOR, OPEN_RANGE, HAS_RANGE, ERROR };
	enum Leksem { NUMBER, SEPARATOR, DASH, EOS, SOMECHAR };

	Status ParseWorker(std::vector<std::pair<int, int>>& ranges);
	Leksem GetLeksem(const TCHAR*& text, int& number);

	struct RangeError : std::exception
	{};
};
