#ifndef BBOPM_H
#define BBOPM_H

/************
 * NAME     :  BBOPM.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */

#include <BBDEF.H>
#include <BBMEM.H>

/* Defines */

/* OPM.status flag set */

#define OPMSTAT_NULL		0L				/* NULL value */
#define OPMSTAT_SELFMEMORY	( 1L << 0L )	/* OPM_New() allocated memory */
#define OPMSTAT_CHANGED		( 1L << 1L )	/* OPM changed since last copy procedure */
#define OPMSTAT_INIT		( 1L << 2L )	/* OPM structure is initialized */
#define OPMSTAT_VIRTUAL		( 1L << 3L )	/* This is a virtual OPM */
#define OPMSTAT_TRANSPARENT	( 1L << 4L )	/* TransparentFarbe bei copyopmopm verwenden */
#define OPMSTAT_USERDEFINED		( 1L << 16L )	/* user defined */

/* Direct calls to assembler functions */
#define OPM_CopyBlock _OPM_ASS_Blit
#define OPM_CopyBlockTrans _OPM_ASS_Blit_Trans

/* Typedefs */

/* Structures */

/* OPM: offscreen pixel map */
/* V1.0: Depth must be 1 !!! */

struct OPM
{
	UNLONG status;				/* Status flag set */

	UNSHORT width;				/* Width of OPM in pixel */
	UNSHORT height;				/* Height of OPM in pixel */
	UNSHORT depth;				/* Number of bytes per pixel (always 1 in OPM1.0) */

	UNBYTE * data;				/* Pointer to pixel area (a pixel is defined as a byte in OPM1.0) */
	UNLONG datasize;			/* Gr”sse der Pixelmap in Bytes */
	UNSHORT nextxpos;			/* Offset to next x pos (always 1 in OPM1.0) */
	UNSHORT nextypos;			/* Offset to next y pos (always width in OPM1.0) */

	SISHORT xoffset;				/* x QuellOffset im OPM / wird bei Clipping von OPM_CreateVirtualopm verwendet */
	SISHORT yoffset;				/* y QuellOffset im OPM / wird bei Clipping von OPM_CreateVirtualopm verwendet */
	struct BBRECT clip;			/* Clipping rectangle */

	UNSHORT	transcol;	/* Transparenzfarbe fr copyopmopm */

	SISHORT virtualx;				/* x Position im physikalischen Bildschirm */
	SISHORT virtualy;				/* y Position im physikalischen Bildschirm */
	struct OPM * virtualsrc;	/* If OPMSTAT_VIRTUAL set: pointer to source OPM */
};

/* Prototypes */

extern SISHORT OPM_New( UNSHORT width, UNSHORT height, UNSHORT depth, struct OPM * opmptr, UNBYTE * data );
extern void OPM_Del( struct OPM * opmptr );
extern void OPM_SetVirtualPosition( struct OPM * opmptr, SISHORT x, SISHORT y );
extern void OPM_CreateVirtualOPM( struct OPM * srcopmptr, struct OPM * virtualopmptr, SISHORT left, SISHORT top, UNSHORT width, UNSHORT height );

extern void OPM_SetPixel( struct OPM * opmptr, SISHORT x, SISHORT y, UNBYTE color );

extern void OPM_HorLine( struct OPM * opmptr, SISHORT x, SISHORT y, UNSHORT width, UNBYTE color );
extern void OPM_VerLine( struct OPM * opmptr, SISHORT x, SISHORT y, UNSHORT height, UNBYTE color );
extern void OPM_Box( struct OPM * opmptr, SISHORT x, SISHORT y, UNSHORT width, UNSHORT height, UNBYTE color );
extern void OPM_FillBox( struct OPM * opmptr, SISHORT x, SISHORT y, UNSHORT width, UNSHORT height, UNBYTE color );

extern UNBYTE OPM_GetPixel( struct OPM * opmptr, SISHORT x, SISHORT y );
extern void OPM_CopyOPMOPM( struct OPM * srcopmptr, struct OPM * destopmptr, SISHORT srcx, SISHORT srcy, UNSHORT srcwidth, UNSHORT srcheight, SISHORT destx, SISHORT desty );
extern void OPM_CopyGFXOPM( struct OPM * opmptr, UNBYTE *gfxptr, SISHORT x, SISHORT y, UNBYTE coloroffset );
extern void OPM_Write(struct OPM *opm,char *string,SILONG x,SILONG y,UNBYTE color);
extern void OPM_printf(struct OPM *opm, SISHORT x, SISHORT y, UNBYTE color,UNCHAR *Format, ...);
extern void OPM_ChangeColor(struct OPM *opmptr, UNBYTE *coltable);
extern void OPM_Zoom(struct OPM *srcopm,struct OPM *dstopm,UNLONG destxpos,UNLONG destypos,UNLONG destwidth,UNLONG destheight);

/* externe Assemblerfunktionen */
extern void _OPM_ASS_Blit( UNBYTE *src,UNBYTE *dst,SILONG srcm,SILONG dstm,UNLONG width,UNLONG heigth);
#pragma aux _OPM_ASS_Blit parm caller [ESI] [EDI] [EAX] [ECX] [EBX] [EDX];

/* externe Assemblerfunktionen */
extern void _OPM_ASS_Blit_Trans( UNBYTE *src,UNBYTE *dst,SILONG srcm,SILONG dstm,UNLONG width,UNLONG heigth,UNLONG transcol);
#pragma aux _OPM_ASS_Blit_Trans parm caller [ESI] [EDI] [EDX] [ECX] [EBX] [EAX] [];

#endif
