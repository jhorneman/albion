/************
 * NAME     : STATAREA.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : STATAREA.H
 ************/

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

/* global variables */

UNSHORT Party_member_objects[6];

static struct Method Party_member_methods[] = {
	{ INIT_METHOD, Init_Party_member_object },
	{ DRAW_METHOD, Draw_Party_member_object },
	{ UPDATE_METHOD, Update_Party_member_object },
	{ LEFT_METHOD, Left_Party_member_object },
	{ DLEFT_METHOD, DLeft_Party_member_object },
	{ RIGHT_METHOD, Normal_rightclicked },
	{ FEEDBACK_METHOD, Feedback_Party_member_object },
	{ HIGHLIGHT_METHOD, Highlight_Party_member_object },
	{ POP_UP_METHOD, Pop_up_Party_member_object },
	{ HELP_METHOD, Help_Party_member_object },
	{ TOUCHED_METHOD, Touch_Party_member_object },
	{ 0, NULL}
};

static struct Object_class Party_member_Class = {
	0, sizeof(struct Party_member_object),
	&Party_member_methods[0]
};

static struct Method Status_area_methods[] = {
	{ INIT_METHOD, Init_Status_area_object },
	{ DRAW_METHOD, Draw_Status_area_object },
	{ UPDATE_METHOD, Update_Status_area_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

static struct Object_class Status_area_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Status_area_object),
	&Status_area_methods[0]
};

static struct Method Help_line_methods[] = {
	{ DRAW_METHOD, Draw_Help_line_object },
	{ UPDATE_METHOD, Update_Help_line_object },
	{ SET_METHOD, Set_Help_line_object },
	{ 0, NULL}
};

static struct Object_class Help_line_Class = {
	0, sizeof(struct Help_line_object),
	&Help_line_methods[0]
};

static struct Method Permanent_text_methods[] = {
	{ INIT_METHOD, Init_Permanent_text_object },
	{ DRAW_METHOD, Draw_Permanent_text_object },
	{ UPDATE_METHOD, Update_Permanent_text_object },
	{ SET_METHOD, Set_Permanent_text_object },
	{ 0, NULL}
};

static struct Object_class Permanent_text_Class = {
	0, sizeof(struct Permanent_text_object),
	&Permanent_text_methods[0]
};

struct OPM Status_area_OPM;

static UNSHORT Status_area_object;
UNSHORT Help_line_object;
UNSHORT Permanent_text_object;

static struct PUME Party_member_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 70, PUM_Go_inventory},
	{PUME_AUTO_CLOSE, 0, 10, PUM_Use_magic},
	{PUME_AUTO_CLOSE, 0, 4, PUM_Make_active_member},
	{PUME_AUTO_CLOSE, 0, 5, PUM_Talk_to_member}
};
static struct PUM Party_member_PUM = {
	4,
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
 * LAST      : 24.10.94 13:20
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
	OPM_CreateVirtualOPM(&Main_OPM, &Status_area_OPM, 0, Panel_Y,
	 Screen_width, Screen_height - Panel_Y);

	Add_update_OPM(&Status_area_OPM);

	/* Add status area object */
	Status_area_object = Add_object(Earth_object, &Status_area_Class, NULL,
	 0, Panel_Y, Screen_width, Screen_height - Panel_Y);

	/* Add help-line object */
	Help_line_object = Add_object(Status_area_object, &Help_line_Class, NULL,
	 Forbidden_X, Forbidden_Y - Panel_Y, 176, 10);

	/* Add permanent text object */
	Permanent_text_object = Add_object(Status_area_object, &Permanent_text_Class,
	 NULL, 181, 208 - Panel_Y, 177, 30);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_status_area
 * FUNCTION  : Reset the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 15:19
 * LAST      : 30.06.95 17:20
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
Reset_status_area(void)
{
	struct Status_area_object *Status_area;

	/* Get pointer to status area object */
	Status_area = (struct Status_area_object *)
	 Get_object_data(Status_area_object);

	/* Force an update */
	Status_area->Active_member = 0xFFFF;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Status_area_object
 * FUNCTION  : Initialize method of Status_area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.01.95 14:12
 * LAST      : 03.01.95 14:12
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
	struct Party_member_OID OID;
	UNSHORT i;

	/* Add first five party member objects */
	for (i=0;i<5;i++)
	{
		/* Set member index */
		OID.Number = i + 1;

		Party_member_objects[i] = Add_object(Object->Self, &Party_member_Class,
		 (UNBYTE *) &OID, 4 + i * 28, 2, 28, 46);
	}

	/* Add last party member object, which is wider */
	OID.Number = 6;

	Party_member_objects[5] = Add_object(Object->Self, &Party_member_Class,
	 (UNBYTE *) &OID, 4 + i * 28, 2, 34, 46);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Status_area_object
 * FUNCTION  : Draw method of Status_area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.01.95 14:14
 * LAST      : 03.01.95 14:14
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
	/* Draw all party members */
	Draw_party_members();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Status_area_object
 * FUNCTION  : Update method of Status_area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.01.95 14:15
 * LAST      : 03.01.95 14:15
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Status_area_object(struct Object *Object, union Method_parms *P)
{
	struct Status_area_object *Status_area;

	Status_area = (struct Status_area_object *) Object;

	/* New active member ? */
	if (PARTY_DATA.Active_member != Status_area->Active_member)
	{
		/* Yes -> Redraw all party members */
		Draw_party_members();

		/* Store active member index */
		Status_area->Active_member = PARTY_DATA.Active_member;
	}

	/* Update children */
	Execute_child_methods(Object->Self, UPDATE_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Status_area
 * FUNCTION  : Update the Status_area object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 17:23
 * LAST      : 23.06.95 19:59
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
	/* Update status area */
	Execute_method(Status_area_object, UPDATE_METHOD, NULL);

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
 * NAME      : Init_Party_member_object
 * FUNCTION  : Initialize method of Party_member object.
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
Init_Party_member_object(struct Object *Object, union Method_parms *P)
{
	struct Party_member_object *Party_member;
	struct Party_member_OID *OID;

	Party_member = (struct Party_member_object *) Object;
	OID = (struct Party_member_OID *) P;

	/* Copy data from OID */
	Party_member->Number = OID->Number;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Party_member_object
 * FUNCTION  : Draw method of Party_member object.
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

	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		/* Make clipping rectangle */
		Clip.left	= Object->X;
		Clip.top		= Object->Y;
		Clip.width	= 34;
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
 * FUNCTION  : Update method of Party_member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 10:54
 * LAST      : 03.05.95 10:54
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
	struct Character_data *Char;
	UNSHORT LP_bar_size;
	UNSHORT SP_bar_size;

	Party_member = (struct Party_member_object *) Object;

	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		/*  Get character data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Party_char_handles[Party_member->Number - 1]);

		/* Any LP ? */
		if (Char->xLife_points)
		{
			/* Yes -> Calculate life-points bar size */
			LP_bar_size = max((Char->xLife_points * 20) /
			 (Char->xLife_points_maximum + Char->xLife_points_magic), 1);
		}
		else
		{
			/* No -> No bar */
			LP_bar_size = 0;
		}

		/* Any SP ? */
		if (Char->xSpell_points)
		{
			/* Yes -> Calculate spell-points bar size */
			SP_bar_size = max((Char->xSpell_points * 20) /
			 (Char->xSpell_points_maximum + Char->xSpell_points_magic), 1);
		}
		else
		{
			/* No -> No bar */
			SP_bar_size = 0;
		}

		MEM_Free_pointer(Party_char_handles[Party_member->Number - 1]);

		/* Should the bars be re-drawn ? */
		if ((Party_member->LP_bar_size != LP_bar_size) ||
		 (Party_member->SP_bar_size != SP_bar_size))
		{
			/* Yes -> Adapt bar sizes */
			Party_member->LP_bar_size += sgn((SISHORT) (LP_bar_size -
			 Party_member->LP_bar_size));
			Party_member->SP_bar_size += sgn((SISHORT) (SP_bar_size -
			 Party_member->SP_bar_size));

			/* Re-draw party member */
			Draw_Party_member_object(Object, P);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Party_member_object
 * FUNCTION  : Highlight method of Party_member object.
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

	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		/* Make clipping rectangle */
		Clip.left = Object->X;
		Clip.top = Object->Y;
		Clip.width = 34;
		Clip.height = Object->Rect.height;

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
 * FUNCTION  : Feedback method of Party_member object.
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

	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		/* Make clipping rectangle */
		Clip.left = Object->X;
		Clip.top = Object->Y;
		Clip.width = 34;
		Clip.height = Object->Rect.height;

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
 * FUNCTION  : Left method of Party_member object.
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

	/* Anyone here ? */
	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
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
 * FUNCTION  : DLeft method of Party_member object.
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

	/* Anyone here ? */
	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		/* Yes -> Dehighlight */
		Dehighlight(Object, P);

		/* Enter inventory */
		Go_Inventory(Party_member->Number, 1);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Party_member_object
 * FUNCTION  : Pop-up method of Party_member object.
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

	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		Get_char_name(Party_char_handles[Party_member->Number - 1], Char_name);

		Party_member_PUM.Title = Char_name;
		Party_member_PUM.Data = (UNLONG) Party_member->Number;

		PUM_source_object_handle = Object->Self;
		PUM_char_handle = Party_char_handles[Party_member->Number - 1];
		Do_PUM(Mouse_X + 5, Mouse_Y - 40, &Party_member_PUM);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Party_member_object
 * FUNCTION  : Help method of Party_member object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 14:08
 * LAST      : 27.10.94 14:08
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
	MEM_HANDLE Handle;
	UNCHAR String[100];
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	Party_member = (struct Party_member_object *) Object;

	if (PARTY_DATA.Member_nrs[Party_member->Number - 1])
	{
		/* Get character name */
		Handle = Party_char_handles[Party_member->Number - 1];
		Get_char_name(Handle, Name);

		/* Make help line string */
		sprintf(String, System_text_ptrs[0], Name, Get_LP(Handle),
		 Get_SP(Handle));

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
 * FUNCTION  : Touch method of Party_member object.
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
 * NAME      : Draw_party_members
 * FUNCTION  : Draw all party members in the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 12:51
 * LAST      : 24.10.94 12:51
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
	UNBYTE *Ptr;

	/* Clear party display */
	OPM_CopyOPMOPM(&Slab_OPM, &Status_area_OPM, 0, Panel_Y, 180, 48, 0, 0);

	/* Display non-active party members' portraits */
	for (i=0;i<6;i++)
	{
		/* Is there anyone here and is he or she not active ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Display portrait */
			Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

			Put_masked_block(&Status_area_OPM, 4 + i * 28, 5, 34, 37, Ptr);

			MEM_Free_pointer(Small_portrait_handles[i]);
		}
	}

	/* Display the active party member's portrait */
	i = PARTY_DATA.Active_member - 1;

	Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

	Put_masked_block(&Status_area_OPM, 4 + i * 28, 2, 34, 37, Ptr);

	MEM_Free_pointer(Small_portrait_handles[i]);

	/* Display LP & SP bars */
	Display_party_status_bars();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_party_members
 * FUNCTION  : Draw all party members in the status area, highlighting one.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.10.94 17:38
 * LAST      : 24.10.94 17:38
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
	OPM_CopyOPMOPM(&Slab_OPM, &Status_area_OPM, 0, Panel_Y, 180, 48, 0, 0);

	/* Display non-active party members' portraits */
	for (i=0;i<6;i++)
	{
		/* Is there anyone here and is he or she not active ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Display portrait */
			Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

			Put_masked_block(&Status_area_OPM, 4 + i * 28, 5, 34, 37, Ptr);

			if (i == Index-1)
			{
				Put_recoloured_block(&Status_area_OPM, 4 + i * 28, 5, 34, 37, Ptr,
				 &(Recolour_tables[5][0]));
			}

			MEM_Free_pointer(Small_portrait_handles[i]);
		}
	}

	/* Display the active party member's portrait */
	i = PARTY_DATA.Active_member - 1;

	Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

	Put_masked_block(&Status_area_OPM, 4 + i * 28, 2, 34, 37, Ptr);
	if (i == Index-1)
	{
		Put_recoloured_block(&Status_area_OPM, 4 + i * 28, 2, 34, 37, Ptr,
		 &(Recolour_tables[5][0]));
	}

	MEM_Free_pointer(Small_portrait_handles[i]);

	/* Display LP & SP bars */
	Display_party_status_bars();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_party_members
 * FUNCTION  : Draw all party members in the status area, "feedbacking" one.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 14:14
 * LAST      : 25.10.94 14:14
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
	OPM_CopyOPMOPM(&Slab_OPM, &Status_area_OPM, 0, Panel_Y, 180, 48, 0, 0);

	/* Display non-active party members' portraits */
	for (i=0;i<6;i++)
	{
		/* Is there anyone here and is he or she not active ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Display portrait */
			Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

			Put_masked_block(&Status_area_OPM, 4 + i * 28, 5, 34, 37, Ptr);

			if (i == Index-1)
			{
				Put_recoloured_block(&Status_area_OPM, 4 + i * 28, 5, 34, 37, Ptr, &(Recolour_tables[3][0]));
			}

			MEM_Free_pointer(Small_portrait_handles[i]);
		}
	}

	/* Display the active party member's portrait */
	i = PARTY_DATA.Active_member - 1;

	Ptr = MEM_Claim_pointer(Small_portrait_handles[i]);

	Put_masked_block(&Status_area_OPM, 4 + i * 28, 2, 34, 37, Ptr);
	if (i == Index-1)
	{
		Put_recoloured_block(&Status_area_OPM, 4 + i * 28, 2, 34, 37, Ptr,
		 &(Recolour_tables[3][0]));
	}

	MEM_Free_pointer(Small_portrait_handles[i]);

	/* Display LP & SP bars */
	Display_party_status_bars();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_party_status_bars
 * FUNCTION  : Draw all party members' status bars in the status area.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 14:12
 * LAST      : 25.10.94 14:12
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
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Is this the active member ? */
			if (i == PARTY_DATA.Active_member - 1)
			{
				/* Yes -> Indicate */
				OPM_FillBox(&Status_area_OPM, 4 + i * 28 + 5, 38, 22, 7, BRIGHT);
			}
			else
			{
				/* No */
				OPM_FillBox(&Status_area_OPM, 4 + i * 28 + 5, 38, 22, 7, BLACK);
			}

			/* Get bar sizes */
			Party_member = (struct Party_member_object *)
			 Get_object_data(Party_member_objects[i]);

			LP_bar_size = Party_member->LP_bar_size;
			SP_bar_size = Party_member->SP_bar_size;

			/* Display bars */
			OPM_FillBox(&Status_area_OPM, 4 + i * 28 + 6, 39, LP_bar_size, 2,
			 GREEN + 1);
			OPM_FillBox(&Status_area_OPM, 4 + i * 28 + 6 + LP_bar_size, 39,
			 20 - LP_bar_size, 2, GREEN + 3);
			OPM_FillBox(&Status_area_OPM, 4 + i * 28 + 6, 42, SP_bar_size, 2,
			 TURQUOISE + 1);
			OPM_FillBox(&Status_area_OPM, 4 + i * 28 + 6 + SP_bar_size, 42,
			 20 - SP_bar_size, 2, TURQUOISE + 3);
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
 * LAST      : 10.08.95 15:11
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
	if ((In_Inventory) && (((UNSHORT) PUM->Data == Inventory_member)))
	{
		/* Yes -> Cannot access inventory */
		PUMES[0].Flags |= PUME_ABSENT;
	}

	/* Does this character have magical abilities / not in combat ? */
	if (Character_has_magical_abilities(Char_handle) &&
	 !(Get_conditions(Char_handle) & MAGIC_MASK) && (!In_Combat))
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

	/* Is this the active party member / can this character not be active ? */
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
	Go_Inventory((UNSHORT) Data, 1);
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
	Do_Dialogue(NORMAL_DIALOGUE, PARTY_CHAR_TYPE,
	 PARTY_DATA.Member_nrs[(UNSHORT) Data - 1], 0xFFFF);
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
//	UNSHORT i;

	Help_line = (struct Help_line_object *) Object;

	/* Erase help line */
	OPM_CopyOPMOPM(&Slab_OPM, &Status_area_OPM, Object->Rect.left,
	 Object->Rect.top, Object->Rect.width, Object->Rect.height,
	 Object->X, Object->Y);

	/* Is there a message ? */
	if (Help_line->Counter)
	{
		/* Yes -> Is it a new message ? */
/*		i = Help_line->Counter - (HELP_LINE_INTERVAL - 11);
		if (i > 0)
		{ */
			/* Yes -> Light up */
/*			Put_recoloured_box(&Status_area_OPM, Object->X, Object->Y,
			 Object->Rect.width, Object->Rect.height,
			 &(Recolour_tables[(i + 1) / 4 + 4][0]));
		} */

		/* Make clipping rectangle */
		Clip.left = Object->X;
		Clip.top = Object->Y;
		Clip.width = Object->Rect.width;
		Clip.height = Object->Rect.height;

		/* Install clip area */
		memcpy(&Old, &(Status_area_OPM.clip), sizeof(struct BBRECT));
		memcpy(&(Status_area_OPM.clip), &Clip, sizeof(struct BBRECT));

		/* Print it */
		Push_textstyle(&Default_text_style);
		Set_ink(GOLD_TEXT);
		Print_centered_string(&Status_area_OPM, Object->X, Object->Y + 1,
		 Object->Rect.width, &(Help_line->Text[0]));
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
		if ((i > HELP_LINE_INTERVAL - 11) || (i == 1))
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

			/* Set counter */
			Help_line->Counter = HELP_LINE_INTERVAL;
		}
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
	PA->Area.left = Object->Rect.left + 2;
	PA->Area.top = Object->Rect.top + 1;
	PA->Area.width = Object->Rect.width - 4;
	PA->Area.height = Object->Rect.height - 2;

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

	/* Erase permanent text area */
	OPM_CopyOPMOPM(&Slab_OPM, &Status_area_OPM, Object->Rect.left,
	 Object->Rect.top, Object->Rect.width, Object->Rect.height, Object->X,
	 Object->Y);

	/* Is there a message ? */
	if (Permanent_text->Counter)
	{
		/* Yes -> Print it */
		Push_PA(&(Permanent_text->Print_area));
		Display_processed_text(&Main_OPM, &(Permanent_text->Processed), 0);
		Pop_PA();
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
 * LAST      : 23.12.94 12:24
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

		/* Is it a new message or is it time to erase ? */
		if ((i > PERMANENT_TEXT_INTERVAL - 1) || (i == 1))
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
 * NAME      : Set_Permanent_text_object
 * FUNCTION  : Set method of Permanent_text object.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.12.94 12:24
 * LAST      : 23.12.94 12:24
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Permanent_text_object(struct Object *Object, union Method_parms *P)
{
	struct Permanent_text_object *Permanent_text;

	Permanent_text = (struct Permanent_text_object *) Object;

	#if FALSE
	/* Is there a relatively new text on the screen ? */
	while (Permanent_text->Counter > PERMANENT_TEXT_MINIMUM)
	{
		/* Yes -> Wait */
		Update_screen();
	}
	#endif

	/* Is there a new text ? */
	if ((UNCHAR *) P)
	{
		/* Yes -> Process text */
		Push_PA(&(Permanent_text->Print_area));
		Process_text((UNCHAR *) P, &(Permanent_text->Processed));
		Pop_PA();

		/* Set counter */
		Permanent_text->Counter = PERMANENT_TEXT_INTERVAL;
	}
	else
	{
		/* No -> Clear counter */
		Permanent_text->Counter = 0;

		/* Destroy processed text */
		Destroy_processed_text(&(Permanent_text->Processed));

		/* Redraw object */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

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
 * LAST      : 12.04.95 21:05
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
	Execute_method(Permanent_text_object, SET_METHOD,
	 (void *) System_text_ptrs[Message_nr]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_permanent_text
 * FUNCTION  : Set a new permanent text.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 10:55
 * LAST      : 05.05.95 10:55
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
	Execute_method(Permanent_text_object, SET_METHOD, (void *) Text);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_permanent_text
 * FUNCTION  : Clear the permanent text.
 * FILE      : STATAREA.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 21:06
 * LAST      : 12.04.95 21:06
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
	Execute_method(Permanent_text_object, SET_METHOD, NULL);
}

