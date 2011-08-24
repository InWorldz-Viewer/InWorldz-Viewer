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

class AOEngine;
class AOEntry;
class AONoteCardDropTarget;
class LLFrameTimer;
class LLComboBox;
class LLScrollListCtrl;
class LLTextBox;
class LLButton;

class FloaterAO : public LLFloater, public LLFloaterSingleton<FloaterAO> 
{
public:
    FloaterAO(const LLSD& seed);
	/*virtual*/ ~FloaterAO();

	/*virtual*/	BOOL postBuild();

	void reset();
	void resetNotecard();

	static void onClickToggleAO(LLUICtrl*, void*);
	static void onClickToggleSits(LLUICtrl*, void*);
	static void onClickPrev(void* userdata);
	static void onClickNext(void* userdata);
	static void onClickReloadCard(void* userdata);
	static void onClickOpenCard(void* userdata);
	static void onClickNewCard(void* userdata);
	static void onAnimSelected(LLUICtrl* ctrl, void* userdata);
	static void onSpinnerCommit(LLUICtrl* ctrl, void* userdata);
	static void onComboBoxCommit(LLUICtrl* ctrl, void* userdata);

	static void updateSelected(EAO::State state, const LLUUID& anim_id);

	static void AOItemDrop(LLViewerInventoryItem* item);

	static AONoteCardDropTarget* sAOItemDropTarget;

private:

	LLComboBox* mCombo;
	LLScrollListCtrl* mList;
	LLTextBox* mDesc;
	LLButton* mNext;
	LLButton* mPrev;

	void updateLayout(EAO::State state);
};

#endif
