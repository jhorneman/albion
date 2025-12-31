#ifndef MTHINTRN_H
#define MTHINTRN_H

/************
 * NAME     :  MTHINTRN.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.H>

/* Master defines */
#define BBMTH_ERRORHANDLING

/* defines */

/* Strukturen */

/* Error struct */

#ifdef BBMTH_ERRORHANDLING

	struct BBMTH_ErrorStruct
	{
		UNBYTE *errorname;
		UNLONG	errordata;
		UNLONG	errordata2;
	};

#endif

/********************************************************************/

/* externe Variablen */

/* ErrorPrintFunktion */
#ifdef BBMTH_ERRORHANDLING
	extern void MTH_PrintError( UNCHAR * buffer, UNBYTE * data );
#endif

/* externe Assemblerfunktionen */
extern SILONG _MTH_ASS_SQRT(SILONG val);
#pragma aux _MTH_ASS_SQRT parm caller [EAX] value [EAX] modify [EBX ECX EDX];

extern UNSHORT _MTH_ASS_Random( void );
#pragma aux _MTH_ASS_Random parm caller [] value [AX] modify [EAX EBX EDX];

extern UNLONG _MTH_ASS_VektorLen( SILONG x, SILONG y, SILONG z );
#pragma aux _MTH_ASS_VektorLen parm caller [EAX] [EBX] [ECX] value [EAX] modify [EDX ESI EDI];

extern UNSHORT _MTH_ASS_ArcTangens( SILONG x, SILONG y);
#pragma aux _MTH_ASS_ArcTangens parm caller [EAX] [EBX] value [AX] modify [ECX EDX ESI EDI];

#endif
