/************
 * NAME     : BBLBM.C
 * AUTOR    : R.Reber, BlueByte
 * START    : 20.07.94
 * PROJECT  : Poject32
 * NOTES    :
 * SEE ALSO :
 * VERSION  : 1.0
 ************/

/* Includes */
#include <stdlib.h>

#ifdef BYTEPIXEL
#define ANZBYTEPROPIXEL 1
#endif

#ifdef WORDPIXEL
#define ANZBYTEPROPIXEL 2
#endif

#ifdef WORDPIXEL
	#include <PCLIBHI.h>
#endif

#ifdef BYTEPIXEL
	#include <PCLIB32.h>
#endif

#ifdef WORDPIXEL
/* Palette aus Bild */
UNSHORT hicolors[256];
#endif

/* Structures */

struct LBM
{
	UNBYTE * lbmptr;	/* pointer to buffer of LBM file */
	SILONG lbmsize;		/* size of LBM file */

	UNBYTE * bmhdptr;	/* pointer to BMHD chunk */
	UNBYTE * bodyptr;	/* pointer to BODY chunk */
	UNBYTE * cmapptr;	/* pointer to CMAP chunk */
};

/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : *LBM_FindChunk
 * FUNCTION :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             UNCHAR chunk[4]: chunk to look for.
 * RESULT    : UNBYTE * : pointer to chunk or NULL if chunk not found.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *LBM_FindChunk( struct LBM * lbmptr, UNCHAR chunk[4] )
{
	/* locals */

	UNBYTE * wptr;
	SILONG wsize;
	SILONG os;
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
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : LBM_GetPal
 * FUNCTION :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             struct BBPALETTE * palptr: pointer to BBPALETTE structure.
 * RESULT    : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void LBM_GetPal( struct LBM * lbmptr, struct BBPALETTE * palptr )
{
	/* locals */
	UNBYTE * lbmpalptr;
#ifdef BYTEPIXEL
	struct BBCOLOR * destcolorptr;
#endif
	UNSHORT i;
#ifdef WORDPIXEL
	UNSHORT red,green,blue;
#endif

	/* get pointer to CMAP chunk */
	lbmpalptr = lbmptr->cmapptr + 8L;

#ifdef BYTEPIXEL
	/* get pointer to BBPALETTE color 0 */
	destcolorptr = &palptr->color[0];

	/* set all colors */
	for ( i=0; i<256; i++ ){
		/* copy one color */
		destcolorptr->red	= ( *lbmpalptr++ );
		destcolorptr->green	= ( *lbmpalptr++ );
		destcolorptr->blue	= ( *lbmpalptr++ );

		/* inc dest color pointer */
		destcolorptr++;
	}

	/* Eintr„ge setzen */
	palptr->entries=256;
#endif

#ifdef WORDPIXEL
	/* set all colors */
	for ( i=0; i<256; i++ ){
		/* copy one color */
		red	= (UNSHORT)( *lbmpalptr++ );
		green	= (UNSHORT)( *lbmpalptr++ );
		blue	= (UNSHORT)( *lbmpalptr++ );

		/* true to hicolor */
		hicolors[i]=HCOL(red,green,blue);
	}
#endif
}

/* #FUNCTION END# */



/*
 ******************************************************************************
 * #SMALL FUNCTION HEADER BEGIN#
 * NAME     : LBM_DisplayLBMinOPM
 * FUNCTION :
 * INPUTS    : struct LBM * lbmptr: pointer to LBM structure.
 *             struct OPM * opmptr: pointer to OPM structure.
 * RESULT   : None.
 * #SMALL FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void LBM_DisplayLBMinOPM( struct LBM * lbmptr, struct OPM * opmptr )
{

#ifdef BYTEPIXEL
#define LBM_COLORDEF color
#endif

#ifdef WORDPIXEL
#define LBM_COLORDEF hicolors[color]
#endif

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

				OPM_SetPixel( opmptr, x, y, LBM_COLORDEF );
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
					OPM_SetPixel( opmptr, x, y, LBM_COLORDEF );
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
					OPM_SetPixel( opmptr, x, y, LBM_COLORDEF );
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
 * FILE      : BBLBM.C
 * AUTHOR    : R.Reber
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : UNBYTE * lbmname: name of lbm.
 *             struct OPM * opmptr: pointer to OPM to display LBM in.
 *             struct BBPALETTE * palptr: pointer to BBPALETTE structure.
 *																				NULL dont read Palette
 *						 flag = How to Use Given OPM (look in BBLBM.H)
 * RESULT    : TRUE = OK ,FALSE = Error
 * BUGS      :
 * NOTES     : A buffer for the LBM is allocated temporary.
 *			LBMSTAT_USEOPM		0L				in bergebenes OPM direkt einschreiben
 * 			LBMSTAT_MAKENEWOPM	( 1L << 0L )	OPM von der Gr”sse des Bildes anlegen
 *			LBMSTAT_WRITESIZEOPM	( 1L << 1L )	Die Breite und H”he direkt in OPM schreiben
 *																				 vorhandenen OPMSpeicherPtr bernehmen
 *																				OPM muss mit OPM_New initialisiert sein
 *			LBMSTAT_GETFROMMEM ( 1L << 16L )  Statt eines Zeigers auf einen Namen
 *																			 wird ein Zeiger auf einen Speicherbereich
 *																				 bergeben und daraus gelesen
 *					   Achtung !!!!
 *						 Diese Routine kann nur LBMS laden die eine durch 2 teilbare
 *						 Breite besitzen also 320x200 160x60 jedoch nicht
 *						 321x20 11x12 ...
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
LBM_DisplayLBM( UNBYTE * lbmname, struct OPM * opmptr, struct BBPALETTE * palptr,UNLONG flag)
{
	/* local */
	struct LBM mylbm;
	UNBYTE *bmhdptr;
	UNSHORT iffbreite;
	UNSHORT iffhoehe;

	/* LBM aus Speicher lesen */
	if(flag&LBMSTAT_GETFROMMEM){
		mylbm.lbmsize = 100000000;
		mylbm.lbmptr = lbmname;
	}
	/* LBM von Disk lesen */
	else{
		/* get length of LBM file */
		if( ( mylbm.lbmsize = DOS_GetFileLength( lbmname ) ) < 0 )
			return( FALSE );
		/* load LBM file */
		if( ( mylbm.lbmptr = DOS_ReadFile( lbmname, NULL, NULL ) ) == NULL )
			return( FALSE );
	}

	/* search BMHD chunk */
	if( ( mylbm.bmhdptr = LBM_FindChunk( &mylbm, "BMHD" ) ) == NULL )
		return( FALSE );

	/* search BODY chunk */
	if( ( mylbm.bodyptr = LBM_FindChunk( &mylbm, "BODY" ) ) == NULL )
		return( FALSE );

	/* search CMAP chunk */
	if( ( mylbm.cmapptr = LBM_FindChunk( &mylbm, "CMAP" ) ) == NULL )
		return( FALSE );

#ifdef WORDPIXEL
	/* get palette of LBM always in Hicolor if not NULL*/
	LBM_GetPal( &mylbm, palptr );
#endif

#ifdef BYTEPIXEL
	/* get palette of LBM if not NULL*/
	if(palptr!=NULL){
		LBM_GetPal( &mylbm, palptr );
	}
#endif

	bmhdptr = mylbm.bmhdptr;
	iffbreite = abs( ( ( ( UNSHORT ) *( bmhdptr +  8 ) ) << 8 ) + * ( bmhdptr +  9 ) );
	iffhoehe  = abs( ( ( ( UNSHORT ) *( bmhdptr + 10 ) ) << 8 ) + * ( bmhdptr + 11 ) );

	/* OPM entsprechend der Gr”sse des LBM's erstellen */
	if(flag&LBMSTAT_MAKENEWOPM){
		/* Create OPM */
		if ( !OPM_New( iffbreite, iffhoehe, ANZBYTEPROPIXEL, opmptr, NULL ) ){
			return(FALSE);
		}
	}
	/* Gr”sse des Bildes in OPM eintragen (fr Speicherptr hochz„hlen) */
	else if(flag&LBMSTAT_WRITESIZEOPM){
		opmptr->width = iffbreite;
		opmptr->height  = iffhoehe;
		opmptr->nextypos = opmptr->width;
		opmptr->datasize= opmptr->width*opmptr->height*opmptr->depth;
#ifdef WORDPIXEL
		opmptr->clipwidth=opmptr->width;
		opmptr->clipheight=opmptr->height;
#endif
#ifdef BYTEPIXEL
		opmptr->clip.left=0;
		opmptr->clip.top=0;
		opmptr->clip.width=opmptr->width;
		opmptr->clip.height=opmptr->height;
#endif
	}

	/* display LBM in OPM */
	LBM_DisplayLBMinOPM( &mylbm, opmptr );

	/* Wenn Object aus Speicher gelesen nicht freigeben */
	if(!(flag&LBMSTAT_GETFROMMEM)){
		/* free memory of LBM */
		BASEMEM_Free( mylbm.lbmptr );
	}

	/* ok */
	return( TRUE );
}

/* #FUNCTION END# */


#ifdef BYTEPIXEL
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LBM_DisplayTGA
 * FUNCTION  : None.
 * FILE      : BBLBM.C
 * AUTHOR    : R.Reber
 * FIRST     : 20.07.94
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      :
 * NOTES     : Funktion not implented
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */


BOOLEAN LBM_DisplayTGA( UNBYTE * tganame, struct OPM * opmptr, UNLONG flag)
{
	return(FALSE);
}

/* #FUNCTION END# */
#endif

