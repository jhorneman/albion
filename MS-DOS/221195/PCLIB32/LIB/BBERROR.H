#ifndef BBERROR_H
#define BBERROR_H

/************
 * NAME     :  BBERROR.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */
#include <BBDEF.H>


/* Defines */

/* Base defines for error stack */
#define BBERROR_MAXERRORSONSTACK	20	/* Max. number of errors on error stack */
#define BBERROR_ERRORDATASIZE		16	/* Max. number of data bytes per error entry */

/* Base defines for string output buffer */
#define BBERROR_OUTSTRINGSIZE		200	/* Max. size of output string */


/* BBERROR_PushError error */
#define BBERROR_PUSH_STACKFULL		 1	/* Error stack is full. Cannot push */
#define BBERROR_PUSH_TOOMUCHDATA	 2	/* Too much data to put on stack */


/* BBERROR_PrintAllErrors */
#define BBERROR_PAE_CALLPRINTERROR	( 1 <<  0 )	/* Call PrintError functions (?) */
#define	BBERROR_PAE_PRINTLIBNAME	( 1 <<  1 )	/* Print library names */
#define BBERROR_PAE_PRINTCR			( 1 <<  2 ) /* Print cr at end of message */
#define BBERROR_PAE_PRINTBLTOP		( 1 <<  3 ) /* Print border line on top of error message */
#define BBERROR_PAE_PRINTBLBOT		( 1 <<  4 ) /* Print border line on bottom of error message */
#define BBERROR_PAE_CALLOUTPUTFUNC	( 1 <<  5 ) /* Do output with output function (?) */

#define BBERROR_PAE_NORMAL			( BBERROR_PAE_CALLPRINTERROR | BBERROR_PAE_PRINTLIBNAME | BBERROR_PAE_PRINTCR | BBERROR_PAE_PRINTBLTOP | BBERROR_PAE_PRINTBLBOT | BBERROR_PAE_CALLOUTPUTFUNC )



/* Typedefs */

/* Typedef for pointer to PrintError functions */
typedef void ( * BBERROR_PrintPtr_type ) ( UNCHAR *, UNBYTE * );


/* Typedef for output function */
typedef void ( * BBERROR_StringOutPtr_type ) ( UNCHAR * );


/* Structures */
struct BBERROR_STACKENTRY
{
	BBERROR_PrintPtr_type PrintErrorPtr;		/* Pointer to PrintError function */
	UNCHAR 	* ErrorLibStringPtr;				/* Pointer to Library name */
	UNBYTE	ErrorData[ BBERROR_ERRORDATASIZE ];	/* Data area */
};



/* Global variables */
extern UNSHORT BBERROR_StackEntries;



/* Prototypes */

/* Public routines */
extern void ERROR_Init( BBERROR_StringOutPtr_type OutputFuncPtr );
extern void ERROR_ClearStack( void );
extern void ERROR_SetOutputFuncPtr( BBERROR_StringOutPtr_type OutputFuncPtr );

extern SISHORT ERROR_PushError( BBERROR_PrintPtr_type PrintPtr, UNCHAR * LibStringPtr, UNSHORT datasize, UNBYTE * dataptr );
extern void ERROR_PopError(void);

extern BOOLEAN ERROR_StackEmpty(void);

extern void ERROR_PrintAllErrors( UNSHORT flagset );


/* Local routines () */
extern void BBERROR_LocalPrintError( UNCHAR * buffer, UNBYTE * data );


#endif


