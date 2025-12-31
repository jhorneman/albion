/************
 * NAME     : INPUT.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 11-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INPUT_H
#define INPUT_H

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

/* Input function */
void Input_string_in_window(UNSHORT Max_length, UNCHAR *Text_ptr,
 UNCHAR *Buffer_ptr);
void Input_string(UNSHORT X, UNSHORT Y, UNSHORT Width, UNSHORT Max_length,
 UNCHAR *Buffer_ptr);

#endif

