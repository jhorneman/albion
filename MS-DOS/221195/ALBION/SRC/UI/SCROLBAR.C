/************
 * NAME     : SCROLBAR.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : SCROLBAR.H, USERFACE.C
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
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <HDOB.H>
#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <COLOURS.H>

/*
 ** Prototypes *************************************************************
 */

/* Scroll bar methods */
UNLONG Init_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Get_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Scroll_bar_object(struct Object *Object, union Method_parms *P);

/* Scroll bar support functions */
void Scroll_bar_page_up(struct Scroll_bar_object *Scroll_bar);
void Scroll_bar_page_down(struct Scroll_bar_object *Scroll_bar);
void Light_slider(struct Scroll_bar_object *Scroll_bar);
void Update_slider(struct Scroll_bar_object *Scroll_bar, UNSHORT Y);

/* Scroll bar display functions */
void Do_draw_scroll_bar(struct Scroll_bar_object *Scroll_bar);

/*
 ** Global variables *******************************************************
 */

static struct Method Scroll_bar_methods[] = {
	{ INIT_METHOD, Init_Scroll_bar_object },
	{ DRAW_METHOD, Draw_Scroll_bar_object },
	{ LEFT_METHOD, Left_Scroll_bar_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ GET_METHOD, Get_Scroll_bar_object },
	{ SET_METHOD, Set_Scroll_bar_object },
	{ 0, NULL}
};

struct Object_class Scroll_bar_Class = {
	0, sizeof(struct Scroll_bar_object),
	&Scroll_bar_methods[0]
};

UNSHORT Slider_colours[2][9] = {
	{
		BRIGHTEST, BRIGHTER, NORMAL,
		BRIGHTER, NORMAL, DARK,
		NORMAL, DARK, DARKER
	},
	{
		WHITE, TURQUOISE, TURQUOISE+2,
		TURQUOISE, TURQUOISE+1, TURQUOISE+3,
		TURQUOISE+2, TURQUOISE+3, TURQUOISE+3
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Scroll_bar_object
 * FUNCTION  : Initialize method of Scrollbar object.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 13:45
 * LAST      : 21.10.94 13:45
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Scroll_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Scroll_bar_object *Scroll_bar;
	struct Scroll_bar_OID *OID;
	UNSHORT Total_rows;

	Scroll_bar = (struct Scroll_bar_object *) Object;
	OID = (struct Scroll_bar_OID *) P;

	/* Copy data from OID */
	Scroll_bar->Total_units = OID->Total_units;
	Scroll_bar->Units_width = OID->Units_width;
 	Scroll_bar->Units_height = OID->Units_height;
 	Scroll_bar->Update = OID->Update;

	/* Is scrolling necessary ? */
	if (Scroll_bar->Total_units <= (Scroll_bar->Units_width
	 * Scroll_bar->Units_height))
	{
		/* No -> Set scroll bar parameters */
		Scroll_bar->Row_height = 1;
		Scroll_bar->Max_Y = 0;
		Scroll_bar->Slider_height = Object->Rect.height;
	}
	else
	{
		/* Yes -> Calculate total number of rows */
		Total_rows = Scroll_bar->Total_units / Scroll_bar->Units_width;

		/* Calculate height of one row for the slider */
		Scroll_bar->Row_height = Object->Rect.height / Total_rows;

		/* Can the scroll bar handle these parameters ? */
		if (Scroll_bar->Row_height)
		{
			/* Yes -> Calculate movement room of slider */
			Scroll_bar->Max_Y = (Total_rows - Scroll_bar->Units_height)
			 * Scroll_bar->Row_height;

			/* Calculate height of slider */
			Scroll_bar->Slider_height = Object->Rect.height - Scroll_bar->Max_Y;
		}
		else
		{
			/* No -> Set safe scroll bar parameters */
			Scroll_bar->Row_height = 1;
			Scroll_bar->Max_Y = 0;
			Scroll_bar->Slider_height = Object->Rect.height;

			/* Report an error */
//			Error(ERROR_SCROLL_BAR_WRONG_PARAMETERS);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Scroll_bar_object
 * FUNCTION  : Draw method of Scrollbar object.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 13:57
 * LAST      : 21.10.94 13:57
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Scroll_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Scroll_bar_object *Scroll_bar;

	Scroll_bar = (struct Scroll_bar_object *) Object;

	/* Deselect slider */
	Scroll_bar->Flags &= ~SLIDER_SELECTED;

	/* Draw scroll bar */
	Do_draw_scroll_bar(Scroll_bar);

	/* Update slots */
	if (Scroll_bar->Update)
		(Scroll_bar->Update)(Scroll_bar);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_Scroll_bar_object
 * FUNCTION  : Set method of Scrollbar object.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 13:59
 * LAST      : 21.10.94 13:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Set_Scroll_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Scroll_bar_object *Scroll_bar;
	UNSHORT New_value;

	Scroll_bar = (struct Scroll_bar_object *) Object;

	/* Get new result */
	New_value = (UNSHORT) (P->Value);

	/* Calculate row */
	New_value /= Scroll_bar->Units_width;

	/* Calculate Y-coordinate */
	New_value *= Scroll_bar->Row_height;

	/* Too high ? */
	if (New_value > Scroll_bar->Max_Y)
	{
		/* Yes -> Clip */
		New_value = Scroll_bar->Max_Y;
	}

	/* Update */
	Update_slider(Scroll_bar, New_value);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_Scroll_bar_object
 * FUNCTION  : Get method of Scrollbar object.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 14:04
 * LAST      : 21.10.94 14:04
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Scroll bar result.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_Scroll_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Scroll_bar_object *Scroll_bar;

	Scroll_bar = (struct Scroll_bar_object *) Object;

	return (UNLONG) Scroll_bar->Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Scroll_bar_object
 * FUNCTION  : Left method of Scrollbar object.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 16:13
 * LAST      : 21.10.94 16:13
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Scroll_bar_object(struct Object *Object, union Method_parms *P)
{
	struct Scroll_bar_object *Scroll_bar;
	struct BBRECT Drag_MA;
	UNSHORT Y, mY, Offset;

	Scroll_bar = (struct Scroll_bar_object *) Object;

	/* Determine action */
	Y = Object->Rect.top;
	mY = Mouse_Y;

	/* Still in bar area ? */
	if ((mY < Y) || (mY >= Y + Object->Rect.height))
		return 0;

	/* Yes -> Above slider ? */
	if (mY < Y + Scroll_bar->Slider_Y)
	{
		/* Yes -> Page up */
		Scroll_bar_page_up(Scroll_bar);
		return 0;
	}

	/* No -> Below slider ? */
	if (mY > Y + Scroll_bar->Slider_Y + Scroll_bar->Slider_height)
	{
		/* Yes -> Page down */
		Scroll_bar_page_down(Scroll_bar);
		return 0;
	}

	/* No -> Select slider */
	Scroll_bar->Flags |= SLIDER_SELECTED;

	/* Make drag mouse area */
	Drag_MA.left = Object->Rect.left;
	Drag_MA.top = Y;
	Drag_MA.width = Object->Rect.width;
	Drag_MA.height = Scroll_bar->Max_Y + 1;

	/* Mouse off */
	Mouse_off();

	/* Hide HDOBs */
	Hide_HDOBs();

	/* Get slider Y-coordinate */
	Y = Scroll_bar->Slider_Y;
	if (Y > Scroll_bar->Max_Y)
		Y = Scroll_bar->Max_Y;
	Y += Object->Rect.top;

	/* Save Y offset */
	Offset = Mouse_Y - Y;

	/* Install mouse area */
	Push_MA(&Drag_MA);

	/* Set mouse position */
	SYSTEM_SetMousePtr(Mouse_X, Y);

	/* Drag slider */
	for (;;)
	{
		/* Anything to slide ? */
		if (Scroll_bar->Max_Y)
		{
			/* Yes -> Update needed ? */
			if ((Scroll_bar->Slider_Y / Scroll_bar->Row_height) !=
				Scroll_bar->Current_row)
			{
				/* Yes -> Update */
				Update_slider(Scroll_bar, Scroll_bar->Slider_Y);
			}
			else
			{
				/* No -> Just draw the scroll bar */
				Do_draw_scroll_bar(Scroll_bar);
			}
		}
		else
		{
			/* No -> Just draw the scroll bar */
			Do_draw_scroll_bar(Scroll_bar);
		}

		/* Update display */
		Update_screen();

		/* Exit if left button isn't pressed */
		if (!(Button_state & 0x0001))
			break;

		/* Set slider position */
		Scroll_bar->Slider_Y = Mouse_Y - Object->Rect.top;
	}

	/* Remove mouse area */
	Pop_MA();

	/* Reset mouse position */
	SYSTEM_SetMousePtr(Mouse_X, Mouse_Y + Offset);

	/* Mouse on */
	Mouse_on();

	/* Deselect slider */
	Scroll_bar->Flags &= ~SLIDER_SELECTED;

	/* Update slider */
	Update_slider(Scroll_bar, Scroll_bar->Slider_Y);

	/* Show HDOBs */
	Show_HDOBs();

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Scroll_bar_page_up
 * FUNCTION  :	Move the slider up one page.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 15:57
 * LAST      : 21.10.94 15:57
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Scroll_bar_page_up(struct Scroll_bar_object *Scroll_bar)
{
	SISHORT i;

	/* Select slider */
	Light_slider(Scroll_bar);

	/* Half page up */
	i = Scroll_bar->Units_height / 2;
	if (!i)
		i = 1;

	i = Scroll_bar->Current_row - i;
	if (i < 0)
		i = 0;
	i *= Scroll_bar -> Row_height;

	/* Wait */
	Wait_4_unclick();

	/* Update */
	Update_slider(Scroll_bar, i);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Scroll_bar_page_down
 * FUNCTION  :	Move the slider down one page.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 16:05
 * LAST      : 21.10.94 16:05
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Scroll_bar_page_down(struct Scroll_bar_object *Scroll_bar)
{
	SISHORT i;

	/* Select slider */
	Light_slider(Scroll_bar);

	/* Half page up */
	i = Scroll_bar->Units_height / 2;
	if (!i)
		i = 1;

	i = (Scroll_bar->Current_row + i) * Scroll_bar->Row_height;
	if (i > Scroll_bar->Max_Y)
		i = Scroll_bar->Max_Y;

	/* Wait */
	Wait_4_unclick();

	/* Update */
	Update_slider(Scroll_bar, i);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Light_slider
 * FUNCTION  : Light the slider.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 16:07
 * LAST      : 21.10.94 16:07
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Light_slider(struct Scroll_bar_object *Scroll_bar)
{
	/* Select slider */
	Scroll_bar->Flags |= SLIDER_SELECTED;

	/* Draw scroll bar */
	Do_draw_scroll_bar(Scroll_bar);

	/* Update display */
	Update_display();
	Switch_screens();

	/* Deselect slider */
	Scroll_bar->Flags &= ~SLIDER_SELECTED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_slider
 * FUNCTION  : Update the slider.
 * FILE      : SCROLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 16:09
 * LAST      : 21.10.94 16:09
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 *             UNSHORT Y - New slider Y-position.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_slider(struct Scroll_bar_object *Scroll_bar, UNSHORT Y)
{
	/* Too far down ? */
	if (Y > Scroll_bar->Max_Y)
		Y = Scroll_bar->Max_Y;

	/* Set variables */
	Scroll_bar->Slider_Y = Y;
	Y /= Scroll_bar->Row_height;
	Scroll_bar->Current_row = Y;
	Y *= Scroll_bar->Units_width;
	Scroll_bar->Result = Y;

	/* Re-draw scroll bar */
	Do_draw_scroll_bar(Scroll_bar);

	/* Update slots */
	if (Scroll_bar->Update)
		(Scroll_bar->Update)(Scroll_bar);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_draw_scroll_bar
 * FUNCTION  : Draw a scroll bar.
 * FILE      : SCROLLBAR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.10.94 14:16
 * LAST      : 21.10.94 14:16
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_draw_scroll_bar(struct Scroll_bar_object *Scroll_bar)
{
	UNSHORT X, Y, W, H;
	UNSHORT i;

	/* Select colours */
	i = (Scroll_bar->Flags & SLIDER_SELECTED) ? 1 : 0;

	/* Get coordinates and dimensions */
	X = Scroll_bar->Object.X;
	Y = Scroll_bar->Object.Y;
	W = Scroll_bar->Object.Rect.width;
	H = Scroll_bar->Object.Rect.height;

	/* Draw inside */
	Draw_window_inside(Current_OPM, X-1, Y-1, W+2, H+2);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, X-1, Y-1, W+2, H+2);

	/* Draw top shadow */
	Put_recoloured_box(Current_OPM, X, Y, W, 1, &(Recolour_tables[2][0]));

	/* Draw left shadow */
	Put_recoloured_box(Current_OPM, X, Y+1, 1, H-1, &(Recolour_tables[2][0]));

	/* Draw slider */
	Y += Scroll_bar->Slider_Y;
	H = Scroll_bar->Slider_height;

	OPM_SetPixel(Current_OPM, X, Y, Slider_colours[i][0]);
	OPM_HorLine(Current_OPM, X+1, Y, W-2, Slider_colours[i][1]);
	OPM_SetPixel(Current_OPM, X+W-1, Y, Slider_colours[i][2]);

	OPM_VerLine(Current_OPM, X, Y+1, H-2, Slider_colours[i][3]);
	OPM_FillBox(Current_OPM, X+1, Y+1, W-2, H-2, Slider_colours[i][4]);
	OPM_VerLine(Current_OPM, X+W-1, Y+1, H-2, Slider_colours[i][5]);

	OPM_SetPixel(Current_OPM, X, Y+H-1, Slider_colours[i][6]);
	OPM_HorLine(Current_OPM, X+1, Y+H-1, W-2, Slider_colours[i][7]);
	OPM_SetPixel(Current_OPM, X+W-1, Y+H-1, Slider_colours[i][8]);

	/* Is the slider at the bottom ? */
	if (Scroll_bar->Slider_Y != Scroll_bar->Max_Y)
	{
		/* No -> Draw shadow below slider */
		Put_recoloured_box(Current_OPM, X, Y+H, W, 1, &(Recolour_tables[2][0]));
	}
}

