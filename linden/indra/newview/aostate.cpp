/*
Info for each AO state
*/

#include "llviewerprecompiledheaders.h"

#include "aostate.h"

#include "llagent.h"
#include "llvoavatar.h"

// map of tokens for getting state info from notecards
static std::map<std::string, EAOState::State> sAONotecardTokens;
// map of uuids associated with each state
static std::map<LLUUID, EAOState::State> sAOSimAnimIDs;

AOState::AOState()
{
	// associate each state with the default anim uuid(s) the server knows
	// EAOState::UNKNOWN is 0 - no state set. No anims are associated with this
	sAOSimAnimIDs[ANIM_AGENT_WALK]					= EAOState::WALK;
	sAOSimAnimIDs[ANIM_AGENT_RUN]					= EAOState::RUN;
	sAOSimAnimIDs[ANIM_AGENT_STAND]					= EAOState::STAND;
	sAOSimAnimIDs[ANIM_AGENT_STAND_1]				= EAOState::STAND;
	sAOSimAnimIDs[ANIM_AGENT_STAND_2]				= EAOState::STAND;
	sAOSimAnimIDs[ANIM_AGENT_STAND_3]				= EAOState::STAND;
	sAOSimAnimIDs[ANIM_AGENT_STAND_4]				= EAOState::STAND;
	sAOSimAnimIDs[ANIM_AGENT_PRE_JUMP]				= EAOState::PRE_JUMP;
	sAOSimAnimIDs[ANIM_AGENT_JUMP]					= EAOState::JUMP;
	sAOSimAnimIDs[ANIM_AGENT_TURNLEFT]				= EAOState::TURN_LEFT;
	sAOSimAnimIDs[ANIM_AGENT_TURNRIGHT]				= EAOState::TURN_RIGHT;
	sAOSimAnimIDs[ANIM_AGENT_SIT]					= EAOState::SIT;
	sAOSimAnimIDs[ANIM_AGENT_SIT_FEMALE]			= EAOState::SIT;
	sAOSimAnimIDs[ANIM_AGENT_SIT_GENERIC]			= EAOState::SIT;
	sAOSimAnimIDs[ANIM_AGENT_SIT_GROUND]			= EAOState::SIT_GROUND;
	sAOSimAnimIDs[ANIM_AGENT_SIT_GROUND_CONSTRAINED] = EAOState::SIT_GROUND;
	sAOSimAnimIDs[ANIM_AGENT_HOVER]					= EAOState::HOVER;
	sAOSimAnimIDs[ANIM_AGENT_HOVER_DOWN]			= EAOState::HOVER_DOWN;
	sAOSimAnimIDs[ANIM_AGENT_HOVER_UP]				= EAOState::HOVER_UP;
	sAOSimAnimIDs[ANIM_AGENT_CROUCH]				= EAOState::CROUCH;
	sAOSimAnimIDs[ANIM_AGENT_CROUCHWALK]			= EAOState::WALK_CROUCH;
	sAOSimAnimIDs[ANIM_AGENT_FALLDOWN]				= EAOState::FALL;
	sAOSimAnimIDs[ANIM_AGENT_STANDUP]				= EAOState::STANDUP;
	sAOSimAnimIDs[ANIM_AGENT_LAND]					= EAOState::LAND;
	sAOSimAnimIDs[ANIM_AGENT_MEDIUM_LAND]			= EAOState::LAND_MEDIUM;
	sAOSimAnimIDs[ANIM_AGENT_FLY]					= EAOState::FLY;
	sAOSimAnimIDs[ANIM_AGENT_FLYSLOW]				= EAOState::FLY_SLOW;
	sAOSimAnimIDs[ANIM_AGENT_TYPE]					= EAOState::TYPE;
	sAOSimAnimIDs[ANIM_AGENT_SHOUT]					= EAOState::SHOUT;
	sAOSimAnimIDs[ANIM_AGENT_CUSTOMIZE]				= EAOState::CUSTOMIZE;
	sAOSimAnimIDs[ANIM_AGENT_CUSTOMIZE_DONE]		= EAOState::CUSTOMIZE_DONE;

	// these states are set when flying underwater - see hasOverride and getOverrideState
	//EAOState::FLOAT (ANIM_AGENT_HOVER)
	//EAOState::SWIM_FORWARD (ANIM_AGENT_FLY)
	//EAOState::SWIM_UP (ANIM_AGENT_HOVER_UP)
	//EAOState::SWIM_DOWN (ANIM_AGENT_HOVER_DOWN)

	// associate strings with EAOState
	sAONotecardTokens["[ Walking ]"]			= EAOState::WALK;
	sAONotecardTokens["[ Running ]"]			= EAOState::RUN;
	sAONotecardTokens["[ Standing ]"]			= EAOState::STAND;
	sAONotecardTokens["[ Pre Jumping ]"]		= EAOState::PRE_JUMP;
	sAONotecardTokens["[ Jumping ]"]			= EAOState::JUMP;
	sAONotecardTokens["[ Turning Left ]"]		= EAOState::TURN_LEFT;
	sAONotecardTokens["[ Turning Right ]"]		= EAOState::TURN_RIGHT;
	sAONotecardTokens["[ Sitting ]"]			= EAOState::SIT;
	sAONotecardTokens["[ Sitting On Ground ]"]	= EAOState::SIT_GROUND;
	sAONotecardTokens["[ Hovering ]"]			= EAOState::HOVER;
	sAONotecardTokens["[ Flying Up ]"]			= EAOState::HOVER_UP;
	sAONotecardTokens["[ Flying Down ]"]		= EAOState::HOVER_DOWN;
	sAONotecardTokens["[ Crouch ]"]				= EAOState::CROUCH;
	sAONotecardTokens["[ Crouch Walking ]"]		= EAOState::WALK_CROUCH;
	sAONotecardTokens["[ Falling ]"]			= EAOState::FALL;
	sAONotecardTokens["[ Standing Up ]"]		= EAOState::STANDUP;
	sAONotecardTokens["[ Landing ]"]			= EAOState::LAND;
	sAONotecardTokens["[ Soft Landing ]"]		= EAOState::LAND_MEDIUM;
	sAONotecardTokens["[ Flying ]"]				= EAOState::FLY;
	sAONotecardTokens["[ Flying Slow ]"]		= EAOState::FLY_SLOW;
	sAONotecardTokens["[ Typing ]"]				= EAOState::TYPE;
	sAONotecardTokens["[ Shouting ]"]			= EAOState::SHOUT;
	sAONotecardTokens["[ Floating ]"]			= EAOState::FLOAT;
	sAONotecardTokens["[ Swimming Forward ]"]	= EAOState::SWIM_FORWARD;
	sAONotecardTokens["[ Swimming Up ]"]		= EAOState::SWIM_UP;
	sAONotecardTokens["[ Swimming Down ]"]		= EAOState::SWIM_DOWN;
	sAONotecardTokens["[ Edit Appearance Begin ]"] = EAOState::CUSTOMIZE;
	sAONotecardTokens["[ Edit Appearance End ]"] = EAOState::CUSTOMIZE_DONE;

	// make sure we always account for everything in both lists
	if (EAOState::COUNT != (sAONotecardTokens.size()+1))
	{
		llerrs << "AO states and tokens aren't synced!" << llendl;
	}
}

AOState::~AOState()
{
}

EAOState::State AOState::getStateFromSimAnimID(const LLUUID& sim_anim_id)
{
	LL_DEBUGS("AO") << "searching for anim: '" << sim_anim_id << "'" << LL_ENDL;

	EAOState::State state = EAOState::UNKNOWN;
	if (sAOSimAnimIDs.empty())
	{
		return state;
	}

	if (sim_anim_id.notNull())
	{
		std::map<LLUUID, EAOState::State>::iterator mIt = sAOSimAnimIDs.find(sim_anim_id);
		if (mIt != sAOSimAnimIDs.end())
		{
			state = (*mIt).second;

			// this is the only way to check for being underwater, really
			if (gAgent.getAvatarObject() && gAgent.getAvatarObject()->mBelowWater && !(gAgent.getAvatarObject()->mIsSitting))
			{
				if		(state == EAOState::HOVER)		state = EAOState::FLOAT;
				else if (state == EAOState::FLY)		state = EAOState::SWIM_FORWARD;
				else if (state == EAOState::HOVER_UP)	state = EAOState::SWIM_UP;
				else if (state == EAOState::HOVER_DOWN)	state = EAOState::SWIM_DOWN;
			}

			return state;
		}
	}
	LL_DEBUGS("AO") << "anim's state not found in map!" << LL_ENDL;
	return state;
}

const LLUUID& AOState::getSimAnimIDFromState(EAOState::State state)
{
	for (std::map<LLUUID, EAOState::State>::iterator mIt = sAOSimAnimIDs.begin();
		mIt != sAOSimAnimIDs.end(); ++mIt)
	{
		if ((*mIt).second == state)
		{
			return (*mIt).first;
		}
	}
	return LLUUID::null;
}

EAOState::State AOState::getStateFromToken(const std::string& str_token)
{
	LL_DEBUGS("AO") << "searching for token: '" << str_token << "' length: " << str_token.length() << LL_ENDL;

	EAOState::State state = EAOState::UNKNOWN;
	if (sAONotecardTokens.empty())
	{
		return state;
	}

	std::map<std::string, EAOState::State>::iterator mIt = sAONotecardTokens.find(str_token);
	if (mIt != sAONotecardTokens.end())
	{
		return (*mIt).second;
	}
	LL_DEBUGS("AO") << "token's state not found in map!" << LL_ENDL;
	return state;
}

std::string AOState::getTokenFromState(EAOState::State state)
{
	for (std::map<std::string, EAOState::State>::iterator mIt = sAONotecardTokens.begin();
		mIt != sAONotecardTokens.end(); ++mIt)
	{
		if ((*mIt).second == state)
		{
			return (*mIt).first;
		}
	}
	return "";
}

bool AOState::hasOverrideState(const LLUUID& sim_anim_id)
{
	if (sim_anim_id.notNull())
	{
		return (sAOSimAnimIDs.find(sim_anim_id) != sAOSimAnimIDs.end());
	}
	return false;
}
