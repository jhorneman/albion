#ifndef OPMINTRN_H
#define OPMINTRN_H

/************
 * NAME     :  OPMINTRN.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/

/* Includes */

#include <BBDEF.H>

/* Master defines */
#define BBOPM_ERRORHANDLING


/* Error struct */

#ifdef BBOPM_ERRORHANDLING

	struct BBOPM_ErrorStruct
	{
		UNBYTE *errorname;
		UNLONG	errordata;
		UNLONG	errordata2;
	};

#endif

/* externe Variablen */

extern UNBYTE internchar[];

/* ErrorPrintFunktion */

#ifdef BBOPM_ERRORHANDLING
	extern void OPM_PrintError( UNCHAR * buffer, UNBYTE * data );
#endif

/* Direct calls to assembler functions */
#define OPM_CopyBlock _OPM_ASS_Blit
#define OPM_CopyBlockTrans _OPM_ASS_Blit_Trans

/* externe Assemblerfunktionen */
extern void _OPM_ASS_FillBlock(UNBYTE *dest,SILONG mod,UNLONG width,UNLONG height,UNLONG color);
#pragma aux _OPM_ASS_FillBlock parm caller [EDI] [EDX] [ESI] [EBX] [EAX] modify [ECX]; // Put values directly in registers

/* externe Assemblerfunktionen */
extern void _OPM_ASS_VerLine(UNBYTE *dest,SILONG mod,UNLONG height,UNLONG color);
#pragma aux _OPM_ASS_VerLine parm caller [EDI] [EDX] [EBX] [EAX] modify [ECX ESI]; // Put values directly in registers

/* externe Assemblerfunktionen */
extern void _OPM_ASS_HorLine(UNBYTE *dest,SILONG mod,UNLONG width,UNLONG color);
#pragma aux _OPM_ASS_HorLine parm caller [EDI] [EDX] [EBX] [EAX] modify [ECX ESI]; // Put values directly in registers

/* externe Assemblerfunktionen */
extern void _OPM_ASS_ChangeCol(UNBYTE *dest,UNBYTE *tab,SILONG mod,UNLONG width,UNLONG height);
#pragma aux _OPM_ASS_ChangeCol parm caller [EDI] [ESI] [EBX] [ECX] [EDX] modify [EAX]; // Put values directly in registers

/* externe Assemblerfunktionen */
extern void _OPM_ASS_Zoom(UNBYTE *src,UNBYTE *dest,UNLONG srcnextypos,UNLONG srcwidth,UNLONG srcheight,UNLONG dstnextypos,UNLONG dstwidth,UNLONG dstheight,SILONG destx,SILONG desty);
#pragma aux _OPM_ASS_Zoom parm caller [] modify [EAX EBX ECX EDX ESI EDI]; // Put values directly in registers

#endif
