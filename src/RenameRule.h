/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

struct RenameRule
{
	RenameRule() : rule_outputs_file_name_only_(true)
	{}

	String name_;
	String expression_;
	bool rule_outputs_file_name_only_;
};
