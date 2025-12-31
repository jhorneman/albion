/************
 * NAME     : COMACTS.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 8-3-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMACTS_H
#define COMACTS_H

#include <BBDEF.H>
#include <COMBAT.H>

/*
 ** Defines ****************************************************************
 */

/* Combat actions */
#define NO_COMACT						(0)
#define MOVE_COMACT					(1)
#define CLOSE_RANGE_COMACT			(2)
#define LONG_RANGE_COMACT			(3)
#define FLEE_COMACT					(4)
#define CAST_SPELL_COMACT			(5)
#define USE_MAGIC_ITEM_COMACT		(6)

#define MAX_COMACTS					(7)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Combat_action_handler) (struct Combat_participant *Part);

/*
 ** Structure definitions **************************************************
 */

/* Combat action */
struct Combat_action {
	UNSHORT Condition_mask;
	Combat_action_handler Handler;
};

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Abort_attack;

extern struct Combat_action Combat_action_table[MAX_COMACTS];

/*
 ** Prototypes *************************************************************
 */

void Move_combat_action(struct Combat_participant *Part);

void Close_range_combat_action(struct Combat_participant *Part);

void Long_range_combat_action(struct Combat_participant *Part);

void Flee_combat_action(struct Combat_participant *Part);

void Cast_spell_combat_action(struct Combat_participant *Part);

void Use_magic_object_combat_action(struct Combat_participant *Part);

#endif

