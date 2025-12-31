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
#include <PRTLOGIC.H>
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
#include <ITMLOGIC.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <MAGIC.H>
#include <INPUTNR.H>
#include <BOOLREQ.H>
#include <TEXTWIN.H>
#include <PLACES.H>
#include <MERCHANT.H>
#include <QUERMOD.H>
#include <EXAMITEM.H>
#include <MEMBRSEL.H>

/* defines */

#define MAX_DROPPING_ITEMS	(5)

/* structure definitions */

/* prototypes */

/* Body item list methods */
UNLONG Init_Body_item_list_object(struct Object *Object,
 union Method_parms *P);
UNLONG Draw_Body_item_list_object(struct Object *Object,
 union Method_parms *P);

/* Body item slot methods (replacing normal item slot methods) */
UNLONG Draw_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Feedback_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Highlight_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);

UNLONG Left_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);
UNLONG Drop_Body_item_slot_object(struct Object *Object,
 union Method_parms *P);

/* Body item slot support functions */
void Drag_body_item(struct Item_slot_object *Item_slot);
void Drop_on_body_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr);
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
void Drop_on_backpack_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr);

/* Inventory item pop-up menu actions */
void Inventory_item_PUM_evaluator(struct PUM *PUM);
void PUM_Drop_inventory_item(UNLONG Data);
void PUM_Learn_spell_inventory_item(UNLONG Data);
void PUM_Examine_inventory_item(UNLONG Data);
void PUM_Use_inventory_item(UNLONG Data);
void PUM_Drink_inventory_item(UNLONG Data);
void PUM_Activate_spell_inventory_item(UNLONG Data);
void PUM_Activate_inventory_item(UNLONG Data);
void PUM_Sell_inventory_item(UNLONG Data);

BOOLEAN Do_use_item(struct Event_action *Action);

/* global variables */

struct Item_descriptor Selected_item;

/* Body item list method list */
static struct Method Body_item_list_methods[] = {
	{ INIT_METHOD,				Init_Body_item_list_object },
	{ DRAW_METHOD,				Draw_Body_item_list_object },
	{ DROP_METHOD,				Drop_Body_item_slot_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_item_slot_object },
	{ TOUCHED_METHOD,			Dehighlight },
	{ 0, NULL}
};

/* Body item list object class */
struct Object_class Body_item_list_Class = {
	0, sizeof(struct Item_list_object),
	&Body_item_list_methods[0]
};

/* Body item slot method list */
static struct Method Body_item_slot_methods[] = {
	{ INIT_METHOD,				Init_Item_slot_object },
	{ DRAW_METHOD,				Draw_Body_item_slot_object },
	{ UPDATE_METHOD,			Update_Item_slot_object },
	{ FEEDBACK_METHOD,		Feedback_Body_item_slot_object },
	{ HIGHLIGHT_METHOD,		Highlight_Body_item_slot_object },
	{ POP_UP_METHOD,			Pop_up_Item_slot_object },
	{ HELP_METHOD,				Help_Item_slot_object },
	{ LEFT_METHOD,				Left_Body_item_slot_object },
	{ RIGHT_METHOD,			Right_Item_slot_object },
	{ TOUCHED_METHOD,			Touch_Item_slot_object },
	{ DROP_METHOD,				Drop_Body_item_slot_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_item_slot_object },
	{ 0, NULL}
};

/* Body item slot object class */
static struct Object_class Body_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Body_item_slot_methods[0]
};

/* Backpack item slot method list */
static struct Method Backpack_item_slot_methods[] = {
	{ INIT_METHOD,				Init_Item_slot_object },
	{ DRAW_METHOD,				Draw_Item_slot_object },
	{ UPDATE_METHOD,			Update_Item_slot_object },
	{ FEEDBACK_METHOD,		Feedback_Item_slot_object },
	{ HIGHLIGHT_METHOD,		Highlight_Item_slot_object },
	{ POP_UP_METHOD,			Pop_up_Item_slot_object },
	{ HELP_METHOD,				Help_Item_slot_object },
	{ LEFT_METHOD,				Left_Backpack_item_slot_object },
	{ DLEFT_METHOD,			DLeft_Backpack_item_slot_object },
	{ RIGHT_METHOD,			Right_Item_slot_object },
	{ TOUCHED_METHOD,			Touch_Item_slot_object },
	{ DROP_METHOD,				Drop_Backpack_item_slot_object },
	{ INQUIRE_DROP_METHOD,	Inquire_drop_item_slot_object },
	{ 0, NULL}
};

/* Backpack item slot object class */
struct Object_class Backpack_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Backpack_item_slot_methods[0]
};

/* Inventory item pop-up menu */
static struct PUME Inventory_item_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 15, PUM_Drop_inventory_item},
	{PUME_AUTO_CLOSE, 0, 17, PUM_Examine_inventory_item},
	{PUME_AUTO_CLOSE, 0, 16, PUM_Learn_spell_inventory_item},
	{PUME_AUTO_CLOSE, 0, 18, PUM_Drink_inventory_item},
	{PUME_AUTO_CLOSE, 0, 19, PUM_Activate_spell_inventory_item},
	{PUME_AUTO_CLOSE, 0, 20, PUM_Use_inventory_item},
	{PUME_AUTO_CLOSE, 0, 131, PUM_Activate_inventory_item},
	{PUME_AUTO_CLOSE, 0, 574, PUM_Sell_inventory_item},
	{PUME_AUTO_CLOSE, 0, 607, PUM_Use_inventory_item}
};

struct PUM Inventory_item_PUM = {
	9,
	NULL,
	(UNLONG) &Selected_item,
	Inventory_item_PUM_evaluator,
	Inventory_item_PUMEs
};

/* Use item event action data */
static struct Event_action Use_item_action =
{
	0,	PARTY_ACTOR_TYPE, 0,
	USE_ITEM_ACTION, 0, 0,
	Do_use_item, NULL, NULL
};

/* Positions of body item slots for each potential party member */
static UNSHORT Body_slot_positions[MAX_PARTY_CHARS][ITEMS_ON_BODY][2] = {
	/* 1 */
	{{220, 25}, {177, 16}, {205, 10}, {151, 67}, {200, 60}, {247, 67},
	 {151, 86}, {200, 155}, {247, 86}},
	/* 2 */
	{{220, 28}, {178, 15}, {205, 10}, {161, 77}, {201, 61}, {244, 77},
	 {161, 96}, {201, 149}, {244, 96}},
	/* 3 */
	{{191, 32}, {191, 13}, {153, 34}, {183, 63}, {211, 58}, {254, 63},
	 {183, 82}, {203, 158}, {254, 82}},
	/* 4 */
	{{233, 27}, {187, 12}, {247, 133}, {168, 45}, {207, 55}, {254, 65},
	 {168, 64}, {202, 159}, {254, 84}},
	/* 5 */
	{{238, 27}, {195, 13}, {205, 10}, {148, 33}, {215, 63}, {253, 82},
	 {148, 52}, {207, 158}, {253, 101}},
	/* 6 */
	{{228, 33}, {194, 13}, {205, 10}, {157, 39}, {206, 63}, {253, 54},
	 {157, 58}, {198, 159}, {253, 73}},
	/* 7 */
	{{220, 17}, {173, 13}, {205, 10}, {152, 57}, {197, 55}, {244, 57},
	 {152, 76}, {193, 157}, {244, 76}},
	/* 8 */
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	/* 9 */
	{{227, 27}, {185, 15}, {205, 10}, {151, 77}, {207, 60}, {245, 77},
	 {151, 96}, {206, 154}, {245, 96}},
	/* 10 */
	{{243, 28}, {191, 13}, {205, 10}, {157, 87}, {218, 60}, {251, 76},
	 {157, 106}, {220, 157}, {251, 95}},
	/* 11 */
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}},
	/* 12 */
	{{145, 10}, {175, 10}, {205, 10}, {145, 40}, {175, 40}, {205, 40},
	 {145, 70}, {175, 70}, {205, 70}}
};

/* Drag data */
struct Item_packet Drag_packet;
UNSHORT Drag_member;

MEM_HANDLE Drag_OPM_handle;
struct OPM Drag_OPM;

UNSHORT Drag_HDOB_nr;
struct HDOB Drag_HDOB = {
	HDOB_MASK,
	HDOB_ATTACHED,
	0, 0,
	0, 0,
	0,
	0,
	NULL,
	0,
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Body_item_list_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_object *Item_list;
	struct Item_list_OID *OID;
	struct Item_slot_OID Item_slot_OID;
	UNSHORT Char_nr, i;

	Item_list = (struct Item_list_object *) Object;
	OID = (struct Item_list_OID *) P;

	/* Copy data from OID */
	Item_list->Type			= OID->Type;
	Item_list->Slots_handle	= OID->Slots_handle;
	Item_list->Slots_offset	= OID->Slots_offset;
	Item_list->Menu			= OID->Menu;

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

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Body_item_list_object
 * FUNCTION  : Draw method of Body item list object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 12:20
 * LAST      : 25.09.95 16:45
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Body_item_list_object(struct Object *Object, union Method_parms *P)
{
	struct Gfx_header *Gfx;

	/* Clear item list */
	Restore_object_background(Object);

	/* Draw full-body picture */
	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Full_body_pic_handle);
	Put_masked_block
	(
		Current_OPM,
		Object->X,
		Object->Y - 1,
		Gfx->Width,
		Gfx->Height,
		(UNBYTE *) (Gfx + 1)
	);
	MEM_Free_pointer(Full_body_pic_handle);

	/* Draw children */
	Execute_child_methods(Object->Self,	DRAW_METHOD, NULL);

	/* Draw rectangle around body item list area */
	Draw_deep_border
	(
		Current_OPM,
		Object->X,
		Object->Y,
		Object->Rect.width,
		Object->Rect.height
	);

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Body_item_slot;

	Body_item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Erase_body_item_slot(Body_item_slot);

	/* Draw border */
	Draw_high_border
	(
		Current_OPM,
		Object->X,
		Object->Y,
		BODY_ITEM_SLOT_WIDTH,
		BODY_ITEM_SLOT_HEIGHT
	);

	/* Draw item */
	Draw_body_item_slot_item(Body_item_slot);

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Body_item_slot;

	Body_item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Erase_body_item_slot(Body_item_slot);

	/* Draw dark box */
	Draw_deep_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		BODY_ITEM_SLOT_WIDTH,
		BODY_ITEM_SLOT_HEIGHT
	);

	/* Draw item */
	Draw_body_item_slot_item(Body_item_slot);

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Body_item_slot;

	Body_item_slot = (struct Item_slot_object *) Object;

	/* Clear item slot */
	Erase_body_item_slot(Body_item_slot);

	/* Draw light box */
	Draw_light_box
	(
		Current_OPM,
		Object->X,
		Object->Y,
		BODY_ITEM_SLOT_WIDTH,
		BODY_ITEM_SLOT_HEIGHT
	);

	/* Draw item */
	Draw_body_item_slot_item(Body_item_slot);

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_Body_item_slot_object
 * FUNCTION  : Drop method of Body item slot AND list object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 18:38
 * LAST      : 06.07.95 15:46
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Body_item_slot_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_body_item((struct Item_slot_object *) Object,
		 P->Drag_drop_data_ptr);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inquire_drop_item_slot_object
 * FUNCTION  : Inquire drop method of Body and Backpack item slot AND Body
 *              item list object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 15:49
 * LAST      : 06.07.95 15:49
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Inquire_drop_item_slot_object(struct Object *Object,
 union Method_parms *P)
{
	struct Drag_drop_data *Drag_drop_data_ptr;
	UNSHORT Data_ID;

	/* Get drag & drop data */
	Drag_drop_data_ptr = P->Drag_drop_data_ptr;

	/* Get drag & drop data ID */
	Data_ID = Drag_drop_data_ptr->Data_ID;

	/* Right data type ? */
	if ((Data_ID == BODY_ITEM_DD_DATA_ID) ||
	 (Data_ID == BACKPACK_ITEM_DD_DATA_ID) ||
	 (Data_ID == CHEST_ITEM_DD_DATA_ID) ||
	 (Data_ID == APRES_ITEM_DD_DATA_ID))
	{
		/* Yes */
		return 1;
	}
	else
	{
		/* No */
		return 0;
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
 * LAST      : 29.09.95 12:51
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
  	struct Object *Object;

	/* Redraw body item list (!) */
	Execute_method(Item_slot->Object.Parent, DRAW_METHOD, NULL);

	/* Initialize item dragging */
	Init_drag(ITEM_SLOT_INNER_WIDTH, ITEM_SLOT_INNER_HEIGHT, MAX_ITEM_FRAMES);

	/* Build graphics */
	Build_item_graphics();

	/* Add HDOB */
	Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

	/* Pick mouse pointer */
	Push_mouse(&(Mouse_pointers[PICK_MPTR]));

	/* In merchant screen ? */
	if (In_Merchant)
	{
		/* Yes -> Keep the mouse in the right inventory area */
		Object = Get_object_data(InvRight_object);
		Push_MA(&(Object->Rect));
	}

	/* Store member index */
	Drag_member = Inventory_member;

	/* Enter drag & drop mode */
	Enter_drag_drop_mode
	(
		BODY_ITEM_DD_DATA_ID,
		Body_backpack_item_drag_abort_handler,
		&(Item_slot->Object),
		NULL
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_on_body_item
 * FUNCTION  : Drop on a body item.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 15:46
 * LAST      : 30.09.95 14:22
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to target item
 *              slot object.
 *             struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_body_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr)
{
	struct Item_slot_object *New_drag_source_item_slot;

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
		Target_slot_index = Find_target_body_slot_for_item(Source_item);

		/* Is the target slot free ? */
		if (Target_slot_index == 0xFFFF)
		{
			/* No -> Does the source packet contain only one item ? */
			if (Drag_packet.Quantity == 1)
			{
				/* Yes -> Force to preferred target */
				Target_slot_index = Source_item->Body_place;
				if (Target_slot_index == HAND_OR_TAIL)
				{
					Target_slot_index = RIGHT_HAND;
				}

				/* Try to remove target item */
				if (Take_item_from_character
				(
					Inventory_char_handle,
					Target_slot_index,
					1,
					&Swap_packet
				))
				{
					/* Success -> Move drag HDOB towards target slot */
					Move_drag_HDOB_towards_char_slot(Target_slot_index);

					/* Try to add source item to body */
					if (Put_item_in_character
					(
						Inventory_char_handle,
						Target_slot_index,
						1,
						&Drag_packet
					))
					{
						/* Success -> Redraw item slots */
						Execute_method(Body_item_list_object, DRAW_METHOD, NULL);

						/* Copy swapped packet to drag packet */
						memcpy
						(
							&Drag_packet,
							&Swap_packet,
							sizeof(struct Item_packet)
						);

						/* Re-build graphics */
						Build_item_graphics();

						/* Change HDOB data */
						Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

						/* Set new drag source */
						New_drag_source_item_slot = Find_item_slot_object(Body_item_list_object,
						 Target_slot_index);

						/* Store member index */
						Drag_member = Inventory_member;

						Enter_drag_drop_mode
						(
							BODY_ITEM_DD_DATA_ID,
							Body_backpack_item_drag_abort_handler,
							&(New_drag_source_item_slot->Object),
							NULL
						);
					}
					else
					{
						/* Failure -> Try to swap back */
						if (!Put_item_in_character
						(
							Inventory_char_handle,
							Target_slot_index,
							1,
							&Swap_packet
						))
						{
							/* Failure -> Error! */
							Error(ERROR_ITEM_NO_SWAP_BACK);
						}
					}
				}
			}
		}
		else
		{
			/* Yes -> Move drag HDOB towards target slot */
			Move_drag_HDOB_towards_char_slot(Target_slot_index);

			/* Try to add source item to body */
			if (Put_item_in_character
			(
				Inventory_char_handle,
				Target_slot_index,
				1,
				&Drag_packet
			))
			{
				/* Success -> Redraw item slots */
				Execute_method(Body_item_list_object, DRAW_METHOD, NULL);

				/* All items gone ? */
				if (!Packet_empty(&Drag_packet))
				{
					/* No -> Re-build graphics */
					Build_item_graphics();

					/* Change HDOB data */
					Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
				}
				else
				{
					/* Yes -> Exit dragging */
					Exit_drag();
					Pop_mouse();

					/* In merchant screen ? */
					if (In_Merchant)
					{
						/* Yes -> Remove mouse area */
						Pop_MA();
					}
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
 * LAST      : 25.09.95 16:46
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

	/* Draw background */
	Restore_object_background(Object);

	/* Make clipping rectangle */
	Clip.left	= Object->X;
	Clip.top		= Object->Y;
	Clip.width	= BODY_ITEM_SLOT_WIDTH;
	Clip.height	= BODY_ITEM_SLOT_HEIGHT;

	/* Install clip area */
	memcpy(&Old, &(Main_OPM.clip), sizeof(struct BBRECT));
	memcpy(&(Main_OPM.clip), &Clip, sizeof(struct BBRECT));

	/* Draw full-body picture */
	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Full_body_pic_handle);
	Put_masked_block
	(
		Current_OPM,
		Parent->X,
		Parent->Y - 1,
		Gfx->Width,
		Gfx->Height,
		(UNBYTE *) (Gfx + 1)
	);
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
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

	return 0;
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
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function won't work properly when the backpack items
 *              have a scroll-bar.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Drop_Backpack_item_slot_object(struct Object *Object, union Method_parms *P)
{
	/* Really clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Drop */
		Drop_on_backpack_item((struct Item_slot_object *) Object,
		 P->Drag_drop_data_ptr);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drag_backpack_item
 * FUNCTION  : Drag backpack items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.02.95 17:03
 * LAST      : 19.08.95 12:28
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
	struct Object *Object;

	/* Any taken ? */
	if (Quantity)
	{
		/* Yes -> Redraw item slot */
		Execute_method(Item_slot->Object.Self, DRAW_METHOD, NULL);

		/* Initialize item dragging */
		Init_drag(ITEM_SLOT_INNER_WIDTH, ITEM_SLOT_INNER_HEIGHT,
		 MAX_ITEM_FRAMES);

		/* Build graphics */
		Build_item_graphics();

		/* Add HDOB */
		Drag_HDOB_nr = Add_HDOB(&Drag_HDOB);

		/* Pick mouse pointer */
		Push_mouse(&(Mouse_pointers[PICK_MPTR]));

		/* In merchant screen ? */
		if (In_Merchant)
		{
			/* Yes -> Keep the mouse in the right inventory area */
			Object = Get_object_data(InvRight_object);
			Push_MA(&(Object->Rect));
		}

		/* Store member index */
		Drag_member = Inventory_member;

		/* Enter drag & drop mode */
		Enter_drag_drop_mode(BACKPACK_ITEM_DD_DATA_ID,
		 Body_backpack_item_drag_abort_handler, &(Item_slot->Object), NULL);
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
 * LAST      : 19.08.95 12:30
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to target item
 *              slot object.
 *             struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Drop_on_backpack_item(struct Item_slot_object *Item_slot,
 struct Drag_drop_data *Drag_drop_data_ptr)
{
	struct Item_slot_object *New_drag_source_item_slot;

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
				 Target_packet->Quantity);

				/* Move drag HDOB towards target slot */
				Move_drag_HDOB_towards_char_slot(Target_slot_index);

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

						/* Change HDOB data */
						Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
					}
					else
					{
						/* Yes -> Exit dragging */
						Exit_drag();
						Pop_mouse();

						/* In merchant screen ? */
						if (In_Merchant)
						{
							/* Yes -> Remove mouse area */
							Pop_MA();
						}
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
				Move_drag_HDOB_towards_char_slot(Target_slot_index);

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

					/* Change HDOB data */
					Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

					/* Set new drag source */
					New_drag_source_item_slot = Find_item_slot_object(Backpack_item_list_object,
					 Target_slot_index);

					/* Store member index */
					Drag_member = Inventory_member;

					Enter_drag_drop_mode(BACKPACK_ITEM_DD_DATA_ID,
					 Body_backpack_item_drag_abort_handler,
					 &(New_drag_source_item_slot->Object), NULL);
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
		Move_drag_HDOB_towards_char_slot(Target_slot_index);

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

				/* Change HDOB data */
				Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);
			}
			else
			{
				/* Yes -> Exit dragging */
				Exit_drag();
				Pop_mouse();

				/* In merchant screen ? */
				if (In_Merchant)
				{
					/* Yes -> Remove mouse area */
					Pop_MA();
				}
			}
		}
	}
	/* Free item packet */
	Free_slot_packet(Item_slot);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_item_graphics
 * FUNCTION  : Build item graphics for dragging.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 17:22
 * LAST      : 10.09.95 22:52
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

	Nr_frames = max(1, min(MAX_ITEM_FRAMES, Item_data->Nr_frames));

	Free_item_data();

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

	/* Set HDOB hotspot */
	Drag_HDOB.X = 3;
	Drag_HDOB.Y = 3;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_drag
 * FUNCTION  : Initialize dragging HDOB & OPM.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.95 17:20
 * LAST      : 07.07.95 12:01
 * INPUTS    : UNSHORT Width - Width of HDOB / OPM.
 *             UNSHORT Height - Height of HDOB / OPM.
 *             UNSHORT Max_frames - Maximum number of animation frames.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will allocate enough memory for the animation
 *              frames.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_drag(UNSHORT Width, UNSHORT Height, UNSHORT Max_frames)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Allocate memory for drag OPM */
	Drag_OPM_handle = MEM_Do_allocate(Width * Height * Max_frames,
	 (UNLONG) &Drag_OPM, &OPM_ftype);
	if (!Drag_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
	}

	/* Insert handle in HDOB data */
	Drag_HDOB.Graphics_handle = Drag_OPM_handle;

	/* Insert dimensions in HDOB data */
	Drag_HDOB.Width 	= Width;
	Drag_HDOB.Height	= Height;

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Drag_OPM_handle);
	Result = OPM_New(Width, Height * Max_frames, 1, &Drag_OPM, Ptr);
	MEM_Free_pointer(Drag_OPM_handle);

	if (!Result)
	{
		Error(ERROR_NO_OPM);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_drag
 * FUNCTION  : Exit dragging.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 15:32
 * LAST      : 06.07.95 17:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_drag(void)
{
	/* Delete HDOB */
	Delete_HDOB(Drag_HDOB_nr);

	/* End of drag & drop mode */
	Leave_drag_drop_mode();

	/* Delete dragged item OPM and free memory */
	OPM_Del(&Drag_OPM);
	MEM_Free_memory(Drag_OPM_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_drag_HDOB_towards_char_slot
 * FUNCTION  : Move drag HDOB towards a slot of a character's inventory.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 09:50
 * LAST      : 05.07.95 17:08
 * INPUTS    : UNSHORT Target_slot_index (1...33).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_drag_HDOB_towards_char_slot(UNSHORT Target_slot_index)
{
	struct Item_slot_object *Target_item_slot;

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

	/* Do it */
	Move_drag_HDOB_towards_object(&(Target_item_slot->Object));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_drag_HDOB_towards_object
 * FUNCTION  : Move drag HDOB towards an object.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.07.95 17:08
 * LAST      : 07.07.95 11:53
 * INPUTS    : struct Object *Target - Pointer to target object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_drag_HDOB_towards_object(struct Object *Target)
{
	SISHORT X, Y;
	SISHORT Old_X, Old_Y;

	/* Already over target ? */
	if (!Is_over_object(Target->Self))
	{
		/* No -> Get target coordinates */
		X = Target->Rect.left + 1;
		Y = Target->Rect.top + 1;

		/* Detach the HDOB from the mouse pointer */
		Drag_HDOB.Flags &= ~HDOB_ATTACHED;

		/* Save relative HDOB coordinates */
		Old_X = Drag_HDOB.X;
		Old_Y = Drag_HDOB.Y;

		/* Set absolute coordinates of HDOB */
		Drag_HDOB.X += Mouse_X;
		Drag_HDOB.Y += Mouse_Y;

		/* Change HDOB data */
		Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

		/* Mouse off */
		Mouse_off();

		/* Move HDOB towards target */
		Move_HDOB(Drag_HDOB_nr, X, Y, 12);

		/* Attach HDOB to mouse pointer */
		Drag_HDOB.Flags |= HDOB_ATTACHED;

		/* Restore relative HDOB coordinates */
		Drag_HDOB.X = Old_X;
		Drag_HDOB.Y = Old_Y;

		/* Change HDOB data */
		Change_HDOB(Drag_HDOB_nr, &Drag_HDOB);

		/* Mouse on */
		Mouse_on();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Body_backpack_item_drag_abort_handler
 * FUNCTION  : Body & backpack item drag & drop abort handler.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.03.95 15:00
 * LAST      : 25.08.95 15:01
 * INPUTS    : struct Drag_drop_data *Drag_drop_data_ptr - Pointer to
 *              drag & drop data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Body_backpack_item_drag_abort_handler(struct Drag_drop_data *Drag_drop_data_ptr)
{
	/* Act depending on drag & drop data ID */
	switch (Drag_drop_data_ptr->Data_ID)
	{
		/* Body item */
		case BODY_ITEM_DD_DATA_ID:
		{
			/* In the right member's inventory ? */
			if (Inventory_member != Drag_member)
			{
				/* Enter inventory */
				Go_Inventory(Drag_member);
			}

			/* Drop the item back on the source slot */
			Drop_on_body_item((struct Item_slot_object *)
			 Drag_drop_data_ptr->Source_object, Drag_drop_data_ptr);

			break;
		}
		/* Backpack item */
		case BACKPACK_ITEM_DD_DATA_ID:
		{
			/* In the right member's inventory ? */
			if (Inventory_member != Drag_member)
			{
				/* Enter inventory */
				Go_Inventory(Drag_member);
			}

			/* Drop the item back on the source slot */
			Drop_on_backpack_item((struct Item_slot_object *)
			 Drag_drop_data_ptr->Source_object, Drag_drop_data_ptr);

			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_item_PUM_evaluator
 * FUNCTION  : Evaluate inventory item pop-up menu.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 12.10.95 12:47
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
	struct Event_action *Event_action;
	UNSHORT Spell_areas;
	UNSHORT Current_spell_area;
	UNSHORT Max_quantity;
	UNSHORT Item_price;
	UNSHORT Chain_nr;

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
		/* Yes -> Block dropping */
		PUMES[0].Flags |= PUME_BLOCKED;
		PUMES[0].Blocked_message_nr = 193;
	}

	/* Remove all item uses per default */
	PUMES[2].Flags |= PUME_ABSENT;
	PUMES[3].Flags |= PUME_ABSENT;
	PUMES[4].Flags |= PUME_ABSENT;
	PUMES[5].Flags |= PUME_ABSENT;
	PUMES[6].Flags |= PUME_ABSENT;
	PUMES[8].Flags |= PUME_ABSENT;

	/* Act depending on item type */
	switch (Item_data->Type)
	{
		/* Spell scroll */
		case SPELL_SCROLL_IT:
		{
			/* Make present */
			PUMES[2].Flags &= ~PUME_ABSENT;

			/* Is the item broken & not cheating ? */
			if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
			{
				/* Yes -> Disable */
				PUMES[2].Flags |= PUME_BLOCKED;
				PUMES[2].Blocked_message_nr = 191;
			}
			else
			{
				/* No -> Right class to use this item ? */
				if (Item_data->Class_use & (1 << Get_class(Active_char_handle)))
				{
					/* Yes -> Right spell class ? */
					if (Spell_class_known(Active_char_handle,
					 Item_data->Spell_class))
					{
						/* Yes -> Does the active member already know this spell ? */
						if (Spell_known(Active_char_handle, Item_data->Spell_class,
						 Item_data->Spell_nr))
						{
							/* Yes -> Block PUM entry */
							PUMES[2].Flags |= PUME_BLOCKED;
							PUMES[2].Blocked_message_nr = 188;
						}
					}
					else
					{
						/* No -> Block PUM entry */
						PUMES[2].Flags |= PUME_BLOCKED;
						PUMES[2].Blocked_message_nr = 187;
					}
				}
				else
				{
					/* No -> Disable */
					PUMES[2].Flags |= PUME_BLOCKED;
					PUMES[2].Blocked_message_nr = 599;
				}
			}
			break;
		}
		/* Drink */
		case DRINK_IT:
		{
			/* Make present */
			PUMES[3].Flags &= ~PUME_ABSENT;

			/* Is the item broken & not cheating ? */
			if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
			{
				/* Yes -> Disable */
				PUMES[3].Flags |= PUME_BLOCKED;
				PUMES[3].Blocked_message_nr = 191;
			}
			else
			{
				/* No -> Right class to use this item ? */
				if (!(Item_data->Class_use & (1 << Get_class(Active_char_handle))))
				{
					/* No -> Disable */
					PUMES[3].Flags |= PUME_BLOCKED;
					PUMES[3].Blocked_message_nr = 599;
				}
			}
			break;
		}
		/* Text scroll */
		case TEXT_SCROLL_IT:
		{
			/* Make present */
			PUMES[5].Flags &= ~PUME_ABSENT;

			/* Is the item broken & not cheating ? */
			if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
			{
				/* Yes -> Disable */
				PUMES[5].Flags |= PUME_BLOCKED;
				PUMES[5].Blocked_message_nr = 191;
			}
			else
			{
				/* No -> Right class to use this item ? */
				if (!(Item_data->Class_use & (1 << Get_class(Active_char_handle))))
				{
					/* No -> Disable */
					PUMES[5].Flags |= PUME_BLOCKED;
					PUMES[5].Blocked_message_nr = 599;
				}
			}
			break;
		}
		/* Special item */
		case SPECIAL_IT:
		{
			/* Make present */
			PUMES[6].Flags &= ~PUME_ABSENT;

			/* Is the item broken & not cheating ? */
			if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
			{
				/* Yes -> Disable */
				PUMES[6].Flags |= PUME_BLOCKED;
				PUMES[6].Blocked_message_nr = 191;
			}
			else
			{
				/* No -> Right class to use this item ? */
				if (Item_data->Class_use & (1 << Get_class(Active_char_handle)))
				{
					/* Yes -> Does the party already have this special item ? */
					if (PARTY_DATA.Special_item_flags &
					 (1 << Item_data->Misc[0]))
					{
						/* Yes -> Block PUM entry */
			 			PUMES[6].Flags |= PUME_BLOCKED;
						PUMES[6].Blocked_message_nr = 132;
					}
				}
				else
				{
					/* No -> Disable */
					PUMES[6].Flags |= PUME_BLOCKED;
					PUMES[6].Blocked_message_nr = 599;
				}
			}
			break;
		}
		/* Anything else */
		default:
		{
			/* Make a new event action */
			Event_action = Push_event_action();

			Event_action->Actor_type	= PARTY_ACTOR_TYPE;
			Event_action->Action_type	= USE_ITEM_ACTION;
			Event_action->Action_value	= Packet->Index;
			Event_action->Action_extra	= Item_data->Type;

			/* Search item event set */
			Chain_nr = Search_event_set(Item_event_set_handle, 0xFFFF);

			Pop_event_action();

			/* Found anything ? */
			if (Chain_nr != 0xFFFF)
			{
				/* Yes -> Make use item present */
				PUMES[8].Flags &= ~PUME_ABSENT;
			}
		}
	}

	/* Does it contain a spell / not in combat ? */
	if (Item_data->Spell_nr && !In_Combat)
	{
		/* Yes -> Make present */
		PUMES[4].Flags &= ~PUME_ABSENT;

		/* Is the item broken & not cheating ? */
		if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
		{
			/* Yes -> Disable */
			PUMES[4].Flags |= PUME_BLOCKED;
			PUMES[4].Blocked_message_nr = 191;
		}
		else
		{
			/* No -> Right class to use this item ? */
			if (Item_data->Class_use & (1 << Get_class(Active_char_handle)))
			{
				/* Yes -> Any charges left ? */
				if (Packet->Charges)
				{
					/* Yes -> Get spell data */
					Spell_areas = Get_spell_areas((UNSHORT) Item_data->Spell_class,
					 (UNSHORT) Item_data->Spell_nr);
					Current_spell_area = Get_spell_area();

					/* Right spell area ? */
					if (!(Spell_areas & (1 << Current_spell_area)))
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
				/* No -> Block PUM entry */
				PUMES[4].Flags |= PUME_BLOCKED;
				PUMES[4].Blocked_message_nr = 599;
			}
		}
	}

	/* In Merchant screen / not a spell merchant ? */
	if (In_Merchant && (Place_type != SPELL_MERCHANT_PLACE_TYPE))
	{
		/* Yes -> Is the item undroppable ? */
		if (Item_data->Flags & UNDROPPABLE)
		{
			/* Yes -> Block selling */
			PUMES[7].Flags |= PUME_BLOCKED;
			PUMES[7].Blocked_message_nr = 193;
		}
		else
		{
			/* No -> Is the item broken & not cheating ? */
			if ((Packet->Flags & BROKEN_ITEM) && (!Cheat_mode))
			{
				/* Yes -> Block selling */
				PUMES[7].Flags |= PUME_BLOCKED;
				PUMES[7].Blocked_message_nr = 751;
			}
			else
			{
				/* Yes -> How many items can the merchant take ? */
				Max_quantity = Count_nr_items_that_will_fit_in_merchant_data(Packet->Index);

				/* Any ? */
				if (Max_quantity)
				{
					/* Yes -> Calculate the item's price */
					Item_price = max((Item_data->Price * Place_price) / 120, 1);

					/* Too cheap ? */
					if (Item_price <= 1)
					{
						/* Yes -> Block selling */
						PUMES[7].Flags |= PUME_BLOCKED;
						PUMES[7].Blocked_message_nr = 587;
					}
				}
				else
				{
					/* No -> Block selling */
					PUMES[7].Flags |= PUME_BLOCKED;
					PUMES[7].Blocked_message_nr = 581;
				}
			}
		}
	}
	else
	{
		/* No -> Cannot sell */
		PUMES[7].Flags |= PUME_ABSENT;
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
 * LAST      : 11.07.95 19:20
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

	BOOLEAN Result;

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
		Quantity = Input_number_with_symbol
		(
			1,
			0,
			(SILONG) Packet->Quantity,
			System_text_ptrs[192],
			16,
			16,
			Item_graphics_handle,
			Item_data->Pic_nr * 256
		);
	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Is quantity zero ? */
	if (Quantity)
	{
		/* No -> Are you sure ? */
		if (Quantity > 1)
			Result = Boolean_requester(System_text_ptrs[531]);
		else
			Result = Boolean_requester(System_text_ptrs[62]);

		if (Result)
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
 * LAST      : 12.10.95 17:48
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
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT Item_slot_index;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is the spell's level too high ? */
	if (Get_spell_level(Item_data->Spell_class, Item_data->Spell_nr) >
	 Get_level(Active_char_handle))
	{
		/* Yes -> Apologise */
		Do_text_window(System_text_ptrs[189]);
	}
	else
	{
		/* No -> Learn spell */
		Learn_spell
		(
			Active_char_handle,
			Item_data->Spell_class,
			Item_data->Spell_nr
		);

		/* Tell the good news */
		Do_text_window(System_text_ptrs[698]);

		/* Get item slot index */
		Item_slot_index = Descriptor->Item_slot_index;

		/* Backpack item ? (yuk) */
		if (Descriptor->Slots_offset == Backpack_items_offset)
		{
			/* Yes -> Adjust index */
			Item_slot_index += ITEMS_ON_BODY;
		}

		/* Set data */
		Used_item_char_handle	= Inventory_char_handle;
		Used_item_slot_index		= Item_slot_index;

		/* Destroy item */
		Destroy_used_item();
	}

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Examine_inventory_item
 * FUNCTION  : Examine an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.01.95 15:41
 * LAST      : 11.09.95 15:21
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
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *)
	 (MEM_Claim_pointer(Descriptor->Slots_handle) + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Examine item */
	Examine_item(Packet);

	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Use_inventory_item
 * FUNCTION  : Use an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 18:28
 * LAST      : 12.10.95 16:18
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Use_inventory_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT Item_slot_index;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *)
	 (MEM_Claim_pointer(Descriptor->Slots_handle) + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Set item index and type */
	Use_item_action.Action_value = Packet->Index;
	Use_item_action.Action_extra = Item_data->Type;

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Set even more data */
	Use_item_action.Actor_index = PARTY_DATA.Active_member;

	/* Get item slot index */
	Item_slot_index = Descriptor->Item_slot_index;

	/* Backpack item ? (yuk) */
	if (Descriptor->Slots_offset == Backpack_items_offset)
	{
		/* Yes -> Adjust index */
		Item_slot_index += ITEMS_ON_BODY;
	}

	/* Set final data */
	Used_item_char_handle	= Inventory_char_handle;
	Used_item_slot_index		= Item_slot_index;

	/* Check events */
	Perform_action(&Use_item_action);

	/* Re-draw the screen */
	Execute_broadcast_method(Earth_object, DRAW_METHOD, NULL);
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Drink_inventory_item
 * FUNCTION  : Drink an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.09.95 19:57
 * LAST      : 12.10.95 16:18
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
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	UNSHORT Selected_party_member;
	UNSHORT Item_slot_index;

	Descriptor = (struct Item_descriptor *) Data;

	/* Which party member should drink this ? */
	Selected_party_member = Select_party_member(System_text_ptrs[719],
	 NULL, 0);

	/* Anyone selected ? */
	if (Selected_party_member != 0xFFFF)
	{
		/* Yes -> Set text subject */
		Subject_char_handle = Party_char_handles[Selected_party_member - 1];

		/* Get item packet */
		Packet = (struct Item_packet *)
		 (MEM_Claim_pointer(Descriptor->Slots_handle) + Descriptor->Slots_offset);
		Packet += Descriptor->Item_slot_index - 1;

		/* Get item data */
		Item_data = Get_item_data(Packet);

		/* Set item index and type */
		Use_item_action.Action_value = Packet->Index;
		Use_item_action.Action_extra = Item_data->Type;

		Free_item_data();
		MEM_Free_pointer(Descriptor->Slots_handle);

		/* Set even more data */
		Use_item_action.Actor_index = Selected_party_member;

		/* Get item slot index */
		Item_slot_index = Descriptor->Item_slot_index;

		/* Backpack item ? (yuk) */
		if (Descriptor->Slots_offset == Backpack_items_offset)
		{
			/* Yes -> Adjust index */
			Item_slot_index += ITEMS_ON_BODY;
		}

		/* Set final data */
		Used_item_char_handle	= Inventory_char_handle;
		Used_item_slot_index		= Item_slot_index;

		/* Check events */
		Perform_action(&Use_item_action);

		/* Re-draw the screen */
		Execute_broadcast_method(Earth_object, DRAW_METHOD, NULL);
		Switch_screens();
	}
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
 * NAME      : PUM_Activate_inventory_item
 * FUNCTION  : Activate an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.05.95 11:55
 * LAST      : 12.10.95 17:51
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
	UNSHORT Special_item_index;
	UNSHORT Item_slot_index;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Get special item index */
	Special_item_index = (UNSHORT) Item_data->Misc[0];

	Free_item_data();
	MEM_Free_pointer(Descriptor->Slots_handle);

	/* Get item slot index */
	Item_slot_index = Descriptor->Item_slot_index;

	/* Backpack item ? (yuk) */
	if (Descriptor->Slots_offset == Backpack_items_offset)
	{
		/* Yes -> Adjust index */
		Item_slot_index += ITEMS_ON_BODY;
	}

	/* Set data */
	Used_item_char_handle	= Inventory_char_handle;
	Used_item_slot_index		= Item_slot_index;

	/* Destroy item */
	Destroy_used_item();

	/* Activate special item */
	PARTY_DATA.Special_item_flags |= (1 << Special_item_index);

	/* Tell the player */
	Set_permanent_message_nr(753 + Special_item_index);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Sell_inventory_item (Merchant screen only)
 * FUNCTION  : Sell an inventory item (inventory item pop-up menu).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 17:44
 * LAST      : 30.08.95 21:08
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Sell_inventory_item(UNLONG Data)
{
	struct Item_slot_object *Item_slot;
	struct Item_list_object *Item_list;
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_packet Sold_packet;
	struct Item_data *Item_data;
	UNSHORT Item_price;
	UNSHORT Max_quantity;
	UNSHORT Quantity;
	UNSHORT Slot_index;

	Item_slot = (struct Item_slot_object *) Get_object_data(PUM_source_object_handle);
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);
	Descriptor = (struct Item_descriptor *) Data;

	/* Get real packet index */
	Slot_index = Item_slot->Number;
	if (Item_list->Type == CHAR_BACKPACK_INV_TYPE)
	{
		Slot_index += ITEMS_ON_BODY;
	}

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* How many items can the merchant take ? */
	Max_quantity = Count_nr_items_that_will_fit_in_merchant_data(Packet->Index);

	/* Any ? */
	if (Max_quantity)
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Packet);

		/* Calculate the price for one item */
		Item_price = max((Item_data->Price * Place_price) / 120, 1);

		Free_item_data();

		/* How many items does the inventory member have ? */
		Max_quantity = min(Max_quantity, Packet->Quantity);

		/* More than one ? */
		if (Packet->Quantity > 1)
		{
			/* Yes -> Ask the player how many should be sold */
			Quantity = Input_number_with_symbol
			(
				1,
				0,
				(SILONG) Max_quantity,
				System_text_ptrs[582],
				16,
				16,
				Item_graphics_handle,
				Item_data->Pic_nr * 256
			);
		}
		else
		{
			/* No */
			Quantity = 1;
		}

		/* Is quantity zero ? */
		if (Quantity)
		{
			/* No -> Do you want to sell ? */
			if (Merchant_sell_requester(Quantity, Item_price))
			{
				/* Yes -> Clear sold packet */
				Clear_packet(&Sold_packet);

				/* Take items from character */
				if (Take_item_from_character(Inventory_char_handle,
				 Slot_index, Quantity, &Sold_packet))
				{
					/* Transfer items */
					Auto_move_packet_to_merchant_data(&Sold_packet);

					/* Give gold to party */
					Set_party_gold(Get_party_gold() + (Quantity * Item_price));

					/* Redraw source and merchant item lists */
					Execute_method(Item_list->Object.Self, DRAW_METHOD, NULL);
					Execute_method(Merchant_item_list_object, DRAW_METHOD, NULL);

					/* Print standard text */
					Do_text_window(System_text_ptrs[583]);
				}
			}
		}
	}
	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Drop_items
 * FUNCTION  : Drop items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.02.95 11:19
 * LAST      : 06.10.95 16:40
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
	struct Item_list_object *Item_list;
	struct Item_slot_object *Item_slot;
	struct Item_packet Target_packet;
	UNSHORT Slot_index;

	Item_slot = (struct Item_slot_object *) Get_object_data(Item_slot_handle);

	/* Get item list data */
	Item_list = (struct Item_list_object *) Get_object_data(Item_slot->Object.Parent);

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

	/* Clear "target" packet */
	Clear_packet(&Target_packet);

	/* Try to take items */
	Take_item_from_character
	(
		Item_list->Slots_handle,
		Slot_index,
		Quantity,
		&Target_packet
	);

	/* Redraw object */
	Execute_method(Item_slot_handle, DRAW_METHOD, NULL);

	/* Were any items taken ? */
	if (Target_packet.Quantity)
	{
		/* Yes -> Show dropping */
		Show_dropping_items
		(
			Item_slot,
			&Target_packet,
			(UNSHORT) Target_packet.Quantity
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_dropping_items
 * FUNCTION  : Show dropping of items.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 15:23
 * LAST      : 06.10.95 15:23
 * INPUTS    : struct Item_slot_object *Item_slot - Pointer to item slot
 *              object.
 *             struct Item_packet *Packet - Pointer to item packet.
 *             UNSHORT Quantity - Quantity that should be dropped.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Beware! The item packet belonging to the item slot has already
 *              been emptied!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_dropping_items(struct Item_slot_object *Item_slot,
 struct Item_packet *Packet, UNSHORT Quantity)
{
	struct HDOB Drop_HDOBs[MAX_DROPPING_ITEMS];
	struct Object *Object;
	struct Item_data *Item_data;
	BOOLEAN HDOBs_are_ready;
	UNSHORT HDOB_nrs[MAX_DROPPING_ITEMS];
	UNSHORT Nr_HDOBs;
	UNSHORT i;

	Object = &(Item_slot->Object);

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Determine number of HDOBs */
	Nr_HDOBs = min(max(Quantity, 1), MAX_DROPPING_ITEMS);

	/* Build HDOBs */
	for (i=0;i<Nr_HDOBs;i++)
	{
		/* Create COMOB */
		Drop_HDOBs[i].Draw_mode			= HDOB_MASK;
		Drop_HDOBs[i].Flags				= 0;
		Drop_HDOBs[i].X					= Object->Rect.left;
		Drop_HDOBs[i].Y					= Object->Rect.top;
		Drop_HDOBs[i].Width				= 16;
		Drop_HDOBs[i].Height				= 16;
		Drop_HDOBs[i].Nr_frames			= (UNSHORT) Item_data->Nr_frames;
		Drop_HDOBs[i].Graphics_handle	= Item_graphics_handle;
		Drop_HDOBs[i].Graphics_offset	= 256 * (UNLONG) Item_data->Pic_nr;
		Drop_HDOBs[i].Draw_data			= 0;

		/* Add HDOB */
		HDOB_nrs[i] = Add_HDOB(&(Drop_HDOBs[i]));

		/* Initialize dropping */
		Init_HDOB_drop(HDOB_nrs[i]);
	}

	Free_item_data();

	/* Wait */
	for (;;)
	{
		/* Check if all HDOBs are ready */
		HDOBs_are_ready = TRUE;
		for (i=0;i<Nr_HDOBs;i++)
		{
			if (!HDOB_is_ready(HDOB_nrs[i]))
			{
				HDOBs_are_ready = FALSE;
				break;
			}
		}

		/* Exit if all HDOBs are ready */
		if (HDOBs_are_ready)
			break;

		/* Redraw */
		Update_screen();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_use_item
 * FUNCTION  : Use item event action.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.10.95 16:16
 * LAST      : 12.10.95 16:16
 * INPUTS    : struct Event_action *Action - Pointer to event action data.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Do_use_item(struct Event_action *Action)
{
	/* Destroy used item */
	Destroy_used_item();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_used_item
 * FUNCTION  : Destroy a used item (if necessary).
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.10.95 16:59
 * LAST      : 12.10.95 16:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_used_item(void)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

	/* Get character data of item user */
	Char = (struct Character_data *) MEM_Claim_pointer(Used_item_char_handle);

	/* Get used item packet */
	if (Used_item_slot_index <= ITEMS_ON_BODY)
	{
		Packet = &(Char->Body_items[Used_item_slot_index - 1]);
	}
	else
	{
		Packet = &(Char->Backpack_items[Used_item_slot_index - ITEMS_ON_BODY - 1]);
	}

	/* Anything in packet ? */
	if (!Packet_empty(Packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Packet);

		/* Destroy after use ? */
		if (Item_data->Flags & DESTROY_AFTER_USE)
		{
			/* Yes -> */
			/* Let object explode */

			/* Remove item */
			Remove_item
			(
				Used_item_char_handle,
				Used_item_slot_index,
				1
			);

			/* In Inventory ? */
			if (In_Inventory)
			{
				/* Yes -> Body or backpack item ? */
				if (Used_item_slot_index <= ITEMS_ON_BODY)
				{
					/* Body -> Redraw body item list */
					Execute_method
					(
						Body_item_list_object,
						DRAW_METHOD,
						NULL
					);
				}
				else
				{
					/* Backpack -> Redraw backpack item list */
					Execute_method
					(
						Backpack_item_list_object,
						DRAW_METHOD,
						NULL
					);
				}
			}
		}
		Free_item_data();
	}

	MEM_Free_pointer(Used_item_char_handle);
}

