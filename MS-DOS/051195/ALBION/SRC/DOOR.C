/************
 * NAME     : DOOR.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : DOOR.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <POPUP.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <DOOR.H>
#include <EVELOGIC.H>
#include <GAMETEXT.H>
#include <STATAREA.H>
#include <BUTTONS.H>
#include <TEXTWIN.H>
#include <SOUND.H>

/* defines */

#define DOOR_PICTURE		(18)

/* structure definitions */

/* Lock object */
struct Lock_object {
	struct Object Object;
	UNSHORT Locked_percentage;
	UNSHORT Key_index;
	BOOLEAN Lock_has_trap;
	UNSHORT *Lock_result_ptr;
};

/* prototypes */

/* Module functions */
void Door_ModInit(void);
void Door_ModExit(void);
void Door_DisInit(void);
void Door_DisExit(void);
void Door_MainLoop(void);

void Exit_Door(struct Button_object *Button);

/* Door methods */
UNLONG Init_Door_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Door_object(struct Object *Object, union Method_parms *P);
UNLONG Restore_Door_object(struct Object *Object, union Method_parms *P);

/* Lock methods */
UNLONG Init_Lock_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Lock_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Lock_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Lock_object(struct Object *Object, union Method_parms *P);
UNLONG Drop_Lock_object(struct Object *Object, union Method_parms *P);
UNLONG Inquire_drop_Lock_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Lock_object(struct Object *Object, union Method_parms *P);

/* Lock pop-up menu actions */
void PUM_Pick_lock(UNLONG Data);

/* global variables */

/* Door parameters */
static UNSHORT Door_locked_percentage;
static UNSHORT Door_key_index;
static UNSHORT Door_lock_type;
static UNSHORT Door_first_text_block_nr;
static UNSHORT Door_opened_text_block_nr;
static UNSHORT Current_door_index;
static BOOLEAN Door_has_trap_flag;

static UNSHORT Door_status;
static UNSHORT Door_lock_status;

//static UNSHORT Door_object;

/* Modules */
static struct Module Door_Mod = {
	LOCAL_MOD, SCREEN_MOD, DOOR_SCREEN,
	Door_MainLoop,
	Door_ModInit,
	Door_ModExit,
	Door_DisInit,
	Door_DisExit,
	NULL
};

/* Door method list */
static struct Method Door_methods[] = {
	{ INIT_METHOD,		Init_Door_object },
	{ DRAW_METHOD,		Draw_Door_object },
	{ RESTORE_METHOD,	Restore_Door_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Door object class */
static struct Object_class Door_Class = {
	0, sizeof(struct Object),
	&Door_methods[0]
};

/* Lock method list */
static struct Method Lock_methods[] = {
	{ INIT_METHOD,				Init_Lock_object },
	{ DRAW_METHOD,				Draw_Lock_object },
	{ FEEDBACK_METHOD,		Feedback_Lock_object },
	{ HELP_METHOD,				Help_Lock_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_Lock_object },
	{ DROP_METHOD,				Drop_Lock_object },
	{ POP_UP_METHOD,			Pop_up_Lock_object },
	{ RIGHT_METHOD,			Normal_rightclicked },
	{ TOUCHED_METHOD,			Normal_touched },
	{ 0, NULL}
};

/* Lock object class */
struct Object_class Lock_Class = {
	0, sizeof(struct Lock_object),
	&Lock_methods[0]
};

/* Lock pop-up menu entry list */
static struct PUME Lock_PUMEs[] = {
	{ PUME_AUTO_CLOSE, 0, 72, PUM_Pick_lock }
};

/* Lock pop-up menu */
static struct PUM Lock_PUM = {
	1,
	NULL,
	0,
	NULL,
	Lock_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_Door
 * FUNCTION  : Enter Door screen.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.07.95 14:31
 * LAST      : 10.07.95 14:31
 * INPUTS    : UNSHORT Locked_percentage - Locked percentage (100 = cannot
 *              be picked).
 *             UNSHORT Key_index - Key index (0 = no key).
 *             UNSHORT Lock_type - Lock type.
 *             UNSHORT First_text_block_nr - Text block number (0...) / 255 =
 *              no text.
 *             UNSHORT Opened_text_block_nr - Text block number (0...) / 255 =
 *              no text.
 *             UNSHORT Door_index - Door index.
 *             BOOLEAN Has_trap - Does the door contain a trap?
 * RESULT    : UNSHORT : Door status (see DOOR.H).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Do_Door(UNSHORT Locked_percentage, UNSHORT Key_index, UNSHORT Lock_type,
 UNSHORT First_text_block_nr, UNSHORT Opened_text_block_nr,
 UNSHORT Door_index, BOOLEAN Has_trap)
{
	/* Still in move mode ? */
	if (Move_mode_flag)
	{
		/* Yes -> Exit move mode */
		Pop_module();

		/* Clear flag again just to be sure */
		Move_mode_flag = FALSE;
	}

	/* Store door parameters */
	Door_locked_percentage		= Locked_percentage;
	Door_key_index					= Key_index;
	Door_lock_type					= Lock_type;
	Door_first_text_block_nr	= First_text_block_nr;
	Door_opened_text_block_nr	= Opened_text_block_nr;
	Current_door_index			= Door_index;
	Door_has_trap_flag			= Has_trap;

	/* Is this door open ? */
	if (((Current_door_index != 0xFFFF) &&
	 (Read_bit_array(DOOR_BIT_ARRAY, Current_door_index))) ||
	 !Door_locked_percentage)
	{
		/* Yes -> OK! */
		Door_status = IS_OPEN_DOOR_STATUS;
	}
	else
	{
		/* No -> Set default door status */
		Door_status = EXIT_DOOR_STATUS;

		/* Do door */
		Exit_display();
		Push_module(&Door_Mod);
		Init_display();
	}

	return Door_status;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Door_ModInit
 * FUNCTION  : Initialize Door module.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 12.08.95 15:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Door_ModInit(void)
{
	struct Event_context *Context;
	BOOLEAN Result;

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
	Exit_inventory_button_OID.Help_message_nr	= 521;
	Exit_inventory_button_OID.Function			= Exit_Door;

	/* Load small picture */
	Load_small_picture(DOOR_PICTURE);

	/* In Inventory */
	In_Inventory = TRUE;

	/* Initialize display */
	Door_DisInit();

	/* Any text ? */
	if (Door_first_text_block_nr != 255)
	{
		/* Yes -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

		/* Print text in window */
		Do_text_file_window(Context->Text_handle, Door_first_text_block_nr);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Door_ModExit
 * FUNCTION  : Exit Door module.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 11.08.95 19:47
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Door_ModExit(void)
{
	Door_DisExit();
	Inventory_ModExit();

	Destroy_small_picture();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Door_DisInit
 * FUNCTION  : Initialize Door display.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 10.07.95 15:18
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Door_DisInit(void)
{
	/* Add interface objects */
	InvLeft_object = Add_object(Earth_object, &Door_Class, NULL,
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
 * NAME      : Door_DisExit
 * FUNCTION  : Exit Door display.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:59
 * LAST      : 10.07.95 15:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Door_DisExit(void)
{
	/* Delete interface objects */
	Delete_object(InvLeft_object);
	Delete_object(InvRight_object);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Door_MainLoop
 * FUNCTION  : Main loop of Door module.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.95 12:59
 * LAST      : 21.10.95 14:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Door_MainLoop(void)
{
	struct Event_context *Context;

	/* Has the lock status changed ? */
	if (Door_lock_status != LOCK_STATUS_OK)
	{
		/* Yes -> Act depending on lock status */
		switch (Door_lock_status)
		{
			/* Lock was picked or unlocked */
			case LOCK_STATUS_PICKED:
			case LOCK_STATUS_UNLOCKED:
			{
				/* Set new door status */
				Door_status = OPENED_DOOR_STATUS;

				/* Any text on opening ? */
				if (Door_opened_text_block_nr != 255)
				{
					/* Yes -> Get current event context */
					Context = &(Event_context_stack[Event_context_stack_index]);

					/* Print text in window */
					Do_text_file_window(Context->Text_handle,
					 Door_opened_text_block_nr);
				}

				/* Leave door screen */
				Pop_module();

				break;
			}
			/* Trap was activated */
			case LOCK_STATUS_TRAP_ACTIVATED:
			{
				/* Set new door status */
				Door_status = TRAP_DOOR_STATUS;

				/* Leave door screen */
				Pop_module();

				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Door
 * FUNCTION  : Leave the door screen.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.07.95 15:30
 * LAST      : 10.07.95 15:30
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Door(struct Button_object *Button)
{
	Pop_module();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Door_object
 * FUNCTION  : Initialize method of Door object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 11:07
 * LAST      : 10.07.95 15:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Door_object(struct Object *Object, union Method_parms *P)
{
	struct Lock_OID OID;

	/* Build lock OID */
	OID.Locked_percentage	= Door_locked_percentage;
	OID.Key_index				= Door_key_index;
	OID.Lock_has_trap			= Door_has_trap_flag;
	OID.Lock_result_ptr		= &Door_lock_status;

	/* Add lock object */
	Add_object
	(
		Object->Self,
		&Lock_Class,
		(UNBYTE *) &OID,
		50,
		112,
		32,
		32
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Door_object
 * FUNCTION  : Draw method of Door object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 11:06
 * LAST      : 26.09.95 20:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Door_object(struct Object *Object, union Method_parms *P)
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

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_Door_object
 * FUNCTION  : Restore method of Door object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 20:52
 * LAST      : 26.09.95 20:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Restore_Door_object(struct Object *Object, union Method_parms *P)
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

	/* Restore clip area */
	memcpy(&(Main_OPM.clip), &Old, sizeof(struct BBRECT));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Lock_object
 * FUNCTION  : Init method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:20
 * LAST      : 11.07.95 12:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Lock_object(struct Object *Object, union Method_parms *P)
{
	struct Lock_object *Lock;
	struct Lock_OID *OID;

	Lock = (struct Lock_object *) Object;
	OID = (struct Lock_OID *) P;

	/* Copy data from OID */
	Lock->Locked_percentage	= OID->Locked_percentage;
	Lock->Key_index			= OID->Key_index;
	Lock->Lock_has_trap 		= OID->Lock_has_trap;
	Lock->Lock_result_ptr	= OID->Lock_result_ptr;

	/* Initialize lock status */
	*(Lock->Lock_result_ptr) = LOCK_STATUS_OK;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Lock_object
 * FUNCTION  : Draw method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:20
 * LAST      : 25.09.95 16:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Lock_object(struct Object *Object, union Method_parms *P)
{
	/* Erase object */
	Restore_object_background(Object);

	/* Draw surrounding box */
	Draw_high_border
	(
		Current_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Draw lock symbol */
	Put_masked_block
	(
		Current_OPM,
		Object->X + 6,
		Object->Y + 5,
		22,
		22,
		&(Lock_symbol[0])
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Lock_object
 * FUNCTION  : Feedback method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:20
 * LAST      : 25.09.95 16:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Lock_object(struct Object *Object, union Method_parms *P)
{
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

	/* Draw lock symbol */
	Put_masked_block
	(
		Current_OPM,
		Object->X + 6,
		Object->Y + 5,
		22,
		22,
		&(Lock_symbol[0])
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Lock_object
 * FUNCTION  : Help method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:20
 * LAST      : 10.07.95 17:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Lock_object(struct Object *Object, union Method_parms *P)
{
	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) System_text_ptrs[73]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Lock_object
 * FUNCTION  : Drop method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.07.95 17:54
 * LAST      : 21.10.95 14:20
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Lock_object(struct Object *Object, union Method_parms *P)
{
	struct Lock_object *Lock;
	struct Item_data *Source_item;

	Lock = (struct Lock_object *) Object;

	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Play sound effect */
		SOUND_Play_sound_effect(104, 100, 100, 0, 0);

		/* Get source item data */
		Source_item = Get_item_data(&Drag_packet);

		/* Is this the right item ? */
		if (Drag_packet.Index == Lock->Key_index)
		{
			/* Yes -> "Opened the door!" */
			Set_permanent_message_nr(526);

			/* Should this item be destroyed after use ? */
			if (Source_item->Flags & DESTROY_AFTER_USE)
			{
				/* Yes -> Remove one item */
				Drag_packet.Quantity -= 1;

				/* All items gone ? */
				if (Packet_empty(&Drag_packet))
				{
					/* Yes -> Exit dragging */
					Exit_drag();
					Pop_mouse();
				}
				else
				{
					/* No -> Abort drag & drop */
					Abort_drag_drop_mode();
				}
			}
			else
			{
				/* No -> Abort drag & drop */
				Abort_drag_drop_mode();
			}

			/* Open the lock */
			*(Lock->Lock_result_ptr) = LOCK_STATUS_UNLOCKED;
		}
		else
		{
			/* No -> Act depending on item type */
			switch (Source_item->Type)
			{
				/* Lockpick */
				case LOCKPICK_IT:
				{
					/* Yes -> Is the door fully locked ? */
					if (Lock->Locked_percentage >= 100)
					{
						/* Yes -> "Lock cannot be picked!" */
						Set_permanent_message_nr(523);
					}
					else
					{
						/* No -> "Yay!" */
						Set_permanent_message_nr(527);

						/* Open the lock */
						*(Lock->Lock_result_ptr) = LOCK_STATUS_PICKED;
					}

					/* Remove lockpick */
					Drag_packet.Quantity -= 1;

					/* All items gone ? */
					if (Packet_empty(&Drag_packet))
					{
						/* Yes -> Exit dragging */
						Exit_drag();
						Pop_mouse();
					}
					else
					{
						/* No -> Was picking successful ? */
						if (*(Lock->Lock_result_ptr) == LOCK_STATUS_PICKED)
						{
							/* Yes -> Abort drag & drop */
							Abort_drag_drop_mode();
						}
						else
						{
							/* No -> Re-build graphics */
							Build_item_graphics();

							/* Change HDOB data */
							Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
						}
					}
					break;
				}
				/* Key */
				case KEY_IT:
				{
					/* "Wrong key" */
					Set_permanent_message_nr(528);

					break;
				}
				/* Other types */
				default:
				{
					/* "Wrong item" */
					Set_permanent_message_nr(529);

					break;
				}
			}
		}
		Free_item_data();
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inquire_drop_Lock_object
 * FUNCTION  : Drop method of Lock object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.07.95 17:56
 * LAST      : 10.07.95 17:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Inquire_drop_Lock_object(struct Object *Object, union Method_parms *P)
{
	struct Drag_drop_data *Drag_drop_data_ptr;
	UNSHORT Data_ID;

	/* Get drag & drop data */
	Drag_drop_data_ptr = P->Drag_drop_data_ptr;

	/* Get drag & drop data ID */
	Data_ID = Drag_drop_data_ptr->Data_ID;

	/* Right data type ? */
	if ((Data_ID == BODY_ITEM_DD_DATA_ID) ||
	 (Data_ID == BACKPACK_ITEM_DD_DATA_ID))
	{
		/* Yes */
		return 1;
	}
	else
	{
		/* No */
		return 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Lock_object
 * FUNCTION  : Pop-up method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:20
 * LAST      : 27.01.95 15:20
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Pop_up_Lock_object(struct Object *Object, union Method_parms *P)
{
	/* Make sure PUM function can reference lock object */
	Lock_PUM.Data = (UNLONG) Object->Self;

	/* Call pop-up menu */
	Lock_PUM.Title = System_text_ptrs[71];
	Do_PUM(Object->X + 16, Object->Y + 8, &Lock_PUM);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Pick_lock
 * FUNCTION  : Pick the lock (lock pop-up menu).
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:24
 * LAST      : 01.08.95 18:53
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Pick_lock(UNLONG Data)
{
	struct Lock_object *Lock;
	BOOLEAN Picked_lock;
	UNSHORT Skill;

	Lock = (struct Lock_object *) Get_object_data((UNSHORT) Data);

	/* In cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes -> "Picked the lock!" */
		Set_permanent_message_nr(522);

		/* Open the lock */
		*(Lock->Lock_result_ptr) = LOCK_STATUS_PICKED;
	}
	else
	{
		/* No -> Is the door fully locked ? */
		if (Lock->Locked_percentage >= 100)
		{
			/* Yes -> "Lock cannot be picked!" */
			Set_permanent_message_nr(523);
		}
		else
		{
			/* No -> Get lock-picking skill of active character */
			Skill = Get_skill(Active_char_handle, LOCK_PICKING);

			/* Is above locked percentage ? */
			if (Skill >= Lock->Locked_percentage)
			{
				/* Yes -> Picked the lock */
				Picked_lock = TRUE;
			}
			else
			{
				/* No -> Transform skill */
				Skill = (Skill * (100 - Lock->Locked_percentage)) / 100;

				/* Probe transformed skill */
				Picked_lock = Probe(Skill, 100);
			}

			/* Picked the lock ? */
			if (Picked_lock)
			{
				/* Yes -> "Picked the lock!" */
				Set_permanent_message_nr(522);

				/* Open the lock */
				*(Lock->Lock_result_ptr) = LOCK_STATUS_PICKED;
			}
			else
			{
				/* No -> Any trap ? */
				if (Lock->Lock_has_trap)
				{
					/* Yes -> Probe dexterity */
					if (Probe_attribute(Active_char_handle, DEXTERITY))
					{
						/* Success -> "Activated and avoided trap!" */
						Set_permanent_message_nr(524);
					}
					else
					{
						/* Failure -> Play sound effect */
						SOUND_Play_sound_effect(108, 100, 100, 0, 0);

						/* "Activated trap!" */
						Set_permanent_message_nr(525);

						/* Trap was activated */
						*(Lock->Lock_result_ptr) = LOCK_STATUS_TRAP_ACTIVATED;
					}

					/* The lock no longer has a trap */
					Lock->Lock_has_trap = FALSE;
				}
				else
				{
					/* No -> "Couldn't pick lock!" */
					Set_permanent_message_nr(538);
				}

			}
		}
	}
}

