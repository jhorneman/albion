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
#include <HDOB.H>

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
UNLONG Init_Body_item_list_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Body_item_list_object(struct Object *Object, union Method_parms *P);

/* Body item slot methods (replacing normal item slot methods) */
UNLONG Draw_Body_item_slot_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Feedback_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Highlight_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);

UNLONG Left_Body_item_slot_object(struct Object *Object, union Method_parms *P);
UNLONG Drop_Body_item_slot_object(struct Object *Object, union Method_parms *P);

/* Body item slot support functions */
void Drag_body_item(struct Item_slot_object *Item_slot);
void Drop_on_body_item(struct Item_slot_object *Item_slot);
void Erase_body_item_slot(struct Item_slot_object *Item_slot);
void Draw_body_item_slot_item(struct Item_slot_object *Item_slot);

/* Backpack item slot methods (replacing normal item slot methods) */
UNLONG Left_Backpack_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG DLeft_Backpack_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Drop_Backpack_item_slot_object(struct Object *Object,
 union Method_parms *P);

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

#endif

