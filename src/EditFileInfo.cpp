/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "EditFileInfo.h"
#include "PropertyDlg.h"
#include "CatchAll.h"
#include "PhotoInfo.h"
#include "HeaderDialog.h"
#include "ItemIdList.h"
#include "XMPAccess.h"
#include "StringConversions.h"
#include "DbOperations.h"
#include "ApplyMetadataThread.h"
#include "PhotoCollection.h"
#include "ProcessingProgressDlg.h"
#include "Exception.h"


EditFileInfo::EditFileInfo(VectPhotoInfo& all_photos, VectPhotoInfo& selected_photos, CWnd* parent, const PhotoChangedFn& fn)
	: all_photos_(all_photos), selected_photos_(selected_photos), parent_wnd_(parent),
		current_photo_(0), photo_changed_(fn), multi_edit_(false)
{
	photos_ = 0;
}


void LoadMetadata(CPropertyDlg* dlg, PhotoInfoPtr photo)
{
	CWaitCursor wait;

	XmpData xmpmeta;
	if (!photo->GetMetadata(xmpmeta))
	{
		// Note: description from my field is assigned to xmp descr. field only if xmp record is not present
		// Once it is written it takes precedence over other descriptions
#ifdef _UNICODE
		xmpmeta.Description = photo->photo_desc_;
#else
		::WideStringToMultiByte(photo->photo_desc_, xmpmeta.Description);
#endif
	}

	dlg->PhotoLoaded(photo, xmpmeta);
}


// find next (previous) photo for editing (skipping those that do not support XMP)
size_t FindPhoto(VectPhotoInfo* photos, size_t current_photo, int step)
{
	for (;;)
	{
		current_photo += step;

		if (current_photo < photos->size())
		{
			int errCode= 0;
			if ((*photos)[current_photo]->CanEditIPTC(errCode))
				return current_photo;
		}
		else
			return ~0;
	}
}


// folder with templates (it will be created if does not exist)
Path GetTemplateDir()
{
	Path templates= GetDocumentsFolder(_T("c:\\"));

	templates.AppendDir(_T("文件信息模板"));

	if (!templates.IsFolder())
		templates.CreateFolders();

	return templates;
}


static TCHAR* XMP_FILTER= _T("XMP 模板文件 (*.xmp)|*.xmp|所有文件 (*.*)|*.*||");
static TCHAR* XMP_EXT= _T("XMP");

// save XMP file in the template folder
void SaveTemplate(const XmpData& xmp)
{
	Path dir= GetTemplateDir();

	CFileDialog dlg(false, XMP_EXT, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, XMP_FILTER);
	dlg.m_pOFN->lpstrInitialDir = dir.c_str();

	if (dlg.DoModal() != IDOK)
		return;

	Xmp::SaveToXmpFile(xmp, dlg.GetPathName());
}

// load XMP file
bool LoadTemplate(XmpData& xmp)
{
	Path dir= GetTemplateDir();

	CFileDialog dlg(true, XMP_EXT, 0, OFN_HIDEREADONLY, XMP_FILTER);
	dlg.m_pOFN->lpstrInitialDir = dir.c_str();

	if (dlg.DoModal() != IDOK)
		return false;

	Xmp::LoadFromXmpFile(xmp, dlg.GetPathName());

	return true;
}


void FindTemplates(std::vector<Path>& templates, size_t limit)
{
	templates.clear();

	Path dir= GetTemplateDir();
	dir.AppendMask(_T("*.xmp"));

	CFileFind finder;

	if (!finder.FindFile(dir.c_str()))
		return;

	size_t count= 1;
	for (bool next= true; next; ++count)
	{
		next = !!finder.FindNextFile();

		templates.push_back(Path(finder.GetFilePath()));

		if (limit > 0 && count >= limit)
			break;
	}

	// sort template files?

	sort(templates.begin(), templates.end());
}


bool LoadTemplate(int index, XmpData& xmp)
{
	std::vector<Path> templates;
	FindTemplates(templates, index + 1);

	if (templates.size() != index + 1)
		return false;

	Xmp::LoadFromXmpFile(xmp, templates[index].c_str());

	return true;
}


extern void SaveMetadata(PhotoInfo& photo, const XmpData& xmp, bool notify, const PhotoInfo::WriteAccessFn& fn)
{
	//Path path= photo.GetPhysicalPath();

	// save to the photo or XMP file
	photo.SaveMetadata(xmp, fn);
	// store changes in the photo's object and sync tags
	photo.SetMetadata(xmp);

	// restore original time
	//CFile::SetStatus(path.c_str(), status);

	// invalidate DB record
	//MarkRecordAsStale(path.c_str());

	// notify about changes
	if (notify)
		PhotoCollection::Instance().FireMetadataChanged(&photo);
}


static void SaveMetadata(VectPhotoInfo& photos, const XmpData& xmp, CWnd* parent)
{
	ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ApplyMetadataThread(photos, xmp, 0)));

	ProcessingProgressDlg progress(parent, proc, _T("正在保存文件信息"),
		_T("正在保存元数据"), ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::OUTPUT_ONLY);

	progress.DoModal();

	// notify about changes; not all 'photos' might have changed, but still
	PhotoCollection::Instance().FireMetadataChanged(photos);

/* this is in-line implementation:

	size_t written= 0;

	const size_t count= photos.size();

	for (size_t i= 0; i < count; ++i)
	{
		PhotoInfoPtr p= photos[i];

		int errCode= 0;
		if (!p->CanEditIPTC(errCode))
			continue;

		try
		{
			SaveMetadata(*p, xmp);

			++written;
		}
		catch (CException* ex)
		{
			TCHAR err[MAX_PATH];
			ex->GetErrorMessage(err, MAX_PATH);
			String msg= _T("Error writing file info metadata for '") + p->GetOriginalPath() + _T("' image.\n\nError: ") + err +
				_T("\n\nContinue writing?");
			if (parent->MessageBox(msg.c_str(), 0, MB_OKCANCEL | MB_ICONERROR) == IDCANCEL)
				break;
		}
		catch (String& err)
		{
			String msg= _T("Error writing file info metadata for '") + p->GetOriginalPath() + _T("' image.\n\nError: ") + err +
				_T("\n\nContinue writing?");
			if (parent->MessageBox(msg.c_str(), 0, MB_OKCANCEL | MB_ICONERROR) == IDCANCEL)
				break;
		}
	}

	if (written < count)
	{
		oStringstream ost;
		ost << _T("File info metadata was recorded in ") << written << _T(" image(s).\n\n");
		ost << _T("For ") << count - written << _T(" image(s) saving metadata failed.");
		parent->MessageBox(ost.str().c_str(), 0, MB_OK | MB_ICONWARNING);
	}
*/
}


bool EditFileInfo::FileInfoCallback(CPropertyDlg* dlg, CPropertyDlg::Action action, PhotoInfoPtr photo, const XmpData& xmp, bool modified, int index)
{
	try
	{
		switch (action)
		{
		case CPropertyDlg::Save:

			//TODO: remove my comment in app marker
			//update IPTC

			if (multi_edit_)
			{
				CWaitCursor wait;
				SaveMetadata(selected_photos_, xmp, dlg);
				dlg->SaveHistory();
			}
			else if (photo && modified)
			{
				SaveMetadata(*photo, xmp, true, PhotoInfo::WriteAccessFn());
				dlg->SaveHistory();
			}

			dlg->EndDialog(IDOK);
			break;

		case CPropertyDlg::Cancel:
			dlg->EndDialog(IDCANCEL);
			break;

		case CPropertyDlg::Previous:
			if (photo && modified)
			{
				SaveMetadata(*photo, xmp, true, PhotoInfo::WriteAccessFn());
				dlg->SaveHistory();
			}
			// get previous
			{
				size_t index= FindPhoto(photos_, current_photo_, -1);
				if (index < photos_->size())
				{
					LoadMetadata(dlg, photos_->at(current_photo_ = index));
					photo_changed_(*photos_->at(current_photo_));
				}
			}
			break;

		case CPropertyDlg::Next:
			if (photo && modified)
			{
				SaveMetadata(*photo, xmp, true, PhotoInfo::WriteAccessFn());
				dlg->SaveHistory();
			}
			// get next
			{
				size_t index= FindPhoto(photos_, current_photo_, +1);
				if (index < photos_->size())
				{
					LoadMetadata(dlg, photos_->at(current_photo_ = index));
					photo_changed_(*photos_->at(current_photo_));
				}
			}
			break;

		case CPropertyDlg::Load:
			ASSERT(photos_ != 0 && !photos_->empty());
			LoadMetadata(dlg, photos_->at(current_photo_));
			break;

		case CPropertyDlg::SaveTemplate:
			SaveTemplate(xmp);
			dlg->SaveHistory();
			break;

		case CPropertyDlg::LoadTemplate:
			//TODO: warn if data modified?
			{
				XmpData meta;
				bool loaded= false;

				if (index >= 0)
					loaded = LoadTemplate(index, meta);	// load given template
				else
					loaded = LoadTemplate(meta);

				if (loaded)
					dlg->SetXmp(meta, false);
			}
			break;

		case CPropertyDlg::HasPrevious:
			{
				size_t index= FindPhoto(photos_, current_photo_, -1);
				return index < photos_->size();
			}

		case CPropertyDlg::HasNext:
			{
				size_t index= FindPhoto(photos_, current_photo_, +1);
				return index < photos_->size();
			}

		default:
			ASSERT(false);
			break;
		}
	}
	CATCH_ALL_W(dlg)

	return false;
}


void EditFileInfo::PopupMenuInit(CMenu* popup, int cmd_id)
{
	try
	{
		std::vector<Path> templates;
		FindTemplates(templates, 0);

		if (templates.empty())
			return;

		popup->AppendMenu(MF_SEPARATOR);

		int key= 0;
		for (size_t i= 0; i < templates.size(); ++i, ++key)
		{
			String name= templates[i].GetFileName();
			if (key < 9)
			{
				// silent assumption regarding shortcut keys (Alt+digit) handled by PropertyDlg...
				name += _T("\tAlt+");
				name += TCHAR('1' + key);
			}
			popup->AppendMenu(MF_STRING, cmd_id++, name.c_str());
		}

	}
	CATCH_ALL_W(AfxGetMainWnd())
}


bool EditFileInfo::DoModal()
{
	if (selected_photos_.empty())
		return false;

	PhotoInfo& photo= *selected_photos_[0];

	int err_code= 0;
	if (!photo.CanEditIPTC(err_code))
	{
		String msg= err_code == -1 ?
			_T("图像 '") + photo.GetOriginalPath() + _T("'\n不能打开以写入.") :
			_T("此类型图像不支持编辑文件信息.");
		parent_wnd_->MessageBox(msg.c_str(), 0, MB_OK | MB_ICONWARNING);
		return false;
	}

	try
	{
		bool enable_next_prev= false;
		const size_t count= selected_photos_.size();

		if (count > 1)
		{
			// multiple images selected
			photos_ = &selected_photos_;

			multi_edit_ = true;

			enable_next_prev = false;
			// next & prev buttons are hidden: the same file info will be written to all the selected images
		}
		else
		{
			// single image selected
			photos_ = &all_photos_;

			multi_edit_ = false;

			// find index of selected image on the list of all images
			current_photo_ = std::find(photos_->begin(), photos_->end(), &photo) - photos_->begin();

			enable_next_prev = all_photos_.size() > 1;
			// when single image is selected FileInfo will enable next & previous buttons
		}

		CPropertyDlg dlg(boost::bind(&EditFileInfo::FileInfoCallback, this, _1, _2, _3, _4, _5, _6),
			boost::bind(&EditFileInfo::PopupMenuInit, this, _1, _2), parent_wnd_, count, enable_next_prev, _T("FileInfoDlg"));

		HeaderDialog dlgHdr(dlg, _T("文件信息"), HeaderDialog::IMG_PENCIL);
		dlgHdr.SetTitleDuringInit(false);	// file info dlg provides its own title
		return dlgHdr.DoModal() == IDOK;
	}
	CATCH_ALL_W(parent_wnd_)

	return false;
}
