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
#include <ITMLOGIC.H>

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

extern struct Item_descriptor Selected_item;

extern struct Object_class Body_item_list_Class;
extern struct Object_class Backpack_item_slot_Class;

extern struct PUM Inventory_item_PUM;

extern MEM_HANDLE Drag_OPM_handle;
extern struct OPM Drag_OPM;

extern struct Item_packet Drag_packet;
extern UNSHORT Drag_member;

extern UNSHORT Drag_HDOB_nr;
extern struct HDOB Drag_HDOB;

/*
 ** Prototypes *************************************************************
 */

UNLONG Inquire_drop_item_slot_object(struct Object *Object,
 union Method_parms *P);

/* Inventory pop-up menu support functions */
void Drop_items(UNSHORT Item_slot_handle, UNSHORT Quantity);
void Show_dropping_items(struct Item_slot_object *Item_slot,
 struct Item_packet *Packet, UNSHORT Quantity);

/* Item drag functions */
void Build_item_graphics(void);

void Init_drag(UNSHORT Width, UNSHORT Height, UNSHORT Max_frames);
void Exit_drag(void);

void Move_drag_HDOB_towards_char_slot(UNSHORT Target_slot_index);
void Move_drag_HDOB_towards_object(struct Object *Target);

void Body_backpack_item_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr);

void Destroy_used_item(void);

#endif

