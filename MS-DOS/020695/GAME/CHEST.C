/************
 * NAME     : CHEST.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : CHEST.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <HDOB.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <ITEMLIST.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <CHEST.H>
#include <EVELOGIC.H>

/* global variables */

struct Module Chest_Mod = {
	0, SCREEN_MOD, CHEST_SCREEN,
	NULL,
	Chest_ModInit,
	Chest_ModExit,
	Init_Chest_display,
	Exit_Chest_display,
	NULL
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Chest_ModInit
 * FUNCTION  : Initialize Chest module.
 * FILE      : CHEST.C
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
Chest_ModInit(void)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Load inventory palette */
	Load_palette(INVENTORY_PAL_NR);

	/* Allocate memory for dragged item graphics */
	Drag_OPM_handle = MEM_Do_allocate(ITEM_SLOT_INNER_WIDTH * ITEM_SLOT_INNER_HEIGHT,
	 (UNLONG) &Drag_OPM, &OPM_ftype);
	if (!Drag_OPM_handle)
	{
		Error(ERROR_FILE_LOAD);
	}

	/* Insert handle in HDOB data */
	Drag_HDOB.Graphics_handle = Drag_OPM_handle;

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Drag_OPM_handle);
	Result = OPM_New(ITEM_SLOT_INNER_WIDTH, ITEM_SLOT_INNER_HEIGHT, 1, &Drag_OPM, Ptr);
	MEM_Free_pointer(Drag_OPM_handle);

	if (!Result)
	{
		Error(ERROR_NO_OPM);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Chest_ModExit
 * FUNCTION  : Exit Chest module.
 * FILE      : CHEST.C
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
Chest_ModExit(void)
{
	/* Free memory */
	MEM_Free_memory(Full_body_pic_handle);

	/* Delete dragged item OPM and free memory */
	OPM_Del(&Drag_OPM);
	MEM_Free_memory(Drag_OPM_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Chest_display
 * FUNCTION  : Initialize Chest display.
 * FILE      : CHEST.C
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
Init_Chest_display(void)
{
/*	InvLeft_object = Add_object(Earth_object, &InvLeft3_Class, NULL,
	 0, 0, INVENTORY_MIDDLE, Panel_Y); */
	InvRight_object = Add_object(Earth_object, &InvRight_Class, NULL,
	 INVENTORY_MIDDLE, 0, Screen_width - INVENTORY_MIDDLE, Panel_Y);

	/* Draw */
	Execute_method(InvLeft_object, DRAW_METHOD, NULL);
	Execute_method(InvRight_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Chest_display
 * FUNCTION  : Exit Chest display.
 * FILE      : CHEST.C
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
Exit_Chest_display(void)
{
	/* Delete interface objects */
	Delete_object(InvLeft_object);
	Delete_object(InvRight_object);
}

