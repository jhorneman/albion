/************
 * NAME     : APRES.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 07-03-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef APRES_H
#define APRES_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_APRES_ITEMS	((6 + MONSTERS_PER_GROUP) * (ITEMS_ON_BODY + ITEMS_PER_CHAR))

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_apres_pool(void);

void Put_item_in_apres_pool(struct Item_packet *Source_packet);

void Put_gold_in_apres_pool(UNSHORT Nr_gold_coins);
void Put_food_in_apres_pool(UNSHORT Nr_food_rations);

#endif

