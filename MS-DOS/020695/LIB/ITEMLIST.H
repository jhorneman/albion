/************
 * NAME     : ITEMLIST.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : ITEMLIST.C
 ************/

#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

#define BETWEEN						(5)

#define ITEM_SLOT_INNER_WIDTH		(16)
#define ITEM_SLOT_INNER_HEIGHT	(20)

#define ITEM_SLOT_BETWEEN_X		(1)
#define ITEM_SLOT_BETWEEN_Y		(1)

#define ITEM_SLOT_OUTER_WIDTH		(ITEM_SLOT_INNER_WIDTH + ITEM_SLOT_BETWEEN_X)
#define ITEM_SLOT_OUTER_HEIGHT	(ITEM_SLOT_INNER_HEIGHT + ITEM_SLOT_BETWEEN_Y)

#define QUANTITY_Y_OFFSET			(12)

/* Inventory types */
#define NO_CHAR_INV_TYPE			(0)
#define CHAR_BODY_INV_TYPE			(1)
#define CHAR_BACKPACK_INV_TYPE	(2)
#define CHAR_COMBINED_INV_TYPE	(3)

/*
 ** Structure definitions **************************************************
 */

/* Item descriptor */
struct Item_descriptor {
	MEM_HANDLE Slots_handle;	/* Memory handle of inventory */
	UNLONG Slots_offset;			/* Offset to inventory */
	UNSHORT Item_slot_index;	/* Index of item slot (1...) */
};

/* Item list OID */
struct Item_list_OID {
	UNSHORT Type;
	UNSHORT Nr_items;
	UNSHORT Slots_width, Slots_height;
	MEM_HANDLE Slots_handle;
	UNLONG Slots_offset;
	struct PUM *Menu;
	struct Object_class *Item_slot_class_ptr;
};

/* Item list object */
struct Item_list_object {
	struct Object Object;
	UNSHORT Type;					/* Inventory type */
	UNSHORT Nr_items;				/* Number of item slots in inventory */
	UNSHORT Slots_width;			/* Number of slots per row */
	UNSHORT Slots_height;		/* Number of rows on screen */
	MEM_HANDLE Slots_handle;	/* Memory handle of inventory */
	UNLONG Slots_offset;			/* Offset to inventory */
	struct PUM *Menu;				/* Pointer to a pop-up menu data structure */
	struct Object_class *Item_slot_class_ptr;
	UNSHORT Scroll_bar_object;	/* Scroll-bar object (if any) */
};

/* Item slot OID */
struct Item_slot_OID {
	UNSHORT Number;
};

/* Item slot object */
struct Item_slot_object {
	struct Object Object;
	UNSHORT Number;
	UNSHORT Frame;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Item_list_Class;
extern struct Object_class Item_slot_Class;

/*
 ** Prototypes *************************************************************
 */

/* Item list methods */
void Init_Item_list_object(struct Object *Object, union Method_parms *P);
void Draw_Item_list_object(struct Object *Object, union Method_parms *P);
void Update_item_list(struct Scroll_bar_object *Scroll_bar);

/* Item slot methods */
void Init_Item_slot_object(struct Object *Object, union Method_parms *P);
void Draw_Item_slot_object(struct Object *Object, union Method_parms *P);
void Update_Item_slot_object(struct Object *Object, union Method_parms *P);
void Feedback_Item_slot_object(struct Object *Object, union Method_parms *P);
void Highlight_Item_slot_object(struct Object *Object, union Method_parms *P);
void Pop_up_Item_slot_object(struct Object *Object, union Method_parms *P);
void Help_Item_slot_object(struct Object *Object, union Method_parms *P);
void Right_Item_slot_object(struct Object *Object, union Method_parms *P);
void Touch_Item_slot_object(struct Object *Object, union Method_parms *P);

/* Item support functions */
void Extract_slot_packet(struct Item_slot_object *Item_slot, struct Item_packet
 *Destination);
struct Item_packet *Claim_slot_packet(struct Item_slot_object *Item_slot);
void Free_slot_packet(struct Item_slot_object *Item_slot);
struct Item_slot_object *Find_item_slot_object(UNSHORT Item_list_object,
 UNSHORT Number);

/* Item display support functions */
void Draw_item_slot_item(struct Item_slot_object *Item_slot);
void Draw_item(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Item_packet *Packet, UNSHORT Frame);
void Draw_body_item(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Item_packet *Packet, UNSHORT Frame);

#endif

