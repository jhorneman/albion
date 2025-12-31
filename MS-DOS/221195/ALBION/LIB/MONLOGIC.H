/************
 * NAME     : MONLOGIC.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 18-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MONSTERLOGIC_H
#define MONSTERLOGIC_H

#include <BBDEF.H>
#include <BBOPM.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

typedef BOOLEAN (*Decider_function)(struct Combat_participant *Part);

typedef struct Combat_participant * (*Target_selector_function)(struct Combat_participant *Part);

typedef void (*Attack_result_function)(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

/*
 ** Structure definitions **************************************************
 */

struct Monster_tactic {
	/* Decides whether to act offensively or defensively */
	Decider_function Decider;

	/* Chooses a target */
	Target_selector_function Target_selector;

	/* Handles the result of a successful attack */
	Attack_result_function Attack_result;

	UNLONG Flags;
};

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

struct Monster_tactic *Get_monster_tactic(struct Combat_participant *Part);

void Select_monster_action(struct Combat_participant *Part);

BOOLEAN Check_if_long_range_weapon_has_ammo(struct Combat_participant *Part,
 UNSHORT Weapon_item_slot_index);

void Select_defensive_monster_action(struct Combat_participant *Part);

UNSHORT Make_defensive_horizontal_move(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNLONG Movement_mask);

UNSHORT Choose_monster_target(UNLONG Possible_targets);

#endif

