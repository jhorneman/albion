/************
 * NAME     : BBERROR.c
 * AUTOR    : Maverick, BlueByte
 * START    : 02.06.94 10:00
 * PROJECT  : Poject32/Macintosh
 * NOTES    :
 * SEE ALSO :
 * VERSION  : 1.0
 ************/


/* Includes */

#include <BBDEF.H>
#include <BBERROR.H>

#include <stdio.h>
#include <string.h>


/* Global variables */


/* Errors on stack */

UNSHORT BBERROR_StackEntries;


/* Stack of Errors */

struct BBERROR_STACKENTRY BBERROR_ErrorStack[ BBERROR_MAXERRORSONSTACK ];



/* Pointer to output function */

BBERROR_StringOutPtr_type BBERROR_OutputFuncPtr;



/* Output string */

UNCHAR BBERROR_MessageString[ BBERROR_OUTSTRINGSIZE ];



/* Name of Library */

char BBERROR_LibraryName[] = "BBERROR Library";

char BBERROR_StackFullMessage[] = "STACK FULL";
char BBERROR_TooMuchDataMessage[] = "TOO MUCH DATA";


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_Init
 * FUNCTION  : Initialize error library.
 * FILE      : BBERROR.C
 * AUTHOR    : MAVERICK
 * FIRST     : 02.06.94 10:00
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

void
ERROR_Init( BBERROR_StringOutPtr_type OutputFuncPtr )
{
	/* Init error stack */

	ERROR_ClearStack();


	/* Set pointer to string output function */

	ERROR_SetOutputFuncPtr( OutputFuncPtr );
}

/* #FUNCTION END# */



/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_ClearStack
 * FUNCTION  : Clear error stack (set number of error messages to 0).
 * FILE      : BBERROR.C
 * AUTHOR    : MAVERICK
 * FIRST     : 02.06.94 10:00
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

void
ERROR_ClearStack( void )
{
	/* Init error stack entry counter */

	BBERROR_StackEntries = 0;
}

/* #FUNCTION END# */



/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_SetOutputFuncPtr
 * FUNCTION  : Set pointer to string output function.
 * FILE      : BBERROR.C
 * AUTHOR    : MAVERICK
 * FIRST     : 02.06.94 10:00
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

void
ERROR_SetOutputFuncPtr( BBERROR_StringOutPtr_type OutputFuncPtr )
{
	/* Set pointer to string output function */

	BBERROR_OutputFuncPtr = OutputFuncPtr;
}

/* #FUNCTION END# */




/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_PushError
 * FUNCTION  : Pushes one error message onto the stack
 * FILE      : BBERROR.C
 * AUTHOR    : MAVERICK
 * FIRST     : 02.06.94 10:00
 * LAST      :
 * INPUTS    : BBERROR_PrintPtr_type PrintErrorPtr : Pointer to PrintError Function.
 *             UNSHORT datasize      : Number of data bytes.
 *             UNBYTE * dataptr      : Pointer to data area.
 * RESULT    : 0                       : o.k.
 *             BBERROR_PUSH_STACKFULL  : stack overflow
 *             BBERROR_PUSH_TOOMUCHDATA: too much data.
 * BUGS      :
 * NOTES     :
 * SEE ALSO  :
 * VERSION   : 1.0
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SISHORT
ERROR_PushError( BBERROR_PrintPtr_type PrintPtr, UNCHAR * LibStringPtr, UNSHORT datasize, UNBYTE * dataptr )
{
	/* Local vars */

	UNSHORT i;
	struct BBERROR_STACKENTRY * stackentryptr;
	UNBYTE * stackdataptr;

	/* Error stack full ? */

	if ( BBERROR_StackEntries == BBERROR_MAXERRORSONSTACK )
	{
		/* Kill latest entry onm stack */

		BBERROR_StackEntries--;

		/* Insert stack full message */

		ERROR_PushError( BBERROR_LocalPrintError, ( UNCHAR * ) &BBERROR_LibraryName[0], strlen( ( char * ) &BBERROR_StackFullMessage[0] ) + 1, ( UNBYTE * ) &BBERROR_StackFullMessage[0] );

		/* Return */

		return( BBERROR_PUSH_STACKFULL );
	}


	/* Too much data ? */

	if ( datasize > BBERROR_ERRORDATASIZE )
	{
		/* Insert stack full message */

		ERROR_PushError( BBERROR_LocalPrintError, ( UNCHAR * ) &BBERROR_LibraryName[0], strlen( ( char * ) &BBERROR_TooMuchDataMessage[0] ) + 1, ( UNBYTE * ) &BBERROR_TooMuchDataMessage[0] );

		/* Return */

		return( BBERROR_PUSH_TOOMUCHDATA );
	}


	/* Get pointer to act error stack entry */

	stackentryptr = &BBERROR_ErrorStack[ BBERROR_StackEntries ];


	/* Set pointer to PrintError function */

	stackentryptr->PrintErrorPtr = PrintPtr;


	/* Set pointer to LibName string */

	stackentryptr->ErrorLibStringPtr = LibStringPtr;


	/* Copy data onto stack */

	stackdataptr = &stackentryptr->ErrorData[0];

	for ( i=0; i<datasize; i++ )
		*stackdataptr++ = *dataptr++;


	/* Inc error stack entry counter */

	BBERROR_StackEntries++;


	/* That«s it */

	return( 0 );
}

/* #FUNCTION END# */


/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_PopError
 * FUNCTION  : Pops one error message from the stack.
 * FILE      : BBERROR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 16:34
 * LAST      : 19.09.95 16:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
ERROR_PopError(void)
{
	/* Is the error stack empty ? */
	if (BBERROR_StackEntries > 0)
	{
		/* No -> Remove the last entry from the stack */
		BBERROR_StackEntries--;
	}
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_StackEmpty
 * FUNCTION  : Checks if the error stack is empty.
 * FILE      : BBERROR.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 20:16
 * LAST      : 22.09.95 20:16
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (stack is empty) or FALSE (stack is not empty).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
ERROR_StackEmpty(void)
{
	/* Is the error stack empty ? */
	if (!BBERROR_StackEntries)
	{
		/* Yes */
		return TRUE;
	}
	else
	{
		/* Yes */
		return FALSE;
	}
}

/* #FUNCTION END# */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_PrintAllErrors
 * FUNCTION  : Call print function for all errors on error stack.
 * FILE      : BBERROR.C
 * AUTHOR    : MAVERICK
 * FIRST     : 02.06.94 10:00
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

void
ERROR_PrintAllErrors( UNSHORT flagset )
{
	/* Local variables */

	struct BBERROR_STACKENTRY * stackentryptr;
	UNSHORT i;
	UNSHORT outputstringlength;



	/* Return if no error on stack */

	if ( ! BBERROR_StackEntries )
		return;



	/* Print borderline on top of message */

	if( ( flagset & BBERROR_PAE_CALLOUTPUTFUNC ) && ( flagset & BBERROR_PAE_PRINTBLTOP ) && ( BBERROR_OutputFuncPtr ) )
	{
		BBERROR_OutputFuncPtr( ( UNCHAR * ) "BBERROR: ERRORSTACK START:--------------\n" );
	}


	/* Init stack entry pointer */

	stackentryptr = &BBERROR_ErrorStack[ 0 ];


	/* Work on all error messages in error stack */

	for( i=0; i<BBERROR_StackEntries; i++, stackentryptr++ )
	{

		/* Init output string */

		BBERROR_MessageString[0] = 0;

		outputstringlength = 0;



		/* Print library name */

		{
			UNCHAR * libnameptr = stackentryptr->ErrorLibStringPtr;

			if( ( flagset & BBERROR_PAE_PRINTLIBNAME ) && ( libnameptr != NULL ) )
			{
				sprintf( ( char * ) &BBERROR_MessageString[outputstringlength], "%s: ", libnameptr );
				outputstringlength = ( UNSHORT ) strlen( ( char * ) &BBERROR_MessageString[0] );
			}
		}


		/* Call Print Error Function */

		{
			BBERROR_PrintPtr_type printerrorptr = stackentryptr->PrintErrorPtr;

			if ( ( flagset & BBERROR_PAE_CALLPRINTERROR ) && ( printerrorptr != NULL ) )
			{
				printerrorptr( &BBERROR_MessageString[outputstringlength], &stackentryptr->ErrorData[0] );
				outputstringlength = ( UNSHORT ) strlen( ( char * ) &BBERROR_MessageString[0] );
			}
		}


		/* Print return */

		if( flagset & BBERROR_PAE_PRINTCR )
		{
			sprintf( ( char * ) &BBERROR_MessageString[outputstringlength], "\n" );
			outputstringlength = ( UNSHORT ) strlen( ( char * ) &BBERROR_MessageString[0] );
		}


		/* Call string output function */

		if( ( flagset & BBERROR_PAE_CALLOUTPUTFUNC ) && ( BBERROR_OutputFuncPtr ) )
		{
			BBERROR_OutputFuncPtr( &BBERROR_MessageString[0] );
		}


	}



	/* Print borderline on bottom of message */

	if( ( flagset & BBERROR_PAE_CALLOUTPUTFUNC ) && ( flagset & BBERROR_PAE_PRINTBLBOT ) && ( BBERROR_OutputFuncPtr ) )
	{
		BBERROR_OutputFuncPtr( ( UNCHAR * ) "BBERROR: ERRORSTACK END-----------------\n" );
	}



	/* Set entry counter to 0 */

	BBERROR_StackEntries = 0;

}


/* #FUNCTION END# */





/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BBERROR_LocalPrintError
 * FUNCTION  :
 * FILE      : BBERROR.C
 * INPUTS    : None.
 * RESULT    : None.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BBERROR_LocalPrintError( UNCHAR * buffer, UNBYTE * data )
{
	sprintf( ( char * ) buffer, "INTERNAL ERROR: %s", ( char * ) data );
}

/* #FUNCTION END# */






/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : ERROR_
 * FUNCTION  :
 * FILE      : BBERROR.C
 * AUTHOR    : MAVERICK
 * FIRST     : 02.06.94 10:00
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


