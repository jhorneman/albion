/************
 * NAME     : POPUP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 20-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : POPUP.H, USERFACE.C
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <COLOURS.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_PUMES				(20)

/*
 ** Structure definitions **************************************************
 */

/* Pop-up menu OID */
struct PUM_OID {
	struct PUM *Data;
	UNSHORT Nr_entries;
	UNSHORT Entry_list[MAX_PUMES];
};

/* Pop-up menu object */
struct PUM_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	struct PUM *Data;
	UNSHORT Nr_entries;
	UNSHORT Entry_list[MAX_PUMES];
};

/* Pop-up menu entry OID */
struct PUME_OID {
	UNSHORT Number;
};

/* Pop-up menu entry object */
struct PUME_object {
	struct Object Object;
	UNSHORT Number;
};

/*
 ** Prototypes *************************************************************
 */

/* Pop-up menu methods */
UNLONG Init_PUM_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_PUM_object(struct Object *Object, union Method_parms *P);

/* Pop-up menu entry methods */
UNLONG Init_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Left_PUME_object(struct Object *Object, union Method_parms *P);

/*
 ** Global variables *******************************************************
 */

UNSHORT PUM_source_object_handle;

/* Pop-up menu method list */
static struct Method PUM_methods[] = {
	{ INIT_METHOD,		Init_PUM_object },
	{ EXIT_METHOD,		Exit_Window_object },
	{ DRAW_METHOD,		Draw_PUM_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ RIGHT_METHOD,	Close_Window_object },
	{ CLOSE_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Pop-up menu class description */
static struct Object_class PUM_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct PUM_object),
	&PUM_methods[0]
};

/* Pop-up menu entry method list */
static struct Method PUME_methods[] = {
	{ INIT_METHOD,			Init_PUME_object },
	{ DRAW_METHOD,			Draw_PUME_object },
	{ FEEDBACK_METHOD,	Feedback_PUME_object },
	{ HIGHLIGHT_METHOD,	Highlight_PUME_object },
	{ TOUCHED_METHOD,		Normal_touched },
	{ LEFT_METHOD,			Left_PUME_object },
	{ 0, NULL}
};

/* Pop-up menu entry class description */
static struct Object_class PUME_Class = {
	0, sizeof(struct PUME_object),
	&PUME_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_PUM
 * FUNCTION  : Display and handle a pop-up menu.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 18:11
 * LAST      : 21.10.95 19:17
 * INPUTS    : SISHORT X - Left X-coordinate of pop-up menu.
 *             SISHORT Y - Top Y-coordinate of pop-up menu.
 *             struct PUM *Data - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_PUM(SISHORT X, SISHORT Y, struct PUM *Data)
{
	struct PUM_OID PUM_OID;
	struct PUME *PUMES;
	UNSHORT PUM_width, PUM_height;
	UNSHORT PUM_object;
	UNSHORT Nr_entries;
	UNSHORT W;
	UNSHORT i;

	PUMES = Data->PUME_list;

	/* Change to normal mouse pointer */
	Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	/* Unblock und activate all entries */
	for (i=0;i<Data->Total_entries;i++)
	{
		/* Is this entry selectable ? */
		if (!(PUMES[i].Flags & PUME_NOT_SELECTABLE))
		{
			/* Yes -> Unblock and activate entry */
			PUMES[i].Flags &= ~(PUME_ABSENT | PUME_BLOCKED);
		}
	}

	/* Evaluate pop-up menu entries */
	if (Data->Evaluator)
 		(Data->Evaluator)(Data);

	/* Get line width */
	Default_text_style.Style = FAT_STYLE;
	PUM_width = Get_line_width(Data->Title);
	Default_text_style.Style = NORMAL_STYLE;

	/* Set initial height */
	PUM_height = 30;

	/* Scan all entries */
	Nr_entries = 0;
	for (i=0;i<Data->Total_entries;i++)
	{
		/* Is this entry absent ? */
		if (!(PUMES[i].Flags & PUME_ABSENT))
		{
			/* No -> Is this entry selectable ? */
			if (PUMES[i].Flags & PUME_NOT_SELECTABLE)
			{
				/* No -> Is this the first entry ? */
				if (Nr_entries)
				{
					/* No -> Mark entry */
					PUM_OID.Entry_list[Nr_entries] = 0xFFFF;

					/* Increase height */
					PUM_height += 4;

					/* Count up */
					Nr_entries++;
				}
			}
			else
			{
				/* Yes -> Insert entry index in list */
				PUM_OID.Entry_list[Nr_entries] = i;

				/* Increase height */
				PUM_height += 12;

				/* See if this entry is wider than the current PUM width */
				W = Get_line_width(System_text_ptrs[PUMES[i].Button_message_nr]);
				if (W > PUM_width)
					PUM_width = W;

				/* Count up */
				Nr_entries++;
			}
		}
	}

	/* Adjust width */
	PUM_width += 26;

	/* Is the last entry empty ? */
	while (Nr_entries && (PUM_OID.Entry_list[Nr_entries - 1] == 0xFFFF))
	{
		/* Yes -> Decrease height */
		PUM_height -= 4;

		/* Count down */
		Nr_entries--;
	}

	/* Any real entries ? */
	if (!Nr_entries)
	{
		/* No -> Adjust height */
		PUM_height += 12;
	}

	/* Make sure PUM isn't off-screen */
	X = min(max(X, 0), Screen_width - PUM_width);
	Y = min(max(Y, 0), Screen_height - PUM_height);

	/* Make sure PUM doesn't cover the help line */
	if (Y + PUM_height >= Forbidden_Y)
	{
		if (X + PUM_width >= Forbidden_X)
		{
			Y = Forbidden_Y - PUM_height - 1;
		}
	}

	/* Prepare OID */
	PUM_OID.Data			= Data;
	PUM_OID.Nr_entries	= Nr_entries;

	/* Do it */
	Push_module(&Window_Mod);
	PUM_object = Add_object
	(
		0,
		&PUM_Class,
		(UNBYTE *) &PUM_OID,
		X,
		Y,
		PUM_width,
		PUM_height
	);

	Execute_method(PUM_object, DRAW_METHOD, NULL);

	Wait_4_object(PUM_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_centered_PUM
 * FUNCTION  : Display and handle a pop-up menu centered in an area.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 17:44
 * LAST      : 21.10.95 19:17
 * INPUTS    : SISHORT X - Left X-coordinate of area.
 *             SISHORT Y - Top Y-coordinate of area.
 *             UNSHORT Width - Width of area.
 *             UNSHORT Height - Height of area.
 *             struct PUM *Data - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_centered_PUM(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height,
 struct PUM *Data)
{
	struct PUM_OID PUM_OID;
	struct PUME *PUMES;
	UNSHORT PUM_width, PUM_height;
	UNSHORT PUM_object;
	UNSHORT Nr_entries;
	UNSHORT W;
	UNSHORT i;

	PUMES = Data->PUME_list;

	/* Change to normal mouse pointer */
	Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	/* Unblock und activate all entries */
	for (i=0;i<Data->Total_entries;i++)
	{
		/* Is this entry selectable ? */
		if (!(PUMES[i].Flags & PUME_NOT_SELECTABLE))
		{
			/* Yes -> Unblock and activate entry */
			PUMES[i].Flags &= ~(PUME_ABSENT | PUME_BLOCKED);
		}
	}

	/* Evaluate pop-up menu entries */
	if (Data->Evaluator)
 		(Data->Evaluator)(Data);

	/* Get line width */
	Default_text_style.Style = FAT_STYLE;
	PUM_width = Get_line_width(Data->Title);
	Default_text_style.Style = NORMAL_STYLE;

	/* Set initial height */
	PUM_height = 30;

	/* Scan all entries */
	Nr_entries = 0;
	for (i=0;i<Data->Total_entries;i++)
	{
		/* Is this entry absent ? */
		if (!(PUMES[i].Flags & PUME_ABSENT))
		{
			/* No -> Is this entry selectable ? */
			if (PUMES[i].Flags & PUME_NOT_SELECTABLE)
			{
				/* No -> Is this the first entry ? */
				if (Nr_entries)
				{
					/* No -> Mark entry */
					PUM_OID.Entry_list[Nr_entries] = 0xFFFF;

					/* Increase height */
					PUM_height += 4;

					/* Count up */
					Nr_entries++;
				}
			}
			else
			{
				/* Yes -> Insert entry index in list */
				PUM_OID.Entry_list[Nr_entries] = i;

				/* Increase height */
				PUM_height += 12;

				/* See if this entry is wider than the current PUM width */
				W = Get_line_width(System_text_ptrs[PUMES[i].Button_message_nr]);
				if (W > PUM_width)
					PUM_width = W;

				/* Count up */
				Nr_entries++;
			}
		}
	}

	/* Adjust width */
	PUM_width += 26;

	/* Is the last entry empty ? */
	while (Nr_entries && (PUM_OID.Entry_list[Nr_entries - 1] == 0xFFFF))
	{
		/* Yes -> Decrease height */
		PUM_height -= 4;

		/* Count down */
		Nr_entries--;
	}

	/* Any real entries ? */
	if (!Nr_entries)
	{
		/* No -> Adjust height */
		PUM_height += 12;
	}

	/* Centre PUM */
	X = X + (Width - PUM_width) / 2;
	Y = Y + (Height - PUM_height) / 2;

	/* Make sure PUM isn't off-screen */
	X = min(max(X, 0), Screen_width - PUM_width);
	Y = min(max(Y, 0), Screen_height - PUM_height);

	/* Make sure PUM doesn't cover the help line */
	if (Y + PUM_height >= Forbidden_Y)
	{
		if (X + PUM_width >= Forbidden_X)
		{
			Y = Forbidden_Y - PUM_height - 1;
		}
	}

	/* Prepare OID */
	PUM_OID.Data			= Data;
	PUM_OID.Nr_entries	= Nr_entries;

	/* Do it */
	Push_module(&Window_Mod);
	PUM_object = Add_object
	(
		0,
		&PUM_Class,
		(UNBYTE *) &PUM_OID,
		X,
		Y,
		PUM_width,
		PUM_height
	);

	Execute_method(PUM_object, DRAW_METHOD, NULL);

	Wait_4_object(PUM_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_PUM_object
 * FUNCTION  : Initialize method of PUM object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 14:50
 * LAST      : 19.10.94 14:50
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_PUM_object(struct Object *Object, union Method_parms *P)
{
	struct PUM_object *PUM;
	struct PUM_OID *OID;
	struct PUME_OID Entry_OID;
	UNSHORT i, Y;

	PUM = (struct PUM_object *) Object;
	OID = (struct PUM_OID *) P;

	/* Copy data from OID */
	PUM->Data = OID->Data;
	PUM->Nr_entries = OID->Nr_entries;
	for (i=0;i<PUM->Nr_entries;i++)
	{
		PUM->Entry_list[i] = OID->Entry_list[i];
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add pop-up menu entry objects */
	Y = 21;
	for (i=0;i<PUM->Nr_entries;i++)
	{
		if (PUM->Entry_list[i] == 0xFFFF)
		{
			Y += 4;
		}
		else
		{
			Entry_OID.Number = i;
			Add_object
			(
				Object->Self,
				&PUME_Class,
				(UNBYTE *) &Entry_OID,
				9,
				Y,
				Object->Rect.width - 18,
				12
			);
			Y += 12;
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_PUM_object
 * FUNCTION  : Draw method of PUM object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 14:50
 * LAST      : 11.09.95 16:09
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_PUM_object(struct Object *Object, union Method_parms *P)
{
	struct PUM_object *PUM;
	struct OPM *OPM;

	PUM = (struct PUM_object *) Object;
	OPM = &(PUM->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Print title */
	Set_ink(SILVER_TEXT);
	Default_text_style.Style = FAT_STYLE;

	Print_centered_string
	(
		OPM,
		8,
		9,
		Object->Rect.width - 18,
		PUM->Data->Title
	);

	Default_text_style.Style = NORMAL_STYLE;

	/* Print line between title and entries */
	OPM_HorLine
	(
		OPM,
		7,
		18,
		Object->Rect.width - 14,
		221
	);

	/* Draw pop-up menu entries */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_PUME_object
 * FUNCTION  : Initialize method of PUME object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 14:50
 * LAST      : 19.10.94 14:50
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_PUME_object(struct Object *Object, union Method_parms *P)
{
	struct PUME_object *PUME;
	struct PUME_OID *OID;

	PUME = (struct PUME_object *) Object;
	OID = (struct PUME_OID *) P;

	/* Copy data from OID */
	PUME->Number = OID->Number;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_PUME_object
 * FUNCTION  : Draw method of PUME object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 14:50
 * LAST      : 19.10.94 14:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_PUME_object(struct Object *Object, union Method_parms *P)
{
 	struct PUM_object *PUM;
	struct PUME_object *PUME;
	struct OPM *OPM;
	struct PUME *PUME_data;

	PUME = (struct PUME_object *) Object;
	PUM = (struct PUM_object *) Get_object_data(Object->Parent);
	OPM = &(PUM->Window_OPM);
	PUME_data = &(PUM->Data->PUME_list[PUM->Entry_list[PUME->Number]]);

	/* Draw button */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width, 12);
	Draw_high_box(OPM, Object->X, Object->Y, Object->Rect.width, 12);

	/* Set ink colour depending on entry state */
	if (PUME_data->Flags & PUME_BLOCKED)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print entry text */
	Print_centered_string(OPM, Object->X + 4, Object->Y + 2, Object->Rect.width - 8,
	 System_text_ptrs[PUM->Data->PUME_list[PUM->Entry_list[PUME->Number]].Button_message_nr]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_PUME_object
 * FUNCTION  : Feedback method of PUME object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 12:53
 * LAST      : 20.10.94 12:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_PUME_object(struct Object *Object, union Method_parms *P)
{
 	struct PUM_object *PUM;
	struct PUME_object *PUME;
	struct OPM *OPM;
	struct PUME *PUME_data;

	PUME = (struct PUME_object *) Object;
	PUM = (struct PUM_object *) Get_object_data(Object->Parent);
	OPM = &(PUM->Window_OPM);
	PUME_data = &(PUM->Data->PUME_list[PUM->Entry_list[PUME->Number]]);

	/* Draw button */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width, 12);
	Draw_deep_box(OPM, Object->X, Object->Y, Object->Rect.width, 12);

	/* Set ink colour depending on entry state */
	if (PUME_data->Flags & PUME_BLOCKED)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print entry text */
	Print_centered_string(OPM, Object->X + 4, Object->Y + 3, Object->Rect.width - 8,
	 System_text_ptrs[PUM->Data->PUME_list[PUM->Entry_list[PUME->Number]].Button_message_nr]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_PUME_object
 * FUNCTION  : Highlight method of PUME object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 14:43
 * LAST      : 20.10.94 14:43
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_PUME_object(struct Object *Object, union Method_parms *P)
{
 	struct PUM_object *PUM;
	struct PUME_object *PUME;
	struct OPM *OPM;
	struct PUME *PUME_data;

	PUME = (struct PUME_object *) Object;
	PUM = (struct PUM_object *) Get_object_data(Object->Parent);
	OPM = &(PUM->Window_OPM);
	PUME_data = &(PUM->Data->PUME_list[PUM->Entry_list[PUME->Number]]);

	/* Draw button */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width, 12);
	Draw_light_box(OPM, Object->X, Object->Y, Object->Rect.width, 12);

	/* Set ink colour depending on entry state */
	if (PUME_data->Flags & PUME_BLOCKED)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print entry text */
	Print_centered_string(OPM, Object->X + 4, Object->Y + 2, Object->Rect.width - 8,
	 System_text_ptrs[PUM->Data->PUME_list[PUM->Entry_list[PUME->Number]].Button_message_nr]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_PUME_object
 * FUNCTION  : Left method of PUME object.
 * FILE      : POPUP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 14:43
 * LAST      : 20.10.94 14:43
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_PUME_object(struct Object *Object, union Method_parms *P)
{
 	struct PUM_object *PUM;
	struct PUME_object *PUME;
	struct PUM *PUM_data;
	struct PUME *PUME_data;

	/* Get data */
	PUME = (struct PUME_object *) Object;
	PUM = (struct PUM_object *) Get_object_data(Object->Parent);
	PUM_data = PUM->Data;
	PUME_data = &(PUM_data->PUME_list[PUM->Entry_list[PUME->Number]]);

	/* Is this entry blocked ? */
	if (PUME_data->Flags & PUME_BLOCKED)
	{
		/* Yes -> Print blocked prompt (if any) */
		if (PUME_data->Blocked_message_nr)
		{
			Execute_method(Help_line_object, SET_METHOD,
			 (void *) System_text_ptrs[PUME_data->Blocked_message_nr]);
		}
	}

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Is this entry blocked ? */
		if (!(PUME_data->Flags & PUME_BLOCKED))
		{
			/* No -> Close pop-up menu ? */
			if (PUME_data->Flags & PUME_AUTO_CLOSE)
			{
				/* Yes */
				Execute_method(PUM->Object.Self, CLOSE_METHOD, NULL);
			}

			/* Execute function (if any) */
			if (PUME_data->Function)
				(PUME_data->Function)(PUM_data->Data);
		}
	}

	return 0;
}

