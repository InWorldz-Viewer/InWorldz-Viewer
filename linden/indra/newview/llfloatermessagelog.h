/**
 * @file floatermessagelog.cpp
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

#ifndef LL_LLFLOATERMESSAGELOG_H
#define LL_LLFLOATERMESSAGELOG_H

#include "llfloater.h"
#include "llmessagelog.h"
#include "lltemplatemessagereader.h"
#include "llfloatermessagebuilder.h"

class LLFloaterMessageLogItem : public LLMessageLogEntry
{
public:
	LLFloaterMessageLogItem(LLMessageLogEntry entry);
	~LLFloaterMessageLogItem();

	LLUUID mID;
	U32 mSequenceID;
	std::string mName;
	std::string mSummary;
	U32 mFlags;
	std::string getFull(BOOL show_header = TRUE);
	BOOL isOutgoing();

private:
	static LLTemplateMessageReader* sTemplateMessageReader;
	static std::string getString(LLTemplateMessageReader* readerp, const char* block_name, S32 block_num, const char* var_name, e_message_variable_type var_type, BOOL &returned_hex, BOOL summary_mode = FALSE);
};


class LLMessageLogFilter
{
public:
	LLMessageLogFilter();
	~LLMessageLogFilter();

	BOOL set(std::string filter);
	std::list<std::string> mPositiveNames;
	std::list<std::string> mNegativeNames;
};


class LLMessageLogFilterApply : public LLEventTimer
{
public:
	LLMessageLogFilterApply();

	void cancel();
	BOOL tick();
	S32 mProgress;
	BOOL mFinished;

private:
	std::deque<LLMessageLogEntry> mQueuedMessages;
	std::deque<LLMessageLogEntry>::iterator mIter;
};


class LLFloaterMessageLog : public LLFloater, public LLEventTimer
{
public:
	LLFloaterMessageLog();
	~LLFloaterMessageLog();

	BOOL postBuild();
	BOOL tick();
	LLNetListItem* findNetListItem(LLHost host);
	LLNetListItem* findNetListItem(LLUUID id);
	void refreshNetList();
	void refreshNetInfo(BOOL force);
	enum ENetInfoMode { NI_NET, NI_LOG };
	void setNetInfoMode(ENetInfoMode mode);
	static void show();
	static void onLog(LLMessageLogEntry entry);
	static void conditionalLog(LLFloaterMessageLogItem item);
	static void onCommitNetList(LLUICtrl* ctrl, void* user_data);
	static void onCommitMessageLog(LLUICtrl* ctrl, void* user_data);
	static void onCommitFilter(LLUICtrl* ctrl, void* user_data);
	static BOOL onClickCloseCircuit(void* user_data);
	static bool onConfirmCloseCircuit(const LLSD& notification, const LLSD& response);
	static bool onConfirmRemoveRegion(const LLSD& notification, const LLSD& response);
	static void onClickFilterApply(void* user_data);
	void startApplyingFilter(std::string filter, BOOL force);
	void stopApplyingFilter();
	void updateFilterStatus();
	static void onClickFilterChoice(void* user_data);
	static void onClickFilterMenu(void* user_data);
	static void onClickSendToMessageBuilder(void* user_data);
	static void onClickClearLog(void* user_data);

	static BOOL sBusyApplyingFilter;
	LLMessageLogFilterApply* mMessageLogFilterApply;
	static LLFloaterMessageLog* sInstance;
	static std::list<LLNetListItem*> sNetListItems;
	static std::deque<LLMessageLogEntry> sMessageLogEntries;
	static std::vector<LLFloaterMessageLogItem> sFloaterMessageLogItems;
	static LLMessageLogFilter sMessageLogFilter;
	static std::string sMessageLogFilterString;
	ENetInfoMode mNetInfoMode;
};

#endif
