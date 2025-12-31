/************
 * NAME     : EXAMITEM.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 9-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : EXAMITEM.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>

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
#include <EXAMITEM.H>
#include <INVITEMS.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <BUTTONS.H>
#include <INPUT.H>
#include <EVELOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <MAGIC.H>
#include <COLOURS.H>

/* defines */

#define EXAMITEM1_WIDTH		(240)
#define EXAMITEM1_HEIGHT	(112)

#define EXAMITEM2_WIDTH		(220)

/* structure definitions */

/* Examine item 1 window OID */
struct ExamItem1_window_OID {
	struct Item_packet *Packet;
};

/* Examine item 1 window object */
struct ExamItem1_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	struct Item_packet *Packet;

	UNSHORT Item_frame;
};

/* Examine item 2 window OID */
struct ExamItem2_window_OID {
	struct Item_packet *Packet;
};

/* Examine item 2 window object */
struct ExamItem2_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	struct Item_packet *Packet;
};

/* prototypes */

/* Examine item 1 window object methods */
UNLONG Init_ExamItem1_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_ExamItem1_object(struct Object *Object, union Method_parms *P);

void ExamItem1_Exit(struct Button_object *Button);
void Go_ExamItem2(struct Button_object *Button);

/* Examine item 2 window object methods */
UNLONG Init_ExamItem2_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_ExamItem2_object(struct Object *Object, union Method_parms *P);

void ExamItem2_Exit(struct Button_object *Button);

/* global variables */

/* Examine item 1 window method list */
static struct Method ExamItem1_methods[] = {
	{ INIT_METHOD,		Init_ExamItem1_object },
	{ EXIT_METHOD,		Exit_Window_object },
	{ DRAW_METHOD,		Draw_ExamItem1_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ CLOSE_METHOD,	Close_Window_object },
	{ RIGHT_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Examine item 1 window class description */
static struct Object_class ExamItem1_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct ExamItem1_window_object),
	&ExamItem1_methods[0]
};

/* Examine item 2 window method list */
static struct Method ExamItem2_methods[] = {
	{ INIT_METHOD,		Init_ExamItem2_object },
	{ EXIT_METHOD,		Exit_Window_object },
	{ DRAW_METHOD,		Draw_ExamItem2_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ CLOSE_METHOD,	Close_Window_object },
	{ RIGHT_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Examine item 2 window class description */
static struct Object_class ExamItem2_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct ExamItem2_window_object),
	&ExamItem2_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Examine_item
 * FUNCTION  : Examine an item.
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 14:23
 * LAST      : 11.09.95 14:23
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Examine_item(struct Item_packet *Packet)
{
	struct ExamItem1_window_OID OID;
	UNSHORT Obj;

	/* Build OID */
	OID.Packet = Packet;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&ExamItem1_Class,
		(UNBYTE *) &OID,
		(Screen_width - EXAMITEM1_WIDTH) / 2,
		(Panel_Y	- EXAMITEM1_HEIGHT) / 2,
		EXAMITEM1_WIDTH,
		EXAMITEM1_HEIGHT
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_ExamItem1_object
 * FUNCTION  : Initialize method of Examine item 1 window object.
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 15:35
 * LAST      : 11.09.95 15:35
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_ExamItem1_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data ExamItem1_go_button_data;
	static struct Button_OID ExamItem1_go_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		694,
		&ExamItem1_go_button_data,
		Go_ExamItem2
	};
	static union Button_data ExamItem1_exit_button_data;
	static struct Button_OID ExamItem1_exit_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		695,
		&ExamItem1_exit_button_data,
		ExamItem1_Exit
	};

	struct ExamItem1_window_object *ExamItem1_window;
	struct ExamItem1_window_OID *OID;

	ExamItem1_window = (struct ExamItem1_window_object *) Object;
	OID = (struct ExamItem1_window_OID *) P;

	/* Copy data from OID */
	ExamItem1_window->Packet = OID->Packet;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Has the item been magically examined / cheat mode ? */
	if ((ExamItem1_window->Packet->Flags & MAGIC_CHECK) || Cheat_mode)
	{
		/* Yes -> Initialize go button data */
		ExamItem1_go_button_data.Text_button_data.Text = System_text_ptrs[683];

		/* Add go button to window */
	 	Add_object
		(
			Object->Self,
			&Button_Class,
			(UNBYTE *) &ExamItem1_go_button_OID,
			Object->Rect.width - 63,
			90,
			50,
			11
		);
	}

	/* Initialize exit button data */
	ExamItem1_exit_button_data.Text_button_data.Text = System_text_ptrs[64];

	/* Add exit button to window */
 	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &ExamItem1_exit_button_OID,
		(Object->Rect.width - 50) / 2,
		90,
		50,
		11
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_ExamItem1_object
 * FUNCTION  : Draw method of Examine item 1 window object.
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 16:45
 * LAST      : 15.10.95 21:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_ExamItem1_object(struct Object *Object, union Method_parms *P)
{
	struct ExamItem1_window_object *ExamItem1_window;
	struct OPM *OPM;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNCHAR Item_name[ITEM_NAME_LENGTH + 1];
	UNBYTE *Ptr;

	ExamItem1_window = (struct ExamItem1_window_object *) Object;
	OPM = &(ExamItem1_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Get item packet and data */
	Packet = ExamItem1_window->Packet;
	Item_data = Get_item_data(Packet);

	/* Draw box around and behind enlarged item */
	Draw_deep_box
	(
		OPM,
		13,
		21,
		(16 * 4) + 2,
		(16 * 4) + 2
	);

	/* Get pointer to item graphics */
	Ptr = MEM_Claim_pointer(Item_graphics_handle) +
	 ((UNSHORT) Item_data->Pic_nr * 256);

	/* Draw enlarged item */
	Draw_enlarged_block
	(
		OPM,
		14,
		22,
		16,
		16,
		Ptr,
		&Palette
	);

	MEM_Free_pointer(Item_graphics_handle);

	/* Get item name */
	Get_item_name(Packet, Item_name);

	Set_ink(GOLD_TEXT);

	/* Print item name */
	Print_centered_line_string
	(
		OPM,
		13,
		11,
		Object->Rect.width - 26,
		Item_name
	);

	Set_ink(SILVER_TEXT);

	/* Print item type description */
	Print_string
	(
		OPM,
		83,
		23,
		System_text_ptrs[676]
	);

	/* Print item type */
	Print_string
	(
		OPM,
		133,
		23,
		System_text_ptrs[22 + Item_data->Type]
	);

	/* Print weight description */
	Print_string
	(
		OPM,
		83,
		33,
		System_text_ptrs[675]
	);

	/* Print weight */
	xprintf
	(
		OPM,
		133,
		33,
		System_text_ptrs[684],
		Item_data->Weight
	);

	/* Cursed / magically examined ? */
	if ((Packet->Flags & CURSED) && (Packet->Flags & MAGIC_CHECK))
	{
		/* Yes */
		Set_ink(RED_TEXT);

		/* Print damage description */
		Print_string
		(
			OPM,
			83,
			43,
			System_text_ptrs[681]
		);

		/* Print damage */
		xprintf
		(
			OPM,
			133,
			43,
			"%d",
			0 - (SISHORT) Item_data->Damage_pts
		);

		/* Print protection description */
		Print_string
		(
			OPM,
			83,
			53,
			System_text_ptrs[682]
		);

		/* Print protection */
		xprintf
		(
			OPM,
			133,
			53,
			"%d",
			0 - (SISHORT) Item_data->Protection_pts
		);
	}
	else
	{
		/* No */
		Set_ink(SILVER_TEXT);

		/* Print damage description */
		Print_string
		(
			OPM,
			83,
			43,
			System_text_ptrs[681]
		);

		/* Print damage */
		xprintf
		(
			OPM,
			133,
			43,
			"%d",
			(SISHORT) Item_data->Damage_pts
		);

		/* Print protection description */
		Print_string
		(
			OPM,
			83,
			53,
			System_text_ptrs[682]
		);

		/* Print protection */
		xprintf
		(
			OPM,
			133,
			53,
			"%d",
			(SISHORT) Item_data->Protection_pts
		);
	}

	Free_item_data();

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ExamItem1_Exit
 * FUNCTION  : Exit Examine item 1 window (button).
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 15:34
 * LAST      : 11.09.95 15:34
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
ExamItem1_Exit(struct Button_object *Button)
{
	Delete_object(Button->Object.Parent);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Go_ExamItem2
 * FUNCTION  : Enter Examine item 2 window (button).
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 19:39
 * LAST      : 11.09.95 19:39
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Go_ExamItem2(struct Button_object *Button)
{
	struct ExamItem1_window_object *ExamItem1_window;
	struct ExamItem2_window_OID OID;
	UNSHORT Obj;

	ExamItem1_window = (struct ExamItem1_window_object *) Get_object_data(Button->Object.Parent);

	/* Build OID */
	OID.Packet = ExamItem1_window->Packet;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&ExamItem2_Class,
		(UNBYTE *) &OID,
		10,
		10,
		100,
		100
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_ExamItem2_object
 * FUNCTION  : Initialize method of Examine item 2 window object.
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 19:41
 * LAST      : 11.09.95 22:41
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_ExamItem2_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data ExamItem2_exit_button_data;
	static struct Button_OID ExamItem2_exit_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		695,
		&ExamItem2_exit_button_data,
		ExamItem2_Exit
	};

	struct ExamItem2_window_object *ExamItem2_window;
	struct ExamItem2_window_OID *OID;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT Window_height;

	ExamItem2_window = (struct ExamItem2_window_object *) Object;
	OID = (struct ExamItem2_window_OID *) P;

	/* Copy data from OID */
	ExamItem2_window->Packet = OID->Packet;

	/* Get item packet and data */
	Packet = ExamItem2_window->Packet;
	Item_data = Get_item_data(Packet);

	/* Does the item contain a spell ? */
	if (Item_data->Spell_nr)
	{
		/* Yes */
		Window_height = 117;
	}
	else
	{
		/* No */
		Window_height = 97;
	}

	Free_item_data();

	/* Set width and height */
	Change_object_size
	(
		Object->Self,
		EXAMITEM2_WIDTH,
		Window_height
	);

	/* Centre window */
	Change_object_position
	(
		Object->Self,
		(Screen_width - Object->Rect.width) / 2,
		(Panel_Y - Object->Rect.height) / 2
	);

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Initialize exit button data */
	ExamItem2_exit_button_data.Text_button_data.Text = System_text_ptrs[64];

	/* Add exit button to window */
 	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &ExamItem2_exit_button_OID,
		(Object->Rect.width - 50) / 2,
		Object->Rect.height - 22,
		50,
		11
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_ExamItem2_object
 * FUNCTION  : Draw method of Examine item 2 window object.
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 19:41
 * LAST      : 28.09.95 17:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_ExamItem2_object(struct Object *Object, union Method_parms *P)
{
	struct ExamItem2_window_object *ExamItem2_window;
	struct OPM *OPM;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT i;

	ExamItem2_window = (struct ExamItem2_window_object *) Object;
	OPM = &(ExamItem2_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	Set_ink(SILVER_TEXT);

	/* Get item packet and data */
	Packet = ExamItem2_window->Packet;
	Item_data = Get_item_data(Packet);

	for (i=0;i<2;i++)
	{
		/* Print skill tax description */
	 	Print_string
		(
			OPM,
			11,
			11 + (i * 10),
			System_text_ptrs[685 + i]
		);

		/* Any skill tax ? */
		if (Item_data->Malus[i])
		{
			/* Yes -> Print skill name and tax */
			xprintf
			(
				OPM,
				101,
				11 + (i * 10),
				"%s %+d",
				System_text_ptrs[643 + Item_data->Item_skills[i]],
				0 - (SISHORT) Item_data->Malus[i]
			);
		}
		else
		{
			/* No -> Print "none" */
			Print_string
			(
				OPM,
				101,
				11 + (i * 10),
				System_text_ptrs[687]
			);
		}
	}

	/* Print LP max bonus description */
	Print_string
	(
		OPM,
		11,
		31,
		System_text_ptrs[688]
	);

	/* Print SP max bonus description */
	Print_string
	(
		OPM,
		11,
		41,
		System_text_ptrs[689]
	);

	/* Print attribute bonus description */
	Print_string
	(
		OPM,
		11,
		51,
		System_text_ptrs[690]
	);

	/* Print skill bonus description */
	Print_string
	(
		OPM,
		11,
		61,
		System_text_ptrs[691]
	);

	/* Does the item contain a spell ? */
	if (Item_data->Spell_nr)
	{
		/* Yes */
		Set_ink(GOLD_TEXT);

		/* Print spell description */
		Print_string
		(
			OPM,
			11,
			71,
			System_text_ptrs[692]
		);

		/* Print spell name and charges */
		xprintf
		(
			OPM,
			101,
			71,
			"%s (%d)",
			Get_spell_name
			(
				Item_data->Spell_class,
				Item_data->Spell_nr
			),
			Packet->Charges
		);

		/* Print enchantment description */
		Print_string
		(
			OPM,
			11,
			81,
			System_text_ptrs[693]
		);

		/* Print enchantments and enchantments maximum */
		xprintf
		(
			OPM,
			101,
			81,
			"%d / %d",
			(UNSHORT) Packet->Nr_enchantments,
			(UNSHORT) Item_data->Max_enchantments
		);
	}

	/* Cursed item ? */
	if (Packet->Flags & CURSED)
	{
		/* Yes */
		Set_ink(RED_TEXT);

		/* Print LP max bonus */
		xprintf
		(
			OPM,
			101,
			31,
			"%+d",
			0 - (SISHORT) Item_data->LP_max
		);

		/* Print SP max bonus */
		xprintf
		(
			OPM,
			101,
			41,
			"%+d",
			0 - (SISHORT) Item_data->SP_max
		);

		/* Any attribute bonus ? */
		if (Item_data->Attribute_normal)
		{
			/* Yes -> Print attribute bonus */
			xprintf
			(
				OPM,
				101,
				51,
				"%s %+d",
				System_text_ptrs[635 + Item_data->Attribute],
				0 - (SISHORT) Item_data->Attribute_normal
			);
		}
		else
		{
			/* No -> Print "none" */
			Print_string
			(
				OPM,
				101,
				51,
				System_text_ptrs[687]
			);
		}

		/* Any skill bonus ? */
		if (Item_data->Skill_normal)
		{
			/* Yes -> Print skill bonus */
			xprintf
			(
				OPM,
				101,
				61,
				"%s %+d",
				System_text_ptrs[643 + Item_data->Skill],
				0 - (SISHORT) Item_data->Skill_normal
			);
		}
		else
		{
			/* No -> Print "none" */
			Print_string
			(
				OPM,
				101,
				61,
				System_text_ptrs[687]
			);
		}
	}
	else
	{
		/* No */
		Set_ink(SILVER_TEXT);

		/* Print LP max bonus */
		xprintf
		(
			OPM,
			101,
			31,
			"%+d",
			(SISHORT) Item_data->LP_max
		);

		/* Print SP max bonus */
		xprintf
		(
			OPM,
			101,
			41,
			"%+d",
			(SISHORT) Item_data->SP_max
		);

		/* Any attribute bonus ? */
		if (Item_data->Attribute_normal)
		{
			/* Yes -> Print attribute bonus */
			xprintf
			(
				OPM,
				101,
				51,
				"%s %+d",
				System_text_ptrs[635 + Item_data->Attribute],
				(SISHORT) Item_data->Attribute_normal
			);
		}
		else
		{
			/* No -> Print "none" */
			Print_string
			(
				OPM,
				101,
				51,
				System_text_ptrs[687]
			);
		}

		/* Any skill bonus ? */
		if (Item_data->Skill_normal)
		{
			/* Yes -> Print skill bonus */
			xprintf
			(
				OPM,
				101,
				61,
				"%s %+d",
				System_text_ptrs[643 + Item_data->Skill],
				(SISHORT) Item_data->Skill_normal
			);
		}
		else
		{
			/* No -> Print "none" */
			Print_string
			(
				OPM,
				101,
				61,
				System_text_ptrs[687]
			);
		}
	}

	Free_item_data();

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ExamItem2_Exit
 * FUNCTION  : Exit Examine item 2 window (button).
 * FILE      : EXAMITEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 19:40
 * LAST      : 11.09.95 19:40
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
ExamItem2_Exit(struct Button_object *Button)
{
	Delete_object(Button->Object.Parent);
}

