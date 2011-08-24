/*
Info for each AO state
*/

#include "llviewerprecompiledheaders.h"

#include "aostate.h"

#include "llagent.h"
#include "llvoavatar.h"

/*
	This is ugly because the whole process of overriding is ugly.
	
	There are:
	- four stand anims that we want to associate with one state
	- four states that use the same anim (but change when underwater)

	We rely on our getters to sort the mess out -- MC
*/

AOState::AOState()
{
	AOStateItem state_list;

	state_list.state = EAO::UNKNOWN;
	state_list.sim_anim = LLUUID::null;
	state_list.label = "unknown";
	state_list.token = "";
	mStates.push_back(state_list);

	state_list.state = EAO::WALK;
	state_list.sim_anim = ANIM_AGENT_WALK;
	state_list.label = "walk";
	state_list.token = "[ Walking ]";
	mStates.push_back(state_list);

	state_list.state = EAO::RUN;
	state_list.sim_anim = ANIM_AGENT_RUN;
	state_list.label = "run";
	state_list.token = "[ Running ]";
	mStates.push_back(state_list);

	state_list.state = EAO::STAND;
	state_list.sim_anim = ANIM_AGENT_STAND;
	state_list.label = "stand";
	state_list.token = "[ Standing ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::STAND;
	state_list.sim_anim = ANIM_AGENT_STAND_1;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::STAND;
	state_list.sim_anim = ANIM_AGENT_STAND_2;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::STAND;	
	state_list.sim_anim = ANIM_AGENT_STAND_3;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::STAND;
	state_list.sim_anim = ANIM_AGENT_STAND_4;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	state_list.state = EAO::PRE_JUMP;
	state_list.sim_anim = ANIM_AGENT_PRE_JUMP;
	state_list.label = "pre_jump";
	state_list.token = "[ Pre Jumping ]";
	mStates.push_back(state_list);

	state_list.state = EAO::JUMP;
	state_list.sim_anim = ANIM_AGENT_JUMP;
	state_list.label = "jump";
	state_list.token = "[ Jumping ]";
	mStates.push_back(state_list);

	state_list.state = EAO::TURN_LEFT;
	state_list.sim_anim = ANIM_AGENT_TURNLEFT;
	state_list.label = "turn_left";
	state_list.token = "[ Turning Left ]";
	mStates.push_back(state_list);

	state_list.state = EAO::TURN_RIGHT;
	state_list.sim_anim = ANIM_AGENT_TURNRIGHT;
	state_list.label = "turn_right";
	state_list.token = "[ Turning Right ]";
	mStates.push_back(state_list);

	state_list.state = EAO::SIT;
	state_list.sim_anim = ANIM_AGENT_SIT;
	state_list.label = "sit";
	state_list.token = "[ Sitting ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::SIT;
	state_list.sim_anim = ANIM_AGENT_SIT_GENERIC;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::SIT;
	state_list.sim_anim = ANIM_AGENT_SIT_FEMALE;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	state_list.state = EAO::SIT_GROUND;
	state_list.sim_anim = ANIM_AGENT_SIT_GROUND;
	state_list.label = "sit_ground";
	state_list.token = "[ Sitting On Ground ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::SIT_GROUND;
	state_list.sim_anim = ANIM_AGENT_SIT_GROUND_CONSTRAINED;
	state_list.label = "";
	state_list.token = "";
	mStates.push_back(state_list);

	state_list.state = EAO::HOVER;
	state_list.sim_anim = ANIM_AGENT_HOVER;
	state_list.label = "hover";
	state_list.token = "[ Hovering ]";
	mStates.push_back(state_list);

	state_list.state = EAO::HOVER_DOWN;
	state_list.sim_anim = ANIM_AGENT_HOVER_DOWN;
	state_list.label = "fly_down";
	state_list.token = "[ Flying Down ]";
	mStates.push_back(state_list);

	state_list.state = EAO::HOVER_UP;
	state_list.sim_anim = ANIM_AGENT_HOVER_UP;
	state_list.label = "fly_up";
	state_list.token = "[ Flying Up ]";
	mStates.push_back(state_list);

	state_list.state = EAO::CROUCH;
	state_list.sim_anim = ANIM_AGENT_CROUCH;
	state_list.label = "crouch";
	state_list.token = "[ Crouching ]";
	mStates.push_back(state_list);

	state_list.state = EAO::WALK_CROUCH;
	state_list.sim_anim = ANIM_AGENT_CROUCHWALK;
	state_list.label = "walk_crouch";
	state_list.token = "[ Crouch Walking ]";
	mStates.push_back(state_list);

	state_list.state = EAO::FALL;
	state_list.sim_anim = ANIM_AGENT_FALLDOWN;
	state_list.label = "fall";
	state_list.token = "[ Falling ]";
	mStates.push_back(state_list);

	state_list.state = EAO::STANDUP;
	state_list.sim_anim = ANIM_AGENT_STANDUP;
	state_list.label = "standup";
	state_list.token = "[ Standing Up ]";
	mStates.push_back(state_list);

	state_list.state = EAO::LAND;
	state_list.sim_anim = ANIM_AGENT_LAND;
	state_list.label = "land";
	state_list.token = "[ Landing ]";
	mStates.push_back(state_list);

	state_list.state = EAO::LAND_MEDIUM;
	state_list.sim_anim = ANIM_AGENT_MEDIUM_LAND;
	state_list.label = "land_medium";
	state_list.token = "[ Soft Landing ]";
	mStates.push_back(state_list);

	state_list.state = EAO::FLY;
	state_list.sim_anim = ANIM_AGENT_FLY;
	state_list.label = "fly";
	state_list.token = "[ Flying ]";
	mStates.push_back(state_list);

	state_list.state = EAO::FLY_SLOW;
	state_list.sim_anim = ANIM_AGENT_FLYSLOW;
	state_list.label = "fly_slow";
	state_list.token = "[ Flying Slow ]";
	mStates.push_back(state_list);

	state_list.state = EAO::TYPE;
	state_list.sim_anim = ANIM_AGENT_TYPE;
	state_list.label = "type";
	state_list.token = "[ Typing ]";
	mStates.push_back(state_list);

	state_list.state = EAO::WHISPER;
	state_list.sim_anim = ANIM_AGENT_WHISPER;
	state_list.label = "whisper";
	state_list.token = "[ Whispering ]";
	mStates.push_back(state_list);

	state_list.state = EAO::TALK;
	state_list.sim_anim = ANIM_AGENT_TALK;
	state_list.label = "talk";
	state_list.token = "[ Talking ]";
	mStates.push_back(state_list);

	state_list.state = EAO::SHOUT;
	state_list.sim_anim = ANIM_AGENT_SHOUT;
	state_list.label = "shout";
	state_list.token = "[ Shouting ]";
	mStates.push_back(state_list);

	state_list.state = EAO::EDIT_OBJ;
	state_list.sim_anim = ANIM_AGENT_EDITING;
	state_list.label = "edit_obj";
	state_list.token = "[ Editing Objects ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::FLOAT;
	state_list.sim_anim = ANIM_AGENT_HOVER;
	state_list.label = "float";
	state_list.token = "[ Floating ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::SWIM_FORWARD;
	state_list.sim_anim = ANIM_AGENT_FLY;
	state_list.label = "swim_forward";
	state_list.token = "[ Swimming Forward ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::SWIM_UP;
	state_list.sim_anim = ANIM_AGENT_HOVER_UP;
	state_list.label = "swim_up";
	state_list.token = "[ Swimming Up ]";
	mStates.push_back(state_list);

	// special
	state_list.state = EAO::SWIM_DOWN;
	state_list.sim_anim = ANIM_AGENT_HOVER_DOWN;
	state_list.label = "swim_down";
	state_list.token = "[ Swimming Down ]";
	mStates.push_back(state_list);

	// ++ the magic number when adding a duplicated state
	if (mStates.size() != EAO::COUNT+7)
	{
		llerrs << "AO states (" << EAO::COUNT+7
			   << ") and items (" << mStates.size()
			   << ") don't match!" << llendl;
	}

	std::sort(mStates.begin(), mStates.end());
}

AOState::~AOState()
{
}

// TODO: templatize these

EAO::State AOState::getStateFromSimAnimID(const LLUUID& sim_anim)
{
	LL_DEBUGS("AO") << "searching for anim: '" << sim_anim << "'" << LL_ENDL;

	if (sim_anim.notNull() && !mStates.empty())
	{
		for (std::vector<AOStateItem>::iterator vIt = mStates.begin(); vIt != mStates.end(); ++vIt)
		{
			if ((*vIt).sim_anim == sim_anim)
			{
				EAO::State state = (*vIt).state;

				// this is the only way to check for swimming, ick
				if (gAgent.getAvatarObject()->mBelowWater && 
					!(gAgent.getAvatarObject()->mIsSitting) &&
					!(gAgent.getRunning()))
				{
					if		(state == EAO::HOVER)		state = EAO::FLOAT;
					else if (state == EAO::FLY)			state = EAO::SWIM_FORWARD;
					else if (state == EAO::HOVER_UP)	state = EAO::SWIM_UP;
					else if (state == EAO::HOVER_DOWN)	state = EAO::SWIM_DOWN;
				}
				else
				{
					if		(state == EAO::FLOAT)			state = EAO::HOVER;
					else if (state == EAO::SWIM_FORWARD)	state = EAO::FLY;
					else if (state == EAO::SWIM_UP)			state = EAO::HOVER_UP;
					else if (state == EAO::SWIM_DOWN)		state = EAO::HOVER_DOWN;
				}
				// returns the same EAO state for different anims
				// we need to stop/start all variants in AOEngine
				return state;
			}
		}
	}
	LL_DEBUGS("AO") << "anim's state not found in map!" << LL_ENDL;

	return EAO::UNKNOWN;
}

const LLUUID& AOState::getSimAnimIDFromState(EAO::State state)
{
	LL_DEBUGS("AO") << "searching for state: " << state << LL_ENDL;

	if (state < EAO::COUNT && !mStates.empty())
	{
		for (std::vector<AOStateItem>::iterator vIt = mStates.begin(); vIt != mStates.end(); ++vIt)
		{
			if ((*vIt).state == state)
			{
				// Whether we're above or below water, we'll get the right anim
				// Also returns the first anim associated with a state *only*
				// we need to stop/start all variants in AOEngine
				return (*vIt).sim_anim;
			}
		}
	}
	LL_DEBUGS("AO") << "state not found!" << LL_ENDL;

	return LLUUID::null;
}

EAO::State AOState::getStateFromToken(const std::string& str_token)
{
	LL_DEBUGS("AO") << "searching for token: '" << str_token << "' length: " << str_token.length() << LL_ENDL;

	if (!str_token.empty() && !mStates.empty())
	{
		for (std::vector<AOStateItem>::iterator vIt = mStates.begin(); vIt != mStates.end(); ++vIt)
		{
			if ((*vIt).token == str_token)
			{
				// Whether we're above or below water, we'll get the right state
				return (*vIt).state;
			}
		}
	}
	LL_DEBUGS("AO") << "token's state not found!" << LL_ENDL;

	return EAO::UNKNOWN;
}

std::string AOState::getTokenFromState(EAO::State state)
{
	LL_DEBUGS("AO") << "searching for state: " << state << LL_ENDL;

	if (state < EAO::COUNT && !mStates.empty())
	{
		for (std::vector<AOStateItem>::iterator vIt = mStates.begin(); vIt != mStates.end(); ++vIt)
		{
			if ((*vIt).state == state)
			{
				// All states should have a token and a label
				return (*vIt).token;
			}
		}
	}
	LL_DEBUGS("AO") << "state not found!" << LL_ENDL;

	return "";
}

std::string AOState::getLabelFromState(EAO::State state)
{
	LL_DEBUGS("AO") << "searching for state: " << state << LL_ENDL;

	if (state < EAO::COUNT && !mStates.empty())
	{
		for (std::vector<AOStateItem>::iterator vIt = mStates.begin(); vIt != mStates.end(); ++vIt)
		{
			if ((*vIt).state == state)
			{
				// All states should have a token and a label
				return (*vIt).label;
			}
		}
	}
	LL_DEBUGS("AO") << "state not found!" << LL_ENDL;

	return "";
}

EAO::State AOState::getStateFromLabel(const std::string& label)
{
	LL_DEBUGS("AO") << "searching for label: '" << label << "' length: " << label.length() << LL_ENDL;

	if (!label.empty() && !mStates.empty())
	{
		for (std::vector<AOStateItem>::iterator vIt = mStates.begin(); vIt != mStates.end(); ++vIt)
		{
			if ((*vIt).label == label)
			{
				// Whether we're above or below water, we'll get the right state
				return (*vIt).state;
			}
		}
	}
	LL_DEBUGS("AO") << "label's state not found!" << LL_ENDL;

	return EAO::UNKNOWN;
}
