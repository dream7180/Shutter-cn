/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Read vector of values from the registry into 'values'
//
// Note: this function function is only intended for simple vector base types (like int)
//

template<class T>
bool GetProfileVector(const TCHAR* section, const TCHAR* entry, std::vector<T>& values)
{
	values.clear();
	BYTE* data= 0;
	UINT bytes= 0;
	AfxGetApp()->GetProfileBinary(section, entry,  &data, &bytes);
	if (data && bytes)
	{
		struct Data
		{
			DWORD count;
			DWORD elem_size;
			T vector[1];
		};
		const Data* vector= reinterpret_cast<const Data*>(data);

		if (bytes < 2 * sizeof DWORD || vector->elem_size != sizeof T)
		{
			ASSERT(false);
			delete [] data;
			return false;
		}

		// memory leak possible here
		values.reserve(vector->count);

		for (DWORD i= 0; i < vector->count; ++i)
			values.push_back(vector->vector[i]);

		delete [] data;
		return true;
	}

	return false;
}


// Write vector of values to the registry
//
// Note: this function function is only intended for simple vector base types (like int)
//

template<class T>
bool WriteProfileVector(const TCHAR* section, const TCHAR* entry, const std::vector<T>& values)
{
	std::vector<BYTE> buffer;
	struct Data
	{
		DWORD count;
		DWORD elem_size;
		T vector[1];
//		static int Size(int count)		{ return 2 * sizeof DWORD + count * sizeof T; }
	};

//	buffer.resize(Data::Size(values.size()));		// reserve buffer space
	buffer.resize(2 * sizeof DWORD + values.size() * sizeof T);	   // reserve buffer space
	Data* data= reinterpret_cast<Data*>(&buffer.front());

	data->count = static_cast<DWORD>(values.size());
	data->elem_size = sizeof T;

	for (DWORD i= 0; i < data->count; ++i)
		data->vector[i] = values[i];

	return !!AfxGetApp()->WriteProfileBinary(section, entry, &buffer.front(), static_cast<UINT>(buffer.size()));
}
