/************
 * NAME     : MAGIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 22-2-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MAGIC.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <GAMETEXT.H>
#include <MAGIC.H>
#include <MAP.H>
#include <EVELOGIC.H>
#include <COMBAT.H>
#include <COMACTS.H>
#include <COMSHOW.H>
#include <COMMAGIC.H>
#include <STATAREA.H>
#include <ITEMLIST.H>
#include <INPUT.H>
#include <TACTICAL.H>
#include <SPELLS.H>
#include <ITMLOGIC.H>
#include <XFTYPES.H>
#include <ITEMSEL.H>
#include <MEMBRSEL.H>
#include <COLOURS.H>

/* defines */

#define MAX_SPELLS_IN_WINDOW	(10)

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

/* structure definitions */

/* Spell data */
struct Spell_data {
	UNBYTE Area_bits;
	UNBYTE Point_cost;
	UNBYTE Level;
	UNBYTE Target_bits;
	UNBYTE _pad01;
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

/* prototypes */

void Update_temporary_spell(struct Temporary_spell_data *Temp_spell_data);
void Set_temporary_spell(struct Temporary_spell_data *Temp_spell_data,
 UNSHORT Duration, UNSHORT Strength);

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

/* global variables */

/* Current use magic data */
struct Use_magic_data Current_use_magic_data;

/* Spells per spell class */
UNSHORT Spells_per_class[SPELL_CLASSES_MAX] = {
	21,
	11,
	11,
	15,
	0,
	0, // 9,
	0
};

/* Spell eXception tables */
static struct Normal_XSpell Normal_spell_exceptions[] = {
	{
		0xFFFF, 0xFFFF,
		NULL
	}
};
static struct Combat_XSpell Combat_spell_exceptions[] = {
	{
		0, 14,
		Select_a_tactical_square
	},
	{
		0, 15,
		Select_a_tactical_square
	},
	{
		1, 4,
		Select_blink_target
	},
	{
		3, 7,
		Select_a_tactical_square
	},
	{
		3, 9,
		Select_a_tactical_square
	},
	{
		0xFFFF, 0xFFFF,
		NULL
	}
};

/* Spell handlers for each spell class */
static Spell_handler Class_0_spell_handlers[21] = {
	C0_Spell_1_handler,
	NULL, //C0_Spell_2_handler,
	C0_Spell_3_handler,
	C0_Spell_4_handler,
	C0_Spell_5_handler,
	C0_Spell_6_handler,
	C0_Spell_7_handler,
	C0_Spell_8_handler,
	C0_Spell_9_handler,
	C0_Spell_10_handler,
	C0_Spell_11_handler,
	C0_Spell_12_handler,
	C0_Spell_13_handler,
	C0_Spell_14_handler,
	C0_Spell_15_handler,
	C0_Spell_16_handler,
	C0_Spell_17_handler,
	C0_Spell_18_handler,
	C0_Spell_19_handler,
	C0_Spell_20_handler,
	C0_Spell_21_handler
};

static Spell_handler Class_1_spell_handlers[11] = {
	C1_Spell_1_handler,
	C1_Spell_2_handler,
	C1_Spell_3_handler,
	C1_Spell_4_handler,
	C1_Spell_5_handler,
	C1_Spell_6_handler,
	C1_Spell_7_handler,
	C1_Spell_8_handler,
	C1_Spell_9_handler,
	C1_Spell_10_handler,
	C1_Spell_11_handler
};

static Spell_handler Class_2_spell_handlers[11] = {
	C2_Spell_1_handler,
	C2_Spell_2_handler,
	C2_Spell_3_handler,
	C2_Spell_4_handler,
	C2_Spell_5_handler,
	C2_Spell_6_handler,
	C2_Spell_7_handler,
	C2_Spell_8_handler,
	C2_Spell_9_handler,
	C2_Spell_10_handler,
	C2_Spell_11_handler
};

static Spell_handler Class_3_spell_handlers[15] = {
	C3_Spell_1_handler,
	C3_Spell_2_handler,
	C3_Spell_3_handler,
	C3_Spell_4_handler,
	C3_Spell_5_handler,
	C3_Spell_6_handler,
	C3_Spell_7_handler,
	C3_Spell_8_handler,
	C3_Spell_9_handler,
	C3_Spell_10_handler,
	C3_Spell_11_handler,
	C3_Spell_12_handler,
	C3_Spell_13_handler,
	C3_Spell_14_handler,
	C3_Spell_15_handler
};

#if FALSE
static Spell_handler Class_5_spell_handlers[9] = {
	C5_Spell_1_handler,
	C5_Spell_2_handler,
	C5_Spell_3_handler,
	C5_Spell_4_handler,
	C5_Spell_5_handler,
	C5_Spell_6_handler,
	C5_Spell_7_handler,
	C5_Spell_8_handler,
	C5_Spell_9_handler
};
#endif

static Spell_handler *Spell_handler_lists[SPELL_CLASSES_MAX] = {
	Class_0_spell_handlers,
	Class_1_spell_handlers,
	Class_2_spell_handlers,
	Class_3_spell_handlers,
	NULL,
	NULL,
//	Class_5_spell_handlers,
	NULL
};

/* Spell window method list */
static struct Method Spell_window_methods[] = {
	{ INIT_METHOD, Init_Spell_window_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_Spell_window_object },
	{ UPDATE_METHOD, Update_help_line },
	{ RIGHT_METHOD, Close_Window_object },
	{ CLOSE_METHOD, Close_Window_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Spell window class description */
static struct Object_class Spell_window_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Spell_window_object),
	&Spell_window_methods[0]
};

/* Spell method list */
static struct Method Spell_methods[] = {
	{ INIT_METHOD, Init_Spell_object },
	{ DRAW_METHOD, Draw_Spell_object },
	{ FEEDBACK_METHOD, Feedback_Spell_object },
	{ HIGHLIGHT_METHOD, Highlight_Spell_object },
	{ HELP_METHOD, Help_Spell_object },
	{ LEFT_METHOD, Left_Spell_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

/* Spell class description */
static struct Object_class Spell_Class = {
	0, sizeof(struct Spell_object),
	&Spell_methods[0]
};

/* Magic data handles */
MEM_HANDLE Magic_event_set_handle;
MEM_HANDLE Magic_event_text_handle;

MEM_HANDLE Spell_data_handle;

/* Will be used when not zero */
UNSHORT Default_spell_strength = 0;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_magic_data
 * FUNCTION  : Initialize magic data.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 12:24
 * LAST      : 20.06.95 12:24
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_magic_data(void)
{
	/* Load magic event-set */
	Magic_event_set_handle = Load_subfile(EVENT_SET, MAGIC_EVENT_SET);
	if (!Magic_event_set_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load magic event-texts */
	Magic_event_text_handle = Load_subfile(EVENT_TEXT, MAGIC_EVENT_SET);
	if (!Magic_event_text_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load spell data */
	Spell_data_handle = Load_file(SPELL_DATA);
	if (!Spell_data_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_magic_data
 * FUNCTION  : Delete magic data.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 12:25
 * LAST      : 20.06.95 12:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_magic_data(void)
{
	/* Destroy magic event-set and -texts */
	MEM_Free_memory(Magic_event_set_handle);
	MEM_Free_memory(Magic_event_text_handle);

	/* Destroy spell data */
	MEM_Free_memory(Spell_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Cast_spell
 * FUNCTION  : A party member casts a spell (NOT in combat).
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 16:43
 * LAST      : 28.04.95 20:08
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 *             UNSHORT Source_item_slot_index - Index of item slot containing
 *              magical item (1...33) / 0xFFFF.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function can be used for any use of magic outside of
 *              the combat screen. If necessary, it will take care of spell
 *              selection and spell target selection.
 *             - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Cast_spell(UNSHORT Member_nr, UNSHORT Source_item_slot_index)
{
	static struct Event_action Cast_spell_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		CAST_SPELL_ACTION, 0, 0,
		Do_cast_spell, NULL, NULL
	};
	struct Character_data *Char;
	struct Item_data *Item_data;
	BOOLEAN Result;

	/* Clear use magic data */
	BASEMEM_FillMemByte((UNBYTE *) &Current_use_magic_data,
	 sizeof(struct Use_magic_data), 0);

	/* Cast from item ? */
	if (Source_item_slot_index == 0xFFFF)
	{
		/* No -> Select a spell */
		Result = Select_spell(Party_char_handles[Member_nr - 1]);

		/* Any selected ? */
		if (!Result)
		{
			/* No -> Exit */
			return;
		}
	}
	else
	{
		/* Yes -> Get casting character's data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Party_char_handles[Member_nr - 1]);

		/* Get item data */
		if (Source_item_slot_index <= ITEMS_ON_BODY)
		{
			Item_data = Get_item_data(&(Char->Body_items[Source_item_slot_index
			 - 1]));
		}
		else
		{
			Item_data = Get_item_data(&(Char->Backpack_items[Source_item_slot_index -
			 ITEMS_ON_BODY - 1]));
		}

		/* Get spell */
		Current_use_magic_data.Class_nr = (UNSHORT) Item_data->Spell_class;
		Current_use_magic_data.Spell_nr = (UNSHORT) Item_data->Spell_nr;

		Free_item_data();
		MEM_Free_pointer(Party_char_handles[Member_nr - 1]);
	}

	/* Write current use magic data */
	Current_use_magic_data.Source_item_slot_index	= Source_item_slot_index;
	Current_use_magic_data.Casting_member_index		= Member_nr;
	Current_use_magic_data.Casting_handle				= Party_char_handles[Member_nr - 1];

	/* Get spell target */
	Result = Enter_spell_target();

	/* Target selected ? */
	if (Result)
	{
		/* Yes -> Insert data in event action structure */
		Cast_spell_action.Actor_index		= Member_nr;
		Cast_spell_action.Action_value	= Current_use_magic_data.Class_nr;
		Cast_spell_action.Action_extra	= Current_use_magic_data.Spell_nr;

		/* Perform event action */
		Perform_action(&Cast_spell_action);
	}
}

BOOLEAN
Do_cast_spell(struct Event_action *Action)
{
	Spell_handler *List, Handler;
	UNSHORT Strength;

	/* Get address of spell handler list for the selected class */
	List = Spell_handler_lists[Action->Action_value];
	if (!List)
		return FALSE;

	/* Get address of spell handler for this spell */
	Handler = List[Action->Action_extra - 1];
	if (!Handler)
		return FALSE;

	/* Get spell strength */
	Strength = Handle_spell();

	/* Any ? */
	if (Strength)
	{
		/* Yes -> Execute spell handler */
		(Handler)(Strength);
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Cast_combat_spell
 * FUNCTION  : A party member casts a spell in combat.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.05.95 15:07
 * LAST      : 04.05.95 15:07
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Source_item_slot_index - Index of item slot containing
 *              magical item (1...33) / 0xFFFF.
 * RESULT    : BOOLEAN : Cast preparations were successful.
 * BUGS      : No known.
 * NOTES     : - This function can be used for any use of magic by party
 *              members inside of the combat screen. If necessary, it will
 *              take care of spell selection and spell target selection.
 *             - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Cast_combat_spell(struct Combat_participant *Part,
 UNSHORT Source_item_slot_index)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	BOOLEAN Result;

	/* Clear use magic data */
	BASEMEM_FillMemByte((UNBYTE *) &Current_use_magic_data,
	 sizeof(struct Use_magic_data), 0);

	/* Cast from item ? */
	if (Source_item_slot_index == 0xFFFF)
	{
		/* No -> Select a spell */
		Result = Select_spell(Part->Char_handle);

		/* Any selected ? */
		if (!Result)
		{
			/* No -> Exit */
			return FALSE;
		}
	}
	else
	{
		/* Yes -> Get casting character's data */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		/* Get item data */
		if (Source_item_slot_index <= ITEMS_ON_BODY)
		{
			Item_data = Get_item_data(&(Char->Body_items[Source_item_slot_index
			 - 1]));
		}
		else
		{
			Item_data = Get_item_data(&(Char->Backpack_items[Source_item_slot_index -
			 ITEMS_ON_BODY - 1]));
		}

		/* Get spell */
		Current_use_magic_data.Class_nr = (UNSHORT) Item_data->Spell_class;
		Current_use_magic_data.Spell_nr = (UNSHORT) Item_data->Spell_nr;

		Free_item_data();
		MEM_Free_pointer(Part->Char_handle);
	}

	/* Write current use magic data */
	Current_use_magic_data.Source_item_slot_index = Source_item_slot_index;
	Current_use_magic_data.Casting_member_index = Part->Number;
	Current_use_magic_data.Casting_handle = Part->Char_handle;
	Current_use_magic_data.Casting_participant = Part;

	/* Get spell target */
	Result = Enter_spell_target();

	/* Any target selected ? */
	if (Result)
	{
		/* Yes -> Copy use magic data */
		memcpy((UNBYTE *) &(Part->Target.Magic_target_data),
		 (UNBYTE *) &Current_use_magic_data, sizeof(struct Use_magic_data));
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_cast_combat_spell
 * FUNCTION  : A combat participant casts a spell in combat.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.05.95 16:59
 * LAST      : 05.07.95 10:55
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_cast_combat_spell(struct Combat_participant *Part)
{
	Spell_handler *List, Handler;
	UNSHORT Strength;

	/* Copy use magic data */
	memcpy((UNBYTE *) &Current_use_magic_data,
	 (UNBYTE *) &(Part->Target.Magic_target_data),
	 sizeof(struct Use_magic_data));

	/* Get address of spell handler list for the selected class */
	List = Spell_handler_lists[Current_use_magic_data.Class_nr];
	if (!List)
		return;

	/* Get address of spell handler for this spell */
	Handler = List[Current_use_magic_data.Spell_nr - 1];
	if (!Handler)
		return;

	/* Default spell strength ? */
	if (Default_spell_strength)
	{
		/* Yes -> Use it */
		Strength = Default_spell_strength;
	}
	else
	{
		/* No -> Get spell strength */
		Strength = Handle_spell();
	}

	/* Any ? */
	if (Strength)
	{
		/* Yes -> Execute spell handler */
		(Handler)(Strength);
	}

	/* Clear action */
	Part->Current_action = NO_COMACT;
}

#if FALSE
	static struct Event_action Cast_spell_combat_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		PART_ATTACKS_MAGIC_ACTION, 0, 0,
		Do_cast_spell_combat, NULL, NULL
	};

	/* Insert data in event action structure */
	Cast_spell_combat_action.Actor_type		= Part->Type;
	Cast_spell_combat_action.Actor_index	= Part->Number;
	Cast_spell_combat_action.Action_value	= Part->Target.Magic_target_data.Class_nr;
	Cast_spell_combat_action.Action_extra	= Part->Target.Magic_target_data.Spell_nr;
	Cast_spell_combat_action.Action_data	= (void *) Part;

	/* Perform event action */
	Perform_action(&Cast_spell_combat_action);
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_spell_target
 * FUNCTION  : Enter the target for the currently selected target.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.04.95 13:34
 * LAST      : 02.09.95 14:48
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Target entered / aborted.
 * BUGS      : No known.
 * NOTES     : - This function assumes the selected spell information has
 *              already been set in Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Enter_spell_target(void)
{
	BOOLEAN Result = TRUE;
	UNLONG Combat_mask = 0;
	UNSHORT Spell_targets;
	UNSHORT Member_index;
	UNSHORT Square_index;
	UNSHORT Party_mask = 0;
	UNSHORT Target_item_slot_index;
	UNSHORT i;

	/* Get target data */
	Spell_targets = Get_spell_targets(Current_use_magic_data.Class_nr,
	 Current_use_magic_data.Spell_nr);

	/* Reset spell target data*/
	Current_use_magic_data.Target_item_slot_index = 0xFFFF;

	/* In combat ? */
	if (In_Combat)
	{
		/* Yes -> Act depending on target type */
		switch (Spell_targets)
		{
			case ONE_FRIEND_TARGET:
			{
				/* Get party member targets */
				Combat_mask = Get_party_targets();

				/* Any ? */
				if (Combat_mask)
				{
					/* Yes -> Select one */
					Square_index = Select_tactical_square(87,
					 Show_potential_attack_targets, Combat_mask, 0);

					/* Any square selected ? */
					if (Square_index != 0xFFFF)
					{
						/* Yes -> Build target mask */
						Combat_mask = (1 << Square_index);

						/* Set target mode */
						Current_use_magic_data.Combat_target_mode = FRIEND_TARGMODE;
					}
					else
					{
						/* No -> Cancelled */
						Combat_mask = 0;

						Result = FALSE;
					}
				}
				else
				{
					/* No */
					Result = FALSE;
				}

				break;
			}
			case ROW_FRIENDS_TARGET:
			{
				/* Select a row of party members */
				Square_index = Select_tactical_square(89,
				 Show_potential_row_targets, COMBAT_PARTY_MASK, 0);

				/* Any square selected ? */
				if (Square_index != 0xFFFF)
				{
					/* Yes -> Build target mask */
					Square_index -= (Square_index % NR_TACTICAL_COLUMNS);
					for (i=0;i<NR_TACTICAL_COLUMNS;i++)
					{
						Combat_mask |= (1 << (Square_index + i));
					}

					/* Set target mode */
					Current_use_magic_data.Combat_target_mode = FRIEND_TARGMODE;
				}
				else
				{
					/* No */
					Result = FALSE;
				}

				break;
			}
			case ALL_FRIENDS_TARGET:
			{
				/* Select all party members */
				Combat_mask = COMBAT_PARTY_MASK;

				/* Set target mode */
				Current_use_magic_data.Combat_target_mode = FRIEND_TARGMODE;

				break;
			}
			case ONE_ENEMY_TARGET:
			{
				/* Get monster targets */
				Combat_mask = Get_monster_targets();

				/* Any ? */
				if (Combat_mask)
				{
					/* Yes -> Select one */
					Square_index = Select_tactical_square(88,
					 Show_potential_attack_targets, Combat_mask, 0);

					/* Any square selected ? */
					if (Square_index != 0xFFFF)
					{
						/* Yes -> Build target mask */
						Combat_mask = (1 << Square_index);

						/* Set target mode */
						Current_use_magic_data.Combat_target_mode = ENEMY_TARGMODE;
					}
					else
					{
						/* No -> Cancelled */
						Combat_mask = 0;

						Result = FALSE;
					}
				}
				else
				{
					/* No */
					Result = FALSE;
				}

				break;
			}
			case ROW_ENEMIES_TARGET:
			{
				/* Select a row of monsters */
				Square_index = Select_tactical_square(89,
				 Show_potential_row_targets, COMBAT_MONSTER_MASK, 0);

				/* Any square selected ? */
				if (Square_index != 0xFFFF)
				{
					/* Yes -> Build target mask */
					Square_index -= (Square_index % NR_TACTICAL_COLUMNS);
					for (i=0;i<NR_TACTICAL_COLUMNS;i++)
					{
						Combat_mask |= (1 << (Square_index + i));
					}

					/* Set target mode */
					Current_use_magic_data.Combat_target_mode = ENEMY_TARGMODE;
				}
				else
				{
					/* No */
					Result = FALSE;
				}

				break;
			}
			case ALL_ENEMIES_TARGET:
			{
				/* Select all monsters */
				Combat_mask = COMBAT_MONSTER_MASK;

				/* Set target mode */
				Current_use_magic_data.Combat_target_mode = ENEMY_TARGMODE;

				break;
			}
			case ITEM_TARGET:
			{
				/* Select a party member */
				Member_index = Select_party_member(System_text_ptrs[90], NULL, 0);

				/* Any member selected ? */
				if (Member_index && (Member_index != 0xFFFF))
				{
					/* Yes -> Select item */
					Target_item_slot_index =
					 Select_character_item(Party_char_handles[Member_index - 1],
					 System_text_ptrs[91], NULL);

					/* Any item selected ? */
					if (Target_item_slot_index != 0xFFFF)
					{
						/* Yes -> Build target mask */
						Square_index = (Party_parts[Member_index - 1].Tactical_Y *
						 NR_TACTICAL_COLUMNS) +
						 Party_parts[Member_index - 1].Tactical_X;

						Combat_mask = (1 << Square_index);

						/* Set target mode */
						Current_use_magic_data.Combat_target_mode = FRIEND_TARGMODE;

						/* Store target item slot index */
						Current_use_magic_data.Target_item_slot_index =
						 Target_item_slot_index;
					}
					else
					{
						/* No */
						Result = FALSE;
					}
				}
				else
				{
					/* No */
					Result = FALSE;
				}
				break;
			}
			case SPECIAL_TARGET:
			{
				/* Search spell exception table */
				Result = FALSE;
				i = 0;
				for (;;)
				{
					/* End of spell exception table ? */
					if ((Combat_spell_exceptions[i].Class_nr == 0xFFFF) &&
					 (Combat_spell_exceptions[i].Spell_nr == 0xFFFF))
					{
						/* Yes -> Spell wasn't found */
						break;
					}

					/* Is this the spell ? */
					if ((Combat_spell_exceptions[i].Class_nr ==
					 Current_use_magic_data.Class_nr) &&
					 (Combat_spell_exceptions[i].Spell_nr ==
					 Current_use_magic_data.Spell_nr))
					{
						/* Yes -> Call spell exception handler */
						Combat_mask = (Combat_spell_exceptions[i].Handler)
						 (Current_use_magic_data.Class_nr,
						 Current_use_magic_data.Spell_nr,
						 &Current_use_magic_data);

						/* Any target selected ? */
						if (Combat_mask)
						{
							/* No */
							Result = TRUE;
						}

						break;
					}
					else
					{
						/* No -> Next spell exception */
						i++;
					}
				}
				break;
			}
		}
	}
	else
	{
		/* No -> Act depending on target type */
		switch (Spell_targets)
		{
			/* One party member */
			case ONE_FRIEND_TARGET:
			{
				/* Select a party member */
				Member_index = Select_party_member(System_text_ptrs[87], NULL, 0);

				/* Anyone selected ? */
				if (Member_index && (Member_index != 0xFFFF))
				{
					/* Yes -> Build target mask */
					Party_mask = (1 << Member_index);
				}
				else
				{
					/* No */
					Result = FALSE;
				}
				break;
			}
			/* All party members */
			case ALL_FRIENDS_TARGET:
			{
				/* Select all party members */
				Party_mask = 0x007e;
				break;
			}
			/* Item */
			case ITEM_TARGET:
			{
				/* Select a party member */
				Member_index = Select_party_member(System_text_ptrs[90], NULL, 0);

				/* Any member selected ? */
				if (Member_index && (Member_index != 0xFFFF))
				{
					/* Yes -> Select item */
					Target_item_slot_index =
					 Select_character_item(Party_char_handles[Member_index - 1],
					 System_text_ptrs[91], NULL);

					/* Any item selected ? */
					if (Target_item_slot_index != 0xFFFF)
					{
						/* Yes -> Build target mask */
						Party_mask = (1 << Member_index);

						/* Store target item slot index */
						Current_use_magic_data.Target_item_slot_index =
						 Target_item_slot_index;
					}
					else
					{
						/* No */
						Result = FALSE;
					}
				}
				else
				{
					/* No */
					Result = FALSE;
				}

				break;
			}
			case SPECIAL_TARGET:
			{
				/* Search spell exception table */
				Result = FALSE;
				i = 0;
				for (;;)
				{
					/* End of spell exception table ? */
					if ((Normal_spell_exceptions[i].Class_nr == 0xFFFF) &&
					 (Normal_spell_exceptions[i].Spell_nr == 0xFFFF))
					{
						/* Yes -> Spell wasn't found */
						break;
					}

					/* Is this the spell ? */
					if ((Normal_spell_exceptions[i].Class_nr ==
					 Current_use_magic_data.Class_nr) &&
					 (Normal_spell_exceptions[i].Spell_nr ==
					 Current_use_magic_data.Spell_nr))
					{
						/* Yes -> Call spell exception handler */
						Party_mask = (UNSHORT)(Normal_spell_exceptions[i].Handler)
						 (Current_use_magic_data.Class_nr,
						 Current_use_magic_data.Spell_nr,
						 &Current_use_magic_data);

						/* Any target selected ? */
						if (Party_mask)
						{
							/* Yes */
							Result = TRUE;
						}
						break;
					}
					else
					{
						/* No -> Next spell exception */
						i++;
					}
				}
				break;
			}
		}
	}

	/* Write target data */
	Current_use_magic_data.Party_target_mask	= Party_mask;
	Current_use_magic_data.Combat_target_mask	= Combat_mask;

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_a_tactical_square
 * FUNCTION  : Select a (possibly empty) tactical square (combat spell target
 *              selection exception handler).
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 15:32
 * LAST      : 03.06.95 14:47
 * INPUTS    : UNSHORT Class_nr - Spell class index (0...SPELL_CLASSES_MAX).
 *             UNSHORT Spell_nr - Spell index (1...SPELLS_PER_CLASS).
 *             struct Use_magic_data *Use_magic_data_ptr - Pointer to use
 *              magic data.
 * RESULT    : UNLONG : Selected tactical squares mask / 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Select_a_tactical_square(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr)
{
	UNLONG Combat_mask = 0;
	UNSHORT Square_index;

	/* Select a tactical square */
	Square_index = Select_tactical_square(133,
	 Show_potential_attack_targets, COMBAT_MONSTER_MASK, 0);

	/* Any square selected ? */
	if (Square_index != 0xFFFF)
	{
		/* Yes -> Build target mask */
		Combat_mask = (1 << Square_index);

		/* Set target mode */
		Use_magic_data_ptr->Combat_target_mode = SQUARE_TARGMODE;
	}

	return Combat_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_blink_target
 * FUNCTION  : Select targets for the blink spell (combat spell target
 *              selection exception handler).
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.95 17:35
 * LAST      : 03.06.95 17:35
 * INPUTS    : UNSHORT Class_nr - Spell class index (0...SPELL_CLASSES_MAX).
 *             UNSHORT Spell_nr - Spell index (1...SPELLS_PER_CLASS).
 *             struct Use_magic_data *Use_magic_data_ptr - Pointer to use
 *              magic data.
 * RESULT    : UNLONG : Selected tactical squares mask / 0.
 * BUGS      : No known.
 * NOTES     : - The tactical square mask indicates who will be blinked,
 *              the blink's destination will be stored directly in the
 *              use magic data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Select_blink_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 struct Use_magic_data *Use_magic_data_ptr)
{
	struct Combat_participant *Target_part;
	UNLONG Combat_mask = 0;
	UNLONG Destination_mask;
	UNSHORT Target_square_index;
	UNSHORT Destination_square_index;

	/* Select a combat participant */
	Target_square_index = Select_tactical_square(428,
	 Show_potential_attack_targets, COMBAT_PARTY_MASK | COMBAT_MONSTER_MASK, 0);

	/* Any square selected ? */
	if (Target_square_index != 0xFFFF)
	{
		/* Yes -> Determine target participant */
		Target_part = Combat_matrix[Target_square_index / NR_TACTICAL_COLUMNS][Target_square_index % NR_TACTICAL_COLUMNS].Part;

		/* Is monster ? */
		if (Target_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Build destination mask */
			Destination_mask = COMBAT_MONSTER_MASK & ~(Get_monster_targets());
		}
		else
		{
			/* No -> Build destination mask */
			Destination_mask = COMBAT_PARTY_MASK & ~(Get_party_targets() |
			 Get_occupied_move_targets(Target_part));
		}

		/* Select destination */
		Destination_square_index = Select_tactical_square(429,
		 Show_potential_attack_targets, Destination_mask, 0);

		/* Any destination selected ? */
		if (Destination_square_index != 0xFFFF)
		{
			/* Yes -> Build target mask */
			Combat_mask = (1 << Target_square_index);

			/* Set target mode */
			Use_magic_data_ptr->Combat_target_mode = PART_TARGMODE;

			/* Store destination square index */
			Use_magic_data_ptr->Extra_target_data = Destination_square_index;
		}
	}

	return Combat_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_all_magic_party_targets
 * FUNCTION  : Execute a spell on all targeted party members.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 12:12
 * LAST      : 02.05.95 11:25
 * INPUTS    : UNSHORT Strength - Spell strength.
 *             Spell_per_target_handler Handler - Pointer to spell function.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_all_magic_party_targets(UNSHORT Strength,
 Spell_per_target_handler Handler)
{
	struct Combat_participant *Part;
	UNLONG Combat_target_mask;
	UNSHORT Party_target_mask;
	UNSHORT i, j;

	/* In combat ? */
	if (In_Combat)
	{
		/* Yes -> Get combat target mask */
		Combat_target_mask = Current_use_magic_data.Combat_target_mask;

		/* Check bottom two rows of combat matrix */
		for (i=3;i<NR_TACTICAL_ROWS;i++)
		{
			for (j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Anyone there ? */
				if (Part)
				{
					/* Yes -> Party member / targeted ? */
					if ((Part->Type == PARTY_PART_TYPE) && (Combat_target_mask &
					 (1 << (i * NR_TACTICAL_COLUMNS + j))))
					{
						/* Yes -> Is target of spell */
						Subject_char_handle = Party_char_handles[i];

						/* Do spell */
						(Handler)(Part->Number, Strength);
					}
				}
			}
		}
	}
	else
	{
		/* No -> Get party target mask */
		Party_target_mask = Current_use_magic_data.Party_target_mask;

		/* Check party */
		for (i=0;i<6;i++)
		{
			/* Anyone there / targeted ? */
			if ((PARTY_DATA.Member_nrs[i]) &&
			 (Party_target_mask & (1 << (i + 1))))
			{
				/* Yes -> Is target of spell */
				Subject_char_handle = Party_char_handles[i];

				/* Do spell */
				(Handler)(i + 1, Strength);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_all_magic_combat_part_targets
 * FUNCTION  : Execute a spell on all targeted combat participants.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 16:49
 * LAST      : 07.06.95 10:21
 * INPUTS    : UNSHORT Strength - Spell strength.
 *             Spell_per_combat_target_handler Handler - Pointer to spell
 *              function.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_all_magic_combat_targets(UNSHORT Strength,
 Spell_per_combat_target_handler Handler)
{
	struct Combat_participant *Part;
	UNLONG Combat_target_mask;
	UNSHORT i, j;

	/* Get combat target mask */
	Combat_target_mask = Current_use_magic_data.Combat_target_mask;

	/* Check combat matrix */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Is this square targeted ? */
			if (Combat_target_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Act depending on current target mode */
				switch (Current_use_magic_data.Combat_target_mode)
				{
					case PART_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Is target of spell */
							Subject_char_handle = Part->Char_handle;

							/* Do spell */
							(Handler)(Part, j, i, Strength);
						}
						break;
					}
					case FRIEND_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Is this participant a friend ? */
							if (Part->Type ==
							 Current_use_magic_data.Casting_participant->Type)
							{
								/* Yes -> Is target of spell */
								Subject_char_handle = Part->Char_handle;

								/* Do spell */
								(Handler)(Part, j, i, Strength);
							}
						}
						break;
					}
					case ENEMY_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Is this participant an enemy ? */
							if (Part->Type !=
							 Current_use_magic_data.Casting_participant->Type)
							{
								/* Yes -> Is target of spell */
								Subject_char_handle = Part->Char_handle;

								/* Do spell */
								(Handler)(Part, j, i, Strength);
							}
						}
						break;
					}
					case SQUARE_TARGMODE:
					{
						/* Anyone there ? */
						if (Part)
						{
							/* Yes -> Is target of spell */
							Subject_char_handle = Part->Char_handle;
						}

						/* Do spell */
						(Handler)(Part, j, i, Strength);

						break;
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_magic_combat_part_target
 * FUNCTION  : Get the targeted combat participant data.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.95 11:38
 * LAST      : 06.06.95 11:38
 * INPUTS    : None.
 * RESULT    : struct Combat_participant * : Pointer to victim participant
 *              data.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Combat_participant *
Get_magic_combat_part_target(void)
{
	struct Combat_participant *Part;
	struct Combat_participant *Victim_part = NULL;
	UNLONG Combat_target_mask;
	UNSHORT i, j;

	/* Get combat target mask */
	Combat_target_mask = Current_use_magic_data.Combat_target_mask;

	/* Check combat matrix */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Is this square targeted ? */
			if (Combat_target_mask & (1 << (i * NR_TACTICAL_COLUMNS + j)))
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Anyone there ? */
				if (Part)
				{
					/* Yes -> Is target of spell */
					Subject_char_handle = Party_char_handles[i];

					Victim_part = Part;
				}
				break;
			}
		}
	}

	return Victim_part;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_spell
 * FUNCTION  : Cast spell logic.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.03.95 13:54
 * LAST      : 30.03.95 13:54
 * INPUTS    : None.
 * RESULT    : UNSHORT : Spell strength (0...100).
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Handle_spell(void)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	SISHORT SP;
	UNSHORT Output_strength;
	UNSHORT Required_SP;
	UNSHORT Slot_index;

	/* Is the spell cast from an object ? */
	if (Current_use_magic_data.Source_item_slot_index != 0xFFFF)
	{
		/* Yes -> Get item slot index */
		Slot_index = Current_use_magic_data.Source_item_slot_index;

		/* Get casting character's data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Current_use_magic_data.Casting_handle);

		/* Get item data */
		if (Slot_index < ITEMS_ON_BODY)
		{
			Item_data = Get_item_data(&(Char->Body_items[Slot_index]));
		}
		else
		{
			Item_data = Get_item_data(&(Char->Backpack_items[Slot_index -
			 ITEMS_ON_BODY]));
		}

		/* Is this a magical item ? */
		if (Item_data->Type == MAGIC_IT)
		{
			/* Yes -> Get spell strength */
			Output_strength = (UNSHORT) Item_data->Misc[0];
		}
		else
		{
			/* No -> Use default spell strength */
			Output_strength = DEFAULT_ITEM_SPELL_STRENGTH;
		}

		Free_item_data();
		MEM_Free_pointer(Current_use_magic_data.Casting_handle);

		/* Remove magic item */
		Remove_used_magic_item(Current_use_magic_data.Casting_handle,
		 Slot_index);
	}
	else
	{
		/* No -> Get SP required for this spell */
		Required_SP = Get_spell_required_SP(Current_use_magic_data.Class_nr,
		 Current_use_magic_data.Spell_nr);

		/* Get casting character's data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Current_use_magic_data.Casting_handle);

		/* Remove required SP from casting character */
		SP = max(Get_SP(Current_use_magic_data.Casting_handle) -
		 Required_SP, 0);
		Set_SP(Current_use_magic_data.Casting_handle, SP);

		/* Get spell strength */
		Output_strength = (UNSHORT)
		 ((Get_spell_strength(Current_use_magic_data.Casting_handle,
		 Current_use_magic_data.Class_nr, Current_use_magic_data.Spell_nr)
		 + 50) / 100);

		MEM_Free_pointer(Current_use_magic_data.Casting_handle);
	}

	return Output_strength;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_used_magic_item
 * FUNCTION  : Remove used magic item.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.03.95 13:58
 * LAST      : 07.04.95 14:17
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Slot_index - Slot index (1...33).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_used_magic_item(MEM_HANDLE Char_handle, UNSHORT Slot_index)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get packet address */
	if (Slot_index <= ITEMS_ON_BODY)
	{
		Packet = &(Char->Body_items[Slot_index - 1]);
	}
	else
	{
		Packet = &(Char->Backpack_items[Slot_index - ITEMS_ON_BODY - 1]);
	}

	/* Anything in packet ? */
	if (!Packet_empty(Packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Packet);

		/* Is this a multiple item ? */
		if (Item_data->Flags & MULTIPLE)
		{
			/* Yes -> Remove it */
			Remove_item(Char_handle, Slot_index, 1);

			/* Show removal of item */

		}
		else
		{
			/* No -> Does it have an infinite number of charges ? */
			if (Packet->Charges != 255)
			{
				/* No -> Subtract one charge */
				Packet->Charges--;
			}

			/* Show sparkling item */

			/* Try to break the item */

		}
	}
	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_magical_defense
 * FUNCTION  : Handle the defense against a magical spell in combat.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 16:15
 * LAST      : 30.08.95 18:12
 * INPUTS    : struct Combat_participant *Defender_part - Pointer to
 *              defender's participant data.
 *             UNSHORT Incoming_strength - Strength of incoming spell
 *              (1...100).
 * RESULT    : UNSHORT : Remaining strength of spell (1...100).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Handle_magical_defense(struct Combat_participant *Defender_part,
 UNSHORT Incoming_strength)
{
	UNSHORT Magic_resistance;
	UNSHORT Remaining_strength;

	/* Get magic resistance attribute of defender */
	Magic_resistance = Get_attribute(Defender_part->Char_handle,
	 MAGIC_RESISTANCE);

	/* Is party member ? */
	if (Defender_part->Type == PARTY_PART_TYPE)
	{
		/* Yes -> Add temporary spell effect */
		Magic_resistance += (Get_member_temporary_spell_strength(Defender_part->Number,
		 ANTI_MAGIC_TEMP_SPELL) * Magic_resistance) / 100;
	}

	/* Implement magic resistance */
	Remaining_strength = max((SISHORT) (Incoming_strength -
	 Magic_resistance), 0);

	/* Deflected completely ? */
	if (!Remaining_strength)
	{
		/* "Deflected!" */

		/* Show deflecting */
		Combat_show(Defender_part, SHOW_DEFLECT, NULL);
	}

	return Remaining_strength;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Learn_spell
 * FUNCTION  : Learn a new spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 16:31
 * LAST      : 31.03.95 16:31
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class index (0...SPELL_CLASSES_MAX).
 *             UNSHORT Spell_nr - Spell index (1...SPELLS_PER_CLASS).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Learn_spell(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Character_data *Char;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

			/* Is the spell already known ? */
			if (!(Spell_known(Char_handle, Class_nr, Spell_nr)))
			{
				/* No -> Indicate the spell is known */
				Char->xKnown_spells[Class_nr] |= (1 << Spell_nr);

				/* Initialize the spell capability */
				Set_spell_strength(Char_handle, Class_nr, Spell_nr,
				 Get_attribute(Char_handle, MAGIC_TALENT) * 50);
			}

			MEM_Free_pointer(Char_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_required_LP
 * FUNCTION  : Calculate the life-points (!) required to cast a spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 14:54
 * LAST      : 03.09.95 17:28
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNSHORT : Required LP / 0xFFFF = too much.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Calculate_required_LP(MEM_HANDLE Char_handle, UNSHORT Class_nr,
 UNSHORT Spell_nr)
{
	UNSHORT LP;
	UNSHORT SP;
	UNSHORT Required_LP = 0;
	UNSHORT Required_SP;
	UNSHORT Stamina;
	SISHORT Factor;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Cheat mode ? */
			if (!Cheat_mode)
			{
				/* No -> Get data */
				LP = Get_LP(Char_handle);
				SP = Get_SP(Char_handle);
				Required_SP = Get_spell_required_SP(Class_nr, Spell_nr);
				Stamina = Get_attribute(Char_handle, STAMINA);

				/* Any LP required ? */
				if (Required_SP > SP)
				{
					/* Yes -> How many ? */
					Factor = max((((100 - Stamina) * 3) / 2) + 50, 1);
					Required_LP = ((Required_SP - SP) * Factor) / 100;

					/* Does this character have enough LP ? */
					if (Required_LP > LP)
					{
						/* No -> Indicate */
						Required_LP = 0xFFFF;
					}
				}
			}
		}
	}

	return Required_LP;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_area
 * FUNCTION  : Return the current spell area type.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 12:18
 * LAST      : 03.04.95 14:42
 * INPUTS    : None.
 * RESULT    : UNSHORT : Current spell area type.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_spell_area(void)
{
	struct Map_data *Map_ptr;
	UNSHORT Current_spell_area = 0;
	UNSHORT Flags;

	/* In combat ? */
	if (In_Combat)
	{
		/* Yes -> Set the combat spell area type */
		Current_spell_area = COMBAT_AREA;
	}
	else
	{
		/* No -> Get current map flags */
		Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
		Flags = Map_ptr->Flags;
		MEM_Free_pointer(Map_handle);

		/* Is this a planet or a spaceship map ? */
		if (Flags & PLANET_SPACESHIP)
		{
			/* Spaceship -> Set the spaceship spell area type */
			Current_spell_area = SPACESHIP_AREA;
		}
		else
		{
			/* Planet -> Act depending on map type */
			switch (Current_map_type)
			{
				/* City map */
				case CITY_MAP:
				{
					Current_spell_area = CITY_AREA;
					break;
				}
				/* Dungeon map */
				case DUNGEON_MAP:
				{
					Current_spell_area = DUNGEON_AREA;
					break;
				}
				/* Wilderness map */
				case WILDERNESS_MAP:
				{
					Current_spell_area = WILDERNESS_AREA;
					break;
				}
				/* Interior map */
				case INTERIOR_MAP:
				{
					Current_spell_area = INTERIOR_AREA;
					break;
				}
			}
		}
	}

	return Current_spell_area;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_temporary_spells
 * FUNCTION  : Update temporary spells.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 17:26
 * LAST      : 30.08.95 17:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_temporary_spells(void)
{
	UNSHORT i, j;

	/* Update light temporary spell */
	Update_temporary_spell(&(PARTY_DATA.Light_spell));

	/* Handle temporary spells for each potential party member */
	for (i=0;i<PARTY_CHARS_MAX;i++)
	{
		/* Update temporary spells */
		for (j=0;j<(UNSHORT) MAX_TEMP_SPELLS;j++)
		{
			Update_temporary_spell(&(PARTY_DATA.Temporary_spells[i][j]));
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_temporary_spell
 * FUNCTION  : Update a temporary spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 17:52
 * LAST      : 30.08.95 17:52
 * INPUTS    : struct Temporary_spell_data *Temp_spell_data - Pointer to
 *              temporary spell data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_temporary_spell(struct Temporary_spell_data *Temp_spell_data)
{
	/* Is the temporary spell on ? */
	if (Temp_spell_data->Duration)
	{
		/* Yes -> Count down */
		Temp_spell_data->Duration--;

		/* Is it out now ? */
		if (!Temp_spell_data->Duration)
		{
			/* Yes -> Clear strength (just to be sure) */
			Temp_spell_data->Strength = 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_member_temporary_spell_strength
 * FUNCTION  : Get the strength of a temporary spell for a party member.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 18:01
 * LAST      : 30.08.95 18:02
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 *             TEMP_SPELL_TYPE_T Temp_spell_index - Temporary spell index.
 * RESULT    : UNSHORT : Strength.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_member_temporary_spell_strength(UNSHORT Member_index,
 TEMP_SPELL_TYPE_T Temp_spell_index)
{
	UNSHORT Char_nr;
	UNSHORT Strength = 0;

	/* Get character index */
	Char_nr = PARTY_DATA.Member_nrs[Member_index - 1];

	/* Is the temporary spell on ? */
	if (PARTY_DATA.Temporary_spells[Char_nr - 1][Temp_spell_index].Duration)
	{
		/* Yes -> Get strength */
		Strength = PARTY_DATA.Temporary_spells[Char_nr - 1][Temp_spell_index].Strength;
	}

	return Strength;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_member_temporary_spell_duration
 * FUNCTION  : Get the duration of a temporary spell for a party member.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 15:26
 * LAST      : 10.09.95 15:26
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 *             TEMP_SPELL_TYPE_T Temp_spell_index - Temporary spell index.
 * RESULT    : UNSHORT : Duration.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_member_temporary_spell_duration(UNSHORT Member_index,
 TEMP_SPELL_TYPE_T Temp_spell_index)
{
	UNSHORT Char_nr;
	UNSHORT Duration;

	/* Get character index */
	Char_nr = PARTY_DATA.Member_nrs[Member_index - 1];

	/* Get duration */
	Duration = PARTY_DATA.Temporary_spells[Char_nr - 1][Temp_spell_index].Strength;

	return Duration;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_light_spell_strength
 * FUNCTION  : Get the strength of the light temporary spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 18:13
 * LAST      : 30.08.95 18:13
 * INPUTS    : None.
 * RESULT    : UNSHORT : Strength.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_light_spell_strength(void)
{
	UNSHORT Strength = 0;

	/* Is the light temporary spell on ? */
	if (PARTY_DATA.Light_spell.Duration)
	{
		/* Yes -> Get strength */
		Strength = PARTY_DATA.Light_spell.Strength;
	}

	return Strength;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_member_temporary_spell
 * FUNCTION  : Set the duration and strength of a temporary spell for a party
 *              member.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 12:36
 * LAST      : 31.08.95 12:36
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 *             TEMP_SPELL_TYPE_T Temp_spell_index - Temporary spell index.
 *             UNSHORT Duration - Duration.
 *             UNSHORT Strength - Strength.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_member_temporary_spell(UNSHORT Member_index,
 TEMP_SPELL_TYPE_T Temp_spell_index, UNSHORT Duration, UNSHORT Strength)
{
	UNSHORT Char_nr;

	/* Get character index */
	Char_nr = PARTY_DATA.Member_nrs[Member_index - 1];

	/* Is the temporary spell on ? */
	if (PARTY_DATA.Temporary_spells[Char_nr - 1][Temp_spell_index].Duration)
	{
		/* Yes -> Set duration and strength */
		Set_temporary_spell(&PARTY_DATA.Temporary_spells[Char_nr - 1][Temp_spell_index],
		 Duration, Strength);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_light_spell
 * FUNCTION  : Set the duration and strength of the light temporary spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 12:30
 * LAST      : 31.08.95 12:30
 * INPUTS    : UNSHORT Duration - Duration.
 *             UNSHORT Strength - Strength.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_light_spell(UNSHORT Duration, UNSHORT Strength)
{
	Set_temporary_spell(&PARTY_DATA.Light_spell, Duration, Strength);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_temporary_spell
 * FUNCTION  : Set the duration and strength of a temporary spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 12:34
 * LAST      : 31.08.95 12:34
 * INPUTS    : struct Temporary_spell_data *Temp_spell_data - Pointer to
 *              temporary spell data.
 *             UNSHORT Duration - Duration.
 *             UNSHORT Strength - Strength.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_temporary_spell(struct Temporary_spell_data *Temp_spell_data,
 UNSHORT Duration, UNSHORT Strength)
{
	/* Is the temporary spell already on ? */
	if (Temp_spell_data->Duration)
	{
		/* Yes -> Add duration */
		Temp_spell_data->Duration += Duration;
		Temp_spell_data->Strength = max(Strength,
		 Temp_spell_data->Strength);
	}
	else
	{
		/* No -> Set new duration and strength */
		Temp_spell_data->Duration = Duration;
		Temp_spell_data->Strength = Strength;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_targets
 * FUNCTION  : Get a spell's targets.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:27
 * LAST      : 03.09.95 17:28
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNSHORT : Spell target bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_spell_targets(UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Spell_data *Spell_data;
	UNSHORT Value = 0;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Get spell targets */
			 Spell_data = (struct Spell_data *) MEM_Claim_pointer(Spell_data_handle);

			Value = (UNSHORT) Spell_data[Class_nr * SPELLS_PER_CLASS +
			 Spell_nr - 1].Target_bits;

			MEM_Free_pointer(Spell_data_handle);
		}
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_areas
 * FUNCTION  : Get the areas in which a spell can be cast.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:29
 * LAST      : 03.09.95 17:28
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNSHORT : Spell area bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_spell_areas(UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Spell_data *Spell_data;
	UNSHORT Value = 0;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Get spell areas */
			Spell_data = (struct Spell_data *) MEM_Claim_pointer(Spell_data_handle);

			Value = (UNSHORT) Spell_data[Class_nr * SPELLS_PER_CLASS +
			 Spell_nr - 1].Area_bits;

			MEM_Free_pointer(Spell_data_handle);
		}
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_required_SP
 * FUNCTION  : Get the amount of SP required to cast a spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:29
 * LAST      : 03.09.95 17:28
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNSHORT : Required amount of SP.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_spell_required_SP(UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Spell_data *Spell_data;
	UNSHORT Value = 1;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Cheat mode ? */
			if (!Cheat_mode)
			{
				/* No -> Get required SP from spell data */
				Spell_data = (struct Spell_data *) MEM_Claim_pointer(Spell_data_handle);

				Value = (UNSHORT) Spell_data[Class_nr * SPELLS_PER_CLASS +
				 Spell_nr - 1].Point_cost;

				MEM_Free_pointer(Spell_data_handle);
			}
		}
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_level
 * FUNCTION  : Get a spell's level.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:30
 * LAST      : 03.09.95 17:29
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNSHORT : Spell level.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_spell_level(UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Spell_data *Spell_data;
	UNSHORT Value = 0;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Get spell level */
			Spell_data = (struct Spell_data *) MEM_Claim_pointer(Spell_data_handle);

			Value = (UNSHORT) Spell_data[Class_nr * SPELLS_PER_CLASS +
			 Spell_nr - 1].Level;

			MEM_Free_pointer(Spell_data_handle);
		}
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_name
 * FUNCTION  : Get a spell's name.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 17:45
 * LAST      : 03.09.95 17:29
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNCHAR * : Pointer to name.
 * BUGS      : No known.
 * NOTES     : - Since the names are among the system texts, this function
 *              can return an absolute pointer.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Get_spell_name(UNSHORT Class_nr, UNSHORT Spell_nr)
{
	UNCHAR *Name = NULL;

	/* Legal spell class number ? */
	if (Class_nr < SPELL_CLASSES_MAX)
	{
		/* Yes -> Legal spell number ? */
		if (Spell_nr && (Spell_nr <= Spells_per_class[Class_nr]))
		{
			/* Yes -> Get spell name */
			Name = System_text_ptrs[203 + (Class_nr * SPELLS_PER_CLASS) +
			 Spell_nr - 1];
		}
	}

	return Name;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_spell
 * FUNCTION  : Let the user select a spell.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 11:03
 * LAST      : 02.08.95 12:28
 * INPUTS    : MEM_HANDLE Char_handle - Handle of casting character.
 * RESULT    : BOOLEAN : Spell selected.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Select_spell(MEM_HANDLE Char_handle)
{
	struct Spell_window_OID OID;
	struct Spell_object_data Spell_info[SPELL_CLASSES_MAX * SPELLS_PER_CLASS];
	UNSHORT Spell_window_object;
	UNSHORT Selected_class = 0xFFFF;
	UNSHORT Selected_spell = 0xFFFF;
	UNSHORT Counter;
	UNSHORT Current_spell_area;
	UNSHORT SP;
	UNSHORT Required_LP;
	UNSHORT Required_SP;
	UNSHORT i, j;

	/* Get SP of character */
	SP = Get_SP(Char_handle);

	/* Get current spell area */
	Current_spell_area = Get_spell_area();

	/* Check all spell classes */
	Counter = 0;
	for (i=0;i<SPELL_CLASSES_MAX;i++)
	{
		/* Is this spell class known ? */
		if (Spell_class_known(Char_handle, i))
		{
			/* Yes -> Check all spells in this class */
			for (j=1;j<=Spells_per_class[i];j++)
			{
				/* Is this spell known ? */
				if (Spell_known(Char_handle, i, j))
				{
					/* Yes -> Clear spell OID */
					BASEMEM_FillMemByte((UNBYTE *) &Spell_info[Counter],
					 sizeof(struct Spell_object_data), 0);

					/* Insert class and spell number */
					Spell_info[Counter].Class_nr = i;
					Spell_info[Counter].Spell_nr = j;

					/* Get spell strength */
					Spell_info[Counter].Strength =
					 Get_spell_strength(Char_handle, i, j) / 100;

					/* Can this spell be cast in the current area ? */
					if (Get_spell_areas(i, j) & (1 << Current_spell_area))
					{
						/* Yes -> Indicate this */
						Spell_info[Counter].Status = SPELL_OK;

						/* Get LP & SP required for this spell */
						Required_LP = Calculate_required_LP(Char_handle, i, j);
						Required_SP = Get_spell_required_SP(i, j);

						/* Too much LP required ? */
						if (Required_LP == 0xFFFF)
						{
							/* Yes -> Indicate this */
							Spell_info[Counter].Status = SPELL_TOO_MUCH_LP;
						}
						else
						{
							/* No -> Any LP required ? */
							if (Required_LP)
							{
								/* Yes -> Indicate this */
								Spell_info[Counter].Status = SPELL_TOO_MUCH_SP;

								/* Spell can only be cast once */
								Spell_info[Counter].Quantity = 1;
							}
							else
							{
								/* No -> Calculate how often this spell can be cast */
								if (Required_SP)
								{
									Spell_info[Counter].Quantity = SP / Required_SP;
								}
								else
								{
									Spell_info[Counter].Quantity = 1;
								}
							}
						}
					}
					else
					{
						/* No -> Indicate this */
						Spell_info[Counter].Status = SPELL_WRONG_AREA;
					}

					/* Count up */
					Counter++;
				}
			}
		}
	}

	/* Any spells known ? */
	if (Counter)
	{
		/* Yes -> Initialize spell window OID */
		OID.Char_handle			= Char_handle;
		OID.Spell_info				= &(Spell_info[0]);
		OID.Selected_class_ptr	= &Selected_class;
		OID.Selected_spell_ptr	= &Selected_spell;
		OID.Nr_spells				= Counter;

		/* Do spell window */
		Push_module(&Window_Mod);

		Spell_window_object = Add_object(0, &Spell_window_Class,
		 (UNBYTE *) &OID, 50, 50, 200, 119);

		Execute_method(Spell_window_object, DRAW_METHOD, NULL);

		Wait_4_object(Spell_window_object);
	}

	/* Copy selected spell */
	Current_use_magic_data.Class_nr = Selected_class;
	Current_use_magic_data.Spell_nr = Selected_spell;

	/* Any spell selected ? */
	return((Selected_class != 0xFFFF) && (Selected_spell != 0xFFFF));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Spell_window_object
 * FUNCTION  : Init method of Spell Window object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 11:55
 * LAST      : 28.04.95 20:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Spell_window_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_window_object *Spell_window;
	struct Spell_window_OID *OID;
	struct Spell_OID Spell_OID;
	struct Scroll_bar_OID Scroll_bar_OID;
	SISHORT Y;
	UNSHORT i;

	Spell_window = (struct Spell_window_object *) Object;
	OID = (struct Spell_window_OID *) P;

	/* Copy data from OID */
	Spell_window->Char_handle			= OID->Char_handle;
	Spell_window->Spell_info			= OID->Spell_info;
	Spell_window->Selected_class_ptr	= OID->Selected_class_ptr;
	Spell_window->Selected_spell_ptr	= OID->Selected_spell_ptr;
	Spell_window->Nr_spells				= OID->Nr_spells;

	/* Scroll bar needed ? */
	if (Spell_window->Nr_spells > MAX_SPELLS_IN_WINDOW)
	{
		/* Yes -> Calculate width and height WITH scroll bar */
		Change_object_size(Object->Self, SPELL_WIDTH + 27 + BETWEEN +
		 SCROLL_BAR_WIDTH, (MAX_SPELLS_IN_WINDOW * (SPELL_HEIGHT + 1)) + 26);

		/* Centre window */
		Change_object_position(Object->Self, (Screen_width -
		 Object->Rect.width) / 2, (Panel_Y - Object->Rect.height) / 2);

		/* Make scroll bar */
		Scroll_bar_OID.Total_units = Spell_window->Nr_spells;
		Scroll_bar_OID.Units_width = 1;
		Scroll_bar_OID.Units_height = MAX_SPELLS_IN_WINDOW;
		Scroll_bar_OID.Update = Update_spell_list;

		/* Add object */
		Spell_window->Scroll_bar_object = Add_object(Object->Self,
		 &Scroll_bar_Class, (UNBYTE *) &Scroll_bar_OID, SPELL_WIDTH + 13 +
		 BETWEEN, 12, SCROLL_BAR_WIDTH, (MAX_SPELLS_IN_WINDOW *
		 (SPELL_HEIGHT + 1)) - 1);

		/* Make spell objects */
		Y = 12;
	   Spell_OID.Number = 0;
		for (i=0;i<MAX_SPELLS_IN_WINDOW;i++)
		{
			/* Add object */
			Add_object(Object->Self, &Spell_Class, (UNBYTE *) &Spell_OID, 13, Y,
			 SPELL_WIDTH, SPELL_HEIGHT);

			/* Increase Y-coordinate */
			Y += SPELL_HEIGHT + 1;

			/* Count up */
			Spell_OID.Number++;
		}
	}
	else
	{
		/* No -> Calculate width and height WITHOUT scroll bar */
		Change_object_size(Object->Self, SPELL_WIDTH + 27,
		 (Spell_window->Nr_spells * (SPELL_HEIGHT + 1)) + 26);

		/* Centre window */
		Change_object_position(Object->Self, (Screen_width -
		 Object->Rect.width) / 2, (Panel_Y - Object->Rect.height) / 2);

		/* Make spell objects */
		Y = 12;
	   Spell_OID.Number = 0;
		for (i=0;i<Spell_window->Nr_spells;i++)
		{
			/* Add object */
			Add_object(Object->Self, &Spell_Class, (UNBYTE *) &Spell_OID, 13, Y,
			 SPELL_WIDTH, SPELL_HEIGHT);

			/* Increase Y-coordinate */
			Y += SPELL_HEIGHT + 1;

			/* Count up */
			Spell_OID.Number++;
		}

		/* Indicate there is no scroll bar */
		Spell_window->Scroll_bar_object = 0;
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Spell_window_object
 * FUNCTION  : Draw method of Spell Window object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 11:55
 * LAST      : 11.09.95 16:17
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Spell_window_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_window_object *Spell_window;
	struct OPM *OPM;

	Spell_window = (struct Spell_window_object *) Object;
	OPM = &(Spell_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Draw border around spells */
	Draw_deep_border
	(
		OPM,
		12,
		11,
		SPELL_WIDTH + 2,
		Object->Rect.height - 25
	);

	/* Draw spells */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_spell_list
 * FUNCTION  : Update the spell list (scroll bar function).
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 18:00
 * LAST      : 04.04.95 18:00
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_spell_list(struct Scroll_bar_object *Scroll_bar)
{
	struct Object *Parent;
	UNSHORT Child;

	/* Get parent object data */
	Parent = Get_object_data(Scroll_bar->Object.Parent);

	/* Draw all child objects except scroll bar */
	Child = Parent->Child;
	while (Child)
	{
		/* Is scroll bar ? */
		if (Child != Scroll_bar->Object.Self)
		{
			/* No -> Draw */
			Execute_method(Child, DRAW_METHOD, NULL);
		}

		/* Next brother */
		Child = (Get_object_data(Child))->Next;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Spell_object
 * FUNCTION  : Init method of Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:53
 * LAST      : 04.04.95 10:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Spell_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_object *Spell;
	struct Spell_OID *OID;

	Spell = (struct Spell_object *) Object;
	OID = (struct Spell_OID *) P;

	/* Copy data from OID */
	Spell->Number = OID->Number;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Spell_object
 * FUNCTION  : Draw method of Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:54
 * LAST      : 04.04.95 10:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Spell_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_object *Spell;
	struct Spell_window_object *Spell_window;
	struct OPM *OPM;

	Spell = (struct Spell_object *) Object;
	Spell_window = (struct Spell_window_object *) Get_object_data(Object->Parent);
	OPM = &(Spell_window->Window_OPM);

	/* Clear spell area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw spell */
	Draw_spell(Spell);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Spell_object
 * FUNCTION  : Feedback method of Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:54
 * LAST      : 04.04.95 10:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Spell_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_object *Spell;
	struct Spell_window_object *Spell_window;
	struct OPM *OPM;

	Spell = (struct Spell_object *) Object;
	Spell_window = (struct Spell_window_object *) Get_object_data(Object->Parent);
	OPM = &(Spell_window->Window_OPM);

	/* Clear spell area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Draw_deep_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw spell */
	Draw_spell(Spell);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Spell_object
 * FUNCTION  : Highlight method of Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:54
 * LAST      : 04.04.95 10:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Spell_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_object *Spell;
	struct Spell_window_object *Spell_window;
	struct OPM *OPM;

	Spell = (struct Spell_object *) Object;
	Spell_window = (struct Spell_window_object *) Get_object_data(Object->Parent);
	OPM = &(Spell_window->Window_OPM);

	/* Clear spell area */
	Draw_window_inside
	(
		OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	Put_recoloured_box
	(
		OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height,
		&(Recolour_tables[6][0])
	);

	/* Draw spell */
	Draw_spell(Spell);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Spell_object
 * FUNCTION  : Help method of Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:55
 * LAST      : 04.04.95 10:55
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Spell_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_object *Spell;
	struct Spell_window_object *Spell_window;
	struct Spell_object_data *Spell_data;
	UNSHORT Index;
	UNCHAR String[100];

	Spell = (struct Spell_object *) Object;
	Spell_window = (struct Spell_window_object *) Get_object_data(Object->Parent);

	/* Get spell data index */
	Index = Spell->Number;
	if (Spell_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Spell_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get spell data */
	Spell_data = Spell_window->Spell_info + Index;

	/* Does this spell cost LP ? */
	if (Spell_data->Status == SPELL_TOO_MUCH_SP)
	{
		/* Yes-> Make special help line string */
		sprintf
		(
			String,
			System_text_ptrs[427],
			Get_spell_required_SP
			(
				Spell_data->Class_nr,
				Spell_data->Spell_nr
			),
			Calculate_required_LP
			(
				Spell_window->Char_handle,
				Spell_data->Class_nr,
 				Spell_data->Spell_nr
			)
		);
	}
	else
	{
		/* No -> Make normal help line string */
		sprintf
		(
			String,
			System_text_ptrs[424],
			Get_spell_required_SP
			(
				Spell_data->Class_nr,
				Spell_data->Spell_nr
			)
		);
	}

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Spell_object
 * FUNCTION  : Left method of Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 10:55
 * LAST      : 28.04.95 20:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Spell_object(struct Object *Object, union Method_parms *P)
{
	struct Spell_object *Spell;
	struct Spell_window_object *Spell_window;
	struct Spell_object_data *Spell_data;
	UNSHORT Index;

	Spell = (struct Spell_object *) Object;
	Spell_window = (struct Spell_window_object *) Get_object_data(Object->Parent);

	/* Get spell data index */
	Index = Spell->Number;
	if (Spell_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Spell_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get spell data */
	Spell_data = Spell_window->Spell_info + Index;

	/* Is this spell for another area ? */
	if (Spell_data->Status == SPELL_WRONG_AREA)
	{
		/* Yes -> Print message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[423]);
	}

	/* Is this spell for another area ? */
	if (Spell_data->Status == SPELL_TOO_MUCH_LP)
	{
		/* Yes -> Print message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[426]);
	}

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Is this spell for another area ? */
		if ((Spell_data->Status != SPELL_WRONG_AREA) &&
		 (Spell_data->Status != SPELL_TOO_MUCH_LP))
		{
			/* No -> Select this spell */
			*(Spell_window->Selected_class_ptr) = Spell_data->Class_nr;
			*(Spell_window->Selected_spell_ptr) = Spell_data->Spell_nr;

			/* Close spell selection window */
			Execute_method(Spell_window->Object.Self, CLOSE_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Spell
 * FUNCTION  : Draw Spell object.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 16:32
 * LAST      : 04.04.95 16:32
 * INPUTS    : struct Spell_object *Spell - Pointer to spell object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_spell(struct Spell_object *Spell)
{
	struct BBRECT Clip, Old;
	struct Object *Object;
	struct Spell_window_object *Spell_window;
	struct Spell_object_data *Spell_data;
	struct OPM *OPM;
	UNSHORT Index;
	UNCHAR Number[4];

	Object = &(Spell->Object);
	Spell_window = (struct Spell_window_object *) Get_object_data(Object->Parent);
	OPM = &(Spell_window->Window_OPM);

	/* Get spell data index */
	Index = Spell->Number;
	if (Spell_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Spell_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get spell data */
	Spell_data = Spell_window->Spell_info + Index;

	/* Select ink colour depending on spell state */
	if (Spell_data->Status == SPELL_WRONG_AREA)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print spell name */
	Print_string(OPM, Object->X + 2, Object->Y + 2,
	 Get_spell_name(Spell_data->Class_nr, Spell_data->Spell_nr));

	/* Draw border around spell strength bar */
	Draw_deep_border(OPM, Object->X + 121, Object->Y + 3, 52, 6);

	/* Save current clip area */
	memcpy(&Old, &(OPM->clip), sizeof(struct BBRECT));

	/* Prepare new clip area */
	Clip.left	= Object->X + 122;
	Clip.top		= Object->Y + 4;
	Clip.height	= 4;

	/* Install new clip area */
	Clip.width	= Spell_data->Strength / 2;
	memcpy(&(OPM->clip), &Clip, sizeof(struct BBRECT));

	/* Display full spell strength bar */
	Put_masked_block(OPM, Object->X + 122, Object->Y + 4, 50, 4,
	 &Full_spell_strength_bar[0]);

	/* Install new clip area */
	Clip.left	= Object->X + 122 + Spell_data->Strength / 2;
	Clip.width	= 50 - Spell_data->Strength / 2;
	memcpy(&(OPM->clip), &Clip, sizeof(struct BBRECT));

	/* Display empty spell strength bar */
	Put_masked_block(OPM, Object->X + 122, Object->Y + 4, 50, 4,
	 &Empty_spell_strength_bar[0]);

	/* Restore old clip area */
	memcpy(&(OPM->clip), &Old, sizeof(struct BBRECT));

	/* Can the spell be cast at all ? */
	if (Spell_data->Status != SPELL_WRONG_AREA)
	{
		/* Yes -> Select ink colour depending on spell state */
		if (Spell_data->Status == SPELL_OK)
			Set_ink(SILVER_TEXT);
		else
			Set_ink(RED_TEXT);

		/* Display the number of times the spell can be cast */
		sprintf(Number, "%u", min(Spell_data->Quantity, 999));
		Print_centered_string(OPM, Object->X + 174, Object->Y + 2, 16, Number);
	}
}

