/************
 * NAME     : SCROLL.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 25-7-1994
 * PROJECT  : Scrolling playfield system
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : SCROLL.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>

#include <SCROLL.H>

/* global variables */
UNSHORT Scroll_OPM_index;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_scroll
 * FUNCTION  : Initialize scroll data.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 11:31
 * LAST      : 25.07.94 11:31
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             UNLONG X - Initial X-coordinate.
 *             UNLONG Y - Initial Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The dimensions, flags and viewport coordinates must have
 *              been set before calling this function. All other elements of
 *              the scroll data will be set by this function.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_scroll(struct Scroll_data *Scroll, UNLONG X, UNLONG Y)
{
	struct OPM *OPM;
	MEM_HANDLE Handle;
	UNSHORT W,H,i;
	UNBYTE *Ptr;

	/* Calculate dimensions of base OPM in units */
	Scroll->sbWidth = (Scroll->Viewport_width + Scroll->Unit_width-1)
	 / Scroll->Unit_width + 1;
  	Scroll->sbHeight = (Scroll->Viewport_height + Scroll->Unit_height-1)
	 / Scroll->Unit_height + 1;

	/* Calculate dimensions of base OPM in pixels */
	W = Scroll->sbWidth * Scroll->Unit_width;
	H = Scroll->sbHeight * Scroll->Unit_height;

	/* Allocate memory for base OPM */
	Handle = MEM_Do_allocate((UNLONG) W * H, (UNLONG) &(Scroll->Base_OPM), &OPM_ftype);
	Scroll->Base_OPM_handle = Handle;

	/* Initialize base OPM */
	Ptr = MEM_Claim_pointer(Handle);
	OPM_New(W, H, 1, &(Scroll->Base_OPM), Ptr);
	MEM_Free_pointer(Handle);

	/* Delete scroll OPMs (if any) */
	for (i=0;i<4;i++)
	{
		OPM = &(Scroll->Scroll_OPMs[i]);
		if (OPM->status & OPMSTAT_INIT)
			OPM_Del(OPM);
	}

	/* Reset scroll OPM index */
	Scroll_OPM_index = 0;

	/* Set playfield coordinates */
	Scroll->Playfield_X = X;
	Scroll->Playfield_Y = Y;

	/* Set unit coordinates */
	Scroll->Unit_X = Scroll->Playfield_X % Scroll->Unit_width;
	Scroll->Unit_Y = Scroll->Playfield_Y % Scroll->Unit_height;

	/* Clear vector */
	Scroll->Vector_X = 0;
	Scroll->Vector_Y = 0;

	/* Fill scroll buffer */
	Fill_scroll_buffer(Scroll);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_scroll
 * FUNCTION  : Terminate scroll data.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 12:11
 * LAST      : 25.07.94 12:11
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_scroll(struct Scroll_data *Scroll)
{
	struct OPM *OPM;
	UNSHORT i;

	/* Delete scroll OPMs (if any) */
	for (i=0;i<4;i++)
	{
		OPM = &(Scroll->Scroll_OPMs[i]);
		if (OPM->status & OPMSTAT_INIT)
			OPM_Del(OPM);
	}

	/* Delete base OPM */
	OPM_Del(&(Scroll->Base_OPM));

	/* Free base OPM memory */
	MEM_Free_memory(Scroll->Base_OPM_handle);
	Scroll->Base_OPM_handle = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_scroll_position
 * FUNCTION  : Set the position of the viewport.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 15:18
 * LAST      : 25.07.94 15:18
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             UNLONG X - New X-coordinate.
 *             UNLONG Y - New Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_scroll_position(struct Scroll_data *Scroll, UNLONG X, UNLONG Y)
{
	Do_scroll(Scroll, X - Scroll->Playfield_X, Y - Scroll->Playfield_Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fill_scroll_buffer
 * FUNCTION  : Fill the scroll buffer.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 13:32
 * LAST      : 25.07.94 13:32
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fill_scroll_buffer(struct Scroll_data *Scroll)
{
	UNSHORT sbX,sbY;
	UNSHORT pfX,pfY;
	UNSHORT i,j;

	/* Set viewport coordinates */
	Scroll->Viewport_X = Scroll->Unit_X;
	Scroll->Viewport_Y = Scroll->Unit_Y;

	/* Calculate initial playfield coordinates in units */
	pfX = Scroll->Playfield_X / Scroll->Unit_width;
	pfY = Scroll->Playfield_Y / Scroll->Unit_height;

	/* Fill base OPM */
	sbY = 0;
	for (i=0;i<Scroll->sbHeight;i++)
	{
		sbX = 0;
		for (j=0;j<Scroll->sbWidth;j++)
		{
			Scroll->Update_unit(Scroll, sbX, sbY, pfX+j, pfY+i);
			sbX += Scroll->Unit_width;
		}
		sbY += Scroll->Unit_height;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_scroll
 * FUNCTION  : Scroll a viewport within a playfield.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 11:42
 * LAST      : 25.07.94 11:42
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             SISHORT dX - X-component of scroll vector.
 *             SISHORT dY - Y-component of scroll vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_scroll(struct Scroll_data *Scroll, SISHORT dX, SISHORT dY)
{
	SILONG pfX, pfY;
	SISHORT X,Y;
	SISHORT Scroll_W = 0, Scroll_H = 0;
	UNSHORT Q;

	/* Update Playfield_X and Playfield_Y */
	pfX = Scroll->Playfield_X + dX;
	pfY = Scroll->Playfield_Y + dY;

	if (pfX < 0)
	{
		dX = 0 - Scroll->Playfield_X;
		pfX = 0;
	}

	if (pfX > Scroll->Playfield_width - Scroll->Viewport_width)
	{
		pfX = Scroll->Playfield_width - Scroll->Viewport_width;
		dX = pfX - Scroll->Playfield_X;
	}

	if (pfY < 0)
	{
		dY = 0 - Scroll->Playfield_Y;
		pfY = 0;
	}

	if (pfY > Scroll->Playfield_height - Scroll->Viewport_height)
	{
		pfY = Scroll->Playfield_height - Scroll->Viewport_height;
		dY = pfY - Scroll->Playfield_Y;
	}

	Scroll->Playfield_X = pfX;
	Scroll->Playfield_Y = pfY;

	/* Exit if scroll vector is zero */
	if (!dX && !dY)
	{
		Scroll->Vector_X = 0;
		Scroll->Vector_Y = 0;
		return;
	}

	/* Update Unit_X and Unit_Y and calculate the number of units that must
	  be added */
	X = Scroll->Unit_X + dX;
	Y = Scroll->Unit_Y + dY;

	if (dX > 0)
		Scroll_W = X / Scroll->Unit_width;
	if (dX < 0)
		Scroll_W = 0 - ((Scroll->Unit_width - X) / Scroll->Unit_width);

	if (dY > 0)
		Scroll_H = Y / Scroll->Unit_height;
	if (dY < 0)
		Scroll_H = 0 - ((Scroll->Unit_height - Y) / Scroll->Unit_height);

	while (X < 0)
		X += Scroll->Unit_width;

	while (X >= Scroll->Unit_width)
		X -= Scroll->Unit_width;

	while (Y < 0)
		Y += Scroll->Unit_height;

	while (Y >= Scroll->Unit_height)
		Y -= Scroll->Unit_height;

	Scroll->Unit_X = X;
	Scroll->Unit_Y = Y;

	/* Store scroll vector */
	Scroll->Vector_X = Scroll_W;
	Scroll->Vector_Y = Scroll_H;

	/* Update Viewport_X and Viewport_Y */
	X = Scroll->Viewport_X + dX;
	Y = Scroll->Viewport_Y + dY;

	while (X < 0)
		X += Scroll->Base_OPM.width;

	while (X >= Scroll->Base_OPM.width)
		X -= Scroll->Base_OPM.width;

	while (Y < 0)
		Y += Scroll->Base_OPM.height;

	while (Y >= Scroll->Base_OPM.height)
		Y -= Scroll->Base_OPM.height;

	Scroll->Viewport_X = X;
	Scroll->Viewport_Y = Y;

	/* Scroll at all ? */
	if ((abs(Scroll_W) >= Scroll->sbWidth) || (abs(Scroll_H) >= Scroll->sbHeight))
	{
		Fill_scroll_buffer(Scroll);
 		return;
	}

	/* Calculate scroll mode */
	Q = (sgn(Scroll_W) + 1) + 3 * (sgn(Scroll_H) + 1);

	Scroll_W = abs(Scroll_W);
	Scroll_H = abs(Scroll_H);

	/* Exit if no movement */
	if (Q == 4)
		return;

	/* Update scroll buffer */
	switch (Q)
	{
		case 0:
			Update_scroll_rectangle(Scroll, 0, 0, Scroll->sbWidth, Scroll_H);
			Update_scroll_rectangle(Scroll, 0, Scroll_H, Scroll_W, Scroll->sbHeight
			 - Scroll_H);
			break;
		case 1:
			Update_scroll_rectangle(Scroll, 0, 0, Scroll->sbWidth, Scroll_H);
			break;
		case 2:
			Update_scroll_rectangle(Scroll, 0, 0, Scroll->sbWidth, Scroll_H);
			Update_scroll_rectangle(Scroll, Scroll->sbWidth - Scroll_W, Scroll_H,
			 Scroll_W, Scroll->sbHeight - Scroll_H);
			break;
		case 3:
			Update_scroll_rectangle(Scroll, 0, 0, Scroll_W, Scroll->sbHeight);
			break;
		case 5:
			Update_scroll_rectangle(Scroll, Scroll->sbWidth - Scroll_W, 0,
			 Scroll_W, Scroll->sbHeight);
			break;
		case 6:
			Update_scroll_rectangle(Scroll, 0, Scroll->sbHeight - Scroll_H,
			 Scroll->sbWidth, Scroll_H);
			Update_scroll_rectangle(Scroll, 0, 0, Scroll_W, Scroll->sbHeight
			 - Scroll_H);
			break;
		case 7:
			Update_scroll_rectangle(Scroll, 0, Scroll->sbHeight - Scroll_H,
			 Scroll->sbWidth, Scroll_H);
			break;
		case 8:
			Update_scroll_rectangle(Scroll, 0, Scroll->sbHeight - Scroll_H,
			 Scroll->sbWidth, Scroll_H);
			Update_scroll_rectangle(Scroll, Scroll->sbWidth - Scroll_W, 0,
			 Scroll_W, Scroll->sbHeight - Scroll_H);
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_scroll_rectangle
 * FUNCTION  : Update a rectangle in the scroll buffer.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 13:32
 * LAST      : 25.07.94 13:32
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             UNSHORT X - Left X-coordinate of rectangle in units.
 *             UNSHORT Y - Top Y-coordinate of rectangle in units.
 *             UNSHORT Width - Width of rectangle in units.
 *             UNSHORT Height - Height of rectangle in units.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This routine bears similarities to Fill_scroll_buffer().
 *             - This routine will take care of ring mapping.
 *             - The input coordinates are relative to the top left of the
 *              viewport, NOT the scroll buffer.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_scroll_rectangle(struct Scroll_data *Scroll, UNSHORT X, UNSHORT Y,
 UNSHORT Width, UNSHORT Height)
{
	UNSHORT sbX,sbY,sbX2;
	UNSHORT pfX,pfY;
	UNSHORT i,j;

	/* Exit if width or height are too small */
	if (!Width || !Height)
		return;

	/* Calculate initial playfield coordinates in units */
	pfX = (Scroll->Playfield_X / Scroll->Unit_width) + X;
	pfY = (Scroll->Playfield_Y / Scroll->Unit_height) + Y;

	/* Calculate initial scroll buffer coordinates in units */
	sbX = ((Scroll->Viewport_X / Scroll->Unit_width) + X) * Scroll->Unit_width;
	while (sbX >= Scroll->Base_OPM.width)
		sbX -= Scroll->Base_OPM.width;

	sbY = ((Scroll->Viewport_Y / Scroll->Unit_height) + Y) * Scroll->Unit_height;
	while (sbY >= Scroll->Base_OPM.height)
		sbY -= Scroll->Base_OPM.height;

	/* Update a rectangle in the base OPM */
	sbX2 = sbX;
	for (i=0;i<Height;i++)
	{
		sbX = sbX2;
		for (j=0;j<Width;j++)
		{
			Scroll->Update_unit(Scroll, sbX, sbY, pfX+j, pfY+i);
			sbX += Scroll->Unit_width;
			/* Too far right ? */
			if (sbX >= Scroll->Base_OPM.width)
				/* Yes -> Back to the left */
				sbX -= Scroll->Base_OPM.width;
		}
		sbY += Scroll->Unit_height;
		/* Too far down ? */
		if (sbY >= Scroll->Base_OPM.height)
			/* Yes -> Back to the top */
			sbY -= Scroll->Base_OPM.height;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Scroll_display
 * FUNCTION  : Display the scroll viewport.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 15:29
 * LAST      : 25.07.94 15:29
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             struct SCREENPORT *Screen - Pointer to screenport.
 *             SISHORT X - Screen X-coordinate.
 *             SISHORT Y - Screen Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will not actually display the scroll viewport,
 *              but will generate the necessary virtual OPMs, which can be
 *              found in the Scroll data structure. It is the task of the
 *              caller to make sure that the virtual OPMs will be displayed.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Scroll_display(struct Scroll_data *Scroll, struct SCREENPORT *Screen,
 SISHORT X, SISHORT Y)
{
	struct OPM *OPM;
	UNSHORT i;

	/* Delete scroll OPMs (if any) */
	for (i=0;i<4;i++)
	{
		OPM = &(Scroll->Scroll_OPMs[i]);
		if (OPM->status & OPMSTAT_INIT)
			OPM_Del(OPM);
	}

	/* Reset scroll OPM index */
	Scroll_OPM_index = 0;

	/* Do it */
	Do_scroll_display(Scroll, Screen, X, Y, Scroll->Viewport_X,
	 Scroll->Viewport_Y, Scroll->Viewport_width, Scroll->Viewport_height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_scroll_display
 * FUNCTION  : Display a part of the scroll viewport.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 15:40
 * LAST      : 25.07.94 15:40
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             struct SCREENPORT *Screen - Pointer to screenport.
 *             SISHORT Screen_X - Screen X-coordinate.
 *             SISHORT Screen_Y - Screen Y-coordinate.
 *             UNSHORT Buffer_X - Left X-coordinate of viewport part.
 *             UNSHORT Buffer_Y - Top Y-coordinate of viewport part.
 *             UNSHORT Width - Width of viewport part.
 *             UNSHORT Height - Height of viewport part.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_scroll_display(struct Scroll_data *Scroll, struct SCREENPORT *Screen,
 SISHORT Screen_X, SISHORT Screen_Y, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Width, UNSHORT Height)
{
	UNSHORT V;

	/* Off the right side ? */
	if (Buffer_X + Width > Scroll->Base_OPM.width)
	{
		/* Yes -> Split */
		V = Scroll->Base_OPM.width - Buffer_X;

		Do_scroll_display(Scroll, Screen, Screen_X, Screen_Y, Buffer_X,
		 Buffer_Y, V, Height);

		Do_scroll_display(Scroll, Screen, Screen_X + V, Screen_Y, 0,
		 Buffer_Y, Width - V, Height);

		return;
	}

	/* Off the bottom side ? */
	if (Buffer_Y + Height > Scroll->Base_OPM.height)
	{
		/* Yes -> Split */
		V = Scroll->Base_OPM.height - Buffer_Y;

		Do_scroll_display(Scroll, Screen, Screen_X, Screen_Y, Buffer_X,
		 Buffer_Y, Width, V);

		Do_scroll_display(Scroll, Screen, Screen_X, Screen_Y + V, Buffer_X,
		 0, Width, Height - V);

		return;
	}

	/* Build virtual OPM */
	OPM_CreateVirtualOPM(&(Scroll->Base_OPM), &(Scroll->Scroll_OPMs[Scroll_OPM_index]),
	 Screen_X, Screen_Y, Width, Height);
	OPM_SetVirtualPosition(&(Scroll->Scroll_OPMs[Scroll_OPM_index]), Buffer_X, Buffer_Y);
	Scroll_OPM_index++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Scroll_display_OPM
 * FUNCTION  : Display the scroll viewport in an OPM.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.08.94 12:18
 * LAST      : 29.08.94 12:18
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             struct OPM *OPM - Pointer to OPM.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Unlike Scroll_display(), this function DOES display the
 *              scroll viewport directly in the target OPM. Naturally, it is
 *              still the responsibility of the caller to make sure that the
 *              target OPM is displayed on the screen.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Scroll_display_OPM(struct Scroll_data *Scroll, struct OPM *OPM,
 SISHORT X, SISHORT Y)
{
	/* Do it */
	Do_scroll_display_OPM(Scroll, OPM, X, Y, Scroll->Viewport_X,
		Scroll->Viewport_Y, Scroll->Viewport_width, Scroll->Viewport_height);

	/* Target OPM was changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_scroll_display_OPM
 * FUNCTION  : Display a part of the scroll viewport in an OPM.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.08.94 12:19
 * LAST      : 29.08.94 12:19
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             struct OPM *OPM - Pointer to OPM.
 *             SISHORT Screen_X - X-coordinate.
 *             SISHORT Screen_Y - Y-coordinate.
 *             UNSHORT Buffer_X - Left X-coordinate of viewport part.
 *             UNSHORT Buffer_Y - Top Y-coordinate of viewport part.
 *             UNSHORT Width - Width of viewport part.
 *             UNSHORT Height - Height of viewport part.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_scroll_display_OPM(struct Scroll_data *Scroll, struct OPM *OPM,
 SISHORT Screen_X, SISHORT Screen_Y, UNSHORT Buffer_X, UNSHORT Buffer_Y,
 UNSHORT Width, UNSHORT Height)
{
	UNSHORT V;

	/* Off the right side ? */
	if (Buffer_X + Width > Scroll->Base_OPM.width)
	{
		/* Yes -> Split */
		V = Scroll->Base_OPM.width - Buffer_X;

		Do_scroll_display_OPM(Scroll, OPM, Screen_X, Screen_Y, Buffer_X,
		 Buffer_Y, V, Height);

		Do_scroll_display_OPM(Scroll, OPM, Screen_X + V, Screen_Y, 0,
		 Buffer_Y, Width - V, Height);

		return;
	}

	/* Off the bottom side ? */
	if (Buffer_Y + Height > Scroll->Base_OPM.height)
	{
		/* Yes -> Split */
		V = Scroll->Base_OPM.height - Buffer_Y;

		Do_scroll_display_OPM(Scroll, OPM, Screen_X, Screen_Y, Buffer_X,
		 Buffer_Y, Width, V);

		Do_scroll_display_OPM(Scroll, OPM, Screen_X, Screen_Y + V, Buffer_X,
		 0, Width, Height - V);

		return;
	}

	/* Display part of scroll OPM in target OPM */
	OPM_CopyOPMOPM(&(Scroll->Base_OPM), OPM, Buffer_X, Buffer_Y, Width, Height, Screen_X, Screen_Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_scroll_coordinates
 * FUNCTION  : Convert playfield coordinates to scroll OPM coordinates.
 * FILE      : SCROLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 17:41
 * LAST      : 28.07.94 17:41
 * INPUTS    : struct Scroll_data *Scroll - Pointer to scroll data.
 *             SILONG *X_coord - Pointer to X-coordinate.
 *             SILONG *Y_coord - Pointer to Y-coordinate.
 * RESULT    : None (Coordinates will be changed).
 * BUGS      : No known.
 * NOTE      : - Never display anything larger than a unit in the scroll
 *              OPM if you don't know what you're doing.
 *             - If the coordinates fall outside of the scroll OPM, the
 *              coordinate pair (-32768,-32768) is returned.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Convert_scroll_coordinates(struct Scroll_data *Scroll, SILONG *X_coord,
 SILONG *Y_coord)
{
	SILONG X,Y;
	SILONG W,H;

	/* Get coordinates */
	X = *X_coord;
	Y = *Y_coord;

	/* Get scroll buffer dimensions */
	W = (SILONG) Scroll->Base_OPM.width;
	H = (SILONG) Scroll->Base_OPM.height;

	/* Convert to scroll buffer coordinates */
	X = X - Scroll->Playfield_X;
	Y = Y - Scroll->Playfield_Y;

	/* Inside the scroll buffer ? */
 	if ((X <= 0 - Scroll->Unit_width) || (X >= W - Scroll->Unit_width)
	 || (Y <= 0 - Scroll->Unit_height) || (Y >= H - Scroll->Unit_height))
	{
		/* No -> Exit */
		*X_coord = -32768;
		*Y_coord = -32768;
		return;
	}

	/* Yes -> Add viewport coordinates */
	X += (SILONG) Scroll->Viewport_X;
	Y += (SILONG) Scroll->Viewport_Y;

	/* Wrap */
	if (X >= W)
		X -= W;

	if (Y >= H)
		Y -= H;

	/* Store output coordinates */
	*X_coord = X;
	*Y_coord = Y;
}

