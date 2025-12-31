/************
 * NAME     : COMBAT.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMBAT_H
#define COMBAT_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <COMOBS.H>
#include <MAGIC.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_MONSTER_TYPES		(1)

#define COMBAT_MUSIC				(1)

#define COMBAT_ITEM_SOUND_PRIORITY		(100)
#define COMBAT_ITEM_SOUND_VOLUME			(127)
#define COMBAT_ITEM_SOUND_VARIABILITY	(50)

#define ENVIRONMENT_SPEED		(50)

#define MOVE_SPEED_FACTOR		(40)
#define MAX_COMBAT_MOVES		(3)

#define DAMAGE_DISPLAY_INTERVAL	(20)

#define COMBAT_PARTY_MASK		(0x3ffc0000)
#define COMBAT_MONSTER_MASK	(0x00ffffff)

/* Combat status */
#define COMBAT_RUNNING			(0)
#define COMBAT_PARTY_WON		(1)
#define COMBAT_PARTY_FLED		(2)
#define COMBAT_PARTY_LOST		(3)

/* Monster animations */
#define NO_COMANIM				(0xFFFF)
#define MOVE_COMANIM				(0)
#define CLOSE_RANGE_COMANIM	(1)
#define LONG_RANGE_COMANIM		(2)
#define MAGIC_COMANIM			(3)
#define HIT_COMANIM				(4)
#define DIE_COMANIM				(5)
#define APPEAR_COMANIM			(6)
#define SPECIAL_COMANIM			(7)

/* Participant types */
#define EMPTY_PART_TYPE		(0)
#define PARTY_PART_TYPE		(1)
#define MONSTER_PART_TYPE	(2)

/* Combat target select modes */
#define NO_TARGMODE				(0)
#define FRIEND_TARGMODE			(1)
#define ENEMY_TARGMODE			(2)
#define FRIEND_ROW_TARGMODE	(3)
#define ENEMY_ROW_TARGMODE		(4)
#define ALL_FRIENDS_TARGMODE	(5)
#define ALL_ENEMIES_TARGMODE	(6)
#define ITEM_TARGMODE			(7)
#define SPECIAL_TARGMODE		(8)

/* Combat event types */
#define PART_COMBAT_EVENT				(0)
#define ENVIRONMENT_COMBAT_EVENT		(1)

/*
 ** Structure definitions **************************************************
 */

/* Combat event */
struct Combat_event {
	UNSHORT Type;	  							/* See above */
	UNSHORT Speed;								/* Time variable */
	struct Combat_participant *Part;		/* Pointer to participant data */
};

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

	struct COMOB *Main_COMOB;			/* Pointer to COMbat OBjects */
	struct COMOB *Shadow_COMOB;

	UNSHORT Gfx_width, Gfx_height;	/* Graphics dimensions approximations */

	/* Tactical window data */
	UNSHORT Tactical_X, Tactical_Y;	/* Position in tactical window */
	UNSHORT Tactical_icon_nr;			/* Tactical icon number (1...) */

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
#define PART_HURRIED			(1<<0)	/* Haste spell active */
#define PART_MAGIC_CAP		(1<<1)	/* Monster capabilities */
#define PART_CLOSE_CAP		(1<<2)	/* (DO NOT CHANGE) */
#define PART_LONG_CAP		(1<<3)
#define PART_WAVEDIR			(1<<4)	/* Wave animation direction */
#define PART_UNCONTROL		(1<<5)	/* Participant is uncontrollable */
#define PART_RECOLOURED		(1<<6)	/* Graphics have been recoloured */
#define PART_HAND_ANIMATED	(1<<7)	/* Is animated by hand */

/*
 ** External global variables **********************************************
 */

extern BOOLEAN In_Combat;
extern BOOLEAN Fighting;
extern BOOLEAN End_combat_flag;

extern UNLONG Combat_square_width;
extern UNLONG Combat_square_depth;

extern MEM_HANDLE Combat_background_handle;
extern MEM_HANDLE Tactical_icons_handle;

extern MEM_HANDLE Transparency_table_handle;
extern MEM_HANDLE Luminance_table_handle;
extern MEM_HANDLE Transluminance_table_handle;

extern UNSHORT Tactical_window_object;

extern UNLONG Gained_experience_points;

extern UNSHORT Nr_monsters;
extern UNSHORT Remaining_monsters;
extern UNSHORT Remaining_members;
extern UNSHORT Fled_members;

extern struct Combat_participant *Current_acting_part;
extern struct Combat_participant *Current_victim_part;
extern UNSHORT Combat_text_damage;
extern UNSHORT Combat_text_weapon_item_index;

extern struct Combat_participant Party_parts[6];
extern struct Combat_participant Monster_parts[MONSTERS_PER_GROUP];

/*
 ** Prototypes *************************************************************
 */

void Enter_Combat(UNSHORT Monster_group_nr, UNSHORT Combat_background_nr);

void Combat_ModInit(void);
void Combat_ModExit(void);

void Init_Combat_display(void);
void Exit_Combat_display(void);
void Update_Combat_display(void);

void Combat_round(void);
BOOLEAN End_of_battle(void);
void Update_control_status(void);
void Remove_participant(struct Combat_participant *Part);
void Reset_combat_actions(void);

void Determine_monster_actions(void);
void Select_monster_action(struct Combat_participant *Part);

void Determine_uncontrollable_actions(void);
void Select_uncontrollable_action(struct Combat_participant *Part);

void Create_sorted_combat_event_list(void);

void Swap_combat_events(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_combat_events(UNLONG A, UNLONG B, UNBYTE *Data);

BOOLEAN Init_party_combat_data(void);
BOOLEAN Init_monster_combat_data(UNSHORT Monster_group_nr);

MEM_HANDLE Clone_monster_data(MEM_HANDLE Original_data_handle);

void Do_monster_appear_animations(void);

void Set_combat_animation(struct Combat_participant *Part, UNSHORT Anim_type);
BOOLEAN Has_combat_animation(struct Combat_participant *Part,
 UNSHORT Anim_type);

void Wait_4_combat_animation(struct Combat_participant *Part);
void Update_combat_animations(void);
void Update_monster_animation(struct Combat_participant *Monster_part);

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
void Kill_monster(struct Combat_participant *Part);
void Destroy_monster(struct Combat_participant *Part);

#endif

