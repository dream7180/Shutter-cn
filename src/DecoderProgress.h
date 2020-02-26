/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class ImageDecoder;


class DecoderProgress
{
public:
	DecoderProgress() : scan_lines_(0) {}

	virtual ~DecoderProgress();

	virtual bool LinesDecoded(int lines_ready, bool finished) = 0;

	int ScanLines() const		{ return scan_lines_; }

private:
	int scan_lines_;

	friend class ImageDecoder;
};
