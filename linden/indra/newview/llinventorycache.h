/**
 * @file llinventorycache.h
 * @brief Class to handle the inventory cache
 *
 * Copyright (c) 2012, McCabe Maxsted
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
