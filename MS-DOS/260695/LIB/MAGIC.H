/************
 * NAME     : MAGIC.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAGIC_H
#define MAGIC_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_SPELLS_IN_WINDOW	(10)

#define DEFAULT_ITEM_SPELL_STRENGTH	(50)

#define SPELL_WIDTH		(192)
#define SPELL_HEIGHT		(12)

/* Spell area types */
#define CITY_AREA				(0)
#define DUNGEON_AREA			(1)
#define WILDERNESS_AREA		(2)
#define INTERIOR_AREA		(3)
#define COMBAT_AREA			(5)
#define SPACESHIP_AREA		(6)

/* Spell target_types */
#define ONE_FRIEND_TARGET		(1)
#define ROW_FRIENDS_TARGET		(2)
#define ALL_FRIENDS_TARGET		(4)
#define ONE_ENEMY_TARGET		(8)
#define ROW_ENEMIES_TARGET		(16)
#define ALL_ENEMIES_TARGET		(32)
#define ITEM_TARGET				(64)
#define SPECIAL_TARGET			(128)

/* Combat target modes */
#define PART_TARGMODE		(1)
#define FRIEND_TARGMODE		(2)
#define ENEMY_TARGMODE		(3)
#define SQUARE_TARGMODE		(4)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Spell_handler) (UNSHORT Strength);

typedef void (*Spell_per_target_handler)(UNSHORT Member_nr, UNSHORT Strength);

typedef void (*Spell_per_combat_target_handler)
 (struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength);

typedef UNSHORT (*Normal_XSpell_handler) (UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);

typedef UNLONG (*Combat_XSpell_handler) (UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);

/*
 ** Structure definitions **************************************************
 */

/* Use magic data */
struct Use_magic_data {
	UNSHORT Class_nr;
	UNSHORT Spell_nr;

	UNSHORT Source_item_slot_index;
	UNSHORT Target_item_slot_index;

	/* Caster data */
	UNSHORT Casting_member_index;
	MEM_HANDLE Casting_handle;
	struct Combat_participant *Casting_participant;

	/* Non-combat magic target data */
	UNSHORT Party_target_mask;

	/* Combat magic target data */
	UNLONG Combat_target_mask;
	UNSHORT Combat_target_mode;		/* See above */
	UNSHORT Extra_target_data;
};

/* Spell exception handlers */
struct Normal_XSpell {
	UNSHORT Class_nr;
	UNSHORT Spell_nr;
	Normal_XSpell_handler Handler;
};

struct Combat_XSpell {
	UNSHORT Class_nr;
	UNSHORT Spell_nr;
	Combat_XSpell_handler Handler;
};

/* Spell data */
struct Spell_data {
	UNBYTE Area_bits;
	UNBYTE Point_cost;
	UNBYTE Level;
	UNBYTE Target_bits;					/* See above */
	UNBYTE _pad01;
};

/* Spell selection window OID */
struct Spell_window_OID {
	MEM_HANDLE Char_handle;
	struct Spell_object_data *Spell_info;
	UNSHORT *Selected_class_ptr;
	UNSHORT *Selected_spell_ptr;
	UNSHORT Nr_spells;
};

/* Spell selection window object */
struct Spell_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	MEM_HANDLE Char_handle;
	struct Spell_object_data *Spell_info;
	UNSHORT *Selected_class_ptr;
	UNSHORT *Selected_spell_ptr;
	UNSHORT Nr_spells;

	UNSHORT Scroll_bar_object;
};

/* Spell object data */
struct Spell_object_data {
	UNSHORT Status;
	UNSHORT Class_nr;
	UNSHORT Spell_nr;
	UNSHORT Strength;
	UNSHORT Quantity;
};

/* Values for Spell_object_data.Status */
#define SPELL_OK						(0)
#define SPELL_WRONG_AREA			(1)
#define SPELL_TOO_MUCH_SP			(2)
#define SPELL_TOO_MUCH_LP			(3)

/* Spell OID */
struct Spell_OID {
	UNSHORT Number;
};

/* Spell object */
struct Spell_object {
	struct Object Object;
	UNSHORT Number;
};

/*
 ** External global variables **********************************************
 */

extern struct Use_magic_data Current_use_magic_data;

extern MEM_HANDLE Magic_event_set_handle;
extern MEM_HANDLE Magic_event_text_handle;

extern MEM_HANDLE Spell_data_handle;

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_magic_data(void);
void Exit_magic_data(void);

void Cast_spell(UNSHORT Member_nr, UNSHORT Source_item_slot_index);
BOOLEAN Do_cast_spell(struct Event_action *Action);

BOOLEAN Cast_combat_spell(struct Combat_participant *Part,
 UNSHORT Source_item_slot_index);
void Do_cast_combat_spell(struct Combat_participant *Part);

BOOLEAN Enter_spell_target(void);

UNLONG Select_a_tactical_square(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);
UNLONG Select_blink_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);

void Do_all_magic_party_targets(UNSHORT Strength,
 Spell_per_target_handler Handler);

void Do_all_magic_combat_targets(UNSHORT Strength,
 Spell_per_combat_target_handler Handler);
struct Combat_participant *Get_magic_combat_part_target(void);

UNSHORT Handle_spell(void);
void Remove_used_magic_item(MEM_HANDLE Char_handle, UNSHORT Slot_index);

UNSHORT Handle_magical_defense(struct Combat_participant *Defender_part,
 UNSHORT Incoming_strength);

void Learn_spell(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr);

UNSHORT Calculate_required_LP(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr);

UNSHORT Get_spell_area(void);

/* Spell data access functions */
UNSHORT Get_spell_targets(UNSHORT Class_nr, UNSHORT Spell_nr);
UNSHORT Get_spell_areas(UNSHORT Class_nr, UNSHORT Spell_nr);
UNSHORT Get_spell_required_SP(UNSHORT Class_nr, UNSHORT Spell_nr);
UNSHORT Get_spell_level(UNSHORT Class_nr, UNSHORT Spell_nr);

UNCHAR *Get_spell_name(UNSHORT Class_nr, UNSHORT Spell_nr);

/* Spell selection function */
BOOLEAN Select_spell(MEM_HANDLE Char_handle);

/* Spell window methods */
UNLONG Init_Spell_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Spell_window_object(struct Object *Object, union Method_parms *P);

void Update_spell_list(struct Scroll_bar_object *Scroll_bar);

/* Spell methods */
UNLONG Init_Spell_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Spell_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Spell_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Spell_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Spell_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Spell_object(struct Object *Object, union Method_parms *P);

void Draw_spell(struct Spell_object *Spell);

#endif
