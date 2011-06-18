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

#include "boost/filesystem.hpp"

#include "llpanelskins.h"

// linden library includes
#include "llradiogroup.h"
#include "llbutton.h"
#include "llscrolllistctrl.h"
#include "lliconctrl.h"
#include "lluictrlfactory.h"

// project includes
#include "llviewercontrol.h"
#include "llviewerwindow.h"

LLPanelSkins::LLPanelSkins()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_skins.xml");
	populateSkins();
}

LLPanelSkins::~LLPanelSkins()
{
}

BOOL LLPanelSkins::postBuild()
{
	mSkinsList = getChild<LLScrollListCtrl>("SkinsList");
	mSkinsList->setCommitCallback(onSelectSkin);
 	mSkinsList->setCallbackUserData(this);

	getChild<LLIconCtrl>("preview_image")->setImage("default_no_skin_preview.png");

	//refresh();
	return TRUE;
}

void LLPanelSkins::refresh()
{
	std::string skin_selection = mSkinsList->getValue().asString();
	LLStringUtil::toLower(skin_selection);
	std::string preview_filename = "skin_thumbnail_" + skin_selection + ".png";

	std::string full_path = gDirUtilp->findSkinnedFilename("textures", "interface", preview_filename);
	if (full_path.empty())	
	{
		getChild<LLIconCtrl>("preview_image")->setImage("default_no_skin_preview.png");	
	}
	else
	{
		getChild<LLIconCtrl>("preview_image")->setImage(preview_filename);
	}
}

void LLPanelSkins::apply()
{
	if (mSkinsList->getSelectedValue().asString() != gSavedSettings.getString("SkinCurrent"))
	{
		LLNotifications::instance().add("ChangeSkin");
		gSavedSettings.setString("SkinCurrent", mSkinsList->getSelectedValue().asString());
		//refresh();
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
	std::string skin_dir = gDirUtilp->getSkinBaseDir();
	boost::filesystem::path path_skins(skin_dir);

	if (!boost::filesystem::exists(path_skins))
	{
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
		std::string texture_path = gDirUtilp->getSkinBaseDir() + skin_names[i] +
									gDirUtilp->getDirDelimiter() + "xui" + 
									gDirUtilp->getDirDelimiter() + "colors_base.xml";

		S32 stat_result = LLFile::stat(texture_path, &s);
		if (!stat_result)
		{
			texture_path = gDirUtilp->getSkinBaseDir() + skin_names[i] +
							gDirUtilp->getDirDelimiter() + "textures" + 
							gDirUtilp->getDirDelimiter() + "textures.xml";
			stat_result = LLFile::stat(texture_path, &s);
			if (!stat_result)
			{
				continue;
			}
		}

		llinfos << "Found skin: " << skin_names[i] << llendl;

		// add names to ui list
		LLSD element;
		element["id"] = skin_names[i];
		element["columns"][0]["column"] = "skin_name";
		element["columns"][0]["type"] = "text";
		element["columns"][0]["value"] = skin_names[i];
		mSkinsList->addElement(element, ADD_BOTTOM);
	}

	mSkinsList->setSelectedByValue(gSavedSettings.getString("SkinCurrent"), TRUE);
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
