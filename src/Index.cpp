/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Index.cpp: implementation of the ImageIndex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Index.h"
#include "Dib.h"
#include <math.h>
#include "MemPointer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const static float INIT= -1e6f;


ImageIndex::ImageIndex()
{
	std::fill(average_, average_ + array_count(average_), 100.0f);
	std::fill(std_deviation_, std_deviation_ + array_count(std_deviation_), 100.0f);
	std::fill(skewness_, skewness_ + array_count(skewness_), 100.0f);
#ifdef _DEBUG
	distance_ = 0.0f;
#endif
	std::fill(histogram_, histogram_ + array_count(histogram_), INIT);
}


bool ImageIndex::IsInitialized() const
{
	return histogram_[0] != INIT;
}


void ImageIndex::CalcHistogram(const Dib& dib)
{
	int bits_per_pixel= dib.GetBitsPerPixel();

	if (bits_per_pixel != 24 && bits_per_pixel != 8)
	{
//		ASSERT(false);
		return;
	}

	std::fill(histogram_, histogram_ + array_count(histogram_), 0.0f);

	int height= dib.GetHeight();
	int width= dib.GetWidth();

	if (height < 4 || width < 4)
		return;

	int offset= bits_per_pixel == 24 ? 3 : 1;

//	if (bits_per_pixel == 24)
	{
		for (int y= 1; y < height - 1; ++y)
		{
			const uint8* prev= dib.LineBuffer(y - 1) + offset;
			const uint8* curr= dib.LineBuffer(y) + offset;
			const uint8* next= dib.LineBuffer(y + 1) + offset;

			for (int x= 1; x < width - 1; ++x)
			{
				BYTE index= 0;
				int c= *curr;	// Y component, central pixel
				// average of 5 pixels (times 3)
				c = (curr[-offset] + c + curr[offset] + *prev + *next) * 3 / 5;

				int top= int(prev[-offset]) + prev[0] + prev[offset];
				int btm= int(next[-offset]) + next[0] + next[offset];

				int left= int(prev[-offset]) + curr[-offset] + next[-offset];
				int right= int(prev[offset]) + curr[offset] + next[offset];

				int left_top= int(curr[-offset]) + prev[-offset] + prev[0];
				int right_top= int(curr[offset]) + prev[offset] + prev[0];

				int left_btm= int(curr[-offset]) + next[-offset] + next[0];
				int right_btm= int(curr[offset]) + next[offset] + next[0];

				if (left_top < c)
					index |= 0x80;
				if (top < c)
					index |= 0x40;
				if (right_top < c)
					index |= 0x20;

				if (left < c)
					index |= 0x10;
				if (right < c)
					index |= 0x08;

				if (left_btm < c)
					index |= 0x04;
				if (btm < c)
					index |= 0x02;
				if (right_btm < c)
					index |= 0x01;

				histogram_[index] += 1.0;

				prev += offset;
				curr += offset;
				next += offset;
			}
		}
	}
	//else
	//{
	//	for (int y= 1; y < height - 1; ++y)
	//	{
	//		const uint8* prev= dib.LineBuffer(y - 1) + 1;
	//		const uint8* curr= dib.LineBuffer(y) + 1;
	//		const uint8* next= dib.LineBuffer(y + 1) + 1;

	//		for (int x= 1; x < width- 1; ++x)
	//		{
	//			BYTE index= 0;
	//			int c= *curr;	// Y component, central pixel

	//			if (prev[-1] > c)
	//				index |= 0x80;
	//			if (prev[0] > c)
	//				index |= 0x40;
	//			if (prev[1] > c)
	//				index |= 0x20;

	//			if (curr[-1] > c)
	//				index |= 0x10;
	//			if (curr[1] > c)
	//				index |= 0x08;

	//			if (next[-1] > c)
	//				index |= 0x04;
	//			if (next[0] > c)
	//				index |= 0x02;
	//			if (next[1] > c)
	//				index |= 0x01;

	//			histogram_[index] += 1.0;

	//			prev += 1;
	//			curr += 1;
	//			next += 1;
	//		}
	//	}
	//}

	uint32 pixels_used= (height - 2) * (width - 2);

	for (int i= 0; i < 0x100; ++i)
		histogram_[i] /= pixels_used;

	// color info

	if (bits_per_pixel == 24)
	{
		int line_size= dib.GetBytesPerLine() / 3;
		int pixels_count= dib.GetWidth() * dib.GetHeight();

		// calculations below are meant for small thumbnail image only;
		// overflow may result otherwise
		ASSERT(pixels_count < 0xffffff);

		uint32 avg_cr= 0;
		uint32 avg_cb= 0;

		for (int line= 0; line < dib.GetHeight(); ++line)
		{
			const uint8* data= dib.LineBuffer(line);

			for (int n= 0; n < line_size; ++n)
			{
				uint8 y= *data++;
				uint8 cr= *data++;
				uint8 cb= *data++;

				avg_cr += cr;
				avg_cb += cb;
			}
		}

		// calc averages for Cr & Cb
		average_[0] = float(avg_cr) / pixels_count;
		average_[1] = float(avg_cb) / pixels_count;

		// calc std deviation and skewness
		std_deviation_[0] = 0.0f;
		std_deviation_[1] = 0.0f;

		skewness_[0] = 0.0f;
		skewness_[1] = 0.0f;

		for (int line2= 0; line2 < dib.GetHeight(); ++line2)
		{
			const uint8* data= dib.LineBuffer(line2);

			for (int n= 0; n < line_size; ++n)
			{
				uint8 y= *data++;
				uint8 cr= *data++;
				uint8 cb= *data++;

				float delta= cr - average_[0];
				float delta_sq= delta * delta;

				std_deviation_[0] += delta_sq;
				skewness_[0] += delta_sq * delta;

				delta = cb - average_[1];
				delta_sq = delta * delta;

				std_deviation_[1] += delta_sq;
				skewness_[1] += delta_sq * delta;
			}
		}

		std_deviation_[0] = static_cast<float>(sqrt(std_deviation_[0] / pixels_count));
		std_deviation_[1] = static_cast<float>(sqrt(std_deviation_[1] / pixels_count));

		for (int chnl= 0; chnl < 2; ++chnl)
		{
			bool negative= skewness_[chnl] < 0.0f;
			skewness_[chnl] = static_cast<float>(pow(static_cast<double>(fabs(skewness_[chnl]) / pixels_count), 1.0 / 3.0));
			if (negative)
				skewness_[chnl] = -skewness_[chnl];
		}

		average_[0] /= 255.0f;
		average_[1] /= 255.0f;
		std_deviation_[0] /= 255.0f;
		std_deviation_[1] /= 255.0f;
		skewness_[0] /= 255.0f;
		skewness_[1] /= 255.0f;
	}
	else
	{
		// grayscale image

		// instead of color channels use grayscale info

		average_[0] = average_[1] = 0.0f;
		std_deviation_[0] = std_deviation_[1] = 0.0f;
		skewness_[0] = skewness_[1] = 0.0f;


		ASSERT(bits_per_pixel == 8);
		int line_size= dib.GetBytesPerLine();
		int pixels_count= dib.GetWidth() * dib.GetHeight();

		// calculations below are meant for small thumbnail image only;
		// overflow may result otherwise
		ASSERT(pixels_count < 0xffffff);

		uint32 avg_y= 0;	// avg brightness

		for (int line= 0; line < dib.GetHeight(); ++line)
		{
			const uint8* data= dib.LineBuffer(line);

			for (int n= 0; n < line_size; ++n)
				avg_y += *data++;
		}

		// calc averages for Y
		average_[0] = float(avg_y) / pixels_count;

		// calc std deviation and skewness
		std_deviation_[0] = 0.0f;
		skewness_[0] = 0.0f;

		for (int line2= 0; line2 < dib.GetHeight(); ++line2)
		{
			const uint8* data= dib.LineBuffer(line2);

			for (int n= 0; n < line_size; ++n)
			{
				uint8 y= *data++;

				float delta= y - average_[0];
				float delta_sq= delta * delta;

				std_deviation_[0] += delta_sq;
				skewness_[0] += delta_sq * delta;
			}
		}

		std_deviation_[0] = static_cast<float>(sqrt(std_deviation_[0] / pixels_count));

		for (int chnl= 0; chnl < 1; ++chnl)
		{
			bool negative= skewness_[chnl] < 0.0f;
			skewness_[chnl] = static_cast<float>(pow(static_cast<double>(fabs(skewness_[chnl]) / pixels_count), 1.0 / 3.0));
			if (negative)
				skewness_[chnl] = -skewness_[chnl];
		}

		average_[0] /= 255.0f;
//		average_[1] /= 255.0f;
		std_deviation_[0] /= 255.0f;
//		std_deviation_[1] /= 255.0f;
		skewness_[0] /= 255.0f;
//		skewness_[1] /= 255.0f;
	}
}


float ImageIndex::Feature(float avg_weight, float std_dev_weight, float skew_weight) const
{
	return avg_weight * average_[0] + std_dev_weight * std_deviation_[0] + skew_weight * skewness_[0] +
		   avg_weight * average_[1] + std_dev_weight * std_deviation_[1] + skew_weight * skewness_[1];
}

static inline float fltabs(float f)
{
	return f >= 0.0f ? f : -f;
}


float ImageIndex::Feature() const
{
	return 0.3f * average_[0] + 5.0f * std_deviation_[0] + fltabs(skewness_[0]) +
		   0.3f * average_[1] + 5.0f * std_deviation_[1] + fltabs(skewness_[1]);
}


String ImageIndex::AsString() const
{
	CString s;
/*	s.Format(_T("%4.1f %03d %03d %03d  %03d %03d %03d  %03d %03d %03d"),
#ifdef _DEBUG
		distance_,
#else
		0.0f,
#endif
		int(average_[0]), int(std_deviation_[0]), int(skewness_[0]),
		int(average_[1]), int(std_deviation_[1]), int(skewness_[1]),
//		int(average_[2]), int(std_deviation_[2]), int(skewness_[2]));
*/
	return static_cast<const TCHAR*>(s);
}


bool ImageIndex::operator < (const ImageIndex& index) const
{
	return false;
//	return 0.3f * average_[0] + 5.0f * std_deviation_[0] + fabs(skewness_[0]) +
//		   0.3f * average_[1] + 5.0f * std_deviation_[1] + fabs(skewness_[1]) +
//		   0.3f * average_[2] + 5.0f * std_deviation_[2] + fabs(skewness_[2]);
}


float ImageIndex::Distance(const ImageIndex& idx, float color_vs_shape_weight) const
{
	// shape

	float sum= 0.0f;
	float sum1= 0.0f, sum2= 0.0f, sum3= 0.0f, sum4= 0.0f;
//LARGE_INTEGER tm[9];
//::QueryPerformanceCounter(&tm[0]);

	//Original loop:
	//for (int i= 0; i < 0x100; ++i)
	//	sum += fabs(histogram_[i] - idx.histogram_[i]);

	// speed up
	for (int i= 0; i < 0x100; i += 4)
	{
		sum1 += fabs(histogram_[i+0] - idx.histogram_[i+0]);
		sum2 += fabs(histogram_[i+1] - idx.histogram_[i+1]);
		sum3 += fabs(histogram_[i+2] - idx.histogram_[i+2]);
		sum4 += fabs(histogram_[i+3] - idx.histogram_[i+3]);
	}
	sum = sum1 + sum2 + sum3 + sum4;

//::QueryPerformanceCounter(&tm[1]);
////::DebugBreak();
//LARGE_INTEGER tmr[9], frq;
//::QueryPerformanceFrequency(&frq);
//for (int ii= 0; ii < 1; ++ii)
//{
//	tmr[ii].QuadPart = tm[ii + 1].QuadPart - tm[ii].QuadPart;
////	tmr[ii].QuadPart *= 1000000;
////	tmr[ii].QuadPart /= frq.QuadPart;
//}
//oStringstream ost;
//ost << tmr[0].QuadPart << endl;
//::OutputDebugStr(ost.str().c_str());

	float shape_distance= /*1.0f -*/ sum;

	//============================================================

	// color

	float w1= 3.0f / 9.0f;
	float w2= 5.0f / 9.0f;
	float w3= 1.0f / 9.0f;

	float color_distance= 0.0f;

	for (int chnl= 0; chnl < 2; ++chnl)
	{
		color_distance += w1 * fltabs(idx.average_[chnl]      - average_[chnl]);
		color_distance += w2 * fltabs(idx.std_deviation_[chnl] - std_deviation_[chnl]);
		color_distance += w3 * fltabs(idx.skewness_[chnl]     - skewness_[chnl]);
	}

	color_distance *= 7.0f;

//TRACE(L"shp: %f   clr: %f\n", double(shape_distance), double(color_distance));


	ASSERT(color_vs_shape_weight >= 0.0f && color_vs_shape_weight <= 1.0f);

	return color_vs_shape_weight * color_distance + (1.0f - color_vs_shape_weight) * shape_distance;
}


ImageIndex& ImageIndex::operator = (const ImageIndex& idx)
{
	for (int chnl= 0; chnl < 2; ++chnl)
	{
		average_[chnl] = idx.average_[chnl];
		std_deviation_[chnl] = idx.std_deviation_[chnl];
		skewness_[chnl] = idx.skewness_[chnl];
	}

	std::copy(idx.histogram_, idx.histogram_ + array_count(idx.histogram_), histogram_);

#ifdef _DEBUG
	distance_ = idx.distance_;
#endif

	return *this;
}


// copy histogram info to the flat buffer (for saving)
void ImageIndex::Serialize(std::vector<uint8>& buffer)
{
	buffer.resize(SizeOf());
	MemPointer ptr(&buffer.front(), buffer.size());

	ptr.Write(average_, sizeof(average_));
	ptr.Write(std_deviation_, sizeof(std_deviation_));
	ptr.Write(skewness_, sizeof(skewness_));
	ptr.Write(histogram_, sizeof(histogram_));
}


// restore histogram from the memory buffer
void ImageIndex::ConstructFromBuffer(const std::vector<uint8>& buffer)
{
	if (buffer.size() >= SizeOf())
	{
		MemPointer ptr(const_cast<uint8*>(&buffer.front()), buffer.size());

		ptr.Read(average_, sizeof(average_));
		ptr.Read(std_deviation_, sizeof(std_deviation_));
		ptr.Read(skewness_, sizeof(skewness_));
		ptr.Read(histogram_, sizeof(histogram_));
	}
	else
		ASSERT(false);
}
