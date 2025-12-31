/************
 * NAME     : ALBION.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 23-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef ALBION_H
#define ALBION_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define BLACK		(192)
#define WHITE		(194)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern struct OPM Main_OPM;
extern struct SCREENPORT Screen;
extern struct BBPALETTE Palette;

extern BOOLEAN Quit_program;

extern UNSHORT Screen_width, Screen_height;

/*
 ** Prototypes *************************************************************
 */

void BBMAIN(UNSHORT argc, UNCHAR **argv);
void Exit_program(void);

#endif
