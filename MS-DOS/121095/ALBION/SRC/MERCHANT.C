/************
 * NAME     : MERCHANT.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 17-08-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MERCHANT.H
 ************/

/* includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <MERCHANT.H>
#include <PRTLOGIC.H>
#include <EVELOGIC.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <GOLDFOOD.H>
#include <BUTTONS.H>
#include <GAMETEXT.H>
#include <BOOLREQ.H>
#include <COLOURS.H>
#include <XFTYPES.H>
#include <PLACES.H>
#include <STATAREA.H>
#include <INPUTNR.H>
#include <TEXTWIN.H>
#include <EXAMITEM.H>

/* defines */

#define ITEMS_PER_MERCHANT		(24)

/* prototypes */

/* Merchant module functions */
void Merchant_ModInit(void);
void Merchant_ModExit(void);
void Merchant_DisInit(void);
void Merchant_DisExit(void);

void Exit_Merchant(struct Button_object *Button);

/* Merchant methods */
UNLONG Init_Merchant_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Merchant_object(struct Object *Object, union Method_parms *P);

/* Merchant item slot methods */
UNLONG Help_Merchant_item_object(struct Object *Object, union Method_parms *P);

/* Merchant item pop-up menu actions */
void Merchant_item_PUM_evaluator(struct PUM *PUM);
void PUM_Buy_merchant_item(UNLONG Data);
void PUM_Examine_merchant_item(UNLONG Data);

/* global variables */

BOOLEAN In_Merchant;

UNSHORT Merchant_item_list_object;

static MEM_HANDLE Merchant_data_handle;

/* Module */
static struct Module Merchant_Mod = {
	LOCAL_MOD, SCREEN_MOD, CHEST_SCREEN,
	NULL,
	Merchant_ModInit,
	Merchant_ModExit,
	Merchant_DisInit,
	Merchant_DisExit,
	NULL
};

/* Merchant method list */
static struct Method Merchant_methods[] = {
	{ INIT_METHOD, Init_Merchant_object },
	{ DRAW_METHOD, Draw_Merchant_object },
//	{ UPDATE_METHOD, Update_Status_area },
	{ TOUCHED_METHOD, Dehighlight },
	{ 0, NULL}
};

/* Merchant object class */
static struct Object_class Merchant_Class = {
	0, sizeof(struct Object),
	&Merchant_methods[0]
};

/* Merchant item slot method list */
static struct Method Merchant_item_slot_methods[] = {
	{ INIT_METHOD,				Init_Item_slot_object },
	{ DRAW_METHOD,				Draw_Item_slot_object },
	{ UPDATE_METHOD,			Update_Item_slot_object },
	{ FEEDBACK_METHOD,		Feedback_Item_slot_object },
	{ HIGHLIGHT_METHOD,		Highlight_Item_slot_object },
	{ POP_UP_METHOD,			Pop_up_Item_slot_object },
	{ HELP_METHOD,				Help_Merchant_item_object },
	{ RIGHT_METHOD,			Right_Item_slot_object },
	{ TOUCHED_METHOD,			Touch_Item_slot_object },
	{ 0, NULL}
};

/* Merchant item slot object class */
static struct Object_class Merchant_item_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&Merchant_item_slot_methods[0]
};

/* Merchant item pop-up menu entry list */
static struct PUME Merchant_item_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 573, PUM_Buy_merchant_item},
   {PUME_AUTO_CLOSE, 0, 17, PUM_Examine_merchant_item},
};

/* Merchant item pop-up menu */
static struct PUM Merchant_item_PUM = {
	2,
	NULL,
	(UNLONG) &Selected_item,
	Merchant_item_PUM_evaluator,
	Merchant_item_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_Merchant
 * FUNCTION  : Enter Merchant screen.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:44
 * LAST      : 17.08.95 16:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_Merchant(void)
{
	/* Enter Merchant screen */
	Exit_display();
	Push_module(&Merchant_Mod);
	Init_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_ModInit
 * FUNCTION  : Initialize Merchant module.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:44
 * LAST      : 20.09.95 12:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merchant_ModInit(void)
{
	BOOLEAN Result;

	/* Set inventory parameters */
	Inventory_char_handle	= Active_char_handle;
	Inventory_member			= PARTY_DATA.Active_member;

	/* Initialize inventory data */
	Result = Init_inventory_data();
	if (!Result)
	{
		Pop_module();
	}

	/* Prepare inventory exit button */
	Exit_inventory_button_OID.Help_message_nr = 547;
	Exit_inventory_button_OID.Function = Exit_Merchant;

	/* In Inventory */
	In_Inventory = TRUE;

	/* In Merchant */
	In_Merchant = TRUE;

	/* Load merchant data */
	Merchant_data_handle = Load_subfile(MERCHANT_DATA, Place_merchant_data_nr);
	if (!Merchant_data_handle)
	{
		Error(ERROR_FILE_LOAD);

		Pop_module();

		Exit_program();

		return;
	}

	/* Initialize display */
	Merchant_DisInit();

	/* Was a selector text given ? */
	if (Place_selector_text_block_nr != 0xFF)
	{
		/* Yes -> Print it */
		Do_text_file_window(Place_text_handle,	Place_selector_text_block_nr);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_ModExit
 * FUNCTION  : Exit Merchant module.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:44
 * LAST      : 17.08.95 17:46
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merchant_ModExit(void)
{
	Merchant_DisExit();

	/* Save merchant data */
	Save_subfile(Merchant_data_handle, MERCHANT_DATA, Place_merchant_data_nr);

	/* Free merchant data */
	MEM_Free_memory(Merchant_data_handle);

	Inventory_ModExit();

	/* No longer in Merchant */
	In_Merchant = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_DisInit
 * FUNCTION  : Initialize Merchant display.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:48
 * LAST      : 17.08.95 16:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merchant_DisInit(void)
{
	/* Add interface objects */
	InvLeft_object = Add_object(Earth_object, &Merchant_Class, NULL,
	 0, 0, INVENTORY_MIDDLE, Panel_Y);
	InvRight_object = Add_object(Earth_object, &InvRight_Class, NULL,
	 INVENTORY_MIDDLE, 0, Screen_width - INVENTORY_MIDDLE, Panel_Y);

	/* Draw */
	Execute_method(InvLeft_object, DRAW_METHOD, NULL);
	Execute_method(InvRight_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_DisExit
 * FUNCTION  : Exit Merchant display.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:48
 * LAST      : 17.08.95 16:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merchant_DisExit(void)
{
	/* Delete interface objects */
	Delete_object(InvLeft_object);
	Delete_object(InvRight_object);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Merchant
 * FUNCTION  : Leave the Merchant screen.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:48
 * LAST      : 17.08.95 16:48
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Merchant(struct Button_object *Button)
{
	/* Exit */
	Pop_module();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Merchant_object
 * FUNCTION  : Initialize method of Merchant object.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:51
 * LAST      : 17.08.95 16:51
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Merchant_object(struct Object *Object, union Method_parms *P)
{
	struct Item_list_OID Item_list_OID;
	SISHORT X;
	UNSHORT Width, Height;

	/* Calculate merchant item list dimensions */
	Width = (6 * ITEM_SLOT_OUTER_WIDTH) - ITEM_SLOT_BETWEEN_X + 2;
	Height = (4 * ITEM_SLOT_OUTER_HEIGHT) + 2;

	/* Calculate merchant item list X-coordinate */
	X = (Object->Rect.width - Width) / 2;

	/* Add merchant item list */
	Item_list_OID.Type						= NO_CHAR_INV_TYPE;
	Item_list_OID.Nr_items					= ITEMS_PER_MERCHANT;
	Item_list_OID.Slots_width				= 6;
	Item_list_OID.Slots_height				= 4;
	Item_list_OID.Slots_handle				= Merchant_data_handle;
	Item_list_OID.Slots_offset				= 0;
	Item_list_OID.Menu						= &Merchant_item_PUM;
	Item_list_OID.Item_slot_class_ptr	= &Merchant_item_slot_Class;

	Merchant_item_list_object = Add_object
	(
		Object->Self,
		&Item_list_Class,
		(UNBYTE *) &Item_list_OID,
		X,
		14,
		Width,
		Height
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Merchant_object
 * FUNCTION  : Draw method of Merchant object.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.95 19:54
 * LAST      : 15.08.95 19:54
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Merchant_object(struct Object *Object, union Method_parms *P)
{
	/* Clear left inventory */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, Object->X, Object->Y,
	 Object->Rect.width, Object->Rect.height, Object->X, Object->Y);

	/* Print merchant description */
	Set_ink(SILVER_TEXT);
	Print_centered_line_string(&Main_OPM, 4, 1, INVENTORY_MIDDLE - 8,
	 System_text_ptrs[572]);

	/* Draw child objects */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Help_Merchant_item_object
 * FUNCTION  : Help method of merchant item object.
 * FILE      : ITEMLIST.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 11:59
 * LAST      : 18.08.95 11:59
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Help_Merchant_item_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	struct Item_data *Item_data;
	UNLONG Party_gold;
	UNSHORT Item_price;
	UNCHAR Item_name[ITEM_NAME_LENGTH + 1];
	UNCHAR String[100];

	Item_slot = (struct Item_slot_object *) Object;

  	/* Get target packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Is the slot empty ? */
	if (!Packet_empty(&Packet))
	{
		/* No -> Get target slot item name */
		Get_item_name(&Packet, Item_name);

		/* Get item data */
		Item_data = Get_item_data(&Packet);

		/* Calculate the price for one item */
		Item_price = max((Item_data->Price * Place_price) / 100, 1);

		Free_item_data();

		/* Is the item too expensive ? */
		Party_gold = Get_party_gold();
		if (Item_price > Party_gold)
		{
			/* Yes -> Make help line string */
			sprintf(String, System_text_ptrs[577], Item_name, Item_price / 10,
			 Item_price % 10);
		}
		else
		{
			/* No -> Make help line string */
			sprintf(String, System_text_ptrs[579], Item_name, Item_price / 10,
			 Item_price % 10);
		}

		/* Print help line */
		Execute_method(Help_line_object, SET_METHOD, (void *) String);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_item_PUM_evaluator
 * FUNCTION  : Evaluate Merchant item pop-up menu.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 21:12
 * LAST      : 18.08.95 12:38
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merchant_item_PUM_evaluator(struct PUM *PUM)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_data *Item_data;
	struct PUME *PUMES;
	UNLONG Party_gold;
	UNSHORT Item_price;

	PUMES = PUM->PUME_list;

	/* Does the inventory character have at least one free slot ? */
	if (Char_inventory_full(Inventory_char_handle))
	{
		/* No -> Block buying */
		PUMES[0].Flags |= PUME_BLOCKED;
		PUMES[0].Blocked_message_nr = 580;
	}
	else
	{
		/* Yes */
		Descriptor = (struct Item_descriptor *) PUM->Data;

		/* Get item packet */
		Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
		 + Descriptor->Slots_offset);
		Packet += Descriptor->Item_slot_index - 1;

		/* Get item data */
		Item_data = Get_item_data(Packet);

		/* Calculate the price for one item */
		Item_price = max((Item_data->Price * Place_price) / 100, 1);

		Free_item_data();
		MEM_Free_pointer(Descriptor->Slots_handle);

		/* Is the item too expensive ? */
		Party_gold = Get_party_gold();
		if (Item_price > Party_gold)
		{
			/* Yes -> Block buying */
			PUMES[0].Flags |= PUME_BLOCKED;
			PUMES[0].Blocked_message_nr = 578;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Buy_merchant_item
 * FUNCTION  : Buy a Merchant item (Merchant item pop-up menu).
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 21:12
 * LAST      : 18.08.95 16:04
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Buy_merchant_item(UNLONG Data)
{
	struct Item_descriptor *Descriptor;
	struct Item_packet *Packet;
	struct Item_packet Bought_packet;
	struct Item_data *Item_data;
	UNLONG Party_gold;
	UNSHORT Item_price;
	UNSHORT Max_quantity;
	UNSHORT Quantity;

	Descriptor = (struct Item_descriptor *) Data;

	/* Get item packet */
	Packet = (struct Item_packet *) (MEM_Claim_pointer(Descriptor->Slots_handle)
	 + Descriptor->Slots_offset);
	Packet += Descriptor->Item_slot_index - 1;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Calculate the price for one item */
	Item_price = max((Item_data->Price * Place_price) / 100, 1);

	/* Any price ? */
	if (Item_price)
	{
		/* Yes -> How many items can the party afford ? */
		Party_gold = Get_party_gold();
		Max_quantity = min(Packet->Quantity, (Party_gold / Item_price));
	}
	else
	{
		/* No */
		Max_quantity = Packet->Quantity;
	}

	/* How many items will fit in the inventory member's backpack ? */
	Max_quantity = min(Count_nr_items_that_will_fit_in_backpack(Inventory_char_handle,
	 Packet->Index), Max_quantity);

	/* More than one ? */
	if (Packet->Quantity > 1)
	{
		/* Yes -> Ask the player how many should be bought */
		Quantity = Input_number_with_symbol(1, 0, (SILONG) Max_quantity,
		 System_text_ptrs[575], 16, 16, Item_graphics_handle,
		 Item_data->Pic_nr * 256);
	}
	else
	{
		/* No */
		Quantity = 1;
	}

	Free_item_data();

	/* Is quantity zero ? */
	if (Quantity)
	{
		/* No -> Are you sure ? */
		if (Place_buy_requester(Quantity * Item_price))
		{
			/* Yes -> Build packet with bought items */
			Build_item_packet(&Bought_packet, Packet->Index, Quantity);

			/* Transfer items */
			Put_item_in_character(Inventory_char_handle, 0,
			 Bought_packet.Quantity, &Bought_packet);

			/* Remove items from merchant */
			Packet->Quantity -= Quantity;

			/* Anything left in merchant packet ? */
			if (!Packet->Quantity)
			{
				/* No -> Destroy packet */
				Clear_packet(Packet);
			}

			/* Redraw backpack and merchant item lists */
			Execute_method(Backpack_item_list_object, DRAW_METHOD, NULL);
			Execute_method(Merchant_item_list_object, DRAW_METHOD, NULL);

			/* Was an action text given ? */
			if (Place_action_text_block_nr != 0xFF)
			{
				/* Yes -> Print it */
				Do_text_file_window(Place_text_handle,
				 Place_action_text_block_nr);
			}
			else
			{
				/* No -> Print standard text */
				Do_text_window(System_text_ptrs[576]);
			}
		}
	}

	MEM_Free_pointer(Descriptor->Slots_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Examine_merchant_item
 * FUNCTION  : Examine a Merchant item (Merchant item pop-up menu).
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 21:12
 * LAST      : 11.09.95 15:23
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Examine_merchant_item(UNLONG Data)
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
 * NAME      : Auto_move_packet_to_merchant_data
 * FUNCTION  : Automatically move an item packet to the current merchant data.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 21:14
 * LAST      : 29.09.95 13:11
 * INPUTS    : struct Item_packet *Source_packet - Source packet.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will remove items from the source packet, and
 *              destroy the source packet if the quantity reaches zero.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Auto_move_packet_to_merchant_data(struct Item_packet *Source_packet)
{
	struct Item_packet *Merchant_data;
	struct Item_packet *Target_packet;
	UNSHORT Fit;
	UNSHORT i;

	Merchant_data = (struct Item_packet *) MEM_Claim_pointer(Merchant_data_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Search for other slots with this item */
		for (i=0;i<ITEMS_PER_MERCHANT;i++)
		{
			Target_packet = &(Merchant_data[i]);

			/* Same item / not full ? */
			if ((Target_packet->Index == Source_packet->Index) &&
			 (Target_packet->Quantity < 99))
			{
				/* Yes -> Calculate how many items will fit in the slot */
				Fit = min((99 - Target_packet->Quantity), Source_packet->Quantity);

				/* Add items to target packet */
				Target_packet->Quantity += Fit;

				/* Remove items from source packet */
				Source_packet->Quantity -= Fit;

				/* Anything left ? */
				if (!Source_packet->Quantity)
				{
					/* No -> Exit */
					break;
				}
			}
		}

		/* Anything left in source packet ? */
		if (Source_packet->Quantity)
		{
			/* Yes -> Find a free slot */
			for (i=0;i<ITEMS_PER_MERCHANT;i++)
			{
				Target_packet = &(Merchant_data[i]);

				/* Free ? */
				if (Packet_empty(Target_packet))
				{
					/* Yes -> Put item in target packet */
					memcpy
					(
						(UNBYTE *) Target_packet,
						(UNBYTE *) Source_packet,
						sizeof(struct Item_packet)
					);

					/* Calculate how many items will fit in the slot */
					Fit = min(99, Source_packet->Quantity);

					/* Add items to target packet */
					Target_packet->Quantity = Fit;

					/* Remove items from source packet */
					Source_packet->Quantity -= Fit;

					/* Exit if all items have been put away */
					if (!Source_packet->Quantity)
						break;
				}
			}
		}
	}

	MEM_Free_pointer(Merchant_data_handle);

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
 * NAME      : Count_nr_items_that_will_fit_in_merchant_data
 * FUNCTION  : Count the number of items of a certain type that will fit in
 *              the current merchant data.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 16:24
 * LAST      : 18.08.95 16:24
 * INPUTS    : UNSHORT Item_index - Index of item.
 * RESULT    : UNSHORT : Quantity that will fit.
 * BUGS      : No known.
 * NOTES     : - This function uses the same algorithm as
 *              Auto_move_packet_to_merchant_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_nr_items_that_will_fit_in_merchant_data(UNSHORT Item_index)
{
	struct Item_packet *Merchant_data;
	UNSHORT Counter = 0;
	UNSHORT Fit;
	UNSHORT i;

	Merchant_data = (struct Item_packet *) MEM_Claim_pointer(Merchant_data_handle);

	/* Scan all merchant slots */
	for (i=0;i<ITEMS_PER_MERCHANT;i++)
	{
		/* Is this slot empty ? */
		if (Packet_empty(&(Merchant_data[i])))
		{
			/* Yes -> Count up */
			Counter += 99;
		}
		else
		{
			/* No -> Does it contain the same item / not full ? */
			if ((Merchant_data[i].Index == Item_index) &&
			 (Merchant_data[i].Quantity < 99))
			{
				/* Calculate how many items will fit in the slot */
				Fit = 99 - Merchant_data[i].Quantity;

				/* Count up */
				Counter += Fit;
			}
		}
	}

	MEM_Free_pointer(Merchant_data_handle);

	return Counter;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_sell_requester
 * FUNCTION  : Ask if the player wants to sell items.
 * FILE      : MERCHANT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.08.95 12:47
 * LAST      : 19.08.95 12:47
 * INPUTS    : UNSHORT Quantity - Number of items offered for sale.
 *             UNSHORT Item_price - Price per item.
 * RESULT    : BOOLEAN : Player wants to sell.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Merchant_sell_requester(UNSHORT Quantity, UNSHORT Item_price)
{
	BOOLEAN Result;
	UNLONG Total_price;
	UNCHAR String[200];

	/* Exit if the quantity is zero or if the price is too low */
	if (!Quantity || (Item_price <= 1))
		return FALSE;

	/* Store price for PRIC text command */
	Place_total_price = Quantity * Item_price;

	#if FALSE
	/* Was a text block number given ? */
	if (Place_value != 0xFF)
	{
		/* Yes -> Get text file address */
		Text_ptr = MEM_Claim_pointer(Place_text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Place_value);

		/* Build sub-block catalogue */
		Build_subblock_catalogue(Text_ptr, Subblock_offsets, 2);

		/* Button texts included ? */
		if (Subblock_offsets[0] && Subblock_offsets[1])
		{
			/* Yes -> Do requester with custom button texts */
			Result = Boolean_requester_with_buttons
			(
				Text_ptr,
				Text_ptr + Subblock_offsets[0],
				Text_ptr + Subblock_offsets[1]
			);
		}
		else
		{
			/* No -> Do requester with default button texts */
			Result = Boolean_requester(Text_ptr);
		}

		MEM_Free_pointer(Place_text_handle);
	}
	else
	#endif
	{
		/* No -> Build standard sell requester string */
		Total_price = Quantity * Item_price;
		sprintf(String, System_text_ptrs[584], Total_price / 10,
		 Total_price % 10);

		/* Do requester */
		Result = Boolean_requester(String);
	}

	return Result;
}

