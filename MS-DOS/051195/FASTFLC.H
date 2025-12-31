/************
 * NAME     : FASTFLC.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 12-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef FASTFLC_H
#define FASTFLC_H

#include <BBDEF.H>
#include <FLC.H>

/*
 ** Structure definitions **************************************************
 */

/*
 ** Prototypes *************************************************************
 */

extern BOOLEAN FASTFLC_Start_flic_playback(struct Flic_playback *Flic_IO);
extern void FASTFLC_Restart_flic(void);
extern BOOLEAN FASTFLC_Stop_flic_playback(void);
extern BOOLEAN FASTFLC_Playback_flic_frame(void);

#endif

