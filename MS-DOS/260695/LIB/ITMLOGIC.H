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
void Positive_item_boni(struct Character_data *Char,
 struct Item_data *Item_data);
void Negative_item_boni(struct Character_data *Char,
 struct Item_data *Item_data);

void Add_item_to_backpack(MEM_HANDLE Char_handle,
 struct Item_packet *Source_packet, UNSHORT Item_slot_index,
 UNSHORT Quantity);
void Auto_move_packet_to_backpack(MEM_HANDLE Char_handle,
 struct Item_packet *Source);

void Curse_item(MEM_HANDLE Char_handle, UNSHORT Slot_index);
void Decurse_item(MEM_HANDLE Char_handle, UNSHORT Slot_index);

/* Inventory check functions */
BOOLEAN Inventory_empty(struct Item_packet *Packet, UNSHORT Nr_items);
BOOLEAN Inventory_full(struct Item_packet *Packet, UNSHORT Nr_items);

/* Item packet & data access functions */
BOOLEAN Packet_empty(struct Item_packet *Packet);

void Clear_packet(struct Item_packet *Packet);

struct Item_data *Get_item_data(struct Item_packet *Packet);
void Free_item_data(void);

void Get_item_name(struct Item_packet *Packet, UNCHAR *Name);

void Build_item_packet(struct Item_packet *Packet, UNSHORT Item_index,
 UNSHORT Quantity);

#endif

