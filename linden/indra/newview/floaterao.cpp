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
#include "lluictrlfactory.h"
#include "llinventoryview.h"
#include "llstartup.h"
#include "llvoavatar.h"

#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llspinctrl.h"
// Uncomment and use instead if we ever add the chatbar as a command line - MC
//#include "chatbar_as_cmdline.h"
#include "llfloaterchat.h"
#include "llfirstuse.h"

#include "llinventory.h"
#include "llviewerregion.h"
#include "roles_constants.h"
#include "llpanelinventory.h"
#include "llinventorybridge.h"

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
S32 FloaterAO::sStandIterator = 0;
LLUUID FloaterAO::sCurrentStandId = LLUUID::null;


FloaterAO::FloaterAO(const LLSD& seed)
	:
	mCombo_stands(NULL),
	mCombo_walks(NULL),
	mCombo_runs(NULL),
	mCombo_jumps(NULL),
	mCombo_sits(NULL),
	mCombo_gsits(NULL),
	mCombo_crouchs(NULL),
	mCombo_cwalks(NULL),
	mCombo_falls(NULL),
	mCombo_hovers(NULL),
	mCombo_flys(NULL),
	mCombo_flyslows(NULL),
	mCombo_flyups(NULL),
	mCombo_flydowns(NULL),
	mCombo_lands(NULL),
	mCombo_standups(NULL),
	mCombo_prejumps(NULL),
	mCombo_typing(NULL),
	mCombo_floating(NULL),
	mCombo_swimmingforward(NULL),
	mCombo_swimmingup(NULL),
	mCombo_swimmingdown(NULL)
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

	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		LLUUID item_id_import = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
		LLViewerInventoryItem* item_import = gInventory.getItem(item_id_import);
		if (item_import)
		{
			childSetValue("ao_nc_text", "Currently set to: " + item_import->getName());
		}
		else if (item_id_import.isNull())
		{
			childSetValue("ao_nc_text", "Currently not set");
		}
		else
		{
			childSetValue("ao_nc_text", "Currently not loaded");
		}
	}
	else
	{
		childSetValue("ao_nc_text", "Not logged in");
	}

	childSetAction("more_btn", onClickMore, this);
	childSetAction("less_btn", onClickLess, this);

	childSetAction("reloadcard",onClickReloadCard,this);
	childSetAction("opencard",onClickOpenCard,this);
	childSetAction("newcard",onClickNewCard,this);
	childSetAction("prevstand",onClickPrevStand,this);
	childSetAction("nextstand",onClickNextStand,this);
	childSetCommitCallback("AOEnabled",onClickToggleAO);
	childSetCommitCallback("AOSitsEnabled",onClickToggleSits);
	childSetCommitCallback("standtime",onSpinnerCommit);

	mCombo_stands = getChild<LLComboBox>("stands");
	mCombo_walks = getChild<LLComboBox>("walks");
	mCombo_runs = getChild<LLComboBox>("runs");
	mCombo_jumps = getChild<LLComboBox>("jumps");
	mCombo_sits = getChild<LLComboBox>("sits");
	mCombo_gsits = getChild<LLComboBox>("gsits");
	mCombo_crouchs = getChild<LLComboBox>("crouchs");
	mCombo_cwalks = getChild<LLComboBox>("cwalks");
	mCombo_falls = getChild<LLComboBox>("falls");
	mCombo_hovers = getChild<LLComboBox>("hovers");
	mCombo_flys = getChild<LLComboBox>("flys");
	mCombo_flyslows = getChild<LLComboBox>("flyslows");
	mCombo_flyups = getChild<LLComboBox>("flyups");
	mCombo_flydowns = getChild<LLComboBox>("flydowns");
	mCombo_lands = getChild<LLComboBox>("lands");
	mCombo_standups = getChild<LLComboBox>("standups");
	mCombo_prejumps = getChild<LLComboBox>("prejumps");
	mCombo_typing = getChild<LLComboBox>("typing");
	mCombo_floating = getChild<LLComboBox>("floating");
	mCombo_swimmingforward = getChild<LLComboBox>("swimmingforward");
	mCombo_swimmingup = getChild<LLComboBox>("swimmingup");
	mCombo_swimmingdown = getChild<LLComboBox>("swimmingdown");
	mCombo_customize = getChild<LLComboBox>("customize");

	mCombo_stands->setCallbackUserData(this);
	mCombo_walks->setCallbackUserData(this);
	mCombo_runs->setCallbackUserData(this);
	mCombo_jumps->setCallbackUserData(this);
	mCombo_sits->setCallbackUserData(this);
	mCombo_gsits->setCallbackUserData(this);
	mCombo_crouchs->setCallbackUserData(this);
	mCombo_cwalks->setCallbackUserData(this);
	mCombo_falls->setCallbackUserData(this);
	mCombo_hovers->setCallbackUserData(this);
	mCombo_flys->setCallbackUserData(this);
	mCombo_flyslows->setCallbackUserData(this);
	mCombo_flyups->setCallbackUserData(this);
	mCombo_flydowns->setCallbackUserData(this);
	mCombo_lands->setCallbackUserData(this);
	mCombo_standups->setCallbackUserData(this);
	mCombo_prejumps->setCallbackUserData(this);
	mCombo_typing->setCallbackUserData(this);
	mCombo_floating->setCallbackUserData(this);
	mCombo_swimmingforward->setCallbackUserData(this);
	mCombo_swimmingup->setCallbackUserData(this);
	mCombo_swimmingdown->setCallbackUserData(this);
	mCombo_customize->setCallbackUserData(this);

	mCombo_stands->setCommitCallback(onComboBoxCommit);
	mCombo_walks->setCommitCallback(onComboBoxCommit);
	mCombo_runs->setCommitCallback(onComboBoxCommit);
	mCombo_jumps->setCommitCallback(onComboBoxCommit);
	mCombo_sits->setCommitCallback(onComboBoxCommit);
	mCombo_gsits->setCommitCallback(onComboBoxCommit);
	mCombo_crouchs->setCommitCallback(onComboBoxCommit);
	mCombo_cwalks->setCommitCallback(onComboBoxCommit);
	mCombo_falls->setCommitCallback(onComboBoxCommit);
	mCombo_hovers->setCommitCallback(onComboBoxCommit);
	mCombo_flys->setCommitCallback(onComboBoxCommit);
	mCombo_flyslows->setCommitCallback(onComboBoxCommit);
	mCombo_flyups->setCommitCallback(onComboBoxCommit);
	mCombo_flydowns->setCommitCallback(onComboBoxCommit);
	mCombo_lands->setCommitCallback(onComboBoxCommit);
	mCombo_standups->setCommitCallback(onComboBoxCommit);
	mCombo_prejumps->setCommitCallback(onComboBoxCommit);
	mCombo_typing->setCommitCallback(onComboBoxCommit);
	mCombo_floating->setCommitCallback(onComboBoxCommit);
	mCombo_swimmingforward->setCommitCallback(onComboBoxCommit);
	mCombo_swimmingup->setCommitCallback(onComboBoxCommit);
	mCombo_swimmingdown->setCommitCallback(onComboBoxCommit);
	mCombo_customize->setCommitCallback(onComboBoxCommit);

	init();

	updateLayout();

	LLFirstUse::useAO();

	return TRUE;
}

void FloaterAO::init()
{
	//EAOState::TURN_LEFT
	//EAOState::TURN_RIGHT
	//EAOState::LAND_MEDIUM
	//EAOState::SHOUT
	//EAOState::CUSTOMIZE
	//EAOState::CUSTOMIZE_DONE

	updateAOCombo(mCombo_stands, EAOState::STAND);
	updateAOCombo(mCombo_walks, EAOState::WALK);
	updateAOCombo(mCombo_runs, EAOState::RUN);
	updateAOCombo(mCombo_jumps, EAOState::JUMP);
	updateAOCombo(mCombo_sits, EAOState::SIT);
	updateAOCombo(mCombo_gsits, EAOState::SIT_GROUND);
	updateAOCombo(mCombo_crouchs, EAOState::CROUCH);
	updateAOCombo(mCombo_cwalks, EAOState::WALK_CROUCH);
	updateAOCombo(mCombo_falls, EAOState::FALL);
	updateAOCombo(mCombo_hovers, EAOState::HOVER);
	updateAOCombo(mCombo_flys, EAOState::FLY);
	updateAOCombo(mCombo_flyslows, EAOState::FLY_SLOW);
	updateAOCombo(mCombo_flyups, EAOState::HOVER_UP);
	updateAOCombo(mCombo_flydowns, EAOState::HOVER_DOWN);
	updateAOCombo(mCombo_lands, EAOState::LAND);
	updateAOCombo(mCombo_standups, EAOState::STANDUP);
	updateAOCombo(mCombo_prejumps, EAOState::PRE_JUMP);
	updateAOCombo(mCombo_typing, EAOState::TYPE);
	updateAOCombo(mCombo_floating, EAOState::FLOAT);
	updateAOCombo(mCombo_swimmingforward, EAOState::SWIM_FORWARD);
	updateAOCombo(mCombo_swimmingup, EAOState::SWIM_UP);
	updateAOCombo(mCombo_swimmingdown, EAOState::SWIM_DOWN);
	updateAOCombo(mCombo_customize, EAOState::CUSTOMIZE);
}

void FloaterAO::updateAOCombo(LLComboBox* combo, EAOState::State state)
{
	if (combo)
	{
		combo->clearRows();
		AOOverride* ao_override = AOEngine::getInstance()->getOverrideFromState(state);
		if (ao_override && ao_override->hasOverrides())
		{
			for (std::map<LLUUID, std::string>::iterator mIt = ao_override->mOverrideList.begin();
				 mIt != ao_override->mOverrideList.end(); ++mIt)
			{
				combo->add((*mIt).second, (*mIt).first, ADD_BOTTOM, TRUE);
			}
		}
	}
}

void FloaterAO::updateLayout()
{
	BOOL advanced = gSavedSettings.getBOOL( "AOAdvanced");
	if (advanced)
	{
		FloaterAO::getInstance()->reshape(840,380); //view->getRect().getWidth(), view->getUIWinHeightLong());
	}
	else
	{
		FloaterAO::getInstance()->reshape(200,380); //view->getRect().getWidth(), view->getUIWinHeightShort());
	}
	
	FloaterAO::getInstance()->childSetVisible("more_btn", !advanced);
	FloaterAO::getInstance()->childSetVisible("less_btn", advanced);

	FloaterAO::getInstance()->childSetVisible("textdefaultwalk", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultrun", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultjump", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultsit", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultgsit", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultcrouch", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultcrouchwalk", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultfall", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaulthover", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultfly", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultflyslow", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultflyup", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultflydown", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultland", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultstandup", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultprejump", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaulttyping", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultfloating", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultswimmingforward", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultswimmingup", advanced);
	FloaterAO::getInstance()->childSetVisible("textdefaultswimmingdown", advanced);

	FloaterAO::getInstance()->childSetVisible("walks", advanced);
	FloaterAO::getInstance()->childSetVisible("runs", advanced);
	FloaterAO::getInstance()->childSetVisible("jumps", advanced);
	FloaterAO::getInstance()->childSetVisible("sits", advanced);
	FloaterAO::getInstance()->childSetVisible("gsits", advanced);
	FloaterAO::getInstance()->childSetVisible("crouchs", advanced);
	FloaterAO::getInstance()->childSetVisible("cwalks", advanced);
	FloaterAO::getInstance()->childSetVisible("falls", advanced);
	FloaterAO::getInstance()->childSetVisible("hovers", advanced);
	FloaterAO::getInstance()->childSetVisible("flys", advanced);
	FloaterAO::getInstance()->childSetVisible("flyslows", advanced);
	FloaterAO::getInstance()->childSetVisible("flyups", advanced);
	FloaterAO::getInstance()->childSetVisible("flydowns", advanced);
	FloaterAO::getInstance()->childSetVisible("lands", advanced);
	FloaterAO::getInstance()->childSetVisible("standups", advanced);
	FloaterAO::getInstance()->childSetVisible("prejumps", advanced);
	FloaterAO::getInstance()->childSetVisible("typing", advanced);
	FloaterAO::getInstance()->childSetVisible("floating", advanced);
	FloaterAO::getInstance()->childSetVisible("swimmingforward", advanced);
	FloaterAO::getInstance()->childSetVisible("swimmingup", advanced);
	FloaterAO::getInstance()->childSetVisible("swimmingdown", advanced);
	FloaterAO::getInstance()->childSetVisible("customize", advanced);
}

// static
void FloaterAO::onComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;
	if (box)
	{
		if (gSavedSettings.getBOOL("AOEnabled"))
		{
			cmdline_printchat("Changing " + box->getName() + " to " + box->getSimple());
		}

		if (box->getName() == "stands")
		{
			sStandIterator = box->getCurrentIndex();
			gSavedPerAccountSettings.setString("AODefaultStand", box->getSimple());
			updateSelected(EAOState::STAND, box->getCurrentID());
		}
		if (box->getName() == "walks")
		{
			gSavedPerAccountSettings.setString("AODefaultWalk", box->getSimple());
			updateSelected(EAOState::WALK, box->getCurrentID());
		}
		else if (box->getName() == "runs")
		{
			gSavedPerAccountSettings.setString("AODefaultRun", box->getSimple());
			updateSelected(EAOState::RUN, box->getCurrentID());
		}
		else if (box->getName() == "jumps")
		{
			gSavedPerAccountSettings.setString("AODefaultJump", box->getSimple());
			updateSelected(EAOState::JUMP, box->getCurrentID());
		}
		else if (box->getName() == "sits")
		{
			if (gAgent.getAvatarObject() && (gSavedSettings.getBOOL("AOEnabled")) && (gSavedSettings.getBOOL("AOSitsEnabled")))
			{
				if ((gAgent.getAvatarObject()->mIsSitting) && (AOEngine::getInstance()->getCurrentState() == EAOState::SIT))
				{
					gAgent.sendAnimationRequest(AOEngine::getInstance()->getLastPlayedIDFromState(EAOState::SIT), ANIM_REQUEST_STOP);
					gAgent.sendAnimationRequest(box->getCurrentID(), ANIM_REQUEST_START);
				}
			}
			gSavedPerAccountSettings.setString("AODefaultSit", box->getSimple());
			updateSelected(EAOState::SIT, box->getCurrentID());
		}
		else if (box->getName() == "gsits")
		{
			if (gAgent.getAvatarObject())
			{
				if ((gAgent.getAvatarObject()->mIsSitting) && (AOEngine::getInstance()->getCurrentState() == EAOState::SIT_GROUND))
				{
					gAgent.sendAnimationRequest(AOEngine::getInstance()->getLastPlayedIDFromState(EAOState::SIT_GROUND), ANIM_REQUEST_STOP);
					gAgent.sendAnimationRequest(box->getCurrentID(), ANIM_REQUEST_START);
				}
			}
			gSavedPerAccountSettings.setString("AODefaultGroundSit", box->getSimple());
			updateSelected(EAOState::SIT_GROUND, box->getCurrentID());
		}
		else if (box->getName() == "crouchs")
		{
			gSavedPerAccountSettings.setString("AODefaultCrouch", box->getSimple());
			updateSelected(EAOState::CROUCH, box->getCurrentID());
		}
		else if (box->getName() == "cwalks")
		{
			gSavedPerAccountSettings.setString("AODefaultCrouchWalk", box->getSimple());
			updateSelected(EAOState::WALK_CROUCH, box->getCurrentID());
		}
		else if (box->getName() == "falls")
		{
			gSavedPerAccountSettings.setString("AODefaultFall", box->getSimple());
			updateSelected(EAOState::FALL, box->getCurrentID());
		}
		else if (box->getName() == "hovers")
		{
			gSavedPerAccountSettings.setString("AODefaultHover", box->getSimple());
			updateSelected(EAOState::HOVER, box->getCurrentID());
		}
		else if (box->getName() == "flys")
		{
			gSavedPerAccountSettings.setString("AODefaultFly", box->getSimple());
			updateSelected(EAOState::FLY, box->getCurrentID());
		}
		else if (box->getName() == "flyslows")
		{
			gSavedPerAccountSettings.setString("AODefaultFlySlow", box->getSimple());
			updateSelected(EAOState::FLY_SLOW, box->getCurrentID());
		}
		else if (box->getName() == "flyups")
		{
			gSavedPerAccountSettings.setString("AODefaultFlyUp", box->getSimple());
			updateSelected(EAOState::HOVER_UP, box->getCurrentID());
		}
		else if (box->getName() == "flydowns")
		{
			gSavedPerAccountSettings.setString("AODefaultFlyDown", box->getSimple());
			updateSelected(EAOState::HOVER_DOWN, box->getCurrentID());
		}
		else if (box->getName() == "lands")
		{
			gSavedPerAccountSettings.setString("AODefaultLand", box->getSimple());
			updateSelected(EAOState::LAND, box->getCurrentID());
		}
		else if (box->getName() == "standups")
		{
			gSavedPerAccountSettings.setString("AODefaultStandUp", box->getSimple());
			updateSelected(EAOState::STAND, box->getCurrentID());
		}
		else if (box->getName() == "prejumps")
		{
			gSavedPerAccountSettings.setString("AODefaultPreJump", box->getSimple());
			updateSelected(EAOState::PRE_JUMP, box->getCurrentID());
		}
		else if (box->getName() == "typing")
		{
			gSavedPerAccountSettings.setString("AODefaultTyping", box->getSimple());
			updateSelected(EAOState::TYPE, box->getCurrentID());
		}
		else if (box->getName() == "floating")
		{
			gSavedPerAccountSettings.setString("AODefaultFloating", box->getSimple());
			updateSelected(EAOState::FLOAT, box->getCurrentID());
		}
		else if (box->getName() == "swimmingforward")
		{
			gSavedPerAccountSettings.setString("AODefaultSwimmingForward", box->getSimple());
			updateSelected(EAOState::SWIM_FORWARD, box->getCurrentID());
		}
		else if (box->getName() == "swimmingup")
		{
			gSavedPerAccountSettings.setString("AODefaultSwimmingUp", box->getSimple());
			updateSelected( EAOState::SWIM_UP, box->getCurrentID());
		}
		else if (box->getName() == "swimmingdown")
		{
			gSavedPerAccountSettings.setString("AODefaultSwimmingDown", box->getSimple());
			updateSelected(EAOState::SWIM_DOWN, box->getCurrentID());
		}
		else if (box->getName() == "customize")
		{
			gSavedPerAccountSettings.setString("AODefaultCustomize", box->getSimple());
			updateSelected(EAOState::CUSTOMIZE, box->getCurrentID());
		}
	}
}

// static
void FloaterAO::updateSelected(EAOState::State state, const LLUUID& anim_id)
{
	AOOverride* ao_override = AOEngine::getInstance()->getOverrideFromState(state);
	if (ao_override)
	{
		ao_override->setSelectedOverride(anim_id);
		gAgent.sendAnimationRequest(AOEngine::getInstance()->getLastPlayedIDFromState(state), ANIM_REQUEST_STOP);
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
void FloaterAO::onClickMore(void* data)
{
	gSavedSettings.setBOOL( "AOAdvanced", TRUE );
	FloaterAO::getInstance()->updateLayout();
}

// static
void FloaterAO::onClickLess(void* data)
{
	gSavedSettings.setBOOL( "AOAdvanced", FALSE );
	FloaterAO::getInstance()->updateLayout();
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
	//		setAnimState(EAOState::SIT);
	//	}
	//	else if (gAgent.getFlying())
	//	{
	//		if (gAgent.getAvatarObject()->mBelowWater)
	//		{
	//			setAnimState(EAOState::FLOAT);
	//		}
	//		else
	//		{
	//			setAnimState(EAOState::HOVER);
	//		}
	//	}
	//}
	//return sAnimState;
//}

// static
//void FloaterAO::setAnimState(const S32 state)
//{
//	sAnimState = state;
//}

// static
LLUUID FloaterAO::getCurrentStandId()
{
	return sCurrentStandId;
}

// static
void FloaterAO::setCurrentStandId(const LLUUID& id)
{
	sCurrentStandId = id;
}

// static
void FloaterAO::AOItemDrop(LLViewerInventoryItem* item)
{
	gSavedPerAccountSettings.setString("AOConfigNotecardID", item->getUUID().asString());
	FloaterAO::getInstance()->childSetValue("ao_nc_text", "Currently set to: " + item->getName());
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
//					return getAnimIDFromState(EAOState::FLOAT);
//				}
//				else if (iter->orig_id == ANIM_AGENT_FLY)
//				{
//					return getAnimIDFromState(EAOState::SWIM_FORWARD);
//				}
//				else if (iter->orig_id == ANIM_AGENT_HOVER_UP)
//				{
//					return getAnimIDFromState(EAOState::SWIM_UP);
//				}
//				else if (iter->orig_id == ANIM_AGENT_HOVER_DOWN)
//				{
//					return getAnimIDFromState(EAOState::SWIM_DOWN);
//				}
//			}
//			return iter->ao_id;
//		}
//	}
//	return LLUUID::null;
//}


// static
void FloaterAO::onClickPrevStand(void* user_data)
{
	/*if (mAOStands.empty())
	{
		return;
	}
	
	sStandIterator = sStandIterator-1;

	if (sStandIterator < 0)
	{
		sStandIterator = S32( mAOStands.size()-sStandIterator);
	}
	if (sStandIterator > S32( mAOStands.size()-1))
	{
		sStandIterator = 0;
	}

	cmdline_printchat(llformat("Changing stand to %s.",mAOStands[sStandIterator].anim_name.c_str()));
	changeStand();*/
}

// static
void FloaterAO::onClickNextStand(void* user_data)
{
	/*if (mAOStands.empty())
	{
		return;
	}

	sStandIterator = sStandIterator+1;

	if (sStandIterator < 0)
	{
		sStandIterator = S32( mAOStands.size()-sStandIterator);
	}
	if (sStandIterator > S32( mAOStands.size()-1))
	{
		sStandIterator = 0;
	}

	cmdline_printchat(llformat("Changing stand to %s.",mAOStands[sStandIterator].anim_name.c_str()));
	changeStand();*/
}

//// static
//BOOL FloaterAO::changeStand()
//{
//	if (gSavedSettings.getBOOL("AOEnabled"))
//	{
//		if (gAgent.getAvatarObject())
//		{
//			if (gSavedSettings.getBOOL("AONoStandsInMouselook") && gAgent.cameraMouselook()) return FALSE;
//
//			if (gAgent.getAvatarObject()->mIsSitting)
//			{
////				stopMotion(getCurrentStandId(), FALSE, TRUE); //stop stand first then set state
////				if (getAnimState() != EAOState::SIT_GROUND) setAnimState(EAOState::SIT);
////				setCurrentStandId(LLUUID::null);
//				return FALSE;
//			}
//		}
//		if ((getAnimState() == STATE_AGENT_IDLE) || (getAnimState() == EAOState::STAND))// stands have lowest priority
//		{
//			if (mAOStands.empty())
//			{
//				return TRUE;
//			}
//
//			if (gSavedSettings.getBOOL("AOStandRandomize"))
//			{
//				sStandIterator = ll_rand(mAOStands.size()-1);
//			}
//
//			if (sStandIterator < 0)
//			{
//				sStandIterator = S32( mAOStands.size()-sStandIterator);
//			}
//			
//			if (sStandIterator > S32( mAOStands.size()-1))
//			{
//				sStandIterator = 0;
//			}
//
//			S32 sStandIterator_previous = sStandIterator -1;
//
//			if (sStandIterator_previous < 0)
//			{
//				sStandIterator_previous = S32( mAOStands.size()-1);
//			}
//			
//			if (mAOStands[sStandIterator].ao_id.notNull())
//			{
//				stopMotion(getCurrentStandId(), FALSE, TRUE); //stop stand first then set state
//				startMotion(mAOStands[sStandIterator].ao_id, TRUE);
//
//				setAnimState(EAOState::STAND);
//				setCurrentStandId(mAOStands[sStandIterator].ao_id);
//				if (sInstance && mCombo_stands)
//				{
//					mCombo_stands->selectNthItem(sStandIterator);
//				}
////				llinfos << "changing stand to " << mAOStands[sStandIterator].anim_name << llendl;
//				return FALSE;
//			}
//		}
//	} 
//	else
//	{
//		stopMotion(getCurrentStandId(), FALSE, TRUE);
//		return TRUE; //stop if ao is off
//	}
//	return TRUE;
//}

// static
void FloaterAO::onClickReloadCard(void* user_data)
{
	// clear ao then populate combos
	AOEngine::getInstance()->reset();
	FloaterAO::getInstance()->init();
}

// static
void FloaterAO::onClickOpenCard(void* user_data)
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
void FloaterAO::onClickNewCard(void* user_data)
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

// static
bool FloaterAO::setDefault(void* userdata, LLUUID ao_id, std::string defaultanim)
{
	/*if (sInstance && (userdata))
	{
		LLComboBox *box = (LLComboBox *) userdata;
		if (LLUUID::null == ao_id)
		{
			box->clear();
			box->removeall();
		}
		else
		{
			box->selectByValue(defaultanim);
		}
	}*/
	return TRUE;
}
