/************
 * NAME     : MAIN.C
 * AUTOR    : VIPER, BlueByte
 * START    : 26.05.94 08:00
 * PROJECT  : Project32 test
 * NOTES    : Initializes PCLIB32 libraries
 * SEE ALSO :
 * VERSION  :
 ************/

#include <stdio.h>

/* Includes */

#include <BBDEF.H>
#include <BBBASMEM.h>
#include <BBDOS.h>
#include <BBDSA.H>
#include <BBERROR.h>
#include <BBEVENT.h>
#include <BBOPM.h>
#include <BBSYSTEM.h>

/* Prototypes */

void MainErrorStringOutputFunction( UNCHAR * buffer );
void main(int argc, char** argv);

UNLONG main_init( void );
void main_exit( UNLONG inited );

/* Defines for inited states */
#define MAIN_BASEMEM_INITED		( 1L << 1 )
#define MAIN_DOS_INITED		( 1L << 2 )
#define MAIN_DSA_INITED		( 1L << 3 )
#define MAIN_ERROR_INITED		( 1L << 4 )
#define MAIN_EVENT_INITED		( 1L << 5 )
#define MAIN_SYSTEM_INITED		( 1L << 7 )
#define MAIN_READY_INITED	( 1L << 31 )


/* Error output function */

void
MainErrorStringOutputFunction( UNCHAR * buffer )
{
	printf( "%s", ( char * ) buffer );
}

/* Main */

void
main(int argc, char** argv)
{
	/* Local vars */
	UNLONG inited;

	/* Init system */
	inited = main_init();


	/* Init done correctly ? */
	if( inited & MAIN_READY_INITED )
	{
		/* Start BBMAIN */
		BBMAIN(argc,argv);
	}

	/* Exit system */
	main_exit( inited );

	/* Show errors */
	ERROR_PrintAllErrors( BBERROR_PAE_NORMAL );
}

/* Init all we need */
UNLONG
main_init( void )
{
	/* Local variables */
	UNLONG inited;

	/* Nothing inited */
	inited = 0L;

	/* Init error handling */
	ERROR_Init( &MainErrorStringOutputFunction );

	/* Init BASEMEM system */
	if( BASEMEM_Init() )
		inited |= MAIN_BASEMEM_INITED;
	else
		return( inited );

	/* Init DOS system */
	if( DOS_Init() )
		inited |= MAIN_DOS_INITED;
	else
		return( inited );

	/* Init DSA system */
	if( DSA_Init() )
		inited |= MAIN_DSA_INITED;
	else
		return( inited );

	/* Init EVENT system */
	if( BLEV_Init() )
		inited |= MAIN_EVENT_INITED;
	else
		return( inited );

	/* Init SYSTEM system */
	if( SYSTEM_Init() )
		inited |= MAIN_SYSTEM_INITED;
	else
		return( inited );

	/* All done correctly */
	inited |= MAIN_READY_INITED;

	/* Return state of system */
	return( inited );
}




/* Exit all we inited */

void
main_exit( UNLONG inited )
{
	if( inited & MAIN_SYSTEM_INITED )
		SYSTEM_Exit();

	if( inited & MAIN_EVENT_INITED )
		BLEV_Exit();

	if( inited & MAIN_DSA_INITED )
		DSA_Exit();

	if( inited & MAIN_DOS_INITED )
		DOS_Exit();

	if( inited & MAIN_BASEMEM_INITED )
		BASEMEM_Exit();
}

