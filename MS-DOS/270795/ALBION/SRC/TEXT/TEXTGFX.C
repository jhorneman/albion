/************
 * NAME     : TEXTGFC.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 24-8-1994
 * PROJECT  : Text functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : TEXT.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>

#include <TEXT.H>
#include "TEXTVAR.H"

#include <FONT.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_character
 * FUNCTION  : Put a character in an OPM.
 * FILE      : TEXTGFX.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 13:59
 * LAST      : 30.01.95 15:42
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to character graphics.
 *             UNSHORT Colour - Index to colour table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXTDISP.C
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = OPM->clip.left;
	cy1 = OPM->clip.top;
	cx2 = cx1 + OPM->clip.width - 1;
	cy2 = cy1 + OPM->clip.height - 1;

	/* Add OPM's offset coordinates */
	X += OPM->xoffset;
	Y += OPM->yoffset;

	/* Get block's bottom-right coordinates */
	X2 = X + Width - 1;
	Y2 = Y + Height - 1;

	/* Is the block completely out ? */
	if ((X2 < cx1) || (X > cx2) || (Y2 < cy1) || (Y > cy2))
		return;

	/* Clip left ? */
	if (X < cx1)
	{
		/* Yes */
		Width = X2 - cx1 + 1;
		Graphics_ptr += (cx1 - X);
		X = cx1;
	}

	/* Clip right ? */
	if (X2 > cx2)
	{
		/* Yes */
		Width = cx2 - X + 1;
		X2 = cx2;
	}

	/* Clip top ? */
	if (Y < cy1)
	{
		/* Yes */
		Height = Y2 - cy1 + 1;
		Graphics_ptr += (cy1 - Y) * Original_width;
		Y = cy1;
	}

	/* Clip bottom ? */
	if (Y2 > cy2)
	{
		/* Yes */
		Height = cy2 - Y + 1;
		Y2 = cy2;
	}

	/* Exit if nothing is left */
	if ((Width < 1) || (Height < 1))
		return;

	/* Do it */
	{
		UNSHORT i, j;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
			{
				c = *Source_ptr++;
				if (c)
				{
					*Destination_ptr = Text_colours[Colour][c-1];
				}
				Destination_ptr++;
			}
			Source_ptr += Original_width - Width;
			Destination_ptr += Modulo - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_fat_character
 * FUNCTION  : Put a fat character in an OPM.
 * FILE      : TEXTGFX.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 13:59
 * LAST      : 30.01.95 15:42
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to character graphics.
 *             UNSHORT Colour - Index to colour table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXTDISP.C
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_fat_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = OPM->clip.left;
	cy1 = OPM->clip.top;
	cx2 = cx1 + OPM->clip.width - 1;
	cy2 = cy1 + OPM->clip.height - 1;

	/* Add OPM's offset coordinates */
	X += OPM->xoffset;
	Y += OPM->yoffset;

	/* Get block's bottom-right coordinates */
	X2 = X + Width - 1;
	Y2 = Y + Height - 1;

	/* Is the block completely out ? */
	if ((X2 < cx1) || (X > cx2) || (Y2 < cy1) || (Y > cy2))
		return;

	/* Clip left ? */
	if (X < cx1)
	{
		/* Yes */
		Width = X2 - cx1 + 1;
		Graphics_ptr += (cx1 - X);
		X = cx1;
	}

	/* Clip right ? */
	if (X2 > cx2)
	{
		/* Yes */
		Width = cx2 - X + 1;
		X2 = cx2;
	}

	/* Clip top ? */
	if (Y < cy1)
	{
		/* Yes */
		Height = Y2 - cy1 + 1;
		Graphics_ptr += (cy1 - Y) * Original_width;
		Y = cy1;
	}

	/* Clip bottom ? */
	if (Y2 > cy2)
	{
		/* Yes */
		Height = cy2 - Y + 1;
		Y2 = cy2;
	}

	/* Exit if nothing is left */
	if ((Width < 1) || (Height < 1))
		return;

	/* Do it */
	{
		UNSHORT i, j, Original_X;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		Original_X = X;
		for (i=0;i<Height;i++)
		{
			X = Original_X;
			for (j=0;j<Width;j++)
			{
				c = *Source_ptr++;
				if (c)
				{
					*Destination_ptr = Text_colours[Colour][c-1];

					if (X < cx2)
	  					*(Destination_ptr + 1) = Text_colours[Colour][c-1];
				}
				X++;
				Destination_ptr++;
			}
			Y++;
			Source_ptr += Original_width - Width;
			Destination_ptr += Modulo - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_high_character
 * FUNCTION  : Put a high character in an OPM.
 * FILE      : TEXTGFX.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.08.94 15:11
 * LAST      : 30.01.95 15:42
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to character graphics.
 *             UNSHORT Colour - Index to colour table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The height is the output height.
 * SEE ALSO  : TEXTDISP.C
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_high_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = OPM->clip.left;
	cy1 = OPM->clip.top;
	cx2 = cx1 + OPM->clip.width - 1;
	cy2 = cy1 + OPM->clip.height - 1;

	/* Add OPM's offset coordinates */
	X += OPM->xoffset;
	Y += OPM->yoffset;

	/* Get block's bottom-right coordinates */
	X2 = X + Width - 1;
	Y2 = Y + Height - 1;

	/* Is the block completely out ? */
	if ((X2 < cx1) || (X > cx2) || (Y2 < cy1) || (Y > cy2))
		return;

	/* Clip left ? */
	if (X < cx1)
	{
		/* Yes */
		Width = X2 - cx1 + 1;
		Graphics_ptr += (cx1 - X);
		X = cx1;
	}

	/* Clip right ? */
	if (X2 > cx2)
	{
		/* Yes */
		Width = cx2 - X + 1;
		X2 = cx2;
	}

	/* Clip top ? */
	if (Y < cy1)
	{
		/* Yes */
		Height = Y2 - cy1 + 1;
		Graphics_ptr += (cy1 - Y) * Original_width;
		Y = cy1;
	}

	/* Clip bottom ? */
	if (Y2 > cy2)
	{
		/* Yes */
		Height = cy2 - Y + 1;
		Y2 = cy2;
	}

	/* Exit if nothing is left */
	if ((Width < 1) || (Height < 1))
		return;

	/* Do it */
	{
		UNSHORT i, j, Original_X;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		Original_X = X;
		for (i=0;i<Height/2;i++)
		{
			X = Original_X;
			for (j=0;j<Width;j++)
			{
				c = *Source_ptr++;
				if (c)
				{
					*Destination_ptr = Text_colours[Colour][c-1];

					if (Y < cy2)
						*(Destination_ptr + Modulo) = Text_colours[Colour][c-1];
				}
				X++;
				Destination_ptr++;
			}
			Y += 2;
			Source_ptr += Original_width - Width;
			Destination_ptr += (Modulo * 2) - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_fat_high_character
 * FUNCTION  : Put a fat, high character in an OPM.
 * FILE      : TEXTGFX.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.01.95 15:42
 * LAST      : 30.01.95 15:42
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to character graphics.
 *             UNSHORT Colour - Index to colour table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The height is the output height.
 * SEE ALSO  : TEXTDISP.C
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_fat_high_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = OPM->clip.left;
	cy1 = OPM->clip.top;
	cx2 = cx1 + OPM->clip.width - 1;
	cy2 = cy1 + OPM->clip.height - 1;

	/* Add OPM's offset coordinates */
	X += OPM->xoffset;
	Y += OPM->yoffset;

	/* Get block's bottom-right coordinates */
	X2 = X + Width - 1;
	Y2 = Y + Height - 1;

	/* Is the block completely out ? */
	if ((X2 < cx1) || (X > cx2) || (Y2 < cy1) || (Y > cy2))
		return;

	/* Clip left ? */
	if (X < cx1)
	{
		/* Yes */
		Width = X2 - cx1 + 1;
		Graphics_ptr += (cx1 - X);
		X = cx1;
	}

	/* Clip right ? */
	if (X2 > cx2)
	{
		/* Yes */
		Width = cx2 - X + 1;
		X2 = cx2;
	}

	/* Clip top ? */
	if (Y < cy1)
	{
		/* Yes */
		Height = Y2 - cy1 + 1;
		Graphics_ptr += (cy1 - Y) * Original_width;
		Y = cy1;
	}

	/* Clip bottom ? */
	if (Y2 > cy2)
	{
		/* Yes */
		Height = cy2 - Y + 1;
		Y2 = cy2;
	}

	/* Exit if nothing is left */
	if ((Width < 1) || (Height < 1))
		return;

	/* Do it */
	{
		UNSHORT i, j, Original_X;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		Original_X = X;
		for (i=0;i<Height/2;i++)
		{
			X = Original_X;
			for (j=0;j<Width;j++)
			{
				c = *Source_ptr++;
				if (c)
				{
					*Destination_ptr = Text_colours[Colour][c-1];

					if (X < cx2)
	  					*(Destination_ptr + 1) = Text_colours[Colour][c-1];

					if (Y < cy2)
					{
						*(Destination_ptr + Modulo) = Text_colours[Colour][c-1];

						if (X < cx2)
		  					*(Destination_ptr + Modulo + 1) = Text_colours[Colour][c-1];
					}
				}
				X++;
				Destination_ptr++;
			}
			Y += 2;
			Source_ptr += Original_width - Width;
			Destination_ptr += (Modulo * 2) - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_big_character
 * FUNCTION  : Put a big character in an OPM.
 * FILE      : TEXTGFX.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.02.95 13:57
 * LAST      : 03.02.95 13:57
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to character graphics.
 *             UNSHORT Colour - Index to colour table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The width and height are the output width and height.
 * SEE ALSO  : TEXTDISP.C
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_big_character(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNSHORT Colour)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = OPM->clip.left;
	cy1 = OPM->clip.top;
	cx2 = cx1 + OPM->clip.width - 1;
	cy2 = cy1 + OPM->clip.height - 1;

	/* Add OPM's offset coordinates */
	X += OPM->xoffset;
	Y += OPM->yoffset;

	/* Get block's bottom-right coordinates */
	X2 = X + Width - 1;
	Y2 = Y + Height - 1;

	/* Is the block completely out ? */
	if ((X2 < cx1) || (X > cx2) || (Y2 < cy1) || (Y > cy2))
		return;

	/* Clip left ? */
	if (X < cx1)
	{
		/* Yes */
		Width = X2 - cx1 + 1;
		Graphics_ptr += (cx1 - X) / 2;
		X = cx1;
	}

	/* Clip right ? */
	if (X2 > cx2)
	{
		/* Yes */
		Width = cx2 - X + 1;
		X2 = cx2;
	}

	/* Clip top ? */
	if (Y < cy1)
	{
		/* Yes */
		Height = Y2 - cy1 + 1;
		Graphics_ptr += (cy1 - Y) * Original_width;
		Y = cy1;
	}

	/* Clip bottom ? */
	if (Y2 > cy2)
	{
		/* Yes */
		Height = cy2 - Y + 1;
		Y2 = cy2;
	}

	/* Exit if nothing is left */
	if ((Width < 1) || (Height < 1))
		return;

	/* Do it */
	{
		UNSHORT i, j, Original_X;
		UNBYTE c;
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE *Source_ptr2, *Destination_ptr2;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		Original_X = X;
		Source_ptr2 = Source_ptr;
		Destination_ptr2 = Destination_ptr;
		for (i=0;i<Height/2;i++)
		{
			X = Original_X;
			Source_ptr = Source_ptr2;
			Destination_ptr = Destination_ptr2;

			for (j=0;j<Width/2;j++)
			{
				c = *Source_ptr++;
				if (c)
				{
					*Destination_ptr = Text_colours[Colour][c-1];

					if (X < cx2)
	  					*(Destination_ptr + 1) = Text_colours[Colour][c-1];

					if (Y < cy2)
					{
						*(Destination_ptr + Modulo) = Text_colours[Colour][c-1];

						if (X < cx2)
		  					*(Destination_ptr + Modulo + 1) = Text_colours[Colour][c-1];
					}
				}
				X += 2;
				Destination_ptr += 2;
			}
			Y += 2;
			Source_ptr2 += Original_width / 2;
			Destination_ptr2 += Modulo * 2;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

