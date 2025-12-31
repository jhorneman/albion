/************
 * NAME     : MEMBRSEL.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MEMBRSEL_H
#define MEMBRSEL_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

typedef UNSHORT (*Member_evaluator)(MEM_HANDLE Char_handle);

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Party member select function */
UNSHORT Select_party_member(UNCHAR *Text, Member_evaluator Evaluator);

#endif

