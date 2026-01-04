/************
 * NAME     : MERCHANT.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 07-03-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MERCHANT_H
#define MERCHANT_H

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

extern BOOLEAN In_Merchant;

extern UNSHORT Merchant_item_list_object;

/*
 ** Prototypes *************************************************************
 */

void Enter_Merchant(void);

void Auto_move_packet_to_merchant_data(struct Item_packet *Source_packet);

UNSHORT Count_nr_items_that_will_fit_in_merchant_data(UNSHORT Item_index);

BOOLEAN Merchant_sell_requester(UNSHORT Quantity, UNSHORT Item_price);

#endif

