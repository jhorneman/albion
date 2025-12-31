/************
 * NAME     : DOOR.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : DOOR.H
 ************/

/* includes */

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
#include <SCROLBAR.H>
#include <POPUP.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <DOOR.H>
#include <EVELOGIC.H>
#include <GAMETEXT.H>
#include <STATAREA.H>

/* global variables */

struct Module Door_Mod = {
	0, SCREEN_MOD, DOOR_SCREEN,
	NULL,
	Door_ModInit,
	Door_ModExit,
	Init_Door_display,
	Exit_Door_display,
	NULL
};

static struct Method Door_methods[] = {
	{ INIT_METHOD, Init_Door_object },
	{ DRAW_METHOD, Draw_Door_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

static struct Object_class Door_Class = {
	0, sizeof(struct Object),
	&Door_methods[0]
};

static struct Method Lock_display_methods[] = {
	{ INIT_METHOD, Init_Lock_display_object },
	{ DRAW_METHOD, Draw_Lock_display_object },
	{ FEEDBACK_METHOD, Feedback_Lock_display_object },
	{ HIGHLIGHT_METHOD, Highlight_Lock_display_object },
	{ HELP_METHOD, Help_Lock_display_object },
	{ POP_UP_METHOD, Pop_up_Lock_display_object },
	{ RIGHT_METHOD, Normal_rightclicked },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

struct Object_class Lock_display_Class = {
	0, sizeof(struct Object),
	&Lock_display_methods[0]
};

static struct PUME Lock_PUMEs[] = {
	{ PUME_AUTO_CLOSE, 0, 72, PUM_Pick_lock }
};
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
 * NAME      : Door_ModInit
 * FUNCTION  : Initialize Door module.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 27.01.95 10:58
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
	/* Load inventory palette */
	Load_palette(INVENTORY_PAL_NR);

	/* Initialize item dragging */
	Init_item_drag();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Door_ModExit
 * FUNCTION  : Exit Door module.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 27.01.95 10:58
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
	/* Exit item dragging */
	Exit_item_drag();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Door_display
 * FUNCTION  : Initialize Door display.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:58
 * LAST      : 27.01.95 10:58
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_Door_display(void)
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
 * NAME      : Exit_Door_display
 * FUNCTION  : Exit Door display.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 10:59
 * LAST      : 27.01.95 10:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Door_display(void)
{
	/* Delete interface objects */
	Delete_object(InvLeft_object);
	Delete_object(InvRight_object);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Door_object
 * FUNCTION  : Initialize method of Door object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 11:07
 * LAST      : 27.01.95 11:07
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
	Add_object(Object->Self, &Lock_display_Class, NULL, 50, 50, 32, 32);

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
 * LAST      : 27.01.95 11:06
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
	OPM_CopyOPMOPM(&Slab_OPM, Current_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Lock_display_object
 * FUNCTION  : Init method of Lock display object.
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
Init_Lock_display_object(struct Object *Object, union Method_parms *P)
{
	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Lock_display_object
 * FUNCTION  : Draw method of Lock display object.
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
Draw_Lock_display_object(struct Object *Object, union Method_parms *P)
{
	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_high_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw lock symbol */
	Put_masked_block(Current_OPM, Object->X + (Object->Rect.width - 12) / 2,
	 Object->Y + 1, 12, 10, &(Gold_symbol[0]));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Lock_display_object
 * FUNCTION  : Feedback method of Lock display object.
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
Feedback_Lock_display_object(struct Object *Object, union Method_parms *P)
{
	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw lock symbol */
	Put_masked_block(Current_OPM, Object->X + (Object->Rect.width - 12) / 2,
	 Object->Y + 1, 12, 10, &(Gold_symbol[0]));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Lock_display_object
 * FUNCTION  : Highlight method of Lock display object.
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
Highlight_Lock_display_object(struct Object *Object, union Method_parms *P)
{
	/* Erase object */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw surrounding box */
	Draw_high_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw lock symbol */
	Put_masked_block(Current_OPM, Object->X + (Object->Rect.width - 12) / 2,
	 Object->Y + 1, 12, 10, &(Gold_symbol[0]));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Lock_display_object
 * FUNCTION  : Help method of Lock display object.
 * FILE      : DOOR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:20
 * LAST      : 27.01.95 15:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Lock_display_object(struct Object *Object, union Method_parms *P)
{
	/* Print help line */
	Execute_method(Help_line_object, SET_METHOD, (void *) System_text_ptrs[74]);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Lock_display_object
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
Pop_up_Lock_display_object(struct Object *Object, union Method_parms *P)
{
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
 * LAST      : 27.01.95 15:24
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
}

