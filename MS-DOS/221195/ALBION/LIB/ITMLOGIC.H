/************
 * NAME     : ITMLOGIC.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 20-6-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef ITEMLOGIC_H
#define ITEMLOGIC_H

#include <BBDEF.H>
#include <BBMEM.H>
#include <GAME.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* Move item event action data
  (needed by CHEST) */
struct Move_item_event_data {
	MEM_HANDLE Handle;
	UNSHORT Slot_index;
	UNSHORT Quantity;
	struct Item_packet *Packet;
};

/* Item descriptor */
struct Item_descriptor {
	MEM_HANDLE Slots_handle;	/* Memory handle of inventory */
	UNLONG Slots_offset;			/* Offset to inventory */
	UNSHORT Item_slot_index;	/* Index of item slot (1...) */
};

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Item_event_set_handle;
extern MEM_HANDLE Item_event_text_handle;

extern MEM_HANDLE Item_graphics_handle;

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_item_data(void);
void Exit_item_data(void);

/* Item management functions */
UNSHORT Find_target_body_slot_for_item(struct Item_data *Source_item);

BOOLEAN Take_item_from_character(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity, struct Item_packet *Target_packet);
BOOLEAN Do_take_item_from_char(struct Event_action *Action);

BOOLEAN Put_item_in_character(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity, struct Item_packet *Source_packet);
BOOLEAN Do_put_item_in_char(struct Event_action *Action);

void Remove_item(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity);

void Add_item_to_body(MEM_HANDLE Char_handle,
 struct Item_packet *Source_packet, UNSHORT Item_slot_index);

void Add_item_to_backpack(MEM_HANDLE Char_handle,
 struct Item_packet *Source_packet, UNSHORT Item_slot_index,
 UNSHORT Quantity);
void Auto_move_packet_to_backpack(MEM_HANDLE Char_handle,
 struct Item_packet *Source_packet);

UNSHORT Count_nr_items_that_will_fit_in_backpack(MEM_HANDLE Char_handle,
 UNSHORT Item_index);

void Curse_item(MEM_HANDLE Char_handle, UNSHORT Slot_index);
void Decurse_item(MEM_HANDLE Char_handle, UNSHORT Slot_index);

/* Inventory functions */
BOOLEAN Inventory_empty(struct Item_packet *Packet, UNSHORT Nr_items);
BOOLEAN Inventory_full(struct Item_packet *Packet, UNSHORT Nr_items);

UNSHORT Search_body_for_item_type(MEM_HANDLE Char_handle, UNSHORT Target_item_type);

void Remove_cursed_items(MEM_HANDLE Char_handle);

BOOLEAN Has_cursed_items(MEM_HANDLE Char_handle);

BOOLEAN Free_item_slot_in_inventory(MEM_HANDLE Char_handle);

BOOLEAN Free_item_slot_in_party(void);

UNSHORT Optimise_inventory(struct Item_packet *Inventory, UNSHORT Nr_items);

/* Item packet & data access functions */
BOOLEAN Packet_empty(struct Item_packet *Packet);

void Clear_packet(struct Item_packet *Packet);

struct Item_data *Get_item_data(struct Item_packet *Packet);
void Free_item_data(void);

void Get_item_name(struct Item_packet *Packet, UNCHAR *Name);

void Build_item_packet(struct Item_packet *Packet, UNSHORT Item_index,
 UNSHORT Quantity);

/* Item evaluators for item selector */
UNSHORT Magical_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);
UNSHORT Vital_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

void Show_moving_item(struct Item_packet *Packet, SISHORT Source_X,
 SISHORT Source_Y, SISHORT Target_X, SISHORT Target_Y);

void Show_moving_gold(SISHORT Source_X, SISHORT Source_Y, SISHORT Target_X,
 SISHORT Target_Y);

void Show_moving_food(SISHORT Source_X, SISHORT Source_Y, SISHORT Target_X,
 SISHORT Target_Y);

#endif

