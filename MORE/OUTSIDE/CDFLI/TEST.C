/*
 ******************************************************************************
 TEST CDFLI
 Thomas H„user
 ******************************************************************************
*/


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

/*
 ** Prototypes ****************************************************************
 */


/*
 ** globals *******************************************************************
 */

short		emshandle = 0;
ubyte far * emsframeaddr = NULL;

uint	inited = 0;

short	Stopplay = 0;


/*
 ** ende **********************************************************************
 */

void
ende( char * message )
{
	if ( inited & 1 )
		EXITVMODE13();

	if ( inited & 2 )
		EXIT();

	if ( emshandle )
		FREEMS( emshandle );

	MEMBACK();

	printf( "%s\n\n", message );

	exit( 0 );
}


/*
 ** main program **************************************************************
 */

void
main ( int argc, char *argv[] )
{

	/* arguments */

	if ( ( argc == 2 ) && ( *( argv[1] ) == '?' ) )
	{
		printf("\nCDFLI V0.0   by T.H„user   (c) 1994 Blue Byte.\n\n");
		printf("DISPLAY AN FLI FILE.\n\n");
		printf("USAGE : CDFLI FLINAME.\n\n");
		printf("PARAMETER :\n\n");
		printf("FLINAME  : name of fli.\n");
		printf("\n\n");
		exit(0);
	}



	/* init EMS */

	if ( INITEMS() < 1 )
		ende( "ERROR: Kein EMS !!!" );

	if ( ( emshandle = GETEMS( FLI_EMS_PAGES ) ) < 0 )
		ende( "ERROR: Nicht genug EMS vorhanden !!!" );




	/* Init irq */

	INIT();
	inited |= 2;




	/* init display */

	INITVMODE13();
	inited |= 1;





	/* play fli */

	{
		short i;

		for ( i=1; i<argc; i++ )
		{
			short ret;

			ret = CDFLI_PlayFli( emshandle, 0, NULL, argv[i], 6, ! ( inited & 1 ) );

			if ( ret )
			{
				EXITVMODE13();
				inited &= ~1;

				printf( "Player returned %d.\n", ret );

				ende( "Player error !!!" );
			}
		}
	}


	/* wait on key */

	#ifdef WAITONKEY

		do
		{
		} while ( ! KP_PORTP->KP_NEW );
		KP_PORTP->KP_NEW = 0;

	#endif




	/* exit */

	ende( "Good bye !!!" );

}




void
SoundManager( void )
{
}

