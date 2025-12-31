/************
 * NAME     : INVENTO.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : INVENTO.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <ALBION.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GAMETEXT.H>
#include <PRTLOGIC.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <INPUT.H>
#include <SCROLBAR.H>
#include <INVENTO.H>
#include <INVLEFT.H>
#include <INVITEMS.H>
#include <ITEMLIST.H>
#include <BUTTONS.H>
#include <INPUT.H>
#include <EVELOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <MAGIC.H>
#include <GOLDFOOD.H>
#include <COLOURS.H>
#include <MERCHANT.H>
#include <PLACES.H>
#include <TACTICAL.H>

/* defines */

#define INVENTORY_PAL_NR	(19)

/* structure definitions */

/* prototypes */

BOOLEAN Do_go_inventory(struct Event_action *Action);

/* Inventory module functions */
void Inventory_ModInit(void);
void Inventory_DisInit(void);
void Inventory_DisExit(void);

/* Right inventory methods */
UNLONG Init_InvRight_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvRight_object(struct Object *Object, union Method_parms *P);

/* Damage display methods */
UNLONG Init_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Damage_display_object(struct Object *Object, union Method_parms *P);

/* Protection display methods */
UNLONG Init_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Protection_display_object(struct Object *Object, union Method_parms *P);

/* Weight display methods */
UNLONG Init_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Weight_display_object(struct Object *Object, union Method_parms *P);

/* Party gold display methods */
UNLONG Init_Party_gold_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Party_gold_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Party_gold_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Party_gold_display_object(struct Object *Object, union Method_parms *P);

/* global variables */

/* Inventory module */
static struct Module Inventory_Mod = {
	0, SCREEN_MOD, INVENTORY_SCREEN,
	Update_display,
	Inventory_ModInit,
	Inventory_ModExit,
	Inventory_DisInit,
	Inventory_DisExit,
	NULL
};

/* Inventory parameters */
UNSHORT Inventory_member;
UNSHORT Inventory_mode = 1;
MEM_HANDLE Inventory_char_handle;

MEM_HANDLE Full_body_pic_handle;

BOOLEAN In_Inventory = FALSE;

/* Object handles */
UNSHORT InvLeft_object;
UNSHORT InvRight_object;

UNSHORT Body_item_list_object;
UNSHORT Backpack_item_list_object;

/* Right inventory object class */
static struct Method InvRight_methods[] = {
	{ INIT_METHOD,		Init_InvRight_object },
	{ DRAW_METHOD,		Draw_InvRight_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

struct Object_class InvRight_Class = {
	0, sizeof(struct Object),
	&InvRight_methods[0]
};

/* Damage display object class */
static struct Method Damage_display_methods[] = {
	{ INIT_METHOD,		Init_Damage_display_object },
	{ DRAW_METHOD,		Draw_Damage_display_object },
	{ UPDATE_METHOD,	Update_Damage_display_object },
	{ HELP_METHOD,		Help_Damage_display_object },
	{ TOUCHED_METHOD,	Normal_touched },
	{ 0, NULL}
};

static struct Object_class Damage_display_Class = {
	0, sizeof(struct Value_display_object),
	&Damage_display_methods[0]
};

/* Protection display object class */
static struct Method Protection_display_methods[] = {
	{ INIT_METHOD,		Init_Protection_display_object },
	{ DRAW_METHOD,		Draw_Protection_display_object },
	{ UPDATE_METHOD,	Update_Protection_display_object },
	{ HELP_METHOD,		Help_Protection_display_object },
	{ TOUCHED_METHOD,	Normal_touched },
	{ 0, NULL}
};

static struct Object_class Protection_display_Class = {
	0, sizeof(struct Value_display_object),
	&Protection_display_methods[0]
};

/* Weight display object class */
static struct Method Weight_display_methods[] = {
	{ INIT_METHOD,		Init_Weight_display_object },
	{ DRAW_METHOD,		Draw_Weight_display_object },
	{ UPDATE_METHOD,	Update_Weight_display_object },
	{ HELP_METHOD,		Help_Weight_display_object },
	{ TOUCHED_METHOD,	Normal_touched },
	{ 0, NULL}
};

static struct Object_class Weight_display_Class = {
	0, sizeof(struct Value_display_object),
	&Weight_display_methods[0]
};

/* Party gold display object class */
static struct Method Party_gold_display_methods[] = {
	{ INIT_METHOD,		Init_Party_gold_display_object },
	{ DRAW_METHOD,		Draw_Party_gold_display_object },
	{ UPDATE_METHOD,	Update_Party_gold_display_object },
	{ HELP_METHOD,		Help_Party_gold_display_object },
	{ TOUCHED_METHOD,	Normal_touched },
	{ 0, NULL}
};

static struct Object_class Party_gold_display_Class = {
	0, sizeof(struct Value_display_object),
	&Party_gold_display_methods[0]
};

/* Exit button data */
static union Button_data Exit_inventory_button_data;
struct Button_OID Exit_inventory_button_OID = {
	DOUBLE_SYMBOL_BUTTON_TYPE,
	0,
	21,
	&Exit_inventory_button_data,
	Exit_Inventory
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Go_Inventory
 * FUNCTION  : Go to a party member's inventory.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 13:00
 * LAST      : 16.10.95 21:44
 * INPUTS    : UNSHORT Member_nr - Party member number (1...6).
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Go_Inventory(UNSHORT Member_nr)
{
	static struct Event_action Inventory_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		SWITCH_TO_INVENTORY_ACTION, 0, 0,
		Do_go_inventory, NULL, NULL
	};

	BOOLEAN Result;

	/* Anyone there ? */
	if (!Member_present(Member_nr))
		return FALSE;

	/* Exit if this member's inventory cannot be accessed */
	if (Get_conditions(Party_char_handles[Member_nr - 1]) & INVENTORY_MASK)
		return FALSE;

	/* Build event action data */
	Inventory_action.Actor_index	= Member_nr;

	/* Check events */
	Result = Perform_action(&Inventory_action);

	return Result;
}

BOOLEAN
Do_go_inventory(struct Event_action *Action)
{
	BOOLEAN Result;
	UNSHORT Member_nr;

	/* Get member number */
	Member_nr = Action->Actor_index;

	/* Already in inventory ? */
	if (In_Inventory)
	{
		/* Yes -> Different member ? */
		if (Inventory_member != Member_nr)
		{
			/* Yes -> Leave current inventory */
			Exit_display();

			/* Exit inventory data */
			Exit_inventory_data();

			/* Set new inventory parameters */
			Inventory_char_handle	= Party_char_handles[Member_nr - 1];
			Inventory_member			= Member_nr;

			/* Initialize inventory data */
			Result = Init_inventory_data();
			if (!Result)
			{
				Pop_module();
			}

			/* Show */
			Init_display();
		}
	}
	else
	{
		/* Set inventory parameters */
		Inventory_char_handle	= Party_char_handles[Member_nr - 1];
		Inventory_member			= Member_nr;

		/* Enter inventory */
		Exit_display();
		Push_module(&Inventory_Mod);
		Init_display();
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Inventory
 * FUNCTION  : Leave the inventory screen (button function).
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 10:04
 * LAST      : 16.10.95 20:04
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Inventory(struct Button_object *Button)
{
	Exit_display();

	/* Display slab */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		0,
		0,
		360,
		Panel_Y,
		0,
		0
	);
	Switch_screens();

	Pop_module();
	Init_display();

	/* In combat ? */
	if (In_Combat)
	{
		/* Yes -> Check all actions */
		Check_party_combat_actions();

		/* Re-draw */
		Execute_method(Tactical_window_object, DRAW_METHOD, NULL);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Go_inventory_mode
 * FUNCTION  : Go to a mode of a party member's inventory.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:04
 * LAST      : 09.09.95 18:16
 * INPUTS    : UNSHORT Mode_nr - Inventory mode number (1...3).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Go_inventory_mode(UNSHORT Mode_nr)
{
	/* Are we already in this mode ? */
	if (Mode_nr != Inventory_mode)
	{
		/* No -> Switch */
		Inventory_mode = Mode_nr;

		/* Delete old mode objects */
		Delete_object(InvLeft_object);

		/* Add new mode objects */
		switch (Inventory_mode)
		{
			case INVLEFT_STATUS_1:
			{
				InvLeft_object = Add_object
				(
					Earth_object,
					&InvLeft1_Class,
					NULL,
					0,
					0,
					INVENTORY_MIDDLE,
					Panel_Y
				);
				break;
			}
			case INVLEFT_STATUS_2:
			{
				InvLeft_object = Add_object
				(
					Earth_object,
					&InvLeft2_Class,
					NULL,
					0,
					0,
					INVENTORY_MIDDLE,
					Panel_Y
				);
				break;
			}
			case INVLEFT_STATUS_3:
			{
				InvLeft_object = Add_object
				(
					Earth_object,
					&InvLeft3_Class,
					NULL,
					0,
					0,
					INVENTORY_MIDDLE,
					Panel_Y
				);
				break;
			}
		}

		/* Redraw */
		Execute_method(InvLeft_object, DRAW_METHOD, NULL);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Switch_inventory_mode
 * FUNCTION  : Switch to mode of a party member's inventory (radio button).
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:09
 * LAST      : 11.01.95 10:09
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Switch_inventory_mode(struct Button_object *Button)
{
	Go_inventory_mode(Button->Number);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_ModInit
 * FUNCTION  : Initialize Inventory module.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 16:55
 * LAST      : 05.07.95 12:46
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Inventory_ModInit(void)
{
	BOOLEAN Result;

	/* Initialize inventory data */
	Result = Init_inventory_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Prepare inventory exit button */
	Exit_inventory_button_OID.Help_message_nr	= 21;
	Exit_inventory_button_OID.Function			= Exit_Inventory;

	/* In Inventory */
	In_Inventory = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_ModExit
 * FUNCTION  : Exit Inventory module.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 16:55
 * LAST      : 05.07.95 12:47
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Inventory_ModExit(void)
{
	/* Exit inventory data */
	Exit_inventory_data();

	/* No longer in Inventory */
	In_Inventory = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_DisInit
 * FUNCTION  : Initialize Inventory display.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 11:16
 * LAST      : 04.01.95 11:16
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Inventory_DisInit(void)
{
	/* Add interface objects */
	switch (Inventory_mode)
	{
		case INVLEFT_STATUS_1:
		{
			InvLeft_object = Add_object
			(
				Earth_object,
				&InvLeft1_Class,
				NULL,
				0,
				0,
				INVENTORY_MIDDLE,
				Panel_Y
			);
			break;
		}
		case INVLEFT_STATUS_2:
		{
			InvLeft_object = Add_object
			(
				Earth_object,
				&InvLeft2_Class,
				NULL,
				0,
				0,
				INVENTORY_MIDDLE,
				Panel_Y
			);
			break;
		}
		case INVLEFT_STATUS_3:
		{
			InvLeft_object = Add_object
			(
				Earth_object,
				&InvLeft3_Class,
				NULL,
				0,
				0,
				INVENTORY_MIDDLE,
				Panel_Y
			);
			break;
		}
	}

	InvRight_object = Add_object
	(
		Earth_object,
		&InvRight_Class,
		NULL,
		INVENTORY_MIDDLE,
		0,
		Screen_width - INVENTORY_MIDDLE,
		Panel_Y
	);

	/* Draw */
	Execute_method(InvLeft_object, DRAW_METHOD, NULL);
	Execute_method(InvRight_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_DisExit
 * FUNCTION  : Exit Inventory display.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 11:15
 * LAST      : 04.01.95 11:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Inventory_DisExit(void)
{
	/* Delete interface objects */
	Delete_object(InvLeft_object);
	Delete_object(InvRight_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_inventory_data
 * FUNCTION  : Init inventory data.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 12:47
 * LAST      : 06.07.95 17:01
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_inventory_data(void)
{
	struct Character_data *Char;
	UNSHORT Nr;

	/* Get full body picture number */
	Char = (struct Character_data *) MEM_Claim_pointer(Inventory_char_handle);
	Nr = (UNSHORT) Char->Full_body_pic_nr;
	MEM_Free_pointer(Inventory_char_handle);

	Nr = PARTY_DATA.Member_nrs[Inventory_member - 1];

	/* Load full-body picture */
	Full_body_pic_handle = Load_subfile(FBODY_PICS, Nr);
	if (!Full_body_pic_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load inventory palette */
	Load_palette(INVENTORY_PAL_NR);

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_inventory_data
 * FUNCTION  : Exit inventory data.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 12:48
 * LAST      : 06.07.95 17:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_inventory_data(void)
{
	/* Free memory */
	MEM_Free_memory(Full_body_pic_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvRight_object
 * FUNCTION  : Initialize method of Right Inventory object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 12:02
 * LAST      : 18.08.95 18:42
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvRight_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_OID Item_list_OID;
	struct Gold_food_OID Gold_food_OID;

	/* Add body item list */
	Item_list_OID.Type = CHAR_BODY_INV_TYPE;
	Item_list_OID.Slots_handle = Inventory_char_handle;
	Item_list_OID.Slots_offset = Body_items_offset;
	Item_list_OID.Menu = &Inventory_item_PUM;

	Body_item_list_object = Add_object
	(
		Object->Self,
		&Body_item_list_Class,
		(UNBYTE *) &Item_list_OID,
		10,
		10,
		BODY_AREA_WIDTH,
		BODY_AREA_HEIGHT
	);

	/* Add backpack item list */
	Item_list_OID.Type						= CHAR_BACKPACK_INV_TYPE;
	Item_list_OID.Nr_items					= ITEMS_PER_CHAR;
	Item_list_OID.Slots_width				= 4;
	Item_list_OID.Slots_height				= 6;
	Item_list_OID.Slots_handle				= Inventory_char_handle;
	Item_list_OID.Slots_offset				= Backpack_items_offset;
	Item_list_OID.Menu						= &Inventory_item_PUM;
	Item_list_OID.Item_slot_class_ptr	= &Backpack_item_slot_Class;

	Backpack_item_list_object = Add_object
	(
		Object->Self,
		&Item_list_Class,
		(UNBYTE *) &Item_list_OID,
		145,
		11,
		100,
		100
	);

	/* Add damage display */
	Add_object
	(
		Object->Self,
		&Damage_display_Class,
		NULL,
		7,
		180,
		29,
		10
	);

	/* Add protection display */
	Add_object
	(
		Object->Self,
		&Protection_display_Class,
		NULL,
		112,
		180,
		29,
		10
	);

	/* Add weight display */
	Add_object
	(
		Object->Self,
		&Weight_display_Class,
		NULL,
		40,
		180,
		68,
		10
	);

	/* In Merchant screen ? */
	if (In_Merchant)
	{
		/* Yes -> Add party gold display */
		Add_object
		(
			Object->Self,
			&Party_gold_display_Class,
			NULL,
			145,
			141,
			70,
			20
		);
	}
	else
	{
		/* No -> Build gold object OID */
		Gold_food_OID.Type			= CHAR_GOLD_TYPE;
		Gold_food_OID.Data_handle	= Inventory_char_handle;

		/* Add gold object */
		Add_object
		(
			Object->Self,
			&Gold_food_Class,
			(UNBYTE *) &Gold_food_OID,
			145,
			141,
			35,
			20
		);

		/* Build food object OID */
		Gold_food_OID.Type = CHAR_FOOD_TYPE;

		/* Add food object */
		Add_object
		(
			Object->Self,
			&Gold_food_Class,
			(UNBYTE *) &Gold_food_OID,
			180,
			141,
			35,
			20
		);
	}

	/* Prepare exit button data */
	Exit_inventory_button_data.Symbol_button_data.Width				= 56;
	Exit_inventory_button_data.Symbol_button_data.Height				= 16;
	Exit_inventory_button_data.Symbol_button_data.Graphics_handle	= NULL;
	Exit_inventory_button_data.Symbol_button_data.Graphics_offset	= (UNLONG) &(Exit_button[0]);

	/* Add exit button */
	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &Exit_inventory_button_OID,
		154,
		171,
		56,
		17
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvRight_object
 * FUNCTION  : Draw method of Right Inventory object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 17:13
 * LAST      : 04.01.95 17:13
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvRight_object(struct Object *Object, union Method_parms *P)
{
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	/* Clear right inventory */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height,
		Object->X,
		Object->Y
	);

	/* Draw middle boundary */
	Put_recoloured_box(&Main_OPM, INVENTORY_MIDDLE, 0, 1, Panel_Y,
	 &(Recolour_tables[5][0]));
	Put_recoloured_box(&Main_OPM, INVENTORY_MIDDLE + 1, 0, 1, Panel_Y,
	 &(Recolour_tables[6][0]));
	Put_recoloured_box(&Main_OPM, INVENTORY_MIDDLE + 2, 0, 1, Panel_Y,
	 &(Recolour_tables[5][0]));
	Put_recoloured_box(&Main_OPM, INVENTORY_MIDDLE + 3, 0, 1, Panel_Y,
	 &(Recolour_tables[1][0]));
	Put_recoloured_box(&Main_OPM, INVENTORY_MIDDLE + 4, 0, 1, Panel_Y,
	 &(Recolour_tables[3][0]));

	Set_ink(SILVER_TEXT);

	/* Print member name */
	Get_char_name(Inventory_char_handle, Name);
	Print_centered_line_string
	(
		&Main_OPM,
		161,
		1,
		92,
		Name
	);

	/* Print backpack description */
	Print_centered_line_string
	(
		&Main_OPM,
		281,
		1,
		70,
		System_text_ptrs[47]
	);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Damage_display_object
 * FUNCTION  : Init method of Damage display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:52
 * LAST      : 10.01.95 11:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Damage_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Damage_display;

	Damage_display = (struct Value_display_object *) Object;

	/* Load initial damage value */
	Damage_display->Value = (SILONG) Get_damage(Inventory_char_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Damage_display_object
 * FUNCTION  : Draw method of Damage display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:20
 * LAST      : 25.09.95 16:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Damage_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Damage_display;
	UNCHAR String[20];

	Damage_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_object_background(Object);

	/* Draw surrounding box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Draw damage symbol */
	Put_masked_block
	(
		Current_OPM,
		Object->X + 1,
		Object->Y + 1,
		8,
		8,
		&(Damage_symbol[0])
	);

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print damage */
	_bprintf(String, 20, ":%3d", Damage_display->Value);
	Print_string
	(
		Current_OPM,
		Object->X + 10,
		Object->Y + 1,
		String
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Damage_display_object
 * FUNCTION  : Update method of Damage display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:20
 * LAST      : 18.08.95 18:48
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Damage_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Damage_display;
	SILONG Delta;
	SILONG Damage;

	Damage_display = (struct Value_display_object *) Object;

	/* Get damage */
	Damage = (SILONG) Get_damage(Inventory_char_handle);

	/* Has the damage value changed ? */
	Delta = Damage - Damage_display->Value;
	if (Delta)
	{
		/* Yes -> Adjust value */
		if (abs(Delta) > 100)
		{
			Damage_display->Value += 100 * sgn(Delta);
		}
		else
		{
			if (abs(Delta) > 10)
			{
				Damage_display->Value += 10 * sgn(Delta);
			}
			else
			{
				Damage_display->Value += sgn(Delta);
			}
		}

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Damage_display_object
 * FUNCTION  : Help method of Damage display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:22
 * LAST      : 10.01.95 11:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Damage_display_object(struct Object *Object, union Method_parms *P)
{
	UNCHAR String[100];

	/* Make help line string */
	_bprintf
	(
		String,
		100,
		System_text_ptrs[677],
		Get_damage(Inventory_char_handle)
	);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Protection_display_object
 * FUNCTION  : Init method of Protection display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:54
 * LAST      : 10.01.95 11:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Protection_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Protection_display;

	Protection_display = (struct Value_display_object *) Object;

	/* Load initial protection value */
	Protection_display->Value = (SILONG) Get_protection(Inventory_char_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Protection_display_object
 * FUNCTION  : Draw method of Protection display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:20
 * LAST      : 25.09.95 16:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Protection_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Protection_display;
	UNCHAR String[20];

	Protection_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_object_background(Object);

	/* Draw surrounding box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Draw protection symbol */
	Put_masked_block
	(
		Current_OPM,
		Object->X + 1,
		Object->Y + 1,
		6,
		8,
		&(Protection_symbol[0])
	);

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print protection */
	_bprintf(String, 20, ":%3d", Protection_display->Value);
	Print_string
	(
		Current_OPM,
		Object->X + 8,
		Object->Y + 1,
		String
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Protection_display_object
 * FUNCTION  : Update method of Protection display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:20
 * LAST      : 18.08.95 18:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Protection_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Protection_display;
	SILONG Delta;
	SILONG Protection;

	Protection_display = (struct Value_display_object *) Object;

	/* Get protection */
	Protection = Get_protection(Inventory_char_handle);

	/* Has the protection value changed ? */
	Delta = Protection - Protection_display->Value;
	if (Delta)
	{
		/* Yes -> Adjust value */
		if (abs(Delta) > 100)
		{
			Protection_display->Value += 100 * sgn(Delta);
		}
		else
		{
			if (abs(Delta) > 10)
			{
				Protection_display->Value += 10 * sgn(Delta);
			}
			else
			{
				Protection_display->Value += sgn(Delta);
			}
		}

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Protection_display_object
 * FUNCTION  : Help method of Protection display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:22
 * LAST      : 10.01.95 11:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Protection_display_object(struct Object *Object, union Method_parms *P)
{
	UNCHAR String[100];

	/* Make help line string */
	_bprintf
	(
		String,
		100,
		System_text_ptrs[678],
		Get_protection(Inventory_char_handle)
	);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Weight_display_object
 * FUNCTION  : Init method of Weight display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 12:12
 * LAST      : 05.10.95 15:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Weight_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Weight_display;

	Weight_display = (struct Value_display_object *) Object;

	/* Load initial weight and maximum weight value */
	Weight_display->Value		= Get_carried_weight(Inventory_char_handle);
	Weight_display->Max_value	= Get_max_weight(Inventory_char_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Weight_display_object
 * FUNCTION  : Draw method of Weight display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 12:16
 * LAST      : 05.10.95 15:36
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Weight_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Weight_display;
	UNCHAR String[80];

	Weight_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_object_background(Object);

	/* Draw surrounding box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);


	/* Is the character overweight ? */
	if (Weight_display->Value > Weight_display->Max_value)
	{
		/* Yes -> Set ink colour */
		Set_ink(RED_TEXT);
	}
	else
	{
		/* No -> Set ink colour */
		Set_ink(SILVER_TEXT);
	}

	/* Print weight */
	_bprintf
	(
		String,
		80,
		System_text_ptrs[679],
		Weight_display->Value / 1000
	);
	Print_centered_string
	(
		Current_OPM,
		Object->X + 1,
		Object->Y + 1,
		Object->Rect.width - 2,
		String
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Weight_display_object
 * FUNCTION  : Update method of Weight display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:20
 * LAST      : 05.10.95 20:32
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Weight_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Weight_display;
	SILONG Delta;
	SILONG Weight;
	SILONG Max_weight;

	Weight_display = (struct Value_display_object *) Object;

	/* Get current maximum weight */
	Max_weight = Get_max_weight(Inventory_char_handle);

	/* Has the maximum weight value changed ? */
	if (Weight_display->Max_value != Max_weight)
	{
		/* Yes -> Set new maximum value */
		Weight_display->Max_value = Max_weight;

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}
	else
	{
		/* No -> Get current weight */
		Weight = Get_carried_weight(Inventory_char_handle);

		/* Has the weight value changed ? */
		if ((Weight / 1000) != (Weight_display->Value / 1000))
		{
			/* Yes -> Calculate difference */
			Delta = Weight - Weight_display->Value;

			/* Adjust value */
			if (abs(Delta) > 10000)
			{
				Weight_display->Value += 10000 * sgn(Delta);
			}
			else
			{
				if (abs(Delta) > 1000)
				{
					Weight_display->Value += 1000 * sgn(Delta);
				}
				else
				{
					Weight_display->Value += Delta;
				}
			}

			/* Redraw */
			Execute_method(Object->Self, DRAW_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Weight_display_object
 * FUNCTION  : Help method of Weight display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:22
 * LAST      : 05.10.95 15:45
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Weight_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Weight_display;
	UNCHAR String[100];

	Weight_display = (struct Value_display_object *) Object;

	/* Make help line string */
	_bprintf
	(
		String,
		100,
		System_text_ptrs[680],
		Weight_display->Value,
		Weight_display->Max_value
	);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Party_gold_display_object
 * FUNCTION  : Init method of party gold display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 18:34
 * LAST      : 18.08.95 18:34
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Party_gold_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Party_gold_display;

	Party_gold_display = (struct Value_display_object *) Object;

	/* Load initial value */
	Party_gold_display->Value = Get_party_gold();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Party_gold_display_object
 * FUNCTION  : Draw method of party gold display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 18:35
 * LAST      : 25.09.95 16:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Party_gold_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Party_gold_display;
	UNCHAR String[80];

	Party_gold_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_object_background(Object);

	/* Draw surrounding box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Draw gold symbol */
	Put_masked_block
	(
		Current_OPM,
		Object->X + (Object->Rect.width - 12) / 2,
		Object->Y + 1,
		12,
		10,
		&(Gold_symbol[0])
	);

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print "party" */
	Print_centered_string
	(
		Current_OPM,
		Object->X + 1,
		Object->Y + 1,
		Object->Rect.width - 2,
		System_text_ptrs[585]
	);

	/* Print party gold */
	_bprintf
	(
		String,
		80,
		"%ld.%d",
		Party_gold_display->Value / 10,
		(SISHORT) (Party_gold_display->Value % 10)
	);

	Print_centered_string
	(
		Current_OPM,
		Object->X + 1,
		Object->Y + 11,
		Object->Rect.width - 2,
		String
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Party_gold_display_object
 * FUNCTION  : Update method of party gold display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 18:36
 * LAST      : 18.08.95 18:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Party_gold_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Party_gold_display;
	SILONG Delta;

	Party_gold_display = (struct Value_display_object *) Object;

	/* Has the party gold value changed ? */
	Delta = Get_party_gold() - Party_gold_display->Value;
	if (Delta)
	{
		/* Yes -> Adjust value */
		if (abs(Delta) > 100)
		{
			Party_gold_display->Value += 100 * sgn(Delta);
		}
		else
		{
			if (abs(Delta) > 10)
			{
				Party_gold_display->Value += 10 * sgn(Delta);
			}
			else
			{
				Party_gold_display->Value += sgn(Delta);
			}
		}

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Party_gold_display_object
 * FUNCTION  : Help method of party gold display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 18:38
 * LAST      : 25.08.95 14:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Party_gold_display_object(struct Object *Object, union Method_parms *P)
{
	UNLONG Party_gold;
	UNCHAR String[100];

	/* Get party gold */
	Party_gold = Get_party_gold();

	/* Make help line string */
	_bprintf
	(
		String,
		100,
		System_text_ptrs[586],
		Party_gold / 10,
		(UNSHORT)(Party_gold % 10)
	);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

