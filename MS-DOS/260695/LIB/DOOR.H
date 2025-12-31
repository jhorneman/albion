/************
 * NAME     : DOOR.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef DOOR_H
#define DOOR_H

#include <BBDEF.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern struct Module Door_Mod;

extern struct Object_class Lock_display_Class;

/*
 ** Prototypes *************************************************************
 */

void Door_ModInit(void);
void Door_ModExit(void);

void Init_Door_display(void);
void Exit_Door_display(void);

UNLONG Init_Door_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Door_object(struct Object *Object, union Method_parms *P);

UNLONG Init_Lock_display_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Lock_display_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Lock_display_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Lock_display_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Lock_display_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Lock_display_object(struct Object *Object, union Method_parms *P);

void PUM_Pick_lock(UNLONG Data);

#endif

