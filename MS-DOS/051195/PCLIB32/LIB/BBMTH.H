#ifndef BBMTH_H
#define BBMTH_H

/************
 * NAME     :  BBMTH.H
 * AUTOR    :  BlueByte
 * PROJECT  :  Project 32
 ************/


/* Includes */

#include <BBDEF.H>

/* Defines */

extern SILONG MTH_sqrt( SILONG val );
extern void MTH_initrnd( SILONG seed );
extern UNSHORT MTH_rnd( void);
extern UNLONG MTH_veklen( SILONG x,SILONG y,SILONG z );
extern SISHORT MTH_sin(UNSHORT winkel);
extern SISHORT MTH_cos(UNSHORT winkel);
extern UNSHORT MTH_atan(SILONG x,SILONG y);

#endif
