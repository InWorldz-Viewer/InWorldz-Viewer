/*
Anything that handles the actual AO workings that's not UI
*/

#include "llviewerprecompiledheaders.h"

#include "aoengine.h"

#include "llboost.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "aoutility.h"
#include "aooverride.h"
#include "llagent.h"
#include "llinventory.h"
#include "llinventoryview.h"
#include "llpreviewnotecard.h"
#include "llstartup.h"
#include "llviewercontrol.h"
#include "llviewertexteditor.h"
#include "llvoavatar.h"
#include "roles_constants.h"



AOStandTimer* mAOStandTimer;
AOState* AOEngine::sStateList = NULL;

AOEngine::AOEngine()
	:
	mInitialized(false),
	mInvFolderID(LLUUID::null)
{
	sStateList = new AOState();
}

AOEngine::~AOEngine()
{
	cleanupAnims();

	if (mAOStandTimer)
	{
		delete mAOStandTimer;
		mAOStandTimer = NULL;
	}
	if (sStateList)
	{
		delete sStateList;
		sStateList = NULL;
	}
}

void AOEngine::cleanupAnims()
{
	if (mAOList.empty())
	{
		return;
	}

	for (std::map<EAOState::State, AOOverride*>::iterator mIt = mAOList.begin(); 
		 mIt != mAOList.end(); ++mIt) 
	{
		AOOverride* entry = mIt->second;
		if (entry)
		{
			delete entry;
			entry = NULL;
		}
	}	

	mAOList.clear();
}

void AOEngine::initSingleton()
{
	// called by ctor
}

bool AOEngine::init()
{
	LL_DEBUGS("AO") << "initializing AO" << LL_ENDL;

	if (mInitialized)
	{
		run();
		return true;
	}

	cleanupAnims();

	if (!mAOList.empty())
	{
		llwarns << "Trying to reinitialize AO after cleanup failed, abort" << llendl;
		return false;
	}

	BOOL success = FALSE;

	if (LLStartUp::getStartupState() >= STATE_INVENTORY_SEND)
	{
		LLUUID configncitem = (LLUUID)gSavedPerAccountSettings.getString("AOConfigNotecardID");
		if (configncitem.notNull())
		{
			const LLInventoryItem* item = gInventory.getItem(configncitem);
			if (item)
			{
				if (gAgent.allowOperation(PERM_COPY, item->getPermissions(),GP_OBJECT_MANIPULATE) || gAgent.isGodlike())
				{
					if (!item->getAssetUUID().isNull())
					{
						LLUUID* new_uuid = new LLUUID(configncitem);
						LLHost source_sim = LLHost::invalid;
						mInvFolderID = item->getParentUUID();
						gAssetStorage->getInvItemAsset(source_sim,
														gAgent.getID(),
														gAgent.getSessionID(),
														item->getPermissions().getOwner(),
														LLUUID::null,
														item->getUUID(),
														item->getAssetUUID(),
														item->getType(),
														&onNotecardLoadComplete,
														(void*)new_uuid,
														TRUE);
						success = TRUE;
						mInitialized = true;
						run();
					}
				}
			}
			else // item
			{
				//llwarns << "no item (notecard)") << llendl;
			}
		}
	}
	else // notecard null
	{
		//llwarns << "Config Notecard set to a null UUID!" << llendl;
	}

	return success;
}

void AOEngine::run()
{
	// Should handle this better in the future
	if (!mInitialized)
	{
		llwarns << "AO trying to run without being initialized yet!" << llendl;
		return;
	}

	if (!(gSavedSettings.getBOOL("AOEnabled")))
	{
		return;
	}

	//setCurrentState(EAOState::UNKNOWN); // reset state
	EAOState::State state = getCurrentState(); // check if sitting or hovering
	if ((state == EAOState::UNKNOWN) || (state == EAOState::STAND))
	{
		if (gSavedSettings.getBOOL("AOStandRandomize"))
		{
			if (mAOStandTimer)
			{
				mAOStandTimer->reset();
				//cycleStand();
				// switch to a new stand
				LLUUID anim_id = getOverrideID(EAOState::STAND);
				gAgent.sendAnimationRequest(anim_id, ANIM_REQUEST_START);
			}
			else
			{
				mAOStandTimer =	new AOStandTimer();
			}
		}
		else
		{
			// stop stand first then set state
			LLUUID override_id = getLastPlayedIDFromState(EAOState::STAND);
			gAgent.sendAnimationRequest(override_id, ANIM_REQUEST_STOP);
			setCurrentState(EAOState::UNKNOWN);
		}
	}
	else
	{
		if (EAOState::SIT == state)
		{
			gAgent.sendAnimationRequest(getOverrideID(state), (gSavedSettings.getBOOL("AOEnabled") && gSavedSettings.getBOOL("AOSitsEnabled")) ? ANIM_REQUEST_START : ANIM_REQUEST_STOP);
		}
		else
		{
			gAgent.sendAnimationRequest(getOverrideID(state), gSavedSettings.getBOOL("AOEnabled") ? ANIM_REQUEST_START : ANIM_REQUEST_STOP);
		}
	}
}

// static
void AOEngine::onNotecardLoadComplete(LLVFS *vfs,const LLUUID& asset_uuid, LLAssetType::EType type, void* user_data, S32 status, LLExtStat ext_status)
{
	if ((status == LL_ERR_NOERR) && (type == LLAssetType::AT_NOTECARD))
	{
		S32 size = vfs->getSize(asset_uuid, type);
		U8* buffer = new U8[size];
		vfs->getData(asset_uuid, type, buffer, 0, size);

		LLViewerTextEditor* edit = new LLViewerTextEditor("", LLRect(0,0,0,0), S32_MAX, "");
		if (edit->importBuffer((char*)buffer, (S32)size))
		{
			llinfos << "ao nc decode success" << llendl;
			std::string card = edit->getText();

			// Legacy viewer AO support (ZHAO-based)
			// TODO: replace this with drag 'n' drop ui support by default
			boost::char_separator<char> sep("\n");
			boost::tokenizer<boost::char_separator<char> > tok_line(card, sep);

			for (boost::tokenizer<boost::char_separator<char> >::iterator line = tok_line.begin(); 
				 line != tok_line.end(); ++line)
			{
				//llinfos << *line << llendl;
				std::string str_line(*line);
				//llinfos << "uncommented line: " << str_line << llendl;

				boost::regex type("^(\\s*)(\\[ )(.*)( \\])");
				boost::smatch what; 
				if (boost::regex_search(str_line, what, type)) 
				{
					LL_DEBUGS("AO") << "type: " << what[0] << LL_ENDL;
					LL_DEBUGS("AO") << "anims in type: " << boost::regex_replace(str_line, type, "") << LL_ENDL;

					boost::char_separator<char> sep("|,");
					std::string str_anim_names(boost::regex_replace(str_line, type, ""));
					boost::tokenizer<boost::char_separator<char> > tok_anim_names(str_anim_names, sep);
					for (boost::tokenizer<boost::char_separator<char> >::iterator anim = tok_anim_names.begin(); anim != tok_anim_names.end(); ++anim)
					{
						std::string str_token(what[0]);
						LLStringUtil::trim(str_token);

						std::string str_anim_name(*anim);
						LLUUID anim_id(getAssetIDByName(str_anim_name));

						LL_DEBUGS("AO") << "Found animation: " << str_anim_name.c_str() 
										<< " (asset id: " << anim_id 
										<< ") in folder: " << AOEngine::getInstance()->getInvFolderID()
										<< LL_ENDL;

						if (anim_id.isNull())
						{
							LL_DEBUGS("AO") << llformat("Warning: animation '%s' could not be found (Section: %s).", str_anim_name.c_str(), str_token.c_str()) 
											<< LL_ENDL;
							continue; 
						}

						EAOState::State state = sStateList->getStateFromToken(str_token);
						if (EAOState::UNKNOWN != state)
						{
							AOEngine::getInstance()->addAnim(state, anim_id, str_anim_name);
							// TODO: random prefs for everyone
							if (EAOState::STAND == state)
							{
								AOEngine::getInstance()->setRandom(EAOState::STAND, gSavedSettings.getBOOL("AOStandRandomize"));
							}
						}
						else
						{
							llwarns << "No state associated with token: '" << str_token 
									<< "' length: " << str_token.length()
									<< ". Cannot add anything to the AO" << llendl;
						}
					}
				}
			edit->die(); // let's not leak this, shall we?
			}
		}
		delete buffer;
	}
}

void AOEngine::reset()
{
	mInitialized = false;
	init();
}

void AOEngine::addAnim(EAOState::State state, const LLUUID& anim_id, const std::string& anim_name)
{
	LL_DEBUGS("AO") << "Attempting to add anim " << anim_name << " (" << anim_id 
					<< ") from token: " << sStateList->getTokenFromState(state) << LL_ENDL;
	AOOverride* ao_override = getOverrideFromState(state);
	if (ao_override)
	{
		ao_override->addOverride(anim_id, anim_name);
	}
	else
	{
		mAOList[state] = new AOOverride(anim_id, anim_name);		
	}
}

AOOverride* AOEngine::getOverrideFromState(EAOState::State state)
{
	std::map<EAOState::State, AOOverride*>::iterator mIt = mAOList.find(state);
	if (mIt != mAOList.end())
	{
		return (*mIt).second;
	}
	return NULL;
}

void AOEngine::setRandom(EAOState::State state, bool random)
{
	AOOverride* ao_override = AOEngine::getInstance()->getOverrideFromState(state);
	if (ao_override)
	{
		ao_override->setRandom(random);
	}
}

// Needed?
bool AOEngine::hasOverride(EAOState::State state)
{
	return (mAOList.find(state) != mAOList.end());
}

LLUUID AOEngine::getOverrideID(const LLUUID& sim_anim_id)
{
	if (sim_anim_id.notNull())
	{
		EAOState::State state = sStateList->getStateFromSimAnimID(sim_anim_id);
		if (EAOState::UNKNOWN != state)
		{
			// we know this anim, get an override if we have one!
			return getOverrideID(state);
		}
	}
	return LLUUID::null;
}

LLUUID AOEngine::getOverrideID(EAOState::State state)
{
	std::map<EAOState::State, AOOverride*>::iterator mIt = mAOList.find(state);
	if (mIt != mAOList.end())
	{
		if ((*mIt).second->isRandom())
		{
			return (*mIt).second->getRandomOverrideID();
		}
		else
		{
			return (*mIt).second->getSelectedOverrideID();
		}
	}
	return LLUUID::null;
}

LLUUID AOEngine::getLastPlayedIDFromState(EAOState::State state)
{ 
	AOOverride* ao_override = getOverrideFromState(state);
	if (ao_override)
	{
		return ao_override->getLastPlayedID();
	}
	return LLUUID::null;
}

void AOEngine::setLastPlayedIDForState(EAOState::State state, const LLUUID& anim_id) 
{
	AOOverride* ao_override = getOverrideFromState(state);
	if (ao_override)
	{
		ao_override->setLastPlayedID(anim_id);
		setLastPlayedIDEver(anim_id); // so we always keep these in sync
	}
}

// 
// For LLVOAvatar
// 

LLUUID AOEngine::getOverride(const LLUUID& sim_anim_id, bool is_starting)
{
	if (sim_anim_id.notNull() && gSavedSettings.getBOOL("AOEnabled") && mInitialized && !mAOList.empty())
	{
		LLUUID override_id = LLUUID::null;
		EAOState::State state = sStateList->getStateFromSimAnimID(sim_anim_id);
		if (is_starting)
		{
			override_id = getOverrideID(state);
			if (override_id.notNull())
			{
				if ((EAOState::STAND == state) && gSavedSettings.getBOOL("AONoStandsInMouselook") && gAgent.cameraMouselook())
				{
					gAgent.sendAnimationRequest(getLastPlayedIDFromState(EAOState::STAND), ANIM_REQUEST_STOP);
					LL_DEBUGS("AO") << "Trying to change stand while in mouselook with mouselook stands disabled" << LL_ENDL;
					return LLUUID::null;
				}

				if (((EAOState::SIT			== state) || 
					 (EAOState::SIT			== getCurrentState()) ||
					 (EAOState::SIT_GROUND	== state) || 
					 (EAOState::SIT_GROUND	== getCurrentState() ) && 
					  !(gSavedSettings.getBOOL("AOSitsEnabled"))))
				{
					// make sure we stop any sits we have enabled
					gAgent.sendAnimationRequest(getLastPlayedIDFromState(EAOState::SIT), ANIM_REQUEST_STOP);
					gAgent.sendAnimationRequest(getLastPlayedIDFromState(EAOState::SIT_GROUND), ANIM_REQUEST_STOP);
					LL_DEBUGS("AO") << "Trying to change sit with sits disabled" << LL_ENDL;
					return LLUUID::null;
				}

				// stop the last played in this state as well as the last played period in case the ao gets stuck somewhere
				gAgent.sendAnimationRequest(getLastPlayedIDFromState(state), ANIM_REQUEST_STOP);
				gAgent.sendAnimationRequest(getLastPlayedIDEver(), ANIM_REQUEST_STOP);

				setLastPlayedIDForState(state, override_id);

				if (ANIM_AGENT_CUSTOMIZE		== sim_anim_id ||
					ANIM_AGENT_CUSTOMIZE_DONE	== sim_anim_id ||
					ANIM_AGENT_LAND				== sim_anim_id ||
					ANIM_AGENT_MEDIUM_LAND		== sim_anim_id ||
					ANIM_AGENT_PRE_JUMP			== sim_anim_id ||
					ANIM_AGENT_SIT_GROUND		== sim_anim_id ||
					ANIM_AGENT_STANDUP			== sim_anim_id)
				{
					gAgent.sendAnimationRequest(sim_anim_id, ANIM_REQUEST_START);
				}

				LL_DEBUGS("AO") << "Attempting to override " << sim_anim_id 
								<< " with " << override_id 
								<< " (token: " << sStateList->getTokenFromState(state) << ")"
								<< LL_ENDL;

				setLastOverriddenID(sim_anim_id);

				return override_id;
			}
		}
		else // stopping override
		{
			if (ANIM_AGENT_CUSTOMIZE		== sim_anim_id ||
				ANIM_AGENT_CUSTOMIZE_DONE	== sim_anim_id ||
				ANIM_AGENT_LAND				== sim_anim_id ||
				ANIM_AGENT_MEDIUM_LAND		== sim_anim_id ||
				ANIM_AGENT_PRE_JUMP			== sim_anim_id ||
				ANIM_AGENT_SIT_GROUND		== sim_anim_id ||
				ANIM_AGENT_STANDUP			== sim_anim_id)
			{
				gAgent.sendAnimationRequest(sim_anim_id, ANIM_REQUEST_STOP);
			}

			// returns null if there isn't one
			return getLastPlayedIDFromState(state);
		}
	}
	return LLUUID::null;
}

void AOEngine::changedUnderwater()
{
	if (gSavedSettings.getBOOL("AOEnabled"))
	{
		// surfacing is usually the problem
		gAgent.sendAnimationRequest(getLastOverriddenID(), ANIM_REQUEST_STOP);
		gAgent.getAvatarObject()->stopMotion(getLastOverriddenID());

		gAgent.sendAnimationRequest(ANIM_AGENT_HOVER, ANIM_REQUEST_START);
		LLUUID override_id = getOverride(getLastOverriddenID(), true);
		gAgent.getAvatarObject()->startMotion(getLastOverriddenID());		
	}
}

//static 
AOState* AOEngine::getStateList()
{
	if (!sStateList)
	{
		sStateList = new AOState();
	}
	return sStateList;
}


class ObjectNameMatches : public LLInventoryCollectFunctor
{
public:
	ObjectNameMatches(std::string name)
	{
		sName = name;
	}

	virtual ~ObjectNameMatches() 
	{
	}

	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item)
	{
		if (item)
		{
			if (item->getParentUUID() == AOEngine::getInstance()->getInvFolderID())
			{
				return (item->getName() == sName);
			}
			return false;
		}
		return false;
	}

private:
	std::string sName;
};


// static
const LLUUID& AOEngine::getAssetIDByName(const std::string& name)
{
	if (name.empty())
	{
		return LLUUID::null;
	}

	LLViewerInventoryCategory::cat_array_t cats;
	LLViewerInventoryItem::item_array_t items;
	ObjectNameMatches object_name_matches(name);
	gInventory.collectDescendentsIf(LLUUID::null, cats, items, FALSE, object_name_matches);

	if (items.count())
	{
		return items[0]->getAssetUUID();
	}
	return LLUUID::null;
};

// -------------------------------------------------------


AOStandTimer::AOStandTimer() : LLEventTimer( gSavedSettings.getF32("AOStandInterval") )
{
	AOStandTimer::tick();
}
AOStandTimer::~AOStandTimer()
{
//	llinfos << "dead" << llendl;
}
void AOStandTimer::reset()
{
	mPeriod = gSavedSettings.getF32("AOStandInterval");
	mEventTimer.reset();
//	llinfos << "reset" << llendl;
}
BOOL AOStandTimer::tick()
{
	///AOEngine::sStandIterator++;
//	llinfos << "tick" << llendl;
	///AOEngine::getInstance()->cycleStand();
	return FALSE;
}

// -------------------------------------------------------
