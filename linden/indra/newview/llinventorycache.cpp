/**
 * @file llinventorycache.cpp
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

#include "llviewerprecompiledheaders.h"

#include "llinventorycache.h"

#include "lldir.h"
#include "llinventory.h"
#include "llinventoryview.h"
#include "llsdserialize.h"
#include "llviewercontrol.h"
#include "llwearablelist.h"
#include "llxorcipher.h"

// constants
const U32 INVENTORY_CACHE_VERSION = 1; // update this with every change to the cache format
const char CACHE_FORMAT_STRING[] = "%s.inv";
const LLUUID MAGIC_ID("3c115e51-04f4-523c-9fa6-98aff1034730"); // uuid used in cypher for inv

// helper functions
bool valid_string(const std::string& str)			{	return !str.empty();	}
bool valid_id(const LLUUID& id)						{	return id.notNull();	}
bool valid_asset_type(LLAssetType::EType type)		{	return (type > LLAssetType::AT_NONE) && (type < LLAssetType::AT_COUNT);	}
bool valid_inv_type(LLInventoryType::EType type)	{	return (type > LLInventoryType::IT_NONE) && (type < LLInventoryType::IT_COUNT);	}
bool valid_asset_id(const LLUUID& id, LLAssetType::EType type)
{
	// this should *really* be part of llassettype.cpp since we
	// want to look for valid items that have null asset ids
	// in the inventory
	if (id.notNull() || (id.isNull() && 
						(type != LLAssetType::AT_CATEGORY) && 
						(type != LLAssetType::AT_ROOT_CATEGORY) && 
						(type != LLAssetType::AT_SNAPSHOT_CATEGORY) && 
						(type != LLAssetType::AT_TRASH) && 
						(type != LLAssetType::AT_LOST_AND_FOUND) && 
						(type != LLAssetType::AT_SIMSTATE)))
	{
		return true;
	}
	return false;
}

// cache test
class LLCanCache : public LLInventoryCollectFunctor 
{
public:
	LLCanCache() {}
	virtual ~LLCanCache() {}
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
protected:
	std::set<LLUUID> mCachedCatIDs;
};

bool LLCanCache::operator()(LLInventoryCategory* cat, LLInventoryItem* item)
{
	bool rv = false;
	if (item)
	{
		if (mCachedCatIDs.find(item->getParentUUID()) != mCachedCatIDs.end())
		{
			rv = true;
		}
	}
	else if (cat)
	{
		// HACK: downcast
		LLViewerInventoryCategory* c = (LLViewerInventoryCategory*)cat;
		S32 descendents_server = c->getDescendentCount();
		if ((c->getVersion() != LLViewerInventoryCategory::VERSION_UNKNOWN)/* && 
			(descendents_server != LLViewerInventoryCategory::DESCENDENT_COUNT_UNKNOWN)*/)
		{
			LLInventoryCache::cat_array_t* cats;
			LLInventoryCache::item_array_t* items;
			gInventory.getDirectDescendentsOf(c->getUUID(), cats, items);
			S32 descendents_actual = 0;
			if (cats && items)
			{
				descendents_actual = cats->count() + items->count();
			}
			if (descendents_server == descendents_actual)
			{
				mCachedCatIDs.insert(c->getUUID());
				rv = true;
			}
		}
	}
	return rv;
}

// cache functions
LLInventoryCache::LLInventoryCache()
{
	// No reason to instantiate this class...yet
}

LLInventoryCache::~LLInventoryCache()
{
}

// static
bool LLInventoryCache::createCache(const LLUUID& parent_folder_id, const LLUUID& agent_id)
{
	LL_DEBUGS("Inventory") << "Caching " << parent_folder_id << " for " << agent_id
						   << LL_ENDL;

	LLViewerInventoryCategory* root_cat = gInventory.getCategory(parent_folder_id);
	if (!root_cat)
	{
		return false;
	}

	LLInventoryCache::cat_array_t categories;	
	LLInventoryCache::item_array_t items;
	std::string agent_id_str = agent_id.asString();
	std::string path(gDirUtilp->getExpandedFilename(LL_PATH_CACHE, agent_id_str));
	std::string inventory_filename = llformat(CACHE_FORMAT_STRING, path.c_str());
	std::string gzip_filename(inventory_filename);

	categories.put(root_cat);

	LLCanCache can_cache;
	can_cache(root_cat, NULL);

	gInventory.collectDescendentsIf(parent_folder_id,
									categories,
									items,
									TRUE, //INCLUDE_TRASH
									can_cache);

	addToCache(inventory_filename, categories, items);

	gzip_filename.append(".gz");
	if (gzip_file(inventory_filename, gzip_filename))
	{
		LL_DEBUGS("Inventory") << "Successfully compressed " << inventory_filename << LL_ENDL;
		LLFile::remove(inventory_filename);
	}
	else
	{
		llwarns << "Unable to compress " << inventory_filename << llendl;
		return false;
	}
	return true;
}

// static
bool LLInventoryCache::addToCache(const std::string& filename,
								  const LLInventoryCache::cat_array_t& categories,
								  const LLInventoryCache::item_array_t& items)
{
	if (filename.empty())
	{
		llwarns << "Cache filename is NULL! Can't save!" << llendl;
		return false;
	}

	LL_INFOS("Inventory") << "Saving inventory cache: " << filename << LL_ENDL;

	S32 count = categories.count();
	if (count <= 0)
	{
		LL_WARNS("Inventory") << "No categories to cache! Skipping cache" << LL_ENDL;
		return false;
	}

	LLSD inv_to_cache = LLSD::emptyArray();

	S32 count_total = 0;
	S32 category_total = 0;

	for (S32 i = 0; i < count; ++i)
	{
		LLViewerInventoryCategory* cat = categories[i];
		if (!cat)
		{
			LL_WARNS("Inventory") << "Trying to cache invalid inventory category!" << LL_ENDL;
		}
		else
		{
			if (cat->getVersion() != LLViewerInventoryCategory::VERSION_UNKNOWN)
			{
				LLSD category = catToCacheLLSD(*cat);
				inv_to_cache.append(category);
				category_total++;
			}
			else
			{
				LL_DEBUGS("Inventory") << "Unknown inventory category version for " 
									<< cat->getName() << " ("
									<< cat->getUUID() << "). Skipping caching it" << LL_ENDL;
			}
		}
	}

	count = items.count();
	for (S32 i = 0; i < count; ++i)
	{
		LLViewerInventoryItem* item = items[i];
		if (item)
		{
			LLSD inv_item = itemToCacheLLSD(*item, true);
			inv_to_cache.append(inv_item);
			count_total++;
		}
		else
		{
			LL_WARNS("Inventory") << "Trying to cache invalid inventory item!" << LL_ENDL;
		}
	}

	if (addToCache(filename, inv_to_cache))
	{
		LL_INFOS("Inventory") << "Cached " << category_total << " categories and " << count_total << " inventory items" << LL_ENDL;
	}

	return true;
}

// static
bool LLInventoryCache::addToCache(const std::string &filename, const LLSD &llsd_to_cache)
{
	// Save to disk
	llofstream out(filename);
	if (!out.good())
	{
		llwarns << "Unable to open \"" << filename << "\" for output." << llendl;
		return false;
	}

	LLSDSerialize::toPrettyXML(llsd_to_cache, out);

	out.close();

	return true;
}

// static
bool LLInventoryCache::loadFromCache(const std::string& filename,
									LLInventoryCache::cat_array_t& categories,
									LLInventoryCache::item_array_t& items)
{
	// version set in settings.xml
	if (!versionCorrect())
	{
		// don't load anything. Old cache belonging to this avatar
		// will be overwritten when we exit
		return false;
	}

	if (filename.empty())
	{
		llwarns << "Filename is empty!" << llendl;
		return false;
	}

	S32 item_count_total = 0;
	S32 cat_count_total = 0;

	LLSD data;

    llifstream file;
	file.open(filename.c_str());
    if (file.is_open())
    {
		LL_DEBUGS("Inventory") << "Loading " << filename << llendl;
        LLSDSerialize::fromXML(data, file);
    }
	file.close();

	LLSD::array_iterator llsd_it = data.beginArray();
	for (; llsd_it != data.endArray(); ++llsd_it)
	{
		if ((*llsd_it).has("inv_category"))
		{
			LLPointer<LLViewerInventoryCategory> inv_cat = createCatFromCache((*llsd_it)["inv_category"]);
			if (inv_cat)
			{
				categories.put(inv_cat);
				cat_count_total++;
			}
			else
			{
				llwarns << "inv_category importing from cache failed. Ignoring invalid category" << llendl;
				inv_cat = NULL;
			}
		}
		else if ((*llsd_it).has("inv_item"))
		{
			LLPointer<LLViewerInventoryItem> inv_item = createItemFromCache((*llsd_it)["inv_item"]);
			if (inv_item)
			{
				// *FIX: Need a better solution, this prevents the
				// application from freezing, but breaks inventory
				// caching.
				if (inv_item->getUUID().isNull())
				{
					LL_DEBUGS("Inventory") << "Ignoring inventory with null item id" << LL_ENDL;
					inv_item = NULL;						
				}
				else
				{
					items.put(inv_item);
					item_count_total++;
				}
			}
			else
			{
				// (Will fail for bodyparts and clothing 'til wearables get cached properly)
				llwarns << "inv_item importing from cache failed. Ignoring invalid inventory item " << (*llsd_it)<< llendl;
				inv_item = NULL;
			}
		}
		else
		{
			llwarns << "Unknown token in inventory LLSD: " << (*llsd_it) << llendl;
			continue;
		}
	}

	LL_INFOS("Inventory") << "Inventory loaded  " << cat_count_total 
							<< " categories and " << item_count_total 
							<< " items from "  << filename << LL_ENDL;
	return true;
}

// static
LLPointer<LLViewerInventoryCategory> LLInventoryCache::createCatFromCache(const LLSD& category)
{
	if ((category.has("type")) && (LLAssetType::lookup(category["type"].asString()) != LLAssetType::AT_CATEGORY))
	{
		llwarns << "Trying to create a category but asset type is wrong!" << llendl;
		return NULL;
	}

	LLUUID id						= (category.has("cat_id"))		? (category["cat_id"].asUUID())		: (LLUUID::null);
	LLUUID parent_id				= (category.has("parent_id"))	? (category["parent_id"].asUUID())	: (LLUUID::null);
	// A -1 value for preferred type is any user-created folder
	LLAssetType::EType pref_type	= (category.has("pref_type"))	? (LLAssetType::lookup(category["pref_type"].asString()))	: (LLAssetType::AT_NONE);
	std::string name				= (category.has("name"))		? (category["name"].asString())		: ("");
	LLUUID owner_id					= (category.has("owner_id"))	? (category["owner_id"].asUUID())	: (LLUUID::null);
	S32 version						= (category.has("version"))		? (category["version"].asInteger())	: (LLViewerInventoryCategory::VERSION_UNKNOWN);

	// create the category if we can
	if (valid_id(id)/* &&
		valid_id(owner_id)*/)
	{
		// note: null parent_id means folders like "My Inventory"
		LLStringUtil::replaceNonstandardASCII(name, ' ');
        LLStringUtil::replaceChar(name, '|', ' ');
		
		LLPointer<LLViewerInventoryCategory> pCat = new LLViewerInventoryCategory(owner_id);
		pCat->setUUID(id);
		pCat->setParent(parent_id);
		pCat->setPreferredType(pref_type);
		pCat->rename(name);
		pCat->setVersion(version);

		return pCat;
	}

	return NULL;
}

// static
LLSD LLInventoryCache::catToCacheLLSD(const LLViewerInventoryCategory &cat_to_cache)
{
	// Note: we handle bad entries on import, don't worry about export
	LLSD inv_category = LLSD::emptyArray();

	LLSD category = LLSD::emptyMap();
	category["cat_id"] = cat_to_cache.getUUID();
	category["parent_id"] = cat_to_cache.getParentUUID();
	category["type"] = LLAssetType::lookup(cat_to_cache.getType());
	// everything but AT_NONE are system folders
	category["pref_type"] = LLAssetType::lookup(cat_to_cache.getPreferredType());
	category["name"] = cat_to_cache.getName();
	category["owner_id"] = cat_to_cache.getOwnerID();
	category["version"] = cat_to_cache.getVersion();

	inv_category["inv_category"] = category;

	return inv_category;
}

// static
LLPointer<LLViewerInventoryItem> LLInventoryCache::createItemFromCache(const LLSD& item)
{
	bool is_complete = true;
	bool is_multiple_object = false;
	bool is_object_slam_perm = false;
	bool is_object_slam_perm_sale = false;
	bool is_object_perm_overwrite_base = false;
	bool is_object_perm_overwrite_owner = false;
	bool is_object_perm_overwrite_group = false;
	bool is_object_perm_overwrite_everyone = false;
	bool is_object_perm_overwrite_next_owner = false;
	LLUUID id						= (item.has("item_id"))		? (item["item_id"].asUUID())	: (LLUUID::null);
	LLUUID parent_id				= (item.has("parent_id"))	? (item["parent_id"].asUUID())	: (LLUUID::null);
	LLAssetType::EType type			= (item.has("type"))		? (LLAssetType::lookup(item["type"].asString())) : (LLAssetType::AT_NONE);
	LLInventoryType::EType inv_type	= (item.has("inv_type"))	? (LLInventoryType::lookup(item.get("inv_type").asString()))	: (LLInventoryType::IT_NONE);
	std::string name				= (item.has("name"))		? (item["name"].asString())		: ("");
	std::string desc				= (item.has("desc"))		? (item["desc"].asString())		: ("");
	LLUUID asset_id					= (item.has("asset_id"))	? (item["asset_id"].asUUID())	: (LLUUID::null);
	S32 creation_date				= (item.has("creation_date"))		? (item["creation_date"].asInteger())	: (-1);
	U32 flags = 0;
	if (item.has("flags"))
	{
		llformat(item.get("flags").asString().c_str(), "%x", &flags);
	}
	LLUUID shadow_id;
	if (item.has("shadow_id"))
	{
		asset_id.set(item.get("shadow_id").asString());
		LLXORCipher cipher(MAGIC_ID.mData, UUID_BYTES);
		cipher.decrypt(asset_id.mData, UUID_BYTES);
	}
	LLPermissions permissions;
	if (item.has("permissions"))
	{
		permissions.set(ll_permissions_from_sd(item.get("permissions")));
	}
	else
	{
		is_complete = false;
	}
	LLSaleInfo sale_info;
	if (item.has("sale_info"))
	{
		// Sale info used to contain next owner perm. It is now in
		// the permissions. Thus, we read that out, and fix legacy
		// objects. It's possible this op would fail, but it
		// should pick up the vast majority of the tasks.
		BOOL has_perm_mask = FALSE;
		U32 perm_mask = 0;
		if (sale_info.fromLLSD(item.get("sale_info"), has_perm_mask, perm_mask) && 
			has_perm_mask)
		{
			if (perm_mask == PERM_NONE)
			{
				perm_mask = permissions.getMaskOwner();
			}
			// fair use fix.
			if (!(perm_mask & PERM_COPY))
			{
				perm_mask |= PERM_TRANSFER;
			}
			permissions.setMaskNext(perm_mask);
		}
	}
	if (item.has("is_multiple_object"))
	{
		is_multiple_object = item.get("is_multiple_object").asBoolean();
	}
	if (item.has("is_object_slam_perm"))
	{
		is_object_slam_perm = item.get("is_object_slam_perm").asBoolean();
	}
	if (item.has("is_object_slam_perm_sale"))
	{
		is_object_slam_perm_sale = item.get("is_object_slam_perm_sale").asBoolean();
	}
	if (item.has("is_object_perm_overwrite_base"))
	{
		is_object_perm_overwrite_base = item.get("is_object_perm_overwrite_base").asBoolean();
	}
	if (item.has("is_object_perm_overwrite_owner"))
	{
		is_object_perm_overwrite_owner = item.get("is_object_perm_overwrite_owner").asBoolean();
	}
	if (item.has("is_object_perm_overwrite_group"))
	{
		is_object_perm_overwrite_group = item.get("is_object_perm_overwrite_group").asBoolean();
	}
	if (item.has("is_object_perm_overwrite_everyone"))
	{
		is_object_perm_overwrite_everyone = item.get("is_object_perm_overwrite_everyone").asBoolean();
	}
	if (item.has("is_object_perm_overwrite_next_owner"))
	{
		is_object_perm_overwrite_next_owner = item.get("is_object_perm_overwrite_next_owner").asBoolean();
	}

	// create the item if we can
	if (valid_id(id) &&
		valid_id(parent_id) &&
		valid_asset_type(type) &&
		valid_inv_type(inv_type) && // IT_NONE is never used
		inventory_and_asset_types_match(inv_type, type) &&
		valid_asset_id(asset_id, type)
		)
	{
		LLStringUtil::replaceNonstandardASCII(name, ' ');
        LLStringUtil::replaceChar(name, '|', ' ');

		if (valid_string(desc))
		{
			LLStringUtil::replaceNonstandardASCII(desc, ' ');
		}

		// Note that we don't create items that have a mismatch. This is new behavior
		LLPointer<LLViewerInventoryItem> pItem = NULL;
		if (is_complete)
		{
			pItem = new LLViewerInventoryItem(id, parent_id, permissions, asset_id, type, inv_type, name, desc, sale_info, flags, creation_date);
		}
		else
		{
			pItem = new LLViewerInventoryItem(id, parent_id, name, inv_type);
		}

		// We don't set flags in the ctor because of the if above
		// TODO: remove variables and make this short and neat
		if (inv_type == LLInventoryType::IT_OBJECT)
		{
			U32 new_flags = 0;
			if (is_multiple_object)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_HAS_MULTIPLE_ITEMS;
			if (is_object_slam_perm)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_SLAM_PERM;
			if (is_object_slam_perm_sale)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_SLAM_SALE;
			if (is_object_perm_overwrite_base)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_BASE;
			if (is_object_perm_overwrite_owner)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_OWNER;
			if (is_object_perm_overwrite_group)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_GROUP;
			if (is_object_perm_overwrite_everyone)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_EVERYONE;
			if (is_object_perm_overwrite_next_owner)
				new_flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_NEXT_OWNER;

			pItem->setFlags(pItem->getFlags() | new_flags);
		}

		if (inv_type == LLInventoryType::IT_WEARABLE)
		{
			EWearableType wearable = LLWearable::typeNameToType(item["wearable"].asString());
			if (wearable != WT_INVALID)
			{
				if (type == LLAssetType::AT_BODYPART)
				{
					switch (wearable)
					{
						case WT_EYES:	pItem->setFlags(pItem->getFlags() | WT_EYES); break;
						case WT_HAIR:	pItem->setFlags(pItem->getFlags() | WT_HAIR); break;
						case WT_SKIN:	pItem->setFlags(pItem->getFlags() | WT_SKIN); break;
						case WT_SHAPE:	pItem->setFlags(pItem->getFlags() | WT_SHAPE); break;
						default: break;
					}
				}
				else if (type == LLAssetType::AT_CLOTHING)
				{
					switch (wearable)
					{
						case WT_TATTOO:		pItem->setFlags(pItem->getFlags() | WT_TATTOO); break;
						case WT_ALPHA:		pItem->setFlags(pItem->getFlags() | WT_ALPHA); break;
						case WT_SKIRT:		pItem->setFlags(pItem->getFlags() | WT_SKIRT); break;
						case WT_UNDERPANTS: pItem->setFlags(pItem->getFlags() | WT_UNDERPANTS); break;
						case WT_UNDERSHIRT: pItem->setFlags(pItem->getFlags() | WT_UNDERSHIRT); break;
						case WT_GLOVES:		pItem->setFlags(pItem->getFlags() | WT_GLOVES); break;
						case WT_JACKET:		pItem->setFlags(pItem->getFlags() | WT_JACKET); break;
						case WT_SOCKS:		pItem->setFlags(pItem->getFlags() | WT_SOCKS); break;
						case WT_SHOES:		pItem->setFlags(pItem->getFlags() | WT_SHOES); break;
						case WT_PANTS:		pItem->setFlags(pItem->getFlags() | WT_PANTS); break;
						case WT_SHIRT:		pItem->setFlags(pItem->getFlags() | WT_SHIRT); break;
						default: break;
					}
				}
			}
			pItem->setFlags(pItem->getFlags() & LLInventoryItem::II_FLAGS_WEARABLES_MASK);
		}

		return pItem;
	}

	return NULL;
}

// static
LLSD LLInventoryCache::itemToCacheLLSD(const LLViewerInventoryItem& item_to_cache, bool include_asset_key)
{
	// Cache items
	LLSD inv_item = LLSD::emptyArray();

	LLSD item = LLSD::emptyMap();
	item["item_id"] = item_to_cache.getUUID();
	item["parent_id"] = item_to_cache.getParentUUID();

	// Cache anyway, even if perms are broken. We want LLViewerInventoryItem to handle that
	item["permissions"] = ll_create_sd_from_permissions(item_to_cache.getPermissions());

	// Check for permissions to see the asset id, and if so write it
	// out as an asset id. Otherwise, apply our cheesy encryption.
	if (include_asset_key)
	{
		U32 mask = item_to_cache.getPermissions().getMaskBase();
		if(((mask & PERM_ITEM_UNRESTRICTED) == PERM_ITEM_UNRESTRICTED)
			|| (item_to_cache.getAssetUUID().isNull()))
		{
			item["asset_id"] = item_to_cache.getAssetUUID();
		}
		else
		{
			LLUUID shadow_id(item_to_cache.getAssetUUID());
			LLXORCipher cipher(MAGIC_ID.mData, UUID_BYTES);
			cipher.encrypt(shadow_id.mData, UUID_BYTES);
			item["shadow_id"] = shadow_id;
		}
	}
	else
	{
		item["asset_id"] = LLUUID::null;
	}
	item["type"] = LLAssetType::lookup(item_to_cache.getType());
	item["inv_type"] = LLInventoryType::lookup(item_to_cache.getInventoryType());
	// Landmark visited/not visited
	if (item_to_cache.getInventoryType() == LLInventoryType::IT_LANDMARK)
	{
		item["visited"] = (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_LANDMARK_VISITED) ? true : false;
	}
	// Object flags
	if (item_to_cache.getInventoryType() == LLInventoryType::IT_OBJECT)
	{ 
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_HAS_MULTIPLE_ITEMS)
		{
			item["is_multiple_object"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_SLAM_PERM)
		{
			item["is_object_slam_perm"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_SLAM_SALE)
		{
			item["is_object_slam_perm_sale"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_BASE)
		{
			item["is_object_perm_overwrite_base"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_OWNER)
		{
			item["is_object_perm_overwrite_owner"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_GROUP)
		{
			item["is_object_perm_overwrite_group"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_EVERYONE)
		{
			item["is_object_perm_overwrite_everyone"] = true;
		}
		if (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_NEXT_OWNER)
		{
			item["is_object_perm_overwrite_next_owner"] = true;
		}
	}
	item["flags"] = llformat("%08x", item_to_cache.getFlags());
	item["name"] = item_to_cache.getName();
	item["desc"] = item_to_cache.getDescription();
	item["creation_date"] = (S32)(item_to_cache.getCreationDate());
	// special rules are involved with wearables
	if (item_to_cache.getInventoryType() == LLInventoryType::IT_WEARABLE)
	{
		if (item_to_cache.getType() == LLAssetType::AT_BODYPART)
		{
			switch (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_WEARABLES_MASK)
			{
				case WT_SHAPE: item["wearable"] =	LLWearable::typeToTypeName(WT_SHAPE); break;
				case WT_SKIN: item["wearable"] =	LLWearable::typeToTypeName(WT_SKIN); break;
				case WT_HAIR: item["wearable"] =	LLWearable::typeToTypeName(WT_HAIR); break;
				case WT_EYES: item["wearable"] =	LLWearable::typeToTypeName(WT_EYES); break;
				default: break;
			}
		}
		else if (item_to_cache.getType() == LLAssetType::AT_CLOTHING)
		{
			switch (item_to_cache.getFlags() & LLInventoryItem::II_FLAGS_WEARABLES_MASK)
			{
				case WT_SHIRT: item["wearable"] =		LLWearable::typeToTypeName(WT_SHIRT); break;
				case WT_PANTS: item["wearable"] =		LLWearable::typeToTypeName(WT_PANTS); break;
				case WT_SHOES: item["wearable"] =		LLWearable::typeToTypeName(WT_SHOES); break;
				case WT_SOCKS: item["wearable"] =		LLWearable::typeToTypeName(WT_SOCKS); break;
				case WT_JACKET: item["wearable"] =		LLWearable::typeToTypeName(WT_JACKET); break;
				case WT_GLOVES: item["wearable"] =		LLWearable::typeToTypeName(WT_GLOVES); break;
				case WT_UNDERSHIRT: item["wearable"] =	LLWearable::typeToTypeName(WT_UNDERSHIRT); break;
				case WT_UNDERPANTS: item["wearable"] =	LLWearable::typeToTypeName(WT_UNDERPANTS); break;
				case WT_SKIRT: item["wearable"] =		LLWearable::typeToTypeName(WT_SKIRT); break;
				case WT_ALPHA: item["wearable"] =		LLWearable::typeToTypeName(WT_ALPHA); break;
				case WT_TATTOO: item["wearable"] =		LLWearable::typeToTypeName(WT_TATTOO); break;
				default: break;
			}
		}
	}

	inv_item["inv_item"] = item;

	return inv_item;
}

//static 
bool LLInventoryCache::versionCorrect()
{
	if (gSavedSettings.getU32("InventoryCacheVersion") != INVENTORY_CACHE_VERSION)
	{
		gSavedSettings.setU32("InventoryCacheVersion", INVENTORY_CACHE_VERSION);
		return false;
	}
	else
	{
		return true;
	}
}
