/************
 * NAME     : SCRIPT.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 31-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef SCRIPT_H
#define SCRIPT_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

/* Animation play modes */
#define UNDER_2D_OBJECTS_PLAY_MODE			(0)
#define OVER_2D_OBJECTS_PLAY_MODE			(1)
#define NO_2D_OBJECTS_PLAY_MODE				(2)
#define OUTSIDE_MAP_PLAY_MODE					(3)

#define NO_PAL_PLAY_MODE_FLAG					(0x80)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern BOOLEAN In_Script;

/*
 ** Prototypes *************************************************************
 */

void Execute_script(UNSHORT Script_index);

void Play_animation(UNSHORT Flic_nr, SISHORT Playback_X, SISHORT Playback_Y,
 UNSHORT Nr_repetitions, UNSHORT Play_mode);

void Wipe(UNSHORT Wipe_type);

#endif

