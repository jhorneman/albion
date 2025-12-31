/************
 * NAME     : GFXFUNC.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 20-7-1994
 * PROJECT  : Graphics functions
 * NOTES    :
 * SEE ALSO : GFXFUNC.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBXOPM.H>
#include <BBBASMEM.H>

#include <GFXFUNC.H>
#include <XXOPM.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_block
 * FUNCTION  : Copies a block from an XOPM to memory.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.94 12:03
 * LAST      : 21.07.94 12.03
 * INPUTS    : struct XOPM * XOPM - Pointer to source XOPM.
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
Get_block(struct XOPM *XOPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Buffer)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Do it */
	BASEMEM_CopyBlock((XOPM->OPM.data + (Y * Modulo) + X), Buffer, Modulo - Width,
	 Original_width - Width, Width, Height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_unmasked_block
 * FUNCTION  : Put an unmasked block in an XOPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.94 12:36
 * LAST      : 21.07.94 12.36
 * INPUTS    : struct XOPM * XOPM - Pointer to target XOPM.
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
Put_unmasked_block(struct XOPM *XOPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Do it */
	BASEMEM_CopyBlock(Graphics_ptr, (XOPM->OPM.data + (Y * Modulo) + X),
	 Original_width - Width, Modulo - Width, Width, Height);

/*	{
		UNSHORT i,j;
		UNBYTE *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (XOPM->OPM.data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
				*Destination_ptr++ = *Source_ptr++;
			Source_ptr += Original_width - Width;
			Destination_ptr += Modulo - Width;
		}
	}*/
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_masked_block
 * FUNCTION  : Put a masked block in an XOPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:10
 * LAST      : 22.07.94 10.10
 * INPUTS    : struct XOPM * XOPM - Pointer to target XOPM.
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
Put_masked_block(struct XOPM *XOPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Do it */
	{
		UNSHORT i,j;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (XOPM->OPM.data + (Y * Modulo) + X);

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
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_silhouette
 * FUNCTION  : Put a silhouette of a block in an XOPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:23
 * LAST      : 22.07.94 10.23
 * INPUTS    : struct XOPM * XOPM - Pointer to target XOPM.
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
Put_silhouette(struct XOPM *XOPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE Colour)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Do it */
	{
		UNSHORT i,j;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (XOPM->OPM.data + (Y * Modulo) + X);

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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_recoloured_block
 * FUNCTION  : Recolour the silhouette of a block in an XOPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:23
 * LAST      : 22.07.94 10.23
 * INPUTS    : struct XOPM * XOPM - Pointer to target XOPM.
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
Put_recoloured_block(struct XOPM *XOPM, SISHORT X, SISHORT Y,
 SISHORT Width, SISHORT Height, UNBYTE *Graphics_ptr, UNBYTE *Recolour_table)
{
	UNSHORT Original_width, Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Save width */
	Original_width = Width;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Do it */
	{
		UNSHORT i,j;
		UNBYTE c, *Source_ptr, *Destination_ptr;

		Source_ptr = Graphics_ptr;
		Destination_ptr = (XOPM->OPM.data + (Y * Modulo) + X);

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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_recoloured_box
 * FUNCTION  : Recolour a box in an XOPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 10:30
 * LAST      : 22.07.94 10.30
 * INPUTS    : struct XOPM * XOPM - Pointer to target XOPM.
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
Put_recoloured_box(struct XOPM *XOPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, UNBYTE *Recolour_table)
{
	UNSHORT Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Do it */
	{
		UNSHORT i,j;
		UNBYTE c, *Destination_ptr;

		Destination_ptr = (XOPM->OPM.data + (Y * Modulo) + X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
			{
				c = *Destination_ptr;
				*Destination_ptr++ = Recolour_table[c];
			}
			Destination_ptr += Modulo - Width;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Scroll_block
 * FUNCTION  : Scroll a block in an XOPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.94 12:24
 * LAST      : 21.07.94 12.24
 * INPUTS    : struct XOPM * XOPM - Pointer to source XOPM.
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
Scroll_block(struct XOPM *XOPM, SISHORT X, SISHORT Y, SISHORT Width,
 SISHORT Height, SISHORT Delta_X, SISHORT Delta_Y)
{
	UNSHORT Modulo;
	SISHORT cx1,cy1,cx2,cy2;
	SISHORT X2, Y2;

	Modulo = XOPM->OPM.nextypos;

	/* Get clipping rectangle */
	cx1 = XOPM->OPM.clip.left;
	cy1 = XOPM->OPM.clip.top;
	cx2 = cx1 + XOPM->OPM.clip.width - 1;
	cy2 = cy1 + XOPM->OPM.clip.height - 1;

	/* Add XOPM's offset coordinates */
	X += XOPM->OPM.xoffset;
	Y += XOPM->OPM.yoffset;

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

	/* Check XOPM updates */
	Check_XOPM_update(XOPM, X, Y, Width, Height);

	/* Forwards or backwards ? */
	if ((Delta_Y > 0) || ((Delta_X > 0) && (Delta_Y == 0)))
	{
		/* Backwards */
		UNSHORT i,j;
		UNBYTE *Source_ptr, *Destination_ptr;

		Source_ptr = (XOPM->OPM.data + ((Y + Height -1 - Delta_Y) * Modulo) +
		 X + Width -1 - Delta_X);
		Destination_ptr = (XOPM->OPM.data + ((Y + Height - 1) * Modulo)
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
		UNSHORT i,j;
		UNBYTE *Source_ptr, *Destination_ptr;

		Source_ptr = (XOPM->OPM.data + (Y * Modulo) + X);
		Destination_ptr = (XOPM->OPM.data + ((Y + Delta_Y) * Modulo)
		 + X + Delta_X);

		for (i=0;i<Height;i++)
		{
			for (j=0;j<Width;j++)
				*Destination_ptr++ = *Source_ptr++;
			Source_ptr += Modulo - Width;
			Destination_ptr += Modulo - Width;
		}
	}
}

