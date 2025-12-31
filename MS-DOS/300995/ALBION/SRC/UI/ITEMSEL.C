/************
 * NAME     : ITEMSEL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : ITEMSEL.H, USERFACE.C
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <GAMETEXT.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <ITEMSEL.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <PRTLOGIC.H>
#include <STATAREA.H>
#include <COLOURS.H>

/* defines */

#define ITEMSELWIN_WIDTH	(213)
#define ITEMSELWIN_HEIGHT	(120)

/* structure definitions */

/* Item select window OID */
struct ItemSel_window_OID {
	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;

	MEM_HANDLE Slots_handle;
	UNLONG Slots_offset;
};

/* Item select window object */
struct ItemSel_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNCHAR *Text;
	UNSHORT *Result_ptr;
	UNSHORT *Blocked_messages_ptr;
};

/* prototypes */

/* Item select window object methods */
UNLONG Init_ItemSel_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_ItemSel_object(struct Object *Object, union Method_parms *P);

/* Selectable item object methods */
UNLONG Draw_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_SelItem_object(struct Object *Object, union Method_parms *P);
UNLONG Left_SelItem_object(struct Object *Object, union Method_parms *P);

/* global variables */

/* Item selection window method list */
static struct Method ItemSel_methods[] = {
	{ INIT_METHOD,		Init_ItemSel_object },
	{ EXIT_METHOD,		Exit_Window_object },
	{ DRAW_METHOD,		Draw_ItemSel_object },
	{ UPDATE_METHOD,	Update_help_line },
	{ RIGHT_METHOD,	Close_Window_object },
	{ CLOSE_METHOD,	Close_Window_object },
	{ TOUCHED_METHOD,	Dehighlight },
	{ RESTORE_METHOD,	Restore_window },
	{ 0, NULL}
};

/* Item selection window class description */
static struct Object_class ItemSel_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct ItemSel_window_object),
	&ItemSel_methods[0]
};

/* Selectable item slot method list */
static struct Method SelItem_slot_methods[] = {
	{ INIT_METHOD,			Init_Item_slot_object },
	{ DRAW_METHOD,			Draw_SelItem_object },
	{ UPDATE_METHOD,		Update_Item_slot_object },
	{ FEEDBACK_METHOD,	Feedback_SelItem_object },
	{ HIGHLIGHT_METHOD,	Highlight_SelItem_object },
	{ HELP_METHOD,			Help_Item_slot_object },
	{ TOUCHED_METHOD,		Touch_Item_slot_object },
	{ LEFT_METHOD,			Left_SelItem_object },
	{ 0, NULL}
};

/* Selectable item slot class description */
static struct Object_class SelItem_slot_Class = {
	0, sizeof(struct Item_slot_object),
	&SelItem_slot_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_character_item
 * FUNCTION  : Select an item from a character's inventory.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 14:56
 * LAST      : 08.04.95 14:56
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNCHAR *Text - Pointer to window text.
 *             Item_evaluator Evaluator - Pointer to evaluation function.
 * RESULT    : UNSHORT : Selected item slot index (1...) /
 *              0xFFFF = cancelled.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_character_item(MEM_HANDLE Char_handle, UNCHAR *Text,
 Item_evaluator Evaluator)
{
	struct Character_data *Char;
	struct ItemSel_window_OID OID;
	struct Item_packet Slots[ITEMS_ON_BODY + ITEMS_PER_CHAR];
	UNSHORT Item_slot_indices[ITEMS_ON_BODY + ITEMS_PER_CHAR];
	UNSHORT Blocked_message_nrs[ITEMS_ON_BODY + ITEMS_PER_CHAR];
	UNSHORT Selected_item = 0xFFFF;
	UNSHORT Obj;
	UNSHORT Message_nr = 0;
	UNSHORT Counter;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Copy body item packets */
	Counter = 0;
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is not tail slot OR Iskai ? */
		if ((i != TAIL-1) || (Get_race(Char_handle) == ISKAI_RACE))
		{
			/* Yes -> Is the slot empty ? */
			if (!Packet_empty(&(Char->Body_items[i])))
			{
				/* No -> Any evaluation function ? */
				if (Evaluator)
				{
					/* Yes -> Evaluate item */
					Message_nr = (Evaluator)(Char, &(Char->Body_items[i]));
				}

				/* Copy packet */
				memcpy((UNBYTE *) &Slots[Counter],
				 (UNBYTE *) &(Char->Body_items[i]), sizeof(struct Item_packet));

				/* Store blocked message number */
				Blocked_message_nrs[Counter] = Message_nr;

				/* Store slot index */
				Item_slot_indices[Counter] = i + 1;

				/* Count up */
				Counter++;
			}
		}
	}

	/* Copy backpack item packets */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Is the slot empty ? */
		if (!Packet_empty(&(Char->Backpack_items[i])))
		{
			/* No -> Any evaluation function ? */
			if (Evaluator)
			{
				/* Yes -> Evaluate item */
				Message_nr = (Evaluator)(Char, &(Char->Backpack_items[i]));
			}

			/* Copy packet */
			memcpy((UNBYTE *) &Slots[Counter],
			 (UNBYTE *) &(Char->Backpack_items[i]), sizeof(struct Item_packet));

			/* Store blocked message number */
			Blocked_message_nrs[Counter] = Message_nr;

			/* Store slot index */
			Item_slot_indices[Counter] = ITEMS_ON_BODY + i + 1;

			/* Count up */
			Counter++;
		}
	}
	MEM_Free_pointer(Char_handle);

	/* Clear remaining item packets */
	for (i=Counter;i<ITEMS_ON_BODY + ITEMS_PER_CHAR;i++)
	{
		BASEMEM_FillMemByte((UNBYTE *) &Slots[i], sizeof(struct Item_packet), 0);
		Blocked_message_nrs[i] = 0;
	}

	/* Prepare selection window OID */
	OID.Text							= Text;
	OID.Result_ptr					= &Selected_item;
	OID.Blocked_messages_ptr	= Blocked_message_nrs;
	OID.Slots_handle				= NULL;
	OID.Slots_offset				= (UNLONG) &Slots[0];

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&ItemSel_Class,
		(UNBYTE *) &OID,
		(Screen_width - ITEMSELWIN_WIDTH) / 2,
		(Panel_Y - ITEMSELWIN_HEIGHT) / 2,
		ITEMSELWIN_WIDTH,
		ITEMSELWIN_HEIGHT
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);

	/* Translate item slot index */
	if (Selected_item != 0xFFFF)
	{
		Selected_item = Item_slot_indices[Selected_item - 1];
	}

	return Selected_item;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_ItemSel_object
 * FUNCTION  : Initialize method of Item selection window object.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.04.95 12:22
 * LAST      : 06.04.95 12:22
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * NOTES     : - This function assumes there will be some party members to
 *              choose from (i.e. at least one).
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_ItemSel_object(struct Object *Object, union Method_parms *P)
{
	struct ItemSel_window_object *ItemSel_window;
	struct ItemSel_window_OID *OID;
	struct Item_list_OID Item_list_OID;

	ItemSel_window = (struct ItemSel_window_object *) Object;
	OID = (struct ItemSel_window_OID *) P;

	/* Copy data from OID */
	ItemSel_window->Text = OID->Text;
	ItemSel_window->Result_ptr = OID->Result_ptr;
	ItemSel_window->Blocked_messages_ptr  = OID->Blocked_messages_ptr;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Add item list to window */
	Item_list_OID.Type						= NO_CHAR_INV_TYPE;
	Item_list_OID.Nr_items					= ITEMS_ON_BODY + ITEMS_PER_CHAR;
	Item_list_OID.Slots_width				= 11;
	Item_list_OID.Slots_height				= 3;
	Item_list_OID.Slots_handle				= OID->Slots_handle;
	Item_list_OID.Slots_offset				= OID->Slots_offset;
	Item_list_OID.Menu						= NULL;
	Item_list_OID.Item_slot_class_ptr	= &SelItem_slot_Class;

	Add_object
	(
		Object->Self,
		&Item_list_Class,
		(UNBYTE *) &Item_list_OID,
		12,
		10,
		40,
		40
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_ItemSel_object
 * FUNCTION  : Draw method of Item selection window object.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 15:10
 * LAST      : 11.09.95 16:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_ItemSel_object(struct Object *Object, union Method_parms *P)
{
	struct ItemSel_window_object *ItemSel_window;
	struct PA PA;
	struct OPM *OPM;

	ItemSel_window = (struct ItemSel_window_object *) Object;
	OPM = &(ItemSel_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Any text ? */
	if (ItemSel_window->Text)
	{
		/* Yes -> Create print area */
		PA.Area.left	= 14;
		PA.Area.top		= 80;
		PA.Area.width	= 185;
		PA.Area.height	= 28;

		/* Draw box around text */
		Draw_deep_box
		(
			OPM,
			PA.Area.left - 2,
			PA.Area.top - 2,
			PA.Area.width + 4,
			PA.Area.height + 4
		);

		/* Print text */
		Push_PA(&PA);
		Set_ink(SILVER_TEXT);
		Display_text(OPM, ItemSel_window->Text);
		Pop_PA();
	}

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_SelItem_object
 * FUNCTION  : Draw method of Item slot object.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:11
 * LAST      : 25.09.95 16:41
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct ItemSel_window_object *ItemSel_window;
	struct OPM *OPM;

	Item_slot = (struct Item_slot_object *) Object;
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);
	OPM = &(ItemSel_window->Window_OPM);

	/* Clear item slot */
	Restore_object_background(Object);

	/* Is a blocked item ? */
	if (*(ItemSel_window->Blocked_messages_ptr + Item_slot->Number - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box
		(
			OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			&(Red_table[0])
		);
	}
	else
	{
		/* No -> Draw dark box */
		Put_recoloured_box
		(
			OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			&(Recolour_tables[3][0])
		);
	}

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_SelItem_object
 * FUNCTION  : Feedback method of Item slot object.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:10
 * LAST      : 25.09.95 16:42
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct ItemSel_window_object *ItemSel_window;
	struct OPM *OPM;

	Item_slot = (struct Item_slot_object *) Object;
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);
	OPM = &(ItemSel_window->Window_OPM);

	/* Clear item slot */
	Restore_object_background(Object);

	/* Is a blocked item ? */
	if (*(ItemSel_window->Blocked_messages_ptr + Item_slot->Number - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box
		(
			OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			&(Red_table[0])
		);
	}

	/* Draw dark box */
	Draw_deep_box
	(
		OPM,
		Object->X,
		Object->Y,
		ITEM_SLOT_INNER_WIDTH,
		ITEM_SLOT_INNER_HEIGHT
	);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_SelItem_object
 * FUNCTION  : Highlight method of Item slot object.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:10
 * LAST      : 25.09.95 16:42
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct Item_slot_object *Item_slot;
	struct ItemSel_window_object *ItemSel_window;
	struct OPM *OPM;

	Item_slot = (struct Item_slot_object *) Object;
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);
	OPM = &(ItemSel_window->Window_OPM);

	/* Clear item slot */
	Restore_object_background(Object);

	/* Is a blocked item ? */
	if (*(ItemSel_window->Blocked_messages_ptr + Item_slot->Number - 1))
	{
		/* Yes -> Draw red box */
		Put_recoloured_box
		(
			OPM,
			Object->X,
			Object->Y,
			ITEM_SLOT_INNER_WIDTH,
			ITEM_SLOT_INNER_HEIGHT,
			&(Red_table[0])
		);
	}

	/* Draw bright box */
	Put_recoloured_box
	(
		OPM,
		Object->X,
		Object->Y,
		ITEM_SLOT_INNER_WIDTH,
		ITEM_SLOT_INNER_HEIGHT,
		&(Recolour_tables[6][0])
	);

	/* Draw item */
	Draw_item_slot_item(Item_slot);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_SelItem_object
 * FUNCTION  : Left method of Selectable item slot object.
 * FILE      : ITEMSEL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.04.95 16:08
 * LAST      : 27.04.95 16:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_SelItem_object(struct Object *Object, union Method_parms *P)
{
	struct ItemSel_window_object *ItemSel_window;
	struct Item_slot_object *Item_slot;
	struct Item_packet Packet;
	UNSHORT Message_nr;

	Item_slot = (struct Item_slot_object *) Object;

	/* Get item selection window */
	ItemSel_window = (struct ItemSel_window_object *)
	 Get_object_data(Get_object_data(Object->Parent)->Parent);

	/* Get item packet */
	Extract_slot_packet(Item_slot, &Packet);

	/* Any item ? */
	if (!Packet_empty(&Packet))
	{
		/* Yes -> Get blocked message number */
		Message_nr = *(ItemSel_window->Blocked_messages_ptr +
		 Item_slot->Number - 1);

		/* Is a blocked item ? */
		if (Message_nr)
		{
			/* Yes -> Print blocked message */
			Execute_method(Help_line_object, SET_METHOD,
			 (void *) System_text_ptrs[Message_nr]);
		}

		/* Really clicked ? */
		if (Normal_clicked(Object))
		{
			/* Yes -> Blocked ? */
			if (!Message_nr)
			{
				/* No -> Select this item */
				*(ItemSel_window->Result_ptr) = Item_slot->Number;

				/* Close item selection window */
				Execute_method(ItemSel_window->Object.Self, CLOSE_METHOD, NULL);
			}
		}
	}

	return 0;
}

