/************
 * NAME     : COMBVAR.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 15-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMBVAR_H
#define COMBVAR_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <COMOBS.H>
#include <COMBAT.H>
#include <MAGIC.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_MONSTER_TYPES		(1)

#define COMBAT_MUSIC				(26)

#define COMBAT_ITEM_SOUND_PRIORITY		(100)
#define COMBAT_ITEM_SOUND_VOLUME			(127)
#define COMBAT_ITEM_SOUND_VARIABILITY	(50)

#define ENVIRONMENT_SPEED			(50)
#define TEMPORARY_EFFECTS_SPEED	(0)

#define MAX_COMBAT_EVENTS		(6 + MONSTERS_PER_GROUP + 2)

#define MOVE_SPEED_FACTOR		(40)
#define MAX_COMBAT_MOVES		(3)

#define DAMAGE_DISPLAY_INTERVAL	(20)

/* Transparency table offsets */
#define LUMINANCE_TABLE_OFFSET			(65536)
#define TRANSLUMINANCE_TABLE_OFFSET		(2 * 65536)

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

/* Combat event types */
#define PART_COMBAT_EVENT						(0)
#define ENVIRONMENT_COMBAT_EVENT				(1)
#define TEMPORARY_EFFECTS_COMBAT_EVENT		(2)

/*
 ** Structure definitions **************************************************
 */

/* Combat event */
struct Combat_event {
	UNSHORT Type;	  							/* See above */
	UNSHORT Speed;								/* Time variable */
	struct Combat_participant *Part;		/* Pointer to participant data */
};

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Fighting;
extern BOOLEAN End_combat_flag;
extern BOOLEAN Show_monster_LP_flag;

extern UNLONG Combat_square_width;
extern UNLONG Combat_square_depth;

extern MEM_HANDLE Combat_background_handle;
extern MEM_HANDLE Tactical_icons_handle;

extern UNSHORT Tactical_window_object;

extern UNSHORT Combat_status;
extern UNLONG Added_experience_points;

extern UNSHORT Nr_monsters;
extern UNSHORT Remaining_monsters;
extern UNSHORT Remaining_members;
extern UNSHORT Fled_members;

extern UNSHORT Nr_combat_updates;
extern BOOLEAN First_combat_update;

/*
 ** Prototypes *************************************************************
 */

void Combat_round(void);

void Update_control_status(void);
void Remove_participant(struct Combat_participant *Part);

/* Temporary effect management functions */
void Add_temporary_effect(struct Combat_participant *Part,
 UNSHORT Effect_type, UNSHORT Strength, UNSHORT Duration);
void Remove_temporary_effect(struct Combat_participant *Part,
 UNSHORT Effect_type);
void Remove_all_temporary_effects(struct Combat_participant *Part);

/* Monster animation functions */
void Set_combat_animation(struct Combat_participant *Part, UNSHORT Anim_type);
BOOLEAN Has_combat_animation(struct Combat_participant *Part,
 UNSHORT Anim_type);

void Wait_4_combat_animation(struct Combat_participant *Part);
void Update_combat_animations(void);
void Update_monster_animation(struct Combat_participant *Monster_part);

/* Trap management functions */
void Install_trap(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 Trap_handler Handler, UNSHORT Strength, struct COMOB *Trap_COMOB);
void Remove_trap(UNSHORT Tactical_X, UNSHORT Tactical_Y);
BOOLEAN Is_trap(UNSHORT Tactical_X, UNSHORT Tactical_Y);

#endif

