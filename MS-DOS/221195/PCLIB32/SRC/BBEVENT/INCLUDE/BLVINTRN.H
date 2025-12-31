#ifndef BLEVINTRN_H
#define BLEVINTRN_H

/************
 * NAME     :  BBBLEV_I.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */
#include "BBDEF.h"


/* Master defines */
#define BBBLEV_ERRORHANDLING


/* Defines */

#ifdef BBBLEV_ERRORHANDLING
	#define BBBLEV_MAXERRORFILES 64
#endif

/* Typedefs */


/* Structures */

/* Error struct */

#ifdef BBBLEV_ERRORHANDLING

	struct BBBLEV_ErrorStruct
	{
		UNBYTE *errorname;
		UNLONG	errordata;
	};

#endif


/* Prototypes */

/* Local functions */


#ifdef BBBLEV_ERRORHANDLING
	extern void BLEV_PrintError( UNCHAR * buffer, UNBYTE * data );
#endif


#endif

