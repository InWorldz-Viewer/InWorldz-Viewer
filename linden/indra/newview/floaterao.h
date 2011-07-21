/**
 * @file floaterao.h
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

#ifndef LL_LLFLOATERAO_H
#define LL_LLFLOATERAO_H

#include "llfloater.h"
#include "llviewercontrol.h"
#include "llagent.h"


class AONoteCardDropTarget;

const S32 STATE_AGENT_IDLE = 0;
const S32 STATE_AGENT_WALK = 1;
const S32 STATE_AGENT_RUN = 2;
const S32 STATE_AGENT_STAND = 3;

const S32 STATE_AGENT_PRE_JUMP = 4;
const S32 STATE_AGENT_JUMP = 5;
const S32 STATE_AGENT_TURNLEFT = 6;
const S32 STATE_AGENT_TURNRIGHT = 7;

const S32 STATE_AGENT_SIT = 8;
const S32 STATE_AGENT_GROUNDSIT = 9;

const S32 STATE_AGENT_HOVER = 10;
const S32 STATE_AGENT_HOVER_DOWN = 11;
const S32 STATE_AGENT_HOVER_UP = 12;

const S32 STATE_AGENT_CROUCH = 13;
const S32 STATE_AGENT_CROUCHWALK = 14;
const S32 STATE_AGENT_FALLDOWN = 15;
const S32 STATE_AGENT_STANDUP = 16;
const S32 STATE_AGENT_LAND = 17;

const S32 STATE_AGENT_FLY = 18;
const S32 STATE_AGENT_FLYSLOW = 19;

const S32 STATE_AGENT_TYPING = 20;

const S32 STATE_AGENT_FLOATING = 21;
const S32 STATE_AGENT_SWIMMINGFORWARD = 22;
const S32 STATE_AGENT_SWIMMINGUP = 23;
const S32 STATE_AGENT_SWIMMINGDOWN = 24;


class LLFrameTimer;
class LLComboBox;

class AOStandTimer : public LLEventTimer
{
public:
    AOStandTimer();
    ~AOStandTimer();
    virtual BOOL tick();
	virtual void reset();
};

class AOInvTimer : public LLEventTimer
{
public:
	AOInvTimer();
	~AOInvTimer();
	BOOL tick();

private:
	static BOOL sInitialized;
};

class LLFloaterAO : public LLFloater
{
public:

    LLFloaterAO();
	virtual	BOOL	postBuild();
    virtual ~LLFloaterAO();

	static S32 sStandIterator;
	static LLUUID sInvFolderID;

	static void show(void*);
	static bool init();

	static void onClickToggleAO(LLUICtrl *, void*);
	static void onClickToggleSits(LLUICtrl *, void*);
	static void run();
	static void updateLayout(LLFloaterAO* floater);

	static BOOL loadAnims();

	static S32 getAnimState();
	static void setAnimState(S32 state);
	static void setStates(const LLUUID& id, BOOL start);

	static LLUUID getCurrentStandId();
	static void setCurrentStandId(const LLUUID& id);
	static BOOL changeStand();

	static BOOL startMotion(const LLUUID& id, BOOL stand = FALSE);
	static BOOL stopMotion(const LLUUID& id, BOOL stop_immediate, BOOL stand = FALSE);

	static LLUUID getAnimID(const LLUUID& id);
	static S32 getStateFromAnimID(const LLUUID& id);
	static LLUUID getAnimIDFromState(const S32 state);
	static S32 getStateFromToken(std::string strtoken);

	static void onClickLess(void* data) ;
	static void onClickMore(void* data) ;

	static void onClickPrevStand(void* userdata);
	static void onClickNextStand(void* userdata);
	static void onClickReloadCard(void* userdata);
	static void onClickOpenCard(void* userdata);
	static void onClickNewCard(void* userdata);

	static const LLUUID& getAssetIDByName(const std::string& name);

	static LLFloaterAO* getInstance();
	static bool getVisible();
	
private:

	static LLFloaterAO* sInstance;
	static S32 sAnimState;
	static LLUUID sCurrentStandId;

	static AONoteCardDropTarget* sAOItemDropTarget;
	static void AOItemDrop(LLViewerInventoryItem* item);
	static void onSpinnerCommit(LLUICtrl* ctrl, void* userdata);
	static void onComboBoxCommit(LLUICtrl* ctrl, void* userdata);
	static bool setDefault(void *userdata, LLUUID ao_id, std::string defaultanim);

	BOOL					mDirty;

protected:

	static void onNotecardLoadComplete(LLVFS *vfs,const LLUUID& asset_uuid,LLAssetType::EType type,void* user_data, S32 status, LLExtStat ext_status);

};

extern AOInvTimer* gAOInvTimer;

#endif
