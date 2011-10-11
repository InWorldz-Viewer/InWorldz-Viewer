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

#include "llagent.h"
#include "llvoavatar.h"
#include "llanimationstates.h"
#include "lluictrlfactory.h"
#include "llinventoryview.h"
#include "llstartup.h"
#include "llpreviewnotecard.h"
#include "llviewertexteditor.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llspinctrl.h"
// Uncomment and use instead if we ever add the chatbar as a command line - MC
//#include "chatbar_as_cmdline.h"
#include "llfloaterchat.h"
#include "llfirstuse.h"

#include "llinventory.h"
#include "llinventoryview.h"
#include "roles_constants.h"
#include "llviewerregion.h"

#include "llpanelinventory.h"
#include "llinventorybridge.h"

#include "llboost.h"
#include <boost/regex.hpp>

static LLFrameTimer sInitTimer;

// Uncomment and use instead if we ever add the chatbar as a command line - MC
//void cmdline_printchat(std::string message);
void cmdline_printchat(std::string message)
{
    LLChat chat;
    chat.mText = message;
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
    LLFloaterChat::addChat(chat, FALSE, FALSE);
}

AOInvTimer* gAOInvTimer = NULL;


class AONotecardCallback : public LLInventoryCallback
{
public:
	AONotecardCallback(std::string &filename)
	{
		mFileName = filename;
	}

	void fire(const LLUUID &inv_item)
	{
		if (!mFileName.empty())
		{ 
			LLPreviewNotecard* nc;
			nc = (LLPreviewNotecard*)LLPreview::find(inv_item);
			if(nc)
			{
				nc->open();
				LLTextEditor *text = nc->getEditor();
				if (text)
				{
					text->setText(LLStringUtil::null);
					text->makePristine();

					std::ifstream file(mFileName.c_str());
				
					std::string line;
					while (!file.eof())
					{ 
						getline(file, line);
						line = line + "\n";
						text->insertText(line);
					}
					file.close();

					if (line.empty())
					{
						llwarns << "Can't open ao_template.ini at << " << mFileName << llendl;
					}
			
					nc->saveIfNeeded();
				}
			}
		}
	}

private:
	std::string mFileName;
};


// -------------------------------------------------------

AOStandTimer* mAOStandTimer;

AOStandTimer::AOStandTimer() : LLEventTimer( gSavedSettings.getF32("AOStandInterval") )
{
	AOStandTimer::tick();
}
AOStandTimer::~AOStandTimer()
{
//	llinfos << "dead" << llendl;
}
void AOStandTimer::reset()
{
	mPeriod = gSavedSettings.getF32("AOStandInterval");
	mEventTimer.reset();
//	llinfos << "reset" << llendl;
}
BOOL AOStandTimer::tick()
{
	LLFloaterAO::sStandIterator++;
//	llinfos << "tick" << llendl;
	LLFloaterAO::changeStand();
	return FALSE;
//	return LLFloaterAO::changeStand(); //timer is always active now ..
}

// -------------------------------------------------------

BOOL AOInvTimer::sInitialized = FALSE;

AOInvTimer::AOInvTimer() : LLEventTimer( (F32)1.0 )
{
	// if we can't find the item we need, start the init timer
	// background inventory fetching has already begun in llstartup -- MC
	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		LLUUID ao_notecard = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
		if (ao_notecard.notNull())
		{
			const LLInventoryItem* item = gInventory.getItem(ao_notecard);
			if (!item)
			{
				sInitTimer.start();
				sInitTimer.setTimerExpirySec(10.0f);
			}
			else
			{
				sInitialized = LLFloaterAO::init();
				if (!sInitialized) // should never happen, but just in case -- MC
				{
					sInitTimer.start();
					sInitTimer.setTimerExpirySec(10.0f);
				}
			}
		}
	}
}

AOInvTimer::~AOInvTimer()
{
}

BOOL AOInvTimer::tick()
{
	/*if (!(gSavedSettings.getBOOL("AOEnabled")))
	{
		return TRUE;
	}*/

	if (gInventory.isEverythingFetched())
	{
		if (!sInitialized)
		{
			LLFloaterAO::init();
			sInitialized = TRUE; // this can happen even if we can't initialize the AO -- MC
		}
		
		if (sInitTimer.getStarted())
		{
			sInitTimer.stop();
		}
	}

	if (!sInitialized && LLStartUp::getStartupState() == STATE_STARTED)
	{
		if (sInitTimer.hasExpired())
		{
			sInitTimer.start();
			sInitTimer.setTimerExpirySec(10.0f);
			sInitialized = LLFloaterAO::init();
		}
	}
	return sInitialized;
}
// NC DROP -------------------------------------------------------

class AONoteCardDropTarget : public LLView
{
public:
	AONoteCardDropTarget(const std::string& name, const LLRect& rect, void (*callback)(LLViewerInventoryItem*));
	~AONoteCardDropTarget();

	void doDrop(EDragAndDropType cargo_type, void* cargo_data);

	//
	// LLView functionality
	virtual BOOL handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
								   EDragAndDropType cargo_type,
								   void* cargo_data,
								   EAcceptance* accept,
								   std::string& tooltip_msg);
protected:
	void	(*mDownCallback)(LLViewerInventoryItem*);
};


AONoteCardDropTarget::AONoteCardDropTarget(const std::string& name, const LLRect& rect,
						  void (*callback)(LLViewerInventoryItem*)) :
	LLView(name, rect, NOT_MOUSE_OPAQUE, FOLLOWS_ALL),
	mDownCallback(callback)
{
}

AONoteCardDropTarget::~AONoteCardDropTarget()
{
}

void AONoteCardDropTarget::doDrop(EDragAndDropType cargo_type, void* cargo_data)
{
//	llinfos << "AONoteCardDropTarget::doDrop()" << llendl;
}

BOOL AONoteCardDropTarget::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 std::string& tooltip_msg)
{
	BOOL handled = FALSE;
	if(getParent())
	{
		handled = TRUE;
		LLViewerInventoryItem* inv_item = (LLViewerInventoryItem*)cargo_data;
		if(gInventory.getItem(inv_item->getUUID()))
		{
			*accept = ACCEPT_YES_COPY_SINGLE;
			if(drop)
			{
				mDownCallback(inv_item);
			}
		}
		else
		{
			*accept = ACCEPT_NO;
		}
	}
	return handled;
}

AONoteCardDropTarget * LLFloaterAO::sAOItemDropTarget;


// STUFF -------------------------------------------------------

S32 LLFloaterAO::sAnimState = 0;
S32 LLFloaterAO::sStandIterator = 0;

LLUUID LLFloaterAO::sInvFolderID = LLUUID::null;
LLUUID LLFloaterAO::sCurrentStandId = LLUUID::null;

LLComboBox*				mComboBox_stands;
LLComboBox* 			mComboBox_walks;
LLComboBox* 			mComboBox_runs;
LLComboBox* 			mComboBox_jumps;
LLComboBox* 			mComboBox_sits;
LLComboBox* 			mComboBox_gsits;
LLComboBox* 			mComboBox_crouchs;
LLComboBox* 			mComboBox_cwalks;
LLComboBox* 			mComboBox_falls;
LLComboBox* 			mComboBox_hovers;
LLComboBox* 			mComboBox_flys;
LLComboBox* 			mComboBox_flyslows;
LLComboBox* 			mComboBox_flyups;
LLComboBox* 			mComboBox_flydowns;
LLComboBox* 			mComboBox_lands;
LLComboBox* 			mComboBox_standups;
LLComboBox* 			mComboBox_prejumps;
LLComboBox* 			mComboBox_typing;
LLComboBox* 			mComboBox_floating;
LLComboBox* 			mComboBox_swimmingforward;
LLComboBox* 			mComboBox_swimmingup;
LLComboBox* 			mComboBox_swimmingdown;

struct struct_overrides
{
	LLUUID orig_id;
	LLUUID ao_id;
	S32 state;
};
std::vector<struct_overrides> mAOOverrides;

struct struct_stands
{
	LLUUID ao_id;
	std::string anim_name;
};
std::vector<struct_stands> mAOStands;

struct struct_tokens
{
	std::string token;
	S32 state;
};
std::vector<struct_tokens> mAOTokens;

LLFloaterAO* LLFloaterAO::sInstance = NULL;

LLFloaterAO::LLFloaterAO()
	: 
	LLFloater(std::string("floater_ao")),
	mDirty(FALSE)
{
//	init();
	llassert_always(sInstance == NULL);
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_ao.xml");
	sInstance = this;
}

LLFloaterAO::~LLFloaterAO()
{
    sInstance = NULL;
	mComboBox_stands = NULL;
	mComboBox_walks = NULL;
	mComboBox_runs = NULL;
	mComboBox_jumps = NULL;
	mComboBox_sits = NULL;
	mComboBox_gsits = NULL;
	mComboBox_crouchs = NULL;
	mComboBox_cwalks = NULL;
	mComboBox_falls = NULL;
	mComboBox_hovers = NULL;
	mComboBox_flys = NULL;
	mComboBox_flyslows = NULL;
	mComboBox_flyups = NULL;
	mComboBox_flydowns = NULL;
	mComboBox_lands = NULL;
	mComboBox_standups = NULL;
	mComboBox_prejumps = NULL;
	mComboBox_typing = NULL;
	mComboBox_floating = NULL;
	mComboBox_swimmingforward = NULL;
	mComboBox_swimmingup = NULL;
	mComboBox_swimmingdown = NULL;

	delete sAOItemDropTarget;
	sAOItemDropTarget = NULL;
//	llinfos << "floater destroyed" << llendl;
}

BOOL LLFloaterAO::postBuild()
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
		LLUUID itemidimport = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
		LLViewerInventoryItem* itemimport = gInventory.getItem(itemidimport);
		if (itemimport)
		{
			childSetValue("ao_nc_text","Currently set to: "+itemimport->getName());
		}
		else if (itemidimport.isNull())
		{
			childSetValue("ao_nc_text","Currently not set");
		}
		else
		{
			childSetValue("ao_nc_text","Currently not loaded");
		}
	}
	else
	{
		childSetValue("ao_nc_text","Not logged in");
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
	mComboBox_stands = getChild<LLComboBox>("stands");
	mComboBox_walks = getChild<LLComboBox>("walks");
	mComboBox_runs = getChild<LLComboBox>("runs");
	mComboBox_jumps = getChild<LLComboBox>("jumps");
	mComboBox_sits = getChild<LLComboBox>("sits");
	mComboBox_gsits = getChild<LLComboBox>("gsits");
	mComboBox_crouchs = getChild<LLComboBox>("crouchs");
	mComboBox_cwalks = getChild<LLComboBox>("cwalks");
	mComboBox_falls = getChild<LLComboBox>("falls");
	mComboBox_hovers = getChild<LLComboBox>("hovers");
	mComboBox_flys = getChild<LLComboBox>("flys");
	mComboBox_flyslows = getChild<LLComboBox>("flyslows");
	mComboBox_flyups = getChild<LLComboBox>("flyups");
	mComboBox_flydowns = getChild<LLComboBox>("flydowns");
	mComboBox_lands = getChild<LLComboBox>("lands");
	mComboBox_standups = getChild<LLComboBox>("standups");
	mComboBox_prejumps = getChild<LLComboBox>("prejumps");
	mComboBox_typing = getChild<LLComboBox>("typing");
	mComboBox_floating = getChild<LLComboBox>("floating");
	mComboBox_swimmingforward = getChild<LLComboBox>("swimmingforward");
	mComboBox_swimmingup = getChild<LLComboBox>("swimmingup");
	mComboBox_swimmingdown = getChild<LLComboBox>("swimmingdown");
	getChild<LLComboBox>("stands")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("walks")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("runs")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("jumps")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("sits")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("gsits")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("crouchs")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("cwalks")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("falls")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("hovers")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("flys")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("flyslows")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("flyups")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("flydowns")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("lands")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("standups")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("prejumps")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("typing")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("floating")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("swimmingforward")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("swimmingup")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("swimmingdown")->setCommitCallback(onComboBoxCommit);

	return TRUE;
}

// static
void LLFloaterAO::show(void*)
{
    if (!sInstance)
	{
		sInstance = new LLFloaterAO();
		updateLayout(sInstance);
		init();

		sInstance->open();
	}
	else
	{
		sInstance->close();
	}
	LLFirstUse::useAO();
}

// static
LLFloaterAO* LLFloaterAO::getInstance()
{
	if (!sInstance)
	{
		sInstance = new LLFloaterAO();
	}
	return sInstance;
}

// static
bool LLFloaterAO::getVisible()
{
	if (sInstance)
	{
		return true;
	}
	return false;
}

// static
void LLFloaterAO::onSpinnerCommit(LLUICtrl* ctrl, void* userdata)
{
	LLSpinCtrl* spin = (LLSpinCtrl*)ctrl;
	if (spin)
	{
		if (spin->getName() == "standtime")
		{
			if (mAOStandTimer) mAOStandTimer->reset();
		}
	}
}

// static
void LLFloaterAO::onComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;
	if (box)
	{
		if (box->getName() == "stands")
		{
			sStandIterator = box->getCurrentIndex();
			cmdline_printchat(llformat("Changing stand to %s.", mAOStands[sStandIterator].anim_name.c_str()));
			changeStand();
		}
		else
		{
			S32 state = STATE_AGENT_IDLE;
			std::string stranim = box->getValue().asString();
//			llinfos << "state " << (gAgent.getAvatarObject()->mIsSitting) << " - " << getAnimState() << llendl;
			if (box->getName() == "walks")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_WALK), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultWalk",stranim);
				state = STATE_AGENT_WALK;
			}
			else if (box->getName() == "runs")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_RUN), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultRun",stranim);
				state = STATE_AGENT_RUN;
			}
			else if (box->getName() == "jumps")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_JUMP), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultJump",stranim);
				state = STATE_AGENT_JUMP;
			}
			else if (box->getName() == "sits")
			{
				if (gAgent.getAvatarObject() && (gSavedSettings.getBOOL("AOEnabled")) && (gSavedSettings.getBOOL("AOSitsEnabled")))
				{
					if ((gAgent.getAvatarObject()->mIsSitting) && (getAnimState() == STATE_AGENT_SIT))
					{
//						llinfos << "sitting " << getAnimID(ANIM_AGENT_SIT) << " " << getAssetIDByName(stranim) << llendl;
						gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_SIT), ANIM_REQUEST_STOP);
						gAgent.sendAnimationRequest(getAssetIDByName(stranim), ANIM_REQUEST_START);
					}
				}
				gSavedPerAccountSettings.setString("AODefaultSit",stranim);
				state = STATE_AGENT_SIT;
			}
			else if (box->getName() == "gsits")
			{
//				llinfos << "gsitting " << getAnimID(ANIM_AGENT_SIT_GROUND) << " " << getAssetIDByName(stranim) << llendl;
				if (gAgent.getAvatarObject())
				{
					if ((gAgent.getAvatarObject()->mIsSitting) && (getAnimState() == STATE_AGENT_GROUNDSIT))
					{
//						llinfos << "gsitting " << getAnimID(ANIM_AGENT_SIT_GROUND) << " " << getAssetIDByName(stranim) << llendl;
						gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_SIT_GROUND), ANIM_REQUEST_STOP);
						gAgent.sendAnimationRequest(getAssetIDByName(stranim), ANIM_REQUEST_START);
					}
				}
				gSavedPerAccountSettings.setString("AODefaultGroundSit",stranim);
				state = STATE_AGENT_GROUNDSIT;
			}
			else if (box->getName() == "crouchs")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_CROUCH), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultCrouch",stranim);
				state = STATE_AGENT_CROUCH;
			}
			else if (box->getName() == "cwalks")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_CROUCHWALK), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultCrouchWalk",stranim);
				state = STATE_AGENT_CROUCHWALK;
			}
			else if (box->getName() == "falls")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_FALLDOWN), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultFall",stranim);
				state = STATE_AGENT_FALLDOWN;
			}
			else if (box->getName() == "hovers")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_HOVER), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultHover",stranim);
				state = STATE_AGENT_HOVER;
			}
			else if (box->getName() == "flys")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_FLY), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultFly",stranim);
				state = STATE_AGENT_FLY;
			}
			else if (box->getName() == "flyslows")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_FLYSLOW), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultFlySlow",stranim);
				state = STATE_AGENT_FLYSLOW;
			}
			else if (box->getName() == "flyups")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_HOVER_UP), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultFlyUp",stranim);
				state = STATE_AGENT_HOVER_UP;
			}
			else if (box->getName() == "flydowns")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_HOVER_DOWN), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultFlyDown",stranim);
				state = STATE_AGENT_HOVER_DOWN;
			}
			else if (box->getName() == "lands")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_LAND), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultLand",stranim);
				state = STATE_AGENT_LAND;
			}
			else if (box->getName() == "standups")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_STAND), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultStandUp",stranim);
				state = STATE_AGENT_STAND;
			}
			else if (box->getName() == "prejumps")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_PRE_JUMP), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultPreJump",stranim);
				state = STATE_AGENT_PRE_JUMP;
			}
			else if (box->getName() == "typing")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_TYPE), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultTyping",stranim);
				state = STATE_AGENT_TYPING;
			}
			else if (box->getName() == "floating")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_HOVER), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultFloating",stranim);
				state = STATE_AGENT_FLOATING;
			}
			else if (box->getName() == "swimmingforward")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_FLY), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultSwimmingForward",stranim);
				state = STATE_AGENT_SWIMMINGFORWARD;
			}
			else if (box->getName() == "swimmingup")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_HOVER_UP), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultSwimmingUp",stranim);
				state =  STATE_AGENT_SWIMMINGUP;
			}
			else if (box->getName() == "swimmingdown")
			{
				gAgent.sendAnimationRequest(getAnimID(ANIM_AGENT_HOVER_DOWN), ANIM_REQUEST_STOP);
				gSavedPerAccountSettings.setString("AODefaultSwimmingDown",stranim);
				state = STATE_AGENT_SWIMMINGDOWN;
			}

			for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
			{
				if (state == iter->state)
				{
					iter->ao_id = getAssetIDByName(stranim);
				}
			}
		}
	}
}

// static
void LLFloaterAO::updateLayout(LLFloaterAO* floater)
{
	if (floater)
	{
		BOOL advanced = gSavedSettings.getBOOL( "AOAdvanced");
		if (advanced)
		{
			floater->reshape(840,380); //view->getRect().getWidth(), view->getUIWinHeightLong());
		}
		else
		{
			floater->reshape(200,380); //view->getRect().getWidth(), view->getUIWinHeightShort());
		}
		
		floater->childSetVisible("more_btn", !advanced);
		floater->childSetVisible("less_btn", advanced);

		floater->childSetVisible("textdefaultwalk", advanced);
		floater->childSetVisible("textdefaultrun", advanced);
		floater->childSetVisible("textdefaultjump", advanced);
		floater->childSetVisible("textdefaultsit", advanced);
		floater->childSetVisible("textdefaultgsit", advanced);
		floater->childSetVisible("textdefaultcrouch", advanced);
		floater->childSetVisible("textdefaultcrouchwalk", advanced);
		floater->childSetVisible("textdefaultfall", advanced);
		floater->childSetVisible("textdefaulthover", advanced);
		floater->childSetVisible("textdefaultfly", advanced);
		floater->childSetVisible("textdefaultflyslow", advanced);
		floater->childSetVisible("textdefaultflyup", advanced);
		floater->childSetVisible("textdefaultflydown", advanced);
		floater->childSetVisible("textdefaultland", advanced);
		floater->childSetVisible("textdefaultstandup", advanced);
		floater->childSetVisible("textdefaultprejump", advanced);
		floater->childSetVisible("textdefaulttyping", advanced);
		floater->childSetVisible("textdefaultfloating", advanced);
		floater->childSetVisible("textdefaultswimmingforward", advanced);
		floater->childSetVisible("textdefaultswimmingup", advanced);
		floater->childSetVisible("textdefaultswimmingdown", advanced);

		floater->childSetVisible("walks", advanced);
		floater->childSetVisible("runs", advanced);
		floater->childSetVisible("jumps", advanced);
		floater->childSetVisible("sits", advanced);
		floater->childSetVisible("gsits", advanced);
		floater->childSetVisible("crouchs", advanced);
		floater->childSetVisible("cwalks", advanced);
		floater->childSetVisible("falls", advanced);
		floater->childSetVisible("hovers", advanced);
		floater->childSetVisible("flys", advanced);
		floater->childSetVisible("flyslows", advanced);
		floater->childSetVisible("flyups", advanced);
		floater->childSetVisible("flydowns", advanced);
		floater->childSetVisible("lands", advanced);
		floater->childSetVisible("standups", advanced);
		floater->childSetVisible("prejumps", advanced);
		floater->childSetVisible("typing", advanced);
		floater->childSetVisible("floating", advanced);
		floater->childSetVisible("swimmingforward", advanced);
		floater->childSetVisible("swimmingup", advanced);
		floater->childSetVisible("swimmingdown", advanced);
	}
}

// static
bool LLFloaterAO::init()
{
	//cmdline_printchat("init() called");
	mAOStands.clear();
	mAOTokens.clear();
	mAOOverrides.clear();

	struct_tokens tokenloader;
	tokenloader.token = "[ Sitting On Ground ]";	tokenloader.state = STATE_AGENT_GROUNDSIT; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Sitting ]";				tokenloader.state = STATE_AGENT_SIT; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Crouching ]";			tokenloader.state = STATE_AGENT_CROUCH; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Crouch Walking ]";		tokenloader.state = STATE_AGENT_CROUCHWALK; mAOTokens.push_back(tokenloader); 
	tokenloader.token = "[ Standing Up ]";			tokenloader.state = STATE_AGENT_STANDUP; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Falling ]";				tokenloader.state = STATE_AGENT_FALLDOWN; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Flying Down ]";			tokenloader.state = STATE_AGENT_HOVER_DOWN; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Flying Up ]";			tokenloader.state = STATE_AGENT_HOVER_UP; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Flying Slow ]";			tokenloader.state = STATE_AGENT_FLYSLOW; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Flying ]";				tokenloader.state = STATE_AGENT_FLY; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Hovering ]";				tokenloader.state = STATE_AGENT_HOVER; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Jumping ]";				tokenloader.state = STATE_AGENT_JUMP; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Pre Jumping ]";			tokenloader.state = STATE_AGENT_PRE_JUMP; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Running ]";				tokenloader.state = STATE_AGENT_RUN; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Turning Right ]";		tokenloader.state = STATE_AGENT_TURNRIGHT; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Turning Left ]";			tokenloader.state = STATE_AGENT_TURNLEFT; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Walking ]";				tokenloader.state = STATE_AGENT_WALK; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Landing ]";				tokenloader.state = STATE_AGENT_LAND; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Standing ]";				tokenloader.state = STATE_AGENT_STAND; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Swimming Down ]";		tokenloader.state = STATE_AGENT_SWIMMINGDOWN; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Swimming Up ]";			tokenloader.state = STATE_AGENT_SWIMMINGUP; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Swimming Forward ]";		tokenloader.state = STATE_AGENT_SWIMMINGFORWARD; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Floating ]";				tokenloader.state = STATE_AGENT_FLOATING; mAOTokens.push_back(tokenloader);
	tokenloader.token = "[ Typing ]";				tokenloader.state = STATE_AGENT_TYPING; mAOTokens.push_back(tokenloader);

	struct_overrides overrideloader;
	overrideloader.orig_id = ANIM_AGENT_WALK;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_WALK;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_RUN;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_RUN;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_PRE_JUMP;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_PRE_JUMP;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_JUMP;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_JUMP;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_TURNLEFT;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_TURNLEFT;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_TURNRIGHT;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_TURNRIGHT;		mAOOverrides.push_back(overrideloader);

	overrideloader.orig_id = ANIM_AGENT_SIT;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_SIT;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_SIT_FEMALE;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_SIT;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_SIT_GENERIC;			overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_SIT;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_SIT_GROUND;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_GROUNDSIT;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_SIT_GROUND_CONSTRAINED;	overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_GROUNDSIT;		mAOOverrides.push_back(overrideloader);

	overrideloader.orig_id = ANIM_AGENT_HOVER;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_HOVER;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_HOVER_DOWN;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_HOVER_DOWN;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_HOVER_UP;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_HOVER_UP;		mAOOverrides.push_back(overrideloader);

	overrideloader.orig_id = ANIM_AGENT_CROUCH;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_CROUCH;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_CROUCHWALK;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_CROUCHWALK;		mAOOverrides.push_back(overrideloader);

	overrideloader.orig_id = ANIM_AGENT_FALLDOWN;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_FALLDOWN;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_STANDUP;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_STANDUP;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_LAND;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_LAND;			mAOOverrides.push_back(overrideloader);

	overrideloader.orig_id = ANIM_AGENT_FLY;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_FLY;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_FLYSLOW;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_FLYSLOW;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_TYPE;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_TYPING;			mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_HOVER;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_FLOATING;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_FLY;					overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_SWIMMINGFORWARD;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_HOVER_UP;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_SWIMMINGUP;		mAOOverrides.push_back(overrideloader);
	overrideloader.orig_id = ANIM_AGENT_HOVER_DOWN;				overrideloader.ao_id = LLUUID::null; overrideloader.state = STATE_AGENT_SWIMMINGDOWN;	mAOOverrides.push_back(overrideloader);

	BOOL success = FALSE;

	if (LLStartUp::getStartupState() >= STATE_INVENTORY_SEND)
	{
		LLUUID configncitem = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
		if (configncitem.notNull())
		{
			const LLInventoryItem* item = gInventory.getItem(configncitem);
			if (item)
			{
				if (gAgent.allowOperation(PERM_COPY, item->getPermissions(),GP_OBJECT_MANIPULATE) || gAgent.isGodlike())
				{
					if (!item->getAssetUUID().isNull())
					{
						LLUUID* new_uuid = new LLUUID(configncitem);
						LLHost source_sim = LLHost::invalid;
						sInvFolderID = item->getParentUUID();
						gAssetStorage->getInvItemAsset(source_sim,
														gAgent.getID(),
														gAgent.getSessionID(),
														item->getPermissions().getOwner(),
														LLUUID::null,
														item->getUUID(),
														item->getAssetUUID(),
														item->getType(),
														&onNotecardLoadComplete,
														(void*)new_uuid,
														TRUE);
						success = TRUE;
					}
				}
			}
			else // item
			{
				//cmdline_printchat("no item (notecard)");
			}
		}
	}
	else // notecard null
	{
		//cmdline_printchat("Config Notecard set to a null UUID!");
	}

//	sAnimState = 0;
//	sCurrentStandId = LLUUID::null;
//	setAnimState(STATE_AGENT_IDLE);

	return success;
}

// static
void LLFloaterAO::onClickMore(void* data)
{
	gSavedSettings.setBOOL( "AOAdvanced", TRUE );
	updateLayout(sInstance);
}

// static
void LLFloaterAO::onClickLess(void* data)
{
	gSavedSettings.setBOOL( "AOAdvanced", FALSE );
	updateLayout(sInstance);
}

// static
void LLFloaterAO::onClickToggleAO(LLUICtrl *, void*)
{
	run();
}

// static
void LLFloaterAO::onClickToggleSits(LLUICtrl *, void*)
{
	run();
}

// static
void LLFloaterAO::run()
{
	setAnimState(STATE_AGENT_IDLE); // reset state
	S32 state = getAnimState(); // check if sitting or hovering
	if ((state == STATE_AGENT_IDLE) || (state == STATE_AGENT_STAND))
	{
		if (gSavedSettings.getBOOL("AOEnabled"))
		{
			if (mAOStandTimer)
			{
				mAOStandTimer->reset();
				changeStand();
			}
			else
			{
				mAOStandTimer =	new AOStandTimer();
			}
		}
		else
		{
			stopMotion(getCurrentStandId(), FALSE, TRUE); //stop stand first then set state
			setAnimState(STATE_AGENT_IDLE);
		}
	}
	else
	{
		if (state == STATE_AGENT_SIT)
		{
			gAgent.sendAnimationRequest(getAnimIDFromState(state), (gSavedSettings.getBOOL("AOEnabled") && gSavedSettings.getBOOL("AOSitsEnabled")) ? ANIM_REQUEST_START : ANIM_REQUEST_STOP);
		}
		else
		{
			gAgent.sendAnimationRequest(getAnimIDFromState(state), gSavedSettings.getBOOL("AOEnabled") ? ANIM_REQUEST_START : ANIM_REQUEST_STOP);
		}
	}
}

// static
S32 LLFloaterAO::getAnimState()
{
	// Why do we need to check for anything here? -- MC
	if (gAgent.getAvatarObject())
	{
		if (gAgent.getAvatarObject()->mIsSitting)
		{
			setAnimState(STATE_AGENT_SIT);
		}
		else if (gAgent.getFlying())
		{
			if (gAgent.getAvatarObject()->mBelowWater)
			{
				setAnimState(STATE_AGENT_FLOATING);
			}
			else
			{
				setAnimState(STATE_AGENT_HOVER);
			}
		}
	}
	return sAnimState;
}

// static
void LLFloaterAO::setAnimState(const S32 state)
{
	sAnimState = state;
}

// static
LLUUID LLFloaterAO::getCurrentStandId()
{
	return sCurrentStandId;
}

// static
void LLFloaterAO::setCurrentStandId(const LLUUID& id)
{
	sCurrentStandId = id;
}

// static
void LLFloaterAO::AOItemDrop(LLViewerInventoryItem* item)
{
	gSavedPerAccountSettings.setString("AOConfigNotecardID", item->getUUID().asString());
	sInstance->childSetValue("ao_nc_text","Currently set to: "+item->getName());
}

// static
LLUUID LLFloaterAO::getAnimID(const LLUUID& id)
{
	for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
	{
		if (iter->orig_id == id)
		{
			// These are special
			if (gAgent.getAvatarObject() && gAgent.getAvatarObject()->mBelowWater && !(gAgent.getAvatarObject()->mIsSitting))
			{
				if (iter->orig_id == ANIM_AGENT_HOVER)
				{
					return getAnimIDFromState(STATE_AGENT_FLOATING);
				}
				else if (iter->orig_id == ANIM_AGENT_FLY)
				{
					return getAnimIDFromState(STATE_AGENT_SWIMMINGFORWARD);
				}
				else if (iter->orig_id == ANIM_AGENT_HOVER_UP)
				{
					return getAnimIDFromState(STATE_AGENT_SWIMMINGUP);
				}
				else if (iter->orig_id == ANIM_AGENT_HOVER_DOWN)
				{
					return getAnimIDFromState(STATE_AGENT_SWIMMINGDOWN);
				}
			}
			return iter->ao_id;
		}
	}
	return LLUUID::null;
}

// static
S32 LLFloaterAO::getStateFromAnimID(const LLUUID& id)
{
	for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
	{
		if (iter->orig_id == id)
		{
			return iter->state;
		}
	}
	return STATE_AGENT_IDLE;
}

// static
LLUUID LLFloaterAO::getAnimIDFromState(const S32 state)
{
	for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
	{
		if (iter->state == state)
		{
			return iter->ao_id;
		}
	}
	return LLUUID::null;
}

// static
S32 LLFloaterAO::getStateFromToken(std::string strtoken)
{
	for (std::vector<struct_tokens>::iterator iter = mAOTokens.begin(); iter != mAOTokens.end(); ++iter)
	{
		if (iter->token == strtoken)
		{
			return iter->state;
		}
	}
	return STATE_AGENT_IDLE;
}

// static
void LLFloaterAO::onClickPrevStand(void* user_data)
{
	if (mAOStands.empty())
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
	changeStand();
}

// static
void LLFloaterAO::onClickNextStand(void* user_data)
{
	if (mAOStands.empty())
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
	changeStand();
}

// static
BOOL LLFloaterAO::changeStand()
{
	if (gSavedSettings.getBOOL("AOEnabled"))
	{
		if (gAgent.getAvatarObject())
		{
			if (gSavedSettings.getBOOL("AONoStandsInMouselook") && gAgent.cameraMouselook()) return FALSE;

			if (gAgent.getAvatarObject()->mIsSitting)
			{
//				stopMotion(getCurrentStandId(), FALSE, TRUE); //stop stand first then set state
//				if (getAnimState() != STATE_AGENT_GROUNDSIT) setAnimState(STATE_AGENT_SIT);
//				setCurrentStandId(LLUUID::null);
				return FALSE;
			}
		}
		if ((getAnimState() == STATE_AGENT_IDLE) || (getAnimState() == STATE_AGENT_STAND))// stands have lowest priority
		{
			if (mAOStands.empty())
			{
				return TRUE;
			}

			if (gSavedSettings.getBOOL("AOStandRandomize"))
			{
				sStandIterator = ll_rand(mAOStands.size()-1);
			}

			if (sStandIterator < 0)
			{
				sStandIterator = S32( mAOStands.size()-sStandIterator);
			}
			
			if (sStandIterator > S32( mAOStands.size()-1))
			{
				sStandIterator = 0;
			}

			S32 sStandIterator_previous = sStandIterator -1;

			if (sStandIterator_previous < 0)
			{
				sStandIterator_previous = S32( mAOStands.size()-1);
			}
			
			if (mAOStands[sStandIterator].ao_id.notNull())
			{
				stopMotion(getCurrentStandId(), FALSE, TRUE); //stop stand first then set state
				startMotion(mAOStands[sStandIterator].ao_id, TRUE);

				setAnimState(STATE_AGENT_STAND);
				setCurrentStandId(mAOStands[sStandIterator].ao_id);
				if (sInstance && mComboBox_stands)
				{
					mComboBox_stands->selectNthItem(sStandIterator);
				}
//				llinfos << "changing stand to " << mAOStands[sStandIterator].anim_name << llendl;
				return FALSE;
			}
		}
	} 
	else
	{
		stopMotion(getCurrentStandId(), FALSE, TRUE);
		return TRUE; //stop if ao is off
	}
	return TRUE;
}


// static
BOOL LLFloaterAO::startMotion(const LLUUID& id, BOOL stand)
{
	if (stand)
	{
		// we don't care about getAnimID here -- MC
		if (gAgent.getAvatarObject() && gAgent.getAvatarObject()->mIsSitting)
		{
			return FALSE;
		}

		gAgent.sendAnimationRequest(id, ANIM_REQUEST_START);
		return TRUE;
	}
	else
	{
		if (getAnimID(id).notNull() && gSavedSettings.getBOOL("AOEnabled"))
		{
			stopMotion(getCurrentStandId(), FALSE, TRUE); //stop stand first then set state 
			setAnimState(getStateFromAnimID(id));
		
//			llinfos << " state " << getAnimState() << " start anim " << id << " overriding with " << getAnimID(id) << llendl;
			if ((getStateFromAnimID(id) == STATE_AGENT_SIT) && !(gSavedSettings.getBOOL("AOSitsEnabled"))) 
			{
				return TRUE;
			}
			gAgent.sendAnimationRequest(getAnimID(id), ANIM_REQUEST_START);
			return TRUE;
		}
	}
	return FALSE;
}

// static
BOOL LLFloaterAO::stopMotion(const LLUUID& id, BOOL stop_immediate, BOOL stand)
{	
	if (stand)
	{
		setAnimState(STATE_AGENT_IDLE);
		gAgent.sendAnimationRequest(id, ANIM_REQUEST_STOP);
		return TRUE;
	}
	else
	{
		if (id.notNull() && gSavedSettings.getBOOL("AOEnabled"))
		{
//			llinfos << "  state " << getAnimState() << "/" << getStateFromAnimID(id) << "(now 0)  stop anim " << id << " overriding with " << getAnimID(id) << llendl;
			bool change_stand = false;
			if (getAnimState() == STATE_AGENT_STAND)
			{
				change_stand = true;
			}
			if (getAnimState() == getStateFromAnimID(id))
			{
				setAnimState(STATE_AGENT_IDLE);
			}
			// startMotion(getCurrentStandId(), 0, TRUE);
			gAgent.sendAnimationRequest(getAnimID(id), ANIM_REQUEST_STOP);
			if (change_stand) changeStand(); 

			return TRUE;
		}
	}
	return FALSE;
}

// static
void LLFloaterAO::onClickReloadCard(void* user_data)
{
	LLFloaterAO::init();
}

// static
void LLFloaterAO::onClickOpenCard(void* user_data)
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
void LLFloaterAO::onClickNewCard(void* user_data)
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

struct AOAssetInfo
{
	std::string path;
	std::string name;
};

// static
void LLFloaterAO::onNotecardLoadComplete(LLVFS *vfs,const LLUUID& asset_uuid,LLAssetType::EType type,void* user_data, S32 status, LLExtStat ext_status)
{
	if (status == LL_ERR_NOERR)
	{
		S32 size = vfs->getSize(asset_uuid, type);
		U8* buffer = new U8[size];
		vfs->getData(asset_uuid, type, buffer, 0, size);

		if (type == LLAssetType::AT_NOTECARD)
		{
			LLViewerTextEditor* edit = new LLViewerTextEditor("",LLRect(0,0,0,0),S32_MAX,"");
			if(edit->importBuffer((char*)buffer, (S32)size))
			{
				llinfos << "ao nc decode success" << llendl;
				std::string card = edit->getText();
				edit->die();

				if (sInstance && mComboBox_stands)
				{
					mComboBox_stands->clear();
					mComboBox_stands->removeall();
				}
				if (sInstance && mComboBox_walks) 
					mComboBox_walks->clear();
				if (sInstance && mComboBox_runs) 
					mComboBox_runs->clear();
				if (sInstance && mComboBox_jumps) 
					mComboBox_jumps->clear();
				if (sInstance && mComboBox_sits) 
					mComboBox_sits->clear();
				if (sInstance && mComboBox_gsits) 
					mComboBox_gsits->clear();
				if (sInstance && mComboBox_crouchs) 
					mComboBox_cwalks->clear();
				if (sInstance && mComboBox_cwalks) 
					mComboBox_cwalks->clear();
				if (sInstance && mComboBox_falls) 
					mComboBox_falls->clear();
				if (sInstance && mComboBox_hovers) 
					mComboBox_hovers->clear();
				if (sInstance && mComboBox_flys) 
					mComboBox_flys->clear();
				if (sInstance && mComboBox_flyslows) 
					mComboBox_flyslows->clear();
				if (sInstance && mComboBox_flyups) 
					mComboBox_flyups->clear();
				if (sInstance && mComboBox_flydowns) 
					mComboBox_flydowns->clear();
				if (sInstance && mComboBox_lands) 
					mComboBox_lands->clear();
				if (sInstance && mComboBox_standups) 
					mComboBox_standups->clear();
				if (sInstance && mComboBox_prejumps) 
					mComboBox_prejumps->clear();
				if (sInstance && mComboBox_typing)
					mComboBox_typing->clear();
				if (sInstance && mComboBox_floating)
					mComboBox_floating->clear();
				if (sInstance && mComboBox_swimmingforward)
					mComboBox_swimmingforward->clear();
				if (sInstance && mComboBox_swimmingup)
					mComboBox_swimmingup->clear();
				if (sInstance && mComboBox_swimmingdown)
					mComboBox_swimmingdown->clear();



				struct_stands loader;

				typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
				boost::char_separator<char> sep("\n");
				tokenizer tokline(card, sep);

				for (tokenizer::iterator line = tokline.begin(); line != tokline.end(); ++line)
				{
//					llinfos << *line << llendl;
					std::string strline(*line);
//					llinfos << "uncommented line: " << strline << llendl;

					boost::regex type("^(\\s*)(\\[ )(.*)( \\])");
					boost::smatch what; 
					if (boost::regex_search(strline, what, type)) 
					{
//						llinfos << "type: " << what[0] << llendl;
//						llinfos << "anims in type: " << boost::regex_replace(strline, type, "") << llendl;

						boost::char_separator<char> sep("|,");
						std::string stranimnames(boost::regex_replace(strline, type, ""));
						tokenizer tokanimnames(stranimnames, sep);
						for (tokenizer::iterator anim = tokanimnames.begin(); anim != tokanimnames.end(); ++anim)
						{
							std::string strtoken(what[0]);
							std::string stranim(*anim);
							LLUUID animid = getAssetIDByName(stranim);

//							llinfos << sInvFolderID.asString().c_str() << llendl;
//							llinfos << "anim: " << stranim.c_str() << " assetid: " << animid << llendl;
							if (animid.isNull())
							{
								cmdline_printchat(llformat("Warning: animation '%s' could not be found (Section: %s).",stranim.c_str(),strtoken.c_str()));
							}
							else
							{
								switch(getStateFromToken(strtoken.c_str()))
								{
								case STATE_AGENT_STAND:
										loader.ao_id = animid; loader.anim_name = stranim.c_str(); mAOStands.push_back(loader);
										if (sInstance && mComboBox_stands != NULL)
										{
											mComboBox_stands->add(stranim.c_str(), ADD_BOTTOM, TRUE);
										}
										break;
								case STATE_AGENT_WALK:
										{
											if (sInstance && mComboBox_walks != NULL)
											{
												//llinfos << "1 anim: " << stranim.c_str() << " assetid: " << animid << llendl;
												if (!(mComboBox_walks->selectByValue(stranim.c_str()))) 
												{
													mComboBox_walks->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										}
										break;
								case STATE_AGENT_RUN:
										{
											if (sInstance && mComboBox_runs != NULL)
											{
												if (!(mComboBox_runs->selectByValue(stranim.c_str())))
												{
													mComboBox_runs->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										}
										break;
								case STATE_AGENT_JUMP:
										 {
											if (sInstance && mComboBox_jumps != NULL)
											{
												if (!(mComboBox_jumps->selectByValue(stranim.c_str())))
												{
													mComboBox_jumps->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_SIT:
										 {
											if (sInstance && mComboBox_sits != NULL)
											{
												if (!(mComboBox_sits->selectByValue(stranim.c_str())))
												{
													mComboBox_sits->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_GROUNDSIT:
										 {
											if (sInstance && mComboBox_gsits != NULL)
											{
												if (!(mComboBox_gsits->selectByValue(stranim.c_str())))
												{
													mComboBox_gsits->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_CROUCH:
										 {
											if (sInstance && mComboBox_crouchs != NULL)
											{
												if (!(mComboBox_crouchs->selectByValue(stranim.c_str())))
												{
													mComboBox_crouchs->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_CROUCHWALK:
										 {
											if (sInstance && mComboBox_cwalks != NULL)
											{
												if (!(mComboBox_cwalks->selectByValue(stranim.c_str())))
												{
													mComboBox_cwalks->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_FALLDOWN:
										 {
											if (sInstance && mComboBox_falls != NULL)
											{
												if (!(mComboBox_falls->selectByValue(stranim.c_str())))
												{
													mComboBox_falls->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_HOVER:
										 {
											if (sInstance && mComboBox_hovers != NULL)
											{
												if (!(mComboBox_hovers->selectByValue(stranim.c_str())))
												{
													mComboBox_hovers->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_FLY:
										 {
											if (sInstance && mComboBox_flys != NULL)
											{
												if (!(mComboBox_flys->selectByValue(stranim.c_str())))
												{
													mComboBox_flys->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_FLYSLOW:
										 {
											if (sInstance && mComboBox_flyslows != NULL)
											{
												if (!(mComboBox_flyslows->selectByValue(stranim.c_str())))
												{
													mComboBox_flyslows->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_HOVER_UP:
										 {
											if (sInstance && mComboBox_flyups != NULL)
											{
												if (!(mComboBox_flyups->selectByValue(stranim.c_str())))
												{
													mComboBox_flyups->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										break;
								case STATE_AGENT_HOVER_DOWN:
										 {
											if (sInstance && mComboBox_flydowns != NULL)
											{
												if (!(mComboBox_flydowns->selectByValue(stranim.c_str())))
												{
													mComboBox_flydowns->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_LAND:
										 {
											if (sInstance && mComboBox_lands != NULL)
											{
												if (!(mComboBox_lands->selectByValue(stranim.c_str())))
												{
													mComboBox_lands->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_STANDUP:
										 {
											if (sInstance && mComboBox_standups != NULL)
											{
												if (!(mComboBox_standups->selectByValue(stranim.c_str())))
												{
													mComboBox_standups->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_PRE_JUMP:
										 {
											if (sInstance && mComboBox_prejumps != NULL)
											{
												if (!(mComboBox_prejumps->selectByValue(stranim.c_str())))
												{
													mComboBox_prejumps->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_TYPING:
										 {
											if (sInstance && mComboBox_typing != NULL)
											{
												if (!(mComboBox_typing->selectByValue(stranim.c_str())))
												{
													mComboBox_typing->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_FLOATING:
										 {
											if (sInstance && mComboBox_floating != NULL)
											{
												if (!(mComboBox_floating->selectByValue(stranim.c_str())))
												{
													mComboBox_floating->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_SWIMMINGFORWARD:
										 {
											if (sInstance && mComboBox_swimmingforward != NULL)
											{
												if (!(mComboBox_swimmingforward->selectByValue(stranim.c_str())))
												{
													mComboBox_swimmingforward->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_SWIMMINGUP:
										 {
											if (sInstance && mComboBox_swimmingup != NULL)
											{
												if (!(mComboBox_swimmingup->selectByValue(stranim.c_str())))
												{
													mComboBox_swimmingup->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								case STATE_AGENT_SWIMMINGDOWN:
										 {
											if (sInstance && mComboBox_swimmingdown != NULL)
											{
												if (!(mComboBox_swimmingdown->selectByValue(stranim.c_str())))
												{
													mComboBox_swimmingdown->add(stranim.c_str(), ADD_BOTTOM, TRUE); //check if exist
												}
											}
										 }
										 break;
								}

								for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
								{
									if (getStateFromToken(strtoken.c_str()) == iter->state)
									{
										iter->ao_id = animid;
									}
								}
							}
						}
					} 
				}
				llinfos << "ao nc read sucess" << llendl;

				for (std::vector<struct_overrides>::iterator iter = mAOOverrides.begin(); iter != mAOOverrides.end(); ++iter)
				{
					switch(iter->state)
					{

					case STATE_AGENT_WALK:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultWalk");
							setDefault(mComboBox_walks, iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null) 
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_RUN:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultRun");
							setDefault(mComboBox_runs, iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_JUMP:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultJump");
							setDefault(mComboBox_jumps, iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null) 
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						break;
					case STATE_AGENT_SIT:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultSit");
							setDefault(mComboBox_sits, iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null) 
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_CROUCH:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultCrouch");
							setDefault(mComboBox_crouchs,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_GROUNDSIT:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultGroundSit");
							setDefault(mComboBox_gsits,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_CROUCHWALK:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultCrouchWalk");
							setDefault(mComboBox_cwalks,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_FALLDOWN:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultFall");
							setDefault(mComboBox_falls,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_HOVER:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultHover");
							setDefault(mComboBox_hovers,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_FLY:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultFly");
							setDefault(mComboBox_flys,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_HOVER_UP:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultFlyUp");
							setDefault(mComboBox_flyups,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_FLYSLOW:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultFlySlow");
							setDefault(mComboBox_flyslows,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_HOVER_DOWN:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultFlyDown");
							setDefault(mComboBox_flydowns,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_LAND:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultLand");
							setDefault(mComboBox_lands,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_STANDUP:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultStandUp");
							setDefault(mComboBox_standups,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_PRE_JUMP:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultPreJump");
							setDefault(mComboBox_prejumps,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_TYPING:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultTyping");
							setDefault(mComboBox_typing,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_FLOATING:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultFloating");
							setDefault(mComboBox_floating,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_SWIMMINGFORWARD:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultSwimmingForward");
							setDefault(mComboBox_swimmingforward,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_SWIMMINGUP:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultSwimmingUp");
							setDefault(mComboBox_swimmingup,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					case STATE_AGENT_SWIMMINGDOWN:
						{
							std::string defaultanim = gSavedPerAccountSettings.getString("AODefaultSwimmingDown");
							setDefault(mComboBox_swimmingdown,iter->ao_id,defaultanim);
							if (getAssetIDByName(defaultanim) != LLUUID::null)
							{
								iter->ao_id = getAssetIDByName(defaultanim);
							}
						}
						 break;
					}
				}
				run();
			}
			else
			{
				llinfos << "ao nc decode error" << llendl;
			}
		}
	}
	else
	{
		llinfos << "ao nc read error" << llendl;
	}
}

// static
bool LLFloaterAO::setDefault(void* userdata, LLUUID ao_id, std::string defaultanim)
{
	if (sInstance && (userdata))
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
	}
	return TRUE;
}

class ObjectNameMatches : public LLInventoryCollectFunctor
{
public:
	ObjectNameMatches(std::string name)
	{
		sName = name;
	}
	virtual ~ObjectNameMatches() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item)
	{
		if(item)
		{
			if (item->getParentUUID() == LLFloaterAO::sInvFolderID)
			{
				return (item->getName() == sName);
			}
			return false;
		}
		return false;
	}
private:
	std::string sName;
};

// static
const LLUUID& LLFloaterAO::getAssetIDByName(const std::string& name)
{
	if (name.empty())
	{
		return LLUUID::null;
	}

	LLViewerInventoryCategory::cat_array_t cats;
	LLViewerInventoryItem::item_array_t items;
	ObjectNameMatches objectnamematches(name);
	gInventory.collectDescendentsIf(LLUUID::null, cats, items, FALSE, objectnamematches);

	if (items.count())
	{
		return items[0]->getAssetUUID();
	}
	return LLUUID::null;
};
