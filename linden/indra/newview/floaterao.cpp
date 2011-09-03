/**
 * @file floaterao.cpp
 * @brief clientside animation overrider
 *
 * Copyright (c) 2011, McCabe Maxsted
 * based on work by Skills Hak
 *
 * The source code in this file ("Source Code") is provided to you
 * under the terms of the GNU General Public License, version 2.0
 * ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
 * this distribution, or online at
 * http://www.inworldz.com/izwiki/index.php/Viewers:Licenses#GNU_General_Public_License_v2.0
 *
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://www.inworldz.com/izwiki/index.php/Viewers:Licenses#FLOSS_Exception
 *
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 *
 * ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llviewerprecompiledheaders.h"

#include "floaterao.h"

#include "aoutility.h"
#include "aoengine.h"
#include "aooverride.h"

#include "llagent.h"
#include "llanimationstates.h"
#include "llfirstuse.h"
#include "llfloaterchat.h"
#include "llinventory.h"
#include "llinventorybridge.h"
#include "llinventoryview.h"
#include "llpanelinventory.h"
#include "llstartup.h"
#include "lluictrlfactory.h"
#include "llviewerregion.h"
#include "llvoavatar.h"

#include "llcheckboxctrl.h"
#include "llscrolllistctrl.h"
#include "llcombobox.h"
#include "llspinctrl.h"
// Uncomment and use instead if we ever add the chatbar as a command line - MC
//#include "chatbar_as_cmdline.h"

#include "roles_constants.h"

#include "llchat.h"


// Uncomment and use instead if we ever add the chatbar as a command line - MC
//void cmdline_printchat(std::string message);
void cmdline_printchat(std::string message)
{
    LLChat chat;
    chat.mText = message;
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
    LLFloaterChat::addChat(chat, FALSE, FALSE);
}


AONoteCardDropTarget* FloaterAO::sAOItemDropTarget;

FloaterAO::FloaterAO(const LLSD& seed)
	:
	mCombo(NULL),
	mDesc(NULL),
	mList(NULL),
	mNext(NULL),
	mPrev(NULL)
{
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_ao.xml");
}

FloaterAO::~FloaterAO()
{
	if (sAOItemDropTarget)
	{
		delete sAOItemDropTarget;
		sAOItemDropTarget = NULL;
	}
}

BOOL FloaterAO::postBuild()
{
	mCombo = getChild<LLComboBox>("anim_combo");
	mList = getChild<LLScrollListCtrl>("anim_list");
	mDesc = getChild<LLTextBox>("anim_desc");
	mNext = getChild<LLButton>("anim_next");
	mPrev = getChild<LLButton>("anim_prev");

	childSetAction("reloadcard", onClickReloadCard, this);
	childSetAction("opencard", onClickOpenCard, this);
	childSetAction("newcard", onClickNewCard, this);
	childSetAction("anim_next", onClickNext, this);
	childSetAction("anim_prev", onClickPrev, this);
	
	childSetCommitCallback("anim_list", onAnimSelected, this);
	childSetCommitCallback("anim_combo", onComboBoxCommit, this);
	childSetCommitCallback("AOEnabled", onClickToggleAO);
	childSetCommitCallback("AOSitsEnabled", onClickToggleSits);
	childSetCommitCallback("standtime", onSpinnerCommit);

	LLView *target_view = getChild<LLView>("ao_notecard");
	if (target_view)
	{
		if (sAOItemDropTarget)
		{
			delete sAOItemDropTarget;
		}
		sAOItemDropTarget = new AONoteCardDropTarget("drop target", target_view->getRect(), AOItemDrop);//, mAvatarID);
		addChild(sAOItemDropTarget);
	}

	// loads the selected notecard and enables the list
	resetNotecard();

	for (S32 i = EAO::UNKNOWN+1; i < EAO::COUNT; ++i)
	{
		std::string label = AOEngine::getInstance()->getStateList()->getLabelFromState((EAO::State)i);
		if (!label.empty())
		{
			LLSD element;
			element["id"] = i;
			element["columns"][0]["column"] = "ao_label";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = getString("list_" + label);
			mList->addElement(element, ADD_BOTTOM);
		}
	}
	mList->sortItems();
	mList->selectByValue(LLSD(AOEngine::getInstance()->getCurrentState()));

	updateLayout(AOEngine::getInstance()->getCurrentState());

	LLFirstUse::useAO();

	return TRUE;
}

void FloaterAO::resetNotecard()
{
	if (mList)
	{
		if (LLStartUp::getStartupState() == STATE_STARTED)
		{
			LLUUID item_id_import = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
			LLViewerInventoryItem* item_import = gInventory.getItem(item_id_import);
			if (item_import)
			{
				childSetTextArg("ao_nc_text", "[NAME]", item_import->getName());
				mList->setEnabled(TRUE);
			}
			else if (item_id_import.isNull())
			{
				childSetTextArg("ao_nc_text", "[NAME]", getString("nc_not_set"));
				mList->setEnabled(FALSE);
			}
			else
			{
				childSetTextArg("ao_nc_text", "[NAME]", getString("nc_not_loaded"));
				mList->setEnabled(FALSE);
			}
		}
		else
		{
			childSetTextArg("ao_nc_text", "[NAME]", getString("nc_not_logged_in"));
			mList->setEnabled(FALSE);
		}
	}
}

void FloaterAO::updateLayout(EAO::State state)
{
	LL_DEBUGS("AO") << "Loading AO options for: " << AOEngine::getInstance()->getStateList()->getLabelFromState(state) << LL_ENDL;

	mCombo->clearRows();
	mCombo->clear();
	BOOL has_overrides = FALSE;

	if ((state >= EAO::UNKNOWN) && (state < EAO::COUNT))
	{
		AOOverride* ao_override = AOEngine::getInstance()->getOverrideFromState(state);
		if (ao_override && ao_override->hasOverrides())
		{
			for (std::map<LLUUID, std::string>::iterator mIt = ao_override->mOverrideList.begin();
				 mIt != ao_override->mOverrideList.end(); ++mIt)
			{
				mCombo->add((*mIt).second, (*mIt).first, ADD_BOTTOM, TRUE);
			}
			mCombo->sortByName();
			mCombo->selectByValue(LLSD(ao_override->getSelectedOverrideID()));

			mDesc->setText(getString("text_" + AOEngine::getInstance()->getStateList()->getLabelFromState(state)));
			has_overrides = TRUE;
		}
		else
		{
			mDesc->setText(getString("text_no_overrides"));
		}
	}
	else
	{
		mDesc->setText(getString("text_unknown"));
	}

	mDesc->setEnabled(has_overrides);
	mCombo->setEnabled(has_overrides);
	mNext->setEnabled(has_overrides);
	mPrev->setEnabled(has_overrides);
}

//static 
void FloaterAO::onAnimSelected(LLUICtrl* ctrl, void* userdata)
{
	FloaterAO* self = (FloaterAO*)userdata;
	LLScrollListCtrl* list = (LLScrollListCtrl*)ctrl;
	if (self && list)
	{
		LLScrollListItem* selected = list->getFirstSelected();
		if (selected)
		{
			// the EAO::State
			self->updateLayout((EAO::State)(selected->getValue().asInteger()));
		}
	}
}

// static
void FloaterAO::onComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;
	if (box)
	{
		S32 int_state = FloaterAO::getInstance()->mList->getValue().asInteger();
		if ((int_state >= EAO::UNKNOWN) && (int_state < EAO::COUNT))
		{
			updateSelected((EAO::State)int_state, box->getCurrentID());
		}
	}
}

// static
void FloaterAO::updateSelected(EAO::State state, const LLUUID& anim_id)
{
	AOOverride* ao_override = AOEngine::getInstance()->getOverrideFromState(state);
	if (ao_override)
	{
		ao_override->setSelectedOverride(anim_id);
		if (AOEngine::getInstance()->getCurrentState() == state)
		{
			gAgent.sendAnimationRequest(AOEngine::getInstance()->getLastPlayedIDFromState(state), ANIM_REQUEST_STOP);
			gAgent.sendAnimationRequest(anim_id, ANIM_REQUEST_START);
		}
	}
}

// static
void FloaterAO::onSpinnerCommit(LLUICtrl* ctrl, void* userdata)
{
	/*LLSpinCtrl* spin = (LLSpinCtrl*)ctrl;
	if (spin)
	{
		if (spin->getName() == "standtime")
		{
			if (mAOStandTimer) mAOStandTimer->reset();
		}
	}*/
}

// static
void FloaterAO::onClickToggleAO(LLUICtrl *, void*)
{
	AOEngine::getInstance()->run();
}

// static
void FloaterAO::onClickToggleSits(LLUICtrl *, void*)
{
	AOEngine::getInstance()->run();
}

// static
//S32 FloaterAO::getAnimState()
//{
	//// Why do we need to check for anything here? -- MC
	//if (gAgent.getAvatarObject())
	//{
	//	if (gAgent.getAvatarObject()->mIsSitting)
	//	{
	//		setAnimState(EAO::SIT);
	//	}
	//	else if (gAgent.getFlying())
	//	{
	//		if (gAgent.getAvatarObject()->mBelowWater)
	//		{
	//			setAnimState(EAO::FLOAT);
	//		}
	//		else
	//		{
	//			setAnimState(EAO::HOVER);
	//		}
	//	}
	//}
	//return sAnimState;
//}

// static
void FloaterAO::AOItemDrop(LLViewerInventoryItem* item)
{
	gSavedPerAccountSettings.setString("AOConfigNotecardID", item->getUUID().asString());
	FloaterAO::getInstance()->reset();
}

void FloaterAO::reset()
{
	AOEngine::getInstance()->reset();
	FloaterAO::getInstance()->resetNotecard();
	FloaterAO::getInstance()->updateLayout(AOEngine::getInstance()->getCurrentState());
}

//// static
//LLUUID FloaterAO::getAnimID(const LLUUID& id)
//{
//	for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
//	{
//		if (iter->orig_id == id)
//		{
//			// These are special
//			if (gAgent.getAvatarObject() && gAgent.getAvatarObject()->mBelowWater && !(gAgent.getAvatarObject()->mIsSitting))
//			{
//				if (iter->orig_id == ANIM_AGENT_HOVER)
//				{
//					return getAnimIDFromState(EAO::FLOAT);
//				}
//				else if (iter->orig_id == ANIM_AGENT_FLY)
//				{
//					return getAnimIDFromState(EAO::SWIM_FORWARD);
//				}
//				else if (iter->orig_id == ANIM_AGENT_HOVER_UP)
//				{
//					return getAnimIDFromState(EAO::SWIM_UP);
//				}
//				else if (iter->orig_id == ANIM_AGENT_HOVER_DOWN)
//				{
//					return getAnimIDFromState(EAO::SWIM_DOWN);
//				}
//			}
//			return iter->ao_id;
//		}
//	}
//	return LLUUID::null;
//}


// static
void FloaterAO::onClickPrev(void* userdata)
{
	FloaterAO* self = (FloaterAO*)userdata;
	if (self)
	{
		S32 count = self->mCombo->getItemCount();
		if (count > 0)
		{
			S32 index = self->mCombo->getFirstSelectedIndex();
			index--;
			if (index > 0)
			{
				self->mCombo->selectNthItem(index);
			}
			else
			{
				self->mCombo->selectNthItem(0);
			}
			cmdline_printchat(llformat("Changing animation to %s", self->mCombo->getSimple()));
		}
	}
}

// static
void FloaterAO::onClickNext(void* userdata)
{
	FloaterAO* self = (FloaterAO*)userdata;
	if (self)
	{
		S32 count = self->mCombo->getItemCount();
		if (count > 0)
		{
			S32 index = self->mCombo->getFirstSelectedIndex();
			index++;
			if (index < count)
			{
				self->mCombo->selectNthItem(index);
			}
			else
			{
				self->mCombo->selectNthItem(0);
			}
			cmdline_printchat(llformat("Changing animation to %s", self->mCombo->getSimple()));
		}
	}
}

// static
void FloaterAO::onClickReloadCard(void* userdata)
{
	// clears the ao then populates combos
	FloaterAO::getInstance()->reset();
}

// static
void FloaterAO::onClickOpenCard(void* userdata)
{
	LLUUID configncitem = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
	if (configncitem.notNull())
	{
		const LLInventoryItem* item = gInventory.getItem(configncitem);
		if (item)
		{
			if (gAgent.allowOperation(PERM_COPY, item->getPermissions(),GP_OBJECT_MANIPULATE) || gAgent.isGodlike())
			{
				if(!item->getAssetUUID().isNull())
				open_notecard((LLViewerInventoryItem*)item, std::string("Note: ") + item->getName(), LLUUID::null, FALSE);
//				open_notecard((LLViewerInventoryItem*)item, std::string("Note: ") + item->getName(), LLUUID::null, FALSE, LLUUID::null, FALSE);
			}
		}
		else
		{
			cmdline_printchat("Could not find notecard UUID " + configncitem.asString() + " in your inventory. Make sure your inventory is fully loaded and try again.");
		}
	}
}

// static
void FloaterAO::onClickNewCard(void* userdata)
{
	// load the template file from app_settings/ao_template.ini then
	// create a new properly-formatted notecard in the user's inventory
	std::string ao_template = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "ao_template.ini");
	if (!ao_template.empty())
	{
		LLPointer<LLInventoryCallback> cb = new AONotecardCallback(ao_template);
		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							LLUUID::null, LLTransactionID::tnull, "New AO Notecard", 
							"Drop this notecard in your AO window to use", LLAssetType::AT_NOTECARD,
							LLInventoryType::IT_NOTECARD, NOT_WEARABLE, PERM_ALL, cb);
	}
	else
	{
		llwarns << "Can't find ao_template.ini in app_settings!" << llendl;
	}	
}
