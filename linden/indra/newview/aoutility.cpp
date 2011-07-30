/*
utility classes for the AO
*/

#include "llviewerprecompiledheaders.h"

#include "aoutility.h"
#include "aoengine.h"
#include "llinventory.h"
#include "llinventoryview.h"
#include "llpreviewnotecard.h"
#include "llstartup.h"
#include "lltexteditor.h"
#include "llview.h"
#include "llviewercontrol.h"


//
// AOInvTimer
//


AOInvTimer* gAOInvTimer = NULL;
BOOL AOInvTimer::sInitialized = FALSE;
static LLFrameTimer sInitTimer;

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
				sInitialized = AOEngine::getInstance()->init();
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
			AOEngine::getInstance()->init();
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
			sInitialized = AOEngine::getInstance()->init();
		}
	}
	return sInitialized;
}


//
// AONoteCardDropTarget
//


AONoteCardDropTarget::AONoteCardDropTarget(const std::string& name, const LLRect& rect, void (*callback)(LLViewerInventoryItem*))
	:
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
	if (getParent())
	{
		handled = TRUE;
		LLViewerInventoryItem* inv_item = (LLViewerInventoryItem*)cargo_data;
		if (gInventory.getItem(inv_item->getUUID()))
		{
			*accept = ACCEPT_YES_COPY_SINGLE;
			if (drop)
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


//
// AONotecardCallback
//


AONotecardCallback::AONotecardCallback(const std::string& filename)
	:
	mFileName(filename)
{
}

void AONotecardCallback::fire(const LLUUID &inv_item)
{
	if (!mFileName.empty())
	{ 
		LLPreviewNotecard* nc;
		nc = (LLPreviewNotecard*)LLPreview::find(inv_item);
		if (nc)
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
