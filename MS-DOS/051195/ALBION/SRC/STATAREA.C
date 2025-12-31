/************
 * NAME     : STATAREA.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : STATAREA.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <ALBION.H>

#include <TEXT.H>
#include <GFXFUNC.H>
#include <HDOB.H>

#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <ITEMLIST.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <INVENTO.H>
#include <MAGIC.H>
#include <COMBAT.H>
#include <DIALOGUE.H>
#include <COLOURS.H>
#include <GOLDFOOD.H>
#include <MERCHANT.H>
#include <INVITEMS.H>

/* defines */

#define BLINK_DURATION				(2)

#define HELP_LINE_DURATION			(200)
#define HELP_LINE_TEXT_MAX			(80)

#define PERMANENT_TEXT_DURATION	(200)

/* structure definitions */

/* Help-line object */
struct Help_line_object {
	struct Object Object;
	UNSHORT Counter;
	UNCHAR Text[HELP_LINE_TEXT_MAX];
};

/* Permanent text object */
struct Permanent_text_object {
	struct Object Object;
	UNSHORT Counter;
	struct Processed_text Processed;
	struct PA Print_area;
};

/* Party display object */
struct Party_display_object {
	struct Object Object;
	UNSHORT Active_member;
};

/* Party member OID */
struct Party_member_OID {
	UNSHORT Number;
};

/* prototypes */

/* Status area object methods */
UNLONG Init_Status_area_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Status_area_object(struct Object *Object, union Method_parms *P);

/* Party display object methods */
UNLONG Init_Party_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Party_display_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Party_display_object(struct Object *Object, union Method_parms *P);

/* Party member object methods */
UNLONG Init_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG DLeft_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Touch_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Drop_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Inquire_drop_Party_member_object(struct Object *Object, union Method_parms *P);

void Calculate_bar_sizes(UNSHORT Member_nr, UNSHORT *LP_bar_size_ptr,
 UNSHORT *SP_bar_size_ptr);

/* Party member display support functions */
void Draw_party_members(void);
void Highlight_party_members(UNSHORT Index);
void Feedback_party_members(UNSHORT Index);

void Display_party_member_portrait(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Member_nr);

void Display_party_status_bars(void);
void Display_party_damage(void);

/* Party member pop-up menu actions */
void Party_member_PUM_evaluator(struct PUM *PUM);
void PUM_Go_inventory(UNLONG Data);
void PUM_Use_magic(UNLONG Data);
void PUM_Make_active_member(UNLONG Data);
void PUM_Talk_to_member(UNLONG Data);
void PUM_Toggle_member(UNLONG Data);

/* On-line help line object methods */
UNLONG Draw_Help_line_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Help_line_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Help_line_object(struct Object *Object, union Method_parms *P);

/* Permanent text area object methods */
UNLONG Init_Permanent_text_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Permanent_text_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Permanent_text_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Permanent_text_object(struct Object *Object, union Method_parms *P);

/* global variables */

BOOLEAN Hide_status_area = FALSE;

UNSHORT Combat_message_speed = DEFAULT_COMBAT_MESSAGE_SPEED;

/* Status area virtual OPM */
struct OPM Status_area_OPM;

/* Object handles */
static UNSHORT Status_area_object;
UNSHORT Help_line_object;
UNSHORT Permanent_text_object;

UNSHORT Party_display_object;
UNSHORT Party_member_objects[6];

/* Status area object method list */
static struct Method Status_area_methods[] = {
	{ INIT_METHOD,		Init_Status_area_object },
	{ DRAW_METHOD,		Draw_Status_area_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Status area object class definition */
static struct Object_class Status_area_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Object),
	&Status_area_methods[0]
};

/* Help line object method list */
static struct Method Help_line_methods[] = {
	{ DRAW_METHOD,		Draw_Help_line_object },
	{ UPDATE_METHOD,	Update_Help_line_object },
	{ SET_METHOD,		Set_Help_line_object },
	{ 0, NULL}
};

/* Help line object class definition */
static struct Object_class Help_line_Class = {
	0, sizeof(struct Help_line_object),
	&Help_line_methods[0]
};

/* Permanent text object method list */
static struct Method Permanent_text_methods[] = {
	{ INIT_METHOD,		Init_Permanent_text_object },
	{ DRAW_METHOD,		Draw_Permanent_text_object },
	{ UPDATE_METHOD,	Update_Permanent_text_object },
	{ SET_METHOD,		Set_Permanent_text_object },
	{ 0, NULL}
};

/* Permanent text object class definition */
static struct Object_class Permanent_text_Class = {
	0, sizeof(struct Permanent_text_object),
	&Permanent_text_methods[0]
};

/* Party display object method list */
static struct Method Party_display_methods[] = {
	{ INIT_METHOD,		Init_Party_display_object },
	{ DRAW_METHOD,		Draw_Party_display_object },
	{ UPDATE_METHOD,	Update_Party_display_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ 0, NULL}
};

/* Party display object class definition */
static struct Object_class Party_display_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Party_display_object),
	&Party_display_methods[0]
};

/* Party member object method list */
static struct Method Party_member_methods[] = {
	{ INIT_METHOD,				Init_Party_member_object },
	{ DRAW_METHOD,				Draw_Party_member_object },
	{ UPDATE_METHOD,			Update_Party_member_object },
	{ LEFT_METHOD,				Left_Party_member_object },
	{ DLEFT_METHOD,			DLeft_Party_member_object },
	{ DRAG_DLEFT_METHOD,	 	DLeft_Party_member_object },
	{ RIGHT_METHOD,			Normal_rightclicked },
	{ FEEDBACK_METHOD,		Feedback_Party_member_object },
	{ HIGHLIGHT_METHOD,		Highlight_Party_member_object },
	{ POP_UP_METHOD,			Pop_up_Party_member_object },
	{ HELP_METHOD,				Help_Party_member_object },
	{ TOUCHED_METHOD,			Touch_Party_member_object },
	{ DROP_METHOD,				Drop_Party_member_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_Party_member_object },
	{ 0, NULL}
};

/* Party member object class definition */
static struct Object_class Party_member_Class = {
	0, sizeof(struct Party_member_object),
	&Party_member_methods[0]
};

/* Party member pop-up menu entry list */
static struct PUME Party_member_PUMEs[] = {
	{PUME_AUTO_CLOSE,	0,	70,	PUM_Go_inventory},
	{PUME_AUTO_CLOSE,	0,	10,	PUM_Use_magic},
	{PUME_AUTO_CLOSE,	0,	4,		PUM_Make_active_member},
	{PUME_AUTO_CLOSE,	0,	5,		PUM_Talk_to_member},
	{PUME_AUTO_CLOSE,	0,	701,	PUM_Toggle_member}
};

/* Party member pop-up menu */
static struct PUM Party_member_PUM = {
	5,
	NULL,
	0,
	Party_member_PUM_evaluator,
	Party_member_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_status_area
 * FUNCTION  : Initialize the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 13:20
 * LAST      : 28.09.95 14:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_status_area(void)
{
	/* Generate status area OPM */
	OPM_CreateVirtualOPM
	(
		&Main_OPM,
		&Status_area_OPM,
		0,
		Panel_Y,
		Screen_width,
		Screen_height - Panel_Y
	);

	Add_update_OPM(&Status_area_OPM);

	/* Add status area object */
	Status_area_object = Add_object
	(
		Earth_object,
		&Status_area_Class,
		NULL,
		0,
		Panel_Y,
		Screen_width,
		Screen_height - Panel_Y
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Status_area_object
 * FUNCTION  : Initialize method of Status_area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.01.95 14:12
 * LAST      : 28.09.95 14:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Status_area_object(struct Object *Object, union Method_parms *P)
{
	/* Add help-line object */
	Help_line_object = Add_object
	(
		Object->Self,
		&Help_line_Class,
		NULL,
		Forbidden_X,
		Forbidden_Y - Panel_Y,
		176,
		10
	);

	/* Add permanent text object */
	Permanent_text_object = Add_object
	(
		Object->Self,
		&Permanent_text_Class,
		NULL,
		181,
		208 - Panel_Y,
		177,
		30
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Status_area_object
 * FUNCTION  : Draw method of Status area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.01.95 14:14
 * LAST      : 28.09.95 14:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Status_area_object(struct Object *Object, union Method_parms *P)
{
	/* Is the status area hidden ? */
	if (!Hide_status_area)
	{
		/* No ->	Draw child objects */
		Execute_child_methods(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Status_area
 * FUNCTION  : Update the Status area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 17:23
 * LAST      : 28.09.95 14:41
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function is needed to update the status area when it is
 *              not a part of the current object tree.
 *             - This function should only be used as part of a update
 *              method broadcast.
 *             - The check whether the object has children is VITAL!!!
 *              because Execute_broadcast_method does the entire tree when
 *              0 is passed, making endless loops possible!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Status_area(struct Object *Object, union Method_parms *P)
{
	/* Is the status area hidden ? */
	if (!Hide_status_area)
	{
		/* No -> Update status area */
		Execute_broadcast_method(Status_area_object, UPDATE_METHOD, NULL);

		/* Does this object have children ? */
		if (Object->Child)
		{
			/* Yes -> Update children of this object */
			Execute_broadcast_method(Object->Child, UPDATE_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_party_display
 * FUNCTION  : Initialize the party display.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 14:42
 * LAST      : 28.09.95 14:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_party_display(void)
{
	/* Add party display object */
	Party_display_object = Add_object
	(
		Status_area_object,
		&Party_display_Class,
		NULL,
		0,
		0,
		Forbidden_X,
		Screen_height - Panel_Y
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_party_display
 * FUNCTION  : Reset the party display.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 15:19
 * LAST      : 28.09.95 14:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Call this function whenever party members have left or
 *              joined.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_party_display(void)
{
	struct Party_display_object *Party_display;

	/* Get pointer to party display object */
	Party_display = (struct Party_display_object *)
	 Get_object_data(Party_display_object);

	/* Force an update */
	Party_display->Active_member = 0xFFFF;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Party_display_object
 * FUNCTION  : Initialize method of Party display object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 14:37
 * LAST      : 28.09.95 14:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Party_display_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_OID OID;
	UNSHORT i;

	/* Add first five party member objects */
	for (i=0;i<5;i++)
	{
		/* Set member index */
		OID.Number = i + 1;

		Party_member_objects[i] = Add_object
		(
			Object->Self,
			&Party_member_Class,
			(UNBYTE *) &OID,
			4 + i * 28,
			2,
			28,
			46
		);
	}

	/* Add last party member object, which is wider */
	OID.Number = 6;

	Party_member_objects[5] = Add_object
	(
		Object->Self,
		&Party_member_Class,
		(UNBYTE *) &OID,
		4 + i * 28,
		2,
		34,
		46
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Party_display_object
 * FUNCTION  : Draw method of Party display object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 14:37
 * LAST      : 28.09.95 14:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Party_display_object(struct Object *Object, union Method_parms *P)
{
	/* Is the status area hidden ? */
	if (!Hide_status_area)
	{
		/* No -> Draw all party members */
		Draw_party_members();
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Party_display_object
 * FUNCTION  : Update method of Party display object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 14:38
 * LAST      : 28.09.95 14:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Party_display_object(struct Object *Object, union Method_parms *P)
{
	struct Party_display_object *Party_display;

	Party_display = (struct Party_display_object *) Object;

	/* Is the status area hidden ? */
	if (!Hide_status_area)
	{
		/* No -> New active member ? */
		if (PARTY_DATA.Active_member != Party_display->Active_member)
		{
			/* Yes -> Redraw all party members */
			Draw_party_members();

			/* Store active member index */
			Party_display->Active_member = PARTY_DATA.Active_member;
		}

		/* Update children */
		Execute_child_methods(Object->Self, UPDATE_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Party_member_object
 * FUNCTION  : Initialize method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 12:51
 * LAST      : 09.10.95 18:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct Party_member_OID *OID;

	Party_member = (struct Party_member_object *) Object;
	OID = (struct Party_member_OID *) P;

	/* Copy data from OID */
	Party_member->Number = OID->Number;

	/* Get bar sizes */
	Calculate_bar_sizes
	(
		Party_member->Number,
		&(Party_member->LP_bar_size),
		&(Party_member->SP_bar_size)
	);

	/* Get conditions */
	Party_member->Conditions = Get_conditions(Party_char_handles[Party_member->Number - 1]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Party_member_object
 * FUNCTION  : Draw method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 12:51
 * LAST      : 24.10.94 12:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct BBRECT Clip, Old;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Make clipping rectangle */
		Clip.left	= Object->X;
		Clip.top		= Object->Y;
		Clip.width	= PORTRAIT_WIDTH;
		Clip.height	= Object->Rect.height;

		/* Install clip area */
		memcpy(&Old, &(Status_area_OPM.clip), sizeof(struct BBRECT));
		memcpy(&(Status_area_OPM.clip), &Clip, sizeof(struct BBRECT));

		/* Draw party member */
		Draw_party_members();

		/* Restore clip area */
		memcpy(&(Status_area_OPM.clip), &Old, sizeof(struct BBRECT));
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Party_member_object
 * FUNCTION  : Update method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 10:54
 * LAST      : 09.10.95 18:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	BOOLEAN Redraw_flag = FALSE;
	UNSHORT LP_bar_size;
	UNSHORT SP_bar_size;
	UNSHORT Conditions;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Yes -> Any damage being displayed ? */
		if (Party_member->Damage_display_timer)
		{
			/* Yes -> Count down */
			Party_member->Damage_display_timer--;

			/* Redraw */
			Redraw_flag = TRUE;
		}

		/* Get bar sizes */
		Calculate_bar_sizes
		(
			Party_member->Number,
			&LP_bar_size,
			&SP_bar_size
		);

		/* Should the bars be re-drawn ? */
		if ((Party_member->LP_bar_size != LP_bar_size) ||
		 (Party_member->SP_bar_size != SP_bar_size))
		{
			/* Yes -> Adapt bar sizes */
			Party_member->LP_bar_size += sgn((SISHORT) (LP_bar_size -
			 Party_member->LP_bar_size));
			Party_member->SP_bar_size += sgn((SISHORT) (SP_bar_size -
			 Party_member->SP_bar_size));

			/* Redraw */
			Redraw_flag = TRUE;
		}

		/* Get current conditions */
		Conditions = Get_conditions(Party_char_handles[Party_member->Number - 1]);

		/* Have the conditions changed ? */
		if (Party_member->Conditions != Conditions)
		{
			/* Yes -> Set new conditions */
			Party_member->Conditions = Conditions;

			/* Redraw */
			Redraw_flag = TRUE;
		}

		/* Redraw ? */
		if (Redraw_flag)
		{
			/* Yes */
			Draw_Party_member_object(Object, P);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Party_member_object
 * FUNCTION  : Highlight method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 12:51
 * LAST      : 24.10.94 12:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct BBRECT Clip, Old;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Make clipping rectangle */
		Clip.left	= Object->X;
		Clip.top		= Object->Y;
		Clip.width	= PORTRAIT_WIDTH;
		Clip.height	= Object->Rect.height;

		/* Install clip area */
		memcpy(&Old, &(Status_area_OPM.clip), sizeof(struct BBRECT));
		memcpy(&(Status_area_OPM.clip), &Clip, sizeof(struct BBRECT));

		/* Draw party member */
		Highlight_party_members(Party_member->Number);

		/* Restore clip area */
		memcpy(&(Status_area_OPM.clip), &Old, sizeof(struct BBRECT));
	}
	else
	{
		Dehighlight(Object, P);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Party_member_object
 * FUNCTION  : Feedback method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 14:15
 * LAST      : 25.10.94 14:15
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct BBRECT Clip, Old;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Make clipping rectangle */
		Clip.left	= Object->X;
		Clip.top		= Object->Y;
		Clip.width	= PORTRAIT_WIDTH;
		Clip.height	= Object->Rect.height;

		/* Install clip area */
		memcpy(&Old, &(Status_area_OPM.clip), sizeof(struct BBRECT));
		memcpy(&(Status_area_OPM.clip), &Clip, sizeof(struct BBRECT));

		/* Draw party member */
		Feedback_party_members(Party_member->Number);

		/* Restore clip area */
		memcpy(&(Status_area_OPM.clip), &Old, sizeof(struct BBRECT));
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Party_member_object
 * FUNCTION  : Left method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.03.95 11:01
 * LAST      : 02.03.95 11:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Yes -> Try to activate */
		Activate_party_member(Party_member->Number);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : DLeft_Party_member_object
 * FUNCTION  : DLeft method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.03.95 11:01
 * LAST      : 02.03.95 11:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
DLeft_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Yes -> Dehighlight */
		Dehighlight(Object, P);

		/* Enter inventory */
		Go_Inventory(Party_member->Number);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Party_member_object
 * FUNCTION  : Pop-up method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 12:51
 * LAST      : 24.10.94 12:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Pop_up_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	UNCHAR Char_name[CHAR_NAME_LENGTH + 1];

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		Get_char_name(Party_char_handles[Party_member->Number - 1], Char_name);

		Party_member_PUM.Title = Char_name;
		Party_member_PUM.Data = (UNLONG) Party_member->Number;

		PUM_source_object_handle = Object->Self;
		PUM_char_handle = Party_char_handles[Party_member->Number - 1];
		Do_PUM
		(
			Mouse_X + 5,
			Mouse_Y - 40,
			&Party_member_PUM
		);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Party_member_object
 * FUNCTION  : Help method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 14:08
 * LAST      : 25.08.95 14:01
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct Character_data *Char;
	MEM_HANDLE Char_handle;
	UNCHAR String[100];
	UNCHAR Char_name[CHAR_NAME_LENGTH + 1];
	UNCHAR Item_name[ITEM_NAME_LENGTH + 1];

	Party_member = (struct Party_member_object *) Object;

	/* Anyone there ? */
	if (Member_present(Party_member->Number))
	{
		/* Yes -> Get character name */
		Char_handle = Party_char_handles[Party_member->Number - 1];
		Get_char_name(Char_handle, Char_name);

		/* In drag & drop mode ? */
		if (Drag_drop_mode)
		{
			/* Yes -> Act depending on data type */
			switch (Current_drag_drop_data.Data_ID)
			{
				/* Item */
				case BODY_ITEM_DD_DATA_ID:
				case BACKPACK_ITEM_DD_DATA_ID:
				case CHEST_ITEM_DD_DATA_ID:
				case APRES_ITEM_DD_DATA_ID:
				{
					/* Is there a free slot in this character's backpack ? */
					if (Char_inventory_full(Char_handle))
					{
						/* No -> "Has no free slots!" */
						_bprintf(String, 100, System_text_ptrs[596], Char_name);
					}
					else
					{
						/* Yes -> Get dragged item name */
						Get_item_name(&Drag_packet, Item_name);

						/* Make help line string */
						_bprintf(String, 100, System_text_ptrs[593], Item_name, Char_name);
					}
					break;
				}
				/* Gold */
				case GOLD_DD_DATA_ID:
				{
					/* Get character data */
					Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

					/* Can this character carry any more gold ? */
					if (Char->Char_gold < 32767)
					{
						/* Yes -> Make help line string */
						_bprintf(String, 100, System_text_ptrs[594], Char_name);
					}
					else
					{
						/* No -> "Cannot carry any more gold!" */
						_bprintf(String, 100, System_text_ptrs[597], Char_name);
					}

					MEM_Free_pointer(Char_handle);
					break;
				}
				/* Food */
				case FOOD_DD_DATA_ID:
				{
					/* Get character data */
					Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

					/* Can this character carry any more food ? */
					if (Char->Char_food < 32767)
					{
						/* Yes -> Make help line string */
						_bprintf(String, 100, System_text_ptrs[595], Char_name);
					}
					else
					{
						/* No -> "Cannot carry any more food!" */
						_bprintf(String, 100, System_text_ptrs[598], Char_name);
					}

					MEM_Free_pointer(Char_handle);
					break;
				}
			}
		}
		else
		{
			/* No -> Make help line string */
			_bprintf
			(
				String,
				100,
				System_text_ptrs[0],
				Char_name,
				Get_LP(Char_handle),
				Get_SP(Char_handle)
			);
		}

		/* Print help line */
		Execute_method(Help_line_object, SET_METHOD, (void *) String);
	}
	else
	{
		/* Remove help message */
		Execute_method(Help_line_object, SET_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touch_Party_member_object
 * FUNCTION  : Touch method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 09:55
 * LAST      : 11.01.95 09:55
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Touch_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;

	Party_member = (struct Party_member_object *) Object;

	/* Anyone here OR in drag & drop mode ? */
	if (PARTY_DATA.Member_nrs[Party_member->Number - 1] || Drag_drop_mode)
	{
		/* Yes -> Touch normally */
		Normal_touched(Object, NULL);
	}
	else
	{
		/* No -> Dehighlight */
		Dehighlight(Object, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Party_member_object
 * FUNCTION  : Drop method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.08.95 12:10
 * LAST      : 30.09.95 14:07
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct Drag_drop_data *Drag_drop_data_ptr;
	struct Character_data *Char;
	MEM_HANDLE Char_handle;
	UNSHORT Added_quantity;
	UNSHORT Data_ID;

	Party_member = (struct Party_member_object *) Object;

	/* Exit if no-one is there */
	if (!Member_present(Party_member->Number))
		return 0;

	/* Get character data handle */
	Char_handle = Party_char_handles[Party_member->Number - 1];

	/* Get drag & drop data */
	Drag_drop_data_ptr = P->Drag_drop_data_ptr;

	/* Get drag & drop data ID */
	Data_ID = Drag_drop_data_ptr->Data_ID;

	/* Act depending on data type */
	switch (Data_ID)
	{
		/* Item */
		case BODY_ITEM_DD_DATA_ID:
		case BACKPACK_ITEM_DD_DATA_ID:
		case CHEST_ITEM_DD_DATA_ID:
		case APRES_ITEM_DD_DATA_ID:
		{
			/* Is there a free slot in this character's backpack ? */
			if (!Char_inventory_full(Char_handle))
			{
				/* Yes -> Give item to character */
				Put_item_in_character
				(
					Char_handle,
					0,
					(UNSHORT) Drag_packet.Quantity,
					&Drag_packet
				);

				/* Gave it to inventory character ? */
				if (Party_member->Number == Inventory_member)
				{
					/* Yes -> Redraw backpack item list */
					Execute_method(Backpack_item_list_object, DRAW_METHOD, NULL);
				}

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
					Pop_mouse();

					/* In merchant screen ? */
					if (In_Merchant)
					{
						/* Yes -> Remove mouse area */
						Pop_MA();
					}
				}
			}
			break;
		}
		/* Gold */
		case GOLD_DD_DATA_ID:
		{
			/* Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

			/* How many will fit in this object ? */
			Added_quantity = min(Drag_quantity, 32767 - Char->Char_gold);

			/* Any ? */
			if (Added_quantity)
			{
				/* Yes -> Give gold */
				Char->Char_gold += Added_quantity;
				Drag_quantity -= Added_quantity;

				MEM_Free_pointer(Char_handle);

				/* Everything transferred ? */
				if (Drag_quantity)
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
					Pop_mouse();
				}
			}
			break;
		}
		/* Food */
		case FOOD_DD_DATA_ID:
		{
			/* Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

			/* How many will fit in this object ? */
			Added_quantity = min(Drag_quantity, 32767 - Char->Char_food);

			/* Any ? */
			if (Added_quantity)
			{
				/* Yes -> Give food */
				Char->Char_food += Added_quantity;
				Drag_quantity -= Added_quantity;

				MEM_Free_pointer(Char_handle);

				/* Everything transferred ? */
				if (Drag_quantity)
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
					Pop_mouse();
				}
			}
			break;
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inquire_drop_Party_member_object
 * FUNCTION  : Inquire drop method of Party member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.08.95 12:10
 * LAST      : 30.09.95 14:07
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Inquire_drop_Party_member_object(struct Object *Object,
 union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct Drag_drop_data *Drag_drop_data_ptr;
	UNLONG Result = 0;
	UNSHORT Data_ID;

	Party_member = (struct Party_member_object *) Object;

	/* Exit if no-one is there */
	if (!Member_present(Party_member->Number))
		return 0;

	/* Get drag & drop data */
	Drag_drop_data_ptr = P->Drag_drop_data_ptr;

	/* Get drag & drop data ID */
	Data_ID = Drag_drop_data_ptr->Data_ID;

	/* Act depending on data type */
	switch (Data_ID)
	{
		/* Item */
		case BODY_ITEM_DD_DATA_ID:
		case BACKPACK_ITEM_DD_DATA_ID:
		case CHEST_ITEM_DD_DATA_ID:
		case APRES_ITEM_DD_DATA_ID:
		{
			/* Can accept */
			Result = 1;
			break;
		}
		/* Gold */
		case GOLD_DD_DATA_ID:
		{
			/* Can accept */
			Result = 1;
			break;
		}
		/* Food */
		case FOOD_DD_DATA_ID:
		{
			/* Can accept */
			Result = 1;
			break;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_bar_sizes
 * FUNCTION  : Calculate LP & SP bar sizes.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.09.95 11:13
 * LAST      : 29.09.95 11:13
 * INPUTS    : UNSHORT Member_nr - Member index (1...6).
 *             UNSHORT *LP_bar_size_ptr - Pointer to LP bar size.
 *             UNSHORT *SP_bar_size_ptr - Pointer to SP bar size.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_bar_sizes(UNSHORT Member_nr, UNSHORT *LP_bar_size_ptr,
 UNSHORT *SP_bar_size_ptr)
{
	MEM_HANDLE Char_handle;
	UNSHORT LP;
	UNSHORT SP;
	UNSHORT Max_LP;
	UNSHORT Max_SP;
	UNSHORT LP_bar_size = 0;
	UNSHORT SP_bar_size = 0;

	/* Anyone there ? */
	if (Member_present(Member_nr))
	{
		/* Yes -> Get character handle */
		Char_handle = Party_char_handles[Member_nr- 1];

		/* Get LP and max LP */
		LP			= Get_LP(Char_handle);
		Max_LP	= Get_max_LP(Char_handle);

		/* Any LP / max LP ? */
		if (LP && Max_LP)
		{
			/* Yes -> Calculate life-points bar size */
			LP_bar_size = min(max((LP * 20) / Max_LP, 1), 20);
		}
		else
		{
			/* No -> No bar */
			LP_bar_size = 0;
		}

		/* Get SP and max SP */
		SP			= Get_SP(Char_handle);
		Max_SP	= Get_max_SP(Char_handle);

		/* Any SP / max SP ? */
		if (SP && Max_SP)
		{
			/* Yes -> Calculate spell-points bar size */
			SP_bar_size = min(max((SP * 20) / Max_SP, 1), 20);
		}
		else
		{
			/* No -> No bar */
			SP_bar_size = 0;
		}
	}

	/* Store */
	*LP_bar_size_ptr = LP_bar_size;
	*SP_bar_size_ptr = SP_bar_size;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_party_members
 * FUNCTION  : Draw all party members in the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 12:51
 * LAST      : 14.10.95 14:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_party_members(void)
{
	UNSHORT i;

	/* Clear party display */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Status_area_OPM,
		0,
		Panel_Y,
		180,
		48,
		0,
		0
	);

	/* Display non-active party members' portraits */
	for (i=0;i<6;i++)
	{
		/* Is there anyone here and is he or she not active ? */
		if (Member_present(i + 1) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Display portrait */
			Display_party_member_portrait
			(
				&Status_area_OPM,
				4 + i * 28,
				5,
				i + 1
			);
		}
	}

	/* Is there an active party member ? */
	if ((PARTY_DATA.Active_member > 0) && (PARTY_DATA.Active_member <= 6))
	{
		/* Yes -> Display the active party member's portrait */
		Display_party_member_portrait
		(
			&Status_area_OPM,
			4 + (PARTY_DATA.Active_member - 1) * 28,
			2,
			PARTY_DATA.Active_member
		);
	}

	/* Display LP & SP bars */
	Display_party_status_bars();

	/* Display damage */
	Display_party_damage();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_party_members
 * FUNCTION  : Draw all party members in the status area, highlighting one.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 17:38
 * LAST      : 14.10.95 14:45
 * INPUTS    : UNSHORT Index - Index of the party member that should be
 *              highlighted (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Highlight_party_members(UNSHORT Index)
{
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear party display */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Status_area_OPM,
		0,
		Panel_Y,
		180,
		48,
		0,
		0
	);

	/* Display non-active party members' portraits */
	for (i=0;i<6;i++)
	{
		/* Is there anyone here and is he or she not active ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Display portrait */
			Display_party_member_portrait
			(
				&Status_area_OPM,
				4 + i * 28,
				5,
				i + 1
			);

			/* Is this the highlighted member ? */
			if (i == Index - 1)
			{
				/* Yes -> Display highlight */
				Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

				Put_recoloured_block
				(
					&Status_area_OPM,
					4 + i * 28,
					5,
					PORTRAIT_WIDTH,
					PORTRAIT_HEIGHT,
					Ptr,
					&(Recolour_tables[5][0])
				);

				MEM_Free_pointer(Small_portrait_handles[i]);
			}
		}
	}

	/* Is there an active party member ? */
	if ((PARTY_DATA.Active_member > 0) && (PARTY_DATA.Active_member <= 6))
	{
		/* Yes -> Display the active party member's portrait */
		Display_party_member_portrait
		(
			&Status_area_OPM,
			4 + (PARTY_DATA.Active_member - 1) * 28,
			2,
			PARTY_DATA.Active_member
		);

		/* Is this the highlighted member ? */
		if (PARTY_DATA.Active_member == Index)
		{
			/* Yes -> Display highlight */
			Ptr = MEM_Claim_pointer(Small_portrait_handles[Index - 1]);

			Put_recoloured_block
			(
				&Status_area_OPM,
				4 + (PARTY_DATA.Active_member - 1) * 28,
				2,
				PORTRAIT_WIDTH,
				PORTRAIT_HEIGHT,
				Ptr,
				&(Recolour_tables[5][0])
			);

			MEM_Free_pointer(Small_portrait_handles[Index - 1]);
		}
	}

	/* Display LP & SP bars */
	Display_party_status_bars();

	/* Display damage */
	Display_party_damage();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_party_members
 * FUNCTION  : Draw all party members in the status area, "feedbacking" one.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 14:14
 * LAST      : 14.10.95 14:46
 * INPUTS    : UNSHORT Index - Index of the party member that should be
 *              "feedbacked" (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Feedback_party_members(UNSHORT Index)
{
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear party display */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Status_area_OPM,
		0,
		Panel_Y,
		180,
		48,
		0,
		0
	);

	/* Display non-active party members' portraits */
	for (i=0;i<6;i++)
	{
		/* Is there anyone here and is he or she not active ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Display portrait */
			Display_party_member_portrait
			(
				&Status_area_OPM,
				4 + i * 28,
				5,
				i + 1
			);

			/* Is this the feedbacked member ? */
			if (i == Index - 1)
			{
				/* Yes -> Display feedback */
				Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

				Put_recoloured_block
				(
					&Status_area_OPM,
					4 + i * 28,
					5,
					PORTRAIT_WIDTH,
					PORTRAIT_HEIGHT,
					Ptr,
					&(Recolour_tables[3][0])
				);

				MEM_Free_pointer(Small_portrait_handles[i]);
			}
		}
	}

	/* Is there an active party member ? */
	if ((PARTY_DATA.Active_member > 0) && (PARTY_DATA.Active_member <= 6))
	{
		/* Yes -> Display the active party member's portrait */
		Display_party_member_portrait
		(
			&Status_area_OPM,
			4 + (PARTY_DATA.Active_member - 1) * 28,
			2,
			PARTY_DATA.Active_member
		);

		/* Is this the feedbacked member ? */
		if (PARTY_DATA.Active_member == Index)
		{
			/* Yes -> Display feedback */
			Ptr = MEM_Claim_pointer(Small_portrait_handles[Index - 1]);

			Put_recoloured_block
			(
				&Status_area_OPM,
				4 + (PARTY_DATA.Active_member - 1) * 28,
				2,
				PORTRAIT_WIDTH,
				PORTRAIT_HEIGHT,
				Ptr,
				&(Recolour_tables[3][0])
			);

			MEM_Free_pointer(Small_portrait_handles[Index - 1]);
		}
	}

	/* Display LP & SP bars */
	Display_party_status_bars();

	/* Display damage */
	Display_party_damage();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_party_member_portrait
 * FUNCTION  : Display a party member's portrait.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 22:12
 * LAST      : 05.10.95 17:37
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Member_nr - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_party_member_portrait(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Member_nr)
{
	UNBYTE *Ptr;

	/* Exit if this party member doesn't exist */
	if (!Member_present(Member_nr))
		return;

	/* Get portrait graphics address */
	Ptr = MEM_Claim_pointer(Small_portrait_handles[Member_nr - 1]);

	/* Is this person "dead" ? */
	if (Get_conditions(Party_char_handles[Member_nr - 1]) & DEAD_MASK)
	{
		/* Yes -> Indicate */
		Put_silhouette
		(
			OPM,
			X,
			Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr,
			DARKER
		);
	}
	else
	{
		/* No -> Display portrait */
		Put_masked_block
		(
			OPM,
			X,
			Y,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Ptr
		);

		/* Is this person unhealthy ? */
		if (Get_conditions(Party_char_handles[Member_nr - 1]))
		{
			/* Yes -> Display sick symbol over portrait */
			Put_masked_block
			(
				OPM,
				X + 14,
				Y + 11,
				14,
				13,
				&(Sick_symbol[0])
			);
		}
	}

	MEM_Free_pointer(Small_portrait_handles[Member_nr - 1]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_party_status_bars
 * FUNCTION  : Draw all party members' status bars in the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 14:12
 * LAST      : 11.09.95 15:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_party_status_bars(void)
{
	struct Party_member_object *Party_member;
	UNSHORT LP_bar_size;
	UNSHORT SP_bar_size;
	UNSHORT i;

	/* Display LP & SP bars */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Has magical abilities ? */
			if (Character_has_magical_abilities(Party_char_handles[i]))
			{
				/* Yes -> Is this the active member ? */
				if (i == PARTY_DATA.Active_member - 1)
				{
					/* Yes -> Indicate */
					OPM_FillBox
					(
						&Status_area_OPM,
						4 + i * 28 + 5,
						38,
						22,
						7,
						BRIGHT
					);
				}
				else
				{
					/* No */
					OPM_FillBox
					(
						&Status_area_OPM,
						4 + i * 28 + 5,
						38,
						22,
						7,
						BLACK
					);
				}

				/* Get bar sizes */
				Party_member = (struct Party_member_object *)
				 Get_object_data(Party_member_objects[i]);

				LP_bar_size = Party_member->LP_bar_size;
				SP_bar_size = Party_member->SP_bar_size;

				/* Display LP bar */
				OPM_FillBox
				(
					&Status_area_OPM,
					4 + i * 28 + 6,
					39,
					LP_bar_size,
					2,
					GREEN + 1
				);

				OPM_FillBox
				(
					&Status_area_OPM,
					4 + i * 28 + 6 + LP_bar_size,
					39,
					20 - LP_bar_size,
					2,
					GREEN + 3
				);

				/* Display SP bar */
				OPM_FillBox
				(
					&Status_area_OPM,
					4 + i * 28 + 6,
					42,
					SP_bar_size,
					2,
					TURQUOISE + 1
				);

				OPM_FillBox
				(
					&Status_area_OPM,
					4 + i * 28 + 6 + SP_bar_size,
					42,
					20 - SP_bar_size,
					2,
					TURQUOISE + 3
				);
			}
			else
			{
				/* Yes -> Is this the active member ? */
				if (i == PARTY_DATA.Active_member - 1)
				{
					/* Yes -> Indicate */
					OPM_FillBox
					(
						&Status_area_OPM,
						4 + i * 28 + 5,
						38,
						22,
						4,
						BRIGHT
					);
				}
				else
				{
					/* No */
					OPM_FillBox
					(
						&Status_area_OPM,
						4 + i * 28 + 5,
						38,
						22,
						4,
						BLACK
					);
				}

				/* Get bar size */
				Party_member = (struct Party_member_object *)
				 Get_object_data(Party_member_objects[i]);

				LP_bar_size = Party_member->LP_bar_size;

				/* Display LP bar */
				OPM_FillBox
				(
					&Status_area_OPM,
					4 + i * 28 + 6,
					39,
					LP_bar_size,
					2,
					GREEN + 1
				);

				OPM_FillBox
				(
					&Status_area_OPM,
					4 + i * 28 + 6 + LP_bar_size,
					39,
					20 - LP_bar_size,
					2,
					GREEN + 3
				);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_party_damage
 * FUNCTION  : Draw all party members' damage displays in the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.09.95 11:31
 * LAST      : 05.10.95 15:57
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_party_damage(void)
{
	struct Party_member_object *Party_member;
	UNSHORT i;
	UNCHAR Number[10];

	/* Set ink colour */
	Set_ink(WHITE_TEXT);

	/* Display damage */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get party member object */
			Party_member = (struct Party_member_object *)
			 Get_object_data(Party_member_objects[i]);

			/* Any damage ? */
			if ((Party_member->Damage) && (Party_member->Damage_display_timer))
			{
				/* Yes -> Damage or healing ? */
				if (Party_member->Damage > 0)
				{
					/* Healing -> Draw healing symbol */
					Put_masked_block
					(
						&Status_area_OPM,
						4 + i * 28 - 1,
						5,
						32,
						32,
						&(Healing_symbol[0])
					);

					/* Build healing string */
					_bprintf(Number, 10, "%u", Party_member->Damage);

					/* Display healing */
					Print_centered_string
					(
						&Status_area_OPM,
						4 + i * 28 - 3,
						5 + 14 - 3,
						32,
						Number
					);
				}
				else
				{
					/* Damage -> Draw pain symbol */
					Put_masked_block
					(
						&Status_area_OPM,
						4 + i * 28 - 1,
						5,
						32,
						32,
						&(Pain_symbol[0])
					);

					/* Build damage string */
					_bprintf(Number, 10, "%u", 0 - Party_member->Damage);

					/* Display damage */
					Print_centered_string
					(
						&Status_area_OPM,
						4 + i * 28 - 2,
						5 + 14,
						32,
						Number
					);
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_member_PUM_evaluator
 * FUNCTION  : Evaluate party member pop-up menu.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.12.94 12:22
 * LAST      : 26.09.95 21:55
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Party_member_PUM_evaluator(struct PUM *PUM)
{
	struct PUME *PUMES;
	MEM_HANDLE Char_handle;

	PUMES = PUM->PUME_list;
	Char_handle = Party_char_handles[(UNSHORT) PUM->Data - 1];

	/* Are we in this member's inventory screen ? */
	if (In_Inventory && (((UNSHORT) PUM->Data == Inventory_member)))
	{
		/* Yes -> Cannot access inventory */
		PUMES[0].Flags |= PUME_ABSENT;
	}
	else
	{
 		/* Can this member's inventory be accessed ? */
		if (Get_conditions(Char_handle) & INVENTORY_MASK)
		{
			/* No -> Cannot access inventory */
			PUMES[0].Flags						|= PUME_BLOCKED;
			PUMES[0].Blocked_message_nr	= 757;
		}
	}

	/* Does this character have magical abilities / not in combat ? */
	if (Character_has_magical_abilities(Char_handle) &&
	 !(Get_conditions(Char_handle) & MAGIC_MASK) &&
	 !In_Combat)
	{
		/* Yes -> Know any spells ? */
		if (!(Character_knows_spells(Char_handle)))
		{
			/* No -> Block use magic */
			PUMES[1].Flags |= PUME_BLOCKED;

			/* Explain */
			PUMES[1].Blocked_message_nr = 425;
		}
	}
	else
	{
		/* No -> Cannot use magic */
		PUMES[1].Flags |= PUME_ABSENT;
	}

	/* Is this the active party member ? */
	if ((UNSHORT) PUM->Data == PARTY_DATA.Active_member)
	{
		/* Yes -> Cannot make leader */
		PUMES[2].Flags |= PUME_ABSENT;
	}
	else
	{
		/* No -> Can this character be active ? */
		if (Get_conditions(Char_handle) & ACTIVE_MASK)
		{
			/* No -> Block make active */
			PUMES[2].Flags |= PUME_BLOCKED;

			/* Explain */
			PUMES[2].Blocked_message_nr = 542;
		}
	}

	/* Is this the main character / are we in combat or in an inventory
	 screen ? */
	if (((UNSHORT) PUM->Data == 1) || In_Combat || In_Inventory)
	{
		/* Yes -> Cannot talk */
		PUMES[3].Flags |= PUME_ABSENT;
	}
	else
	{
		/* No -> Can this character be talked to ? */
		if (Get_conditions(Char_handle) & TALK_MASK)
		{
			/* No -> Block talking */
			PUMES[3].Flags |= PUME_BLOCKED;

			/* Explain */
			PUMES[3].Blocked_message_nr = 512;
		}
	}

	/* In cheat mode ? */
	if (!Cheat_mode)
	{
		/* No -> Cannot toggle */
		PUMES[4].Flags |= PUME_ABSENT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Go_inventory_1.
 * FUNCTION  : Go to inventory mode 1 (party member pop-up menu).
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 13:49
 * LAST      : 09.01.95 18:15
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Go_inventory(UNLONG Data)
{
	Go_Inventory((UNSHORT) Data);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Use_magic
 * FUNCTION  : Use magic (party member pop-up menu).
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 05.04.95 13:37
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Use_magic(UNLONG Data)
{
	Cast_spell((UNSHORT) Data, 0xFFFF);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Make_active_member
 * FUNCTION  : Make someone the active member (party member pop-up menu).
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 13:49
 * LAST      : 29.12.94 13:49
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Make_active_member(UNLONG Data)
{
	Activate_party_member((UNSHORT) Data);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Talk_to_member
 * FUNCTION  : Talk to a party member (party member pop-up menu).
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 13:49
 * LAST      : 30.06.95 15:23
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Talk_to_member(UNLONG Data)
{
	BOOLEAN Result;

	/* Is the main character the active member ? */
	if (PARTY_DATA.Active_member != 1)
	{
		/* No -> Try to make the main character the active member */
		Result = Activate_party_member(1);

		/* Success ? */
		if (!Result)
		{
			/* No -> Error */
			return;
		}
	}

	/* Start dialogue with party member */
	Do_Dialogue
	(
		NORMAL_DIALOGUE,
		PARTY_CHAR_TYPE,
		PARTY_DATA.Member_nrs[(UNSHORT) Data - 1],
		0xFFFF
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Toggle_member
 * FUNCTION  : Toggle a party member (party member pop-up menu).
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 21:53
 * LAST      : 26.09.95 21:53
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Toggle_member(UNLONG Data)
{
	UNSHORT Member_nr;
	UNSHORT LP;

	Member_nr = (UNSHORT) Data;

	LP = Get_LP(Party_char_handles[Member_nr - 1]);

	if (LP)
	{
		Cheat_mode = FALSE;
		Do_damage(Member_nr, LP);
		Cheat_mode = TRUE;
	}
	else
	{
		Do_healing(Member_nr, Get_max_LP(Party_char_handles[Member_nr - 1]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Help_line_object
 * FUNCTION  : Draw method of Help_line object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.10.94 10:26
 * LAST      : 26.10.94 10:26
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Help_line_object(struct Object *Object, union Method_parms *P)
{
	struct Help_line_object *Help_line;
	struct BBRECT Clip, Old;

	Help_line = (struct Help_line_object *) Object;

	/* Erase help line */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Status_area_OPM,
		Object->Rect.left,
		Object->Rect.top,
		Object->Rect.width,
		Object->Rect.height,
		Object->X,
		Object->Y
	);

	/* Is there a message ? */
	if (Help_line->Counter)
	{
		/* Yes -> Is it a new message ? */
		if (Help_line->Counter > HELP_LINE_DURATION - BLINK_DURATION)
		{
			/* Yes -> Light up */
			Put_recoloured_box
			(
				&Status_area_OPM,
				Object->X,
				Object->Y,
				Object->Rect.width,
				Object->Rect.height,
				&(Recolour_tables[5][0])
			);
		}

		/* Make clipping rectangle */
		Clip.left	= Object->X;
		Clip.top		= Object->Y;
		Clip.width	= Object->Rect.width;
		Clip.height	= Object->Rect.height;

		/* Install clip area */
		memcpy(&Old, &(Status_area_OPM.clip), sizeof(struct BBRECT));
		memcpy(&(Status_area_OPM.clip), &Clip, sizeof(struct BBRECT));

		/* Print it */
		Push_textstyle(&Default_text_style);
		Set_ink(SILVER_TEXT);
		Print_centered_string
		(
			&Status_area_OPM,
			Object->X,
			Object->Y + 1,
			Object->Rect.width,
			&(Help_line->Text[0])
		);
		Pop_textstyle();

		/* Restore clip area */
		memcpy(&(Status_area_OPM.clip), &Old, sizeof(struct BBRECT));
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Help_line_object
 * FUNCTION  : Update method of Help_line object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.10.94 10:26
 * LAST      : 26.10.94 10:26
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Help_line_object(struct Object *Object, union Method_parms *P)
{
	struct Help_line_object *Help_line;
	UNSHORT i;

	Help_line = (struct Help_line_object *) Object;

	/* Is there a message ? */
	i = Help_line->Counter;
	if (i)
	{
		/* Yes -> Count down */
		Help_line->Counter--;

		/* Is it a new message or is it time to erase ? */
		if ((i > HELP_LINE_DURATION - BLINK_DURATION) || (i == 1))
		{
			/* Yes -> Print message */
			Execute_method(Object->Self, DRAW_METHOD, NULL);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Help_line_object
 * FUNCTION  : Set method of Help_line object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.10.94 10:26
 * LAST      : 27.10.95 21:52
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Help_line_object(struct Object *Object, union Method_parms *P)
{
	struct Help_line_object *Help_line;

	Help_line = (struct Help_line_object *) Object;

	/* Is there a message ? */
	if ((UNCHAR *) P)
	{
		/* Yes -> Is it a new message ? */
		if (strncmp(&(Help_line->Text[0]), (UNCHAR *) P, HELP_LINE_TEXT_MAX - 1)
		 || (!Help_line->Counter))
		{
			/* Yes -> Copy message */
			strncpy(&(Help_line->Text[0]), (UNCHAR *) P, HELP_LINE_TEXT_MAX - 1);
		}

		/* Set counter */
		Help_line->Counter = HELP_LINE_DURATION;
	}
	else
	{
		/* No -> Erase message */
		Help_line->Text[0] = 0;
		Help_line->Counter = 0;

		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_help_line
 * FUNCTION  : Update the Help_line object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.10.94 11:49
 * LAST      : 23.06.95 19:58
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function is needed to update the help line when it is
 *              not a part of the current object tree.
 *             - This function should only be used as part of a update
 *              method broadcast.
 *             - The check whether the object has children is VITAL!!!
 *              because Execute_broadcast_method does the entire tree when
 *              0 is passed, making endless loops possible!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_help_line(struct Object *Object, union Method_parms *P)
{
	/* Update help line */
	Execute_method(Help_line_object, UPDATE_METHOD, NULL);

	/* Does this object have children ? */
	if (Object->Child)
	{
		/* Yes -> Update children of this object */
		Execute_broadcast_method(Object->Child, UPDATE_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Permanent_text_object
 * FUNCTION  : Init method of Permanent_text object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.12.94 12:31
 * LAST      : 23.12.94 12:31
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Permanent_text_object(struct Object *Object, union Method_parms *P)
{
	struct Permanent_text_object *Permanent_text;
	struct PA *PA;

	Permanent_text = (struct Permanent_text_object *) Object;
	PA = &(Permanent_text->Print_area);

	/* Make print area */
	PA->Area.left		= Object->Rect.left + 2;
	PA->Area.top		= Object->Rect.top + 1;
	PA->Area.width		= Object->Rect.width - 4;
	PA->Area.height	= Object->Rect.height - 2;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Permanent_text_object
 * FUNCTION  : Draw method of Permanent_text object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.12.94 12:23
 * LAST      : 23.12.94 12:23
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Permanent_text_object(struct Object *Object, union Method_parms *P)
{
	struct Permanent_text_object *Permanent_text;

	Permanent_text = (struct Permanent_text_object *) Object;

	/* Is the status area hidden ? */
	if (!Hide_status_area)
	{
		/* No -> Erase permanent text area */
		OPM_CopyOPMOPM
		(
			&Slab_OPM,
			&Status_area_OPM,
			Object->Rect.left,
			Object->Rect.top,
			Object->Rect.width,
			Object->Rect.height,
			Object->X,
			Object->Y
		);

		/* Is there a message ? */
		if (Permanent_text->Counter)
		{
			/* Yes -> Is it a new message ? */
			if (Permanent_text->Counter > PERMANENT_TEXT_DURATION - BLINK_DURATION)
			{
				/* Yes -> Light up */
				Put_recoloured_box
				(
					&Status_area_OPM,
					Object->X,
					Object->Y,
					Object->Rect.width,
					Object->Rect.height,
					&(Recolour_tables[5][0])
				);
			}

			/* Print it */
			Push_PA(&(Permanent_text->Print_area));
			Set_ink(SILVER_TEXT);
			Display_processed_text
			(
				&Main_OPM,
				&(Permanent_text->Processed),
				0
			);
			Pop_PA();
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Permanent_text_object
 * FUNCTION  : Update method of Permanent_text object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.12.94 12:24
 * LAST      : 02.10.95 23:03
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Permanent_text_object(struct Object *Object, union Method_parms *P)
{
	struct Permanent_text_object *Permanent_text;
	UNSHORT i;

	Permanent_text = (struct Permanent_text_object *) Object;

	/* Is there a text ? */
	i = Permanent_text->Counter;
	if (i)
	{
		/* Yes -> Count down */
		Permanent_text->Counter--;

		/* Is it a new message ? */
		if (i > PERMANENT_TEXT_DURATION - BLINK_DURATION)
		{
			/* Yes -> Re-draw */
			Execute_method(Object->Self, DRAW_METHOD, NULL);
		}
		else
		{
			/* No -> Is it time to erase ? */
			if (i == 1)
			{
				/* Yes -> Destroy processed text */
				Destroy_processed_text(&(Permanent_text->Processed));

				/* Re-draw */
				Execute_method(Object->Self, DRAW_METHOD, NULL);
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Permanent_text_object
 * FUNCTION  : Set method of Permanent_text object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.12.94 12:24
 * LAST      : 24.10.95 18:35
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Permanent_text_object(struct Object *Object, union Method_parms *P)
{
	struct Permanent_text_object *Permanent_text;
	SISHORT Delay;
	SISHORT i;
	UNCHAR *Text_ptr;

	Permanent_text = (struct Permanent_text_object *) Object;
	Text_ptr = (UNCHAR *) P;

	/* In combat / during a combat round ? */
	if (In_Combat && Fighting)
	{
		/* Yes -> Are there objects in the current object tree ? */
		if (Get_root_object_handle())
		{
			/* Yes -> Calculate required delay */
			Delay = Permanent_text->Counter -
			 (PERMANENT_TEXT_DURATION - Combat_message_speed);

			/* Wait */
			for (i=0;i<Delay;i++)
			{
				Update_screen();
			}
		}
	}

	/* Is a text still on the screen ? */
	if (Permanent_text->Counter)
	{
		/* Yes -> Clear counter */
		Permanent_text->Counter = 0;

		/* Destroy processed text */
		Destroy_processed_text(&(Permanent_text->Processed));
	}

	/* Is there a new text ? */
	if (Text_ptr)
	{
		/* Yes -> Process text */
		Push_PA(&(Permanent_text->Print_area));
		Process_text(Text_ptr, &(Permanent_text->Processed));
		Pop_PA();

		/* Set counter */
		Permanent_text->Counter = PERMANENT_TEXT_DURATION;
	}

	/* Redraw object */
	Execute_method(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_permanent_message_nr
 * FUNCTION  : Set a new permanent message number.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 21:05
 * LAST      : 30.09.95 15:08
 * INPUTS    : UNSHORT Message_nr - Message number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_permanent_message_nr(UNSHORT Message_nr)
{
	/* Does the permanent text object exist ? */
	if (Is_object_present(Permanent_text_object))
	{
		/* Yes -> Clear */
		Execute_method
		(
			Permanent_text_object,
			SET_METHOD,
			(void *) System_text_ptrs[Message_nr]
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_permanent_text
 * FUNCTION  : Set a new permanent text.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 10:55
 * LAST      : 30.09.95 15:08
 * INPUTS    : UNCHAR *Text - Pointer to text.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_permanent_text(UNCHAR *Text)
{
	/* Does the permanent text object exist ? */
	if (Is_object_present(Permanent_text_object))
	{
		/* Yes -> Clear */
		Execute_method
		(
			Permanent_text_object,
			SET_METHOD,
			(void *) Text
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_permanent_text
 * FUNCTION  : Clear the permanent text.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 21:06
 * LAST      : 30.09.95 15:07
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_permanent_text(void)
{
	/* Does the permanent text object exist ? */
	if (Is_object_present(Permanent_text_object))
	{
		/* Yes -> Clear */
		Execute_method
		(
			Permanent_text_object,
			SET_METHOD,
			NULL
		);
	}
}

