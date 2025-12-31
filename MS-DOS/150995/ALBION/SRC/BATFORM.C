/************
 * NAME     : BATFORM.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 9-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : BATFORM.H
 ************/

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
#include <BATFORM.H>
#include <INVITEMS.H>
#include <ITEMLIST.H>
#include <BUTTONS.H>
#include <INPUT.H>
#include <EVELOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <MAGIC.H>
#include <COLOURS.H>

/* defines */

#define BATFORM_WIDTH	(270)
#define BATFORM_HEIGHT	(117)

/* structure definitions */

/* Battle formation window object */
struct BatForm_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNSHORT Battle_order_slots[12];
};

/* Battle formation slot OID */
struct BatFormSlot_OID {
	UNSHORT Index;
};

/* Battle formation slot object */
struct BatFormSlot_object {
	struct Object Object;
	UNSHORT Index;
};

/* prototypes */

/* Battle formation window object methods */
UNLONG Init_BatForm_object(struct Object *Object, union Method_parms *P);
UNLONG Exit_BatForm_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_BatForm_object(struct Object *Object, union Method_parms *P);

void BatForm_Exit(struct Button_object *Button);

/* Battle formation slot object methods */
UNLONG Init_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Help_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Left_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Drop_BatFormSlot_object(struct Object *Object, union Method_parms *P);
UNLONG Inquire_drop_BatFormSlot_object(struct Object *Object, union Method_parms *P);

void Drag_BatFormSlot(struct BatFormSlot_object *BatFormSlot);
void Drop_on_BatFormSlot(struct BatFormSlot_object *BatFormSlot,
 struct Drag_drop_data *Drag_drop_data_ptr);
void BatFormSlot_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr);

/* global variables */

/* Battle formation window method list */
static struct Method BatForm_methods[] = {
	{ INIT_METHOD,		Init_BatForm_object },
	{ EXIT_METHOD,		Exit_BatForm_object },
	{ DRAW_METHOD,		Draw_BatForm_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ CLOSE_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Battle formation window class description */
static struct Object_class BatForm_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct BatForm_window_object),
	&BatForm_methods[0]
};

/* Battle formation member method list */
static struct Method BatFormSlot_methods[] = {
	{ INIT_METHOD,				Init_BatFormSlot_object },
	{ DRAW_METHOD,		  		Draw_BatFormSlot_object },
	{ FEEDBACK_METHOD,  		Feedback_BatFormSlot_object },
	{ HIGHLIGHT_METHOD, 		Highlight_BatFormSlot_object },
	{ HELP_METHOD,				Help_BatFormSlot_object },
	{ LEFT_METHOD,	  			Left_BatFormSlot_object },
	{ DROP_METHOD,				Drop_BatFormSlot_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_BatFormSlot_object },
	{ TOUCHED_METHOD,			Normal_touched },
	{ 0, NULL}
};

/* Battle formation member class description */
static struct Object_class BatFormSlot_Class = {
	0, sizeof(struct BatFormSlot_object),
	&BatFormSlot_methods[0]
};

static UNSHORT Dragged_BatForm_member_index;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Go_Battle_formation
 * FUNCTION  : Let the user change the battle formation (button).
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 20:20
 * LAST      : 10.09.95 20:20
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Go_Battle_formation(struct Button_object *Button)
{
	UNSHORT Obj;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&BatForm_Class,
		NULL,
		(Screen_width - BATFORM_WIDTH) / 2,
		(Panel_Y	- BATFORM_HEIGHT) / 2,
		BATFORM_WIDTH,
		BATFORM_HEIGHT
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_BatForm_object
 * FUNCTION  : Initialize method of Battle formation window object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 20:59
 * LAST      : 10.09.95 20:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_BatForm_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data BatForm_exit_button_data;
	static struct Button_OID BatForm_exit_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&BatForm_exit_button_data,
		BatForm_Exit
	};

	struct BatForm_window_object *BatForm_window;
	struct BatForm_window_OID *OID;
	struct BatFormSlot_OID BatFormSlot_OID;
	UNSHORT Position;
	UNSHORT i, j;

	BatForm_window = (struct BatForm_window_object *) Object;
	OID = (struct BatForm_window_OID *) P;

	/* Clear battle order slots */
	for (i=0;i<12;i++)
	{
		BatForm_window->Battle_order_slots[i] = 0;
	}

	/* Insert party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get position */
			Position = PARTY_DATA.Battle_order[i];

			/* Insert party member */
			BatForm_window->Battle_order_slots[Position] = i + 1;
		}
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add battle formation slots to window */
	BatFormSlot_OID.Index = 1;
	for (i=0;i<2;i++)
	{
		for (j=0;j<6;j++)
		{
	 		Add_object
			(
				Object->Self,
				&BatFormSlot_Class,
				(UNBYTE *) &BatFormSlot_OID,
				23 + (j * (PORTRAIT_WIDTH + 4)),
				11 + (i * (PORTRAIT_HEIGHT + 4)),
				PORTRAIT_WIDTH,
				PORTRAIT_HEIGHT
			);

			BatFormSlot_OID.Index++;
		}
	}

	/* Initialize button data */
	BatForm_exit_button_data.Text_button_data.Text = System_text_ptrs[64];

	/* Add button to window */
 	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &BatForm_exit_button_OID,
		(Object->Rect.width - 50) / 2,
		95,
		50,
		11
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_BatForm_object
 * FUNCTION  : Exit method of Battle formation window object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 12:48
 * LAST      : 11.09.95 12:48
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Exit_BatForm_object(struct Object *Object, union Method_parms *P)
{
	struct BatForm_window_object *BatForm_window;
	UNSHORT Member_index;
	UNSHORT i;

	BatForm_window = (struct BatForm_window_object *) Object;

	/* Re-build battle formation */
	for (i=0;i<12;i++)
	{
		/* Get party member index */
		Member_index = BatForm_window->Battle_order_slots[i];

		/* Anyone there ? */
		if (Member_index)
		{
			/* Yes -> Store in battle formation */
			PARTY_DATA.Battle_order[Member_index - 1] = i;
		}
	}

	/* Exit window */
	Exit_Window_object(Object, P);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_BatForm_object
 * FUNCTION  : Draw method of Battle formation window object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:07
 * LAST      : 11.09.95 16:18
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_BatForm_object(struct Object *Object, union Method_parms *P)
{
	/* Draw window */
	Draw_Window_object(Object, P);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BatForm_Exit
 * FUNCTION  : Exit Battle formation window (button).
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:12
 * LAST      : 10.09.95 21:12
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BatForm_Exit(struct Button_object *Button)
{
	Delete_object(Button->Object.Parent);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_BatFormSlot_object
 * FUNCTION  : Init method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 20:53
 * LAST      : 10.09.95 20:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	struct BatFormSlot_object *BatFormSlot;
	struct BatFormSlot_OID *OID;

	BatFormSlot = (struct BatFormSlot_object *) Object;
	OID = (struct BatFormSlot_OID *) P;

	/* Copy data from OID */
	BatFormSlot->Index = OID->Index;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_BatFormSlot_object
 * FUNCTION  : Draw method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 20:54
 * LAST      : 10.09.95 20:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	struct BatFormSlot_object *BatFormSlot;
	struct BatForm_window_object *BatForm_window;
	struct OPM *OPM;
	UNSHORT Member_index;
	UNBYTE *Ptr;

	BatFormSlot = (struct BatFormSlot_object *) Object;
	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(BatForm_window->Window_OPM);

	/* Clear slot area */
	Draw_window_inside
	(
		OPM,
		Object->X - 1,
		Object->Y - 1,
		Object->Rect.width + 2,
		Object->Rect.height + 2
	);

	Draw_deep_border
	(
		OPM,
		Object->X - 1,
		Object->Y - 1,
		Object->Rect.width + 2,
		Object->Rect.height + 2
	);

	/* Is there a party member here ? */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];
	if (Member_index)
	{
		/* Yes -> Draw portrait */
		Ptr = MEM_Claim_pointer(Small_portrait_handles[Member_index - 1]);

		Put_masked_block
		(
			OPM,
			Object->X,
			Object->Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr
		);

		MEM_Free_pointer(Small_portrait_handles[Member_index - 1]);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_BatFormSlot_object
 * FUNCTION  : Feedback method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:37
 * LAST      : 10.09.95 21:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	struct BatFormSlot_object *BatFormSlot;
	struct BatForm_window_object *BatForm_window;
	struct OPM *OPM;
	UNSHORT Member_index;
	UNBYTE *Ptr;

	BatFormSlot = (struct BatFormSlot_object *) Object;
	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(BatForm_window->Window_OPM);

	/* Clear slot area */
	Draw_deep_box
	(
		OPM,
		Object->X - 1,
		Object->Y - 1,
		Object->Rect.width + 2,
		Object->Rect.height + 2
	);

	/* Is there a party member here ? */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];
	if (Member_index)
	{
		/* Yes -> Draw portrait */
		Ptr = MEM_Claim_pointer(Small_portrait_handles[Member_index - 1]);

		Put_masked_block
		(
			OPM,
			Object->X,
			Object->Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr
		);

		Put_recoloured_block
		(
			OPM,
			Object->X,
			Object->Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr,
			&(Recolour_tables[3][0])
		);

		MEM_Free_pointer(Small_portrait_handles[Member_index - 1]);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_BatFormSlot_object
 * FUNCTION  : Highlight method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:38
 * LAST      : 10.09.95 21:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	struct BatFormSlot_object *BatFormSlot;
	struct BatForm_window_object *BatForm_window;
	struct OPM *OPM;
	UNSHORT Member_index;
	UNBYTE *Ptr;

	BatFormSlot = (struct BatFormSlot_object *) Object;
	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(BatForm_window->Window_OPM);

	/* Clear slot area */
	Draw_window_inside
	(
		OPM,
		Object->X - 1,
		Object->Y - 1,
		Object->Rect.width + 2,
		Object->Rect.height + 2
	);

	Draw_deep_border
	(
		OPM,
		Object->X - 1,
		Object->Y - 1,
		Object->Rect.width + 2,
		Object->Rect.height + 2
	);

	/* Is there a party member here ? */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];
	if (Member_index)
	{
		/* Yes -> Draw portrait */
		Ptr = MEM_Claim_pointer(Small_portrait_handles[Member_index - 1]);

		Put_masked_block
		(
			OPM,
			Object->X,
			Object->Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr
		);

		Put_recoloured_block
		(
			OPM,
			Object->X,
			Object->Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr,
			&(Recolour_tables[5][0])
		);

		MEM_Free_pointer(Small_portrait_handles[Member_index - 1]);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_BatFormSlot_object
 * FUNCTION  : Help method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:39
 * LAST      : 10.09.95 21:39
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	struct BatFormSlot_object *BatFormSlot;
	struct BatForm_window_object *BatForm_window;
	UNSHORT Member_index;
	UNCHAR Name1[CHAR_NAME_LENGTH + 1];
	UNCHAR Name2[CHAR_NAME_LENGTH + 1];
	UNCHAR String[100];

	BatFormSlot = (struct BatFormSlot_object *) Object;
	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(Object->Parent);

	/* Get party member index */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];

	/* Anyone here ? */
	if (Member_index)
	{
		/* Yes -> Get character name */
		Get_char_name(Party_char_handles[Member_index - 1], Name1);
	}

	/* In drag & drop mode / dragging a party member ? */
	if (Drag_drop_mode &&
	 (Current_drag_drop_data.Data_ID == BATFORMSLOT_DD_DATA_ID))
	{
		/* Yes -> Get dragged character name */
		Get_char_name(Party_char_handles[Dragged_BatForm_member_index - 1],
		 Name2);

		/* Anyone here ? */
		if (Member_index)
		{
			/* Yes -> Build help string */
			sprintf(String, System_text_ptrs[69], Name1, Name2);
		}
		else
		{
			/* No -> Build help string */
			sprintf(String, System_text_ptrs[674], Name2);
		}

		/* Print help line */
		Execute_method(Help_line_object, SET_METHOD, (void *) String);
	}
	else
	{
		/* Anyone here ? */
		if (Member_index)
		{
			/* Yes -> Print help line */
			Execute_method(Help_line_object, SET_METHOD, (void *) Name1);
		}
		else
		{
			/* No -> Clear help line */
			Execute_method(Help_line_object, SET_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_BatFormSlot_object
 * FUNCTION  : Left method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:41
 * LAST      : 10.09.95 21:41
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	struct BatFormSlot_object *BatFormSlot;
	struct BatForm_window_object *BatForm_window;
	UNSHORT Member_index;

	BatFormSlot = (struct BatFormSlot_object *) Object;
	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(Object->Parent);

	/* Get party member index */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];

	/* Anyone here ? */
	if (Member_index)
	{
		/* Yes -> Clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Drag */
			Drag_BatFormSlot(BatFormSlot);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_BatFormSlot_object
 * FUNCTION  : Drop method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 22:06
 * LAST      : 10.09.95 22:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_BatFormSlot_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_BatFormSlot((struct BatFormSlot_object *) Object,
		 P->Drag_drop_data_ptr);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inquire_drop_BatFormSlot_object
 * FUNCTION  : Inquire drop method of Battle formation slot object.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 22:07
 * LAST      : 10.09.95 22:07
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Inquire_drop_BatFormSlot_object(struct Object *Object,
 union Method_parms *P)
{
	struct Drag_drop_data *Drag_drop_data_ptr;
	UNSHORT Data_ID;

	/* Get drag & drop data */
	Drag_drop_data_ptr = P->Drag_drop_data_ptr;

	/* Get drag & drop data ID */
	Data_ID = Drag_drop_data_ptr->Data_ID;

	/* Right data type ? */
	if (Data_ID == BATFORMSLOT_DD_DATA_ID)
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
 * NAME      : Drag_BatFormSlot
 * FUNCTION  : Drag Battle formation slot.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 22:04
 * LAST      : 11.09.95 12:37
 * INPUTS    : struct BatFormSlot_object *BatFormSlot - Pointer to object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drag_BatFormSlot(struct BatFormSlot_object *BatFormSlot)
{
	struct BatForm_window_object *BatForm_window;
	struct BBRECT MA;
	UNSHORT Member_index;
	UNBYTE *Ptr;

	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(BatFormSlot->Object.Parent);

	/* Get party member index */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];

	/* Anyone here ? */
	if (Member_index)
	{
		/* Yes -> Store member index */
		Dragged_BatForm_member_index = Member_index;

		/* Clear slot */
		BatForm_window->Battle_order_slots[BatFormSlot->Index - 1] = 0;

		/* Redraw Battle formation slot */
		Execute_method(BatFormSlot->Object.Self, DRAW_METHOD, NULL);

		/* Initialize dragging */
		Init_drag(PORTRAIT_WIDTH, PORTRAIT_HEIGHT, 1);

		/* Draw portrait */
		Ptr = MEM_Claim_pointer(Small_portrait_handles[Member_index - 1]);

		Put_unmasked_block
		(
			&Drag_OPM,
			0,
			0,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr
		);

		MEM_Free_pointer(Small_portrait_handles[Member_index - 1]);

		/* Set number of animation frames */
		Drag_HDOB.Nr_frames = 1;

		/* Set HDOB hotspot
		 (+ correction for weakness in BBOPM) */
		Drag_HDOB.X = 3 - BatForm_window->Object.Rect.left;
		Drag_HDOB.Y = 3 - BatForm_window->Object.Rect.top;

		/* Add HDOB */
		Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

		/* Pick mouse pointer */
		Push_mouse(&(Mouse_pointers[PICK_MPTR]));

		/* Build Mouse Area */
		MA.left		= BatForm_window->Object.Rect.left;
		MA.top		= BatForm_window->Object.Rect.top;
		MA.width 	= BatForm_window->Object.Rect.width - PORTRAIT_WIDTH;
		MA.height	= BatForm_window->Object.Rect.height - PORTRAIT_HEIGHT;

		/* Install Mouse Area */
		Push_MA(&MA);

		/* Enter drag & drop mode */
		Enter_drag_drop_mode
		(
			BATFORMSLOT_DD_DATA_ID,
			BatFormSlot_drag_abort_handler,
			&(BatFormSlot->Object),
			NULL
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_BatFormSlot
 * FUNCTION  : Drop on Battle formation slot.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 22:10
 * LAST      : 10.09.95 22:58
 * INPUTS    : struct BatFormSlot_object *BatFormSlot - Pointer to object.
 *             struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_BatFormSlot(struct BatFormSlot_object *BatFormSlot,
 struct Drag_drop_data *Drag_drop_data_ptr)
{
	struct BatForm_window_object *BatForm_window;
	UNSHORT Member_index;
	UNBYTE *Ptr;

	BatForm_window = (struct BatForm_window_object *)
	 Get_object_data(BatFormSlot->Object.Parent);

	/* Get party member index */
	Member_index = BatForm_window->Battle_order_slots[BatFormSlot->Index - 1];

	/* Anyone here ? */
	if (Member_index)
	{
		/* Yes -> Insert party member */
		BatForm_window->Battle_order_slots[BatFormSlot->Index - 1] =
		 Dragged_BatForm_member_index;

		/* Store new member index */
		Dragged_BatForm_member_index = Member_index;

		/* Draw new portrait */
		Ptr = MEM_Claim_pointer(Small_portrait_handles[Member_index - 1]);

		Put_unmasked_block
		(
			&Drag_OPM,
			0,
			0,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr
		);

		MEM_Free_pointer(Small_portrait_handles[Member_index - 1]);

		/* Enter drag & drop mode */
		Enter_drag_drop_mode
		(
			BATFORMSLOT_DD_DATA_ID,
			BatFormSlot_drag_abort_handler,
			&(BatFormSlot->Object),
			NULL
		);

	}
	else
	{
		/* No -> Insert party member */
		BatForm_window->Battle_order_slots[BatFormSlot->Index - 1] =
		 Dragged_BatForm_member_index;

		/* Exit dragging */
		Exit_drag();
		Pop_MA();
		Pop_mouse();
	}

	/* Redraw Battle formation slot */
	Execute_method(BatFormSlot->Object.Self, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BatFormSlot_drag_abort_handler
 * FUNCTION  : Battle formation slot drag & drop abort handler.
 * FILE      : BATFORM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.09.95 21:59
 * LAST      : 10.09.95 21:59
 * INPUTS    : struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BatFormSlot_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr)
{
	/* Is battle formation party member ? */
	if (Drag_drop_data_ptr->Data_ID == BATFORMSLOT_DD_DATA_ID)
	{
		/* Drop the battle formation party member back on the source slot */
		Drop_on_BatFormSlot((struct BatFormSlot_object *)
		 Drag_drop_data_ptr->Source_object, Drag_drop_data_ptr);
	}
}

