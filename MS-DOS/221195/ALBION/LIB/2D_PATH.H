/************
 * NAME     : 2D_PATH.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef PATH_2D_H
#define PATH_2D_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define NR_PARTY_ANIMFRAMES	(12)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Hide_party;

extern UNLONG Party_object_X, Party_object_Y;

extern UNSHORT Active_member_state;

extern struct Object_2D Party_objects[6];

extern UNSHORT Party_animation[NR_PARTY_ANIMFRAMES];

extern UNSHORT _2D_steps_left;
extern UNSHORT _2D_speed;
extern UNSHORT _2D_scale;

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_2D_party_path(void);
void Exit_2D_party_path(void);

void Reset_2D_path(void);

void Add_member_to_2D_trail(UNSHORT Member_nr);
void Remove_member_from_2D_trail(UNSHORT Member_nr);

void Change_active_member_2D_path(UNSHORT New_active_member_index);

void Update_2D_party_position(void);

void Store_2D_movement(SISHORT dX, SISHORT dY);
void Store_2D_current_position(void);

void Make_2D_path_entry(UNSHORT X, UNSHORT Y);

void Display_2D_party(void);

#endif

