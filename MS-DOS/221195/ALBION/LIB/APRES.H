/************
 * NAME     : APRES.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 07-03-1995
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

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern UNLONG Apres_gold_coins;
extern UNLONG Apres_food_rations;

/*
 ** Prototypes *************************************************************
 */

void Enter_Apres_combat(void);
void Distribute_char_items(MEM_HANDLE Char_handle);

void Clear_combat_conditions(void);

/* Apres pool management functions */
BOOLEAN Init_Apres_pool(void);
void Exit_Apres_pool(void);

void Store_item_in_Apres_pool(struct Item_packet *Source_packet);

void Put_gold_in_Apres_pool(UNSHORT Nr_gold_coins);
void Put_food_in_Apres_pool(UNSHORT Nr_food_rations);

#endif

