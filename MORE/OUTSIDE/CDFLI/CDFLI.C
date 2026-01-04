/*
 ******************************************************************************
 CDFLI
 Thomas H„user
 ******************************************************************************
*/


/*
 ** #defines ******************************************************************
 */

// #define DEBUG_PRINTF
// #define DEBUG_COLOR

// #define DEBUG_NOFRAMETEST

/*
 ** extern ********************************************************************
 */

extern	void	SoundManager( void );
extern	short	Stopplay;


/*
 ** #includes *****************************************************************
 */

#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys\stat.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <conio.h>
#include <dir.h>

#include "bb.h"

#include "cdfli.h"

#include "bbdefxx.h"
#include "flc.h"
#include "fli_fast.h"
#include "fli_asm.h"


/*---------------------------------------------------------------------------
vmode : vga 320*200 256 colors
---------------------------------------------------------------------------*/

void INITVMODE13( void )
{
	union REGS regs;

	regs.h.ah = 0x00;
	regs.h.al = 0x13;
	int86( 0x10, &regs, &regs );

	regs.h.ah = 0x05;
	regs.h.al = 0;
	int86( 0x10, &regs, &regs );
}


void EXITVMODE13( void )
{
	union REGS regs;

	regs.h.ah = 0x00;
	regs.h.al = 3;
	int86( 0x10, &regs, &regs );

	regs.h.ah = 0x05;
	regs.h.al = 0;
	int86( 0x10, &regs, &regs );
}



void SETVMODE13COL( UNSHORT c, UNBYTE r, UNBYTE g, UNBYTE b )
{
	outportb ( 0x03c8, c );
	outportb ( 0x03c9, r );
	outportb ( 0x03c9, g );
	outportb ( 0x03c9, b );
}


void SETVMODE13PAL( UNBYTE * palptr )
{
	UNSHORT i;

	for ( i=0 ; i<256 ; i++ )
	{
		SETVMODE13COL( 	i,
						( ( *( palptr + 0 ) ) >> 2 ) & 63,
						( ( *( palptr + 1 ) ) >> 2 ) & 63,
						( ( *( palptr + 2 ) ) >> 2 ) & 63
					 );
		palptr += 3;
	}
}



/*---------------------------------------------------------------------------
play fli
---------------------------------------------------------------------------*/

short		CDFLI_EMS_HANDLE = 0;
ubyte far * CDFLI_EMS_FRAMEADDR = NULL;
short		CDFLI_EMS_OFFSET = 0;


/*
	1: ende( "ERROR: Konnte FLI File L„nge nicht feststellen" );
	2: ende( "ERROR: Konnte FLI File nicht ”ffnen" );
	3: ende( "ERROR: Konnte FLI File nicht lesen" );
	4: ende( "ERROR: Konnte FLI File nicht initialisieren" );
	5: ende( "ERROR: Konnte FLI File nicht laden (PRELOAD)" );
	6: ende( "ERROR: Fehler bei FLI Darstellung" );
	7: ende( "ERROR: Konnte FLI File nicht laden (ONLINE)" );
*/

short
CDFLI_PlayFli( short ems_handle, short ems_offset, ubyte * dosmemptr, char * fliname, unsigned short framestoplay, short setmode13 )
{

	/* local */

	B_HANDLE flihandle = 0;
	ulong	flisize = 0L;
	ulong	fliloaded = 0L;
	ulong	fliworked = 0L;
	ulong	flitoload;
	ubyte	flipal[ 256 * 3 ];
	struct flc_struct flistruc;




	/* set ems */

	CDFLI_EMS_HANDLE	= ems_handle;

	CDFLI_EMS_FRAMEADDR = NORMPTR( EMS_PORTP->FRAMEADDR );

	CDFLI_EMS_OFFSET	= ems_offset;



	/* open fli */

	if ( ( flisize = GETLENGTH( fliname ) ) == -1 )
		return( 1 );

	flitoload = flisize;

	if ( ( flihandle = OPENFILE( ACCESS_READ, fliname ) ) == 0 )
		return( 2 );


	/* load first 128 bytes of fli (fli header) */

	SETEMS( CDFLI_EMS_HANDLE, 0, 0 + CDFLI_EMS_OFFSET );

	if ( GETFILE( flihandle, CDFLI_EMS_FRAMEADDR, 128L ) )
	{
		CLOSEFILE( flihandle );
		return( 3 );
	}

	flitoload -= 128L;



	/* init fli sctructure */

	flistruc.animptr = CDFLI_EMS_FRAMEADDR;
	flistruc.palptr	= &flipal[0];



	/* init anim */

	if ( ! flc_init_anim( &flistruc ) )
	{
		CLOSEFILE( flihandle );
		return( 4 );
	}



	/* preload fli into ems buffer */

	{
		/* local */

		UNLONG	topreload;


		/* number of bytes to preload */

		topreload = min( flitoload, FLI_PRELOAD );


		/* preload in 64k blocks */

		do
		{
			/* local */

			UNSHORT loadedemsos;
			UNLONG bytestoload;

			/* get ems page id */

			loadedemsos = fliloaded >> 14;

			/* set ems pages */

			SETEMS( CDFLI_EMS_HANDLE, 0, ( ( loadedemsos + 0 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
			SETEMS( CDFLI_EMS_HANDLE, 1, ( ( loadedemsos + 1 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
			SETEMS( CDFLI_EMS_HANDLE, 2, ( ( loadedemsos + 2 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );
			SETEMS( CDFLI_EMS_HANDLE, 3, ( ( loadedemsos + 3 ) & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );

			/* number of bytes to load */

			bytestoload = min( topreload, 0x10000L );

			/* load block */

			if( GETFILE( flihandle, CDFLI_EMS_FRAMEADDR, bytestoload ) )
			{
				CLOSEFILE( flihandle );
				return( 5 );
			}

			/* setup vars */

			fliloaded += bytestoload;
			flitoload -= bytestoload;
			topreload -= bytestoload;

		}
		while ( topreload );
	}




	/* set video mode */

	if ( setmode13 )
		INITVMODE13();

	CLEARVGA();



	/* display fli */



	/* display first frame (use standart next frame) */

	{
		UNSHORT ret;

		ret = flc_next_frame( &flistruc );

		fliworked = flistruc.workos;

		if( ret & FLC_ANIM_ERROR )
		{
			CLOSEFILE( flihandle );
			return( 6 );
		}
	}




	/* all other frames */

	{
		UNSHORT endeflag = 0;


		do
		{



			/* next fli frame if buffer loaded */

			if (	(		( ! flitoload )
						||	( ( fliloaded - fliworked ) > 0x10000L )
					)

				#ifndef DEBUG_NOFRAMETEST

				&&
					( SCRPORTP->FRAMES >= framestoplay )

				#endif

				)
			{

				UNSHORT ret;

				#ifdef DEBUG_COLOR
				   	SETVMODE13COL( 0, 0, 0, 30 );
				#endif

				ret = fli_fast_next_frame( &flistruc );

				#ifdef DEBUG_COLOR
					SETVMODE13COL( 0, 0, 0, 0 );
				#endif

				SCRPORTP->FRAMES = 0;

				fliworked = flistruc.workos;

				if( ret & FLC_ANIM_ERROR )
				{
					CLOSEFILE( flihandle );
					return( 6 );
				}

				if( ret )
					endeflag = 1;
			}



			/* debug */

			#ifdef DEBUG_PRINTF

				printf( "\n" );
				printf( "FLI FRAME:  %d\n", flistruc.actframe );
				printf( "FLI TOLOAD: %ld\n", flitoload );
				printf( "FLI LOADED: %ld\n", fliloaded );
				printf( "FLI WORKED: %ld\n", fliworked );
				printf( "FLI DIFFER: %ld\n", fliloaded - fliworked );
				printf( "FLI TOEND:  %ld\n", ( FLI_MEM ) - ( fliloaded - fliworked ) );

			#endif




			/* load fli into buffer */

			#define FLI_LOAD_LINESIZE		0x4000L

			#define FLI_LOAD_LINEEMSAND		( 0x4000L - FLI_LOAD_LINESIZE )



			/* anything to load ? */

			if ( flitoload )
			{

				/* yes ! */


				/* load until time is over */

				do
				{

					/* buffer full ? */

					if ( ( fliloaded - fliworked ) < ( FLI_MEM - 0x4000L ) )
					{

						/* no: load into buffer */

						/* local */

						UNSHORT loadedemsos;
						UNBYTE far * loadaddr;
						UNLONG  bytestoload;


						/* calc ems page to load into */

						loadedemsos = fliloaded >> 14;

						/* set ems page */

						SETEMS( CDFLI_EMS_HANDLE, loadedemsos & 0, ( loadedemsos & FLI_EMS_PAGES_AND ) + CDFLI_EMS_OFFSET );


						/* calc load address in ems frame */

						loadaddr = CDFLI_EMS_FRAMEADDR + ( fliloaded & FLI_LOAD_LINEEMSAND );



						/* prrintf debug */

						#ifdef DEBUG_PRINTF

							printf( "load into ems page: %d\n", loadedemsos );

						#endif



						/* calc bytes to load */

						bytestoload = min( flitoload, FLI_LOAD_LINESIZE );



						/* color debug */

						#ifdef DEBUG_COLOR
							SETVMODE13COL( 0, 0, 30, 0 );
						#endif


						/* load */

						if( GETFILE( flihandle, loadaddr, bytestoload ) )
						{
							CLOSEFILE( flihandle );
							return( 7 );
						}

						#ifdef DEBUG_COLOR
							SETVMODE13COL( 0, 0, 0, 0 );
						#endif

						fliloaded += bytestoload;
						flitoload -= bytestoload;


						#ifdef DEBUG_PRINTF

							printf( "LOADED: %ld BYTES\n", bytestoload );

						#endif

					}

				}

				#ifndef DEBUG_NOFRAMETEST

					while ( SCRPORTP->FRAMES < ( framestoplay / 2 ) );

				#else

					while ( 0 );

				#endif
			}



			if ( KP_PORTP->KP_NEW )
			{
				if ( KP_PORTP->KP_ASC == 27 )
					Stopplay = TRUE;

				KP_PORTP->KP_NEW = 0;

				endeflag = 1;
			}



			SoundManager();


		}
		while ( ! endeflag );
	}


	/* close fli file */

	CLOSEFILE( flihandle );



	/* rolling home */

	return( 0 );
}
