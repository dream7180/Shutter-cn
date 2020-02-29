/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ParseRange.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ParseRange::Status ParseRange::Parse(std::vector<std::pair<int, int>>& ranges)
{
	try
	{
		return ParseWorker(ranges);
	}
	catch (RangeError&)
	{
		return NUM_RANGE_ERR;
	}
	catch (std::exception&)
	{
		return OUT_OF_MEM;
	}
}


ParseRange::Status ParseRange::ParseWorker(std::vector<std::pair<int, int>>& ranges)
{
	const TCHAR* text= text_;
	Status status= OK;

	int number_from= 0;
	int number= 0;

	ranges.clear();

	for (State state= START; state != ERROR; )
	{
		int cur_num= 0;
		Leksem leks= GetLeksem(text, cur_num);

		switch (state)
		{
		case START: ///////////////////////////////////////////////////////////
			switch (leks)
			{
			case NUMBER:
				state = HAS_NUMBER;
				break;

			default:
				state = ERROR;
				status = leks == EOS ? EXPECTED_NUM : UNEXPECTED_CHAR;
				break;
			}
			break;

		case HAS_NUMBER: //////////////////////////////////////////////////////
			switch (leks)
			{
			case DASH:
				state = OPEN_RANGE;
				number_from = number;
				break;

			case SEPARATOR:
				state = HAS_SEPARATOR;
				ranges.push_back(std::make_pair(number, number));
				break;

			case NUMBER:
				ranges.push_back(std::make_pair(number, number));
				break;

			case EOS:	// exit now
				ranges.push_back(std::make_pair(number, number));
				return OK;

			default:
				state = ERROR;
				status = UNEXPECTED_CHAR;
				break;
			}
			break;

		case HAS_SEPARATOR: ///////////////////////////////////////////////////
			switch (leks)
			{
			case NUMBER:
				state = HAS_NUMBER;
				break;

			case EOS:
				state = ERROR;
				status = EXPECTED_NUM;
				break;

			default:
				state = ERROR;
				status = UNEXPECTED_CHAR;
				break;
			}
			break;

		case OPEN_RANGE: //////////////////////////////////////////////////////
			switch (leks)
			{
			case NUMBER:
				if (number_from > cur_num)
				{
					state = ERROR;
					status = WRONG_RANGE;
				}
				else
				{
					state = HAS_RANGE;
					ranges.push_back(std::make_pair(number_from, cur_num));
				}
				break;

			default:
				state = ERROR;
				status = EXPECTED_NUM;
				break;
			}
			break;

		case HAS_RANGE: ///////////////////////////////////////////////////////
			switch (leks)
			{
			case NUMBER:
				state = HAS_NUMBER;
				break;

			case SEPARATOR:
				state = HAS_SEPARATOR;
				break;

			case EOS:
				return OK;

			default:
				state = ERROR;
				status = UNEXPECTED_CHAR;
				break;
			}
			break;

		default: //////////////////////////////////////////////////////////////
			ASSERT(false);
			break;
		}

		number = cur_num;
	}

	return status;
}


ParseRange::Leksem ParseRange::GetLeksem(const TCHAR*& text, int& number)
{
	if (text == 0 || *text == 0)
		return EOS;

	while (*text == ' ')
		++text;

	if (*text >= '0' && *text <= '9')
	{
		TCHAR* end= 0;
		errno = 0;
		number = _tcstol(text, &end, 10);
		if (errno)
			throw RangeError();

		text = end;
		return NUMBER;
	}

	if (*text == ',' || *text == ';')
	{
		++text;
		return SEPARATOR;
	}

	if (*text == '-')
	{
		++text;
		return DASH;
	}

	return SOMECHAR;
}


String ParseRange::GetErrMessage(Status stat)
{
	switch (stat)
	{
	case OK:
		break;

	case UNEXPECTED_CHAR:
		return _T("遇到非法字符");

	case EXPECTED_NUM:
		return _T("缺少需要的数字");

	case NUM_RANGE_ERR:
		return _T("数字超出范围");

	case OUT_OF_MEM:
		return _T("内存溢出");

	case WRONG_RANGE:
		return _T("第二个数字在范围内大于第一个");

	default:
		ASSERT(false);
		break;
	}

	return _T("");
}
