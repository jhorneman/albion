/************
 * NAME     : APRES.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 07-03-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : APRES.H
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <APRES.H>
#include <PRTLOGIC.H>
#include <EVELOGIC.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <GOLDFOOD.H>
#include <BUTTONS.H>
#include <GAMETEXT.H>
#include <BOOLREQ.H>
#include <COLOURS.H>
#include <TEXTWIN.H>
#include <INPUTNR.H>
#include <EXAMITEM.H>
#include <COMBAT.H>

/*
 ** Defines ****************************************************************
 */

#define APRES_COMBAT_PICTURE	(14)

#define MAX_APRES_ITEMS	((6 + MONSTERS_PER_GROUP) * (ITEMS_ON_BODY + ITEMS_PER_CHAR))

/*
 ** Prototypes *************************************************************
 */

/* Apres combat module functions */
void Apres_ModInit(void);
void Apres_ModExit(void);
void Apres_DisInit(void);
void Apres_DisExit(void);
void Apres_MainLoop(void);

void Exit_Apres(struct Button_object *Button);

void Divide_EP(UNLONG Added_EP);

/* Distribute character items module functions */
void Distribute_char_items_ModInit(void);

/* Apres combat methods */
UNLONG Init_Apres_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Apres_object(struct Object *Object, union Method_parms *P);
UNLONG Restore_Apres_object(struct Object *Object, union Method_parms *P);

/* Apres pool item slot methods (replacing normal item slot methods) */
UNLONG Draw_Apres_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Feedback_Apres_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Highlight_Apres_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Left_Apres_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG DLeft_Apres_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Drop_Apres_item_slot_object(struct Object *Object,
 union Method_parms *P);

/* Apres item slot support functions */
void Drag_Apres_item(struct Item_slot_object *Item_slot, UNSHORT Quantity);
void Drop_on_Apres_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr);
void Apres_item_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr);

/* Apres item pop-up menu actions */
void Apres_item_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_Apres_item(UNLONG Data);
void PUM_Examine_Apres_item(UNLONG Data);

void Drop_Apres_items(UNSHORT Item_slot_handle, UNSHORT Quantity);

BOOLEAN Take_item_from_Apres_pool(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Target_packet);
BOOLEAN Put_item_in_Apres_pool(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Source_packet);

void Auto_move_packet_to_Apres_pool(struct Item_packet *Source_packet);

void Move_drag_HDOB_towards_Apres_slot(UNSHORT Target_slot_index);

void Pool_Apres_pool(void);

/*
 ** Global variables *******************************************************
 */

UNSHORT Apres_item_list_object;

/* Apres pool variables */
static UNSHORT Nr_Apres_items;

/* (needed by GoldFood object) */
UNLONG Apres_gold_coins;
UNLONG Apres_food_rations;

static MEM_HANDLE Apres_pool_handle;

/* Modules */
static struct Module Apres_Mod = {
	LOCAL_MOD, SCREEN_MOD, CHEST_SCREEN,
	Apres_MainLoop,
	Apres_ModInit,
	Apres_ModExit,
	Apres_DisInit,
	Apres_DisExit,
	NULL
};

static struct Module Distribute_char_items_Mod = {
	LOCAL_MOD, SCREEN_MOD, CHEST_SCREEN,
	Apres_MainLoop,
	Distribute_char_items_ModInit,
	Apres_ModExit,
	Apres_DisInit,
	Apres_DisExit,
	NULL
};

/* Apres combat method list */
static struct Method Apres_methods[] = {
	{ INIT_METHOD,		Init_Apres_object },
	{ DRAW_METHOD,		Draw_Apres_object },
	{ RESTORE_METHOD,	Restore_Apres_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Apres combat object class */
static struct Object_class Apres_Class = {
	0, sizeof(struct Object),
	&Apres_methods[0]
};

/* Apres combat item slot method list */
static struct Method Apres_item_slot_methods[] = {
	{ INIT_METHOD,				Init_Item_slot_object },
	{ DRAW_METHOD,				Draw_Apres_item_slot_object },
	{ UPDATE_METHOD,			Update_Item_slot_object },
	{ FEEDBACK_METHOD,		Feedback_Apres_item_slot_object },
	{ HIGHLIGHT_METHOD,		Highlight_Apres_item_slot_object },
	{ POP_UP_METHOD,			Pop_up_Item_slot_object },
	{ HELP_METHOD,				Help_Item_slot_object },
	{ LEFT_METHOD,				Left_Apres_item_slot_object },
	{ DLEFT_METHOD,			DLeft_Apres_item_slot_object },
	{ RIGHT_METHOD,			Right_Item_slot_object },
	{ TOUCHED_METHOD,			Touch_Item_slot_object },
	{ DROP_METHOD,				Drop_Apres_item_slot_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_item_slot_object },
	{ 0, NULL}
};

/* Apres combat item slot object class */
static struct Object_class Apres_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Apres_item_slot_methods[0]
};

/* Apres combat item pop-up menu entry list */
static struct PUME Apres_item_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 15, PUM_Drop_Apres_item},
   {PUME_AUTO_CLOSE, 0, 17, PUM_Examine_Apres_item},
};

/* Apres combat item pop-up menu */
static struct PUM Apres_item_PUM = {
	2,
	NULL,
	(UNLONG) &Selected_item,
	Apres_item_PUM_evaluator,
	Apres_item_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_Apres_combat
 * FUNCTION  : Enter Apres combat screen.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 11:15
 * LAST      : 01.10.95 17:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_Apres_combat(void)
{
	/* Any items, gold or food ? */
	if (Nr_Apres_items || Apres_gold_coins || Apres_food_rations)
	{
		/* Yes -> Enter Apres combat screen */
		Push_module(&Apres_Mod);

		/* Re-draw the screen */
		Init_display();
	}
	else
	{
		/* No -> Re-draw the screen */
		Init_display();

		/* Divide experience points */
		Divide_EP(Added_experience_points);

		/* Clear combat conditions */
		Clear_combat_conditions();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Distribute_char_items
 * FUNCTION  : Distribute character items screen.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.10.95 16:28
 * LAST      : 23.10.95 14:37
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Distribute_char_items(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_packet *Apres_pool;
	struct Item_packet *Packet;
	BOOLEAN Result;
	UNSHORT i;

	/* Still in move mode ? */
	if (Move_mode_flag)
	{
		/* Yes -> Exit move mode */
		Pop_module();

		/* Clear flag again just to be sure */
		Move_mode_flag = FALSE;
	}

	/* Initialise Apres pool */
	Result = Init_Apres_pool();
	if (!Result)
	{
		Exit_program();
		return;
	}

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Put items in Apres pool */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			Store_item_in_Apres_pool(&(Char->Body_items[i]));
			Remove_item
			(
				Char_handle,
				i + 1,
				Char->Body_items[i].Quantity
			);
		}
	}
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		if (!Packet_empty(&(Char->Backpack_items[i])))
		{
			Store_item_in_Apres_pool(&(Char->Backpack_items[i]));
			Remove_item
			(
				Char_handle,
				i + ITEMS_ON_BODY + 1,
				Char->Backpack_items[i].Quantity
			);
		}
	}

	/* Put gold and food in Apres pool */
	Put_gold_in_Apres_pool(Char->Char_gold);
	Put_food_in_Apres_pool(Char->Char_food);

	MEM_Free_pointer(Char_handle);

	/* Any items, gold or food ? */
	if (Nr_Apres_items || Apres_gold_coins || Apres_food_rations)
	{
		/* Yes -> Enter Distribute character items screen */
		Exit_display();
		Push_module(&Distribute_char_items_Mod);
		Init_display();
	}

	/* Put items left in Apres pool back in character */
	Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

	for (i=0;i<Nr_Apres_items;i++)
	{
		Packet = &(Apres_pool[i]);

		/* Anything in this packet ? */
		if (!Packet_empty(Packet))
		{
			/* Yes -> Move to backpack */
			Auto_move_packet_to_backpack(Char_handle, Packet);
		}
	}

	MEM_Free_pointer(Apres_pool_handle);

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Put gold and food left in Apres pool back in character */
	Char->Char_gold = Apres_gold_coins;
	Char->Char_food = Apres_food_rations;

	MEM_Free_pointer(Char_handle);

	/* Destroy Apres pool */
	Exit_Apres_pool();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_ModInit
 * FUNCTION  : Initialize Apres combat module.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:41
 * LAST      : 02.09.95 14:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_ModInit(void)
{
	BOOLEAN Result;

	/* Set inventory parameters */
	Inventory_char_handle	= Active_char_handle;
	Inventory_member			= PARTY_DATA.Active_member;

	/* Initialize inventory data */
	Result = Init_inventory_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Prepare inventory exit button */
	Exit_inventory_button_OID.Help_message_nr = 547;
	Exit_inventory_button_OID.Function = Exit_Apres;

	/* Load small picture */
	Load_small_picture(APRES_COMBAT_PICTURE);

	/* In Inventory */
	In_Inventory = TRUE;

	/* Pool the Apres pool */
	Pool_Apres_pool();

	/* Initialize display */
	Apres_DisInit();

	/* Divide experience points */
	Divide_EP(Added_experience_points);

	/* Clear combat conditions */
	Clear_combat_conditions();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_ModExit
 * FUNCTION  : Exit Apres combat module.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:43
 * LAST      : 15.08.95 19:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_ModExit(void)
{
	Apres_DisExit();

	Inventory_ModExit();

	Destroy_small_picture();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_DisInit
 * FUNCTION  : Initialize Apres combat display.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:45
 * LAST      : 15.08.95 19:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_DisInit(void)
{
	/* Add interface objects */
	InvLeft_object = Add_object(Earth_object, &Apres_Class, NULL,
	 0, 0, INVENTORY_MIDDLE, Panel_Y);
	InvRight_object = Add_object(Earth_object, &InvRight_Class, NULL,
	 INVENTORY_MIDDLE, 0, Screen_width - INVENTORY_MIDDLE, Panel_Y);

	/* Draw */
	Execute_method(InvLeft_object, DRAW_METHOD, NULL);
	Execute_method(InvRight_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_DisExit
 * FUNCTION  : Exit Apres combat display.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:45
 * LAST      : 15.08.95 19:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_DisExit(void)
{
	/* Delete interface objects */
	Delete_object(InvLeft_object);
	Delete_object(InvRight_object);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_MainLoop
 * FUNCTION  : Main loop of Apres combat module.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.10.95 22:59
 * LAST      : 14.10.95 15:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_MainLoop(void)
{
	struct Item_packet *Apres_pool;
	BOOLEAN All_items_gone_flag;

	/* Check if all items are gone */
	Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

	All_items_gone_flag = Inventory_empty
	(
		Apres_pool,
		Nr_Apres_items
	);

	MEM_Free_pointer(Apres_pool_handle);

	/* Check if gold and food are gone */
	if (Apres_gold_coins || Apres_food_rations)
		All_items_gone_flag = FALSE;

	/* Not in drag & drop mode / everything gone ? */
	if (!Drag_drop_mode && All_items_gone_flag)
	{
		/* Yes -> Exit */
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Apres
 * FUNCTION  : Leave the Apres combat screen.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 10:55
 * LAST      : 16.08.95 18:51
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Apres(struct Button_object *Button)
{
	struct Item_data *Item_data;
	struct Item_packet *Apres_pool;
	struct Item_packet *Packet;
	UNSHORT Nr_vital_items;
	UNSHORT Nr_items;
	UNSHORT i;

	/* Count the items and vital items in the Apres pool */
	Nr_items = 0;
	Nr_vital_items = 0;

	Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

	for (i=0;i<Nr_Apres_items;i++)
	{
		Packet = &(Apres_pool[i]);

		/* Anything in this packet ? */
		if (!Packet_empty(Packet))
		{
			/* Yes -> Get item data */
			Item_data = Get_item_data(Packet);

			/* Is the item undroppable ? */
			if (Item_data->Flags & UNDROPPABLE)
			{
				/* Yes -> Count up */
				Nr_vital_items++;
			}

			/* Count up */
			Nr_items++;

			Free_item_data();
		}
	}

	MEM_Free_pointer(Apres_pool_handle);

	/* Any vital items left ? */
	if (Nr_vital_items)
	{
		/* Yes -> Tell the player */
		Do_text_window(System_text_ptrs[550]);
	}
	else
	{
		/* No -> Any items, gold or food left ? */
		if (Nr_items || Apres_gold_coins || Apres_food_rations)
		{
			/* Yes -> Are you sure ? */
			if (Boolean_requester(System_text_ptrs[549]))
			{
				/* Yes -> Exit */
				Pop_module();
			}
		}
		else
		{
			/* No -> Exit */
			Pop_module();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Distribute_char_items_ModInit
 * FUNCTION  : Initialize Distribute character items module.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.10.95 16:46
 * LAST      : 11.10.95 16:46
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Distribute_char_items_ModInit(void)
{
	BOOLEAN Result;

	/* Set inventory parameters */
	Inventory_char_handle	= Active_char_handle;
	Inventory_member			= PARTY_DATA.Active_member;

	/* Initialize inventory data */
	Result = Init_inventory_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Prepare inventory exit button */
	Exit_inventory_button_OID.Help_message_nr = 421;
	Exit_inventory_button_OID.Function = Exit_Apres;

	/* Load small picture */
	Load_small_picture(APRES_COMBAT_PICTURE);

	/* In Inventory */
	In_Inventory = TRUE;

	/* Pool the Apres pool */
	Pool_Apres_pool();

	/* Initialize display */
	Apres_DisInit();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Divide_EP
 * FUNCTION  : Divide experience points over party members.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 21:10
 * LAST      : 28.09.95 12:25
 * INPUTS    : UNLONG Added_EP - Added experience points.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - It is important that this be done before clearing combat
 *              conditions.
 *             - If the criteria for receiving EP change, I can no longer use
 *              Count_alive_party_members.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Divide_EP(UNLONG Added_EP)
{
	UNSHORT EP_per_member;
	UNSHORT Total_members;
	UNSHORT i;
	UNCHAR String[200];

	/* Count party members */
	Total_members = Count_alive_party_members();

	/* Any eligible members ? */
	if (Total_members)
	{
		/* Yes -> Calculate EP per member */
		EP_per_member = max(1, (Added_EP / Total_members));

		/* Tell the player */
		_bprintf(String, 200, System_text_ptrs[552], EP_per_member);
		Do_text_window(String);

		/* Divide experience points */
		for (i=0;i<6;i++)
		{
			/* Anyone there ? */
			if (Member_present(i + 1))
			{
				/* Yes -> Alive and kicking ? */
				if (Character_alive(Party_char_handles[i]))
				{
					/* Yes -> Experience */
					Set_EP
					(
						Party_char_handles[i],
						EP_per_member + Get_EP(Party_char_handles[i])
					);
				}
			}
		}
	}
	else
	{
		/* No -> Tell the player */
		Do_text_window(System_text_ptrs[551]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_combat_conditions
 * FUNCTION  : Clear all combat-only body conditions for all party members.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 21:09
 * LAST      : 26.09.95 20:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - It is important that this be done after dividing EP.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_combat_conditions(void)
{
	UNSHORT i;

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Clear combat conditions */
			Clear_condition(Party_char_handles[i], IRRITATED);
			Clear_condition(Party_char_handles[i], ASLEEP);
			Clear_condition(Party_char_handles[i], PANICKED);
			Clear_condition(Party_char_handles[i], FLEEING);
			Clear_condition(Party_char_handles[i], LAMED);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Apres_object
 * FUNCTION  : Initialize method of Apres combat object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:46
 * LAST      : 11.10.95 18:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Apres_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_OID Item_list_OID;
	struct Gold_food_OID Gold_food_OID;
	SISHORT X;
	UNSHORT Nr_item_rows;
	UNSHORT Width, Height;

	/* Any items in apres pool ? */
	if (Nr_Apres_items)
	{
		/* Yes -> Calculate number of item rows */
		Nr_item_rows = max(4, min(7, (Nr_Apres_items / 6)));

		/* Change number of Apres items (!) */
		Nr_Apres_items = Nr_item_rows * 6;

		/* Calculate Apres combat item list dimensions */
		Width = (6 * ITEM_SLOT_OUTER_WIDTH) - ITEM_SLOT_BETWEEN_X + 2;
		Height = (Nr_item_rows * ITEM_SLOT_OUTER_HEIGHT) + 2;

		/* Calculate Apres combat item list X-coordinate */
		X = (Object->Rect.width - Width) / 2;

		/* Add Apres combat item list */
		Item_list_OID.Type						= NO_CHAR_INV_TYPE;
		Item_list_OID.Nr_items					= Nr_Apres_items;
		Item_list_OID.Slots_width				= 6;
		Item_list_OID.Slots_height				= Nr_item_rows;
		Item_list_OID.Slots_handle				= Apres_pool_handle;
		Item_list_OID.Slots_offset				= 0;
		Item_list_OID.Menu						= &Apres_item_PUM;
		Item_list_OID.Item_slot_class_ptr	= &Apres_item_slot_Class;

		Apres_item_list_object = Add_object
		(
			Object->Self,
			&Item_list_Class,
			(UNBYTE *) &Item_list_OID,
			X,
			14,
			Width,
			Height
		);
	}
	else
	{
		/* No -> Set coordinates */
		X = 10;
		Width = Object->Rect.width - (2 * X);
	}

	/* Any gold in apres pool ? */
	if (Apres_gold_coins)
	{
		/* Yes -> Build gold object OID */
		Gold_food_OID.Type			= APRES_GOLD_TYPE;
		Gold_food_OID.Data_handle	= NULL;

		/* Add gold object */
		Add_object
		(
			Object->Self,
			&Gold_food_Class,
			(UNBYTE *) &Gold_food_OID,
			X,
			Panel_Y - 25,
			35,
			20
		);
	}

	/* Any food in apres pool ? */
	if (Apres_food_rations)
	{
		/* Yes -> Build food object OID */
		Gold_food_OID.Type			= APRES_FOOD_TYPE;
		Gold_food_OID.Data_handle	= NULL;

		/* Add food object */
		Add_object
		(
			Object->Self,
			&Gold_food_Class,
			(UNBYTE *) &Gold_food_OID,
			X + Width - 35,
			Panel_Y - 25,
			35,
			20
		);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Apres_object
 * FUNCTION  : Draw method of Apres combat object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:54
 * LAST      : 26.09.95 20:48
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Apres_object(struct Object *Object, union Method_parms *P)
{
	/* Clear left inventory */
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

	/* Draw picture */
	Display_small_picture_centered
	(
		&Main_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Print apres combat description */
	Set_ink(SILVER_TEXT);
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		1,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[548]
	);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_Apres_object
 * FUNCTION  : Restore method of Apres combat object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 20:47
 * LAST      : 26.09.95 20:47
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Restore_Apres_object(struct Object *Object, union Method_parms *P)
{
	struct BBRECT *Rect;
	struct BBRECT Old;

	Rect = P->Rect;

	/* Install clip area */
	memcpy(&Old, &(Main_OPM.clip), sizeof(struct BBRECT));
	memcpy(&(Main_OPM.clip), Rect, sizeof(struct BBRECT));

	/* Clear left inventory */
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

	/* Draw picture */
	Display_small_picture_centered
	(
		&Main_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Print apres combat description */
	Set_ink(SILVER_TEXT);
	Print_centered_line_string
	(
		&Main_OPM,
		4,
		1,
		INVENTORY_MIDDLE - 8,
		System_text_ptrs[548]
	);

	/* Restore clip area */
	memcpy(&(Main_OPM.clip), &Old, sizeof(struct BBRECT));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Apres_item_slot_object
 * FUNCTION  : Draw method of Apres item slot object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 20:17
 * LAST      : 25.09.95 16:48
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Apres_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item_data;

	Item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Restore_object_background(Object);

	/* Draw dark box */
	Put_recoloured_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		ITEM_SLOT_INNER_WIDTH,
		ITEM_SLOT_INNER_HEIGHT,
		&(Recolour_tables[3][0])
	);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

 	/* Get item data */
	Item_data = Get_item_data(&Packet);

	/* Is a vital item ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes -> Show this */
		OPM_Box
		(
			Current_OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			BLUE
		);
	}

	Free_item_data();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Apres_item_slot_object
 * FUNCTION  : Feedback method of Apres item slot object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 20:18
 * LAST      : 25.09.95 16:48
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Apres_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item_data;

	Item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Restore_object_background(Object);

	/* Draw dark box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		ITEM_SLOT_INNER_WIDTH,
		ITEM_SLOT_INNER_HEIGHT
	);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

 	/* Get item data */
	Item_data = Get_item_data(&Packet);

	/* Is a vital item ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes -> Show this */
		OPM_Box
		(
			Current_OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			BLUE
		);
	}

	Free_item_data();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Apres_item_slot_object
 * FUNCTION  : Highlight method of Apres item slot object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 20:18
 * LAST      : 25.09.95 16:49
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Apres_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item_data;

	Item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Restore_object_background(Object);

	/* Draw bright box */
	Put_recoloured_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		ITEM_SLOT_INNER_WIDTH,
		ITEM_SLOT_INNER_HEIGHT,
		&(Recolour_tables[6][0])
	);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

 	/* Get item data */
	Item_data = Get_item_data(&Packet);

	/* Is a vital item ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes -> Show this */
		OPM_Box
		(
			Current_OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			BLUE
		);
	}

	Free_item_data();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Apres_item_slot_object
 * FUNCTION  : Left method of Apres item slot object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 13:43
 * LAST      : 16.08.95 15:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Apres_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_object *Item_list;
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item;
	UNSHORT Quantity;
	UNSHORT Slot_index;

	Item_slot = (struct Item_slot_object *) Object;
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Get item data */
			Item = Get_item_data(&Packet);

			/* More than one item in the packet ? */
			Quantity = 1;
			if (Packet.Quantity > 1)
			{
				/* Yes -> Ask the player how many should be taken */
				Quantity = (UNSHORT) Input_number_with_symbol
				(
					1,
					0,
					(SILONG) Packet.Quantity,
					System_text_ptrs[66],
					16,
					16,
					Item_graphics_handle,
					Item->Pic_nr * 256
				);
			}

			Free_item_data();

			/* Zero ? */
			if (Quantity)
			{
				/* No -> Get packet index */
				Slot_index = Item_slot->Number;
				if (Item_list->Scroll_bar_object)
				{
					Slot_index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
					 GET_METHOD, NULL);
				}

				/* Try to take the items */
				if (Take_item_from_Apres_pool(Slot_index, Quantity, &Drag_packet))
				{
					/* Success -> Drag items */
					Drag_Apres_item(Item_slot, Quantity);
				}
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : DLeft_Apres_item_slot_object
 * FUNCTION  : DLeft method of Apres item slot object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 13:44
 * LAST      : 16.08.95 15:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
DLeft_Apres_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_object *Item_list;
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	UNSHORT Slot_index;

	Item_slot = (struct Item_slot_object *) Object;
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Get packet index */
			Slot_index = Item_slot->Number;
			if (Item_list->Scroll_bar_object)
			{
				Slot_index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
					GET_METHOD, NULL);
			}

			/* Try to take all the items */
			if (Take_item_from_Apres_pool(Slot_index, Packet.Quantity, &Drag_packet))
			{
				/* Success -> Drag all items */
				Drag_Apres_item(Item_slot, Packet.Quantity);
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Apres_item_slot_object
 * FUNCTION  : Drop method of Apres item slot object.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 14:30
 * LAST      : 16.08.95 14:30
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Apres_item_slot_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_Apres_item((struct Item_slot_object *) Object,
		 P->Drag_drop_data_ptr);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drag_Apres_item
 * FUNCTION  : Drag Apres items.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 13:45
 * LAST      : 21.10.95 19:21
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 *             UNSHORT Quantity - Quantity that should be dragged.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drag_Apres_item(struct Item_slot_object *Item_slot, UNSHORT Quantity)
{
	/* Any taken ? */
	if (Quantity)
	{
		/* Yes -> Redraw item slot */
		Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

		/* Initialize item dragging */
		Init_drag
		(
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			MAX_ITEM_FRAMES
		);

		/* Build graphics */
		Build_item_graphics();

		/* Add HDOB */
		Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

		/* Change to normal mouse pointer */
		Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

		/* Pick mouse pointer */
		Push_mouse(&(Mouse_pointers[PICK_MPTR]));

		/* Store member index */
		Drag_member = Inventory_member;

		/* Enter drag & drop mode */
		Enter_drag_drop_mode
		(
			APRES_ITEM_DD_DATA_ID,
			Apres_item_drag_abort_handler,
			&(Item_slot->Object),
			NULL
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_Apres_item
 * FUNCTION  : Drop on Apres item.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 15:39
 * LAST      : 16.08.95 15:39
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to target item
 *              slot object.
 *             struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_Apres_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr)
{
	struct Item_list_object *Item_list;

	struct Item_slot_object *New_drag_source_item_slot;

	struct Item_packet *Target_packet;
	struct Item_packet Swap_packet;

	struct Item_data *Source_item;

	UNSHORT Target_slot_index;
	UNSHORT Quantity;

	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Claim target packet */
	Target_packet = Claim_slot_packet(Item_slot);

	/* Calculate target slot index */
	Target_slot_index = Item_slot->Number;
	if (Item_list->Scroll_bar_object)
	{
		Target_slot_index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Any item in target packet ? */
	if (!Packet_empty(Target_packet))
	{
		/* Yes -> Get source item data */
		Source_item = Get_item_data(&Drag_packet);

		/* Both packets contain same item / multiple item ? */
		if ((Drag_packet.Index == Target_packet->Index) &&
		 (Source_item->Flags & MULTIPLE))
		{
			/* Yes -> Is the target packet full ? */
			if (Target_packet->Quantity < ITEMS_PER_PACKET)
			{
				/* No -> Determine quantity that can be added to the
				 target packet */
				Quantity = min(Drag_packet.Quantity, ITEMS_PER_PACKET -
				 Target_packet->Quantity - Drag_packet.Quantity);

				/* Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_Apres_slot(Target_slot_index);

				/* Try to add source items to Apres pool */
				if (Put_item_in_Apres_pool(Target_slot_index, Quantity, &Drag_packet))
				{
					/* Success -> Redraw item slot */
					Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

					/* All items gone ? */
					if (!Packet_empty(&Drag_packet))
					{
						/* No -> Re-build graphics */
						Build_item_graphics();

						/* Change HDOB data */
						Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
					}
					else
					{
						/* Yes -> Exit dragging */
						Exit_drag();
					}
				}
			}
		}
		else
		{
			/* No -> Try to remove target items */
			if (Take_item_from_Apres_pool(Target_slot_index, Target_packet->Quantity,
			 &Swap_packet))
			{
				/* Success -> Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_Apres_slot(Target_slot_index);

				/* Try to add source items to Apres pool */
				if (Put_item_in_Apres_pool(Target_slot_index, Drag_packet.Quantity,
				 &Drag_packet))
				{
					/* Success -> Redraw item slot */
					Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

					/* Copy swapped packet to drag packet */
					memcpy(&Drag_packet, &Swap_packet, sizeof(struct Item_packet));

					/* Re-build graphics */
					Build_item_graphics();

					/* Change HDOB data */
					Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

					/* Set new drag source */
					New_drag_source_item_slot = Find_item_slot_object(Apres_item_list_object,
					 Target_slot_index);

					/* Store member index */
					Drag_member = Inventory_member;

					Enter_drag_drop_mode
					(
						APRES_ITEM_DD_DATA_ID,
						Apres_item_drag_abort_handler,
						&(New_drag_source_item_slot->Object),
						NULL
					);
				}
				else
				{
					/* Failure -> Try to swap back */
					if (!Put_item_in_Apres_pool
					(
						Target_slot_index,
						Swap_packet.Quantity,
						&Swap_packet
					))
					{
						/* Failure -> Error! */
						Error(ERROR_ITEM_NO_SWAP_BACK);
					}
				}
			}
		}
		Free_item_data();
	}
	else
	{
		/* No -> Move drag HDOB towards target slot */
		Move_drag_HDOB_towards_Apres_slot(Target_slot_index);

		/* Try to add items to Apres */
		if (Put_item_in_Apres_pool(Target_slot_index, Drag_packet.Quantity,
		 &Drag_packet))
		{
			/* Success -> Redraw item slot */
			Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

			/* All items gone ? */
			if (!Packet_empty(&Drag_packet))
			{
				/* No -> Re-build graphics */
				Build_item_graphics();

				/* Change HDOB data */
				Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
			}
			else
			{
				/* Yes -> Exit dragging */
				Exit_drag();
			}
		}
	}
	/* Free item packet */
	Free_slot_packet(Item_slot);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_item_drag_abort_handler
 * FUNCTION  : Apres item drag & drop abort handler.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 13:46
 * LAST      : 16.10.95 21:42
 * INPUTS    : struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_item_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr)
{
	BOOLEAN Result;

	/* Is Apres item ? */
	if (Drag_drop_data_ptr->Data_ID == APRES_ITEM_DD_DATA_ID)
	{
		/* Yes -> In the right member's inventory ? */
		if (Inventory_member != Drag_member)
		{
			/* Yes -> Drop the item back on the source slot */
			Drop_on_Apres_item
			(
				(struct Item_slot_object *) Drag_drop_data_ptr->Source_object,
				Drag_drop_data_ptr
			);
		}
		else
		{
			/* No -> Enter inventory */
			Result = Go_Inventory(Drag_member);
			if (Result)
			{
				/* Drop the item back on the source slot */
				Drop_on_Apres_item
				(
					(struct Item_slot_object *) Drag_drop_data_ptr->Source_object,
					Drag_drop_data_ptr
				);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Apres_item_PUM_evaluator
 * FUNCTION  : Evaluate Apres item pop-up menu.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 14:17
 * LAST      : 16.08.95 14:17
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Apres_item_PUM_evaluator(struct PUM *PUM)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	struct PUME *PUMES;

	PUMES = PUM->PUME_list;

	Descriptor = (struct Item_descriptor *) PUM->Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is the item undroppable ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes */
		PUMES[0].Flags |= PUME_BLOCKED;
		PUMES[0].Blocked_message_nr = 193;
	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Drop_Apres_item
 * FUNCTION  : Drop an Apres item (Apres item pop-up menu).
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 14:17
 * LAST      : 16.08.95 14:17
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Drop_Apres_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT Quantity;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* More than one item in the packet ? */
	Quantity = 1;
	if (Packet->Quantity > 1)
	{
		/* Yes -> Ask the player how many should be dropped */
		Quantity = Input_number_with_symbol(1, 0, (SILONG) Packet->Quantity,
		 System_text_ptrs[192], 16, 16, Item_graphics_handle,
		 Item_data->Pic_nr * 256);
	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Is quantity zero ? */
	if (Quantity)
	{
		/* No -> Are you sure ? */
		if (Boolean_requester(System_text_ptrs[62]))
		{
			/* Yes -> Drop */
			Drop_Apres_items(PUM_source_object_handle, Quantity);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Examine_Apres_item
 * FUNCTION  : Examine an Apres item (Apres item pop-up menu).
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 14:17
 * LAST      : 11.09.95 15:24
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Examine_Apres_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *)
	 (MEM_Claim_pointer(Descriptor->Slots_handle) + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Examine item */
	Examine_item(Packet);

	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Apres_items
 * FUNCTION  : Drop Apres pool items.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 14:18
 * LAST      : 06.10.95 16:03
 * INPUTS    : UNSHORT Item_slot_handle - Handle of item slot object.
 *             UNSHORT Quantity - Number of items that should be dropped.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only works for item lists belonging
 *              to the Apres pool.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_Apres_items(UNSHORT Item_slot_handle, UNSHORT Quantity)
{
	struct Item_list_object *Item_list;
	struct Item_slot_object *Item_slot;
	struct Item_packet Target_packet;
	UNSHORT Slot_index;

	Item_slot = (struct Item_slot_object *) Get_object_data(Item_slot_handle);

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get packet index */
	Slot_index = Item_slot->Number;
	if (Item_list->Scroll_bar_object)
	{
		Slot_index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Clear "target" packet */
	Clear_packet(&Target_packet);

	/* Try to take items */
	Take_item_from_Apres_pool
	(
		Slot_index,
		Quantity,
		&Target_packet
	);

	/* Redraw object */
	Execute_method(Item_slot_handle, DRAW_METHOD, NULL);

	/* Were any items taken ? */
	if (Target_packet.Quantity)
	{
		/* Yes -> Show dropping */
		Show_dropping_items
		(
			Item_slot,
			&Target_packet,
			(UNSHORT) Target_packet.Quantity
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Take_item_from_Apres_pool
 * FUNCTION  : Take an item out of the Apres pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 13:54
 * LAST      : 16.08.95 13:54
 * INPUTS    : UNSHORT Slot_index - Slot index (1...Nr_Apres_items).
 *             UNSHORT Quantity - Quantity that must be removed.
 *             struct Item_packet *Target_packet - Target packet.
 * RESULT    : BOOLEAN : Was the item taken?
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Take_item_from_Apres_pool(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Target_packet)
{
	struct Item_packet *Apres_pool;
	struct Item_packet *Source_packet;

	/* Get source packet data */
	Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);
	Source_packet = &(Apres_pool[Slot_index - 1]);

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> How many items should be removed ? */
		if (Source_packet->Quantity < Quantity)
			Quantity = Source_packet->Quantity;

		/* Was a target packet given ? */
		if (Target_packet)
		{
			/* Yes -> Put item in target packet */
			memcpy(Target_packet, Source_packet, sizeof(struct Item_packet));
			Target_packet->Quantity = Quantity;
		}

		/* Reduce quantity */
		Source_packet->Quantity -= Quantity;

		/* Any left ? */
		if (!Source_packet->Quantity)
		{
			/* No -> Destroy packet */
			Clear_packet(Source_packet);
		}
	}

	MEM_Free_pointer(Apres_pool_handle);

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_item_in_Apres_pool
 * FUNCTION  : Put an item in the Apres pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 20:48
 * LAST      : 15.08.95 20:48
 * INPUTS    : UNSHORT Slot_index - Slot index (1...Nr_Apres_items) /
 *              0 = auto-move.
 *             UNSHORT Quantity - Quantity that must be added.
 *             struct Item_packet *Target_packet - Source packet.
 * RESULT    : BOOLEAN : Was an item put?
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Put_item_in_Apres_pool(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Source_packet)
{
	struct Item_packet *Apres_pool;
	struct Item_packet *Target_packet;

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Auto-move to slot ? */
		if (Slot_index)
		{
			/* No ->  Get target packet */
			Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);
			Target_packet = &(Apres_pool[Slot_index - 1]);

			/* Does target slot contain the same item(s) ? */
			if (Source_packet->Index == Target_packet->Index)
			{
				/* Yes -> Add items to target slot */
				Target_packet->Quantity += Quantity;
			}
			else
			{
				/* No -> Move items to target slot */
				memcpy
				(
					(UNBYTE *) Target_packet,
					(UNBYTE *) Source_packet,
					sizeof(struct Item_packet)
				);
				Target_packet->Quantity = Quantity;
			}

			/* Remove items from source packet */
			Source_packet->Quantity -= Quantity;

			MEM_Free_pointer(Apres_pool_handle);

			/* Anything left in source packet ? */
			if (!Source_packet->Quantity)
			{
				/* No -> Destroy packet */
				Clear_packet(Source_packet);
			}
		}
		else
		{
			/* Yes -> Auto-move to Apres pool */
			Auto_move_packet_to_Apres_pool(Source_packet);
		}
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Auto_move_packet_to_Apres_pool
 * FUNCTION  : Automatically move an item packet to the Apres pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 20:43
 * LAST      : 15.08.95 20:43
 * INPUTS    : struct Item_packet *Source_packet - Source packet.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will remove items from the source packet, and
 *              destroy the source packet if the quantity reaches zero.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Auto_move_packet_to_Apres_pool(struct Item_packet *Source_packet)
{
	struct Item_packet *Apres_pool;
	struct Item_packet *Target_packet;
	struct Item_data *Item_data;
	UNSHORT Fit;
	UNSHORT i;

	Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Is this a multiple item ? */
		if (Item_data->Flags & MULTIPLE)
		{
			/* Yes -> Search for other slots with this item */
			for (i=0;i<Nr_Apres_items;i++)
			{
				Target_packet = &(Apres_pool[i]);

				/* Same item / not full ? */
				if ((Target_packet->Index == Source_packet->Index) &&
				 (Target_packet->Quantity < 99))
				{
					/* Calculate how many items will fit in the slot */
					Fit = 99 - Target_packet->Quantity;

					/* Enough ? */
					if (Fit > Source_packet->Quantity)
					{
						/* Yes -> Clip */
						Fit = Source_packet->Quantity;
					}

					/* Add items to target packet */
					Target_packet->Quantity += Fit;

					/* Remove items from source packet */
					Source_packet->Quantity -= Fit;

					/* Anything left ? */
					if (!Source_packet->Quantity)
					{
						/* No -> Exit */
						break;
					}
				}
			}

			/* Anything left in source packet ? */
			if (Source_packet->Quantity)
			{
				/* Yes -> Find a free slot */
				for (i=0;i<Nr_Apres_items;i++)
				{
					Target_packet = &(Apres_pool[i]);

					/* Free ? */
					if (Packet_empty(Target_packet))
					{
						/* Yes -> Put item in target packet */
						memcpy
						(
							(UNBYTE *) Target_packet,
							(UNBYTE *) Source_packet,
							sizeof(struct Item_packet)
						);

						/* Remove items from source packet */
						Source_packet->Quantity = 0;

						break;
					}
				}
			}
		}
		else
		{
			/* No -> Find a free slot */
			for (i=0;i<Nr_Apres_items;i++)
			{
				Target_packet = &(Apres_pool[i]);

				/* Free ? */
				if (Packet_empty(Target_packet))
				{
					/* Yes -> Put item in target packet */
					memcpy
					(
						(UNBYTE *) Target_packet,
						(UNBYTE *) Source_packet,
						sizeof(struct Item_packet)
					);
					Target_packet->Quantity = 1;

					/* Remove item from source packet */
					Source_packet->Quantity -= 1;

					/* Exit if all items have been put away */
					if (!Source_packet->Quantity)
						break;
				}
			}
		}
		Free_item_data();
	}

	MEM_Free_pointer(Apres_pool_handle);

	/* Anything left in source packet ? */
	if (!Source_packet->Quantity)
	{
		/* No -> Destroy packet */
		Clear_packet(Source_packet);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_drag_HDOB_towards_Apres_slot
 * FUNCTION  : Move drag HDOB towards a slot of the Apres combat screen.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 20:42
 * LAST      : 15.08.95 20:42
 * INPUTS    : UNSHORT Target_slot_index (1...Nr_Apres_items).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_drag_HDOB_towards_Apres_slot(UNSHORT Target_slot_index)
{
	struct Item_slot_object *Target_item_slot;

	/* Find corresponding Apres item slot */
	Target_item_slot = Find_item_slot_object(Apres_item_list_object,
	 Target_slot_index);

	/* Do it */
	Move_drag_HDOB_towards_object(&(Target_item_slot->Object));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Apres_pool
 * FUNCTION  : Init Apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 10:46
 * LAST      : 11.10.95 16:43
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Apres pool was initialized successfully.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_Apres_pool(void)
{
	BOOLEAN Result = FALSE;

	/* Is an Apres pool already initialised ? */
	if (!Apres_pool_handle)
	{
		/* No -> Clear counter */
		Nr_Apres_items = 0;

		/* Clear gold and food */
		Apres_gold_coins		= 0;
		Apres_food_rations	= 0;

		/* Allocate Apres pool memory */
		Apres_pool_handle = MEM_Allocate_memory(MAX_APRES_ITEMS *
		 sizeof(struct Item_packet));

		/* Success ? */
		if (Apres_pool_handle)
		{
			/* Yes -> Clear Apres pool */
			MEM_Clear_memory(Apres_pool_handle);

			Result = TRUE;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Apres_pool
 * FUNCTION  : Exit Apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:44
 * LAST      : 11.10.95 16:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Apres_pool(void)
{
	/* Clear counter */
	Nr_Apres_items = 0;

	/* Clear gold and food */
	Apres_gold_coins		= 0;
	Apres_food_rations	= 0;

	/* Is an Apres pool already initialised ? */
	if (Apres_pool_handle)
	{
		/* Yes -> Free Apres pool memory */
		MEM_Free_memory(Apres_pool_handle);

		/* Clear handle */
		Apres_pool_handle = NULL;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Store_item_in_Apres_pool
 * FUNCTION  : Store item in Apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:15
 * LAST      : 16.08.95 14:33
 * INPUTS    : struct Item_packet *Source_packet - Pointer to item packet.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Store_item_in_Apres_pool(struct Item_packet *Source_packet)
{
	struct Item_packet *Apres_pool;

	/* Is there room in the Apres pool ? */
	if (Nr_Apres_items < MAX_APRES_ITEMS)
	{
		/* Yes -> Store item in Apres pool */
		Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

		memcpy((UNBYTE *) &Apres_pool[Nr_Apres_items], Source_packet,
		 sizeof(struct Item_packet));

		MEM_Free_pointer(Apres_pool_handle);

		/* Count up */
		Nr_Apres_items++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_gold_in_Apres_pool
 * FUNCTION  : Put a number of gold coins in the Apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 20:27
 * LAST      : 11.05.95 20:27
 * INPUTS    : UNSHORT Nr_gold_coins - Number of gold coins.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_gold_in_Apres_pool(UNSHORT Nr_gold_coins)
{
	/* Add up */
	Apres_gold_coins += (UNLONG) Nr_gold_coins;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_food_in_Apres_pool
 * FUNCTION  : Put a number of food rations in the Apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 20:28
 * LAST      : 11.05.95 20:28
 * INPUTS    : UNSHORT Nr_food_rations - Number of food rations.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_food_in_Apres_pool(UNSHORT Nr_food_rations)
{
	/* Add up */
	Apres_food_rations += (UNLONG) Nr_food_rations;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pool_Apres_pool
 * FUNCTION  : Try to optimise the Apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 14:01
 * LAST      : 12.10.95 17:13
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pool_Apres_pool(void)
{
	struct Item_packet *Apres_pool;

	Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

	/* Optimise Apres pool */
	Nr_Apres_items = Optimise_inventory(Apres_pool, Nr_Apres_items);

	MEM_Free_pointer(Apres_pool_handle);
}

