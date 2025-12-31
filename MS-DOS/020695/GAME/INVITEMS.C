/************
 * NAME     : INVITEMS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : INVITEMS.H
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <HDOB.H>
#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GAMETEXT.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <POPUP.H>
#include <INPUT.H>
#include <SCROLBAR.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <ITEMLIST.H>
#include <BUTTONS.H>
#include <INPUT.H>
#include <EVELOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <MAGIC.H>

/* structure definitions */
struct Take_item_from_char_data {
	MEM_HANDLE Char_handle;
	UNSHORT Slot_index;
	UNSHORT Quantity;
	struct Item_packet *Target_packet;
};

struct Put_item_in_char_data {
	MEM_HANDLE Char_handle;
	UNSHORT Slot_index;
	UNSHORT Quantity;
	struct Item_packet *Source_packet;
};

/* global variables */

static struct Item_descriptor Selected_item;

/* Body item slot object class */
static struct Method Body_item_list_methods[] = {
	{ INIT_METHOD, Init_Body_item_list_object },
	{ DRAW_METHOD, Draw_Body_item_list_object },
	{ DROP_METHOD, Drop_Body_item_slot_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

struct Object_class Body_item_list_Class = {
	0, sizeof(struct Item_list_object),
	&Body_item_list_methods[0]
};

static struct Method Body_item_slot_methods[] = {
	{ INIT_METHOD, Init_Item_slot_object },
	{ DRAW_METHOD, Draw_Body_item_slot_object },
	{ UPDATE_METHOD, Update_Body_item_slot_object },
	{ FEEDBACK_METHOD, Feedback_Body_item_slot_object },
	{ HIGHLIGHT_METHOD, Highlight_Body_item_slot_object },
	{ POP_UP_METHOD, Pop_up_Item_slot_object },
	{ HELP_METHOD, Help_Item_slot_object },
	{ LEFT_METHOD, Left_Body_item_slot_object },
	{ RIGHT_METHOD, Right_Item_slot_object },
	{ TOUCHED_METHOD, Touch_Item_slot_object },
	{ DROP_METHOD, Drop_Body_item_slot_object },
	{ 0, NULL}
};

static struct Object_class Body_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Body_item_slot_methods[0]
};

/* Backpack item slot object class */
static struct Method Backpack_item_slot_methods[] = {
	{ INIT_METHOD, Init_Item_slot_object },
	{ DRAW_METHOD, Draw_Item_slot_object },
	{ UPDATE_METHOD, Update_Item_slot_object },
	{ FEEDBACK_METHOD, Feedback_Item_slot_object },
	{ HIGHLIGHT_METHOD, Highlight_Item_slot_object },
	{ POP_UP_METHOD, Pop_up_Item_slot_object },
	{ HELP_METHOD, Help_Item_slot_object },
	{ LEFT_METHOD, Left_Backpack_item_slot_object },
	{ DLEFT_METHOD, DLeft_Backpack_item_slot_object },
	{ RIGHT_METHOD, Right_Item_slot_object },
	{ TOUCHED_METHOD, Touch_Item_slot_object },
	{ DROP_METHOD, Drop_Backpack_item_slot_object },
	{ 0, NULL}
};

struct Object_class Backpack_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Backpack_item_slot_methods[0]
};

/* Inventory item pop-up menu */
static struct PUME Inventory_item_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 15, PUM_Drop_inventory_item},
	{PUME_AUTO_CLOSE, 0, 16, PUM_Learn_spell_inventory_item},
	{PUME_AUTO_CLOSE, 0, 17, PUM_Examine_inventory_item},
	{PUME_AUTO_CLOSE, 0, 18, PUM_Drink_inventory_item},
	{PUME_AUTO_CLOSE, 0, 19, PUM_Activate_spell_inventory_item},
	{PUME_AUTO_CLOSE, 0, 20, PUM_Read_inventory_item},
	{PUME_AUTO_CLOSE, 0, 131, PUM_Activate_inventory_item}
};

struct PUM Inventory_item_PUM = {
	7,
	NULL,
	(UNLONG) &Selected_item,
	Inventory_item_PUM_evaluator,
	Inventory_item_PUMEs
};

/* Positions of body item slots for each potential party member */
static UNSHORT Body_slot_positions[PARTY_CHARS_MAX][ITEMS_ON_BODY][2] = {
	{{220, 25}, {177, 16}, {0, 0}, {151, 67}, {200, 60}, {247, 67},
	 {151, 86}, {200, 155}, {247, 86}},
	{{220, 28}, {178, 15}, {0, 0}, {161, 77}, {201, 61}, {244, 77},
	 {161, 96}, {201, 149}, {244, 96}},
	{{191, 32}, {191, 13}, {153, 34}, {183, 63}, {211, 58}, {254, 63},
	 {183, 82}, {203, 158}, {254, 82}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{227, 27}, {185, 15}, {0, 0}, {151, 77}, {207, 60}, {245, 77},
	 {151, 96}, {206, 154}, {245, 96}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}}
};

/* Drag data */
UNSHORT Drag_source_item_slot_index;
struct Item_slot_object *Drag_source_item_slot_object;

struct Item_packet Drag_packet;

MEM_HANDLE Drag_OPM_handle;
struct OPM Drag_OPM;

UNSHORT Drag_HDOB_nr;
struct HDOB Drag_HDOB = {
	HDOB_MASK,
	HDOB_ATTACHED,
	3, 3,
	ITEM_SLOT_INNER_WIDTH, ITEM_SLOT_INNER_HEIGHT,
	0,
	0,
	NULL,
	0,
	NULL,
	NULL,
	0,
	0,
	NULL
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Body_item_list_object
 * FUNCTION  : Initialize method of Body item list object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 12:21
 * LAST      : 09.01.95 12:21
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_Body_item_list_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_object *Item_list;
	struct Item_list_OID *OID;
	struct Item_slot_OID Item_slot_OID;
	UNSHORT Char_nr, i;

	Item_list = (struct Item_list_object *) Object;
	OID = (struct Item_list_OID *) P;

	/* Copy data from OID */
	Item_list->Type = OID->Type;
	Item_list->Slots_handle = OID->Slots_handle;
	Item_list->Slots_offset = OID->Slots_offset;
	Item_list->Menu = OID->Menu;

	/* Get inventory character number */
	Char_nr = PARTY_DATA.Member_nrs[Inventory_member - 1];

	/* Make item slots */
	Item_slot_OID.Number = NECK;
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is not tail slot OR Iskai ? */
		if ((i != TAIL-1) || (Get_race(Inventory_char_handle) == ISKAI_RACE))
		{
			/* Yes -> Add object */
			Add_object(Object->Self, &Body_item_slot_Class, (UNBYTE *) &Item_slot_OID,
			 Body_slot_positions[Char_nr-1][i][0] - Object->Rect.left,
			 Body_slot_positions[Char_nr-1][i][1] - Object->Rect.top,
			 BODY_ITEM_SLOT_WIDTH, BODY_ITEM_SLOT_HEIGHT);
		}

		/* Count up */
		Item_slot_OID.Number++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Body_item_list_object
 * FUNCTION  : Draw method of Body item list object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 12:20
 * LAST      : 09.01.95 12:20
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_Body_item_list_object(struct Object *Object, union Method_parms *P)
{
	struct Gfx_header *Gfx;

	/* Clear item list */
	Restore_background(Object->Rect.left, Object->Rect.top, Object->Rect.width,
	 Object->Rect.height);

	/* Draw full-body picture */
	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Full_body_pic_handle);
	Put_masked_block(Current_OPM, Object->X, Object->Y, Gfx->Width, Gfx->Height,
	 (UNBYTE *) (Gfx + 1));
	MEM_Free_pointer(Full_body_pic_handle);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	/* Draw rectangle around body item list area */
	Draw_deep_border(Current_OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Body_item_slot_object
 * FUNCTION  : Draw method of Body item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 15:50
 * LAST      : 09.01.95 15:50
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Body_item_slot;

	Body_item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Erase_body_item_slot(Body_item_slot);

	/* Draw border */
	Draw_high_border(Current_OPM, Object->X, Object->Y, BODY_ITEM_SLOT_WIDTH,
	 BODY_ITEM_SLOT_HEIGHT);

	/* Draw item */
	Draw_body_item_slot_item(Body_item_slot);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Body_item_slot_object
 * FUNCTION  : Update method of Body item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 18:24
 * LAST      : 01.03.95 18:24
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_Body_item_slot_object(struct Object *Object, union Method_parms *P)
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
				Feedback_Body_item_slot_object(Object, P);
			}
			else
			{
				/* No -> Is this the highlighted object ? */
				if (Highlighted_object == Object->Self)
				{
					/* Yes -> Re-draw item, highlighted */
					Highlight_Body_item_slot_object(Object, P);
				}
				else
				{
					/* No -> Re-draw item */
					Draw_Body_item_slot_object(Object, P);
				}
			}
		}
		Free_item_data();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Body_item_slot_object
 * FUNCTION  : Feedback method of Body item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 15:49
 * LAST      : 09.01.95 15:49
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Feedback_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Body_item_slot;

	Body_item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Erase_body_item_slot(Body_item_slot);

	/* Draw dark box */
	Draw_deep_box(Current_OPM, Object->X, Object->Y, BODY_ITEM_SLOT_WIDTH,
	 BODY_ITEM_SLOT_HEIGHT);

	/* Draw item */
	Draw_body_item_slot_item(Body_item_slot);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Body_item_slot_object
 * FUNCTION  : Highlight method of Body item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 15:49
 * LAST      : 09.01.95 15:49
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Highlight_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Body_item_slot;

	Body_item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Erase_body_item_slot(Body_item_slot);

	/* Draw light box */
	Draw_light_box(Current_OPM, Object->X, Object->Y, BODY_ITEM_SLOT_WIDTH,
	 BODY_ITEM_SLOT_HEIGHT);

	/* Draw item */
	Draw_body_item_slot_item(Body_item_slot);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Body_item_slot_object
 * FUNCTION  : Left method of Body item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 17:06
 * LAST      : 11.01.95 17:06
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Left_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Try to take the item */
			if (Take_item_from_character(Inventory_char_handle,
			 Item_slot->Number, 1, &Drag_packet))
			{
				/* Success -> Drag item */
				Drag_body_item(Item_slot);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Body_item_slot_object
 * FUNCTION  : Drop method of Body item slot AND list object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 18:38
 * LAST      : 01.03.95 18:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_body_item((struct Item_slot_object *) Object);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drag_body_item
 * FUNCTION  : Drag a body item.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.02.95 18:15
 * LAST      : 03.03.95 11:55
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drag_body_item(struct Item_slot_object *Item_slot)
{
	/* Redraw item slot */
	Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

	/* Build graphics */
	Build_item_graphics();

	/* Add HDOB */
	Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

	/* Pick mouse pointer */
	Change_mouse(&(Mouse_pointers[PICK_MPTR]));

	/* Set drag source */
	Drag_source_item_slot_index = Item_slot->Number;
	Drag_source_item_slot_object = Item_slot;

	/* Enter drag & drop mode */
	Enter_drag_drop_mode(Item_drag_abort_handler);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_body_item
 * FUNCTION  : Drop on a body item.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 15:46
 * LAST      : 03.03.95 15:46
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to target item
 *              slot object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_body_item(struct Item_slot_object *Item_slot)
{
	struct Item_packet Swap_packet;
	struct Item_data *Source_item;
	UNSHORT Target_slot_index;

	/* Get source item data */
	Source_item = Get_item_data(&Drag_packet);

	/* Is the source item equippable ? */
	if (Source_item->Body_place == NO_BODY_PLACE)
	{
		/* No -> Apologise to the player */
		Set_permanent_message_nr(68);
	}
	else
	{
		/* Yes -> Determine target slot index */
		Target_slot_index = Find_target_slot_for_item(Source_item);

		/* Is the target slot free ? */
		if (Target_slot_index == 0xFFFF)
		{
			/* No -> Set target slot index */
			Target_slot_index = Source_item->Body_place;

			/* Try to remove target item */
			if (Take_item_from_character(Inventory_char_handle,
			 Target_slot_index, 1, &Swap_packet))
			{
				/* Success -> Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_slot(Target_slot_index);

				/* Try to add source item to body */
				if (Put_item_in_character(Inventory_char_handle,
				 Target_slot_index, 1, &Drag_packet))
				{
					/* Success -> Redraw item slots */
					Execute_method(Body_item_list_object, DRAW_METHOD, NULL);

					/* Copy swapped packet to drag packet */
					memcpy(&Drag_packet, &Swap_packet,
					 sizeof(struct Item_packet));

					/* Re-build graphics */
					Build_item_graphics();

					/* Set new drag source */
					Drag_source_item_slot_index = Target_slot_index;
					Drag_source_item_slot_object = Find_item_slot_object(Body_item_list_object,
					 Target_slot_index);
				}
				else
				{
					/* Failure -> Try to swap back */
					if (!Put_item_in_character(Inventory_char_handle,
					 Target_slot_index, 1, &Swap_packet))
					{
						/* Failure -> Error! */
						Error(ERROR_ITEM_NO_SWAP_BACK);
					}
				}
			}
		}
		else
		{
			/* Yes -> Move drag HDOB towards target slot */
			Move_drag_HDOB_towards_slot(Target_slot_index);

			/* Try to add source item to body */
			if (Put_item_in_character(Inventory_char_handle,
			 Target_slot_index, 1, &Drag_packet))
			{
				/* Success -> Redraw item slots */
				Execute_method(Body_item_list_object, DRAW_METHOD, NULL);

				/* All items gone ? */
				if (!Packet_empty(&Drag_packet))
				{
					/* No -> Re-build graphics */
					Build_item_graphics();
				}
				else
				{
					/* Yes -> Delete HDOB */
					Delete_HDOB(Drag_HDOB_nr);

					/* End of drag & drop mode */
					Leave_drag_drop_mode();
				}
			}
		}
	}
	Free_item_data();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Erase_body_item_slot
 * FUNCTION  : Erase a body item slot.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 15:57
 * LAST      : 09.01.95 15:57
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Erase_body_item_slot(struct Item_slot_object *Item_slot)
{
	struct Object *Object, *Parent;
	struct Gfx_header *Gfx;
	struct BBRECT Clip, Old;

	Object = &(Item_slot->Object);
	Parent = Get_object_data(Object->Parent);

	/* Draw slab */
	Restore_background(Object->Rect.left, Object->Rect.top, BODY_ITEM_SLOT_WIDTH,
	 BODY_ITEM_SLOT_HEIGHT);

	/* Make clipping rectangle */
	Clip.left = Object->X;
	Clip.top = Object->Y;
	Clip.width = BODY_ITEM_SLOT_WIDTH;
	Clip.height = BODY_ITEM_SLOT_HEIGHT;

	/* Install clip area */
	memcpy(&Old, &(Main_OPM.clip), sizeof(struct BBRECT));
	memcpy(&(Main_OPM.clip), &Clip, sizeof(struct BBRECT));

	/* Draw full-body picture */
	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Full_body_pic_handle);
	Put_masked_block(Current_OPM, Parent->X, Parent->Y, Gfx->Width, Gfx->Height,
	 (UNBYTE *) (Gfx + 1));
	MEM_Free_pointer(Full_body_pic_handle);

	/* Restore clip area */
	memcpy(&(Main_OPM.clip), &Old, sizeof(struct BBRECT));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_body_item_slot_item
 * FUNCTION  : Draw the item of a body item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 15:27
 * LAST      : 09.01.95 15:27
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_body_item_slot_item(struct Item_slot_object *Item_slot)
{
	struct Object *Object;
	struct Item_packet Packet;
	struct Item_list_object *Item_list;
	struct Character_data *Char;
	struct Item_data *Item_data;

	Object = &(Item_slot->Object);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Draw item */
		Draw_body_item(Current_OPM, Object->X + 1, Object->Y + 1, &Packet,
		 Item_slot->Frame);
	}
	else
	{
		/* No -> Is left hand slot ? */
		if (Item_slot->Number == LEFT_HAND)
		{
			/* Yes -> Get item list data */
			Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);
			Char = (struct Character_data *) MEM_Claim_pointer(Item_list->Slots_handle);

			/* Anything carried in right hand ? */
			if (!Packet_empty(&(Char->Body_items[RIGHT_HAND - 1])))
			{
				/* Yes -> Get item data */
				Item_data = Get_item_data(&(Char->Body_items[RIGHT_HAND - 1]));

				/* Is a two-handed weapon ? */
				if (Item_data->Hand_use == 2)
				{
					/* Yes -> Draw cross over left hand slot */
					Put_masked_block(Current_OPM, Object->X + 1, Object->Y + 1,
					 16, 16, &(Cross_symbol[0]));
				}

				Free_item_data();
			}

			MEM_Free_pointer(Item_list->Slots_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Backpack_item_slot_object
 * FUNCTION  : Left method of Backpack item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 13:50
 * LAST      : 11.01.95 13:50
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Left_Backpack_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item;
	UNSHORT Quantity;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Get item data */
			Item = Get_item_data(&Packet);

			/* More than one item in the packet ? */
			Quantity = 1;
			if (Packet.Quantity > 1)
			{
				/* Yes -> Ask the player how many should be taken */
				Quantity = (UNSHORT) Input_number_with_symbol(1, 0,
				 (SILONG) Packet.Quantity, System_text_ptrs[66], 16, 16,
				 Item_graphics_handle, Item->Pic_nr * 256);
			}

			Free_item_data();

			/* Zero ? */
			if (Quantity)
			{
				/* No -> Try to take the items */
				if (Take_item_from_character(Inventory_char_handle,
				 Item_slot->Number + ITEMS_ON_BODY, Quantity, &Drag_packet))
				{
					/* Success -> Drag items */
					Drag_backpack_item(Item_slot, Quantity);
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : DLeft_Backpack_item_slot_object
 * FUNCTION  : DLeft method of Backpack item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.02.95 17:05
 * LAST      : 27.02.95 17:05
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
DLeft_Backpack_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Try to take all the items */
			if (Take_item_from_character(Inventory_char_handle,
			 Item_slot->Number + ITEMS_ON_BODY, Packet.Quantity, &Drag_packet))
			{
				/* Success -> Drag all items */
				Drag_backpack_item(Item_slot, Packet.Quantity);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Backpack_item_slot_object
 * FUNCTION  : Drop method of Backpack item slot object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 15:19
 * LAST      : 11.01.95 15:19
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function won't work properly when the backpack items
 *              have a scroll-bar.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_Backpack_item_slot_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_backpack_item((struct Item_slot_object *) Object);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drag_backpack_item
 * FUNCTION  : Drag backpack items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.02.95 17:03
 * LAST      : 03.03.95 11:55
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot object.
 *             UNSHORT Quantity - Quantity that should be dragged.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drag_backpack_item(struct Item_slot_object *Item_slot, UNSHORT Quantity)
{
	/* Any taken ? */
	if (Quantity)
	{
		/* Yes -> Redraw item slot */
		Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

		/* Build graphics */
		Build_item_graphics();

		/* Add HDOB */
		Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

		/* Pick mouse pointer */
		Change_mouse(&(Mouse_pointers[PICK_MPTR]));

		/* Set drag source */
		Drag_source_item_slot_index = Item_slot->Number + ITEMS_ON_BODY;
		Drag_source_item_slot_object = Item_slot;

		/* Enter drag & drop mode */
		Enter_drag_drop_mode(Item_drag_abort_handler);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_backpack_item
 * FUNCTION  : Drop on backpack item.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 15:43
 * LAST      : 03.03.95 15:43
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to target item
 *              slot object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_backpack_item(struct Item_slot_object *Item_slot)
{
	struct Item_packet *Target_packet;
	struct Item_packet Swap_packet;

	struct Item_data *Source_item;

	UNSHORT Target_slot_index;
	UNSHORT Quantity;

	/* Claim target packet */
	Target_packet = Claim_slot_packet(Item_slot);

	/* Calculate target slot index */
	Target_slot_index = Item_slot->Number + ITEMS_ON_BODY;

	/* Any item in target packet ? */
	if (!Packet_empty(Target_packet))
	{
		/* Yes -> Get source item data */
		Source_item = Get_item_data(&Drag_packet);

		/* Both packets contain same item / multiple item ? */
		if ((Drag_packet.Index == Target_packet->Index) &&
		 (Source_item->Flags & MULTIPLE))
		{
			/* Yes -> Is the target packet full ? */
			if (Target_packet->Quantity < ITEMS_PER_PACKET)
			{
				/* No -> Determine quantity that can be added to the
				 target packet */
				Quantity = min(Drag_packet.Quantity, ITEMS_PER_PACKET -
				 Target_packet->Quantity - Drag_packet.Quantity);

				/* Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_slot(Target_slot_index);

				/* Try to add source items to backpack */
				if (Put_item_in_character(Inventory_char_handle,
				 Target_slot_index, Quantity, &Drag_packet))
				{
					/* Success -> Redraw item slot */
					Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

					/* All items gone ? */
					if (!Packet_empty(&Drag_packet))
					{
						/* No -> Re-build graphics */
						Build_item_graphics();
					}
					else
					{
						/* Yes -> Delete HDOB */
						Delete_HDOB(Drag_HDOB_nr);

						/* End of drag & drop mode */
						Leave_drag_drop_mode();
					}
				}
			}
		}
		else
		{
			/* No -> Try to remove target items */
			if (Take_item_from_character(Inventory_char_handle,
			 Target_slot_index, Target_packet->Quantity, &Swap_packet))
			{
				/* Success -> Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_slot(Target_slot_index);

				/* Try to add source items to backpack */
				if (Put_item_in_character(Inventory_char_handle,
				 Target_slot_index, Drag_packet.Quantity, &Drag_packet))
				{
					/* Success -> Redraw item slot */
					Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

					/* Copy swapped packet to drag packet */
					memcpy(&Drag_packet, &Swap_packet, sizeof(struct Item_packet));

					/* Re-build graphics */
					Build_item_graphics();

					/* Set new drag source */
					Drag_source_item_slot_index = Target_slot_index;
					Drag_source_item_slot_object = Find_item_slot_object(Backpack_item_list_object,
					 Target_slot_index - ITEMS_ON_BODY);
				}
				else
				{
					/* Failure -> Try to swap back */
					if (!Put_item_in_character(Inventory_char_handle,
					 Target_slot_index, Swap_packet.Quantity, &Swap_packet))
					{
						/* Failure -> Error! */
						Error(ERROR_ITEM_NO_SWAP_BACK);
					}
				}
			}
		}
		Free_item_data();
	}
	else
	{
		/* No -> Move drag HDOB towards target slot */
		Move_drag_HDOB_towards_slot(Target_slot_index);

		/* Try to add items to backpack */
		if (Put_item_in_character(Inventory_char_handle,
		 Target_slot_index, Drag_packet.Quantity, &Drag_packet))
		{
			/* Success -> Redraw item slot */
			Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

			/* All items gone ? */
			if (!Packet_empty(&Drag_packet))
			{
				/* No -> Re-build graphics */
				Build_item_graphics();
			}
			else
			{
				/* Yes -> Delete HDOB */
				Delete_HDOB(Drag_HDOB_nr);

				/* End of drag & drop mode */
				Leave_drag_drop_mode();
			}
		}
	}
	/* Free item packet */
	Free_slot_packet(Item_slot);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_item_drag
 * FUNCTION  : Initialize item dragging.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:27
 * LAST      : 27.01.95 15:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_item_drag(void)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Allocate memory for dragged item graphics */
	Drag_OPM_handle = MEM_Do_allocate(ITEM_SLOT_INNER_WIDTH *
	 ITEM_SLOT_INNER_HEIGHT * MAX_ITEM_FRAMES, (UNLONG) &Drag_OPM, &OPM_ftype);
	if (!Drag_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
	}

	/* Insert handle in HDOB data */
	Drag_HDOB.Graphics_handle = Drag_OPM_handle;

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Drag_OPM_handle);
	Result = OPM_New(ITEM_SLOT_INNER_WIDTH, ITEM_SLOT_INNER_HEIGHT *
	 MAX_ITEM_FRAMES, 1, &Drag_OPM, Ptr);
	MEM_Free_pointer(Drag_OPM_handle);

	if (!Result)
	{
		Error(ERROR_NO_OPM);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_item_drag
 * FUNCTION  : Exit item dragging.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:32
 * LAST      : 27.01.95 15:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_item_drag(void)
{
	/* Delete dragged item OPM and free memory */
	OPM_Del(&Drag_OPM);
	MEM_Free_memory(Drag_OPM_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_item_graphics
 * FUNCTION  : Build item graphics for dragging.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 17:22
 * LAST      : 01.03.95 17:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Since this function uses the drag OPM and is only intended
 *              for dragging items, it will use the drag item packet for
 *              source data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Build_item_graphics(void)
{
	struct Item_data *Item_data;
	UNSHORT Nr_frames;
	UNSHORT i;

	/* Get number of animation frames */
	Item_data = Get_item_data(&Drag_packet);
	Nr_frames = Item_data->Nr_frames;
	Free_item_data();

	if (Nr_frames < 1)
		Nr_frames = 1;

	if (Nr_frames > MAX_ITEM_FRAMES)
		Nr_frames = MAX_ITEM_FRAMES;

	/* Clear drag OPM */
	OPM_FillBox(&Drag_OPM, 0, 0, ITEM_SLOT_INNER_WIDTH,
	 Nr_frames * ITEM_SLOT_INNER_HEIGHT, 0);

	/* Draw each frame of item */
	for (i=0;i<Nr_frames;i++)
	{
		Draw_item(&Drag_OPM, 0, i * ITEM_SLOT_INNER_HEIGHT, &Drag_packet, i);
	}

	/* Set number of animation frames */
	Drag_HDOB.Nr_frames = Nr_frames;

	/* Change HDOB data */
	Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_drag_HDOB_towards_slot
 * FUNCTION  : Move drag HDOB towards target slot.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 09:50
 * LAST      : 03.03.95 09:50
 * INPUTS    : UNSHORT Target_slot_index (1...33).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_drag_HDOB_towards_slot(UNSHORT Target_slot_index)
{
	struct Item_slot_object *Target_item_slot;
	SISHORT X, Y;

	/* Body or backpack item ? */
	if (Target_slot_index <= ITEMS_ON_BODY)
	{
		/* Body -> Find corresponding body item slot */
		Target_item_slot = Find_item_slot_object(Body_item_list_object,
		 Target_slot_index);
	}
	else
	{
		/* Backpack -> Find corresponding backpack item slot */
		Target_item_slot = Find_item_slot_object(Backpack_item_list_object,
		 Target_slot_index - ITEMS_ON_BODY);
	}

	/* Already over target ? */
	if (!Is_over_object(Target_item_slot->Object.Self))
	{
		/* No -> Get target coordinates */
		X = Target_item_slot->Object.Rect.left;
		Y = Target_item_slot->Object.Rect.top;

		/* Body item ? */
		if (Target_slot_index <= ITEMS_ON_BODY)
		{
			/* Yes -> Add offset */
			X++;
			Y++;
		}

		/* Detach the HDOB from the mouse pointer */
		Drag_HDOB.Flags &= ~HDOB_ATTACHED;

		/* Set absolute coordinates of HDOB */
		Drag_HDOB.X += Mouse_X;
		Drag_HDOB.Y += Mouse_Y;

		/* Change HDOB data */
		Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

		/* Mouse off */
		Mouse_off();

		/* Move HDOB towards target */
		Move_HDOB(Drag_HDOB_nr, X, Y, 8);

		/* Attach HDOB to mouse pointer */
		Drag_HDOB.Flags |= HDOB_ATTACHED;

		/* Set relative coordinates of HDOB */
		Drag_HDOB.X = 3;
		Drag_HDOB.Y = 3;

		/* Change HDOB data */
		Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

		/* Mouse on */
		Mouse_on();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Item_drag_abort_handler
 * FUNCTION  : Item drag & drop abort handler.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 15:00
 * LAST      : 03.03.95 15:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Item_drag_abort_handler(void)
{

/*
	move.w	Moving_member,d0		; Same guy ?
	cmp.w	Inventory_member,d0
	beq.s	.Skip
	lsl.w	#8,d0			; No -> switch
	jsr	Inv2_Mright
.Skip:
*/

	if (Drag_source_item_slot_index <= ITEMS_ON_BODY)
		Drop_on_body_item(Drag_source_item_slot_object);
	else
		Drop_on_backpack_item(Drag_source_item_slot_object);

/*	move.w	Source_slot,d0		; Try to put it back
	cmp.w	#9+1,d0			; Body ?
	bmi	Body_item_moved
	sub.w	#9,d0			; No -> Backpack
	move.w	Scroll_bar_result,d1	; Slot out of view ?
	move.w	d0,d2
	sub.w	d1,d2
	cmp.w	#12,d2
	ble.s	.No
	jsr	Set_scroll_bar2		; Set scroll bar
.No:	sub.w	Scroll_bar_result,d0	; Do
	bra	Backpack_item_moved
*/
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_item_PUM_evaluator
 * FUNCTION  : Evaluate inventory item pop-up menu.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 05.01.95 15:41
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Inventory_item_PUM_evaluator(struct PUM *PUM)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	struct PUME *PUMES;
	UNSHORT Spell_areas;
	UNSHORT Current_spell_area;

	PUMES = PUM->PUME_list;

	Descriptor = (struct Item_descriptor *) PUM->Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is the item undroppable ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes */
		PUMES[0].Flags |= PUME_BLOCKED;
		PUMES[0].Blocked_message_nr = 193;
	}
	else
	{
		/* No */
		PUMES[0].Flags &= ~PUME_BLOCKED;
	}

	/* Is the item broken & not cheating ? */
	if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
	{
		/* Yes -> Disable all item uses */
		PUMES[1].Flags |= PUME_BLOCKED;
		PUMES[1].Blocked_message_nr = 191;
		PUMES[3].Flags |= PUME_BLOCKED;
		PUMES[3].Blocked_message_nr = 191;
		PUMES[4].Flags |= PUME_BLOCKED;
		PUMES[4].Blocked_message_nr = 191;
		PUMES[5].Flags |= PUME_BLOCKED;
		PUMES[5].Blocked_message_nr = 191;
	}
	else
	{
		/* Yes -> Right class to use this item ? */
		if (!(Item_data->Class_use & (1 << Get_class(Active_char_handle))))
		{
			/* No -> Disable all item uses */
			PUMES[1].Flags |= PUME_BLOCKED;
			PUMES[1].Blocked_message_nr = 185;
			PUMES[3].Flags |= PUME_BLOCKED;
			PUMES[3].Blocked_message_nr = 185;
			PUMES[4].Flags |= PUME_BLOCKED;
			PUMES[4].Blocked_message_nr = 185;
			PUMES[5].Flags |= PUME_BLOCKED;
			PUMES[5].Blocked_message_nr = 185;
		}
		else
		{
			/* Yes -> Is it a spell scroll ? */
			if (Item_data->Type == SPELL_SCROLL_IT)
			{
				/* Yes -> Spell learning may be possible */
		 		PUMES[1].Flags &= ~PUME_ABSENT;

				/* Right spell class ? */
				if (Spell_class_known(Active_char_handle, Item_data->Spell_class))
				{
					/* Yes -> Does the active character already know this spell ? */
					if (Spell_known(Active_char_handle, Item_data->Spell_class,
					 Item_data->Spell_nr))
					{
						/* Yes -> Block PUM entry */
						PUMES[1].Flags |= PUME_BLOCKED;
						PUMES[1].Blocked_message_nr = 188;
					}
					else
					{
						/* No -> Can learn spell */
						PUMES[1].Flags &= ~PUME_BLOCKED;
					}
				}
				else
				{
					/* No -> Block PUM entry */
					PUMES[1].Flags |= PUME_BLOCKED;
					PUMES[1].Blocked_message_nr = 187;
				}
			}
			else
			{
				/* No -> Cannot learn spell */
				PUMES[1].Flags |= PUME_ABSENT;
			}

			/* Can it be drunk ? */
			if (Item_data->Type == DRINK_IT)
			{
				/* Yes -> Drinking is possible */
		 		PUMES[3].Flags &= ~(PUME_ABSENT | PUME_BLOCKED);
			}
			else
			{
				/* No -> Cannot drink */
				PUMES[3].Flags |= PUME_ABSENT;
			}

			/* Does it contain a spell / not in combat ? */
			if (Item_data->Spell_nr && !In_Combat)
			{
				/* Yes -> Activating spell may be possible */
				PUMES[4].Flags &= ~PUME_ABSENT;

				/* Any charges left ? */
				if (Packet->Charges)
				{
					/* Yes -> Get spell data */
					Spell_areas = Get_spell_areas((UNSHORT) Item_data->Spell_class,
					 (UNSHORT) Item_data->Spell_nr);
					Current_spell_area = Get_spell_area();

					/* Right spell area ? */
					if (Spell_areas & (1 << Current_spell_area))
					{
						/* Yes -> Can activate spell */
						PUMES[4].Flags &= ~PUME_BLOCKED;
					}
					else
					{
						/* No -> Block PUM entry */
						PUMES[4].Flags |= PUME_BLOCKED;
						PUMES[4].Blocked_message_nr = 186;
					}
				}
				else
				{
					/* No -> Block PUM entry */
					PUMES[4].Flags |= PUME_BLOCKED;
					PUMES[4].Blocked_message_nr = 184;
				}
			}
			else
			{
				/* No -> Cannot activate spell */
				PUMES[4].Flags |= PUME_ABSENT;
			}

			/* Can it be read ? */
			if (Item_data->Type == TEXT_SCROLL_IT)
			{
				/* Yes -> Reading is possible */
		 		PUMES[5].Flags &= ~(PUME_ABSENT | PUME_BLOCKED);
			}
			else
			{
				/* No -> Cannot read */
				PUMES[5].Flags |= PUME_ABSENT;
			}

			/* Is a special item ? */
			if (Item_data->Type == SPECIAL_IT)
			{
				/* Yes -> Activating may be possible */
		 		PUMES[6].Flags &= ~PUME_ABSENT;

				/* Does the party already have this special item ? */
				if (PARTY_DATA.Special_item_flags & (1 << Item_data->Misc[0]))
				{
					/* Yes -> Block PUM entry */
		 			PUMES[6].Flags |= PUME_BLOCKED;
					PUMES[6].Blocked_message_nr = 132;
				}
				else
				{
					/* No -> Can activate item */
		 			PUMES[6].Flags &= ~PUME_BLOCKED;
				}
			}
			else
			{
				/* No -> Cannot activate */
				PUMES[6].Flags |= PUME_ABSENT;
			}
		}
	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Drop_inventory_item
 * FUNCTION  : Drop an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 05.01.95 15:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Drop_inventory_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT Quantity;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* More than one item in the packet ? */
	Quantity = 1;
	if (Packet->Quantity > 1)
	{
		/* Yes -> Ask the player how many should be dropped */
		Quantity = Input_number_with_symbol(1, 0, (SILONG) Packet->Quantity,
		 System_text_ptrs[192], 16, 16, Item_graphics_handle,
		 Item_data->Pic_nr * 256);
	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Is quantity zero ? */
	if (Quantity)
	{
		/* No -> Are you sure ? */
		if (Boolean_requester(System_text_ptrs[62]))
		{
			/* Yes -> Drop */
			Drop_items(PUM_source_object_handle, Quantity);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Learn_spell_inventory_item
 * FUNCTION  : Learn a spell from an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 05.01.95 15:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Learn_spell_inventory_item(UNLONG Data)
{
	struct Character_data *Char;
	struct Spell_data *Spell_data;
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

	Descriptor = (struct Item_descriptor *) Data;

	Char = (struct Character_data *) MEM_Claim_pointer(Active_char_handle);

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Get spell data */
	Spell_data = (struct Spell_data *) MEM_Claim_pointer(Spell_data_handle);
	Spell_data += (Item_data->Spell_class * SPELLS_PER_CLASS) +
	 Item_data->Spell_nr - 1;

	/* Is the spell's level too high ? */
	if (Spell_data->Level > (Get_level(Active_char_handle) / 5))
	{
		/* Yes -> Apologise */
		Do_text_window(System_text_ptrs[189]);
	}
	else
	{
		/* No -> Remove item */
		Remove_item(Descriptor->Slots_handle, Descriptor->Item_slot_index, 1);

		/* Learn spell */
		Char->xKnown_spells[Item_data->Spell_class] |=
		 (1 << Item_data->Spell_nr);

		/* Set initial spell strength */
		Char->xSpell_capabilities[Item_data->Spell_class][Item_data->Spell_nr-1]
		 = 50 * 100;

		/* Tell the good news */
		Do_text_window(System_text_ptrs[190]);

		/* Let object explode */

	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);
	MEM_Free_pointer(Active_char_handle);
}

/*
;*****************************************************************************
; [ Learning a spell succeeded ]
;   IN : d0 - Slot number (0...23) (.w)
; All registers are restored
;*****************************************************************************
Learn_spell_succeeded:
	movem.l	d0/d1/a0/a1,-(sp)
	sub.w	Scroll_bar_result,d0	; Adjust
	addq.w	#1,d0
	jsr	Display_object		; Update object display
	jsr	Update_screen
	move.l	Object_pos_list,a0		; Get slot coordinates
	subq.w	#1,d0
	lsl.w	#2,d0
	add.w	d0,a0
	lea.l	Dissolve_HDOB,a1		; Initialize HDOB
	move.w	(a0)+,HDOB_drawX(a1)
	move.w	(a0)+,HDOB_drawY(a1)
	move.l	a1,a0			; Install HDOB
	jsr	Add_HDOB
	moveq.l	#11,d0			; Poof !
	moveq.l	#3,d1
	lea.l	Dissolve,a0
	jsr	Animate_HDOB
	move.l	a1,a0			; Destroy HDOB
	jsr	Remove_HDOB
	jsr	Update_screen
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Examine_inventory_item
 * FUNCTION  : Examine an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 05.01.95 15:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Examine_inventory_item(UNLONG Data)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Drink_inventory_item
 * FUNCTION  : Drink an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 05.01.95 15:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Drink_inventory_item(UNLONG Data)
{
	static struct Event_action Action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		USE_ITEM_ACTION, 0, 0,
		NULL, NULL, NULL			// Use_item
	};

	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *)
	 (MEM_Claim_pointer(Descriptor->Slots_handle) + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Set item index and type */
	Action.Action_value = Packet->Index;
	Action.Action_extra = Item_data->Type;

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Check events */
	Action.Actor_index = PARTY_DATA.Active_member;
	Perform_action(&Action);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Activate_spell_inventory_item
 * FUNCTION  : Activate a spell in an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 02.05.95 11:10
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Activate_spell_inventory_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	UNSHORT Source_item_slot_index;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item slot index */
	Source_item_slot_index = Descriptor->Item_slot_index;

	/* Backpack item ? (yuk) */
	if (Descriptor->Slots_offset == Backpack_items_offset)
	{
		/* Yes -> Adjust index */
		Source_item_slot_index += ITEMS_ON_BODY;
	}

	/* Cast spell */
	Cast_spell(Inventory_member, Source_item_slot_index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Read_inventory_item
 * FUNCTION  : Read an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 05.01.95 15:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Read_inventory_item(UNLONG Data)
{
	static struct Event_action Action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		USE_ITEM_ACTION, 0, 0,
		NULL, NULL, NULL				// Use_item
	};

	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Set item index and type */
	Action.Action_value = Packet->Index;
	Action.Action_extra = Item_data->Type;

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Check events */
	Action.Actor_index = PARTY_DATA.Active_member;
	Perform_action(&Action);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Activate_inventory_item
 * FUNCTION  : Activate an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.05.95 11:55
 * LAST      : 04.05.95 11:55
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Activate_inventory_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Activate special item */
	PARTY_DATA.Special_item_flags |= (1 << Item_data->Misc[0]);

	/* Remove used item */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_items
 * FUNCTION  : Drop items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.02.95 11:19
 * LAST      : 03.03.95 12:07
 * INPUTS    : UNSHORT Item_slot_handle - Handle of item slot object.
 *             UNSHORT Quantity - Number of items that should be dropped.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function currently only works for item lists belonging
 *              to characters.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_items(UNSHORT Item_slot_handle, UNSHORT Quantity)
{
	struct HDOB HDOB;

	struct Item_data *Item_data;

	struct Item_list_object *Item_list;

	struct Item_slot_object *Item_slot;

	struct Item_packet Packet;

	SISHORT Slot_X;
	SISHORT Slot_Y;

	UNSHORT HDOB_nr;
	UNSHORT Slot_index;
	UNSHORT i;

	Item_slot = (struct Item_slot_object *) Get_object_data(Item_slot_handle);

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

	/* Get packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Get packet index */
	Slot_index = Item_slot->Number;
	if (Item_list->Scroll_bar_object)
	{
		Slot_index += (UNSHORT) Execute_method(Item_list->Scroll_bar_object,
		 GET_METHOD, NULL);
	}

	/* Get real packet index */
	if (Item_list->Type == CHAR_BACKPACK_INV_TYPE)
	{
		Slot_index += ITEMS_ON_BODY;
	}

	/* Get item data */
	Item_data = Get_item_data(&Packet);

	/* Get item slot coordinates (for HDOB) */
	Slot_X = Item_slot->Object.Rect.left;
	Slot_Y = Item_slot->Object.Rect.top;

	/* Initialize HDOB data */
	BASEMEM_FillMemByte((UNBYTE *) &HDOB, sizeof(struct HDOB), 0);

	HDOB.Draw_mode = HDOB_MASK;
	HDOB.X = Slot_X;
	HDOB.Y = Slot_Y;
	HDOB.Width = 16;
	HDOB.Height = 16;
	HDOB.Nr_frames = (UNSHORT) Item_data->Nr_frames;
	HDOB.Graphics_handle = Item_graphics_handle;
	HDOB.Graphics_offset = 256 * Item_data->Pic_nr;

	/* Add HDOB */
	HDOB_nr = Add_HDOB(&HDOB);

	for (i=0;i<Quantity;i++)
	{
		/* Try to take an item */
		if (Take_item_from_character(Item_list->Slots_handle,
		 Slot_index, 1, NULL))
		{
			/* Success -> Redraw object */
			Execute_method(Item_slot_handle, DRAW_METHOD, NULL);

			/* Update display */
			Update_display();
			Update_input();
			Switch_screens();

			/* Only show 10 dropping items */
			if (i < 10)
			{
				/* Reset HDOB coordinates */
				Set_HDOB_position(HDOB_nr, Slot_X, Slot_Y);

				/* Show dropping item */
				Drop_HDOB(HDOB_nr);
			}
		}
		else
			break;
	}

	/* Remove HDOB */
	Delete_HDOB(HDOB_nr);

	Free_item_data();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Auto_move_packet_to_backpack
 * FUNCTION  : Automatically move an item packet to a character's backpack.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 15:34
 * LAST      : 11.01.95 15:34
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             struct Item_packet *Source - Source packet.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will remove items from the source packet, and
 *              destroy the source packet if the quantity reaches zero.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Auto_move_packet_to_backpack(MEM_HANDLE Char_handle, struct Item_packet *Source)
{
	struct Character_data *Char;
	struct Item_packet *Target;
	struct Item_data *Item_data;
	UNSHORT i, Fit;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source);

		/* Is this a multiple item ? */
		if (Item_data->Flags & MULTIPLE)
		{
			/* Yes -> Search for other slots with this item */
			for (i=0;i<ITEMS_PER_CHAR;i++)
			{
				Target = &(Char->Backpack_items[i]);

				/* Same item / not full ? */
				if ((Target->Index == Source->Index) && (Target->Quantity < 99))
				{
					/* Calculate how many items will fit in the slot */
					Fit = 99 - Target->Quantity;

					/* Enough ? */
					if (Fit > Source->Quantity)
					{
						/* Yes -> Clip */
						Fit = Source->Quantity;
					}

					/* Add items to target packet */
					Target->Quantity += Fit;

					/* Increase weight */
//					Char->Carried_weight += (Fit * Item_data->Weight);

					/* Remove items from source packet */
					Source->Quantity -= Fit;

					/* Anything left ? */
					if (!Source->Quantity)
					{
						/* No -> Exit */
						break;
					}
				}
			}

			/* Anything left in source packet ? */
			if (Source->Quantity)
			{
				/* Yes -> Find a free slot */
				for (i=0;i<ITEMS_PER_CHAR;i++)
				{
					Target = &(Char->Backpack_items[i]);

					/* Free ? */
					if (Packet_empty(Target))
					{
						/* Yes -> Clear target packet */
						Clear_packet(Target);

						/* Put items in target packet */
						Target->Index = Source->Index;
						Target->Quantity = Source->Quantity;

						/* Increase weight */
//						Char->Carried_weight += (Source->Quantity * Item_data->Weight);

						/* Remove items from source packet */
						Source->Quantity = 0;

						break;
					}
				}
			}
		}
		else
		{
			/* No -> Find a free slot */
			for (i=0;i<ITEMS_PER_CHAR;i++)
			{
				Target = &(Char->Backpack_items[i]);

				/* Free ? */
				if (Packet_empty(Target))
				{
					/* Yes -> Clear target packet */
					Clear_packet(Target);

					/* Put item in target packet */
					Target->Index = Source->Index;
					Target->Quantity = 1;

					/* Increase weight */
//					Char->Carried_weight += Item_data->Weight;

					/* Remove item from source packet */
					Source->Quantity -= 1;

					/* Exit if all items have been put away */
					if (!Source->Quantity)
						break;
				}
			}
		}
		Free_item_data();
	}

	MEM_Free_pointer(Char_handle);

	/* Anything left in source packet ? */
	if (!Source->Quantity)
	{
		/* No -> Destroy packet */
		Clear_packet(Source);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_target_slot_for_item
 * FUNCTION  : Find the target slot for an item in a character's inventory.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 16:30
 * LAST      : 01.03.95 16:30
 * INPUTS    : struct Item_data *Source_item - Pointer to source item data.
 * RESULT    : UNSHORT : Target slot index (0xFFFF for no slot).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Find_target_slot_for_item(struct Item_data *Source_item)
{
	struct Character_data *Char;
	UNSHORT Target_slot_index = 0xFFFF;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Inventory_char_handle);

	/* Is this a special case ? */
	if (Source_item->Body_place == HAND_OR_TAIL)
	{
		/* Yes -> Left hand free ? */
		if (Packet_empty(&(Char->Body_items[LEFT_HAND - 1])))
		{
			/* Yes -> Select left hand */
			Target_slot_index = LEFT_HAND;
		}
		else
		{
			/* No -> Is this an Iskai / tail free ? */
			if ((Get_race(Inventory_char_handle) == ISKAI_RACE) &&
			 Packet_empty(&(Char->Body_items[TAIL - 1])))
			{
				/* Yes -> Select tail */
				Target_slot_index = TAIL;
			}
		}
	}
	else
	{
		/* No -> Is the target slot free ? */
		if (Packet_empty(&(Char->Body_items[Source_item->Body_place - 1])))
		{
			/* Yes -> Select */
			Target_slot_index = Source_item->Body_place;
		}
		else
		{
			/* No -> Another special case ? */
			if (Source_item->Body_place == RIGHT_FINGER)
			{
				/* Yes -> Left ringfinger free ? */
				if (Packet_empty(&(Char->Body_items[LEFT_FINGER - 1])))
				{
					/* Yes -> Select left ringfinger */
					Target_slot_index = LEFT_FINGER;
				}
			}
		}
	}

	return(Target_slot_index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Take_item_from_character
 * FUNCTION  : Take an item out of a character's inventory.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 12:28
 * LAST      : 01.03.95 12:28
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 *             UNSHORT Slot_index - Slot index (1...33).
 *             UNSHORT Quantity - Quantity that must be removed.
 *             struct Item_packet *Target_packet - Target packet.
 * RESULT    : BOOLEAN : Was the item taken?
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Take_item_from_character(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity, struct Item_packet *Target_packet)
{
	static struct Event_action Take_item_from_char_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		TAKE_ITEM_FROM_CHAR_ACTION, 0, 0,
		Do_take_item_from_char, NULL, NULL
	};

	struct Take_item_from_char_data Action_data;
	struct Character_data *Char;
	struct Item_packet *Source_packet;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get source packet data */
	if (Slot_index <= ITEMS_ON_BODY)
		Source_packet = &(Char->Body_items[Slot_index - 1]);
	else
		Source_packet = &(Char->Backpack_items[Slot_index - ITEMS_ON_BODY - 1]);

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Build data for event action */
		Action_data.Char_handle = Char_handle;
		Action_data.Slot_index = Slot_index;
		Action_data.Quantity = Quantity;
		Action_data.Target_packet = Target_packet;

		/* Build event action data */
		Take_item_from_char_action.Actor_index = Inventory_member;
		Take_item_from_char_action.Action_value = Source_packet->Index;
		Take_item_from_char_action.Action_extra = (UNSHORT) Item_data->Type;
		Take_item_from_char_action.Action_data = &Action_data;

		/* Check events */
		Result = Perform_action(&Take_item_from_char_action);

		Free_item_data();
	}
	MEM_Free_pointer(Char_handle);

	return(Result);
}

BOOLEAN
Do_take_item_from_char(struct Event_action *Action)
{
	struct Take_item_from_char_data *Data;
	struct Character_data *Char;
	struct Item_packet *Source_packet;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Get action data */
	Data = (struct Take_item_from_char_data *) Action->Action_data;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Data->Char_handle);

	/* Get source packet data */
	if (Data->Slot_index <= ITEMS_ON_BODY)
		Source_packet = &(Char->Body_items[Data->Slot_index - 1]);
	else
		Source_packet = &(Char->Backpack_items[Data->Slot_index -
		 ITEMS_ON_BODY - 1]);

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes */
		Result = TRUE;

		/* Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Is it a body item ? */
		if (Data->Slot_index <= ITEMS_ON_BODY)
		{
			/* Yes -> Is the item cursed ? */
			if (Source_packet->Flags & CURSED)
			{
				/* Yes -> Apologise to the player */
				Set_permanent_message_nr(63);

				Result = FALSE;
			}
			else
			{
				/* No -> Are we in combat & is this item un-equippable in combat ? */
				if (In_Combat && (Item_data->Flags & COMBAT_EQUIP))
				{
					/* Yes -> Apologise to the player */
					Set_permanent_message_nr(65);

					Result = FALSE;
				}
			}
		}

		/* Continue ? */
		if (Result)
		{
			/* Yes -> Was a target packet given ? */
			if (Data->Target_packet)
			{
				/* Yes -> Put item in target packet */
				memcpy(Data->Target_packet, Source_packet,
				 sizeof(struct Item_packet));
				Data->Target_packet->Quantity = Data->Quantity;
			}

			/* Remove item(s) */
			Remove_item(Data->Char_handle, Data->Slot_index, Data->Quantity);
		}

		Free_item_data();
	}
	MEM_Free_pointer(Data->Char_handle);

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_item_in_character
 * FUNCTION  : Put an item in a character's inventory.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.03.95 15:24
 * LAST      : 02.03.95 15:24
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 *             UNSHORT Slot_index - Slot index (1...33).
 *             UNSHORT Quantity - Quantity that must be added.
 *             struct Item_packet *Target_packet - Source packet.
 * RESULT    : BOOLEAN : Was the item put?
 * BUGS      : No known.
 * NOTES     : - This function assumes the target slot is empty when it is
 *              a body slot.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Put_item_in_character(MEM_HANDLE Char_handle, UNSHORT Slot_index,
 UNSHORT Quantity, struct Item_packet *Source_packet)
{
	static struct Event_action Put_item_in_char_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		PUT_ITEM_IN_CHAR_ACTION, 0, 0,
		Do_put_item_in_char, NULL, NULL
	};

	struct Put_item_in_char_data Action_data;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Build data for event action */
		Action_data.Char_handle = Char_handle;
		Action_data.Slot_index = Slot_index;
		Action_data.Quantity = Quantity;
		Action_data.Source_packet = Source_packet;

		/* Build event action data */
		Put_item_in_char_action.Actor_index = Inventory_member;
		Put_item_in_char_action.Action_value = Source_packet->Index;
		Put_item_in_char_action.Action_extra = (UNSHORT) Item_data->Type;
		Put_item_in_char_action.Action_data = &Action_data;

		/* Check events */
		Result = Perform_action(&Put_item_in_char_action);

		Free_item_data();
	}

	return(Result);
}

BOOLEAN
Do_put_item_in_char(struct Event_action *Action)
{
	struct Put_item_in_char_data *Data;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Get action data */
	Data = (struct Put_item_in_char_data *) Action->Action_data;

	/* Anything in the source packet ? */
	if (!Packet_empty(Data->Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Data->Source_packet);

		/* Is it a body item ? */
		if (Data->Slot_index <= ITEMS_ON_BODY)
		{
			/* Yes -> Try to equip the item */
			Result = FALSE;

			/* Are we in combat & is the source item un-equippable in combat ? */
			if (In_Combat && (Item_data->Flags & COMBAT_EQUIP))
			{
				/* Yes -> Apologise to the player */
				Set_permanent_message_nr(67);
			}
			else
			{
				/* No -> Is the source item broken ? */
				if (Data->Source_packet->Flags & BROKEN_ITEM)
				{
					/* Yes -> Apologise to the player */
					Set_permanent_message_nr(194);
				}
				else
				{
					/* No -> Right class to equip this item ? */
					if (!(Item_data->Class_use & (1 << Get_class(Data->Char_handle))))
					{
						/* No -> Apologise to the player */
						Set_permanent_message_nr(195);
					}
					else
					{
						/* Yes -> Right sex to equip this item ? */
						if (!(Item_data->Sex_use & (1 << Get_sex(Data->Char_handle))))
						{
							/* No -> Apologise to the player */
							Set_permanent_message_nr(196);
						}
						else
						{
							/* Yes -> Enough hands free ? */
							if ((Get_nr_occupied_hands(Data->Char_handle) +
							 Item_data->Hand_use) > 2)
							{
								/* No -> Apologise to the player */
								Set_permanent_message_nr(197);
							}
							else
							{
								/* Yes -> Yay! It can be done! */
								Result = TRUE;
							}
						}
					}
				}
			}

			/* Continue ? */
			if (Result)
			{
				/* Yes -> Add items to body */
				Add_item_to_body(Data->Char_handle, Data->Source_packet,
				 Data->Slot_index);
			}
		}
		else
		{
			/* No -> Add items to backpack */
			Add_item_to_backpack(Data->Char_handle, Data->Source_packet,
			 Data->Slot_index - ITEMS_ON_BODY, Data->Quantity);

			/* Everything worked out fine */
			Result = TRUE;
		}

		Free_item_data();
	}

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_item
 * FUNCTION  : Remove an item from a character's inventory.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 14:40
 * LAST      : 11.01.95 14:40
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             UNSHORT Slot_index - Slot index (1...33).
 *             UNSHORT Quantity - Quantity that must be removed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The quantity is ignored for body items.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_item(MEM_HANDLE Char_handle, UNSHORT Slot_index, UNSHORT Quantity)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT i, Skill_nr;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get packet address */
	if (Slot_index <= ITEMS_ON_BODY)
	{
		Packet = &(Char->Body_items[Slot_index - 1]);
	}
	else
	{
		Packet = &(Char->Backpack_items[Slot_index - ITEMS_ON_BODY - 1]);
	}

	/* Anything in packet ? */
	if (!Packet_empty(Packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Packet);

		/* Body or backpack ? */
		if (Slot_index <= ITEMS_ON_BODY)
		{
			/* Adjust hands occupied */
//			Char->Hands_occupied -= Item_data->Hand_use;

			/* Execute skill taxes */
			for (i=0;i<2;i++)
			{
				/* Any skill tax ? */
				Skill_nr = Item_data->Item_skills[i];
				if (Skill_nr)
				{
					/* Yes -> Tax */
					Char->Skills[Skill_nr - 1].Magic -= Item_data->Malus[i];
				}
			}

			/* Cursed item ? */
			if (Packet->Flags & CURSED)
			{
				/* Yes -> Add boni */
				Positive_item_boni(Char, Item_data);
			}
			else
			{
				/* No -> Remove boni */
				Negative_item_boni(Char, Item_data);
			}

			/* Decrease weight */
//			Char->Carried_weight -= Item_data->Weight;

			/* Reduce quantity */
			Packet->Quantity -= 1;

			/* Any left ? */
			if (!Packet->Quantity)
			{
				/* No -> Destroy packet */
				Clear_packet(Packet);
			}
		}
		else
		{
			/* Backpack */
			if (Packet->Quantity < Quantity)
				Quantity = Packet->Quantity;

			/* Reduce quantity */
			Packet->Quantity -= Quantity;

			/* Any left ? */
			if (!Packet->Quantity)
			{
				/* No -> Destroy packet */
				Clear_packet(Packet);
			}

			/* Decrease weight */
//			Char->Carried_weight -= (Quantity * Item_data->Weight);
		}

		Free_item_data();
	}

	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_item_to_body
 * FUNCTION  : Add an item to a character's body items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 15:53
 * LAST      : 11.01.95 15:53
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             struct Item_packet *Source_packet - Source packet.
 *             UNSHORT Item_slot_index - Target slot number (1...9).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will remove ONE item from the source packet,
 *              and destroy the source packet if the quantity reaches zero.
 *             - This function assumes the target slot is empty.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_item_to_body(MEM_HANDLE Char_handle, struct Item_packet *Source_packet,
 UNSHORT Item_slot_index)
{
	struct Character_data *Char;
	struct Item_packet *Target;
	struct Item_data *Item_data;
	UNSHORT i, Skill_nr;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Get target packet */
		Target = &(Char->Body_items[Item_slot_index - 1]);

		/* Copy packet */
		memcpy(Target, Source_packet, sizeof(struct Item_packet));

		/* Set quantity */
		Target->Quantity = 1;

		/* Adjust hands occupied */
//		Char->Hands_occupied += Item_data->Hand_use;

		/* Execute skill taxes */
		for (i=0;i<2;i++)
		{
			/* Any skill tax ? */
			Skill_nr = Item_data->Item_skills[i];
			if (Skill_nr)
			{
				/* Yes -> Tax */
				Char->Skills[Skill_nr - 1].Magic -= Item_data->Malus[i];
			}
		}

		/* Cursed item ? */
		if (Source_packet->Flags & CURSED)
		{
			/* Yes -> Remove boni */
			Negative_item_boni(Char, Item_data);
		}
		else
		{
			/* No -> Add boni */
			Positive_item_boni(Char, Item_data);
		}

		/* Increase weight */
//		Char->Carried_weight += Item_data->Weight;

		/* Remove item from source packet */
		Source_packet->Quantity -= 1;

		Free_item_data();
	}
	MEM_Free_pointer(Char_handle);

	/* Anything left in source packet ? */
	if (!Source_packet->Quantity)
	{
		/* No -> Destroy packet */
		Clear_packet(Source_packet);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Positive_item_boni
 * FUNCTION  : Execute positive item boni.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 16:05
 * LAST      : 11.01.95 16:05
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_data *Item_data - Pointer to item data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is an internal function.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Positive_item_boni(struct Character_data *Char, struct Item_data *Item_data)
{
	UNSHORT Number;

	/* LP max bonus */
	Char->Life_points_magic += (UNSHORT) Item_data->LP_max;

	/* SP max bonus */
	Char->Spell_points_magic += (UNSHORT) Item_data->SP_max;

	/* Any attribute bonus ? */
	Number = Item_data->Attribute;
	if (Number)
	{
		/* Yes -> Execute bonus */
		Char->Attributes[Number - 1].Magic += (UNSHORT) Item_data->Attribute_normal;
	}

	/* Any skill bonus ? */
	Number = Item_data->Skill;
	if (Number)
	{
		/* Yes -> Execute bonus */
		Char->Skills[Number - 1].Magic += (UNSHORT) Item_data->Skill_normal;
	}

	/* Damage bonus */
//	Char->Damage_magic += (UNSHORT) Item_data->Damage_pts;

	/* Protection bonus */
//	Char->Protection_magic += (UNSHORT) Item_data->Protection_pts;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Negative_item_boni
 * FUNCTION  : Execute negative item boni.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 16:12
 * LAST      : 11.01.95 16:12
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_data *Item_data - Pointer to item data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is an internal function.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Negative_item_boni(struct Character_data *Char, struct Item_data *Item_data)
{
	UNSHORT Number;

	/* LP max bonus */
	Char->Life_points_magic -= (UNSHORT) Item_data->LP_max;

	/* SP max bonus */
	Char->Spell_points_magic -= (UNSHORT) Item_data->SP_max;

	/* Any attribute bonus ? */
	Number = Item_data->Attribute;
	if (Number)
	{
		/* Yes -> Execute bonus */
		Char->Attributes[Number - 1].Magic -= (UNSHORT) Item_data->Attribute_normal;
	}

	/* Any skill bonus ? */
	Number = Item_data->Skill;
	if (Number)
	{
		/* Yes -> Execute bonus */
		Char->Skills[Number - 1].Magic -= (UNSHORT) Item_data->Skill_normal;
	}

	/* Damage bonus */
//	Char->Damage_magic -= (UNSHORT) Item_data->Damage_pts;

	/* Protection bonus */
//	Char->Protection_magic -= (UNSHORT) Item_data->Protection_pts;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_item_to_backpack
 * FUNCTION  : Add an item to a character's backpack items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.03.95 16:04
 * LAST      : 02.03.95 16:04
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             struct Item_packet *Source_packet - Source packet.
 *             UNSHORT Item_slot_index - Target item slot index (1...24).
 *             UNSHORT Quantity - Number of items to take from source packet.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will destroy the source packet if the quantity
 *              reaches zero.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_item_to_backpack(MEM_HANDLE Char_handle, struct Item_packet *Source_packet,
 UNSHORT Item_slot_index, UNSHORT Quantity)
{
	struct Character_data *Char;
	struct Item_packet *Target_packet;
//	struct Item_data *Item_data;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
//		Item_data = Get_item_data(Source_packet);

		/* Get target packet */
		Target_packet = &(Char->Backpack_items[Item_slot_index - 1]);

		/* Does target slot contain the same item(s) ? */
		if (Source_packet->Index == Target_packet->Index)
		{
			/* Yes -> Add items to target slot */
			Target_packet->Quantity += Quantity;
		}
		else
		{
			/* No -> Move items to target slot */
			memcpy((UNBYTE *) Target_packet, (UNBYTE *) Source_packet,
			 sizeof(struct Item_packet));
			Target_packet->Quantity = Quantity;
		}

		/* Increase weight */
//		Char->Carried_weight += (Quantity * Item_data->Weight);

		/* Remove items from source packet */
		Source_packet->Quantity -= Quantity;

//		Free_item_data();
	}
	MEM_Free_pointer(Char_handle);

	/* Anything left in source packet ? */
	if (!Source_packet->Quantity)
	{
		/* No -> Destroy packet */
		Clear_packet(Source_packet);
	}
}

