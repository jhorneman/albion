/************
 * NAME     : PLACES.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 16-08-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef PLACES_H
#define PLACES_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* Place types */
#define TRAIN_SKILL_PLACE_TYPE			(0)
#define HEAL_LP_PLACE_TYPE					(1)
#define HEAL_CONDITION_PLACE_TYPE		(2)
#define REMOVE_CURSED_ITEM_PLACE_TYPE	(3)
#define EXAMINE_ITEM_PLACE_TYPE			(4)
#define RECHARGE_ITEM_PLACE_TYPE			(5)
#define RENT_ROOM_PLACE_TYPE				(6)
#define MERCHANT_PLACE_TYPE				(7)
#define BUY_FOOD_PLACE_TYPE				(8)
#define SPELL_MERCHANT_PLACE_TYPE		(9)
#define BUY_TRANSPORT_PLACE_TYPE			(10)
#define LEARN_SPELL_PLACE_TYPE			(11)
#define REPAIR_ITEM_PLACE_TYPE			(12)

#define MAX_PLACE_TYPES						(13)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/* Place parameters */
extern UNSHORT Place_type;
extern UNSHORT Place_selector_text_block_nr;
extern UNSHORT Place_buy_req_text_block_nr;
extern UNSHORT Place_action_text_block_nr;
extern UNSHORT Place_value;
extern UNSHORT Place_price;
extern UNSHORT Place_merchant_data_nr;

extern MEM_HANDLE Place_text_handle;

/* Needed for PRIC text command */
extern UNLONG Place_total_price;

/*
 ** Prototypes *************************************************************
 */

void Do_place_action(void);

#endif

