/**
 * @file floatermessagebuilder.h
 *
 * The source code in this file ("Source Code") is provided to you
 * under the terms of the GNU General Public License, version 2.0
 * ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
 * this distribution
 *
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution
 *
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 *
 * ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#ifndef LL_LLFLOATERMESSAGEBUILDER_H
#define LL_LLFLOATERMESSAGEBUILDER_H

#include "llfloater.h"
#include "lltemplatemessagereader.h"
#include "llmessagelog.h"

class LLNetListItem
{
public:
	LLNetListItem(LLUUID id);

	LLUUID mID;
	BOOL mAutoName;
	std::string mName;
	std::string mPreviousRegionName;
	LLCircuitData* mCircuitData;
};


class LLFloaterMessageBuilder : public LLFloater, public LLEventTimer
{
public:
	LLFloaterMessageBuilder(std::string initial_text);
	~LLFloaterMessageBuilder();

	void refreshNetList();
	BOOL postBuild();
	BOOL handleKeyHere(KEY key, MASK mask);
	BOOL tick();
	static void show(std::string initial_text);
	static LLNetListItem* findNetListItem(LLHost host);
	static LLNetListItem* findNetListItem(LLUUID id);
	static BOOL addField(e_message_variable_type var_type, const char* var_name, std::string input, BOOL hex);
	static void onClickSend(void* user_data);
	static void onCommitPacketCombo(LLUICtrl* ctrl, void* user_data);
	static void onCommitMessageLog(LLUICtrl* ctrl, void* user_data);
	static void onCommitFilter(LLUICtrl* ctrl, void* user_data);

	struct parts_var
	{
		std::string name;
		std::string value;
		BOOL hex;
		e_message_variable_type var_type;
	};

	struct parts_block
	{
		std::string name;
		std::vector<parts_var> vars;
	};

	enum ENetInfoMode { NI_NET, NI_LOG };
	ENetInfoMode mNetInfoMode;
	static std::list<LLNetListItem*> sNetListItems;
	static LLFloaterMessageBuilder* sInstance;
	std::string mInitialText;
};

#endif
