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

#ifndef LL_FLOATERAO_H
#define LL_FLOATERAO_H

#include "llfloater.h"
#include "aoutility.h"
#include "aostate.h"
/////#include "llviewercontrol.h"
/////#include "llagent.h"

class AOEngine;
class AOEntry;
class AONoteCardDropTarget;
class LLFrameTimer;
class LLComboBox;


class FloaterAO : public LLFloater, public LLFloaterSingleton<FloaterAO> 
{
public:
    FloaterAO(const LLSD& seed);
	/*virtual*/ ~FloaterAO();

	/*virtual*/	BOOL postBuild();

    

	static S32 sStandIterator;

	// When the AO is tied to the UI, initializing will always have to be done
	//static bool init();

	static void onClickToggleAO(LLUICtrl *, void*);
	static void onClickToggleSits(LLUICtrl *, void*);

	void init();

	static BOOL loadAnims();

	/*static S32 getAnimState();
	static void setAnimState(S32 state);
	static void setStates(const LLUUID& id, BOOL start);*/

	static LLUUID getCurrentStandId();
	static void setCurrentStandId(const LLUUID& id);

	//static LLUUID getAnimID(const LLUUID& id);
	//static S32 getStateFromAnimID(const LLUUID& id);
	//static LLUUID getAnimIDFromState(const S32 state);
	//static S32 getStateFromToken(std::string strtoken);

	static void onClickLess(void* data);
	static void onClickMore(void* data);

	static void onClickPrevStand(void* userdata);
	static void onClickNextStand(void* userdata);
	static void onClickReloadCard(void* userdata);
	static void onClickOpenCard(void* userdata);
	static void onClickNewCard(void* userdata);

	//static const LLUUID& getAssetIDByName(const std::string& name);

	 static void updateSelected(EAOState::State state, const LLUUID& anim_id);
	
private:

	LLComboBox*				mCombo_stands;
	LLComboBox* 			mCombo_walks;
	LLComboBox* 			mCombo_runs;
	LLComboBox* 			mCombo_jumps;
	LLComboBox* 			mCombo_sits;
	LLComboBox* 			mCombo_gsits;
	LLComboBox* 			mCombo_crouchs;
	LLComboBox* 			mCombo_cwalks;
	LLComboBox* 			mCombo_falls;
	LLComboBox* 			mCombo_hovers;
	LLComboBox* 			mCombo_flys;
	LLComboBox* 			mCombo_flyslows;
	LLComboBox* 			mCombo_flyups;
	LLComboBox* 			mCombo_flydowns;
	LLComboBox* 			mCombo_lands;
	LLComboBox* 			mCombo_standups;
	LLComboBox* 			mCombo_prejumps;
	LLComboBox* 			mCombo_typing;
	LLComboBox* 			mCombo_floating;
	LLComboBox* 			mCombo_swimmingforward;
	LLComboBox* 			mCombo_swimmingup;
	LLComboBox* 			mCombo_swimmingdown;
	LLComboBox* 			mCombo_customize;

	void updateLayout();

	void updateAOCombo(LLComboBox* combo, EAOState::State state);

public:

	//static S32 sAnimState;
	static LLUUID sCurrentStandId;

	static AONoteCardDropTarget* sAOItemDropTarget;
	static void AOItemDrop(LLViewerInventoryItem* item);
	static void onSpinnerCommit(LLUICtrl* ctrl, void* userdata);
	static void onComboBoxCommit(LLUICtrl* ctrl, void* userdata);
	static bool setDefault(void *userdata, LLUUID ao_id, std::string defaultanim);

protected:

	//

};

#endif
