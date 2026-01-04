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

#define DEFAULT_ITEM_SPELL_STRENGTH	(50)

/* Combat target modes */
#define PART_TARGMODE		(1)
#define FRIEND_TARGMODE		(2)
#define ENEMY_TARGMODE		(3)
#define SQUARE_TARGMODE		(4)

/* Spell area types */
#define CITY_AREA				(0)
#define DUNGEON_AREA			(1)
#define WILDERNESS_AREA		(2)
#define INTERIOR_AREA		(3)
#define COMBAT_AREA			(5)
#define SPACESHIP_AREA		(6)

/* Spell target_types */
#define ONE_FRIEND_TARGET		(1 << 0)
#define ROW_FRIENDS_TARGET		(1 << 1)
#define ALL_FRIENDS_TARGET		(1 << 2)
#define ONE_ENEMY_TARGET		(1 << 3)
#define ROW_ENEMIES_TARGET		(1 << 4)
#define ALL_ENEMIES_TARGET		(1 << 5)
#define ITEM_TARGET				(1 << 6)
#define SPECIAL_TARGET			(1 << 7)

#define FRIEND_SPELL_TARGET_MASK		(ONE_FRIEND_TARGET | ROW_FRIENDS_TARGET | ALL_FRIENDS_TARGET)
#define ENEMY_SPELL_TARGET_MASK		(ONE_ENEMY_TARGET | ROW_ENEMIES_TARGET | ALL_ENEMIES_TARGET)

/*
 ** Type definitions *******************************************************
 */

typedef void (*Spell_handler)(UNSHORT Strength);

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

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Reverse_combat_magic_flag;

extern struct Use_magic_data Current_use_magic_data;

extern UNSHORT Spells_per_class[MAX_SPELL_CLASSES];

extern MEM_HANDLE Magic_event_set_handle;
extern MEM_HANDLE Magic_event_text_handle;

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_magic_data(void);
void Exit_magic_data(void);

void Cast_spell(UNSHORT Member_nr, UNSHORT Source_item_slot_index);

BOOLEAN Cast_combat_spell(struct Combat_participant *Part,
 UNSHORT Source_item_slot_index);

void Do_cast_combat_spell(struct Combat_participant *Part);

BOOLEAN Enter_spell_target(void);

UNLONG Select_a_tactical_square(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);

UNLONG Select_row_of_tactical_squares(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);

UNLONG Select_blink_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr);

void Select_mega_killer_target(UNSHORT Strength);

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

/* Temporary spell functions */
void Update_temporary_spells(void);

UNSHORT Get_member_temporary_spell_strength(UNSHORT Member_index,
 TEMP_SPELL_TYPE_T Temp_spell_index);

UNSHORT Get_member_temporary_spell_duration(UNSHORT Member_index,
 TEMP_SPELL_TYPE_T Temp_spell_index);

UNSHORT Get_light_spell_strength(void);

void Set_member_temporary_spell(UNSHORT Member_index,
 TEMP_SPELL_TYPE_T Temp_spell_index, UNSHORT Duration, UNSHORT Strength);

void Set_light_spell(UNSHORT Duration, UNSHORT Strength);

/* Spell data access functions */
UNSHORT Get_spell_targets(UNSHORT Class_nr, UNSHORT Spell_nr);
UNSHORT Get_spell_areas(UNSHORT Class_nr, UNSHORT Spell_nr);
UNSHORT Get_spell_required_SP(UNSHORT Class_nr, UNSHORT Spell_nr);
UNSHORT Get_spell_level(UNSHORT Class_nr, UNSHORT Spell_nr);

UNCHAR *Get_spell_name(UNSHORT Class_nr, UNSHORT Spell_nr);

/* Spell selection function */
BOOLEAN Select_spell(MEM_HANDLE Char_handle);

BOOLEAN Spell_class_exists(UNSHORT Class_nr);
BOOLEAN Spell_exists(UNSHORT Class_nr, UNSHORT Spell_nr);

#endif
