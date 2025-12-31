#ifndef DOSINTRN_H
#define DOSINTRN_H

/************
 * NAME     :  DOSINTRN.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */
#include <BBDEF.h>


/* Master defines */
#define BBDOS_ERRORHANDLING


/* Defines */

#ifdef BBDOS_ERRORHANDLING
	#define BBDOS_MAXERRORFILES 32
#endif

/* Typedefs */



/* Structures */

/* Error struct */

#ifdef BBDOS_ERRORHANDLING

	struct BBDOS_ErrorStruct
	{
		UNBYTE *errorname;
		UNBYTE *errorfile;
		UNLONG	errordata;
		SISHORT	erroros;
	};

#endif



/* Prototypes */


/* Local functions */


#ifdef BBDOS_ERRORHANDLING
	extern void DOS_PrintError( UNCHAR * buffer, UNBYTE * data );
#endif


#endif
