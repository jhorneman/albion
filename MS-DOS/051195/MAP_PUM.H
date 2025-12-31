/************
 * NAME     : MAP_PUM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP_PUM_H
#define MAP_PUM_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

/* Map selection states (mainly for 3D) */
#define NO_MAP_SELECTION	  					(0)
#define A_MAP_SELECTION							(1)
#define OUT_OF_RANGE_MAP_SELECTION			(2)
#define OUT_OF_SPEAK_RANGE_MAP_SELECTION	(3)
#define OUT_OF_TOUCH_RANGE_MAP_SELECTION	(4)
#define BLOCKED_MAP_SELECTION					(5)

/*
 ** Structure definitions **************************************************
 */

/* Map selection data */
struct Map_selection_data {
	UNSHORT State;
	UNSHORT Map_X, Map_Y;
	UNSHORT NPC_index;
	UNSHORT Trigger_modes;
	UNLONG Distance;
};

/*
 ** External global variables **********************************************
 */

extern struct Map_selection_data Current_map_selection_data;

extern struct PUM Map_PUM;

/*
 ** Prototypes *************************************************************
 */

#endif

