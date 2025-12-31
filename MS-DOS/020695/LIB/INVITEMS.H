/************
 * NAME     : INVITEMS.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INVITEMS_H
#define INVITEMS_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_ITEM_FRAMES		(8)

#define BODY_ITEM_SLOT_WIDTH		(18)
#define BODY_ITEM_SLOT_HEIGHT		(18)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern struct Object_class Body_item_list_Class;
extern struct Object_class Backpack_item_slot_Class;

extern struct PUM Inventory_item_PUM;

extern MEM_HANDLE Drag_OPM_handle;
extern struct OPM Drag_OPM;

extern struct Item_packet Drag_packet;

extern UNSHORT Drag_HDOB_nr;
extern struct HDOB Drag_HDOB;

/*
 ** Prototypes *************************************************************
 */

/* Body item list methods */
void Init_Body_item_list_object(struct Object *Object, union Method_parms *P);
void Draw_Body_item_list_object(struct Object *Object, union Method_parms *P);

/* Body item slot methods (replacing normal item slot methods) */
void Draw_Body_item_slot_object(struct Object *Object, union Method_parms *P);
void Update_Body_item_slot_object(struct Object *Object, union Method_parms *P);
void Feedback_Body_item_slot_object(struct Object *Object, union Method_parms *P);
void Highlight_Body_item_slot_object(struct Object *Object, union Method_parms *P);

void Left_Body_item_slot_object(struct Object *Object, union Method_parms *P);
void Drop_Body_item_slot_object(struct Object *Object, union Method_parms *P);

/* Body item slot support functions */
void Drag_body_item(struct Item_slot_object *Item_slot);
void Drop_on_body_item(struct Item_slot_object *Item_slot);
void Erase_body_item_slot(struct Item_slot_object *Item_slot);
void Draw_body_item_slot_item(struct Item_slot_object *Item_slot);

/* Backpack item slot methods (replacing normal item slot methods) */
void Left_Backpack_item_slot_object(struct Object *Object, union Method_parms *P);
void DLeft_Backpack_item_slot_object(struct Object *Object, union Method_parms *P);
void Drop_Backpack_item_slot_object(struct Object *Object, union Method_parms *P);

/* Backpack item slot support functions */
void Drag_backpack_item(struct Item_slot_object *Item_slot, UNSHORT Quantity);
void Drop_on_backpack_item(struct Item_slot_object *Item_slot);

/* Inventory pop-up menu actions */
void Inventory_item_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_inventory_item(UNLONG Data);
void PUM_Learn_spell_inventory_item(UNLONG Data);
void PUM_Examine_inventory_item(UNLONG Data);
void PUM_Drink_inventory_item(UNLONG Data);
void PUM_Activate_spell_inventory_item(UNLONG Data);
void PUM_Read_inventory_item(UNLONG Data);
void PUM_Activate_inventory_item(UNLONG Data);

/* Inventory pop-up menu support functions */
void Drop_items(UNSHORT Item_slot_handle, UNSHORT Quantity);

/* Item drag functions */
void Init_item_drag(void);
void Exit_item_drag(void);
void Build_item_graphics(void);
void Move_drag_HDOB_towards_slot(UNSHORT Target_slot_index);
void Item_drag_abort_handler(void);

/* Item management functions */
void Auto_move_packet_to_backpack(MEM_HANDLE Char_handle, struct Item_packet *Source);
UNSHORT Find_target_slot_for_item(struct Item_data *Source_item);

BOOLEAN Take_item_from_character(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity, struct Item_packet *Target_packet);
BOOLEAN Do_take_item_from_char(struct Event_action *Action);

BOOLEAN Put_item_in_character(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity, struct Item_packet *Source_packet);
BOOLEAN Do_put_item_in_char(struct Event_action *Action);

void Remove_item(MEM_HANDLE Char_handle, UNSHORT Slot_index, UNSHORT Quantity);

void Add_item_to_body(MEM_HANDLE Char_handle, struct Item_packet *Source_packet,
 UNSHORT Item_slot_index);
void Positive_item_boni(struct Character_data *Char, struct Item_data *Item_data);
void Negative_item_boni(struct Character_data *Char, struct Item_data *Item_data);

void Add_item_to_backpack(MEM_HANDLE Char_handle, struct Item_packet *Source_packet,
 UNSHORT Item_slot_index, UNSHORT Quantity);

#endif

