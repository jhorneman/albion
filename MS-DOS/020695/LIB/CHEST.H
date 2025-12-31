/************
 * NAME     : CHEST.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef CHEST_H
#define CHEST_H

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

extern struct Module Chest_Mod;

/*
 ** Prototypes *************************************************************
 */

void Chest_ModInit(void);
void Chest_ModExit(void);

void Init_Chest_display(void);
void Exit_Chest_display(void);

#endif

