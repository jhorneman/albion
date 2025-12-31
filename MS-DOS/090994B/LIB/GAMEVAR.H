/************
 * NAME     : GAMEVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GAMEVAR_H
#define GAMEVAR_H

#include <BBDEF.H>

/*
 ** External global variables **********************************************
 */

extern struct OPM Main_OPM, Panel_OPM;
extern struct SCREENPORT Screen;
extern struct BBPALETTE Palette;

extern MEM_HANDLE Slab_handle;

extern BOOLEAN Quit_program;
extern BOOLEAN Cheat_mode;

extern struct Party_data PARTY_DATA;

extern UNSHORT Screen_width, Screen_height, Panel_Y;

#endif
