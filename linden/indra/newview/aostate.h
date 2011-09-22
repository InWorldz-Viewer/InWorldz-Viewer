/*
Info for each AO state
*/

#ifndef AO_STATE_H
#define AO_STATE_H

namespace EAOState
{
	enum State
	{
		UNKNOWN = 0,
		WALK,
		RUN,
		STAND,

		PRE_JUMP,
		JUMP,
		TURN_LEFT,
		TURN_RIGHT,

		SIT,
		SIT_GROUND,

		HOVER,
		HOVER_DOWN,
		HOVER_UP,

		CROUCH,
		WALK_CROUCH,
		FALL,
		STANDUP,
		LAND,
		LAND_MEDIUM,

		FLY,
		FLY_SLOW,

		TYPE,
		SHOUT,

		FLOAT,
		SWIM_FORWARD,
		SWIM_UP,
		SWIM_DOWN,

		CUSTOMIZE,
		CUSTOMIZE_DONE,

		COUNT
	};
}

class AOState
{
public:
	AOState();
	virtual ~AOState();

	// Returns the EAOState::State associated with a simulator animation
	EAOState::State getStateFromSimAnimID(const LLUUID& sim_anim_id);
	// Returns the first simulator animation associated with an EAOState::State
	const LLUUID& getSimAnimIDFromState(EAOState::State state);
	// Returns the EAOState::State associated with a notecard token
	EAOState::State getStateFromToken(const std::string& str_token);
	// returns true if a simulator animation can be overridden
	bool hasOverrideState(const LLUUID& sim_anim_id);
	// returns the token associated with the current state
	// returns "" when the state is UNKNOWN or not found
	std::string getTokenFromState(EAOState::State state);
};

#endif
