/************
 * NAME     : 2D_COLL.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COLLISION_2D_H
#define COLLISION_2D_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* 2D movement data */
struct Movement_2D {
	UNSHORT Flags;					/* See below */
	UNSHORT X, Y;					/* Changed */
	UNSHORT Direction;			/* Input */
	SISHORT dX, dY;				/* Output */
	UNSHORT View_direction;		/* Changed */
	UNSHORT Slide_direction;	/* Internal */
	UNSHORT Reverse_slide;		/* Internal */
	UNSHORT Travel_mode;			/* Input */
	UNSHORT NPC_index;			/* Input */
	UNSHORT State;					/* Changed */
};

/* Bitmasks for Movement_2D.Flags */
#define UPDATE_AFTER_MOVE		(1 << 0)		/* Output */
#define DONT_SUCK					(1 << 1)    /* Internal */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* 2D movement and collision detection */
void Make_2D_move(UNSHORT Direction);

BOOLEAN Try_2D_move(struct Movement_2D *Move);

#endif

