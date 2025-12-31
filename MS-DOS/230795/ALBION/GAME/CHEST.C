/************
 * NAME     : CHEST.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : CHEST.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <HDOB.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAME.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <ITEMLIST.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <CHEST.H>
#include <EVELOGIC.H>
#include <BUTTONS.H>
#include <XFTYPES.H>
#include <GAMETEXT.H>
#include <DOOR.H>
#include <GOLDFOOD.H>
#include <STATAREA.H>

/* defines */

/* Chest flags */
#define CHEST_OR_JUNKPILE		(1 << 0)
#define DONT_SAVE_CHEST_FLAG 	(1 << 1)

/* structure definitions */

/* prototypes */

/* Closed chest module functions */
void Closed_chest_ModInit(void);
void Closed_chest_ModExit(void);
void Closed_chest_DisInit(void);
void Closed_chest_DisExit(void);
void Closed_chest_MainLoop(void);

void Exit_Chest(struct Button_object *Button);

/* Closed chest methods */
UNLONG Init_Closed_chest_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Closed_chest_object(struct Object *Object, union Method_parms *P);

/* Open chest module functions */
void Open_chest_ModInit(void);
void Open_chest_ModExit(void);
void Open_chest_DisInit(void);
void Open_chest_DisExit(void);
void Open_chest_MainLoop(void);

/* Open chest methods */
UNLONG Init_Open_chest_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Open_chest_object(struct Object *Object, union Method_parms *P);

/* Chest item slot methods (replacing normal item slot methods) */
UNLONG Left_Chest_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG DLeft_Chest_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Drop_Chest_item_slot_object(struct Object *Object,
 union Method_parms *P);

/* Chest item slot support functions */
void Drag_chest_item(struct Item_slot_object *Item_slot, UNSHORT Quantity);
void Drop_on_chest_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr);
void Chest_item_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr);

/* Chest item pop-up menu actions */
void Chest_item_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_chest_item(UNLONG Data);
void PUM_Examine_chest_item(UNLONG Data);

void Drop_chest_items(UNSHORT Item_slot_handle, UNSHORT Quantity);

BOOLEAN Take_item_from_chest(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Target_packet);
BOOLEAN Do_take_item_from_chest(struct Event_action *Action);
BOOLEAN Put_item_in_chest(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Source_packet);
BOOLEAN Do_put_item_in_chest(struct Event_action *Action);

void Auto_move_packet_to_chest(struct Item_packet *Source_packet);

void Move_drag_HDOB_towards_chest_slot(UNSHORT Target_slot_index);

/* global variables */

/* Chest parameters */
static UNSHORT Chest_locked_percentage;
static UNSHORT Chest_key_index;
static UNSHORT Chest_flags;
static UNSHORT Chest_first_text_block_nr;
static UNSHORT Chest_opened_text_block_nr;
static UNSHORT Current_chest_index;
static BOOLEAN Chest_has_trap_flag;

static UNSHORT Chest_status;
static UNSHORT Chest_lock_status;

static UNSHORT Chest_object;
static UNSHORT Chest_item_list_object;

MEM_HANDLE Chest_data_handle;
MEM_HANDLE Small_picture_handle;

/* Modules */
static struct Module Closed_chest_Mod = {
	LOCAL_MOD, SCREEN_MOD, CHEST_SCREEN,
	Closed_chest_MainLoop,
	Closed_chest_ModInit,
	Closed_chest_ModExit,
	Closed_chest_DisInit,
	Closed_chest_DisExit,
	NULL
};

static struct Module Open_chest_Mod = {
	LOCAL_MOD, SCREEN_MOD, CHEST_SCREEN,
	Open_chest_MainLoop,
	Open_chest_ModInit,
	Open_chest_ModExit,
	Open_chest_DisInit,
	Open_chest_DisExit,
	NULL
};

/* Closed chest method list */
static struct Method Closed_chest_methods[] = {
	{ INIT_METHOD, Init_Closed_chest_object },
	{ DRAW_METHOD, Draw_Closed_chest_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

/* Closed chest object class */
static struct Object_class Closed_chest_Class = {
	0, sizeof(struct Object),
	&Closed_chest_methods[0]
};

/* Open chest method list */
static struct Method Open_chest_methods[] = {
	{ INIT_METHOD, Init_Open_chest_object },
	{ DRAW_METHOD, Draw_Open_chest_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

/* Open chest object class */
static struct Object_class Open_chest_Class = {
	0, sizeof(struct Object),
	&Open_chest_methods[0]
};

/* Chest item slot method list */
static struct Method Chest_item_slot_methods[] = {
	{ INIT_METHOD, Init_Item_slot_object },
	{ DRAW_METHOD, Draw_Item_slot_object },
	{ UPDATE_METHOD, Update_Item_slot_object },
	{ FEEDBACK_METHOD, Feedback_Item_slot_object },
	{ HIGHLIGHT_METHOD, Highlight_Item_slot_object },
	{ POP_UP_METHOD, Pop_up_Item_slot_object },
	{ HELP_METHOD, Help_Item_slot_object },
	{ LEFT_METHOD, Left_Chest_item_slot_object },
	{ DLEFT_METHOD, DLeft_Chest_item_slot_object },
	{ RIGHT_METHOD, Right_Item_slot_object },
	{ TOUCHED_METHOD, Touch_Item_slot_object },
	{ DROP_METHOD, Drop_Chest_item_slot_object },
	{ INQUIRE_DROP_METHOD, Inquire_drop_item_slot_object },
	{ 0, NULL}
};

/* Chest item slot object class */
static struct Object_class Chest_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Chest_item_slot_methods[0]
};

/* Chest item pop-up menu entry list */
static struct PUME Chest_item_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 15, PUM_Drop_chest_item},
	{PUME_AUTO_CLOSE, 0, 17, PUM_Examine_chest_item},
};

/* Chest item pop-up menu */
static struct PUM Chest_item_PUM = {
	2,
	NULL,
	(UNLONG) &Selected_item,
	Chest_item_PUM_evaluator,
	Chest_item_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_Chest
 * FUNCTION  : Enter Chest screen.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.07.95 18:46
 * LAST      : 05.07.95 12:10
 * INPUTS    : UNSHORT Locked_percentage - Locked percentage (100 = cannot
 *              be picked).
 *             UNSHORT Key_index - Key index (0 = no key).
 *             UNSHORT Flags - Flags (see above).
 *             UNSHORT First_text_block_nr - Text block number (0...) / 255 =
 *              no text.
 *             UNSHORT Opened_text_block_nr - Text block number (0...) / 255 =
 *              no text.
 *             UNSHORT Chest_index - Chest index.
 *             BOOLEAN Has_trap - Does the chest contain a trap?
 * RESULT    : UNSHORT : Chest status (see CHEST.H).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Do_Chest(UNSHORT Locked_percentage, UNSHORT Key_index, UNSHORT Flags,
 UNSHORT First_text_block_nr, UNSHORT Opened_text_block_nr,
 UNSHORT Chest_index, BOOLEAN Has_trap)
{
	/* Store chest parameters */
	Chest_locked_percentage = Locked_percentage;
	Chest_key_index = Key_index;
	Chest_flags = Flags;
	Chest_first_text_block_nr = First_text_block_nr;
	Chest_opened_text_block_nr = Opened_text_block_nr;
	Current_chest_index = Chest_index;
	Chest_has_trap_flag = Has_trap;

	/* Exit current display */
	Exit_display();

	/* Is it a chest or a trashpile ? */
	if (Chest_flags & CHEST_OR_JUNKPILE)
	{
		/* Junkpile -> Set default chest status */
		Chest_status = EXIT_OPEN_CHEST_STATUS;

		/* Do open chest */
		Push_module(&Open_chest_Mod);
	}
	else
	{
		/* Chest -> Is this chest open ? */
		if (Read_bit_array(CHEST_BIT_ARRAY, Current_chest_index) ||
		 !Chest_locked_percentage)
		{
			/* Yes -> Set default chest status */
			Chest_status = EXIT_OPEN_CHEST_STATUS;

			/* Do open chest */
			Push_module(&Open_chest_Mod);
		}
		else
		{
			/* No -> Set default chest status */
			Chest_status = EXIT_CLOSED_CHEST_STATUS;

			/* Do closed chest */
			Push_module(&Closed_chest_Mod);
		}
	}

	/* Restore display */
	Init_display();

	return Chest_status;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Closed_chest_ModInit
 * FUNCTION  : Initialize Closed chest module.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 13.07.95 09:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Closed_chest_ModInit(void)
{
	struct Event_context *Context;
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Set inventory parameters */
	Inventory_char_handle = Active_char_handle;
	Inventory_member = PARTY_DATA.Active_member;

	/* Initialize inventory data */
	Result = Init_inventory_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Prepare inventory exit button */
	Exit_inventory_button_OID.Help_message_nr = 521;
	Exit_inventory_button_OID.Function = Exit_Chest;

	/* In Inventory */
	In_Inventory = TRUE;

	/* Initialize display */
	Closed_chest_DisInit();

	/* Any text ? */
	if (Chest_first_text_block_nr != 255)
	{
		/* Yes -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

		/* Print text in window */
		Do_text_file_window(Context->Text_handle, Chest_first_text_block_nr);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Closed_chest_ModExit
 * FUNCTION  : Exit Closed chest module.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 11.07.95 12:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Closed_chest_ModExit(void)
{
	Closed_chest_DisExit();
	Inventory_ModExit();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Closed_chest_DisInit
 * FUNCTION  : Initialize Closed chest display.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 11.07.95 12:33
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Closed_chest_DisInit(void)
{
	/* Add interface objects */
	Chest_object = Add_object(Earth_object, &Closed_chest_Class, NULL,
	 0, 0, INVENTORY_MIDDLE, Panel_Y);
	InvRight_object = Add_object(Earth_object, &InvRight_Class, NULL,
	 INVENTORY_MIDDLE, 0, Screen_width - INVENTORY_MIDDLE, Panel_Y);

	/* Draw */
	Execute_method(Chest_object, DRAW_METHOD, NULL);
	Execute_method(InvRight_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Closed_chest_DisExit
 * FUNCTION  : Exit Closed chest display.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:59
 * LAST      : 11.07.95 12:33
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Closed_chest_DisExit(void)
{
	/* Delete interface objects */
	Delete_object(Chest_object);
	Delete_object(InvRight_object);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Closed_chest_MainLoop
 * FUNCTION  : Main loop of Closed chest module.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 12:52
 * LAST      : 11.07.95 12:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Closed_chest_MainLoop(void)
{
	/* Has the lock status changed ? */
	if (Chest_lock_status != LOCK_STATUS_OK)
	{
		/* Yes -> Act depending on lock status */
		switch (Chest_lock_status)
		{
			/* Lock was picked or unlocked */
			case LOCK_STATUS_PICKED:
			case LOCK_STATUS_UNLOCKED:
			{
				/* Set new chest status */
				Chest_status = OPENED_CHEST_STATUS;

				/* Enter open chest screen */
				Pop_module();
				Push_module(&Open_chest_Mod);

				break;
			}
			/* Trap was activated */
			case LOCK_STATUS_TRAP_ACTIVATED:
			{
				/* Set new chest status */
				Chest_status = TRAP_CHEST_STATUS;

				/* Leave closed chest screen */
				Pop_module();

				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Chest
 * FUNCTION  : Leave the Chest screen.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 12:49
 * LAST      : 11.07.95 12:49
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Chest(struct Button_object *Button)
{
	/* Exit */
	Pop_module();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Closed_chest_object
 * FUNCTION  : Initialize method of Closed chest object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 12:35
 * LAST      : 11.07.95 12:35
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Closed_chest_object(struct Object *Object, union Method_parms *P)
{
	struct Lock_OID OID;

	/* Build lock OID */
	OID.Locked_percentage = Chest_locked_percentage;
	OID.Key_index = Chest_key_index;
	OID.Lock_has_trap = Chest_has_trap_flag;
	OID.Lock_result_ptr = &Chest_lock_status;

	/* Add lock object */
	Add_object(Object->Self, &Lock_Class, (UNBYTE *) &OID, 50, 50, 32, 32);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Closed_chest_object
 * FUNCTION  : Draw method of Closed chest object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 12:34
 * LAST      : 11.07.95 12:34
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Closed_chest_object(struct Object *Object, union Method_parms *P)
{
	UNBYTE *Ptr;

	/* Clear left inventory */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Draw closed chest picture */
/*	Ptr = MEM_Claim_pointer(Small_picture_handle);
	Put_unmasked_block(&Main_OPM, (Object->Rect.width - SMALL_PICTURE_WIDTH) /
	 2, (Object->Rect.height - SMALL_PICTURE_HEIGHT) / 2, SMALL_PICTURE_WIDTH,
	 SMALL_PICTURE_HEIGHT, Ptr);
	MEM_Free_pointer(Small_picture_handle); */

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_chest_ModInit
 * FUNCTION  : Initialize Open chest module.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 12.07.95 18:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Open_chest_ModInit(void)
{
	struct Event_context *Context;
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Set inventory parameters */
	Inventory_char_handle = Active_char_handle;
	Inventory_member = PARTY_DATA.Active_member;

	/* Initialize inventory data */
	Result = Init_inventory_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Prepare inventory exit button */
	Exit_inventory_button_OID.Help_message_nr = 514;
	Exit_inventory_button_OID.Function = Exit_Chest;

	/* In Inventory */
	In_Inventory = TRUE;

	/* Load chest data */
	Chest_data_handle = Load_subfile(CHEST_DATA, Current_chest_index);
	if (!Chest_data_handle)
	{
		Error(ERROR_FILE_LOAD);

		Pop_module();

		Exit_program();

		return;
	}

	/* Initialize display */
	Open_chest_DisInit();

	/* Any text on opening ? */
	if (Chest_opened_text_block_nr != 255)
	{
		/* Yes -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

		/* Print text in window */
		Do_text_file_window(Context->Text_handle, Chest_opened_text_block_nr);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_chest_ModExit
 * FUNCTION  : Exit Open chest module.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 12.07.95 10:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Open_chest_ModExit(void)
{
	Open_chest_DisExit();

	/* Save chest ? */
	if (Chest_flags & DONT_SAVE_CHEST_FLAG)
	{
		/* No -> Destroy (!) chest data */
		MEM_Kill_memory(Chest_data_handle);
	}
	else
	{
		/* Yes -> Save chest data */
		Save_subfile(Chest_data_handle, CHEST_DATA, Current_chest_index);

		/* Free chest data */
		MEM_Free_memory(Chest_data_handle);
	}

	Inventory_ModExit();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_chest_DisInit
 * FUNCTION  : Initialize Open chest display.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 04.07.95 18:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Open_chest_DisInit(void)
{
	/* Add interface objects */
	Chest_object = Add_object(Earth_object, &Open_chest_Class, NULL,
	 0, 0, INVENTORY_MIDDLE, Panel_Y);
	InvRight_object = Add_object(Earth_object, &InvRight_Class, NULL,
	 INVENTORY_MIDDLE, 0, Screen_width - INVENTORY_MIDDLE, Panel_Y);

	/* Draw */
	Execute_method(Chest_object, DRAW_METHOD, NULL);
	Execute_method(InvRight_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_chest_DisExit
 * FUNCTION  : Exit Open chest display.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:59
 * LAST      : 05.07.95 18:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Open_chest_DisExit(void)
{
	/* Delete interface objects */
	Delete_object(Chest_object);
	Delete_object(InvRight_object);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_chest_MainLoop
 * FUNCTION  : Main loop of Open chest module.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 13:04
 * LAST      : 12.07.95 11:41
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Open_chest_MainLoop(void)
{
	struct Chest_data *Chest;
	BOOLEAN Chest_empty;

	/* Is the chest empty ? */
	Chest = (struct Chest_data *) MEM_Claim_pointer(Chest_data_handle);

	Chest_empty = Inventory_empty(&(Chest->Items[0]), ITEMS_PER_CHEST);

	if (Chest->Chest_gold || Chest->Chest_food)
		Chest_empty = FALSE;

	MEM_Free_pointer(Chest_data_handle);

	/* Well ? */
	if (Chest_empty)
	{
		/* Yes -> Junkpile or chest ? */
		if (Chest_flags & CHEST_OR_JUNKPILE)
		{
			/* Junkpile -> In drag & drop mode ? */
			if (!Drag_drop_mode)
			{
				/* No -> Set chest status */
				Chest_status = JUNKPILE_EMPTY_STATUS;

				/* Leave open chest screen */
				Pop_module();
			}
		}
		else
		{
			/* Chest -> Change picture */
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Open_chest_object
 * FUNCTION  : Initialize method of Open chest object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 15:01
 * LAST      : 23.07.95 18:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Open_chest_object(struct Object *Object, union Method_parms *P)
{
	struct Chest_data *Chest;
	struct Item_list_OID Item_list_OID;
	struct Gold_food_OID Gold_food_OID;
	SISHORT X;
	UNSHORT Width, Height;

	/* Calculate chest item list dimensions */
	Width = (6 * ITEM_SLOT_OUTER_WIDTH) - ITEM_SLOT_BETWEEN_X + 2;
	Height = (4 * ITEM_SLOT_OUTER_HEIGHT) + 2;

	/* Calculate chest item list X-coordinate */
	X = (Object->Rect.width - Width) / 2;

	/* Add chest item list */
	Item_list_OID.Type = NO_CHAR_INV_TYPE;
	Item_list_OID.Nr_items = ITEMS_PER_CHEST;
	Item_list_OID.Slots_width = 6;
	Item_list_OID.Slots_height = 4;
	Item_list_OID.Slots_handle = Chest_data_handle;
	Item_list_OID.Slots_offset = 0;
	Item_list_OID.Menu = &Chest_item_PUM;
	Item_list_OID.Item_slot_class_ptr = &Chest_item_slot_Class;

	Chest_item_list_object = Add_object(Object->Self, &Item_list_Class,
	 (UNBYTE *) &Item_list_OID, X, Panel_Y - Height - 30, Width, Height);

	/* Get chest data */
	Chest = (struct Chest_data *) MEM_Claim_pointer(Chest_data_handle);

	/* Is this a chest or a junkpile with gold ? */
	if (!(Chest_flags & CHEST_OR_JUNKPILE) || (Chest->Chest_gold))
	{
		/* Yes -> Build gold object OID */
		Gold_food_OID.Type = CHEST_GOLD_TYPE;
		Gold_food_OID.Data_handle = Chest_data_handle;

		/* Add gold object */
		Add_object(Object->Self, &Gold_food_Class, (UNBYTE *) &Gold_food_OID,
		 X, Panel_Y - 25, 35, 20);
	}

	/* Is this a chest or a junkpile with food ? */
	if (!(Chest_flags & CHEST_OR_JUNKPILE) || (Chest->Chest_food))
	{
		/* Yes -> Build food object OID */
		Gold_food_OID.Type = CHEST_FOOD_TYPE;

		/* Add food object */
		Add_object(Object->Self, &Gold_food_Class, (UNBYTE *) &Gold_food_OID,
		 X + Width - 35, Panel_Y - 25, 35, 20);
	}

	MEM_Free_pointer(Chest_data_handle);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Open_chest_object
 * FUNCTION  : Draw method of Open chest object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 15:00
 * LAST      : 11.07.95 18:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Open_chest_object(struct Object *Object, union Method_parms *P)
{
	UNBYTE *Ptr;

	/* Clear left inventory */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Draw chest picture */
/*	Ptr = MEM_Claim_pointer(Small_picture_handle);
	Put_unmasked_block(&Main_OPM, (Object->Rect.width - SMALL_PICTURE_WIDTH) /
	 2, (Object->Rect.height - SMALL_PICTURE_HEIGHT) / 2, SMALL_PICTURE_WIDTH,
	 SMALL_PICTURE_HEIGHT, Ptr);
	MEM_Free_pointer(Small_picture_handle); */

	/* Print chest description */
	Print_centered_line_string(&Main_OPM, 4, 1, INVENTORY_MIDDLE - 8,
	 System_text_ptrs[530]);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Chest_item_slot_object
 * FUNCTION  : Left method of Chest item slot object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:56
 * LAST      : 05.07.95 16:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Chest_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item;
	UNSHORT Quantity;

	Item_slot = (struct Item_slot_object *) Object;

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
				Quantity = (UNSHORT) Input_number_with_symbol(1, 0,
				 (SILONG) Packet.Quantity, System_text_ptrs[66], 16, 16,
				 Item_graphics_handle, Item->Pic_nr * 256);
			}

			Free_item_data();

			/* Zero ? */
			if (Quantity)
			{
				/* No -> Try to take the items */
				if (Take_item_from_chest(Item_slot->Number, Quantity,
				 &Drag_packet))
				{
					/* Success -> Drag items */
					Drag_chest_item(Item_slot, Quantity);
				}
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : DLeft_Chest_item_slot_object
 * FUNCTION  : DLeft method of Chest item slot object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:56
 * LAST      : 05.07.95 16:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
DLeft_Chest_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Try to take all the items */
			if (Take_item_from_chest(Item_slot->Number, Packet.Quantity,
			 &Drag_packet))
			{
				/* Success -> Drag all items */
				Drag_chest_item(Item_slot, Packet.Quantity);
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Chest_item_slot_object
 * FUNCTION  : Drop method of Chest item slot object.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:28
 * LAST      : 09.07.95 16:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function won't work properly when the chest items
 *              have a scroll-bar.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Chest_item_slot_object(struct Object *Object, union Method_parms *P)
{
	/* Junkpile or chest ? */
	if (Chest_flags & CHEST_OR_JUNKPILE)
	{
		/* Junkpile -> Print message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[516]);
	}
	else
	{
		/* Chest -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Drop */
			Drop_on_chest_item((struct Item_slot_object *) Object,
			 P->Drag_drop_data_ptr);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drag_chest_item
 * FUNCTION  : Drag chest items.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:49
 * LAST      : 07.07.95 11:50
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 *             UNSHORT Quantity - Quantity that should be dragged.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drag_chest_item(struct Item_slot_object *Item_slot, UNSHORT Quantity)
{
	/* Any taken ? */
	if (Quantity)
	{
		/* Yes -> Redraw item slot */
		Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

		/* Initialize item dragging */
		Init_drag(ITEM_SLOT_INNER_WIDTH, ITEM_SLOT_INNER_HEIGHT,
		 MAX_ITEM_FRAMES);

		/* Build graphics */
		Build_item_graphics();

		/* Add HDOB */
		Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

		/* Pick mouse pointer */
		Change_mouse(&(Mouse_pointers[PICK_MPTR]));

		/* Enter drag & drop mode */
		Enter_drag_drop_mode(CHEST_ITEM_DD_DATA_ID,
		 Chest_item_drag_abort_handler, &(Item_slot->Object), NULL);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_chest_item
 * FUNCTION  : Drop on chest item.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:55
 * LAST      : 06.07.95 16:52
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
Drop_on_chest_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr)
{
	struct Item_slot_object *New_drag_source_item_slot;

	struct Item_packet *Target_packet;
	struct Item_packet Swap_packet;

	struct Item_data *Source_item;

	UNSHORT Target_slot_index;
	UNSHORT Quantity;

	/* Claim target packet */
	Target_packet = Claim_slot_packet(Item_slot);

	/* Calculate target slot index */
	Target_slot_index = Item_slot->Number;

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
				Move_drag_HDOB_towards_chest_slot(Target_slot_index);

				/* Try to add source items to chest */
				if (Put_item_in_chest(Target_slot_index, Quantity, &Drag_packet))
				{
					/* Success -> Redraw item slot */
					Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

					/* All items gone ? */
					if (!Packet_empty(&Drag_packet))
					{
						/* No -> Re-build graphics */
						Build_item_graphics();
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
			if (Take_item_from_chest(Target_slot_index, Target_packet->Quantity,
			 &Swap_packet))
			{
				/* Success -> Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_chest_slot(Target_slot_index);

				/* Try to add source items to chest */
				if (Put_item_in_chest(Target_slot_index, Drag_packet.Quantity,
				 &Drag_packet))
				{
					/* Success -> Redraw item slot */
					Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

					/* Copy swapped packet to drag packet */
					memcpy(&Drag_packet, &Swap_packet, sizeof(struct Item_packet));

					/* Re-build graphics */
					Build_item_graphics();

					/* Set new drag source */
					New_drag_source_item_slot = Find_item_slot_object(Chest_item_list_object,
					 Target_slot_index);

					Enter_drag_drop_mode(CHEST_ITEM_DD_DATA_ID,
					 Chest_item_drag_abort_handler,
					 &(New_drag_source_item_slot->Object), NULL);
				}
				else
				{
					/* Failure -> Try to swap back */
					if (!Put_item_in_chest(Target_slot_index,
					 Swap_packet.Quantity, &Swap_packet))
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
		Move_drag_HDOB_towards_chest_slot(Target_slot_index);

		/* Try to add items to chest */
		if (Put_item_in_chest(Target_slot_index, Drag_packet.Quantity,
		 &Drag_packet))
		{
			/* Success -> Redraw item slot */
			Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

			/* All items gone ? */
			if (!Packet_empty(&Drag_packet))
			{
				/* No -> Re-build graphics */
				Build_item_graphics();
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
 * NAME      : Chest_item_drag_abort_handler
 * FUNCTION  : Chest item drag & drop abort handler.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 11:03
 * LAST      : 07.07.95 11:03
 * INPUTS    : struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Chest_item_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr)
{
	if (Drag_drop_data_ptr->Data_ID == CHEST_ITEM_DD_DATA_ID)
	{
		/* Drop the item back on the source slot */
		Drop_on_chest_item((struct Item_slot_object *)
		 Drag_drop_data_ptr->Source_object, Drag_drop_data_ptr);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Chest_item_PUM_evaluator
 * FUNCTION  : Evaluate chest item pop-up menu.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:53
 * LAST      : 05.07.95 16:53
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Chest_item_PUM_evaluator(struct PUM *PUM)
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
 * NAME      : PUM_Drop_chest_item
 * FUNCTION  : Drop an chest item (chest item pop-up menu).
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 18:58
 * LAST      : 11.07.95 18:58
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Drop_chest_item(UNLONG Data)
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
			Drop_chest_items(PUM_source_object_handle, Quantity);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Examine_chest_item
 * FUNCTION  : Examine an chest item (chest item pop-up menu).
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Examine_chest_item(UNLONG Data)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_chest_items
 * FUNCTION  : Drop chest items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 18:57
 * LAST      : 11.07.95 18:57
 * INPUTS    : UNSHORT Item_slot_handle - Handle of item slot object.
 *             UNSHORT Quantity - Number of items that should be dropped.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only works for item lists belonging
 *              to chests.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_chest_items(UNSHORT Item_slot_handle, UNSHORT Quantity)
{
	struct HDOB HDOB;

	struct Item_data *Item_data;

	struct Item_list_object *Item_list;

	struct Item_slot_object *Item_slot;

	struct Item_packet Packet;

	SISHORT Slot_X;
	SISHORT Slot_Y;

	UNSHORT HDOB_nr;
	UNSHORT Slot_index;
	UNSHORT i;

	Item_slot = (struct Item_slot_object *) Get_object_data(Item_slot_handle);

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Get packet index */
	Slot_index = Item_slot->Number;
	if (Item_list->Scroll_bar_object)
	{
		Slot_index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get item data */
	Item_data = Get_item_data(&Packet);

	/* Get item slot coordinates (for HDOB) */
	Slot_X = Item_slot->Object.Rect.left;
	Slot_Y = Item_slot->Object.Rect.top;

	/* Initialize HDOB data */
	BASEMEM_FillMemByte((UNBYTE *) &HDOB, sizeof(struct HDOB), 0);

	HDOB.Draw_mode = HDOB_MASK;
	HDOB.X = Slot_X;
	HDOB.Y = Slot_Y;
	HDOB.Width = 16;
	HDOB.Height = 16;
	HDOB.Nr_frames = (UNSHORT) Item_data->Nr_frames;
	HDOB.Graphics_handle = Item_graphics_handle;
	HDOB.Graphics_offset = 256 * Item_data->Pic_nr;

	/* Add HDOB */
	HDOB_nr = Add_HDOB(&HDOB);

	for (i=0;i<Quantity;i++)
	{
		/* Try to take an item */
		if (Take_item_from_chest(Slot_index, 1, NULL))
		{
			/* Success -> Redraw object */
			Execute_method(Item_slot_handle, DRAW_METHOD, NULL);

			/* Update display */
			Update_display();
			Update_input();
			Switch_screens();

			/* Only show some dropping items */
			if (i < MAX_DROPPING_ITEMS)
			{
				/* Reset HDOB coordinates */
				Set_HDOB_position(HDOB_nr, Slot_X, Slot_Y);

				/* Show dropping item */
				Drop_HDOB(HDOB_nr);
			}
		}
		else
			break;
	}

	/* Remove HDOB */
	Delete_HDOB(HDOB_nr);

	Free_item_data();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Take_item_from_chest
 * FUNCTION  : Take an item out of a chest.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:51
 * LAST      : 05.07.95 16:51
 * INPUTS    : UNSHORT Slot_index - Slot index (1...ITEMS_PER_CHEST).
 *             UNSHORT Quantity - Quantity that must be removed.
 *             struct Item_packet *Target_packet - Target packet.
 * RESULT    : BOOLEAN : Was the item taken?
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Take_item_from_chest(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Target_packet)
{
	static struct Event_action Take_item_from_chest_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		0, 0, 0,
		Do_take_item_from_chest, NULL, NULL
	};

	struct Move_item_event_data Action_data;
	struct Character_data *Char;
	struct Item_packet *Source_packet;
	struct Item_data *Item_data;
	struct Chest_data *Chest;
	BOOLEAN Result = FALSE;

	/* Get source packet data */
	Chest = (struct Chest_data *) MEM_Claim_pointer(Chest_data_handle);
	Source_packet = &(Chest->Items[Slot_index - 1]);

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Build data for event action */
		Action_data.Handle = Chest_data_handle;
		Action_data.Slot_index = Slot_index;
		Action_data.Quantity = Quantity;
		Action_data.Packet = Target_packet;

		/* Build event action data */
		Take_item_from_chest_action.Actor_index = PARTY_DATA.Active_member;
		Take_item_from_chest_action.Action_value = Source_packet->Index;
		Take_item_from_chest_action.Action_extra = (UNSHORT) Item_data->Type;
		Take_item_from_chest_action.Action_data = &Action_data;

		/* Chest or junkpile ? */
		if (Chest_flags & CHEST_OR_JUNKPILE)
		{
			/* Junkpile */
			Take_item_from_chest_action.Action_type =
			 TAKE_ITEM_FROM_JUNKPILE_ACTION;
		}
		else
		{
			/* Chest */
			Take_item_from_chest_action.Action_type =
			 TAKE_ITEM_FROM_CHEST_ACTION;
		}

		/* Check events */
		Result = Perform_action(&Take_item_from_chest_action);

		Free_item_data();
	}
	MEM_Free_pointer(Chest_data_handle);

	return Result;
}

BOOLEAN
Do_take_item_from_chest(struct Event_action *Action)
{
	struct Move_item_event_data *Data;
	struct Item_packet *Source_packet;
	struct Chest_data *Chest;
	UNSHORT Quantity;

	/* Get action data */
	Data = (struct Move_item_event_data *) Action->Action_data;

	/* Get source packet data */
	Chest = (struct Chest_data *) MEM_Claim_pointer(Data->Handle);
	Source_packet = &(Chest->Items[Data->Slot_index - 1]);

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> How many items should be removed ? */
		Quantity = Data->Quantity;
		if (Source_packet->Quantity < Quantity)
			Quantity = Source_packet->Quantity;

		/* Was a target packet given ? */
		if (Data->Packet)
		{
			/* Yes -> Put item in target packet */
			memcpy(Data->Packet, Source_packet, sizeof(struct Item_packet));
			Data->Packet->Quantity = Data->Quantity;
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
	MEM_Free_pointer(Data->Handle);

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_item_in_chest
 * FUNCTION  : Put an item in a chest.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 16:52
 * LAST      : 05.07.95 16:52
 * INPUTS    : UNSHORT Slot_index - Slot index (1...33) / 0 = auto-move to
 *              backpack.
 *             UNSHORT Quantity - Quantity that must be added.
 *             struct Item_packet *Target_packet - Source packet.
 * RESULT    : BOOLEAN : Was an item put?
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Put_item_in_chest(UNSHORT Slot_index, UNSHORT Quantity,
 struct Item_packet *Source_packet)
{
	static struct Event_action Put_item_in_chest_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		STORE_ITEM_IN_CHEST_ACTION, 0, 0,
		Do_put_item_in_chest, NULL, NULL
	};

	struct Move_item_event_data Action_data;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Build data for event action */
		Action_data.Handle = Chest_data_handle;
		Action_data.Slot_index = Slot_index;
		Action_data.Quantity = Quantity;
		Action_data.Packet = Source_packet;

		/* Build event action data */
		Put_item_in_chest_action.Actor_index = PARTY_DATA.Active_member;
		Put_item_in_chest_action.Action_value = Source_packet->Index;
		Put_item_in_chest_action.Action_extra = (UNSHORT) Item_data->Type;
		Put_item_in_chest_action.Action_data = &Action_data;

		/* Check events */
		Result = Perform_action(&Put_item_in_chest_action);

		Free_item_data();
	}

	return Result;
}

BOOLEAN
Do_put_item_in_chest(struct Event_action *Action)
{
	struct Move_item_event_data *Data;
	struct Chest_data *Chest;
	struct Item_packet *Target_packet;

	/* Get action data */
	Data = (struct Move_item_event_data *) Action->Action_data;

	/* Anything in the source packet ? */
	if (!Packet_empty(Data->Packet))
	{
		/* Yes -> Auto-move to slot ? */
		if (Data->Slot_index)
		{
			/* No ->  Get target packet */
			Chest = (struct Chest_data *) MEM_Claim_pointer(Data->Handle);
			Target_packet = &(Chest->Items[Data->Slot_index - 1]);

			/* Does target slot contain the same item(s) ? */
			if (Data->Packet->Index == Target_packet->Index)
			{
				/* Yes -> Add items to target slot */
				Target_packet->Quantity += Data->Quantity;
			}
			else
			{
				/* No -> Move items to target slot */
				memcpy((UNBYTE *) Target_packet, (UNBYTE *) Data->Packet,
				 sizeof(struct Item_packet));
				Target_packet->Quantity = Data->Quantity;
			}

			/* Remove items from source packet */
			Data->Packet->Quantity -= Data->Quantity;

			MEM_Free_pointer(Data->Handle);

			/* Anything left in source packet ? */
			if (!Data->Packet->Quantity)
			{
				/* No -> Destroy packet */
				Clear_packet(Data->Packet);
			}
		}
		else
		{
			/* Yes -> Auto-move to chest */
			Auto_move_packet_to_chest(Data->Packet);
		}
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Auto_move_packet_to_chest
 * FUNCTION  : Automatically move an item packet to a chest.
 * FILE      : CHEST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 17:48
 * LAST      : 06.07.95 12:11
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
Auto_move_packet_to_chest(struct Item_packet *Source_packet)
{
	struct Item_packet *Target_packet;
	struct Item_data *Item_data;
	struct Chest_data *Chest;
	UNSHORT Fit;
	UNSHORT i;

	Chest = (struct Chest_data *) MEM_Claim_pointer(Chest_data_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Is this a multiple item ? */
		if (Item_data->Flags & MULTIPLE)
		{
			/* Yes -> Search for other slots with this item */
			for (i=0;i<ITEMS_PER_CHEST;i++)
			{
				Target_packet = &(Chest->Items[i]);

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
				for (i=0;i<ITEMS_PER_CHEST;i++)
				{
					Target_packet = &(Chest->Items[i]);

					/* Free ? */
					if (Packet_empty(Target_packet))
					{
						/* Yes -> Clear target packet */
						Clear_packet(Target_packet);

						/* Put items in target packet */
						Target_packet->Index = Source_packet->Index;
						Target_packet->Quantity = Source_packet->Quantity;

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
			for (i=0;i<ITEMS_PER_CHEST;i++)
			{
				Target_packet = &(Chest->Items[i]);

				/* Free ? */
				if (Packet_empty(Target_packet))
				{
					/* Yes -> Clear target packet */
					Clear_packet(Target_packet);

					/* Put item in target packet */
					Target_packet->Index = Source_packet->Index;
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

	MEM_Free_pointer(Chest_data_handle);

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
 * NAME      : Move_drag_HDOB_towards_chest_slot
 * FUNCTION  : Move drag HDOB towards a slot of a chest.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 17:11
 * LAST      : 07.07.95 11:59
 * INPUTS    : UNSHORT Target_slot_index (1...ITEMS_PER_CHEST).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_drag_HDOB_towards_chest_slot(UNSHORT Target_slot_index)
{
	struct Item_slot_object *Target_item_slot;

	/* Find corresponding chest item slot */
	Target_item_slot = Find_item_slot_object(Chest_item_list_object,
	 Target_slot_index);

	/* Do it */
	Move_drag_HDOB_towards_object(&(Target_item_slot->Object));
}

