/************
 * NAME     : MAINMENU.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 22-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAINMENU_H
#define MAINMENU_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define MAIN_MENU_PROGRAM_START	(0)
#define MAIN_MENU_MAP				(1)
#define MAIN_MENU_COMBAT			(2)
#define MAIN_MENU_GAME_OVER		(3)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void Enter_Main_menu(UNSHORT Mode);

#endif

