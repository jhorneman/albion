/************
 * NAME     : COMBAT.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMBAT_H
#define COMBAT_H

#include <BBDEF.H>
#include <COMOBS.H>
#include <MAGIC.H>

/*
 ** Defines ****************************************************************
 */

#define COMBAT_PARTY_MASK		(0x3ffc0000)
#define COMBAT_MONSTER_MASK	(0x00ffffff)

/* Combat status */
#define COMBAT_RUNNING			(0)
#define COMBAT_PARTY_WON		(1)
#define COMBAT_PARTY_FLED		(2)
#define COMBAT_PARTY_LOST		(3)

/* Participant types */
#define EMPTY_PART_TYPE		(0)
#define PARTY_PART_TYPE		(1)
#define MONSTER_PART_TYPE	(2)

/* Participant temporary effect types */
#define POISON_TEMP_EFFECT			(0)
#define HURRY_TEMP_EFFECT			(1)
#define FROZEN_TEMP_EFFECT			(2)
#define BLINDED_TEMP_EFFECT		(3)
#define BERSERKER_TEMP_EFFECT		(4)

#define MAX_TEMP_EFFECTS			(5)

/*
 ** Structure definitions **************************************************
 */

/* Combat participant action target data */
union Combat_target_data {
	struct {										/* Move action */
		UNSHORT Target_square_index;
	} Move_target_data;
	struct {										/* Close- and long-range attack action */
		UNSHORT Target_square_index;
		UNSHORT Weapon_item_slot_index;
	} Attack_target_data;
	struct Use_magic_data Magic_target_data;	/* Magic action */
};

/* Combat participant temporary effect data */
struct Part_temp_effect_data {
	UNSHORT Timer;
	UNSHORT Strength;
};

/* Combat participant data */
struct Combat_participant {
	UNSHORT Type;							/* See above */
	UNSHORT Number;						/* Member index / monster index */
	UNLONG Flags;							/* See below */

	MEM_HANDLE Char_handle;				/* (Data is cloned for monsters) */
	MEM_HANDLE Graphics_handle;

	UNSHORT Show_set_index;				/* 0 = party members */

	/* Animation data */
	UNSHORT Anim_type;					/* Current animation type */
	UNSHORT Anim_index;					/* Current index in animation frame list */

	UNSHORT Default_frame;				/* Default animation frame */

	struct COMOB *Main_COMOB;			/* Pointer to COMbat OBject */

	/* Temporary effect data */
	struct Part_temp_effect_data Temp_effect_data[MAX_TEMP_EFFECTS];

	/* Tactical window data */
	UNSHORT Tactical_X, Tactical_Y;	/* Position in tactical window */
	MEM_HANDLE Tactical_icon_handle;

	/* Damage display data */
	UNSHORT Damage;						/* Damage being displayed */
	UNSHORT Damage_display_timer;

	/* Action data */
	UNLONG Nr_attacks;					/* Number of attacks until now */
	UNSHORT Current_action;
	UNSHORT Weapon_item_index;
	union Combat_target_data Target;	/* Target info for current action */

	/* Danger statistics data */
	UNLONG Total_damage;					/* Total damage done until now */
	UNLONG Mean_damage;					/* Mean damage done until now */

	UNSHORT Work;							/* Work variable */
};

/* Bitmasks for Combat_participant.Flags */
#define PART_HURRIED			(1 << 0)		/* APR doubled */
#define PART_WAVEDIR			(1 << 4)		/* Wave animation direction */
#define PART_UNCONTROL		(1 << 5)		/* Participant is uncontrollable */
#define PART_RECOLOURED		(1 << 6)		/* Graphics have been recoloured */
#define PART_HAND_ANIMATED	(1 << 7)		/* Is animated by hand */
#define PART_SHOW_LP			(1 << 8) 	/* LP will be shown */

#define PART_MAGIC_CAP		(1 << 1)		/* Monster capabilities */
#define PART_CLOSE_CAP		(1 << 2)		/* (DO NOT CHANGE) */
#define PART_LONG_CAP		(1 << 3)

/*
 ** External global variables **********************************************
 */

extern BOOLEAN In_Combat;

extern struct Combat_participant *Current_acting_part;
extern struct Combat_participant *Current_victim_part;
extern UNSHORT Combat_text_damage;
extern UNSHORT Combat_text_weapon_item_index;

extern struct Combat_participant Party_parts[6];
extern struct Combat_participant Monster_parts[MONSTERS_PER_GROUP];

/*
 ** Prototypes *************************************************************
 */

UNSHORT Enter_Combat(UNSHORT Monster_group_nr, UNSHORT Combat_background_nr);

UNLONG Get_close_range_targets(struct Combat_participant *Attacker_part);
UNLONG Do_close_range_targets(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNSHORT Attacker_type);
UNLONG Get_long_range_targets(struct Combat_participant *Attacker_part);
UNLONG Get_movement_range(struct Combat_participant *Part);
UNLONG Get_occupied_move_targets(struct Combat_participant *Part);
UNLONG Get_party_targets(void);
UNLONG Get_monster_targets(void);

void Print_combat_message(struct Combat_participant *Acting_part, struct
 Combat_participant *Victim_part, UNSHORT Damage, UNSHORT Message_nr);
void Print_part_message(struct Combat_participant *Part, UNSHORT Message_nr);

void Do_combat_damage(struct Combat_participant *Part, UNSHORT Damage);
void Kill_participant(struct Combat_participant *Part);
void Destroy_participant(struct Combat_participant *Part);

#endif

