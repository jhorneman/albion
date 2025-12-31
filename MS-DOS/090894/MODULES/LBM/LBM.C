/************
 * NAME     : LBM.c
 * AUTOR    : Maverick, BlueByte
 * START    : 20.07.94
 * PROJECT  : Poject32
 * NOTES    :
 * SEE ALSO :
 * VERSION  : 1.0
  ************/

/* Includes */

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBDSA.H>
#include <BBMISC.H>
#include <BBDOS.H>

#include "LBM.H"



/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_FindChunk
 * FUNCTION  : Search a chunk in LBM file memory.
 * FILE      : LBM.C
 * AUTHOR    : MAVERICK
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             UNCHAR chunk[4]: chunk to look for.
 * RESULT    : UNBYTE * : pointer to chunk or NULL if chunk not found.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
LBM_FindChunk( struct LBM * lbmptr, UNCHAR chunk[4] )
{
	/* locals */

	UNBYTE * wptr;
	SILONG wsize;
	UNLONG os;
	UNSHORT i, count;


	/* set work pointer */

	wptr = lbmptr->lbmptr;

	/* get size of lbm */

	wsize = lbmptr->lbmsize;


	/* search chunk */

	for ( os=0 ; os<wsize ; os++ )
	{

		count = 0;

		for ( i=0 ; i<4 ; i++ )
			if ( ( *( wptr + os +i ) ) == chunk[i] )
				count++;

		if ( count == 4 )
			return( lbmptr->lbmptr + os );
	}


	/* chunk not found */

	return ( NULL );
}

/* #FUNCTION END# */





/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_GetPal
 * FUNCTION  : Copy pal of LBM into BB palette.
 * FILE      : LBM.C
 * AUTHOR    : MAVERICK
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             struct BBPALETTE * palptr: pointer to BBPALETTE structure.
 * RESULT    : None.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
LBM_GetPal( struct LBM * lbmptr, struct BBPALETTE * palptr )
{
	/* locals */

	UNBYTE * lbmpalptr;
	struct BBCOLOR * destcolorptr;
	UNSHORT i;


	/* get pointer to CMAP chunk */

	lbmpalptr = lbmptr->cmapptr + 8L;

	/* get pointer to BBPALETTE color 0 */

	destcolorptr = &palptr->color[0];


	/* set all colors */

	for ( i=0; i<256; i++ )
	{
		/* copy one color */

		destcolorptr->red	= ( *lbmpalptr++ );
		destcolorptr->green	= ( *lbmpalptr++ );
		destcolorptr->blue	= ( *lbmpalptr++ );

		/* inc dest color pointer */

		destcolorptr++;
	}
}

/* #FUNCTION END# */





/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_DisplayLBMinOPM
 * FUNCTION  : Display a LBM into an OPM.
 * FILE      : LBM.C
 * AUTHOR    : MAVERICK
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             struct OPM * opmptr: pointer to OPM structure.
 * RESULT    : None.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
LBM_DisplayLBMinOPM( struct LBM * lbmptr, struct OPM * opmptr )
{
	/* locals */

	UNSHORT	x, y, i, j;
   	UNBYTE 	color;
   	UNBYTE	cbyte;

	UNBYTE * wbodyptr;

	UNSHORT	iffhoehe, iffbreite, iffpacked;

	UNBYTE * bmhdptr;



	/* get pointer to BMHD chunk */

   	bmhdptr = lbmptr->bmhdptr;


	/* get LBM parameter */

	iffbreite = abs( ( ( ( UNSHORT ) *( bmhdptr +  8 ) ) << 8 ) + * ( bmhdptr +  9 ) );
	iffhoehe  = abs( ( ( ( UNSHORT ) *( bmhdptr + 10 ) ) << 8 ) + * ( bmhdptr + 11 ) );
	iffpacked = *( bmhdptr + 18 );



	/* set work body */

	wbodyptr = lbmptr->bodyptr + 8L;





	/* display lbm file */

	if ( iffpacked == 0 )
	{
		/* non packed lbm file */

		for( y = 0 ; y < iffhoehe ; y++ )
		{
			for( x = 0 ; x < iffbreite ; x++ )
			{
				color = *wbodyptr++;

				OPM_SetPixel( opmptr, x, y, color );
			}
		}
	}
	else
	{
		/* packed lbm */

		/* init out pos */

		x = 0;
		y = 0;

		/* depack it */

		do
		{
			/* get todo byte */

			cbyte = *wbodyptr++;


			/* work on byte */

			if( cbyte < 0x80 )
			{
				/* copy bytes */

				/* number of bytes to copy */

				cbyte++;
				j = cbyte;

				/* copy it */

				for ( i=0; i<j; i++ )
				{
					color = *wbodyptr++;
					OPM_SetPixel( opmptr, x, y, color );
					x++;
				}

			}
			else if ( cbyte > 0x80 )
			{
				/* fill with one byte */

				/* number of bytes to fill up */

				cbyte = ~cbyte + 2;
				j = cbyte;

				/* get byte to fill with */

				color = *wbodyptr++;

				/* fill it */

				for ( i=0; i<j; i++ )
				{
					OPM_SetPixel( opmptr, x, y, color );
					x++;
				}
			}


			/* line done ? */

			if ( x == iffbreite )
			{
				/* yes: start next line */

				x = 0;
				y++;
			}

		}
		while ( y < iffhoehe );
	}
}

/* #FUNCTION END# */






/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_DisplayLBM
 * FUNCTION  : Loads and displays a LBM.
 * FILE      : LBM.C
 * AUTHOR    : MAVERICK
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : UNCHAR * lbmname: name of lbm.
 *             struct OPM * opmptr: pointer to OPM to display LBM in.
 *             struct BBPALETTE * palptr: pointer to BBPALETTE structure.
 * RESULT    : BOOLEAN: FALSE: error.
 * BUGS      :
 * NOTES     : A buffer for the LBM is allocated temporary.
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
LBM_DisplayLBM( UNCHAR * lbmname, struct OPM * opmptr, struct BBPALETTE * palptr )
{
	/* local */

	struct LBM mylbm;


	/* get length of LBM file */

	if( ( mylbm.lbmsize = DOS_GetFileLength( lbmname ) ) < 0 )
		return( FALSE );

	/* load LBM file */

	if( ( mylbm.lbmptr = DOS_ReadFile( lbmname, NULL, NULL ) ) == NULL )
		return( FALSE );


	/* search BMHD chunk */

	if( ( mylbm.bmhdptr = LBM_FindChunk( &mylbm, "BMHD" ) ) == NULL )
		return( FALSE );

	/* search BODY chunk */

	if( ( mylbm.bodyptr = LBM_FindChunk( &mylbm, "BODY" ) ) == NULL )
		return( FALSE );

	/* search CMAP chunk */

	if( ( mylbm.cmapptr = LBM_FindChunk( &mylbm, "CMAP" ) ) == NULL )
		return( FALSE );


	/* get palette of LBM */

	LBM_GetPal( &mylbm, palptr );


	/* display LBM in OPM */

	LBM_DisplayLBMinOPM( &mylbm, opmptr );


	/* free memory of LBM */

	BASEMEM_Free( mylbm.lbmptr );


	/* ok */

	return( TRUE );
}

/* #FUNCTION END# */



/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_
 * FUNCTION  :
 * FILE      : LBM.C
 * AUTHOR    : MAVERICK
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

/* #FUNCTION END# */

