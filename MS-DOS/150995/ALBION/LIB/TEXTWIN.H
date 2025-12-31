/************
 * NAME     : TEXTWIN.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef TEXTWIN_H
#define TEXTWIN_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

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

/* Text window functions */
void Do_text_file_window(MEM_HANDLE Text_file_handle, UNSHORT Text_block_nr);
void Do_text_file_window_with_symbol(MEM_HANDLE Text_file_handle,
 UNSHORT Text_block_nr, UNSHORT Width, UNSHORT Height,
 MEM_HANDLE Graphics_handle, UNLONG Graphics_offset);

void Do_text_window(UNBYTE *Ptr);
void Do_text_window_with_symbol(UNBYTE *Ptr, UNSHORT Width, UNSHORT Height,
 MEM_HANDLE Graphics_handle, UNLONG Graphics_offset);

#endif

