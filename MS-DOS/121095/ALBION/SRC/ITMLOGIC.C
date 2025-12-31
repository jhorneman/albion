/************
 * NAME     : ITMLOGIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 20-6-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : ITMLOGIC.H
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <GAMEVAR.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <INVENTO.H>
#include <EVELOGIC.H>
#include <GAMETEXT.H>
#include <STATAREA.H>
#include <ITMLOGIC.H>
#include <PRTLOGIC.H>
#include <COMBAT.H>
#include <XFTYPES.H>

/* structure definitions */

/* prototypes */

void Positive_item_boni(MEM_HANDLE Char_handle, struct Item_data *Item_data);
void Negative_item_boni(MEM_HANDLE Char_handle, struct Item_data *Item_data);

/* global variables */

/* Item data handles */
MEM_HANDLE Item_event_set_handle;
MEM_HANDLE Item_event_text_handle;

static MEM_HANDLE Item_data_handle;
static MEM_HANDLE Item_names_handle;
MEM_HANDLE Item_graphics_handle;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_item_data
 * FUNCTION  : Initialize item data.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 12:24
 * LAST      : 20.06.95 12:24
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_item_data(void)
{
	/* Load item event-set */
	Item_event_set_handle = Load_subfile(EVENT_SET, ITEM_EVENT_SET);
	if (!Item_event_set_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load item event-texts */
	Item_event_text_handle = Load_subfile(EVENT_TEXT, ITEM_EVENT_SET);
	if (!Item_event_text_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load item data */
	Item_data_handle = Load_file(ITEM_LIST);
	if (!Item_data_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load item names */
	Item_names_handle = Load_file(ITEM_NAMES);
	if (!Item_names_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load item graphics */
	Item_graphics_handle = Load_file(ITEM_GFX);
	if (!Item_graphics_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_item_data
 * FUNCTION  : Delete item data.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 12:25
 * LAST      : 20.06.95 12:25
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_item_data(void)
{
	/* Destroy item event-set and -texts */
	MEM_Free_memory(Item_event_set_handle);
	MEM_Free_memory(Item_event_text_handle);

	/* Destroy item data */
	MEM_Free_memory(Item_data_handle);
	MEM_Free_memory(Item_names_handle);
	MEM_Free_memory(Item_graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_target_body_slot_for_item
 * FUNCTION  : Find the target body slot for an item.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 16:30
 * LAST      : 30.09.95 14:22
 * INPUTS    : struct Item_data *Source_item - Pointer to source item data.
 * RESULT    : UNSHORT : Target slot index (0xFFFF for no free (!) slot).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Find_target_body_slot_for_item(struct Item_data *Source_item)
{
	struct Character_data *Char;
	UNSHORT Target_slot_index = 0xFFFF;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Inventory_char_handle);

	/* Is this a special case ? */
	if (Source_item->Body_place == HAND_OR_TAIL)
	{
		/* Yes -> Right hand free ? */
		if (Packet_empty(&(Char->Body_items[RIGHT_HAND - 1])))
		{
			/* Yes -> Select right hand */
			Target_slot_index = RIGHT_HAND;
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
			else
			{
				/* Yes -> No free slot */
				Target_slot_index = 0xFFFF;
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
				/* Yes -> Left ring-finger free ? */
				if (Packet_empty(&(Char->Body_items[LEFT_FINGER - 1])))
				{
					/* Yes -> Select left ringfinger */
					Target_slot_index = LEFT_FINGER;
				}
			}
		}
	}

	return Target_slot_index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Take_item_from_character
 * FUNCTION  : Take an item out of a character's inventory.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 12:28
 * LAST      : 29.09.95 12:37
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
		0, 0, 0,
		Do_take_item_from_char, NULL, NULL
	};

	struct Move_item_event_data Action_data;
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

		/* Is it a body item ? */
		if (Slot_index && (Slot_index <= ITEMS_ON_BODY))
		{
			/* Yes -> Unequip action */
			Take_item_from_char_action.Action_type = UNEQUIP_ITEM_ACTION;
		}
		else
		{
			/* No -> Take action */
			Take_item_from_char_action.Action_type = TAKE_ITEM_FROM_CHAR_ACTION;
		}

		/* Build data for event action */
		Action_data.Handle		= Char_handle;
		Action_data.Slot_index	= Slot_index;
		Action_data.Quantity		= Quantity;
		Action_data.Packet		= Target_packet;

		/* Build event action data */
		Take_item_from_char_action.Actor_index		= Inventory_member;
		Take_item_from_char_action.Action_value	= Source_packet->Index;
		Take_item_from_char_action.Action_extra	= (UNSHORT) Item_data->Type;
		Take_item_from_char_action.Action_data		= &Action_data;

		/* Check events */
		Result = Perform_action(&Take_item_from_char_action);

		Free_item_data();
	}
	MEM_Free_pointer(Char_handle);

	return Result;
}

BOOLEAN
Do_take_item_from_char(struct Event_action *Action)
{
	struct Move_item_event_data *Data;
	struct Character_data *Char;
	struct Item_packet *Source_packet;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Get action data */
	Data = (struct Move_item_event_data *) Action->Action_data;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Data->Handle);

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
			/* Yes -> Is the item cursed / not in cheat mode ? */
			if ((Source_packet->Flags & CURSED) && !Cheat_mode)
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
			if (Data->Packet)
			{
				/* Yes -> Put item in target packet */
				memcpy
				(
					Data->Packet,
					Source_packet,
					sizeof(struct Item_packet)
				);
				Data->Packet->Quantity = Data->Quantity;
			}

			/* Remove item(s) */
			Remove_item(Data->Handle, Data->Slot_index, Data->Quantity);
		}

		Free_item_data();
	}
	MEM_Free_pointer(Data->Handle);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_item_in_character
 * FUNCTION  : Put an item in a character's inventory.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.03.95 15:24
 * LAST      : 29.09.95 12:37
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 *             UNSHORT Slot_index - Slot index (1...33) / 0 = auto-move to
 *              backpack.
 *             UNSHORT Quantity - Quantity that must be added.
 *             struct Item_packet *Target_packet - Source packet.
 * RESULT    : BOOLEAN : Was an item put?
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
		0, 0, 0,
		Do_put_item_in_char, NULL, NULL
	};

	struct Move_item_event_data Action_data;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;

	/* Anything in the source packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Is it a body item ? */
		if (Slot_index && (Slot_index <= ITEMS_ON_BODY))
		{
			/* Yes -> Equip action */
			Put_item_in_char_action.Action_type = EQUIP_ITEM_ACTION;
		}
		else
		{
			/* No -> Put action */
			Put_item_in_char_action.Action_type = PUT_ITEM_IN_CHAR_ACTION;
		}

		/* Build data for event action */
		Action_data.Handle		= Char_handle;
		Action_data.Slot_index	= Slot_index;
		Action_data.Quantity		= Quantity;
		Action_data.Packet		= Source_packet;

		/* Build event action data */
		Put_item_in_char_action.Actor_index		= Inventory_member;
		Put_item_in_char_action.Action_value	= Source_packet->Index;
		Put_item_in_char_action.Action_extra	= (UNSHORT) Item_data->Type;
		Put_item_in_char_action.Action_data		= &Action_data;

		/* Check events */
		Result = Perform_action(&Put_item_in_char_action);

		Free_item_data();
	}

	return Result;
}

BOOLEAN
Do_put_item_in_char(struct Event_action *Action)
{
	struct Move_item_event_data *Data;
	struct Item_data *Item_data;
	BOOLEAN Result = FALSE;
	UNSHORT Max_hands;

	/* Get action data */
	Data = (struct Move_item_event_data *) Action->Action_data;

	/* Anything in the source packet ? */
	if (!Packet_empty(Data->Packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Data->Packet);

		/* Is it a body item ? */
		if ((Data->Slot_index) && (Data->Slot_index <= ITEMS_ON_BODY))
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
				if (Data->Packet->Flags & BROKEN_ITEM)
				{
					/* Yes -> Apologise to the player */
					Set_permanent_message_nr(194);
				}
				else
				{
					/* No -> Right class to equip this item ? */
					if (!(Item_data->Class_use &
					 (1 << Get_class(Data->Handle))))
					{
						/* No -> Apologise to the player */
						Set_permanent_message_nr(195);
					}
					else
					{
						/* Yes -> Right sex to equip this item ? */
						if (!(Item_data->Sex_use &
						 (1 << Get_sex(Data->Handle))))
						{
							/* No -> Apologise to the player */
							Set_permanent_message_nr(196);
						}
						else
						{
							/* Yes -> Determine number of hands */
							if (Get_race(Data->Handle) == ISKAI_RACE)
								Max_hands = 3;
							else
								Max_hands = 2;

							/* Enough hands free ? */
							if ((Get_nr_occupied_hands(Data->Handle) +
							 Item_data->Hand_use) > Max_hands)
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
				Add_item_to_body
				(
					Data->Handle,
					Data->Packet,
					Data->Slot_index
				);
			}
		}
		else
		{
			/* No -> Auto-move to backpack ? */
			if (Data->Slot_index)
			{
				/* No -> Add items to backpack */
				Add_item_to_backpack
				(
					Data->Handle,
					Data->Packet,
					Data->Slot_index - ITEMS_ON_BODY,
					Data->Quantity
				);
			}
			else
			{
				/* Yes -> Auto-move to backpack */
				Auto_move_packet_to_backpack(Data->Handle, Data->Packet);
			}

			/* Everything worked out fine */
			Result = TRUE;
		}
		Free_item_data();
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_item
 * FUNCTION  : Remove an item from a character's inventory.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 14:40
 * LAST      : 28.09.95 17:19
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
	SISHORT Skill;
	UNSHORT i;

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
			/* Body -> Remove skill taxes */
			for (i=0;i<2;i++)
			{
				/* Any skill tax ? */
				if (Item_data->Malus[i])
				{
					/* Yes -> Remove tax */
					Skill = Get_skill
					(
						Char_handle,
						Item_data->Item_skills[i]
					);
					Set_skill
					(
						Char_handle,
						Item_data->Item_skills[i],
						Skill + (SISHORT) Item_data->Malus[i]
					);
				}
			}

			/* Cursed item ? */
			if (Packet->Flags & CURSED)
			{
				/* Yes -> Add boni */
				Positive_item_boni(Char_handle, Item_data);
			}
			else
			{
				/* No -> Remove boni */
				Negative_item_boni(Char_handle, Item_data);
			}

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
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 15:53
 * LAST      : 28.09.95 17:19
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
	SISHORT Skill;
	UNSHORT i;

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

		/* Execute skill taxes */
		for (i=0;i<2;i++)
		{
			/* Any skill tax ? */
			if (Item_data->Malus[i])
			{
				/* Yes -> Tax */
				Skill = Get_skill
				(
					Char_handle,
					Item_data->Item_skills[i]
				);
				Set_skill
				(
					Char_handle,
					Item_data->Item_skills[i],
					Skill - (SISHORT) Item_data->Malus[i]
				);
			}
		}

		/* Cursed item ? */
		if (Source_packet->Flags & CURSED)
		{
			/* Yes -> Remove boni */
			Negative_item_boni(Char_handle, Item_data);
		}
		else
		{
			/* No -> Add boni */
			Positive_item_boni(Char_handle, Item_data);
		}

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
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 16:05
 * LAST      : 28.09.95 17:17
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             struct Item_data *Item_data - Pointer to item data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Positive_item_boni(MEM_HANDLE Char_handle, struct Item_data *Item_data)
{
	UNSHORT Number;

	/* Give LP max bonus */
	Set_max_LP
	(
		Char_handle,
		Get_max_LP(Char_handle) + (SISHORT) Item_data->LP_max
	);

	/* Give SP max bonus */
	Set_max_SP
	(
		Char_handle,
		Get_max_SP(Char_handle) + (SISHORT) Item_data->SP_max
	);

	/* Any attribute bonus ? */
	if (Item_data->Attribute_normal)
	{
		/* Yes -> Give attribute bonus */
		Number = Item_data->Attribute;
		Set_attribute
		(
			Char_handle,
			Number,
			Get_attribute(Char_handle, Number) +
			 (SISHORT) Item_data->Attribute_normal
		);
	}

	/* Any skill bonus ? */
	if (Item_data->Skill_normal)
	{
		/* Yes -> Give skill bonus */
		Number = Item_data->Skill;
		Set_skill
		(
			Char_handle,
			Number,
			Get_skill(Char_handle, Number) +	(SISHORT) Item_data->Skill_normal
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Negative_item_boni
 * FUNCTION  : Execute negative item boni.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 16:12
 * LAST      : 29.09.95 11:00
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             struct Item_data *Item_data - Pointer to item data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Negative_item_boni(MEM_HANDLE Char_handle, struct Item_data *Item_data)
{
	SISHORT Value;
	SISHORT Damage;
	UNSHORT Number;
	UNSHORT Member_nr;

	/* Remove LP max bonus */
	Value = max(Get_max_LP(Char_handle) - (SISHORT) Item_data->LP_max, 1);
	Set_max_LP
	(
		Char_handle,
		Value
	);

	/* Is new LP max smaller than LP ? */
	Damage = Get_LP(Char_handle) - Value;
	if (Damage > 0)
	{
		/* Yes -> Is party member ? */
		Member_nr = Find_char_data_in_party(Char_handle);
		if (Member_nr != 0xFFFF)
		{
			/* Yes -> Do damage */
			Do_damage(Member_nr, Damage);
		}
	}

	/* Remove SP max bonus */
	Value = max(Get_max_SP(Char_handle) - (SISHORT) Item_data->SP_max, 0);
	Set_max_SP
	(
		Char_handle,
		Value
	);

	/* Is new SP max smaller than SP ? */
	Damage = Get_SP(Char_handle) - Value;
	if (Damage > 0)
	{
		/* Yes -> Remove SP */
		Set_SP
		(
			Char_handle,
			Value
		);
	}

	/* Any attribute bonus ? */
	if (Item_data->Attribute_normal)
	{
		/* Yes -> Remove attribute bonus */
		Number = Item_data->Attribute;

		Value = Get_attribute(Char_handle, Number) -
		 (SISHORT) Item_data->Attribute_normal;

		Set_attribute
		(
			Char_handle,
			Number,
			max(Value, 0)
		);
	}

	/* Any skill bonus ? */
	if (Item_data->Skill_normal)
	{
		/* Yes -> Remove skill bonus */
		Number = Item_data->Skill;

		Value = Get_skill(Char_handle, Number) -
		 (SISHORT) Item_data->Skill_normal;

		Set_skill
		(
			Char_handle,
			Number,
			max(Value, 0)
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_item_to_backpack
 * FUNCTION  : Add an item to a character's backpack items.
 * FILE      : ITMLOGIC.C
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

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get target packet */
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
			memcpy
			(
				(UNBYTE *) Target_packet,
				(UNBYTE *) Source_packet,
				sizeof(struct Item_packet)
			);
			Target_packet->Quantity = Quantity;
		}

		/* Remove items from source packet */
		Source_packet->Quantity -= Quantity;
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
 * NAME      : Auto_move_packet_to_backpack
 * FUNCTION  : Automatically move an item packet to a character's backpack.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.01.95 15:34
 * LAST      : 29.09.95 13:08
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             struct Item_packet *Source_packet - Source packet.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will remove items from the source packet, and
 *              destroy the source packet if the quantity reaches zero.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Auto_move_packet_to_backpack(MEM_HANDLE Char_handle,
 struct Item_packet *Source_packet)
{
	struct Character_data *Char;
	struct Item_packet *Target_packet;
	struct Item_data *Item_data;
	UNSHORT Fit;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Anything in packet ? */
	if (!Packet_empty(Source_packet))
	{
		/* Yes -> Get item data */
		Item_data = Get_item_data(Source_packet);

		/* Is this a multiple item ? */
		if (Item_data->Flags & MULTIPLE)
		{
			/* Yes -> Search for other slots with this item */
			for (i=0;i<ITEMS_PER_CHAR;i++)
			{
				Target_packet = &(Char->Backpack_items[i]);

				/* Same item / not full ? */
				if ((Target_packet->Index == Source_packet->Index) && (Target_packet->Quantity < 99))
				{
					/* Calculate how many items will fit in the slot */
					Fit = 99 - Target_packet->Quantity;

					/* Enough ? */
					if (Fit > Source_packet->Quantity)
					{
						/* Yes -> Clip */
						Fit = Source_packet->Quantity;
					}

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
				for (i=0;i<ITEMS_PER_CHAR;i++)
				{
					Target_packet = &(Char->Backpack_items[i]);

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

						/* Remove items from source packet */
						Source_packet->Quantity = 0;

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
				Target_packet = &(Char->Backpack_items[i]);

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
					Target_packet->Quantity	= 1;

					/* Remove item from source packet */
					Source_packet->Quantity -= 1;

					/* Exit if all items have been put away */
					if (!Source_packet->Quantity)
						break;
				}
			}
		}
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
 * NAME      : Count_nr_items_that_will_fit_in_backpack
 * FUNCTION  : Count the number of items of a certain type that will fit in
 *              a character's backpack.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 15:53
 * LAST      : 18.08.95 15:53
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             UNSHORT Item_index - Index of item.
 * RESULT    : UNSHORT : Quantity that will fit.
 * BUGS      : No known.
 * NOTES     : - This function uses the same algorithm as
 *              Auto_move_packet_to_backpack.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_nr_items_that_will_fit_in_backpack(MEM_HANDLE Char_handle,
 UNSHORT Item_index)
{
	struct Character_data *Char;
	struct Item_packet *Target;
	struct Item_packet Dummy_packet;
	struct Item_data *Item_data;
	UNSHORT Counter = 0;
	UNSHORT Fit;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get item data */
	Build_item_packet(&Dummy_packet, Item_index, 1);
	Item_data = Get_item_data(&Dummy_packet);

	/* Is this a multiple item ? */
	if (Item_data->Flags & MULTIPLE)
	{
		/* Yes -> Search for other slots with this item */
		for (i=0;i<ITEMS_PER_CHAR;i++)
		{
			Target = &(Char->Backpack_items[i]);

			/* Same item / not full ? */
			if ((Target->Index == Item_index) && (Target->Quantity < 99))
			{
				/* Calculate how many items will fit in the slot */
				Fit = 99 - Target->Quantity;

				/* Count up */
				Counter += Fit;
			}
		}
	}

	/* Find free slots */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Free ? */
		if (Packet_empty(&(Char->Backpack_items[i])))
		{
			/* Yes -> Is this a multiple item ? */
			if (Item_data->Flags & MULTIPLE)
			{
				/* Yes -> Count up */
				Counter += 99;
			}
			else
			{
				/* No -> Count up */
				Counter += 1;
			}
		}
	}

	Free_item_data();
	MEM_Free_pointer(Char_handle);

	return Counter;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Curse_item
 * FUNCTION  : Curse an item in a character's inventory.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.06.95 15:39
 * LAST      : 07.06.95 15:39
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             UNSHORT Slot_index - Slot index (1...33).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Curse_item(MEM_HANDLE Char_handle, UNSHORT Slot_index)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

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
		/* Yes -> Is the item already cursed ? */
		if (!(Packet->Flags & CURSED))
		{
			/* No -> Curse item */
			Packet->Flags |= CURSED;

			/* Get item data */
			Item_data = Get_item_data(Packet);

			/* Remove boni */
			Negative_item_boni(Char_handle, Item_data);

			/* Implement effects of curse */
			Negative_item_boni(Char_handle, Item_data);

			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Decurse_item
 * FUNCTION  : De-curse an item in a character's inventory.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.06.95 15:42
 * LAST      : 07.06.95 15:42
 * INPUTS    : MEM_HANDLE Char_handle - Character data memory handle.
 *             UNSHORT Slot_index - Slot index (1...33).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Decurse_item(MEM_HANDLE Char_handle, UNSHORT Slot_index)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	struct Item_data *Item_data;

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
		/* Yes -> Is the item cursed ? */
		if (Packet->Flags & CURSED)
		{
			/* Yes -> Dec-curse item */
			Packet->Flags &= ~CURSED;

			/* Get item data */
			Item_data = Get_item_data(Packet);

			/* Remove effects of curse */
			Positive_item_boni(Char_handle, Item_data);

			/* Add boni */
			Positive_item_boni(Char_handle, Item_data);

			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_empty
 * FUNCTION  : Check if an inventory is empty.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 14:26
 * LAST      : 27.04.95 14:26
 * INPUTS    : struct Item_packet *Packet - Pointer to first item packet.
 *             UNSHORT Nr_items - Number of items in inventory.
 * RESULT    : BOOLEAN : Inventory empty.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Inventory_empty(struct Item_packet *Packet, UNSHORT Nr_items)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Check inventory */
	for (i=0;i<Nr_items;i++)
	{
		/* Is this packet full ? */
		if (!Packet_empty(Packet + i))
		{
			/* Yes -> Break */
			Result = FALSE;
			break;
		}
	}
	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Inventory_full
 * FUNCTION  : Check if an inventory is full.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 14:34
 * LAST      : 27.04.95 14:34
 * INPUTS    : struct Item_packet *Packet - Pointer to first item packet.
 *             UNSHORT Nr_items - Number of items in inventory.
 * RESULT    : BOOLEAN : Inventory full.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Inventory_full(struct Item_packet *Packet, UNSHORT Nr_items)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Check inventory */
	for (i=0;i<Nr_items;i++)
	{
		/* Is this packet empty ? */
		if (Packet_empty(Packet + i))
		{
			/* Yes -> Break */
			Result = FALSE;
			break;
		}
	}
	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_body_for_item_type
 * FUNCTION  : Search all body items for an item of a certain type.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 13:49
 * LAST      : 20.09.95 11:22
 *	INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Target_item_type - Target item type.
 * RESULT    : UNSHORT : Body item slot index (1...9) / 0xFFFF.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_body_for_item_type(MEM_HANDLE Char_handle, UNSHORT Target_item_type)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT Body_slot_index;
	UNSHORT Item_type;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Search all body slots */
	Body_slot_index = 0xFFFF;
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything there ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes -> Get item type */
			Item_data = Get_item_data(&(Char->Body_items[i]));
			Item_type = (UNSHORT) Item_data->Type;
			Free_item_data();

			/* Is the desired type ? */
			if (Item_type == Target_item_type)
			{
				/* Yes -> Found the item! */
				Body_slot_index = i + 1;
				break;
			}
		}
	}

	MEM_Free_pointer(Char_handle);

	return Body_slot_index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_cursed_items
 * FUNCTION  : Remove all cursed body items.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.09.95 14:48
 * LAST      : 29.09.95 14:48
 *	INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_cursed_items(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Search all body slots */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything there ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes -> Cursed ? */
			if (Char->Body_items[i].Flags & CURSED)
			{
				/* Yes -> Remove item */
				Remove_item
				(
					Char_handle,
					i + 1,
					1
				);
			}
		}
	}
	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Has_cursed_items
 * FUNCTION  : Check if a character has any cursed body items.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.09.95 14:53
 * LAST      : 29.09.95 14:53
 *	INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : BOOLEAN : Has cursed items.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Has_cursed_items(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Flag = FALSE;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Search all body slots */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything there ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes -> Cursed ? */
			if (Char->Body_items[i].Flags & CURSED)
			{
				/* Yes -> Yay! */
				Flag = TRUE;
				break;
			}
		}
	}
	MEM_Free_pointer(Char_handle);

	return Flag;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_item_packet
 * FUNCTION  : Build an item packet.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.06.95 15:16
 * LAST      : 19.06.95 15:16
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 *             UNSHORT Item_index - Item index.
 *             UNSHORT Quantity - Quantity.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will automatically set all the other elements
 *              of the item packet to their correct values (as specified in
 *              the item data).
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Free_item_slot_in_inventory(MEM_HANDLE Char_handle)
{
	return FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Optimise_inventory
 * FUNCTION  : Try to optimise an inventory
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 14:01
 * LAST      : 12.10.95 17:07
 * INPUTS    : struct Item_packet *Inventory - Pointer to first item packet.
 *             UNSHORT Nr_items - Number of items in the inventory.
 * RESULT    : UNSHORT : New number of items in the inventory.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Optimise_inventory(struct Item_packet *Inventory, UNSHORT Nr_items)
{
	struct Item_data *Item_data;
	UNSHORT Fit;
	UNSHORT New_counter;
	UNSHORT Target_index;
	UNSHORT i, j;

	/* Try to optimise the inventory */
	for (i=0;i<Nr_items;i++)
	{
		/* Anything here ? */
		if (!Packet_empty(&(Inventory[i])))
		{
			/* Yes -> Get item data */
			Item_data = Get_item_data(&(Inventory[i]));

			/* Is this a multiple item ? */
			if (Item_data->Flags & MULTIPLE)
			{
				/* Yes -> Try to pool */
				for (j=0;j<i;j++)
				{
					/* Anything here ? */
					if (!Packet_empty(&(Inventory[j])))
					{
						/* Yes -> Same item ? */
						if (Inventory[i].Index == Inventory[j].Index)
						{
							/* Yes -> How many will fit in this slot ? */
							Fit = 99 - Inventory[j].Quantity;

							/* Enough ? */
							if (Fit > Inventory[i].Quantity)
							{
								/* Yes -> Clip */
								Fit = Inventory[i].Quantity;
							}

							/* Add items to target packet */
							Inventory[j].Quantity += Fit;

							/* Remove items from source packet */
							Inventory[i].Quantity -= Fit;

							/* Anything left ? */
							if (!Inventory[i].Quantity)
							{
								/* Yes -> Destroy packet */
								Clear_packet(&(Inventory[i]));

								break;
							}
						}
					}
				}
			}
			Free_item_data();
		}
	}

	/* Move all slots to the front of the inventory */
	New_counter = 0;
	Target_index = 0;
	for (i=0;i<Nr_items;i++)
	{
		/* Anything here ? */
		if (!Packet_empty(&(Inventory[i])))
		{
			/* Yes -> Is there a need to copy ? */
			if (Target_index != i)
			{
				/* Yes -> Copy the packet down */
				memcpy
				(
					&(Inventory[Target_index]),
					&(Inventory[i]),
					sizeof(struct Item_packet)
				);

				/* Destroy original packet */
				Clear_packet(&(Inventory[i]));
			}

			/* Count up */
			New_counter++;
			Target_index++;
		}
	}

	/* Return new counter */
	return New_counter;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Packet_empty
 * FUNCTION  : Check if an item packet is empty.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 16:45
 * LAST      : 03.04.95 16:45
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : BOOLEAN : TRUE (empty) or FALSE (not empty).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Packet_empty(struct Item_packet *Packet)
{
	/* Anything in there ? */
	if (Packet->Index)
	{
		/* Yes -> Quantity not zero ? */
		if (Packet->Quantity)
		{
			/* Yes -> Not empty */
			return FALSE;
		}
		else
		{
			/* No -> Empty */
			return TRUE;
		}
	}
	else
	{
		/* No -> Empty */
		return TRUE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_packet
 * FUNCTION  : Clear an item packet.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 16:45
 * LAST      : 03.04.95 16:45
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_packet(struct Item_packet *Packet)
{
	BASEMEM_FillMemByte((UNBYTE *) Packet, sizeof(struct Item_packet), 0);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_item_data
 * FUNCTION  : Get the item data of the item in an item packet.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 16:46
 * LAST      : 03.04.95 16:46
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : struct Item_data * : Pointer to item data.
 * BUGS      : No known.
 * NOTES     : - The caller MUST call Free_item_data once it's finished with
 *              the item data.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Item_data *
Get_item_data(struct Item_packet *Packet)
{
	struct Item_data *Item_data;

	/* Is the packet empty ? */
	if (Packet_empty(Packet))
	{
		/* Yes -> Return NULL */
		Item_data = NULL;
	}
	else
	{
		/* No -> Get pointer to item data */
		Item_data = (struct Item_data *) MEM_Claim_pointer(Item_data_handle);
		Item_data += Packet->Index - 1;
	}

	return Item_data;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Free_item_data
 * FUNCTION  : Free item data after calling Get_item_data.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 16:47
 * LAST      : 03.04.95 16:47
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Free_item_data(void)
{
	MEM_Free_pointer(Item_data_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_item_name
 * FUNCTION  : Get an item's name.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 17:57
 * LAST      : 03.04.95 17:57
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 *             UNCHAR *Name - Pointer to name buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The buffer should be at least ITEM_NAME_LENGTH + 1 bytes long.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_item_name(struct Item_packet *Packet, UNCHAR *Name)
{
	UNCHAR *Item_names;

	/* Get item name data */
	Item_names = (UNCHAR *) MEM_Claim_pointer(Item_names_handle);

	/* Get specific item's name */
	Item_names += (((Packet->Index - 1) * MAX_LANGUAGES) + Language)
	 * ITEM_NAME_LENGTH;

	/* Copy item name */
	strcpy(Name, Item_names);

	MEM_Free_pointer(Item_names_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_item_packet
 * FUNCTION  : Build an item packet.
 * FILE      : ITMLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.06.95 15:16
 * LAST      : 19.06.95 15:16
 * INPUTS    : struct Item_packet *Packet - Pointer to item packet.
 *             UNSHORT Item_index - Item index.
 *             UNSHORT Quantity - Quantity.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will automatically set all the other elements
 *              of the item packet to their correct values (as specified in
 *              the item data).
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Build_item_packet(struct Item_packet *Packet, UNSHORT Item_index,
 UNSHORT Quantity)
{
	struct Item_data *Item_data;

	/* Clear target packet */
	Clear_packet(Packet);

	/* Insert data in packet */
	Packet->Quantity	= Quantity;
	Packet->Index		= Item_index;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Initialize rest of data */
	Packet->Charges			= Item_data->Init_charges;
	Packet->Nr_enchantments	= Item_data->Init_enchantment_normal;
	Packet->Flags				= Item_data->Init_packet_flags;

	Free_item_data();
}

