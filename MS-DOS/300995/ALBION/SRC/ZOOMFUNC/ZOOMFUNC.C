/************
 * NAME     : ZOOMFUNC.c
 * AUTOR    : R.Reber, BlueByte
 * START    : 26.05.94 08:00
 * PROJECT  : Albion
 * NOTES    : Zoomfunktionen fÅr AlbionKampf
 * SEE ALSO :
 * VERSION  : 1.0
 ************/

#include <stdio.h>

#include <BBDEF.H>

#include <GFXFUNC.H>
#include <ZOOMFUNC.H>

/* Externe Variablen aus ZOOM.ASM */
/* Clipping Grenzen */
extern SILONG ZOOMFUNC_DESTCLIPX0;
extern SILONG ZOOMFUNC_DESTCLIPY0;
extern SILONG ZOOMFUNC_DESTCLIPX1;
extern SILONG ZOOMFUNC_DESTCLIPY1;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_zoomed_block
 * FUNCTION  : Put a zoomed block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman / Rainer Reber
 * FIRST     : 24.01.95 18:21
 * LAST      : 24.01.95 18:21
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Source_width - Source width of block.
 *             SISHORT Source_height - Source height of block.
 *             SISHORT Target_width - Target width of block.
 *             SISHORT Target_height - Target height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The core drawing function :
 *
 *              Source_pixel = *Graphics_ptr;
 *              if (Source_pixel != 0)
 *              {
 *                 *Target_ptr = Source_pixel;
 *              }
 *
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_zoomed_block(struct OPM *OPM, SISHORT X, SISHORT Y, UNSHORT Source_width,
 UNSHORT Source_height, UNSHORT Target_width, UNSHORT Target_height,
 UNBYTE *Graphics_ptr)
{
	/* Are the input sizes legal ? */
	if (Source_width && Source_height && Target_width && Target_height)
	{
		/* Yes -> Is zooming necessary ? */
		if ((Source_width != Target_width) || (Source_height != Target_height))
		{
			/* Yes -> Set clipping area */
			ZOOMFUNC_DESTCLIPX0 = OPM->clip.left;
			ZOOMFUNC_DESTCLIPY0 = OPM->clip.top;
			ZOOMFUNC_DESTCLIPX1 = OPM->clip.left + OPM->clip.width-1;
			ZOOMFUNC_DESTCLIPY1 = OPM->clip.top + OPM->clip.height-1;

			/* Zoom block */
			_ZOOM_BLOCK(Graphics_ptr, OPM->data, X, Y, Source_width, Source_width,
			 Source_height, OPM->nextypos, Target_width, Target_height,
			 -1, ZOOMMODE_NORMAL);

			/* OPM has changed */
			OPM->status |= OPMSTAT_CHANGED;
		}
		else
		{
			/* No -> Just display it */
			Put_masked_block(OPM, X, Y, Source_width, Source_height, Graphics_ptr);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_zoomed_silhouette
 * FUNCTION  : Put a single-colour silhouette of a zoomed block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman / Rainer Reber
 * FIRST     : 21.03.95 11:54
 * LAST      : 21.03.95 11:54
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Source_width - Source width of block.
 *             SISHORT Source_height - Source height of block.
 *             SISHORT Target_width - Target width of block.
 *             SISHORT Target_height - Target height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNSHORT Colour - Colour index (0...255).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The core drawing function :
 *
 *              Source_pixel = *Graphics_ptr;
 *              if (Source_pixel != 0)
 *              {
 *                 *Target_ptr = Colour;
 *              }
 *
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_zoomed_silhouette(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNSHORT Colour)
{
	/* Are the input sizes legal ? */
	if (Source_width && Source_height && Target_width && Target_height)
	{
		/* Yes -> Is zooming necessary ? */
		if ((Source_width != Target_width) || (Source_height != Target_height))
		{
			/* Yes -> Set clipping area */
			ZOOMFUNC_DESTCLIPX0 = OPM->clip.left;
			ZOOMFUNC_DESTCLIPY0 = OPM->clip.top;
			ZOOMFUNC_DESTCLIPX1 = OPM->clip.left + OPM->clip.width-1;
			ZOOMFUNC_DESTCLIPY1 = OPM->clip.top + OPM->clip.height-1;

			/* Zoom block */
			_ZOOM_BLOCK(Graphics_ptr, OPM->data, X, Y, Source_width, Source_width,
			 Source_height, OPM->nextypos, Target_width, Target_height,
			 Colour, ZOOMMODE_SILHOUETTE);

			/* OPM has changed */
			OPM->status |= OPMSTAT_CHANGED;
		}
		else
		{
			/* No -> Just display it */
			Put_silhouette(OPM, X, Y, Source_width, Source_height, Graphics_ptr,
			 Colour);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_zoomed_recoloured_silhouette
 * FUNCTION  : Put a recoloured silhouette of a zoomed block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman / Rainer Reber
 * FIRST     : 21.03.95 11:55
 * LAST      : 21.03.95 11:55
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Source_width - Source width of block.
 *             SISHORT Source_height - Source height of block.
 *             SISHORT Target_width - Target width of block.
 *             SISHORT Target_height - Target height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNBYTE *Recolouring_table - Pointer to 256-byte recolouring
 *              table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The core drawing function :
 *
 *              Source_pixel = *Graphics_ptr;
 *              if (Source_pixel != 0)
 *              {
 *                 *Target_ptr = Recolouring_table[ *Target_ptr ];
 *              }
 *
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_zoomed_recoloured_silhouette(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNBYTE *Recolouring_table)
{
	/* Are the input sizes legal ? */
	if (Source_width && Source_height && Target_width && Target_height)
	{
		/* Yes -> Is zooming necessary ? */
		if ((Source_width != Target_width) || (Source_height != Target_height))
		{
			/* Yes -> Set clipping area */
			ZOOMFUNC_DESTCLIPX0 = OPM->clip.left;
			ZOOMFUNC_DESTCLIPY0 = OPM->clip.top;
			ZOOMFUNC_DESTCLIPX1 = OPM->clip.left + OPM->clip.width-1;
			ZOOMFUNC_DESTCLIPY1 = OPM->clip.top + OPM->clip.height-1;

			/* Zoom block */
			_ZOOM_BLOCK(Graphics_ptr, OPM->data, X, Y, Source_width, Source_width,
			 Source_height, OPM->nextypos, Target_width, Target_height,
			 (SILONG) Recolouring_table, ZOOMMODE_RECOLOUR_BACK);

			/* OPM has changed */
			OPM->status |= OPMSTAT_CHANGED;
		}
		else
		{
			/* No -> Just display it */
			Put_recoloured_block(OPM, X, Y, Source_width, Source_height,
			 Graphics_ptr, Recolouring_table);
		}
	}

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_zoomed_recoloured_block
 * FUNCTION  : Put a zoomed and recoloured block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman / Rainer Reber
 * FIRST     : 21.03.95 11:55
 * LAST      : 21.03.95 11:55
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Source_width - Source width of block.
 *             SISHORT Source_height - Source height of block.
 *             SISHORT Target_width - Target width of block.
 *             SISHORT Target_height - Target height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNBYTE *Recolouring_table - Pointer to 256-byte recolouring
 *              table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The core drawing function :
 *
 *              Source_pixel = *Graphics_ptr;
 *              if (Source_pixel != 0)
 *              {
 *                 *Target_ptr = Recolouring_table[ Source_pixel ];
 *              }
 *
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_zoomed_recoloured_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNBYTE *Recolouring_table)
{
	/* Are the input sizes legal ? */
	if (Source_width && Source_height && Target_width && Target_height)
	{
		/* Yes -> Set clipping area */
		ZOOMFUNC_DESTCLIPX0 = OPM->clip.left;
		ZOOMFUNC_DESTCLIPY0 = OPM->clip.top;
		ZOOMFUNC_DESTCLIPX1 = OPM->clip.left + OPM->clip.width-1;
		ZOOMFUNC_DESTCLIPY1 = OPM->clip.top + OPM->clip.height-1;

		/* Zoom block */
		_ZOOM_BLOCK(Graphics_ptr, OPM->data, X, Y, Source_width, Source_width,
		 Source_height, OPM->nextypos, Target_width, Target_height,
		 (SILONG) Recolouring_table, ZOOMMODE_RECOLOUR);

		/* OPM has changed */
		OPM->status |= OPMSTAT_CHANGED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_zoomed_transparent_block
 * FUNCTION  : Put a zoomed transparent block in an OPM.
 * FILE      : GFXFUNC.C
 * AUTHOR    : Jurie Horneman / Rainer Reber
 * FIRST     : 21.03.95 11:55
 * LAST      : 21.03.95 11:55
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             SISHORT Source_width - Source width of block.
 *             SISHORT Source_height - Source height of block.
 *             SISHORT Target_width - Target width of block.
 *             SISHORT Target_height - Target height of block.
 *             UNBYTE *Graphics_ptr - Pointer to graphics.
 *             UNBYTE *Recolouring_table - Pointer to 64K recolouring table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The core drawing function :
 *
 *              Source_pixel = *Graphics_ptr;
 *              if (Source_pixel != 0)
 *              {
 *                 *Target_ptr = Recolouring_table[ *Target_ptr ][ Source_pixel ];
 *              }
 *
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_zoomed_transparent_block(struct OPM *OPM, SISHORT X, SISHORT Y,
 UNSHORT Source_width, UNSHORT Source_height, UNSHORT Target_width,
 UNSHORT Target_height, UNBYTE *Graphics_ptr, UNBYTE *Recolouring_table)
{
	/* Are the input sizes legal ? */
	if (Source_width && Source_height && Target_width && Target_height)
	{
		/* Yes -> Set clipping area */
		ZOOMFUNC_DESTCLIPX0 = OPM->clip.left;
		ZOOMFUNC_DESTCLIPY0 = OPM->clip.top;
		ZOOMFUNC_DESTCLIPX1 = OPM->clip.left + OPM->clip.width-1;
		ZOOMFUNC_DESTCLIPY1 = OPM->clip.top + OPM->clip.height-1;

		/* Zoom block */
		_ZOOM_BLOCK(Graphics_ptr, OPM->data, X, Y, Source_width, Source_width,
		 Source_height, OPM->nextypos, Target_width, Target_height,
		 (SILONG) Recolouring_table, ZOOMMODE_TRANSPARENT);

		/* OPM has changed */
		OPM->status |= OPMSTAT_CHANGED;
	}
}

