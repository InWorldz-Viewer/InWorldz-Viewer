/** 
 * @file llpanelskins.cpp
 * @brief General preferences panel in preferences floater
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include <boost/filesystem.hpp>

#include "llpanelskins.h"

// linden library includes
#include "llradiogroup.h"
#include "llbutton.h"
#include "llscrolllistctrl.h"
#include "lltextbox.h"
#include "lliconctrl.h"
#include "lluictrlfactory.h"
#include "llnotifications.h"

// project includes
#include "llviewercontrol.h"
#include "llviewerwindow.h"


void copyTemplateToSkin(const boost::filesystem::path& template_path, const boost::filesystem::path& new_path)
{
	bool f_exists = false;
	try
	{
		f_exists = boost::filesystem::exists(status(template_path));
	}
	catch (boost::filesystem::basic_filesystem_error<boost::filesystem::path> e)
	{
		f_exists = false;
	}
	if (f_exists)
	{
		boost::filesystem::copy_file(template_path, new_path);
	}
	else
	{
		llinfos << template_path << " not found!" << llendl;
	}
}


LLPanelSkins::LLPanelSkins()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_skins.xml");
}

LLPanelSkins::~LLPanelSkins()
{
}

BOOL LLPanelSkins::postBuild()
{
	mSkinsList = getChild<LLScrollListCtrl>("SkinsList");
	mSkinsList->setCommitCallback(onSelectSkin);
 	mSkinsList->setCallbackUserData(this);

	childSetAction("btn_create_new", onClickNewSkin, this);

	getChild<LLIconCtrl>("preview_image")->setImage("default_no_skin_preview.png");

	populateSkins();

	return TRUE;
}

void LLPanelSkins::refresh()
{
	if (mSkinsList->getChildCount() <= 0)
	{
		return;
	}

	std::string skin_selection = mSkinsList->getValue().asString();
	LLStringUtil::toLower(skin_selection);
	std::string preview_filename = "skin_thumbnail_" + skin_selection + ".png";

	std::string full_path = gDirUtilp->findSkinnedFilename("textures", "interface", preview_filename);
	if (full_path.empty())	
	{
		getChild<LLIconCtrl>("preview_image")->setImage("default_no_skin_preview.png");
		getChild<LLTextBox>("no_preview_text")->setVisible(TRUE);
	}
	else
	{
		getChild<LLIconCtrl>("preview_image")->setImage(preview_filename);
		getChild<LLTextBox>("no_preview_text")->setVisible(FALSE);
	}
}

void LLPanelSkins::apply()
{
	if (mSkinsList->getSelectedValue().asString() != gSavedSettings.getString("SkinCurrent"))
	{
		LLNotifications::instance().add("ChangeSkin");
		gSavedSettings.setString("SkinCurrent", mSkinsList->getSelectedValue().asString());
	}
}

void LLPanelSkins::cancel()
{
	// reverts any changes to current skin
	//gSavedSettings.setString("SkinCurrent", mSkinCurrent);
}

//static
void LLPanelSkins::populateSkins()
{
	std::string skin_selection = mSkinsList->getValue().asString();
	if (skin_selection.empty())
	{
		skin_selection = gSavedSettings.getString("SkinCurrent");
	}
	//S32 scroll_pos = mSkinsList->getScrollPos();

	mSkinsList->clearRows();

	std::string skin_dir = gDirUtilp->getSkinBaseDir();
	boost::filesystem::path path_skins(skin_dir);

	try
	{
		boost::filesystem::exists(path_skins);
	}
	catch (boost::filesystem::basic_filesystem_error<boost::filesystem::path> e)
	{
		llwarns << path_skins << " could not be found. HOW CAN THIS BE?!?!" << llendl;
		return;
	}

	std::vector<std::string> skin_names;
	boost::filesystem::directory_iterator end_itr; // default ctor is end
	for (boost::filesystem::directory_iterator itr(path_skins); itr != end_itr; ++itr)
	{
		if (boost::filesystem::is_directory(itr->status()))
		{
			//llinfos << "paths found: " << itr->path() << llendl;
			skin_names.push_back(itr->filename());
		}
	}

	llstat s;
	for (S32 i = 0; i < (S32)skin_names.size(); ++i)
	{
		// only bother with folders that actually change things
		bool valid_skin = false;

		std::string texture_path = gDirUtilp->getSkinBaseDir() + gDirUtilp->getDirDelimiter() +
									skin_names[i] + gDirUtilp->getDirDelimiter() + "colors_base.xml";
		
		if (!(S32)LLFile::stat(texture_path, &s))
		{
			valid_skin = true;
		}
		else
		{
			texture_path = gDirUtilp->getSkinBaseDir() + gDirUtilp->getDirDelimiter() + skin_names[i] +
							gDirUtilp->getDirDelimiter() + "textures" + 
							gDirUtilp->getDirDelimiter() + "textures.xml";

			if (!(S32)LLFile::stat(texture_path, &s))
			{
				valid_skin = true;
			}
		}

		if (valid_skin)
		{
			llinfos << "Found skin: " << skin_names[i] << llendl;
			LLSD element;
			element["id"] = skin_names[i];
			element["columns"][0]["column"] = "skin_name";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = skin_names[i];
			mSkinsList->addElement(element, ADD_BOTTOM);
		}
	}

	mSkinsList->sortItems();
	mSkinsList->setSelectedByValue(LLSD(skin_selection), TRUE);
	//mSkinsList->setScrollPos(scroll_pos);

	refresh();
}

//static
void LLPanelSkins::onSelectSkin(LLUICtrl* ctrl, void* data)
{
	LLPanelSkins* self = (LLPanelSkins*)data;
	if (self)
	{
		self->refresh();
	}
}

//static 
void LLPanelSkins::onClickNewSkin(void* data)
{
	LLPanelSkins* self = (LLPanelSkins*)data;
	if (self)
	{
		LLNotifications::instance().add("NewSkinName", 
		LLSD(), 
		LLSD(), 
		boost::bind(&newSkinCallback, _1, _2, self));
	}
}

// static
bool LLPanelSkins::newSkinCallback(const LLSD& notification, const LLSD& response, LLPanelSkins* self)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if (option == 0)
	{
		std::string skin_name = response["message"].asString();
		std::string trimmed_skin_name(skin_name);
		LLStringUtil::trim(trimmed_skin_name);

		if (skin_name.empty() || 
			LLStringUtil::containsNonprintable(skin_name) || 
			(skin_name.length() != trimmed_skin_name.length()) ||
			(self->mSkinsList->getItem(LLSD(skin_name)) != NULL))
		{
			LLNotifications::instance().add("InvalidSkinName", LLSD(), LLSD());
		}
		else
		{
			llinfos << "Creating files and folders for new skin: " << skin_name << llendl;
			std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_SKINS, skin_name);
			boost::filesystem::path new_skin(filename);

			bool dir_created = false;
			try
			{
				dir_created = boost::filesystem::create_directory(new_skin);
			}
			catch (boost::filesystem::basic_filesystem_error<boost::filesystem::path> e)
			{
				dir_created = false;
				llwarns << "Couldn't create new skin directory: " << new_skin.filename() << llendl;
			}

			if (dir_created)
			{
				boost::filesystem::create_directory(boost::filesystem::path(filename + gDirUtilp->getDirDelimiter() + "textures"));
				boost::filesystem::create_directory(boost::filesystem::path(filename + gDirUtilp->getDirDelimiter() + "textures" + gDirUtilp->getDirDelimiter() + "interface"));
				boost::filesystem::create_directory(boost::filesystem::path(filename + gDirUtilp->getDirDelimiter() + "xui"));
				boost::filesystem::create_directory(boost::filesystem::path(filename + gDirUtilp->getDirDelimiter() + "xui" + gDirUtilp->getDirDelimiter() + "en-us"));

				boost::filesystem::path colors_temp_from(gDirUtilp->getSkinBaseDir() + gDirUtilp->getDirDelimiter() + "colors_template.xml");
				boost::filesystem::path colors_temp_to(filename + gDirUtilp->getDirDelimiter() + "colors.xml");
				copyTemplateToSkin(colors_temp_from, colors_temp_to);

				boost::filesystem::path base_temp_from(gDirUtilp->getSkinBaseDir() + gDirUtilp->getDirDelimiter() + "colors_base_template.xml");
				boost::filesystem::path base_temp_to(filename + gDirUtilp->getDirDelimiter() + "colors_base.xml");
				copyTemplateToSkin(base_temp_from, base_temp_to);

				boost::filesystem::path textures_temp_from(gDirUtilp->getSkinBaseDir() + gDirUtilp->getDirDelimiter() + "textures_template.xml");
				boost::filesystem::path textures_temp_to(filename + gDirUtilp->getDirDelimiter() + "textures" + gDirUtilp->getDirDelimiter() + "textures.xml");
				copyTemplateToSkin(textures_temp_from, textures_temp_to);
			}

			self->populateSkins();
			self->mSkinsList->setSelectedByValue(LLSD(skin_name), TRUE);
		}
	}
	return false;
}
