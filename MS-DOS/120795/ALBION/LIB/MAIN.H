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

#define MAIN_BASEMEM_INITED	(1L << 1)
#define MAIN_DOS_INITED			(1L << 2)
#define MAIN_DSA_INITED			(1L << 3)
//#define MAIN_ERROR_INITED		(1L << 4)
#define MAIN_EVENT_INITED		(1L << 5)
#define MAIN_MEM_INITED			(1L << 6)
#define MAIN_SYSTEM_INITED		(1L << 7)
#define MAIN_READY_INITED		(1L << 15)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

void main(int argc, const char** argv);

void Evaluate_program_arguments(UNSHORT argc, UNCHAR **argv);

UNLONG Main_init (void);
void Main_exit (UNLONG Init_state);

void Main_error_output_function(UNCHAR * buffer);

void *AIL_Alloc(size_t Size);
void AIL_Free(void *Ptr);

BOOLEAN Init_AIL_system(void);
void Exit_AIL_system(void);

void cdecl My_system_timer(void);

UNSHORT Parse_argument(UNCHAR *Argument, UNSHORT Nr_commands,
 UNCHAR **Command_list);
UNSHORT Parse_value(UNCHAR *Argument);

#endif
