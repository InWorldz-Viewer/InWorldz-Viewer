/*
User overrides for sim animations
*/

#ifndef AO_OVERRIDE_H
#define AO_OVERRIDE_H

class AOOverride
{
public:

	AOOverride(const LLUUID& anim_id, const std::string& anim_name);
	virtual ~AOOverride();

	bool hasOverrideID(const LLUUID& anim_id);

	// Adds a name/UUID combo to the AO. Returns an iterator of the added position in the map
	std::map<LLUUID, std::string>::iterator addOverride(const LLUUID& anim_id, const std::string& anim_name);
	bool removeOverride(const LLUUID& anim_id);
	// *Strongly* prefer removing by ID instead. Use this if you can't for some reason
	bool removeOverride(const std::string& anim_name);

	LLUUID getRandomOverrideID();
	// returns the first ID we know about if there is no default, or a random ID if we're random
	LLUUID getSelectedOverrideID();
	std::string getSelectedOverrideName();
	bool setSelectedOverride(const LLUUID& anim_id);
	// Prefer setting by ID rather than name, but if you have to use names there's this
	bool setSelectedOverride(const std::string& anim_name);

protected:

	LLUUID mSelectedAnimID;
	LLUUID mLastPlayedID;
	bool mRandom;
	bool mCycled;

	void clearOverrides() { mOverrideList.clear(); }

public:

	// making this public defeats the whole point of this class. All this stuff should be deleted and moved into a struct
	std::map<LLUUID, std::string> mOverrideList;

	bool hasOverrides() { return !(mOverrideList.empty()); }

	bool isRandom() { return mRandom; }
	void setRandom(bool random) { mRandom = random; }

	const LLUUID& getLastPlayedID() const { return mLastPlayedID; }
	void setLastPlayedID(const LLUUID& anim_id) { mLastPlayedID = anim_id; }
};

#endif
