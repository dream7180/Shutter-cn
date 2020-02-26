/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

namespace {
	template<class T> struct DeleteArray
	{
		DeleteArray(T* p) : p_(p)
		{}

		~DeleteArray()
		{
			delete [] p_;
		}

	private:
		T* p_;

		DeleteArray(const DeleteArray&);
		DeleteArray& operator = (const DeleteArray&);
	};
}
