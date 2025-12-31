#ifndef BBLBM_H
#define BBLBM_H

/************
 * NAME     :  BBLBM.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */

#include <BBDEF.H>


/* Defines */
#define LBMSTAT_USEOPM		0L				/* in Åbergebenes OPM direkt einschreiben */
#define LBMSTAT_MAKENEWOPM	( 1L << 0L )	/* WORD PRO Pixel OPM von der Grîsse des Bildes anlegen */
#define LBMSTAT_WRITESIZEOPM	( 1L << 1L )	/* Die Breite und Hîhe direkt in OPM schreiben
																					 vorhandenen OPMSpeicherPtr Åbernehmen */
#define LBMSTAT_MAKENEWOPM24	( 1L << 2L )	/* 3 Byte Pro Pixel OPM von der Grîsse des Bildes anlegen */
#define LBMSTAT_MAKENEWOPM32	( 1L << 3L )	/* Langwort Pro Pixel (r,g,b,alpha) OPM von der Grîsse des Bildes anlegen */
#define LBMSTAT_GETFROMMEM ( 1L << 16L ) /* Statt eines Zeigers auf einen Namen
																					 wird ein Zeiger auf einen Speicherbereich
																					 Åbergeben und daraus gelesen */
/* Prototypes */
extern BOOLEAN LBM_DisplayLBM( UNBYTE * lbmname, struct OPM * opmptr, struct BBPALETTE * palptr, UNLONG flag);
extern BOOLEAN LBM_ReadTGA( UNBYTE * tganame, struct OPM * opmptr, UNLONG flag);
extern BOOLEAN LBM_WriteTGA( UNBYTE * tganame, struct OPM * opmptr, UNLONG flag);

#endif

