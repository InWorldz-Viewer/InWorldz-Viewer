/*
User overrides for sim animations
*/

#include "llviewerprecompiledheaders.h"

#include "aooverride.h"

#include "aostate.h"


AOOverride::AOOverride(const LLUUID& anim_id, const std::string& anim_name)
	:
	mSelectedAnimID(anim_id), //always the first one we know about
	mLastPlayedID(LLUUID::null)
{
	addOverride(anim_id, anim_name);
}

AOOverride::~AOOverride()
{
}

std::map<LLUUID, std::string>::iterator AOOverride::addOverride(const LLUUID& anim_id, const std::string& anim_name) 
{
	if (mSelectedAnimID.isNull())
	{
		mSelectedAnimID = anim_id;
	}
	return mOverrideList.insert(mOverrideList.begin(), std::make_pair<LLUUID, std::string>(anim_id, anim_name));
}

bool AOOverride::removeOverride(const LLUUID& anim_id) 
{
	return (bool)(mOverrideList.erase(anim_id));
}

bool AOOverride::removeOverride(const std::string& anim_name)
{
	for (std::map<LLUUID, std::string>::iterator mIt = mOverrideList.begin();
		mIt != mOverrideList.end();)
	{
		if ((*mIt).second == anim_name)
		{
			mIt = mOverrideList.erase(mIt);
			return true;
		}
		else
		{
			 ++mIt;
		}
	}
	return false;
}

LLUUID AOOverride::getRandomOverrideID()
{
	// TODO: add cycling support
	if (hasOverrides())
	{
		if (mRandom)
		{
			// this *better not* ever generate negatives
			S32 rand_anim = ll_rand(mOverrideList.size());
			std::map<LLUUID, std::string>::iterator mIt = mOverrideList.begin();
			std::advance(mIt, rand_anim);
			return (*mIt).first;
		}
		else
		{
			return getSelectedOverrideID();
		}
	}
	return LLUUID::null;
}

LLUUID AOOverride::getSelectedOverrideID()
{
	if (hasOverrides())
	{
		if (mSelectedAnimID.isNull())
		{
			return mOverrideList.begin()->first;			
		}
		else
		{
			return mSelectedAnimID;
		}
	}
	return LLUUID::null;
}

std::string AOOverride::getSelectedOverrideName()
{
	if (mSelectedAnimID.notNull() && hasOverrideID(mSelectedAnimID))
	{
		return mOverrideList[mSelectedAnimID];
	}
	return "";
}

bool AOOverride::setSelectedOverride(const std::string& anim_name)
{
	for (std::map<LLUUID, std::string>::iterator mIt = mOverrideList.begin();
		mIt != mOverrideList.end(); ++mIt)
	{
		return (anim_name == (*mIt).second);
	}
	return false;
}

bool AOOverride::setSelectedOverride(const LLUUID &anim_id)
{
	if (hasOverrideID(anim_id))
	{
		mSelectedAnimID = anim_id;
		return true;
	}
	return false;
}

bool AOOverride::hasOverrideID(const LLUUID& anim_id)
{
	return (mOverrideList.find(anim_id) != mOverrideList.end());
}
