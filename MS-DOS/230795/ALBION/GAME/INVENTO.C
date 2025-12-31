/************
 * NAME     : INVENTO.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : INVENTO.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GAMETEXT.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <INPUT.H>
#include <SCROLBAR.H>
#include <INVENTO.H>
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

/* defines */

#define INVENTORY_PAL_NR	(19)

/* structure definitions */

/* prototypes */

/* Inventory module functions */
void Inventory_ModInit(void);
void Inventory_ModExit(void);
void Inventory_DisInit(void);
void Inventory_DisExit(void);

/* Left inventory methods */
UNLONG Init_InvLeft1_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvLeft1_object(struct Object *Object, union Method_parms *P);

UNLONG Init_InvLeft2_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvLeft2_object(struct Object *Object, union Method_parms *P);

UNLONG Init_InvLeft3_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvLeft3_object(struct Object *Object, union Method_parms *P);

/* Right inventory methods */
UNLONG Init_InvRight_object(struct Object *Object, union Method_parms *P);
UNLONG Exit_InvRight_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_InvRight_object(struct Object *Object, union Method_parms *P);

/* Damage display methods */
UNLONG Init_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Damage_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Damage_display_object(struct Object *Object, union Method_parms *P);

/* Protection display methods */
UNLONG Init_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Protection_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Protection_display_object(struct Object *Object, union Method_parms *P);

/* Weight display methods */
UNLONG Init_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Weight_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Weight_display_object(struct Object *Object, union Method_parms *P);

/* global variables */

static struct Module Inventory_Mod = {
	0, SCREEN_MOD, INVENTORY_SCREEN,
	Update_display,
	Inventory_ModInit,
	Inventory_ModExit,
	Inventory_DisInit,
	Inventory_DisExit,
	NULL
};

UNSHORT Inventory_member;
UNSHORT Inventory_mode = 1;
MEM_HANDLE Inventory_char_handle;

MEM_HANDLE Full_body_pic_handle;

BOOLEAN In_Inventory = FALSE;
BOOLEAN Left_Inventory = FALSE;

/* Object handles */
UNSHORT InvLeft_object;
UNSHORT InvRight_object;

UNSHORT Body_item_list_object;
UNSHORT Backpack_item_list_object;

/* Inventory mode per party member */
static UNSHORT Party_inventory_modes[6];

/* Left inventory 1 object class */
static struct Method InvLeft1_methods[] = {
	{ INIT_METHOD, Init_InvLeft1_object },
	{ DRAW_METHOD, Draw_InvLeft1_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

static struct Object_class InvLeft1_Class = {
	0, sizeof(struct Object),
	&InvLeft1_methods[0]
};

/* Left inventory 2 object class */
static struct Method InvLeft2_methods[] = {
	{ INIT_METHOD, Init_InvLeft2_object },
	{ DRAW_METHOD, Draw_InvLeft2_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

static struct Object_class InvLeft2_Class = {
	0, sizeof(struct Object),
	&InvLeft2_methods[0]
};

/* Left inventory 3 object class */
static struct Method InvLeft3_methods[] = {
	{ INIT_METHOD, Init_InvLeft3_object },
	{ DRAW_METHOD, Draw_InvLeft3_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

static struct Object_class InvLeft3_Class = {
	0, sizeof(struct Object),
	&InvLeft3_methods[0]
};

/* Right inventory object class */
static struct Method InvRight_methods[] = {
	{ INIT_METHOD, Init_InvRight_object },
	{ EXIT_METHOD, Exit_InvRight_object },
	{ DRAW_METHOD, Draw_InvRight_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

struct Object_class InvRight_Class = {
	0, sizeof(struct Object),
	&InvRight_methods[0]
};

/* Damage display object class */
static struct Method Damage_display_methods[] = {
	{ INIT_METHOD, Init_Damage_display_object },
	{ DRAW_METHOD, Draw_Damage_display_object },
	{ UPDATE_METHOD, Update_Damage_display_object },
//	{ HIGHLIGHT_METHOD, Highlight_Damage_display_object },
	{ HELP_METHOD, Help_Damage_display_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

static struct Object_class Damage_display_Class = {
	0, sizeof(struct Value_display_object),
	&Damage_display_methods[0]
};

/* Protection display object class */
static struct Method Protection_display_methods[] = {
	{ INIT_METHOD, Init_Protection_display_object },
	{ DRAW_METHOD, Draw_Protection_display_object },
	{ UPDATE_METHOD, Update_Protection_display_object },
//	{ HIGHLIGHT_METHOD, Highlight_Protection_display_object },
	{ HELP_METHOD, Help_Protection_display_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

static struct Object_class Protection_display_Class = {
	0, sizeof(struct Value_display_object),
	&Protection_display_methods[0]
};

/* Weight display object class */
static struct Method Weight_display_methods[] = {
	{ INIT_METHOD, Init_Weight_display_object },
	{ DRAW_METHOD, Draw_Weight_display_object },
	{ UPDATE_METHOD, Update_Weight_display_object },
//	{ HIGHLIGHT_METHOD, Highlight_Weight_display_object },
	{ HELP_METHOD, Help_Weight_display_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

static struct Object_class Weight_display_Class = {
	0, sizeof(struct Value_display_object),
	&Weight_display_methods[0]
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

/* Left inventory radio buttons data */
static union Button_data Left_inventory_button_data[3];
static struct Button_OID Left_inventory_button_OID[3] = {
	{
	  	TEXT_BUTTON_TYPE | RADIO_BUTTON,
		1,
		54,
		&(Left_inventory_button_data[0]),
		Switch_inventory_mode
	},
	{
	  	TEXT_BUTTON_TYPE | RADIO_BUTTON,
		2,
		55,
		&(Left_inventory_button_data[1]),
		Switch_inventory_mode
	},
	{
	  	TEXT_BUTTON_TYPE | RADIO_BUTTON,
		3,
		56,
		&(Left_inventory_button_data[2]),
		Switch_inventory_mode
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Go_Inventory
 * FUNCTION  : Go to a party member's inventory.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 13:00
 * LAST      : 29.12.94 13:01
 * INPUTS    : UNSHORT Member_nr - Party member number (1...6).
 *             UNSHORT Mode_nr - Inventory mode number (1...3).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Go_Inventory(UNSHORT Member_nr, UNSHORT Mode_nr)
{
	static struct Event_action Inventory_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		SWITCH_TO_INVENTORY_ACTION, 0, 0,
		Do_go_inventory, NULL, NULL
	};

	/* Check events */
	Inventory_action.Actor_index = Member_nr;
	Perform_action(&Inventory_action);
}

BOOLEAN
Do_go_inventory(struct Event_action *Action)
{
	UNSHORT Member_nr;

	Member_nr = Action->Actor_index;

	if (In_Inventory)
	{
		if (Inventory_member != Member_nr)
		{
			Inventory_char_handle = Party_char_handles[Member_nr - 1];

			Inventory_member = Member_nr;

			Exit_display();
			Pop_module();
			Push_module(&Inventory_Mod);
			Init_display();
		}
	}
	else
	{
		Inventory_char_handle = Party_char_handles[Member_nr - 1];

		Inventory_member = Member_nr;

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
 * LAST      : 09.01.95 10:04
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
	Pop_module();
	Init_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Go_inventory_mode
 * FUNCTION  : Go to a mode of a party member's inventory.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:04
 * LAST      : 11.01.95 10:04
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

		/* Delete old mode */
		Delete_object(InvLeft_object);

		/* Add new mode */
		switch (Inventory_mode)
		{
			case INVLEFT_STATUS_1:
			{
				InvLeft_object = Add_object(Earth_object, &InvLeft1_Class, NULL,
				 0, 0, INVENTORY_MIDDLE, Panel_Y);
				break;
			}
			case INVLEFT_STATUS_2:
			{
				InvLeft_object = Add_object(Earth_object, &InvLeft2_Class, NULL,
				 0, 0, INVENTORY_MIDDLE, Panel_Y);
				break;
			}
			case INVLEFT_STATUS_3:
			{
				InvLeft_object = Add_object(Earth_object, &InvLeft3_Class, NULL,
				 0, 0, INVENTORY_MIDDLE, Panel_Y);
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
	Exit_inventory_button_OID.Help_message_nr = 21;
	Exit_inventory_button_OID.Function = Exit_Inventory;

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
			InvLeft_object = Add_object(Earth_object, &InvLeft1_Class, NULL,
			 0, 0, INVENTORY_MIDDLE, Panel_Y);
			break;
		}
		case INVLEFT_STATUS_2:
		{
			InvLeft_object = Add_object(Earth_object, &InvLeft2_Class, NULL,
			 0, 0, INVENTORY_MIDDLE, Panel_Y);
			break;
		}
		case INVLEFT_STATUS_3:
		{
			InvLeft_object = Add_object(Earth_object, &InvLeft3_Class, NULL,
			 0, 0, INVENTORY_MIDDLE, Panel_Y);
			break;
		}
	}

	InvRight_object = Add_object(Earth_object, &InvRight_Class, NULL,
	 INVENTORY_MIDDLE, 0, Screen_width - INVENTORY_MIDDLE, Panel_Y);

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

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
	Switch_screens();
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
 * NAME      : Init_InvLeft1_object
 * FUNCTION  : Initialize method of Left Inventory 1 object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:42
 * LAST      : 10.01.95 16:42
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft1_object(struct Object *Object, union Method_parms *P)
{
	union Method_parms P2;
	UNSHORT Obj;

	/* Add radio group object */
	Obj = Add_object(Object->Self, &Radio_group_Class, NULL, 85, 175, 48, 14);

	/* Initialize radio button data */
	Left_inventory_button_data[0].Text_button_data.Text = System_text_ptrs[57];
 	Left_inventory_button_data[1].Text_button_data.Text = System_text_ptrs[58];
	Left_inventory_button_data[2].Text_button_data.Text = System_text_ptrs[59];

	/* Add radio button to group */
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[0],
	 0, 0, 14, 14);
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[1],
	 17, 0, 14, 14);
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[2],
	 34, 0, 14, 14);

	/* Set radio button */
	P2.Value = Inventory_mode;
	Execute_method(Obj, SET_METHOD, &P2);

	/* There is a left inventory */
	Left_Inventory = TRUE;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvLeft1_object
 * FUNCTION  : Draw method of Left Inventory 1 object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 16:42
 * LAST      : 10.01.95 16:42
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvLeft1_object(struct Object *Object, union Method_parms *P)
{
	/* Clear left inventory */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvLeft2_object
 * FUNCTION  : Initialize method of Left Inventory 2 object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 11.01.95 10:02
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft2_object(struct Object *Object, union Method_parms *P)
{
	union Method_parms P2;
	UNSHORT Obj;

	/* Add radio group object */
	Obj = Add_object(Object->Self, &Radio_group_Class, NULL, 85, 175, 48, 14);

	/* Initialize radio button data */
	Left_inventory_button_data[0].Text_button_data.Text = System_text_ptrs[57];
 	Left_inventory_button_data[1].Text_button_data.Text = System_text_ptrs[58];
	Left_inventory_button_data[2].Text_button_data.Text = System_text_ptrs[59];

	/* Add radio button to group */
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[0],
	 0, 0, 14, 14);
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[1],
	 17, 0, 14, 14);
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[2],
	 34, 0, 14, 14);

	/* Set radio button */
	P2.Value = Inventory_mode;
	Execute_method(Obj, SET_METHOD, &P2);

	/* There is a left inventory */
	Left_Inventory = TRUE;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvLeft2_object
 * FUNCTION  : Draw method of Left Inventory 2 object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 11.01.95 10:02
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvLeft2_object(struct Object *Object, union Method_parms *P)
{
	/* Clear left inventory */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvLeft3_object
 * FUNCTION  : Initialize method of Left Inventory 3 object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 11.01.95 10:02
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_InvLeft3_object(struct Object *Object, union Method_parms *P)
{
	union Method_parms P2;
	UNSHORT Obj;

	/* Add radio group object */
	Obj = Add_object(Object->Self, &Radio_group_Class, NULL, 85, 175, 48, 14);

	/* Initialize radio button data */
	Left_inventory_button_data[0].Text_button_data.Text = System_text_ptrs[57];
 	Left_inventory_button_data[1].Text_button_data.Text = System_text_ptrs[58];
	Left_inventory_button_data[2].Text_button_data.Text = System_text_ptrs[59];

	/* Add radio button to group */
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[0],
	 0, 0, 14, 14);
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[1],
	 17, 0, 14, 14);
	Add_object(Obj, &Button_Class, (UNBYTE *) &Left_inventory_button_OID[2],
	 34, 0, 14, 14);

	/* Set radio button */
	P2.Value = Inventory_mode;
	Execute_method(Obj, SET_METHOD, &P2);

	/* There is a left inventory */
	Left_Inventory = TRUE;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_InvLeft3_object
 * FUNCTION  : Draw method of Left Inventory 3 object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 10:02
 * LAST      : 11.01.95 10:02
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_InvLeft3_object(struct Object *Object, union Method_parms *P)
{
	/* Clear left inventory */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_InvRight_object
 * FUNCTION  : Initialize method of Right Inventory object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 12:02
 * LAST      : 06.07.95 11:11
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

	Body_item_list_object = Add_object(Object->Self, &Body_item_list_Class,
	 (UNBYTE *) &Item_list_OID, 10, 10, BODY_AREA_WIDTH, BODY_AREA_HEIGHT);

	/* Add backpack item list */
	Item_list_OID.Type = CHAR_BACKPACK_INV_TYPE;
	Item_list_OID.Nr_items = ITEMS_PER_CHAR;
	Item_list_OID.Slots_width = 4;
	Item_list_OID.Slots_height = 6;
	Item_list_OID.Slots_handle = Inventory_char_handle;
	Item_list_OID.Slots_offset = Backpack_items_offset;
	Item_list_OID.Menu = &Inventory_item_PUM;
	Item_list_OID.Item_slot_class_ptr = &Backpack_item_slot_Class;

	Backpack_item_list_object = Add_object(Object->Self, &Item_list_Class,
	 (UNBYTE *) &Item_list_OID, 145, 11, 100, 100);

	/* Add damage display */
	Add_object(Object->Self, &Damage_display_Class, NULL, 7, 180, 29, 10);

	/* Add protection display */
	Add_object(Object->Self, &Protection_display_Class, NULL, 112, 180, 29, 10);

	/* Add weight display */
	Add_object(Object->Self, &Weight_display_Class, NULL, 40, 180, 68, 10);

	/* Build gold object OID */
	Gold_food_OID.Type = CHAR_GOLD_TYPE;
	Gold_food_OID.Data_handle = Inventory_char_handle;

	/* Add gold object */
	Add_object(Object->Self, &Gold_food_Class, (UNBYTE *) &Gold_food_OID, 145,
	 141, 35, 20);

	/* Build food object OID */
	Gold_food_OID.Type = CHAR_FOOD_TYPE;

	/* Add food object */
	Add_object(Object->Self, &Gold_food_Class, (UNBYTE *) &Gold_food_OID, 180,
	 141, 35, 20);

	/* Prepare exit button data */
	Exit_inventory_button_data.Symbol_button_data.Width = 56;
	Exit_inventory_button_data.Symbol_button_data.Height = 16;
	Exit_inventory_button_data.Symbol_button_data.Graphics_handle = NULL;
	Exit_inventory_button_data.Symbol_button_data.Graphics_offset =
	 (UNLONG) &(Exit_button[0]);

	/* Add exit button */
	Add_object(Object->Self, &Button_Class,
	 (UNBYTE *) &Exit_inventory_button_OID, 154, 171, 56, 17);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_InvRight_object
 * FUNCTION  : Exit method of Right Inventory object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 17:01
 * LAST      : 04.01.95 17:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Exit_InvRight_object(struct Object *Object, union Method_parms *P)
{
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
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

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

	/* Print member name */
	Get_char_name(Inventory_char_handle, Name);
	Print_centered_line_string(&Main_OPM, 161, 1, 92, Name);

	/* Print backpack description */
	Print_centered_line_string(&Main_OPM, 281, 1, 70, System_text_ptrs[47]);

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
	Damage_display->Value = Get_damage(Inventory_char_handle);

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
 * LAST      : 10.01.95 11:20
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
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw damage symbol */
	Put_masked_block(Current_OPM, Object->X + 1, Object->Y + 1, 8, 8,
	 &(Damage_symbol[0]));

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print damage */
	sprintf(String, ":%3u", Damage_display->Value);
	Print_string(Current_OPM, Object->X + 10, Object->Y + 1, String);

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
 * LAST      : 10.01.95 11:20
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
	UNSHORT Damage;

	Damage_display = (struct Value_display_object *) Object;

	/* Get damage */
	Damage = Get_damage(Inventory_char_handle);

	/* Has the damage value changed ? */
	if (Damage != Damage_display->Value)
	{
		/* Yes -> Adjust value */
		Damage_display->Value += sgn(Damage - Damage_display->Value);

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Damage_display_object
 * FUNCTION  : Highlight method of Damage display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:21
 * LAST      : 10.01.95 11:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Damage_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Damage_display;
	UNCHAR String[20];

	Damage_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw damage symbol */
	Put_masked_block(Current_OPM, Object->X + 1, Object->Y + 1, 8, 8,
	 &(Damage_symbol[0]));

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print damage */
	sprintf(String, ":%3u", Damage_display->Value);
	Print_string(Current_OPM, Object->X + 10, Object->Y + 1, String);

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
	sprintf(String, System_text_ptrs[43], Get_damage(Inventory_char_handle));

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
	Protection_display->Value = Get_protection(Inventory_char_handle);

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
 * LAST      : 10.01.95 11:20
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
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw protection symbol */
	Put_masked_block(Current_OPM, Object->X + 1, Object->Y + 1, 6, 8,
	 &(Protection_symbol[0]));

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print protection */
	sprintf(String, ":%3u", Protection_display->Value);
	Print_string(Current_OPM, Object->X + 8, Object->Y + 1, String);

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
 * LAST      : 10.01.95 11:20
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
	UNSHORT Protection;

	Protection_display = (struct Value_display_object *) Object;

	/* Get protection */
	Protection = Get_protection(Inventory_char_handle);

	/* Has the protection value changed ? */
	if (Protection != Protection_display->Value)
	{
		/* Yes -> Adjust value */
		Protection_display->Value += sgn(Protection - Protection_display->Value);

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Protection_display_object
 * FUNCTION  : Highlight method of Protection display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:21
 * LAST      : 10.01.95 11:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Protection_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Protection_display;
	UNCHAR String[20];

	Protection_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw protection symbol */
	Put_masked_block(Current_OPM, Object->X + 1, Object->Y + 1, 6, 8,
	 &(Protection_symbol[0]));

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print protection */
	sprintf(String, ":%3u", Protection_display->Value);
	Print_string(Current_OPM, Object->X + 8, Object->Y + 1, String);

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
	sprintf(String, System_text_ptrs[44], Get_protection(Inventory_char_handle));

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
 * LAST      : 10.01.95 12:12
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

	/* Load initial weight value */
	Weight_display->Value = Get_carried_weight(Inventory_char_handle);

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
 * LAST      : 10.01.95 12:16
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
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print weight */
	sprintf(String, System_text_ptrs[45], Weight_display->Value / 1000);
	Print_centered_string(Current_OPM, Object->X + 1, Object->Y + 1,
	 Object->Rect.width - 2, String);

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
 * LAST      : 10.01.95 11:20
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
	UNLONG Weight;

	Weight_display = (struct Value_display_object *) Object;

	Weight = Get_carried_weight(Inventory_char_handle);

	/* Has the weight value changed ? */
	if (Weight != Weight_display->Value)
	{
		/* Yes -> Adjust value */
		Weight_display->Value += sgn(Weight - Weight_display->Value);

		/* Redraw */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Weight_display_object
 * FUNCTION  : Highlight method of Weight display object.
 * FILE      : INVENTO.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.01.95 11:21
 * LAST      : 10.01.95 11:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Weight_display_object(struct Object *Object, union Method_parms *P)
{
	struct Value_display_object *Weight_display;
	UNCHAR String[80];

	Weight_display = (struct Value_display_object *) Object;

	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Print weight */
	sprintf(String, System_text_ptrs[45], Weight_display->Value / 1000);
	Print_centered_string(Current_OPM, Object->X + 1, Object->Y + 1,
	 Object->Rect.width - 2, String);

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
Help_Weight_display_object(struct Object *Object, union Method_parms *P)
{
	UNLONG Weight;
	UNCHAR String[100];

	/* Make help line string */
	Weight = Get_carried_weight(Inventory_char_handle);
	sprintf(String, System_text_ptrs[46], Weight);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) String);

	return 0;
}

