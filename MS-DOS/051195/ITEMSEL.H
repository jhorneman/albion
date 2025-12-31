/************
 * NAME     : ITEMSEL.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef ITEMSEL_H
#define ITEMSEL_H

#include <BBDEF.H>
#include <GAME.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

typedef UNSHORT (*Item_evaluator)(struct Character_data *Char,
 struct Item_packet *Packet);

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Item select function */
UNSHORT Select_party_member_item(UNSHORT Member_index, UNCHAR *Text,
 Item_evaluator Evaluator);

#endif

