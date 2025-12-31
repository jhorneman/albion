/************
 * NAME     : SAVELOAD.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 4-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef SAVELOAD_H
#define SAVELOAD_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Global variables *******************************************************
 */

extern UNCHAR Save_game_fname[20];

extern UNCHAR Current_save_dir[];
extern UNCHAR Initial_save_dir[];
extern UNCHAR *Save_dir_ptr;

/*
 ** Prototypes *************************************************************
 */

void Load_initial_game_state(void);

BOOLEAN Load_game_state(UNSHORT Number, BOOLEAN No_load_flag);

void Save_game_state(UNSHORT Number, UNCHAR *Name);

#endif

