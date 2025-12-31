/************
 * NAME     : MAIN.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-2-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAIN_H
#define MAIN_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define START_PROGRAM_COMMAND		(0)
#define EXIT_PROGRAM_COMMAND		(1)
#define SHOW_INTRO_COMMAND			(2)
#define SHOW_CREDITS_COMMAND		(3)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern BOOLEAN No_AIL;

extern UNSHORT Main_program_command;

extern struct File_type OPM_ftype;
extern struct File_type ROPM_ftype;

/*
 ** Prototypes *************************************************************
 */

void Critical_error_handler(void);

UNSHORT Parse_argument(UNCHAR *Argument, UNSHORT Nr_commands,
 UNCHAR **Command_list);

UNSHORT Parse_value(UNCHAR *Argument);

#endif
