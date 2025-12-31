#ifndef LBM_H
#define LBM_H


/************
 * NAME     :  LBM.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBDSA.H>



/* Defines */



/* Structures */

struct LBM
{
	UNBYTE * lbmptr;	/* pointer to buffer of LBM file */
	SILONG lbmsize;		/* size of LBM file */

	UNBYTE * bmhdptr;	/* pointer to BMHD chunk */
	UNBYTE * bodyptr;	/* pointer to BODY chunk */
	UNBYTE * cmapptr;	/* pointer to CMAP chunk */
};



/* Prototypes */

extern BOOLEAN LBM_DisplayLBM( UNCHAR * lbmname, struct OPM * opmptr, struct BBPALETTE * palptr );



#endif

