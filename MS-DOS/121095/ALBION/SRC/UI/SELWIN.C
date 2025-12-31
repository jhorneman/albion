/************
 * NAME     : SELWIN.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : SELWIN.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <SELWIN.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <PRTLOGIC.H>
#include <STATAREA.H>
#include <COLOURS.H>
#include <SCROLBAR.H>

/* defines */

#define MAX_SELECTABLES					(100)

#define MAX_SELECTABLES_IN_WINDOW	(10)

/* structure definitions */

/* Selection window OID */
struct Select_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
	UNSHORT *Selectable_indices_ptr;
	UNSHORT Nr_selectables;
	Draw_selectable Draw_function;
	UNBYTE *Data;
	UNSHORT Selectable_width;
	UNSHORT Selectable_height;
};

/* Select window object */
struct Select_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Help_messages_ptr;
	UNSHORT *Blocked_messages_ptr;
	UNSHORT *Selectable_indices_ptr;
	UNSHORT Nr_selectables;
	Draw_selectable Draw_function;
	UNBYTE *Data;
	UNSHORT Selectable_width;
	UNSHORT Selectable_height;

	UNSHORT Scroll_bar_object;
};

/* Selectable OID */
struct Selectable_OID {
	UNSHORT Index;
};

/* Selectable object */
struct Selectable_object {
	struct Object Object;
	UNSHORT Index;
};

/* prototypes */

/* Select window methods */
UNLONG Init_Select_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Select_window_object(struct Object *Object, union Method_parms *P);

void Update_Selectable_list(struct Scroll_bar_object *Scroll_bar);

/* Selectable methods */
UNLONG Init_Selectable_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Selectable_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Selectable_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Selectable_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Selectable_object(struct Object *Object, union Method_parms *P);

/* global variables */

/* Select window method list */
static struct Method Select_window_methods[] = {
	{ INIT_METHOD,		Init_Select_window_object },
	{ EXIT_METHOD,		Exit_Window_object },
	{ DRAW_METHOD,		Draw_Select_window_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ RIGHT_METHOD,	Close_Window_object },
	{ CLOSE_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Select window class description */
static struct Object_class Select_window_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Select_window_object),
	&Select_window_methods[0]
};

/* Selectable method list */
static struct Method Selectable_methods[] = {
	{ INIT_METHOD,			Init_Selectable_object },
	{ DRAW_METHOD,			Draw_Selectable_object },
	{ FEEDBACK_METHOD,	Feedback_Selectable_object },
	{ HIGHLIGHT_METHOD,	Highlight_Selectable_object },
	{ LEFT_METHOD,			Left_Selectable_object },
	{ TOUCHED_METHOD,		Normal_touched },
	{ 0, NULL}
};

/* Selectable class description */
static struct Object_class Selectable_Class = {
	0, sizeof(struct Selectable_object),
	&Selectable_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_select_window
 * FUNCTION  : Let the user select something.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 21:10
 * LAST      : 27.09.95 16:43
 * INPUTS    : UNCHAR *Text -Pointer to window text.
 *             Selectable_evaluator Evaluator - Pointer to evaluation
 *              function.
 *             Draw_selectable Draw_function - Pointer to draw function.
 *             UNSHORT Nr_entries - Number of entries to check.
 *             UNSHORT Entry_width - Width of entry in pixels.
 *             UNSHORT Entry_height - Height of entry in pixels.
 *             UNBYTE *Data - Data for evaluation and draw functions.
 * RESULT    : UNSHORT : Selected entry (0...) / 0xFFFF = aborted.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Do_select_window(UNCHAR *Text, Selectable_evaluator Evaluator,
 Draw_selectable Draw_function, UNSHORT Nr_entries, UNSHORT Entry_width,
 UNSHORT Entry_height, UNBYTE *Data)
{
	struct Select_window_OID OID;
	UNSHORT Blocked_message_nrs[MAX_SELECTABLES];
	UNSHORT Selectable_indices[MAX_SELECTABLES];
	UNSHORT Counter;
	UNSHORT Result = 0xFFFF;
	UNSHORT Select_window_object;
	UNSHORT Message_nr;
	UNSHORT i;

	/* Check all entries */
	Counter = 0;
	for (i=0;i<Nr_entries;i++)
	{
		/* Any evaluation function ? */
		if (Evaluator)
		{
			/* Yes -> Evaluate entry */
			Message_nr = (Evaluator)(i, Data);

			/* Absent ? */
			if (Message_nr != 0xFFFF)
			{
				/* No -> Store index */
				Selectable_indices[Counter] = i;

				/* Store message number */
				Blocked_message_nrs[Counter] = Message_nr;

				/* Count up */
				Counter++;
			}
		}
		else
		{
			/* No -> Entry is present */
			/* Store index */
			Selectable_indices[Counter] = i;

			/* Store message number */
			Blocked_message_nrs[Counter] = 0;

			/* Count up */
			Counter++;
		}
	}

	/* Any entries ? */
	if (Counter)
	{
		/* Yes -> Initialize select window OID */
		OID.Text							= Text;
		OID.Result_ptr					= &Result;
		OID.Blocked_messages_ptr	= Blocked_message_nrs;
		OID.Selectable_indices_ptr	= Selectable_indices;
		OID.Nr_selectables			= Counter;
		OID.Draw_function				= Draw_function;
		OID.Data							= Data;
		OID.Selectable_width			= Entry_width;
		OID.Selectable_height		= Entry_height;

		/* Do select window */
		Push_module(&Window_Mod);

		Select_window_object = Add_object
		(
			0,
			&Select_window_Class,
			(UNBYTE *) &OID,
			50,
			50,
			200,
			119
		);

		Execute_method(Select_window_object, DRAW_METHOD, NULL);

		Wait_4_object(Select_window_object);
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Select_window_object
 * FUNCTION  : Init method of Select window object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 20:56
 * LAST      : 27.09.95 16:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Select_window_object(struct Object *Object, union Method_parms *P)
{
	struct Select_window_object *Select_window;
	struct Select_window_OID *OID;
	struct Selectable_OID Selectable_OID;
	struct Scroll_bar_OID Scroll_bar_OID;
	SISHORT Y;
	UNSHORT H;
	UNSHORT i;

	Select_window = (struct Select_window_object *) Object;
	OID = (struct Select_window_OID *) P;

	/* Copy data from OID */
	Select_window->Text							= OID->Text;
	Select_window->Result_ptr					= OID->Result_ptr;
	Select_window->Blocked_messages_ptr		= OID->Blocked_messages_ptr;
	Select_window->Selectable_indices_ptr	= OID->Selectable_indices_ptr;
	Select_window->Nr_selectables				= OID->Nr_selectables;
	Select_window->Draw_function				= OID->Draw_function;
	Select_window->Data							= OID->Data;
	Select_window->Selectable_width			= OID->Selectable_width;
	Select_window->Selectable_height			= OID->Selectable_height;

	/* Scroll bar needed ? */
	if (Select_window->Nr_selectables > MAX_SELECTABLES_IN_WINDOW)
	{
		/* Yes -> Calculate window height WITH scroll bar */
		H = (MAX_SELECTABLES_IN_WINDOW *
		 (Select_window->Selectable_height + 1)) + 26;

		/* Any text ? */
		if (Select_window->Text)
		{
			/* Yes -> More height */
			H += 14;
		}

		/* Set width and height */
		Change_object_size
		(
			Object->Self,
			Select_window->Selectable_width + 27 + BETWEEN + SCROLL_BAR_WIDTH,
			H
		);

		/* Centre window */
		Change_object_position
		(
			Object->Self,
			(Screen_width - Object->Rect.width) / 2,
			(Panel_Y - Object->Rect.height) / 2
		);

		/* Make scroll bar */
		Scroll_bar_OID.Total_units		= Select_window->Nr_selectables;
		Scroll_bar_OID.Units_width		= 1;
		Scroll_bar_OID.Units_height	= MAX_SELECTABLES_IN_WINDOW;
		Scroll_bar_OID.Update			= Update_Selectable_list;

		/* Add object */
		Select_window->Scroll_bar_object = Add_object
		(
			Object->Self,
			&Scroll_bar_Class,
			(UNBYTE *) &Scroll_bar_OID,
			Select_window->Selectable_width + 13 + BETWEEN,
			12,
			SCROLL_BAR_WIDTH,
			(MAX_SELECTABLES_IN_WINDOW *
			 (Select_window->Selectable_height + 1)) - 1
		);

		/* Make selectable objects */
		Y = 12;
	   Selectable_OID.Index = 0;
		for (i=0;i<MAX_SELECTABLES_IN_WINDOW;i++)
		{
			/* Add object */
			Add_object
			(
				Object->Self,
				&Selectable_Class,
				(UNBYTE *) &Selectable_OID,
				13,
				Y,
				Select_window->Selectable_width,
				Select_window->Selectable_height
			);

			/* Increase Y-coordinate */
			Y += Select_window->Selectable_height + 1;

			/* Count up */
			Selectable_OID.Index++;
		}
	}
	else
	{
		/* No -> Calculate window height WITHOUT scroll bar */
		H = (Select_window->Nr_selectables *
		 (Select_window->Selectable_height + 1)) + 26;

		/* Any text ? */
		if (Select_window->Text)
		{
			/* Yes -> More height */
			H += 14;
		}

		/* Set width and height */
		Change_object_size
		(
			Object->Self,
			Select_window->Selectable_width + 27,
			H
		);

		/* Centre window */
		Change_object_position
		(
			Object->Self,
			(Screen_width - Object->Rect.width) / 2,
			(Panel_Y - Object->Rect.height) / 2
		);

		/* Make selectable objects */
		Y = 12;
	   Selectable_OID.Index = 0;
		for (i=0;i<Select_window->Nr_selectables;i++)
		{
			/* Add object */
			Add_object
			(
				Object->Self,
				&Selectable_Class,
				(UNBYTE *) &Selectable_OID,
				13,
				Y,
				Select_window->Selectable_width,
				Select_window->Selectable_height
			);

			/* Increase Y-coordinate */
			Y += Select_window->Selectable_height + 1;

			/* Count up */
			Selectable_OID.Index++;
		}

		/* Indicate there is no scroll bar */
		Select_window->Scroll_bar_object = 0;
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Select_window_object
 * FUNCTION  : Draw method of Select window object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 21:02
 * LAST      : 11.09.95 15:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Select_window_object(struct Object *Object, union Method_parms *P)
{
	struct Select_window_object *Select_window;
	struct OPM *OPM;

	Select_window = (struct Select_window_object *) Object;
	OPM = &(Select_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Any text ? */
	if (Select_window->Text)
	{
		/* Yes -> Draw box around text */
		Draw_deep_box
		(
			OPM,
			12,
			Object->Rect.height - 25,
			Select_window->Selectable_width + 2,
			12
		);

		/* Print text */
		Set_ink(SILVER_TEXT);
		Print_centered_string
		(
			OPM,
			14,
			Object->Rect.height - 23,
			Select_window->Selectable_width - 2,
			Select_window->Text
		);
	}

	/* Draw box around selectables */
	Draw_deep_border
	(
		OPM,
		12,
		11,
		Select_window->Selectable_width + 2,
		Object->Rect.height - 39
	);

	/* Draw selectables */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Selectable_list
 * FUNCTION  : Update the Selectable list (scroll bar function).
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 20:47
 * LAST      : 02.09.95 20:47
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_Selectable_list(struct Scroll_bar_object *Scroll_bar)
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
 * NAME      : Init_Selectable_object
 * FUNCTION  : Init method of Selectable object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.95 22:37
 * LAST      : 24.08.95 22:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Selectable_object(struct Object *Object, union Method_parms *P)
{
	struct Selectable_object *Selectable;
	struct Selectable_OID *OID;

	Selectable = (struct Selectable_object *) Object;
	OID = (struct Selectable_OID *) P;

	/* Copy data from OID */
	Selectable->Index = OID->Index;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Selectable_object
 * FUNCTION  : Draw method of Selectable object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.95 21:59
 * LAST      : 27.09.95 17:04
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Selectable_object(struct Object *Object, union Method_parms *P)
{
	struct Selectable_object *Selectable;
	struct Select_window_object *Select_window;
	struct OPM *OPM;
	UNSHORT Index;
	UNSHORT Real_index;
	UNSHORT Message_nr;

	Selectable = (struct Selectable_object *) Object;
	Select_window = (struct Select_window_object *) Get_object_data(Object->Parent);
	OPM = &(Select_window->Window_OPM);

	/* Clear selectable area */
	Draw_window_inside
	(
		OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Get selectable index */
	Index = Selectable->Index;
	if (Select_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method
		(
			Select_window->Scroll_bar_object,
			GET_METHOD,
			NULL
		);
	}

	/* Get REAL index (jeez) */
	Real_index = *(Select_window->Selectable_indices_ptr + Index);

	/* Get blocked message number */
	Message_nr = *(Select_window->Blocked_messages_ptr + Index);

	/* Draw selectable */
	(Select_window->Draw_function)
	(
		OPM,
		Object,
		Real_index,
		Select_window->Data,
		(BOOLEAN)(Message_nr != 0)
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Selectable_object
 * FUNCTION  : Feedback method of Selectable object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.95 22:00
 * LAST      : 27.09.95 17:04
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Selectable_object(struct Object *Object, union Method_parms *P)
{
	struct Selectable_object *Selectable;
	struct Select_window_object *Select_window;
	struct OPM *OPM;
	UNSHORT Index;
	UNSHORT Real_index;
	UNSHORT Message_nr;

	Selectable = (struct Selectable_object *) Object;
	Select_window = (struct Select_window_object *) Get_object_data(Object->Parent);
	OPM = &(Select_window->Window_OPM);

	/* Clear selectable area */
	Draw_window_inside
	(
		OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);
	Draw_deep_box
	(
		OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	/* Get selectable index */
	Index = Selectable->Index;
	if (Select_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method
		(
			Select_window->Scroll_bar_object,
			GET_METHOD,
			NULL
		);
	}

	/* Get REAL index (jeez) */
	Real_index = *(Select_window->Selectable_indices_ptr + Index);

	/* Get blocked message number */
	Message_nr = *(Select_window->Blocked_messages_ptr + Index);

	/* Draw selectable */
	(Select_window->Draw_function)
	(
		OPM,
		Object,
		Real_index,
		Select_window->Data,
		(BOOLEAN)(Message_nr != 0)
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Selectable_object
 * FUNCTION  : Highlight method of Selectable object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.95 22:01
 * LAST      : 27.09.95 17:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Selectable_object(struct Object *Object, union Method_parms *P)
{
	struct Selectable_object *Selectable;
	struct Select_window_object *Select_window;
	struct OPM *OPM;
	UNSHORT Index;
	UNSHORT Real_index;
	UNSHORT Message_nr;

	Selectable = (struct Selectable_object *) Object;
	Select_window = (struct Select_window_object *) Get_object_data(Object->Parent);
	OPM = &(Select_window->Window_OPM);

	/* Clear selectable area */
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

	/* Get selectable index */
	Index = Selectable->Index;
	if (Select_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method
		(
			Select_window->Scroll_bar_object,
			GET_METHOD,
			NULL
		);
	}

	/* Get REAL index (jeez) */
	Real_index = *(Select_window->Selectable_indices_ptr + Index);

	/* Get blocked message number */
	Message_nr = *(Select_window->Blocked_messages_ptr + Index);

	/* Draw selectable */
	(Select_window->Draw_function)
	(
		OPM,
		Object,
		Real_index,
		Select_window->Data,
		(BOOLEAN)(Message_nr != 0)
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Selectable_object
 * FUNCTION  : Left method of Selectable object.
 * FILE      : SELWIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.95 22:43
 * LAST      : 28.09.95 15:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Selectable_object(struct Object *Object, union Method_parms *P)
{
	struct Selectable_object *Selectable;
	struct Select_window_object *Select_window;
	UNSHORT Index;
	UNSHORT Real_index;
	UNSHORT Message_nr;

	Selectable = (struct Selectable_object *) Object;
	Select_window = (struct Select_window_object *) Get_object_data(Object->Parent);

	/* Get selectable index */
	Index = Selectable->Index;
	if (Select_window->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Select_window->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get REAL index (jeez) */
	Real_index = *(Select_window->Selectable_indices_ptr + Index);

	/* Get blocked message number */
	Message_nr = *(Select_window->Blocked_messages_ptr + Index);

	/* Is blocked ? */
	if (Message_nr)
	{
		/* Yes -> Print blocked message */
		Execute_method
		(
			Help_line_object,
			SET_METHOD,
			(void *) System_text_ptrs[Message_nr]
		);
	}

	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Blocked ? */
		if (!Message_nr)
		{
			/* No -> Select this element */
			*(Select_window->Result_ptr) = Real_index;

			/* Close selection window */
			Execute_method(Select_window->Object.Self, CLOSE_METHOD, NULL);
		}
	}

	return 0;
}

