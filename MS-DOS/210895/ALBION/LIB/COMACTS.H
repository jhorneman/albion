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
BOOLEAN Do_close_range_attack(struct Event_action *Action);

UNSHORT Calculate_afflicted_damage(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

void Long_range_combat_action(struct Combat_participant *Part);
BOOLEAN Do_long_range_attack(struct Event_action *Action);

void Remove_ammo_and_reload(struct Combat_participant *Attacker_part,
 UNSHORT Current_weapon_slot_index);

void Flee_combat_action(struct Combat_participant *Part);

void Cast_spell_combat_action(struct Combat_participant *Part);
BOOLEAN Do_cast_spell_combat(struct Event_action *Action);

void Use_magic_object_combat_action(struct Combat_participant *Part);

void Break_combat_items(MEM_HANDLE Char_handle, UNSHORT Body_item_slot_mask,
 UNLONG Item_type_mask);

#endif

