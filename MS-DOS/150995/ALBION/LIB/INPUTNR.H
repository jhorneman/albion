/************
 * NAME     : INPUTNR.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INPUTNR_H
#define INPUTNR_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
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

/* Number input functions */
SILONG Input_number(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text);

SILONG Input_number_with_symbol(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text, UNSHORT Width, UNSHORT Height, MEM_HANDLE Graphics_handle,
 UNLONG Graphics_offset);

#endif

