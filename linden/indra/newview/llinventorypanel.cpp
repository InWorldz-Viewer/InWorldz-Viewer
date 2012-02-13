#include "llviewerprecompiledheaders.h"

#include <utility> // for std::pair<>

#include "llinventorypanel.h"

#include "message.h"

#include "llagent.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"
#include "llinventorybridge.h"
#include "llscrollcontainer.h"

const std::string LLInventoryPanel::DEFAULT_SORT_ORDER = std::string("InventorySortOrder");
const std::string LLInventoryPanel::RECENTITEMS_SORT_ORDER = std::string("RecentItemsSortOrder");
const std::string LLInventoryPanel::WORNITEMS_SORT_ORDER = std::string("WornItemsSortOrder");
const std::string LLInventoryPanel::INHERIT_SORT_ORDER = std::string("");

LLInventoryPanel::LLInventoryPanel(const std::string& name,
								    const std::string& sort_order_setting,
									const LLRect& rect,
									LLInventoryModel* inventory,
									BOOL allow_multi_select,
									LLView *parent_view) :
	LLPanel(name, rect, TRUE),
	mInventory(inventory),
	mInventoryObserver(NULL),
	mFolders(NULL),
	mScroller(NULL),
	mAllowMultiSelect(allow_multi_select),
	mSortOrderSetting(sort_order_setting)
{
	setBackgroundColor(gColors.getColor("InventoryBackgroundColor"));
	setBackgroundVisible(TRUE);
	setBackgroundOpaque(TRUE);
}

BOOL LLInventoryPanel::postBuild()
{
	init_inventory_panel_actions(this);

	LLRect folder_rect(0,
					   0,
					   getRect().getWidth(),
					   0);
	mFolders = new LLFolderView(getName(), NULL, folder_rect, LLUUID::null, this);
	mFolders->setAllowMultiSelect(mAllowMultiSelect);

	// scroller
	LLRect scroller_view_rect = getRect();
	scroller_view_rect.translate(-scroller_view_rect.mLeft, -scroller_view_rect.mBottom);
	mScroller = new LLScrollableContainerView(std::string("Inventory Scroller"),
											   scroller_view_rect,
											  mFolders);
	mScroller->setFollowsAll();
	mScroller->setReserveScrollCorner(TRUE);
	addChild(mScroller);
	mFolders->setScrollContainer(mScroller);

	// set up the callbacks from the inventory we're viewing, and then
	// build everything.
	mInventoryObserver = new LLInventoryPanelObserver(this);
	mInventory->addObserver(mInventoryObserver);
	rebuildViewsFor(LLUUID::null, LLInventoryObserver::ADD);

	// bit of a hack to make sure the inventory is open.
	mFolders->openFolder(std::string("My Inventory"));

	if (mSortOrderSetting != INHERIT_SORT_ORDER)
	{
		setSortOrder(gSavedSettings.getU32(mSortOrderSetting));
	}
	else
	{
		setSortOrder(gSavedSettings.getU32(DEFAULT_SORT_ORDER));
	}
	mFolders->setSortOrder(mFolders->getFilter()->getSortOrder());

	return TRUE;
}

LLInventoryPanel::~LLInventoryPanel()
{
	// should this be a global setting?
	U32 sort_order = mFolders->getSortOrder();
	if (mSortOrderSetting != INHERIT_SORT_ORDER)
	{
		gSavedSettings.setU32(mSortOrderSetting, sort_order);
	}

	// LLView destructor will take care of the sub-views.
	mInventory->removeObserver(mInventoryObserver);
	delete mInventoryObserver;
	mScroller = NULL;
}

// virtual
LLXMLNodePtr LLInventoryPanel::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLPanel::getXML(false); // Do not print out children

	node->setName(LL_INVENTORY_PANEL_TAG);

	node->createChild("allow_multi_select", TRUE)->setBoolValue(mFolders->getAllowMultiSelect());

	return node;
}

LLView* LLInventoryPanel::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{
	LLInventoryPanel* panel;

	std::string name("inventory_panel");
	node->getAttributeString("name", name);

	BOOL allow_multi_select = TRUE;
	node->getAttributeBOOL("allow_multi_select", allow_multi_select);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	std::string sort_order(INHERIT_SORT_ORDER);
	node->getAttributeString("sort_order", sort_order);

	panel = new LLInventoryPanel(name, sort_order,
								 rect, &gInventory,
								 allow_multi_select, parent);

	panel->initFromXML(node, parent);

	panel->postBuild();

	return panel;
}

void LLInventoryPanel::draw()
{
	// select the desired item (in case it wasn't loaded when the selection was requested)
	if (mSelectThisID.notNull())
	{
		setSelection(mSelectThisID, false);
	}
	LLPanel::draw();
}

void LLInventoryPanel::setFilterTypes(U32 filter_types)
{
	mFolders->getFilter()->setFilterTypes(filter_types);
}	

void LLInventoryPanel::setFilterPermMask(PermissionMask filter_perm_mask)
{
	mFolders->getFilter()->setFilterPermissions(filter_perm_mask);
}

void LLInventoryPanel::setFilterSubString(const std::string& string)
{
	mFolders->getFilter()->setFilterSubString(string);
}

void LLInventoryPanel::setFilterWorn(bool worn)
{
	mFolders->getFilter()->setFilterWorn(worn);
}

void LLInventoryPanel::setSortOrder(U32 order)
{
	mFolders->getFilter()->setSortOrder(order);
	if (mFolders->getFilter()->isModified())
	{
		mFolders->setSortOrder(order);
		// try to keep selection onscreen, even if it wasn't to start with
		mFolders->scrollToShowSelection();
	}
}

void LLInventoryPanel::setSinceLogoff(BOOL sl)
{
	mFolders->getFilter()->setDateRangeLastLogoff(sl);
}

void LLInventoryPanel::setHoursAgo(U32 hours)
{
	mFolders->getFilter()->setHoursAgo(hours);
}

void LLInventoryPanel::setShowFolderState(LLInventoryFilter::EFolderShow show)
{
	mFolders->getFilter()->setShowFolderState(show);
}

LLInventoryFilter::EFolderShow LLInventoryPanel::getShowFolderState()
{
	return mFolders->getFilter()->getShowFolderState();
}

void LLInventoryPanel::modelChanged(U32 mask)
{
	LLFastTimer t2(LLFastTimer::FTM_REFRESH);

	bool handled = false;
	if(mask & LLInventoryObserver::LABEL)
	{
		handled = true;
		// label change - empty out the display name for each object
		// in this change set.
		const std::set<LLUUID>& changed_items = gInventory.getChangedIDs();
		std::set<LLUUID>::const_iterator id_it = changed_items.begin();
		std::set<LLUUID>::const_iterator id_end = changed_items.end();
		LLFolderViewItem* view = NULL;
		LLInvFVBridge* bridge = NULL;
		for (;id_it != id_end; ++id_it)
		{
			view = mFolders->getItemByID(*id_it);
			if(view)
			{
				// request refresh on this item (also flags for filtering)
				bridge = (LLInvFVBridge*)view->getListener();
				if(bridge)
				{	// Clear the display name first, so it gets properly re-built during refresh()
					bridge->clearDisplayName();
				}
				view->refresh();
			}
		}
	}
	if((mask & (LLInventoryObserver::STRUCTURE
				| LLInventoryObserver::ADD
				| LLInventoryObserver::REMOVE)) != 0)
	{
		handled = true;
		// Record which folders are open by uuid.
		LLInventoryModel* model = getModel();
		if (model)
		{
			const std::set<LLUUID>& changed_items = gInventory.getChangedIDs();

			std::set<LLUUID>::const_iterator id_it = changed_items.begin();
			std::set<LLUUID>::const_iterator id_end = changed_items.end();
			for (;id_it != id_end; ++id_it)
			{
				// sync view with model
				LLInventoryObject* model_item = model->getObject(*id_it);
				LLFolderViewItem* view_item = mFolders->getItemByID(*id_it);

				if (model_item)
				{
					if (!view_item)
					{
						// this object was just created, need to build a view for it
						if ((mask & LLInventoryObserver::ADD) != LLInventoryObserver::ADD)
						{
							llwarns << *id_it << " is in model but not in view, but ADD flag not set" << llendl;
						}
						buildNewViews(*id_it);
						
						// select any newly created object
						// that has the auto rename at top of folder
						// root set
						if(mFolders->getRoot()->needsAutoRename())
						{
							setSelection(*id_it, FALSE);
						}
					}
					else
					{
						// this object was probably moved, check its parent
						if ((mask & LLInventoryObserver::STRUCTURE) != LLInventoryObserver::STRUCTURE)
						{
							llwarns << *id_it << " is in model and in view, but STRUCTURE flag not set" << llendl;
						}

						LLFolderViewFolder* new_parent = (LLFolderViewFolder*)mFolders->getItemByID(model_item->getParentUUID());
						if (view_item->getParentFolder() != new_parent)
						{
							view_item->getParentFolder()->extractItem(view_item);
							view_item->addToFolder(new_parent, mFolders);
						}
					}
				}
				else
				{
					if (view_item)
					{
						if ((mask & LLInventoryObserver::REMOVE) != LLInventoryObserver::REMOVE)
						{
							llwarns << *id_it << " is not in model but in view, but REMOVE flag not set" << llendl;
						}
						// item in view but not model, need to delete view
						view_item->destroyView();
					}
					else
					{
						llwarns << *id_it << "Item does not exist in either view or model, but notification triggered" << llendl;
					}
				}
			}
		}
	}

	if (!handled)
	{
		// it's a small change that only requires a refresh.
		// *TODO: figure out a more efficient way to do the refresh
		// since it is expensive on large inventories
		mFolders->refresh();
	}
}

void LLInventoryPanel::rebuildViewsFor(const LLUUID& id, U32 mask)
{
	LLFolderViewItem* old_view = NULL;

	// get old LLFolderViewItem
	old_view = mFolders->getItemByID(id);
	if (old_view && id.notNull())
	{
		old_view->destroyView();
	}

	buildNewViews(id);
}

void LLInventoryPanel::buildNewViews(const LLUUID& id)
{
	LLFolderViewItem* itemp = NULL;
	LLInventoryObject* objectp = gInventory.getObject(id);

	if (objectp)
	{		
		if (objectp->getType() <= LLAssetType::AT_NONE ||
			objectp->getType() >= LLAssetType::AT_COUNT)
		{
			LL_DEBUGS("Inventory") << "LLInventoryPanel::buildNewViews called with objectp->mType == " 
				<< ((S32) objectp->getType())
				<< " (shouldn't happen)" << LL_ENDL;
		}
		else if (objectp->getType() == LLAssetType::AT_CATEGORY) // build new view for category
		{
			LLInvFVBridge* new_listener = LLInvFVBridge::createBridge(objectp->getType(),
													LLInventoryType::IT_CATEGORY,
													this,
													objectp->getUUID());

			if (new_listener)
			{
				LLFolderViewFolder* folderp = new LLFolderViewFolder(new_listener->getDisplayName(),
													new_listener->getIcon(),
													mFolders,
													new_listener);
				
				folderp->setItemSortOrder(mFolders->getSortOrder());
				itemp = folderp;
			}
		}
		else // build new view for item
		{
			LLInventoryItem* item = (LLInventoryItem*)objectp;
			LLInvFVBridge* new_listener = LLInvFVBridge::createBridge(
				item->getType(),
				item->getInventoryType(),
				this,
				item->getUUID(),
				item->getFlags());
			if (new_listener)
			{
				itemp = new LLFolderViewItem(new_listener->getDisplayName(),
												new_listener->getIcon(),
												new_listener->getCreationDate(),
												mFolders,
												new_listener);
			}
		}

		LLFolderViewFolder* parent_folder = (LLFolderViewFolder*)mFolders->getItemByID(objectp->getParentUUID());

		if (itemp)
		{
			if (parent_folder)
			{
				itemp->addToFolder(parent_folder, mFolders);
			}
			else
			{
				llwarns << "Couldn't find parent folder for child " << itemp->getLabel() << llendl;
				delete itemp;
			}
		}
	}

	if ((id.isNull() ||
		(objectp && objectp->getType() == LLAssetType::AT_CATEGORY)))
	{
		LLViewerInventoryCategory::cat_array_t* categories;
		LLViewerInventoryItem::item_array_t* items;

		mInventory->lockDirectDescendentArrays(id, categories, items);
		if(categories)
		{
			S32 count = categories->count();
			for(S32 i = 0; i < count; ++i)
			{
				LLInventoryCategory* cat = categories->get(i);
				buildNewViews(cat->getUUID());
			}
		}
		if(items)
		{
			S32 count = items->count();
			for(S32 i = 0; i < count; ++i)
			{
				LLInventoryItem* item = items->get(i);
				buildNewViews(item->getUUID());
			}
		}
		mInventory->unlockDirectDescendentArrays(id);
	}
}

struct LLConfirmPurgeData
{
	LLUUID mID;
	LLInventoryModel* mModel;
};

class LLIsNotWorn : public LLInventoryCollectFunctor
{
public:
	LLIsNotWorn() {}
	virtual ~LLIsNotWorn() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item)
	{
		return !gAgent.isWearingItem(item->getUUID());
	}
};

class LLOpenFolderByID : public LLFolderViewFunctor
{
public:
	LLOpenFolderByID(const LLUUID& id) : mID(id) {}
	virtual ~LLOpenFolderByID() {}
	virtual void doFolder(LLFolderViewFolder* folder)
		{
			if (folder->getListener() && folder->getListener()->getUUID() == mID) folder->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
		}
	virtual void doItem(LLFolderViewItem* item) {}
protected:
	const LLUUID& mID;
};


void LLInventoryPanel::openSelected()
{
	LLFolderViewItem* folder_item = mFolders->getCurSelectedItem();
	if(!folder_item) return;
	LLInvFVBridge* bridge = (LLInvFVBridge*)folder_item->getListener();
	if(!bridge) return;
	bridge->openItem();
}

BOOL LLInventoryPanel::handleHover(S32 x, S32 y, MASK mask)
{
	BOOL handled = LLView::handleHover(x, y, mask);
	if(handled)
	{
		ECursorType cursor = getWindow()->getCursor();
		if (LLInventoryModel::backgroundFetchActive() && cursor == UI_CURSOR_ARROW)
		{
			// replace arrow cursor with arrow and hourglass cursor
			getWindow()->setCursor(UI_CURSOR_WORKING);
		}
	}
	else
	{
		getWindow()->setCursor(UI_CURSOR_ARROW);
	}
	return TRUE;
}

BOOL LLInventoryPanel::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
										   EDragAndDropType cargo_type,
										   void* cargo_data,
										   EAcceptance* accept,
										   std::string& tooltip_msg)
{
	BOOL handled = LLPanel::handleDragAndDrop(x, y, mask, drop, cargo_type, cargo_data, accept, tooltip_msg);

	if (handled)
	{
		mFolders->setDragAndDropThisFrame();
	}

	return handled;
}


void LLInventoryPanel::openAllFolders()
{
	mFolders->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_DOWN);
	mFolders->arrangeAll();
}

void LLInventoryPanel::closeAllFolders()
{
	mFolders->setOpenArrangeRecursively(FALSE, LLFolderViewFolder::RECURSE_DOWN);
	mFolders->arrangeAll();
}

void LLInventoryPanel::openDefaultFolderForType(LLAssetType::EType type)
{
	LLUUID category_id = mInventory->findCategoryUUIDForType(type);
	LLOpenFolderByID opener(category_id);
	mFolders->applyFunctorRecursively(opener);
}

void LLInventoryPanel::setSelection(const LLUUID& obj_id, BOOL take_keyboard_focus)
{
	LLFolderViewItem* itemp = mFolders->getItemByID(obj_id);
	if(itemp && itemp->getListener())
	{
		itemp->getListener()->arrangeAndSet(itemp, TRUE, take_keyboard_focus);
		mSelectThisID.setNull();
		return;
	}
	else
	{
		// save the desired item to be selected later (if/when ready)
		mSelectThisID = obj_id;
	}
}

void LLInventoryPanel::clearSelection()
{
	mFolders->clearSelection();
	mSelectThisID.setNull();
}

void LLInventoryPanel::createNewItem(const std::string& name,
									const LLUUID& parent_id,
									LLAssetType::EType asset_type,
									LLInventoryType::EType inv_type,
									U32 next_owner_perm)
{
	std::string desc;
	LLAssetType::generateDescriptionFor(asset_type, desc);
	next_owner_perm = (next_owner_perm) ? next_owner_perm : PERM_MOVE | PERM_TRANSFER;

	LLPointer<LLInventoryCallback> cb = NULL;
	if (inv_type == LLInventoryType::IT_GESTURE)
	{
		cb = new CreateGestureCallback(); // This better not screw up LLPointer -- MC
	}
	create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
						  parent_id, LLTransactionID::tnull, name, desc, asset_type, inv_type,
						  NOT_WEARABLE, next_owner_perm, cb);
	/*else
	{
		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							  parent_id, LLTransactionID::tnull, name, desc, asset_type, inv_type,
							  NOT_WEARABLE, next_owner_perm, cb);
	}*/
}

// static DEBUG ONLY:
void LLInventoryPanel::dumpSelectionInformation(void* user_data)
{
	LLInventoryPanel* iv = (LLInventoryPanel*)user_data;
	iv->mFolders->dumpSelectionInformation();
}
