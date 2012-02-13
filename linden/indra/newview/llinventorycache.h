// Class to handle the inventory cache

#ifndef LL_INVENTORYCACHE_H
#define LL_INVENTORYCACHE_H


class LLViewerInventoryItem;
class LLViewerInventoryCategory;

class LLInventoryCache
{
public:
	LLInventoryCache();
	~LLInventoryCache();

	// Annoyingly repeated typedefs. Why aren't these in a common file?
	typedef LLDynamicArray<LLPointer<LLViewerInventoryCategory> > cat_array_t;
	typedef LLDynamicArray<LLPointer<LLViewerInventoryItem> > item_array_t;

	// Create's a cache file named after an agent's id then gzips it 
	// parent_folder_id should be the root folder, e.g. "My Inventory"
	static bool createCache(const LLUUID& parent_folder_id, const LLUUID& agent_id);

	// Load valid category and item entries from the cache file
	static bool loadFromCache(const std::string& filename,
								LLInventoryCache::cat_array_t& categories,
								LLInventoryCache::item_array_t& items);

protected:
	static bool addToCache(const std::string& filename, const LLSD& llsd_to_cache);
	static bool addToCache(const std::string& filename, const LLInventoryCache::cat_array_t& categories, const LLInventoryCache::item_array_t& items);
	
	// Converts an item to the LLSD format used in the cache
	static LLSD itemToCacheLLSD(const LLViewerInventoryItem& item_to_cache, bool include_asset_key = false);
	// Converts a category to the LLSD format used in the cache
	static LLSD catToCacheLLSD(const LLViewerInventoryCategory& cat_to_cache);

	// Creates an item from a cache file. Only LLSD formatting is supported
	static LLPointer<LLViewerInventoryItem> createItemFromCache(const LLSD& item);
	// Creates a category from a cache file. Only LLSD formatting is supported
	static LLPointer<LLViewerInventoryCategory> createCatFromCache(const LLSD& category);
};

#endif //LL_INVENTORYCACHE_H
