/*
 ** FLI / FLC stuff ************************************************************
 */

#include "BBDEFxx.H"

#include "CDFLI.H"
#include "FLC.H"
#include "FLI_FAST.H"
#include "FLI_ASM.H"

#include <stdio.h>
#include <dos.h>
#include <mem.h>

#include "BB.h"


/* extern */

extern	short 	CDFLI_EMS_HANDLE;
extern	ubyte far * CDFLI_EMS_FRAMEADDR;
extern	short	CDFLI_EMS_OFFSET;



/* global file vars */

UNSHORT WORKEMSOS;




/* fast next frame */

UNSHORT
fli_fast_next_frame( struct flc_struct * flc_struct )
{

	/* locals */

	UNSHORT	chunks;
	UNLONG	workos;
	UNBYTE far * animworkptr;



	/* init animworkptr */

	workos = flc_struct->workos;
	WORKEMSOS = ( workos >> 14 );


	SETEMS( CDFLI_EMS_HANDLE, 0, ( ( WORKEMSOS + 0 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 1, ( ( WORKEMSOS + 1 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 2, ( ( WORKEMSOS + 2 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 3, ( ( WORKEMSOS + 3 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );


	animworkptr = CDFLI_EMS_FRAMEADDR + ( workos & 0x00003fffL );





	/* is it a frame chunk ? */

	if ( *( ( UNSHORT * ) ( animworkptr + 4L ) ) != 0xf1fa )
		return( FLC_ANIM_ERROR );



	/* get pointer to next frame chunk */

	flc_struct->workos = flc_struct->workos + *( ( UNLONG * ) ( animworkptr ) );



	/* get number of chunks in frame */

	chunks = *( ( UNSHORT * ) ( animworkptr + 6L ) );


	/* pointer to first chunk */

	animworkptr += 16L;


	/* work on chunk of frame */

	if ( chunks )
	{

		/* work on all chunks */

		do
		{

			/* locals */

			UNSHORT typeofchunk;



			/* get type of chunk */

			typeofchunk = *( ( UNSHORT * ) ( animworkptr + 4L ) );



			/* pointer to chunk */

			animworkptr += 6;



			/* work on chunk */

			if( typeofchunk == 12 )
				animworkptr = DOCHUNK12( animworkptr );


			/* dec chunks */

			chunks--;




		}
		while ( chunks );

	}



	/* inc actframe */

	flc_struct->actframe++;

	if ( flc_struct->actframe == flc_struct->frames )
	{
		flc_struct->actframe = 0;

		return( FLC_ANIM_DONE );
	}



	/* return code */

	return( 0 );
}




/* Skip 3 EMS pages */
/* Called fron DOCHUNK12 */

void
SKIP3EMSPAGES( void )
{
	WORKEMSOS += 3;

	SETEMS( CDFLI_EMS_HANDLE, 0, ( ( WORKEMSOS + 0 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 1, ( ( WORKEMSOS + 1 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 2, ( ( WORKEMSOS + 2 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
	SETEMS( CDFLI_EMS_HANDLE, 3, ( ( WORKEMSOS + 3 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
}
