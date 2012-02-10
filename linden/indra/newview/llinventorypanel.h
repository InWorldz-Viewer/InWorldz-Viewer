// Portions of the inventory window compatible with llui.
// For the rest, see llinventoryview -- MC

#ifndef LL_INVENTORYPANEL_H
#define LL_INVENTORYPANEL_H

#include "llpanel.h"

#include "llfolderview.h"

class LLInventoryModel;
class LLInventoryObserver;
class LLScrollableContainerView;
class LLInventoryView;

class LLInventoryPanel : public LLPanel
{
public:
	static const std::string DEFAULT_SORT_ORDER;
	static const std::string RECENTITEMS_SORT_ORDER;
	static const std::string WORNITEMS_SORT_ORDER;
	static const std::string INHERIT_SORT_ORDER;

	LLInventoryPanel(const std::string& name,
			const std::string& sort_order_setting,
			const LLRect& rect,
			LLInventoryModel* inventory,
			BOOL allow_multi_select,
			LLView *parent_view = NULL);
	~LLInventoryPanel();

	LLInventoryModel* getModel() { return mInventory; }

	BOOL postBuild();

	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	static LLView* fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory);

	// LLView methods
	void draw();
	BOOL handleHover(S32 x, S32 y, MASK mask);
	BOOL handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
								   EDragAndDropType cargo_type,
								   void* cargo_data,
								   EAcceptance* accept,
								   std::string& tooltip_msg);

	// Call this method to set the selection.
	void openAllFolders();
	void closeAllFolders();
	void openDefaultFolderForType(LLAssetType::EType);
	void setSelection(const LLUUID& obj_id, BOOL take_keyboard_focus);
	void setSelectCallback(LLFolderView::SelectCallback callback, void* user_data) { if (mFolders) mFolders->setSelectCallback(callback, user_data); }
	void clearSelection();
	LLInventoryFilter* getFilter() { return mFolders->getFilter(); }
	void setFilterTypes(U32 filter);
	U32 getFilterTypes() const { return mFolders->getFilterTypes(); }
	void setFilterPermMask(PermissionMask filter_perm_mask);
	U32 getFilterPermMask() const { return mFolders->getFilterPermissions(); }
	void setFilterSubString(const std::string& string);
	const std::string getFilterSubString() { return mFolders->getFilterSubString(); }
	void setFilterWorn(bool worn);
	bool getFilterWorn() const { return mFolders->getFilterWorn(); }
	
	void setSortOrder(U32 order);
	U32 getSortOrder() { return mFolders->getSortOrder(); }
	void setSinceLogoff(BOOL sl);
	void setHoursAgo(U32 hours);
	BOOL getSinceLogoff() { return mFolders->getFilter()->isSinceLogoff(); }
	
	void setShowFolderState(LLInventoryFilter::EFolderShow show);
	LLInventoryFilter::EFolderShow getShowFolderState();
	void setAllowMultiSelect(BOOL allow) { mFolders->setAllowMultiSelect(allow); }
	// This method is called when something has changed about the inventory.
	void modelChanged(U32 mask);
	LLFolderView* getRootFolder() { return mFolders; }
	LLScrollableContainerView* getScrollableContainer() { return mScroller; }

	// DEBUG ONLY:
	static void dumpSelectionInformation(void* user_data);

	void openSelected();

	void unSelectAll()	{ mFolders->setSelection(NULL, FALSE, FALSE); }

protected:
	// Given the id and the parent, build all of the folder views.
	void rebuildViewsFor(const LLUUID& id, U32 mask);
	void buildNewViews(const LLUUID& id);

public:
	// TomY TODO: Move this elsewhere?
	// helper method which creates an item with a good description,
	// updates the inventory, updates the server, and pushes the
	// inventory update out to other observers.
	void createNewItem(const std::string& name,
					   const LLUUID& parent_id,
					   LLAssetType::EType asset_type,
					   LLInventoryType::EType inv_type,
					   U32 next_owner_perm = 0);

protected:
	LLInventoryModel*			mInventory;
	LLInventoryObserver*		mInventoryObserver;
	LLFolderView*				mFolders;
	LLScrollableContainerView*	mScroller;
	BOOL 						mAllowMultiSelect;
	const std::string			mSortOrderSetting;
	LLUUID						mSelectThisID; // if non null, select this item
};

#endif
