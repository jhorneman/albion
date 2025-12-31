/************
 * NAME     : MEMBRSEL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MEMBRSEL.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <MEMBRSEL.H>
#include <STATAREA.H>
#include <COLOURS.H>

/* defines */

#define MEMBERSELWIN_WIDTH		(270)
#define MEMBERSELWIN_HEIGHT	(99)

/* structure definitions */

/* Party member select window OID */
struct MemberSel_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
	UNSHORT Abort_blocked_message_nr;
};

/* Party member select window object */
struct MemberSel_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
	UNSHORT Abort_blocked_message_nr;
};

/* Selectable party member OID */
struct SelMember_OID {
	UNSHORT Member_index;
};

/* Selectable party member object */
struct SelMember_object {
	struct Object Object;
	UNSHORT Member_index;
};

/* prototypes */

/* Party member select window object methods */
UNLONG Init_MemberSel_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_MemberSel_object(struct Object *Object, union Method_parms *P);
UNLONG Abort_MemberSel_object(struct Object *Object, union Method_parms *P);

/* Selectable party member object methods */
UNLONG Init_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Help_SelMember_object(struct Object *Object, union Method_parms *P);
UNLONG Left_SelMember_object(struct Object *Object, union Method_parms *P);

/* global variables */

/* Member selection window method list */
static struct Method MemberSel_methods[] = {
	{ INIT_METHOD,		Init_MemberSel_object },
	{ EXIT_METHOD,		Exit_Window_object },
	{ DRAW_METHOD,		Draw_MemberSel_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ RIGHT_METHOD,	Abort_MemberSel_object },
	{ CLOSE_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Member selection window class description */
static struct Object_class MemberSel_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct MemberSel_window_object),
	&MemberSel_methods[0]
};

/* Selectable member method list */
static struct Method SelMember_methods[] = {
	{ INIT_METHOD,			Init_SelMember_object },
	{ DRAW_METHOD,			Draw_SelMember_object },
	{ FEEDBACK_METHOD,	Feedback_SelMember_object },
	{ HIGHLIGHT_METHOD,	Highlight_SelMember_object },
	{ HELP_METHOD,			Help_SelMember_object },
	{ LEFT_METHOD,			Left_SelMember_object },
	{ TOUCHED_METHOD,		Normal_touched },
	{ 0, NULL}
};

/* Selectable member class description */
static struct Object_class SelMember_Class = {
	0, sizeof(struct SelMember_object),
	&SelMember_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_party_member
 * FUNCTION  : Select a party member.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 17:02
 * LAST      : 17.08.95 17:12
 * INPUTS    : UNCHAR *Text - Pointer to window text.
 *             Member_evaluator Evaluator - Pointer to evaluation function.
 *             UNSHORT Abort_blocked_message_nr - Message number to display
 *              if aborting is blocked / 0 (abortable).
 * RESULT    : UNSHORT : Selected member (1...6) /
 *              0 = no members selectable / 0xFFFF = cancelled.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_party_member(UNCHAR *Text, Member_evaluator Evaluator,
 UNSHORT Abort_blocked_message_nr)
{
	struct MemberSel_window_OID OID;
	UNSHORT Blocked_message_nrs[6];
	UNSHORT Selected_member = 0xFFFF;
	UNSHORT Counter;
	UNSHORT Obj;
	UNSHORT i;

	/* Evaluate party members */
	Counter = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Any evaluation function ? */
			if (Evaluator)
			{
				/* Yes -> Evaluate party member */
				Blocked_message_nrs[i] = (Evaluator)(Party_char_handles[i]);

				/* Act depending on status */
				switch (Blocked_message_nrs[i])
				{
					/* Not blocked */
					case 0:
					{
						/* Count up */
						Counter++;
						break;
					}
					/* Absent */
					case 0xFFFF:
					{
						break;
					}
					/* Blocked */
					default:
					{
						break;
					}
				}
			}
			else
			{
				/* No -> Party member is present */
				Blocked_message_nrs[i] = 0;

				/* Count up */
				Counter++;
			}
		}
		else
		{
			/* No -> Absent */
			Blocked_message_nrs[i] = 0xFFFF;
		}
	}

	/* Anyone to choose from ? */
	if (Counter)
	{
		/* Yes -> Prepare selection window OID */
		OID.Text								= Text;
		OID.Result_ptr						= &Selected_member;
		OID.Blocked_messages_ptr		= Blocked_message_nrs;
 		OID.Abort_blocked_message_nr	= Abort_blocked_message_nr;

		/* Do it */
		Push_module(&Window_Mod);

		Obj = Add_object
		(
			0,
			&MemberSel_Class,
			(UNBYTE *) &OID,
			(Screen_width - MEMBERSELWIN_WIDTH) / 2,
			(Panel_Y - MEMBERSELWIN_HEIGHT) / 2,
			MEMBERSELWIN_WIDTH,
			MEMBERSELWIN_HEIGHT
		);

		Execute_method(Obj, DRAW_METHOD, NULL);

		Wait_4_object(Obj);
	}
	else
	{
		/* No -> Indicate this */
		Selected_member = 0;
	}

	return Selected_member;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_MemberSel_object
 * FUNCTION  : Initialize method of Member selection window object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 12:22
 * LAST      : 10.08.95 14:58
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function assumes there will be some party members to
 *              choose from (i.e. at least one).
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_MemberSel_object(struct Object *Object, union Method_parms *P)
{
	struct MemberSel_window_object *MemberSel_window;
	struct MemberSel_window_OID *OID;
	struct SelMember_OID SelMember_OIDs[6];
	UNSHORT Counter;
	UNSHORT X;
	UNSHORT i;

	MemberSel_window = (struct MemberSel_window_object *) Object;
	OID = (struct MemberSel_window_OID *) P;

	/* Copy data from OID */
	MemberSel_window->Text								= OID->Text;
	MemberSel_window->Result_ptr						= OID->Result_ptr;
	MemberSel_window->Blocked_messages_ptr			= OID->Blocked_messages_ptr;
 	MemberSel_window->Abort_blocked_message_nr	= OID->Abort_blocked_message_nr;

	/* Count present members */
	Counter = 0;
	for (i=0;i<6;i++)
	{
		/* Present ? */
		if (*(MemberSel_window->Blocked_messages_ptr + i) != 0xFFFF)
		{
			/* Yes -> Create selectable member object OID */
			SelMember_OIDs[Counter].Member_index = i + 1;

			/* Count up */
			Counter++;
		}
	}

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add selectable members to window */
	X = (MEMBERSELWIN_WIDTH - (Counter * 38) + 4) / 2;
	for (i=0;i<Counter;i++)
	{
	 	Add_object
		(
			Object->Self,
			&SelMember_Class,
			(UNBYTE *) &(SelMember_OIDs[i]),
			X,
			11,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT
		);

		X += PORTRAIT_WIDTH + 4;
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_MemberSel_object
 * FUNCTION  : Draw method of Member selection window object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:01
 * LAST      : 11.09.95 16:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_MemberSel_object(struct Object *Object, union Method_parms *P)
{
	struct MemberSel_window_object *MemberSel_window;
	struct PA PA;
	struct OPM *OPM;

	MemberSel_window = (struct MemberSel_window_object *) Object;
	OPM = &(MemberSel_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Any text ? */
	if (MemberSel_window->Text)
	{
		/* Yes -> Create print area */
		PA.Area.left	= 15;
		PA.Area.top		= 57;
		PA.Area.width	= MEMBERSELWIN_WIDTH - PA.Area.left - 16;
		PA.Area.height	= 28;

		/* Draw box around text */
		Draw_deep_box
		(
			OPM,
			PA.Area.left - 2,
			PA.Area.top - 2,
			PA.Area.width + 4,
			PA.Area.height + 4
		);

		/* Print text */
		Push_PA(&PA);

		Set_ink(SILVER_TEXT);

		Display_text(OPM, MemberSel_window->Text);

		Pop_PA();
	}

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Abort_MemberSel_object
 * FUNCTION  : Abort method of Member selection window object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 14:59
 * LAST      : 10.08.95 14:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Abort_MemberSel_object(struct Object *Object, union Method_parms *P)
{
	struct MemberSel_window_object *MemberSel_window;

	MemberSel_window = (struct MemberSel_window_object *) Object;

	/* Abort blocked ? */
	if (MemberSel_window->Abort_blocked_message_nr)
	{
		/* Yes -> Print blocked message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[MemberSel_window->Abort_blocked_message_nr]);
	}
	else
	{
		/* No -> Abort */
		Close_Window_object(Object, P);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_SelMember_object
 * FUNCTION  : Init method of Selectable member object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 12:59
 * LAST      : 06.04.95 12:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct SelMember_OID *OID;

	SelMember = (struct SelMember_object *) Object;
	OID = (struct SelMember_OID *) P;

	/* Copy data from OID */
	SelMember->Member_index = OID->Member_index;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_SelMember_object
 * FUNCTION  : Draw method of Selectable member object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:05
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	struct OPM *OPM;
	UNBYTE *Ptr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(MemberSel_window->Window_OPM);

	/* Clear portrait area */
	Draw_window_inside(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);
	Draw_deep_border(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);

	/* Is blocked ? */
	if (*(MemberSel_window->Blocked_messages_ptr + SelMember->Member_index - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
		 Object->Rect.height, &(Red_table[0]));
	}

	/* Draw portrait */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	Put_masked_block
	(
		OPM,
		Object->X,
		Object->Y,
		PORTRAIT_WIDTH,
		PORTRAIT_HEIGHT,
		Ptr
	);

	MEM_Free_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_SelMember_object
 * FUNCTION  : Feedback method of Selectable member object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:08
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	struct OPM *OPM;
	UNBYTE *Ptr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(MemberSel_window->Window_OPM);

	/* Clear portrait area */
	Draw_deep_box(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);

	/* Is blocked ? */
	if (*(MemberSel_window->Blocked_messages_ptr + SelMember->Member_index - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
		 Object->Rect.height, &(Red_table[0]));
	}

	/* Draw portrait and feedback */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

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

	MEM_Free_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_SelMember_object
 * FUNCTION  : Highlight method of Selectable member object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 13:08
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	struct OPM *OPM;
	UNBYTE *Ptr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(MemberSel_window->Window_OPM);

	/* Clear portrait area */
	Draw_window_inside(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);
	Draw_deep_border(OPM, Object->X - 1, Object->Y - 1, Object->Rect.width + 2,
	 Object->Rect.height + 2);

	/* Is blocked ? */
	if (*(MemberSel_window->Blocked_messages_ptr + SelMember->Member_index - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
		 Object->Rect.height, &(Red_table[0]));
	}

	/* Draw portrait and highlight */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

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

	MEM_Free_pointer(Small_portrait_handles[SelMember->Member_index - 1]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_SelMember_object
 * FUNCTION  : Help method of Selectable member object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 16:48
 * LAST      : 28.04.95 14:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	MEM_HANDLE Handle;
	UNSHORT Index;
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	SelMember = (struct SelMember_object *) Object;

	/* Get party member index */
	Index = SelMember->Member_index;

	/* Get character name */
	Handle = Party_char_handles[Index - 1];
	Get_char_name(Handle, Name);

	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) Name);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_SelMember_object
 * FUNCTION  : Left method of Selectable member object.
 * FILE      : MEMBRSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 16:39
 * LAST      : 28.04.95 14:18
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_SelMember_object(struct Object *Object, union Method_parms *P)
{
	struct SelMember_object *SelMember;
	struct MemberSel_window_object *MemberSel_window;
	UNSHORT Index;
	UNSHORT Message_nr;

	SelMember = (struct SelMember_object *) Object;
	MemberSel_window = (struct MemberSel_window_object *)
	 Get_object_data(Object->Parent);

	/* Get party member index */
	Index = SelMember->Member_index;

	/* Yes -> Get blocked message number */
	Message_nr = *(MemberSel_window->Blocked_messages_ptr +
	 SelMember->Member_index - 1);

	/* Is this party member blocked ? */
	if (Message_nr)
	{
		/* Yes -> Print blocked message */
		Execute_method(Help_line_object, SET_METHOD,
		 (void *) System_text_ptrs[Message_nr]);
	}

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Is this party member blocked ? */
		if (!Message_nr)
		{
			/* No -> Select this party member */
			*(MemberSel_window->Result_ptr) = Index;

			/* Close party member selection window */
			Execute_method(MemberSel_window->Object.Self, CLOSE_METHOD, NULL);
		}
	}

	return 0;
}

