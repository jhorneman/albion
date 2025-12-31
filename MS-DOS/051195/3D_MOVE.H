/************
 * NAME     : 3D_MOVE.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 3-10-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MOVE_3D_H
#define MOVE_3D_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define FULLTURN_STEPS			(16)

/*
 ** Structure definitions **************************************************
 */

/* 3D movement data */
struct Movement_3D {
	SILONG X, Y;					/* Changed */
	SILONG dX, dY;					/* Changed */
	UNSHORT Travel_mode;			/* Input */
	UNSHORT NPC_index;			/* Input */
};

/*
 ** External global variables **********************************************
 */

extern struct Module Move_mouse_M3_Mod;
extern struct Module Move_key_M3_Mod;

extern UNSHORT Mice_3D[];

/*
 ** Prototypes *************************************************************
 */

void Initialize_3D_position(void);

UNSHORT Get_3D_mouse_state(SISHORT X, SISHORT Y);
UNSHORT Get_3D_mouse_state2(SISHORT X, SISHORT Y);

void Rotate_3D(SISHORT dAlpha);
void Add_3D_vector(SILONG dX, SILONG dY);

/* 3D movement and collision detection */
BOOLEAN Make_3D_move(struct Movement_3D *Move);

BOOLEAN Movement_check_3D(SISHORT Map_X, SISHORT Map_Y, UNSHORT NPC_index,
 UNSHORT Travel_mode);

/* Coordinate conversion functions */
void Map_to_3D(SISHORT Map_X, SISHORT Map_Y, UNLONG *X_3D_ptr,
 UNLONG *Y_3D_ptr);
void _3D_to_map(UNLONG X_3D, UNLONG Y_3D, SISHORT *Map_X_ptr,
 SISHORT *Map_Y_ptr);

BOOLEAN Check_if_line_in_3D_map_is_blocked(UNSHORT Source_X, UNSHORT Source_Y,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT NPC_index, UNSHORT Travel_mode);

#endif

