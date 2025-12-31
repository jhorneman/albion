/************
 * NAME     : ITEMLIST.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : ITEMLIST.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <ITEMLIST.H>
#include <INVITEMS.H>
#include <ITMLOGIC.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <SCROLBAR.H>
#include <POPUP.H>

/* defines */

/* structure definitions */

/* prototypes */

/* Item list methods */
UNLONG Init_Item_list_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Item_list_object(struct Object *Object, union Method_parms *P);

void Update_item_list(struct Scroll_bar_object *Scroll_bar);

/* global variables */

/* Item list method list */
static struct Method Item_list_methods[] = {
	{ INIT_METHOD, Init_Item_list_object },
	{ DRAW_METHOD, Draw_Item_list_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

/* Item list class description */
struct Object_class Item_list_Class = {
	0, sizeof(struct Item_list_object),
	&Item_list_methods[0]
};

/* Item slot method list */
static struct Method Item_slot_methods[] = {
	{ INIT_METHOD, Init_Item_slot_object },
	{ DRAW_METHOD, Draw_Item_slot_object },
	{ UPDATE_METHOD, Update_Item_slot_object },
	{ FEEDBACK_METHOD, Feedback_Item_slot_object },
	{ HIGHLIGHT_METHOD, Highlight_Item_slot_object },
	{ POP_UP_METHOD, Pop_up_Item_slot_object },
	{ HELP_METHOD, Help_Item_slot_object },
	{ RIGHT_METHOD, Right_Item_slot_object },
	{ TOUCHED_METHOD, Touch_Item_slot_object },
	{ 0, NULL}
};

/* Item slot class description */
struct Object_class Item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Item_slot_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Item_list_object
 * FUNCTION  : Initialize method of Item list object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 15:35
 * LAST      : 04.01.95 15:35
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Item_list_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_object *Item_list;
	struct Item_list_OID *OID;
	struct Scroll_bar_OID Scroll_bar_OID;
	struct Item_slot_OID Item_slot_OID;
	UNSHORT i, j;

	Item_list = (struct Item_list_object *) Object;
	OID = (struct Item_list_OID *) P;

	/* Copy data from OID */
	Item_list->Nr_items = OID->Nr_items;
	Item_list->Type = OID->Type;
	Item_list->Slots_width = OID->Slots_width;
	Item_list->Slots_height = OID->Slots_height;
	Item_list->Slots_handle = OID->Slots_handle;
	Item_list->Slots_offset = OID->Slots_offset;
	Item_list->Menu = OID->Menu;
	Item_list->Item_slot_class_ptr = OID->Item_slot_class_ptr;

	/* Scroll bar needed ? */
	if ((Item_list->Slots_width * Item_list->Slots_height) < Item_list->Nr_items)
	{
		/* Yes -> Calculate width and height WITH scroll bar */
		Change_object_size(Object->Self, (Item_list->Slots_width *
		 ITEM_SLOT_OUTER_WIDTH) + BETWEEN + SCROLL_BAR_WIDTH -
		 ITEM_SLOT_BETWEEN_X + 2, (Item_list->Slots_height *
		 ITEM_SLOT_OUTER_HEIGHT) + 2);

		/* Make scroll bar */
		Scroll_bar_OID.Total_units = Item_list->Nr_items;
		Scroll_bar_OID.Units_width = Item_list->Slots_width;
		Scroll_bar_OID.Units_height = Item_list->Slots_height;
		Scroll_bar_OID.Update = Update_item_list;

		/* Add object */
		Item_list->Scroll_bar_object = Add_object(Object->Self,
		 &Scroll_bar_Class, (UNBYTE *) &Scroll_bar_OID, Object->Rect.width
		 - BETWEEN - 2, 0, SCROLL_BAR_WIDTH, Object->Rect.height - 1);
	}
	else
	{
		/* No -> Calculate width and height WITHOUT scroll bar */
		/* (previously, BETWEEN was added in the X-position calculation) */
		Change_object_size(Object->Self, (Item_list->Slots_width *
		 ITEM_SLOT_OUTER_WIDTH) - ITEM_SLOT_BETWEEN_X + 2,
		 (Item_list->Slots_height * ITEM_SLOT_OUTER_HEIGHT) + 2);

		/* Indicate there is no scroll bar */
		Item_list->Scroll_bar_object = 0;
	}

	/* Make item slots */
	Item_slot_OID.Number = 1;
	for (i=0;i<Item_list->Slots_height;i++)
	{
		for (j=0;j<Item_list->Slots_width;j++)
		{
			/* Add object */
			Add_object(Object->Self, Item_list->Item_slot_class_ptr,
			 (UNBYTE *) &Item_slot_OID, j * ITEM_SLOT_OUTER_WIDTH + 1,
			 i * ITEM_SLOT_OUTER_HEIGHT + 1, ITEM_SLOT_INNER_WIDTH,
			 ITEM_SLOT_INNER_HEIGHT);

			/* Count up */
			Item_slot_OID.Number++;
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Item_list_object
 * FUNCTION  : Draw method of Item list object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 15:56
 * LAST      : 04.01.95 15:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Item_list_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_object *Item_list;
	UNSHORT W;

	Item_list = (struct Item_list_object *) Object;

	/* Calculate width of item area */
	W = (Item_list->Slots_width * ITEM_SLOT_OUTER_WIDTH)
	 - ITEM_SLOT_BETWEEN_X + 3;

	/* Clear item list */
	Restore_background(Object->Rect.left, Object->Rect.top, W, Object->Rect.height);

	/* Draw surrounding box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, W, Object->Rect.height);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_item_list
 * FUNCTION  : Update the item list (scroll bar function).
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.01.95 18:44
 * LAST      : 07.01.95 18:44
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_item_list(struct Scroll_bar_object *Scroll_bar)
{
	struct Object *Parent;
	UNSHORT Child;

	/* Get parent object data */
	Parent = Get_object_data(Scroll_bar->Object.Parent);

	/* Draw all child objects except scroll bar */
	Child = Parent->Child;
	while (Child)
	{
		/* Is scroll bar ? */
		if (Child != Scroll_bar->Object.Self)
		{
			/* No -> Draw */
			Execute_method(Child, DRAW_METHOD, NULL);
		}

		/* Next brother */
		Child = (Get_object_data(Child))->Next;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Item_slot_object
 * FUNCTION  : Initialize method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 16:09
 * LAST      : 04.01.95 16:09
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_slot_OID *OID;

	Item_slot = (struct Item_slot_object *) Object;
	OID = (struct Item_slot_OID *) P;

	/* Copy data from OID */
	Item_slot->Number = OID->Number;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Item_slot_object
 * FUNCTION  : Draw method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 16:10
 * LAST      : 04.01.95 16:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;

	Item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Restore_background(Object->Rect.left, Object->Rect.top, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Draw dark box */
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT, &(Recolour_tables[3][0]));

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Item_slot_object
 * FUNCTION  : Update method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.02.95 15:28
 * LAST      : 28.02.95 15:28
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Update_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item_data;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(&Packet);

		/* Is this an animated item ? */
		if (Item_data->Nr_frames > 1)
		{
			/* Yes ->  Reached last frame ? */
			if (Item_slot->Frame == Item_data->Nr_frames - 1)
			{
				/* Yes -> Back to the first frame */
				Item_slot->Frame = 0;
			}
			else
			{
				/* No -> Go forth one frame */
			   Item_slot->Frame++;
			}

			/* Is this the feedback object ? */
			if (Feedback_object == Object->Self)
			{
				/* Yes -> Re-draw item with feedback */
				Feedback_Item_slot_object(Object, P);
			}
			else
			{
				/* No -> Is this the highlighted object ? */
				if (Highlighted_object == Object->Self)
				{
					/* Yes -> Re-draw item, highlighted */
					Highlight_Item_slot_object(Object, P);
				}
				else
				{
					/* No -> Re-draw item */
					Draw_Item_slot_object(Object, P);
				}
			}
		}
		Free_item_data();
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Item_slot_object
 * FUNCTION  : Feedback method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 16:43
 * LAST      : 04.01.95 16:43
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;

	Item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Restore_background(Object->Rect.left, Object->Rect.top, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Draw dark box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Item_slot_object
 * FUNCTION  : Highlight method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.01.95 16:44
 * LAST      : 04.01.95 16:44
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;

	Item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Restore_background(Object->Rect.left, Object->Rect.top, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT);

	/* Draw bright box */
	Put_recoloured_box(Current_OPM, Object->X, Object->Y, ITEM_SLOT_INNER_WIDTH,
	 ITEM_SLOT_INNER_HEIGHT, &(Recolour_tables[6][0]));

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_up_Item_slot_object
 * FUNCTION  : Pop-up method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 13:36
 * LAST      : 05.01.95 13:36
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Pop_up_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	static UNCHAR Item_name[ITEM_NAME_LENGTH + 1];
	struct Item_list_object *Item_list;
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_descriptor *Selected_item;
	UNSHORT Index;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Does this item list have a pop-up menu ? */
	if (Item_list->Menu)
	{
		/* Yes -> Get packet */
		Extract_slot_packet(Item_slot, &Packet);

		/* Is there something in the slot ? */
		if (!Packet_empty(&Packet))
		{
			/* Yes -> Get item name */
			Get_item_name(&Packet, Item_name);

			/* Make this the menu's title */
			Item_list->Menu->Title = Item_name;

			/* Get packet index */
			Index = Item_slot->Number;
			if (Item_list->Scroll_bar_object)
			{
				Index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
				 GET_METHOD, NULL);
			}

			/* Set item data */
			Selected_item = (struct Item_descriptor *) Item_list->Menu->Data;
			Selected_item->Slots_handle = Item_list->Slots_handle;
			Selected_item->Slots_offset = Item_list->Slots_offset;
			Selected_item->Item_slot_index = Index;

			/* Call pop-up menu */
			PUM_source_object_handle = Object->Self;
			Do_PUM(Object->X + 16, Object->Y + 8, Item_list->Menu);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Item_slot_object
 * FUNCTION  : Help method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.01.95 19:24
 * LAST      : 10.07.95 13:15
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;

	struct Item_data *Target_item;

	UNCHAR Item_name[ITEM_NAME_LENGTH + 1];
	UNCHAR Item_name2[ITEM_NAME_LENGTH + 1];

	UNCHAR String[100];

	Item_slot = (struct Item_slot_object *) Object;

  	/* Get target packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* In drag & drop mode / dragging an item ? */
	if (Drag_drop_mode &&
	((Current_drag_drop_data.Data_ID == BODY_ITEM_DD_DATA_ID) ||
	 (Current_drag_drop_data.Data_ID == BACKPACK_ITEM_DD_DATA_ID) ||
	 (Current_drag_drop_data.Data_ID == CHEST_ITEM_DD_DATA_ID)))
	{
		/* Yes -> Is the slot empty ? */
		if (Packet_empty(&Packet))
		{
		 	/* Yes -> Get dragged item name */
			Get_item_name(&Drag_packet, Item_name);

			/* Make help line string */
			sprintf(String, System_text_ptrs[517], Item_name);

			/* Print help line */
			Execute_method(Help_line_object, SET_METHOD, (void *) String);
		}
		else
		{
			/* No -> Get target slot item data */
			Target_item = Get_item_data(&Packet);

			/* Both packets contain same item / multiple item ? */
			if ((Drag_packet.Index == Packet.Index) &&
			 (Target_item->Flags & MULTIPLE))
			{
				/* Yes -> Is the target packet full ? */
				if (Packet.Quantity >= ITEMS_PER_PACKET)
				{
					/* Yes -> "Slot is full!" */
					Execute_method(Help_line_object, SET_METHOD,
					 (void *) System_text_ptrs[519]);
				}
				else
				{
					/* No -> "Add to slot" */
					Execute_method(Help_line_object, SET_METHOD,
					 (void *) System_text_ptrs[520]);
				}
			}
			else
			{
				/* No -> Get target slot item name */
				Get_item_name(&Packet, Item_name);

			 	/* Get dragged item name */
				Get_item_name(&Drag_packet, Item_name2);

				/* Make help line string */
				sprintf(String, System_text_ptrs[518], Item_name2, Item_name);

				/* Print help line */
				Execute_method(Help_line_object, SET_METHOD, (void *) String);
			}

			Free_item_data();
		}
	}
	else
	{
		/* No -> Is the slot empty ? */
		if (!Packet_empty(&Packet))
		{
			/* Yes -> Get target slot item name */
			Get_item_name(&Packet, Item_name);

			/* Make help line string */
			sprintf(String, System_text_ptrs[22], Item_name);

			/* Print help line */
			Execute_method(Help_line_object, SET_METHOD, (void *) String);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_Item_slot_object
 * FUNCTION  : Right method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 16:20
 * LAST      : 11.01.95 16:20
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Is there something in the slot ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Do normal action */
		Normal_rightclicked(Object, P);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touch_Item_slot_object
 * FUNCTION  : Touch method of Item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 09:50
 * LAST      : 11.01.95 09:50
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Touch_Item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Is there something in the slot OR are we in drag & drop mode ? */
	if (!Packet_empty(&Packet) || Drag_drop_mode)
	{
		/* Yes -> Touch normally */
		Normal_touched(Object, NULL);
	}
	else
	{
		/* No -> Dehighlight */
		Dehighlight(Object, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Extract_slot_packet
 * FUNCTION  : Make a COPY of an item packet belonging to an item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 13:42
 * LAST      : 24.02.95 11:32
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 *             struct Item_packet *Destination : Pointer to destination item
 *              packet.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Extract_slot_packet(struct Item_slot_object *Item_slot, struct Item_packet
 *Destination)
{
	struct Item_list_object *Item_list;
	struct Item_packet *Inventory;
	UNSHORT Index;

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get inventory address */
	Inventory = (struct Item_packet *) (MEM_Claim_pointer(Item_list->Slots_handle)
	 + Item_list->Slots_offset);

	/* Get packet index */
	Index = Item_slot->Number;
	if (Item_list->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Copy the packet */
	memcpy(Destination, &Inventory[Index - 1], sizeof(struct Item_packet));

	MEM_Free_pointer(Item_list->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Claim_slot_packet
 * FUNCTION  : Claim the pointer to an item packet belonging to an item slot
 *              object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.02.95 17:33
 * LAST      : 27.02.95 17:33
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 * RESULT    : struct Item_packet * : Pointer to item packet.
 * BUGS      : No known.
 * NOTES     : - Every call to this function should be accompanied by a call
 *              to Free_slot_packet.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Item_packet *
Claim_slot_packet(struct Item_slot_object *Item_slot)
{
	struct Item_list_object *Item_list;
	struct Item_packet *Inventory;
	UNSHORT Index;

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get inventory address */
	Inventory = (struct Item_packet *) (MEM_Claim_pointer(Item_list->Slots_handle)
	 + Item_list->Slots_offset);

	/* Get packet index */
	Index = Item_slot->Number;
	if (Item_list->Scroll_bar_object)
	{
		Index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Return pointer */
	return(&Inventory[Index - 1]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Free_slot_packet
 * FUNCTION  : Free the pointer to an item packet belonging to an item slot
 *              object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.02.95 17:35
 * LAST      : 27.02.95 17:35
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Free_slot_packet(struct Item_slot_object *Item_slot)
{
	struct Item_list_object *Item_list;

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Free pointer */
	MEM_Free_pointer(Item_list->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_item_slot_object
 * FUNCTION  : Find an item slot object belonging to an item list.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 16:38
 * LAST      : 03.03.95 16:38
 * INPUTS    : UNSHORT Item_list_object - Handle of item list object.
 *             UNSHORT Number - Item slot number.
 * RESULT    : struct Item_slot_object * : Pointer to item slot object /
 *              NULL = failure.
 * BUGS      : No known.
 * NOTES     : - The item list better contain item slots.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Item_slot_object *
Find_item_slot_object(UNSHORT Item_list_object, UNSHORT Number)
{
	struct Item_list_object *Item_list;
	struct Item_slot_object *Item_slot;
	UNSHORT Handle;

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_list_object);

	/* Get first potential item slot */
	Handle = Item_list->Object.Child;

	while (Handle)
	{
		/* Get object data */
		Item_slot = (struct Item_slot_object *) Get_object_data(Handle);

		/* Has the right number ? */
		if (Item_slot->Number == Number)
		{
			/* Yes -> Exit */
			return(Item_slot);
		}

		/* No -> Next object */
		Handle = Item_slot->Object.Next;
	}

	/* Nothing was found */
	return(NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_item_slot_item
 * FUNCTION  : Draw the item of an item slot object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.01.95 15:42
 * LAST      : 07.01.95 15:42
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_item_slot_item(struct Item_slot_object *Item_slot)
{
	struct Object *Object;
	struct Item_packet Packet;

	Object = &(Item_slot->Object);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Draw item */
		Draw_item(Current_OPM, Object->X, Object->Y, &Packet, Item_slot->Frame);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_item
 * FUNCTION  : Draw an item.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.01.95 15:46
 * LAST      : 05.07.95 18:03
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             struct Item_packet *Packet - Pointer to item packet.
 *             UNSHORT Frame - Animation frame number (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes the background has been cleared.
 *             - This function cannot be used for body items, because it
 *              may display a quantity.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_item(struct OPM *OPM, SISHORT X, SISHORT Y, struct Item_packet *Packet,
 UNSHORT Frame)
{
	/* Draw item */
	Draw_body_item(OPM, X, Y, Packet, Frame);

	/* Draw quantity (if > 1) */
	if (Packet->Quantity > 1)
	{
		UNCHAR Number[4];

		/* Set ink colour */
		Set_ink(SILVER_TEXT);

		/* Print quantity */
		sprintf(Number, "%3u", Packet->Quantity);
		Print_centered_string(OPM, X, Y + QUANTITY_Y_OFFSET,
		 ITEM_SLOT_INNER_WIDTH, Number);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_body_item
 * FUNCTION  : Draw a body item.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 13:54
 * LAST      : 27.04.95 15:13
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             struct Item_packet *Packet - Pointer to item packet.
 *             UNSHORT Frame - Animation frame number (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes the background has been cleared.
 *             - This function can be used for body items, because it
 *              does NOT display a quantity.
 *             - When the contents of item slots are swapped, it is very hard
 *              to make sure that the animation frame remains valid. Since
 *              this is the central drawing function for item slots, this
 *              function checks if the given animation frame is valid.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_body_item(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Item_packet *Packet, UNSHORT Frame)
{
	struct Item_data *Item_data;
	UNSHORT Pic_nr;
	UNBYTE *Ptr;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is the given animation frame too high ? */
	if (Frame >= Item_data->Nr_frames)
	{
		/* Yes -> Take first frame */
		Frame = 0;
	}

	/* Get item graphics number */
	Pic_nr = Item_data->Pic_nr + Frame;

	/* Get pointer to item graphics */
	Ptr = MEM_Claim_pointer(Item_graphics_handle) + (Pic_nr * 256);

	/* Draw item */
	Put_masked_block(OPM, X, Y, 16, 16, Ptr);

	MEM_Free_pointer(Item_graphics_handle);
	Free_item_data();

	/* Is the item broken ? */
	if (Packet->Flags & BROKEN_ITEM)
	{
		/* Yes -> Draw broken symbol over item graphics */
		Put_masked_block(OPM, X, Y, 16, 16, Broken_symbol);
	}
}

