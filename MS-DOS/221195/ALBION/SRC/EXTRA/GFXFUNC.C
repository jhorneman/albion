/************
 * NAME     : GFXFUNC.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 20-7-1994
 * PROJECT  : Graphics functions
 * NOTES    :
 * SEE ALSO : GFXFUNC.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>

#include <GFXFUNC.H>
#include <FINDCOL.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_block
 * FUNCTION  : Copies a block from an OPM to memory.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.94 12:03
 * LAST      : 21.07.94 12.03
 * INPUTS    : struct OPM * OPM - Pointer to source OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Buffer - Pointer to target buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Clipping will be done correctly. If the buffer is displayed
 *              at the same coordinates within the same clipping rectangle,
 *              the correct portion will be displayed.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Buffer)
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
		Buffer += (cx1 - X);
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
		Buffer += (cy1 - Y) * Original_width;
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
	OPM_CopyBlock((OPM->data + (Y * Modulo) + X), Buffer, Modulo - Width,
	 Original_width - Width, Width, Height);

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_unmasked_block
 * FUNCTION  : Put an unmasked block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.94 12:36
 * LAST      : 21.07.94 12.36
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_unmasked_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr)
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
	OPM_CopyBlock(Graphics_ptr, (OPM->data + (Y * Modulo) + X),
	 Original_width - Width, Modulo - Width, Width, Height);

/*	{
		UNSHORT i, j;
		UNBYTE *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
				*Destination_ptr++ = *Source_ptr++;
			Source_ptr += Original_width - Width;
			Destination_ptr += Modulo - Width;
		}
	}*/

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_masked_block
 * FUNCTION  : Put a masked block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:10
 * LAST      : 22.07.94 10.10
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_masked_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr)
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
	OPM_CopyBlockTrans(Graphics_ptr, (OPM->data + (Y * Modulo) + X),
	 Original_width - Width, Modulo - Width, Width, Height, 0);

/*	{
		UNSHORT i, j;
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE c;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
			{
				c = *Source_ptr++;
				if (c)
					*Destination_ptr = c;
				Destination_ptr++;
			}
			Source_ptr += Original_width - Width;
			Destination_ptr += Modulo - Width;
		}
	} */

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_silhouette
 * FUNCTION  : Put a silhouette of a block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:23
 * LAST      : 22.07.94 10.23
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNBYTE Colour - Silhouette colour.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_silhouette(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE Colour)
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
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE c;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
			{
				c = *Source_ptr++;
				if (c)
					*Destination_ptr = Colour;
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
 * NAME      : Put_recoloured_block
 * FUNCTION  : Recolour the silhouette of a block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:23
 * LAST      : 22.07.94 10.23
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNBYTE *Recolour_table - Pointer to recolouring table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_recoloured_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE *Recolour_table)
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
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE c;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
			{
				if (*Source_ptr++)
				{
					c = *Destination_ptr;
					*Destination_ptr = Recolour_table[c];
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
 * NAME      : Put_recoloured_box
 * FUNCTION  : Recolour a box in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:30
 * LAST      : 22.07.94 10.30
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of box.
 *             SISHORT Height - Height of box.
 *             UNBYTE *Recolour_table - Pointer to recolouring table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_recoloured_box(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Recolour_table)
{
	UNSHORT Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

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
	_OPM_ASS_ChangeCol((OPM->data + (Y * Modulo) + X), Recolour_table,
	 Modulo - Width, Width, Height);

/*	{
		UNSHORT i, j;
 		UNBYTE *Destination_ptr;
		UNBYTE c;

		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
			{
				c = *Destination_ptr;
				*Destination_ptr++ = Recolour_table[c];
			}
			Destination_ptr += Modulo - Width;
		}
	} */

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Scroll_block
 * FUNCTION  : Scroll a block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.94 12:24
 * LAST      : 21.07.94 12.24
 * INPUTS    : struct OPM * OPM - Pointer to source OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             SISHORT Delta_X - X-displacement.
 *             SISHORT Delta_Y - Y-displacement.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Scroll_block(struct OPM *OPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, SISHORT Delta_X, SISHORT Delta_Y)
{
	UNSHORT Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = OPM->nextypos;

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
	if ((Width < abs(Delta_X)) || (Height < abs(Delta_Y)))
		return;
	if ((Width < 1) || (Height < 1))
		return;

	/* Forwards or backwards ? */
	if ((Delta_Y > 0) || ((Delta_X > 0) && (Delta_Y == 0)))
	{
		/* Backwards */
		UNSHORT i, j;
		UNBYTE *Source_ptr, *Destination_ptr;

		Source_ptr = (OPM->data + ((Y + Height -1 - Delta_Y) * Modulo) +
		 X + Width -1 - Delta_X);
		Destination_ptr = (OPM->data + ((Y + Height - 1) * Modulo)
		 + X + Width -1);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
				*Destination_ptr-- = *Source_ptr--;
			Source_ptr -= Modulo - Width;
			Destination_ptr -= Modulo - Width;
		}
	}
	else
	{
		/* Forwards */
		UNSHORT i, j;
		UNBYTE *Source_ptr, *Destination_ptr;

		Source_ptr = (OPM->data + (Y * Modulo) + X);
		Destination_ptr = (OPM->data + ((Y + Delta_Y) * Modulo)
		 + X + Delta_X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
				*Destination_ptr++ = *Source_ptr++;
			Source_ptr += Modulo - Width;
			Destination_ptr += Modulo - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_unmasked_X_mirrored_block
 * FUNCTION  : Put an unmasked block in an OPM, mirrored around the Y-axis.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 12:28
 * LAST      : 23.06.95 12:28
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_unmasked_X_mirrored_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr)
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
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE *Source_ptr2;

		Source_ptr = Graphics_ptr + Original_width;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			Source_ptr2 = Source_ptr;
			for (j=0;j<Width;j++)
				*Destination_ptr++ = *(--Source_ptr2);
			Source_ptr += Original_width;
			Destination_ptr += Modulo - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_masked_X_mirrored_block
 * FUNCTION  : Put a masked block in an OPM, mirrored around the Y-axis.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 12:28
 * LAST      : 23.06.95 12:28
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_masked_X_mirrored_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr)
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
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE *Source_ptr2;
		UNBYTE c;

		Source_ptr = Graphics_ptr + Original_width;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			Source_ptr2 = Source_ptr;
			for (j=0;j<Width;j++)
			{
				c = *(--Source_ptr2);
				if (c)
					*Destination_ptr = c;
				Destination_ptr++;
			}
			Source_ptr += Original_width;
			Destination_ptr += Modulo - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_recoloured_X_mirrored_block
 * FUNCTION  : Recolour the silhouette of a block in an OPM, mirrored
 *              around the Y-axis.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.06.95 13:06
 * LAST      : 23.06.95 13:06
 * INPUTS    : struct OPM * OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Width - Width of block.
 *             SISHORT Height - Height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNBYTE *Recolour_table - Pointer to recolouring table.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GFXFUNC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_recoloured_X_mirrored_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE *Recolour_table)
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
		UNBYTE *Source_ptr, *Destination_ptr;
		UNBYTE *Source_ptr2;
		UNBYTE c;

		Source_ptr = Graphics_ptr + Original_width;
		Destination_ptr = (OPM->data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			Source_ptr2 = Source_ptr;
			for (j=0;j<Width;j++)
			{
				if (*(--Source_ptr2))
				{
					c = *Destination_ptr;
					*Destination_ptr = Recolour_table[c];
				}
				Destination_ptr++;
			}
			Source_ptr += Original_width;
			Destination_ptr += Modulo - Width;
		}
	}

	/* OPM has changed */
	OPM->status |= OPMSTAT_CHANGED;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_nr_frames
 * FUNCTION  : Get the number of frames out of a standard graphics header.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 15:08
 * LAST      : 20.03.95 15:08
 * INPUTS    : MEM_HANDLE Handle - Handle of graphics memory.
 * RESULT    : UNSHORT: Number of animation frames.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_nr_frames(MEM_HANDLE Handle)
{
	struct Gfx_header *Gfx;
	UNSHORT Nr_frames;

	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Handle);

	Nr_frames = Gfx->Nr_frames;

	MEM_Free_pointer(Handle);

	return Nr_frames;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_gfx_size
 * FUNCTION  : Get size of a block out of a standard graphics header.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.10.95 16:38
 * LAST      : 16.10.95 16:38
 * INPUTS    : MEM_HANDLE Handle - Handle of graphics memory.
 *             UNSHORT *Width_ptr - Pointer to width.
 *             UNSHORT *Height_ptr - Pointer to height.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_gfx_size(MEM_HANDLE Handle, UNSHORT *Width_ptr, UNSHORT *Height_ptr)
{
	struct Gfx_header *Gfx;

	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Handle);

	*Width_ptr	= Gfx->Width;
	*Height_ptr	= Gfx->Height;

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_enlarged_block
 * FUNCTION  : Draw a block in a specially enlarged way.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.09.95 20:14
 * LAST      : 09.09.95 20:24
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Source_width - Width of source graphics.
 *             UNSHORT Source_height - Height of source graphics.
 *             UNBYTE *Graphics_ptr - Pointer to source graphics.
 *             struct BBPALETTE *Palette_ptr - Pointer to palette.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The block will be displayed 4 times as big.
 *             - The draw behaviour can be easily modified using tables.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_enlarged_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNBYTE *Graphics_ptr,
 struct BBPALETTE *Palette_ptr)
{
	static const UNSHORT Enlarged_draw_table[4][4] = {
		{ 1, 0, 2, 3 },
		{ 0, 0, 2, 3 },
		{ 2, 2, 2, 3 },
		{ 3, 3, 3, 3 }
	};
	static const UNSHORT Enlarged_brightnesses[4] = { 0, 125, 75, 50 };

	UNSHORT Original_X;
	UNSHORT R, G, B;
	UNSHORT Colours[4];
	UNSHORT i, j, k, l;

	/* Save X-coordinate */
	Original_X = X;

	/* Handle each row */
	for (i=0;i<Source_height;i++)
	{
		/* Restore X-coordinate */
		X = Original_X;

		/* Handle each column */
		for (j=0;j<Source_width;j++)
		{
			/* Get pixel colour */
			Colours[0] = *Graphics_ptr++;

			/* Transparent ? */
			if (Colours[0])
			{
				/* No -> Get R, G, and B values */
				R = Palette_ptr->color[Colours[0]].red;
				G = Palette_ptr->color[Colours[0]].green;
			   B = Palette_ptr->color[Colours[0]].blue;

				/* Calculate other colours */
				for (k=1;k<4;k++)
				{
					Colours[k] = Find_closest_colour
					(
						(R * Enlarged_brightnesses[k]) / 100,
						(G * Enlarged_brightnesses[k]) / 100,
						(B * Enlarged_brightnesses[k]) / 100
					);
				}

				/* Draw enlarged pixel */
				for (k=0;k<4;k++)
				{
					for (l=0;l<4;l++)
					{
						OPM_SetPixel
						(
							OPM,
							X + l,
							Y + k,
							Colours[Enlarged_draw_table[k][l]]
						);
					}
				}
			}

			/* Next X */
			X += 4;
		}
		/* Next Y */
		Y += 4;
	}
}

