/************
 * NAME     : BOOLREQ.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef BOOLREQ_H
#define BOOLREQ_H

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

/* Boolean requester functions */
BOOLEAN Boolean_requester(UNCHAR *Text);
BOOLEAN Boolean_requester_with_buttons(UNCHAR *Text, UNCHAR *Yes_button_text,
 UNCHAR *No_button_text);

#endif

