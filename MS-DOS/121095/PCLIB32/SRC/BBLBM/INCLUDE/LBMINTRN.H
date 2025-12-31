#ifndef LBMINTRN_H
#define LBMINTRN_H

/************
 * NAME     :  LBMINTRN.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.H>

/* Master defines */
#define BBLBM_ERRORHANDLING

/* Error struct */
#ifdef BBLBM_ERRORHANDLING

	struct BBLBM_ErrorStruct
	{
		UNBYTE *errorname;
		UNLONG	errordata;
	};

#endif

/********************************************************************/

/* ErrorPrintFunktion */
#ifdef BBLBM_ERRORHANDLING
	extern void LBM_PrintError( UNCHAR * buffer, UNBYTE * data );
#endif

#endif
