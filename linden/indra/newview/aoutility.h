/*
utility classes for the AO
*/

#ifndef AO_UTILITY_H
#define AO_UTILITY_H

#include "llview.h"
#include "llinventory.h"
#include "llinventoryview.h"


//
// AOInvTimer
//


class AOInvTimer : public LLEventTimer
{
public:
	AOInvTimer();
	~AOInvTimer();
	BOOL tick();

private:
	static BOOL sInitialized;
};

extern AOInvTimer* gAOInvTimer;


//
// AONoteCardDropTarget
//


class LLViewerInventoryItem;

class AONoteCardDropTarget : public LLView
{
public:
	AONoteCardDropTarget(const std::string& name, const LLRect& rect, void (*callback)(LLViewerInventoryItem*));
	virtual ~AONoteCardDropTarget();

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


//
// AONotecardCallback
//


class AONotecardCallback : public LLInventoryCallback
{
public:
	AONotecardCallback(const std::string& filename);

	void fire(const LLUUID &inv_item);

private:
	std::string mFileName;
};

#endif
