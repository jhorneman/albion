/************
 * NAME     : USERFACE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 13-9-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>

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
#include <STATAREA.H>
#include <2D_DISPL.H>

/* global variables */

/* Earth method list */
static struct Method Earth_methods[] = {
	{ DRAW_METHOD, Draw_children },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_slab },
	{ 0, NULL}
};

/* Earth class description */
struct Object_class Earth_Class = {
	0, sizeof(struct Object),
	&Earth_methods[0]
};

#if FALSE
/* Empty method list */
static struct Method No_methods[] = {
	{ 0, NULL }
};
#endif

/* Group class description */
struct Object_class Group_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Object),
	&Earth_methods[0]
};

/* Window module */
struct Module Window_Mod = {
	0, WINDOW_MOD, NO_SCREEN,
	NULL,
	Window_ModInit, Window_ModExit,
	NULL, NULL,
	NULL
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_background
 * FUNCTION  : Restore the background in an area.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 13:25
 * LAST      : 12.01.95 13:25
 * INPUTS    : SISHORT X - Left X-coordinate of area.
 *             SISHORT Y - Top Y-coordinate of area.
 *             UNSHORT Width - Width of area in pixels.
 *             UNSHORT Height - Height of area in pixels.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Restore_background(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	struct BBRECT Rect;
	union Method_parms P;
	UNSHORT Obj;

	/* Initialize rectangle */
	Rect.left = X;
	Rect.top = Y;
	Rect.width = Width;
	Rect.height = Height;

	/* Restore background */
	P.Rect = &Rect;
	Obj = Get_root_object_handle();
	Execute_method(Obj, RESTORE_METHOD, &P);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_children
 * FUNCTION  : Draw method of Earth object.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 13:53
 * LAST      : 23.06.95 13:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_children(struct Object *Object, union Method_parms *P)
{
	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_slab
 * FUNCTION  : Normal restore method.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 13:34
 * LAST      : 12.01.95 13:34
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Restore_slab(struct Object *Object, union Method_parms *P)
{
	struct BBRECT *Rect;

	Rect = P->Rect;

	OPM_CopyOPMOPM(&Slab_OPM, Current_OPM, Rect->left - Object->Rect.left,
	 Rect->top - Object->Rect.top, Rect->width, Rect->height,
	 Rect->left, Rect->top);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_window
 * FUNCTION  : Restore method of window-like object.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.01.95 16:53
 * LAST      : 12.01.95 16:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Restore_window(struct Object *Object, union Method_parms *P)
{
	struct BBRECT *Rect;

	Rect = P->Rect;

	Draw_window_inside(Current_OPM, Rect->left - Object->Rect.left,
	 Rect->top - Object->Rect.top, Rect->width, Rect->height);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Window_ModInit
 * FUNCTION  : Initialize window module.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 15:55
 * LAST      : 26.06.95 12:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Window_ModInit(void)
{
	/* Current screen is 2D map ? */
	if (Current_screen_type(1) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Current_2D_OPM = NULL;
	}
	else
	{
		/* No -> Prepare screen */
		Switch_screens();
	}

	/* Install stuff */
	Push_textstyle(&Default_text_style);
	Push_root(&Main_OPM);
	Push_mouse(&(Mouse_pointers[DEFAULT_MPTR]));

	/* Update OPM */
	Add_update_OPM(&Status_area_OPM);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Window_ModExit
 * FUNCTION  : Exit window module.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 15:55
 * LAST      : 08.04.95 15:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Window_ModExit(void)
{
	/* Remove update OPM */
	Remove_update_OPM(&Status_area_OPM);

	/* Remove stuff */
	Pop_mouse();
	Pop_root();
	Pop_textstyle();

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Prepare screen */
		Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
		Switch_screens();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Window_object
 * FUNCTION  : Exit method of Window object.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 13:00
 * LAST      : 22.06.95 13:00
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Exit_Window_object(struct Object *Object, union Method_parms *P)
{
	/* Destroy window */
	Destroy_window((struct Window_object *) Object);

	/* Remove help message */
	Execute_method(Help_line_object, SET_METHOD, NULL);

	/* Exit module (MUST be here !!!) */
	Pop_module();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Close_Window_object
 * FUNCTION  : Close method of Window object.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 15:08
 * LAST      : 08.04.95 15:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Close_Window_object(struct Object *Object, union Method_parms *P)
{
	/* Dehighlight */
	Dehighlight(Object, NULL);

	/* Delete self */
	Delete_object(Object->Self);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_window
 * FUNCTION  : Prepare a window.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 13:17
 * LAST      : 22.06.95 16:32
 * INPUTS    : struct Window_object *Window - Pointer to window object.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will create the background and window OPMs
 *              and save the background.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Prepare_window(struct Window_object *Window)
{
	struct Object *Object;
	UNBYTE *Ptr;

	Object = &(Window->Object);

	/* Make background buffer and OPM */
	Window->Background_handle = MEM_Allocate_memory(Object->Rect.width
	 * Object->Rect.height);

	Ptr = MEM_Claim_pointer(Window->Background_handle);
	OPM_New(Object->Rect.width, Object->Rect.height, 1,
	 &(Window->Background_OPM), Ptr);
	MEM_Free_pointer(Window->Background_handle);

	/* Save background */
	OPM_CopyOPMOPM(&Main_OPM, &(Window->Background_OPM), Object->Rect.left,
	 Object->Rect.top, Object->Rect.width, Object->Rect.height, 0, 0);

	/* Build window OPM */
	OPM_CreateVirtualOPM(&Main_OPM, &(Window->Window_OPM), Object->Rect.left,
	 Object->Rect.top, Object->Rect.width, Object->Rect.height);

	/* Set root OPM */
	Set_root_OPM(&(Window->Window_OPM));

	/* Install mouse area */
	Push_MA(&(Object->Rect));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_window
 * FUNCTION  : Destroy a window.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.09.94 13:17
 * LAST      : 19.10.94 14:24
 * INPUTS    : struct Window_object *Window - Pointer to window object.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will restore the background and destroy the
 *              background and window OPMs.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_window(struct Window_object *Window)
{
	struct Object *Object;

	Object = &(Window->Object);

	/* Has the window been initialized ? */
	if (Window->Background_handle)
	{
		/* Yes -> Remove mouse area */
		Pop_MA();

		/* Destroy window OPM */
		OPM_Del(&(Window->Window_OPM));

		/* Restore background */
		OPM_CopyOPMOPM(&(Window->Background_OPM), &Main_OPM, 0, 0, Object->Rect.width,
		 Object->Rect.height, Object->Rect.left, Object->Rect.top);

		/* Destroy background buffer and OPM */
		OPM_Del(&(Window->Background_OPM));
		MEM_Free_memory(Window->Background_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_window_inside
 * FUNCTION  : Draw the inside of a window.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.10.94 12:32
 * LAST      : 18.10.94 12:32
 * INPUTS    : struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - Left X-coordinate of window.
 *             SISHORT Y - Top Y-coordinate of window.
 *             UNSHORT Width - Width of window in pixels.
 *             UNSHORT Height - Height of window in pixels.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The inside of the window will be filled with a 32 by 32
 *              pixel pattern.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_window_inside(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height)
{
	struct BBRECT Clip, Old;
	UNSHORT Seed;
	UNSHORT W, H, W2, H2, X2;
	UNSHORT i, j;

	/* Make clip area */
	Clip.left = X;
	Clip.top = Y;
	Clip.width = Width;
	Clip.height = Height;

	/* Install clip area */
	memcpy(&Old, &(OPM->clip), sizeof(struct BBRECT));
	memcpy(&(OPM->clip), &Clip, sizeof(struct BBRECT));

	/* Determine number of tiles */
	W = (Width + 31) / 32 + 1;
	W2 = 32 - (Width % 32);
	if (!W2)
	{
		W++;
		W2 = 32;
	}

	H = (Height + 31) / 32 + 1;
	H2 = 32 - (Height % 32);
	if (!H2)
	{
		H++;
		H2 = 32;
	}

	/* Add random vector */
	X -= (X % W2);
	Y -= (Y % H2);

	/* Get seed */
	Seed = (X % W2) + (Y % W2);

	/* Display tiles */
	for (i=0;i<H;i++)
	{
		X2 = X;
		for (j=0;j<W;j++)
		{
			Put_unmasked_block(OPM, X2, Y, 32, 32, &(Pop_up_bg[Seed % 2][0]));
			Seed = (Seed + 87) * 17;
			X2 += 32;
		}
		Y += 32;
	}

	/* Restore clip area */
	memcpy(&(OPM->clip), &Old, sizeof(struct BBRECT));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_window_border
 * FUNCTION  : Draw a window border.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.10.94 12:29
 * LAST      : 18.10.94 12:29
 * INPUTS    : struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - Left X-coordinate of window border.
 *             SISHORT Y - Top Y-coordinate of window border.
 *             UNSHORT Width - Width of window border in pixels.
 *             UNSHORT Height - Height of window border in pixels.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The actual border is 3 pixels wide and starts at a distance
 *              of 4 pixels from the input box.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_window_border(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height)
{
	UNSHORT i, j, k;
	UNSHORT Seed;

	/* Get seed */
	Seed = X + Y;

	/* Draw top-left border */
	Put_masked_block(OPM, X, Y, 16, 16, &Window_TL[0]);

	/* Draw top edge */
	j = (Width - 32) / 16;
	for (i=1;i<=j;i++)
	{
		k = Seed & 0x0007;
		Seed = (Seed + 87) * 17;
		if (k > 3)
			k = 0;
		Put_unmasked_block(OPM, X + (16 * i), Y + 4, 16, 3, &Window_horizontal[k][0]);
	}

	/* Draw last element of top edge */
	k = Seed & 0x0007;
	Seed = (Seed + 87) * 17;
	if (k > 3)
		k = 0;
	Put_unmasked_block(OPM, X + Width - 32, Y + 4, 16, 3, &Window_horizontal[k][0]);

	/* Draw top-right border */
	Put_masked_block(OPM, X + Width - 16, Y, 16, 16, &Window_TR[0]);

 	/* Draw left edge */
	j = (Height - 32) / 16;
	for (i=1;i<=j;i++)
	{
		k = Seed & 0x0007;
		Seed = (Seed + 87) * 17;
		if (k > 3)
			k = 0;
		Put_unmasked_block(OPM, X + 4, Y + (16 * i), 3, 16, &Window_vertical[k][0]);
	}

	/* Draw last element of left edge */
	k = Seed & 0x0007;
	Seed = (Seed + 87) * 17;
	if (k > 3)
		k = 0;
	Put_unmasked_block(OPM, X + 4, Y + Height - 32, 3, 16, &Window_vertical[k][0]);

	/* Draw bottom-left border */
	Put_masked_block(OPM, X, Y + Height - 16, 16, 16, &Window_BL[0]);

	/* Draw bottom edge */
	j = (Width - 32) / 16;
	for (i=1;i<=j;i++)
	{
		k = Seed & 0x0007;
		Seed = (Seed + 87) * 17;
		if (k > 3)
			k = 0;
		Put_unmasked_block(OPM, X + (16 * i), Y + Height - 7, 16, 3, &Window_horizontal[k][0]);
	}

	/* Draw last element of bottom edge */
	k = Seed & 0x0007;
	Seed = (Seed + 87) * 17;
	if (k > 3)
		k = 0;
	Put_unmasked_block(OPM, X + Width - 32, Y + Height - 7, 16, 3, &Window_horizontal[k][0]);

	/* Draw bottom-right border */
	Put_masked_block(OPM, X + Width - 16, Y + Height - 16, 16, 16, &Window_BR[0]);

 	/* Draw right edge */
	j = (Height - 32) / 16;
	for (i=1;i<=j;i++)
	{
		k = Seed & 0x0007;
		Seed = (Seed + 87) * 17;
		if (k > 3)
			k = 0;
		Put_unmasked_block(OPM, X + Width - 7, Y + (16 * i), 3, 16, &Window_vertical[k][0]);
	}

	/* Draw last element of right edge */
	k = Seed & 0x0007;
	Seed = (Seed + 87) * 17;
	if (k > 3)
		k = 0;
	Put_unmasked_block(OPM, X + Width - 7, Y + Height - 32, 3, 16, &Window_vertical[k][0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_high_border
 * FUNCTION  : Draw a high border.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 16:00
 * LAST      : 05.01.95 16:00
 * INPUTS    : struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             UNSHORT Width - Width of box.
 *             UNSHORT Height - Height of box.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_high_border(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	Put_recoloured_box(OPM, X, Y, Width - 1, 1, &(Recolour_tables[7][0]));
	Put_recoloured_box(OPM, X, Y + 1, 1, Height - 2, &(Recolour_tables[7][0]));
	Put_recoloured_box(OPM, X + 1, Y + Height - 1, Width - 1, 1, &(Recolour_tables[1][0]));
	Put_recoloured_box(OPM, X + Width - 1, Y + 1, 1, Height - 1, &(Recolour_tables[1][0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_deep_box
 * FUNCTION  : Draw a deep box.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 16:02
 * LAST      : 05.01.95 16:02
 * INPUTS    : struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             UNSHORT Width - Width of box.
 *             UNSHORT Height - Height of box.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_deep_border(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	Put_recoloured_box(OPM, X, Y, Width - 1, 1, &(Recolour_tables[0][0]));
	Put_recoloured_box(OPM, X, Y + 1, 1, Height - 2, &(Recolour_tables[0][0]));
	Put_recoloured_box(OPM, X + 1, Y + Height - 1, Width - 2, 1, &(Recolour_tables[6][0]));
	Put_recoloured_box(OPM, X + Width - 1, Y + 1, 1, Height - 1, &(Recolour_tables[6][0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_deep_box
 * FUNCTION  : Draw a deep box.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 12:44
 * LAST      : 20.10.94 12:44
 * INPUTS    : struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             UNSHORT Width - Width of box.
 *             UNSHORT Height - Height of box.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_deep_box(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	Draw_deep_border(OPM, X, Y, Width, Height);
	Put_recoloured_box(OPM, X + 1, Y + 1, Width - 2, Height - 2, &(Recolour_tables[2][0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_light_box
 * FUNCTION  : Draw a light box.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 12:53
 * LAST      : 20.10.94 12:53
 * INPUTS    : struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             UNSHORT Width - Width of box.
 *             UNSHORT Height - Height of box.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_light_box(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	Put_recoloured_box(OPM, X, Y, Width - 1, 1, &(Recolour_tables[8][0]));
	Put_recoloured_box(OPM, X, Y + 1, 1, Height - 2, &(Recolour_tables[8][0]));
	Put_recoloured_box(OPM, X + 1, Y + 1, Width - 2, Height - 2, &(Recolour_tables[6][0]));
	Put_recoloured_box(OPM, X + 1, Y + Height - 1, Width - 1, 1, &(Recolour_tables[3][0]));
	Put_recoloured_box(OPM, X + Width - 1, Y + 1, 1, Height - 2, &(Recolour_tables[3][0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Normal_touched
 * FUNCTION  : Normal touch interaction.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 12:57
 * LAST      : 10.07.95 13:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Normal_touched(struct Object *Object, union Method_parms *P)
{
	union Method_parms Parameters;
	UNLONG Result;

	/* Still the same object ? */
	if (Object->Self != Highlighted_object)
	{
		/* No -> Redraw previously highlighted object (if any) */
		if (Highlighted_object)
		{
			Execute_method(Highlighted_object, DRAW_METHOD, NULL);
		}

		/* Store new highlighted object handle */
		Highlighted_object = Object->Self;

		/* Drag & drop mode ? */
		if (Drag_drop_mode)
		{
			/* Yes */
			Result = TRUE;

			/* Does this object have a drop method ? */
			if (Has_method(Object->Self, DROP_METHOD))
			{
				/* Yes -> Pass drag & drop data as parameters */
				Parameters.Drag_drop_data_ptr = &Current_drag_drop_data;

				/* Does this object have the INQUIRE_DROP method ? */
				if (Has_method(Object->Self, INQUIRE_DROP_METHOD))
				{
					/* Yes -> Inquire whether this object can handle the
					 current drag & drop data ID */
					Result = Execute_method(Object->Self, INQUIRE_DROP_METHOD,
					 &Parameters);
				}
			}

			/* Well ? */
			if (Result)
			{
				/* Yes -> Highlight new object */
				Execute_method(Object->Self, HIGHLIGHT_METHOD, NULL);

				/* Does this object have a help method ? */
				if (Has_method(Object->Self, HELP_METHOD))
				{
					/* Yes -> Execute help method */
					Execute_method(Object->Self, HELP_METHOD, NULL);
				}
				else
				{
					/* No -> Remove help message */
					Execute_method(Help_line_object, SET_METHOD, NULL);
				}
			}

			/* Pick mouse pointer */
			Change_mouse(&(Mouse_pointers[PICK_MPTR]));
		}
		else
		{
			/* No -> Highlight new object */
			Execute_method(Object->Self, HIGHLIGHT_METHOD, NULL);

			/* Does this object have a help method ? */
			if (Has_method(Object->Self, HELP_METHOD))
			{
				/* Yes -> Execute help method */
				Execute_method(Object->Self, HELP_METHOD, NULL);
			}
			else
			{
				/* No -> Remove help message */
				Execute_method(Help_line_object, SET_METHOD, NULL);
			}

			/* Does this object have a pop-up menu ? */
			if (Has_method(Object->Self, POP_UP_METHOD))
			{
				/* Yes -> Pop-up mouse pointer */
				Change_mouse(&(Mouse_pointers[POP_UP_MPTR]));
			}
			else
			{
				/* No -> Normal mouse pointer */
				Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));
			}
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dehighlight
 * FUNCTION  : Remove current highlighting.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 14:23
 * LAST      : 20.10.94 14:23
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Dehighlight(struct Object *Object, union Method_parms *P)
{
	/* Is any object highlighted ? */
	if (Highlighted_object)
	{
		/* Yes -> Redraw previously highlighted object (if any) */
		if (Highlighted_object)
		{
			Execute_method(Highlighted_object, DRAW_METHOD, NULL);
			Highlighted_object = 0;
		}

		/* Remove help message */
		Execute_method(Help_line_object, SET_METHOD, NULL);

		/* Drop mode ? */
		if (Drag_drop_mode)
		{
			/* Yes -> Pick mouse pointer */
			Change_mouse(&(Mouse_pointers[PICK_MPTR]));
		}
		else
		{
			/* No -> Normal mouse pointer */
			Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Normal_clicked
 * FUNCTION  : Normal left-click interaction.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 14:33
 * LAST      : 20.10.94 14:33
 * INPUTS    : struct Object *Object - Pointer to object.
 * RESULT    : BOOLEAN : TRUE (clicked) or FALSE (aborted).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Normal_clicked(struct Object *Object)
{
	BOOLEAN Flag = FALSE, New;

	/* While the mouse button is pressed */
	while (Button_state & 0x0001)
	{
		/* Check if the mouse is over the object */
		New = Is_over_object(Object->Self);

		/* Any change ? */
		if (Flag != New)
		{
			/* Yes */
			Flag = New;

			/* Up or down ? */
			if (Flag)
			{
				/* Down -> Mark */
				Feedback_object = Object->Self;

				/* Draw feedback */
				Execute_method(Object->Self, FEEDBACK_METHOD, NULL);
			}
			else
			{
				/* Up -> Mark */
				Feedback_object = 0;

				/* Draw normal */
				Execute_method(Object->Self, DRAW_METHOD, NULL);
			}
		}

		/* Update */
		Update_display();
		Update_input();
		Switch_screens();
	}

	/* Is down ? */
	if (Flag)
	{
		/* Yes -> Restore object */
		Execute_method(Object->Self, DRAW_METHOD, NULL);
	}

	/* Reset feedback & highlighting */
	Feedback_object = 0;
	Highlighted_object = 0;

	return Flag;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Normal_rightclicked
 * FUNCTION  : Normal right-click interaction.
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 13:37
 * LAST      : 25.10.94 13:37
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Normal_rightclicked(struct Object *Object, union Method_parms *P)
{
	BOOLEAN Flag = FALSE, New;

	/* While the mouse button is pressed */
	while (Button_state & 0x0010)
	{
		/* Check if the mouse is over the object */
		New = Is_over_object(Object->Self);

		/* Any change ? */
		if (Flag != New)
		{
			/* Yes */
			Flag = New;

			/* Up or down ? */
			if (Flag)
			{
				/* Down */
				Execute_method(Object->Self, FEEDBACK_METHOD, NULL);
			}
			else
			{
				/* Up */
				Execute_method(Object->Self, DRAW_METHOD, NULL);
			}
		}

		/* Update */
		Update_display();
		Update_input();
		Switch_screens();
	}

	/* Reset feedback & highlighting */
	Feedback_object = 0;
	Highlighted_object = 0;

	/* Is down ? */
	if (Flag)
	{
		/* Yes -> Restore object */
		Execute_method(Object->Self, DRAW_METHOD, NULL);

		/* Pop up */
		Execute_method(Object->Self, POP_UP_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wipe_out
 * FUNCTION  : Wipe a screen area out (showing the slab).
 * FILE      : USERFACE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 12:16
 * LAST      : 11.01.95 12:16
 * INPUTS    : SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             UNSHORT Width - Width of area.
 *             UNSHORT Height - Height of area.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wipe_out(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height)
{
	UNSHORT i, d;

	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, Screen_width, 8,	0, 0);
	d = 8;

	for (i=0;i< (Height - 8) / 16 - 1;i++)
	{
		OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, X, Y + d, Width, 16,
		 0, Y + d);
		d += 16;

		Switch_screens();
	}

	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, Y + d, Screen_width, Height - d,
	 0, Y + d);
}

