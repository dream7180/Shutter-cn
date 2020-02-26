/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Index.h: ImageIndex object is used to identify shape/color of image.
// It's used to sort images by similarity.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEX_H__E3D60A82_14A0_41E2_958E_04DFC7529EDB__INCLUDED_)
#define AFX_INDEX_H__E3D60A82_14A0_41E2_958E_04DFC7529EDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Dib;


class ImageIndex
{
public:
	ImageIndex();
	ImageIndex(const ImageIndex& idx)	{ * this = idx; }

	ImageIndex& operator = (const ImageIndex& idx);

	void CalcHistogram(const Dib& dib);

	bool operator < (const ImageIndex& index) const;

	float Feature(float avg_weight, float std_dev_weight, float skew_weight) const;

	float Feature() const;

	String AsString() const;

	float Distance(const ImageIndex& index, float color_vs_shape_weight) const;

	const float* GetSimilarityHistogram() const		{ return histogram_; }

	// copy histogram info to the flat buffer (for saving)
	void Serialize(std::vector<uint8>& buffer);

	// restore histogram from the memory buffer
	void ConstructFromBuffer(const std::vector<uint8>& buffer);

	bool IsInitialized() const;

private:
	// image features: average, standard deviation & skewness
	// (for two color channels: Cb & Cr) for color histogram
	float average_[2];
	float std_deviation_[2];
	float skewness_[2];

	// shape histogram based on luminance channel
	float histogram_[0x100];

public:
#ifdef _DEBUG
	float distance_;
#endif

	uint32 SizeOf() const
	{ return sizeof(average_) + sizeof(std_deviation_) + sizeof(skewness_) + sizeof(histogram_); }
};


#endif // !defined(AFX_INDEX_H__E3D60A82_14A0_41E2_958E_04DFC7529EDB__INCLUDED_)
