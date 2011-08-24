/*
Info for each AO state
*/

#ifndef AO_STATE_H
#define AO_STATE_H

namespace EAO
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
		WHISPER,
		TALK,
		SHOUT,

		FLOAT,
		SWIM_FORWARD,
		SWIM_UP,
		SWIM_DOWN,

		EDIT_OBJ,

		COUNT
	};
}

class AOState
{
public:
	AOState();
	virtual ~AOState();
	
private:

	struct AOStateItem
	{
		EAO::State state;		// State of the AO
		std::string token;		// Token when reading ZHAO notecards
		std::string label;		// Label associated with the state (used for UI stuff, debug msgs, etc)
		LLUUID sim_anim;		// Simulator anim we want to override
	};
	
	std::vector<AOStateItem> mStates;

	friend bool operator== (const AOStateItem& item1, const AOStateItem& item2)
	{
	    return ((item1.state == item2.state) &&
	            (item1.token == item2.token) &&
	            (item1.label == item2.label) &&
				(item1.sim_anim == item2.sim_anim));
	}
	 
	friend bool operator!= (const AOStateItem& item1, const AOStateItem& item2)
	{
		return !(item1 == item2);
	}

	friend bool operator> (const AOStateItem& item1, const AOStateItem& item2)
	{
	    return item1.state > item2.state;
	}

	friend bool operator< (const AOStateItem& item1, const AOStateItem& item2)
	{
	    return item1.state < item2.state;
	}

	friend bool operator>= (const AOStateItem& item1, const AOStateItem& item2)
	{
	    return item1.state >= item2.state;
	}

	friend bool operator<= (const AOStateItem& item1, const AOStateItem& item2)
	{
	    return item1.state <= item2.state;
	}

public:

	// Returns the EAO::State associated with a simulator animation
	EAO::State getStateFromSimAnimID(const LLUUID& sim_anim);
	// Returns the first simulator animation associated with an EAO::State
	const LLUUID& getSimAnimIDFromState(EAO::State state);
	// Returns the EAO::State associated with a notecard token
	EAO::State getStateFromToken(const std::string& str_token);
	// returns the token associated with the current state
	// returns "" when the state is UNKNOWN or not found
	std::string getTokenFromState(EAO::State state);
	std::string getLabelFromState(EAO::State state);
	// Returns the EAO::State associated with a label
	EAO::State getStateFromLabel(const std::string& label);
};

#endif
