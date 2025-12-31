/************
 * NAME     : PLACES.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 16-08-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : PLACES.H
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

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
#include <PLACES.H>
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
#include <INPUTNR.H>
#include <TEXTWIN.H>
#include <MEMBRSEL.H>
#include <ITEMSEL.H>
#include <MERCHANT.H>
#include <SELWIN.H>
#include <MAGIC.H>
#include <CAMP.H>

/*
 ** Type definitions *******************************************************
 */

typedef void (*Place_handler) (void);

/*
 ** Prototypes *************************************************************
 */

/* Place handlers */
void Train_skill_place_handler(void);
void Heal_LP_place_handler(void);
void Heal_condition_place_handler(void);
void Remove_cursed_item_place_handler(void);
void Examine_item_place_handler(void);
void Recharge_item_place_handler(void);
void Rent_room_place_handler(void);
void Merchant_place_handler(void);
void Buy_food_place_handler(void);
void Buy_transport_place_handler(void);
void Learn_spell_place_handler(void);
void Repair_item_place_handler(void);

UNSHORT Heal_LP_member_evaluator(MEM_HANDLE Char_handle);

UNSHORT Heal_condition_member_evaluator(MEM_HANDLE Char_handle);
UNSHORT Heal_condition_evaluator(UNSHORT Index, UNBYTE *Data);
void Draw_condition(struct OPM *OPM, struct Object *Object, UNSHORT Index,
 UNBYTE *Data, BOOLEAN Blocked);

UNSHORT Remove_cursed_item_member_evaluator(MEM_HANDLE Char_handle);

UNSHORT Examine_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

UNSHORT Recharge_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

UNSHORT Learn_spell_evaluator(UNSHORT Index, UNBYTE *Data);
void Draw_learn_spell(struct OPM *OPM, struct Object *Object, UNSHORT Index,
 UNBYTE *Data, BOOLEAN Blocked);

UNSHORT Repair_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

/*
 ** Global variables *******************************************************
 */

/* Place parameters */
UNSHORT Place_type;
UNSHORT Place_selector_text_block_nr;
UNSHORT Place_buy_req_text_block_nr;
UNSHORT Place_action_text_block_nr;
UNSHORT Place_value;
UNSHORT Place_price;
UNSHORT Place_merchant_data_nr;

/* Text handle used for all place texts */
MEM_HANDLE Place_text_handle;

/* Handle of place "victim" character data */
static MEM_HANDLE Place_victim_handle;
static UNSHORT Place_victim_member_nr;

/* Total price of current place service */
/* (needed by text command) */
UNLONG Place_total_price;

/* Place handler list */
Place_handler Place_handlers[MAX_PLACE_TYPES] = {
	Train_skill_place_handler,
	Heal_LP_place_handler,
	Heal_condition_place_handler,
	Remove_cursed_item_place_handler,
	Examine_item_place_handler,
	Recharge_item_place_handler,
	Rent_room_place_handler,
	Merchant_place_handler,
	Buy_food_place_handler,
	Merchant_place_handler,		  	/* Distinction is made inside Merchant handler */
	Buy_transport_place_handler,
	Learn_spell_place_handler,
	Repair_item_place_handler
};

/* Heal condition price percentages */
static UNSHORT Heal_condition_prices[MAX_CONDITIONS] = {
	0,		/* UNCONSCIOUS	*/
	75,	/* POISONED		*/
	150,	/* DISEASED		*/
	0,		/* EXHAUSTED	*/
	0,		/* LAMED			*/
	0,		/* FLEEING		*/
	40,	/* STONED		*/
	100,	/* BLIND			*/
	0,		/* PANICKED		*/
	0,		/* ASLEEP		*/
	120,	/* INSANE		*/
	0		/* IRRITATED	*/
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_place_action
 * FUNCTION  : Execute a place action.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 21:37
 * LAST      : 17.08.95 11:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_place_action(void)
{
	struct Event_context *Context;
	struct Event_block *Event;

	/* Clear success flag */
	Clear_success_flag();

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get handle of text file */
	Place_text_handle = Context->Text_handle;

	/* Get current event block */
	Event = &(Context->Data);

	/* Get place parameters */
	Place_type							= (UNSHORT) Event->Byte_1;
	Place_selector_text_block_nr	= (UNSHORT) Event->Byte_2;
	Place_buy_req_text_block_nr	= (UNSHORT) Event->Byte_3;
	Place_action_text_block_nr		= (UNSHORT) Event->Byte_4;
	Place_value							= (UNSHORT) Event->Byte_5;
	Place_price							= (UNSHORT) Event->Word_6;
	Place_merchant_data_nr			= (UNSHORT) Event->Word_8;

	/* Set place "victim" data */
	Place_victim_handle		= Active_char_handle;
	Place_victim_member_nr	= PARTY_DATA.Active_member;

	/* Set text subject */
	Subject_char_handle = Place_victim_handle;

	/* Legal place type ? */
	if (Place_type < MAX_PLACE_TYPES)
	{
		/* Yes -> Execute place handler */
		if (Place_handlers[Place_type])
			Place_handlers[Place_type]();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Train_skill_place_handler
 * FUNCTION  : Train skill place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:39
 * LAST      : 22.10.95 22:14
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Train_skill_place_handler(void)
{
	UNLONG Party_gold;
	UNLONG Total_price;
	SISHORT Max_delta_skill;
	UNSHORT TP;
	UNSHORT Delta_skill;
	UNBYTE *Text_ptr;

	/* Get the victim character's training points */
	TP = Get_TP(Place_victim_handle);

	/* Does the victim character have any training points ? */
	if (TP)
	{
		/* Yes -> Calculate by how many percent this skill may be increased */
		Max_delta_skill = Get_max_skill(Place_victim_handle, Place_value) -
		 Get_real_skill(Place_victim_handle, Place_value);

		/* Any ? */
		if (Max_delta_skill > 0)
		{
			/* Yes -> Does the victim character have enough training points
			  for this ? */
			Max_delta_skill = min(Max_delta_skill, TP);

			/* How many training points can the party afford ? */
			Party_gold = Get_party_gold();
			if (Place_price)
			{
				Max_delta_skill = min((UNSHORT) Max_delta_skill,
				 (Party_gold / Place_price));
			}

			/* Any ? */
			if (Max_delta_skill)
			{
				/* Yes -> Was a selector text given ? */
				if (Place_selector_text_block_nr != 0xFF)
				{
					/* Yes -> Get text file address */
					Text_ptr = MEM_Claim_pointer(Place_text_handle);

					/* Find text block */
					Text_ptr = Find_text_block
					(
						Text_ptr,
						Place_selector_text_block_nr
					);

					/* How many exactly ? */
					Delta_skill = (UNSHORT) Input_number
					(
						1,
						0,
						(SILONG) Max_delta_skill,
						Text_ptr
					);

					MEM_Free_pointer(Place_text_handle);
				}
				else
				{
					/* No -> How many exactly ? */
					Delta_skill = (UNSHORT) Input_number
					(
						1,
						0,
						(SILONG) Max_delta_skill,
						System_text_ptrs[556]
					);
				}

				/* Any ? */
				if (Delta_skill)
				{
					/* Yes -> Calculate total price */
					Total_price = Delta_skill * Place_price;

					/* Does the player want to buy ? */
					if (Place_buy_requester(Total_price))
					{
						/* Yes -> Train skill */
						Set_skill
						(
							Place_victim_handle,
							Place_value,
							Get_real_skill(Place_victim_handle, Place_value) +
							 (SISHORT) Delta_skill
						);

						/* Decrease number of training points */
						Set_TP(Place_victim_handle, TP - Delta_skill);

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
							Do_text_window(System_text_ptrs[558]);
						}
					}
				}
			}
			else
			{
				/* Not enough money -> Insult the player */
				Do_text_window(System_text_ptrs[555]);
			}
		}
		else
		{
			/* Skill at maximum -> Insult the player */
			Do_text_window(System_text_ptrs[554]);
		}

		MEM_Free_pointer(Place_victim_handle);
	}
	else
	{
		/* No training points -> Insult the player */
		Do_text_window(System_text_ptrs[553]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Heal_LP_place_handler
 * FUNCTION  : Heal LP place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:39
 * LAST      : 06.10.95 13:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Heal_LP_place_handler(void)
{
	UNLONG Party_gold;
	UNLONG Total_price;
	SISHORT Max_delta_LP;
	UNSHORT Delta_LP;
	UNSHORT Selected_party_member;
	UNBYTE *Text_ptr;

	/* Was a selector text given ? */
	if (Place_selector_text_block_nr != 0xFF)
	{
		/* Yes -> Get text file address */
		Text_ptr = MEM_Claim_pointer(Place_text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Place_value);

		/* Heal which party member ? */
		Selected_party_member = Select_party_member(Text_ptr,
		 Heal_LP_member_evaluator,	0);

		MEM_Free_pointer(Place_text_handle);
	}
	else
	{
		/* No -> Heal which party member ? */
		Selected_party_member = Select_party_member(System_text_ptrs[563],
		 Heal_LP_member_evaluator,	0);
	}

	/* Anyone selectable ? */
	if (Selected_party_member)
	{
		/* Yes -> Anyone selected ? */
		if (Selected_party_member != 0xFFFF)
		{
			/* Yes -> Set place "victim" data */
			Place_victim_handle		= Party_char_handles[Selected_party_member - 1];
			Place_victim_member_nr	= Selected_party_member;

			/* Set text subject */
			Subject_char_handle = Place_victim_handle;

			/* Calculate by how much the life points may be increased */
			Max_delta_LP = Get_max_LP(Place_victim_handle) -
			 Get_LP(Place_victim_handle);

			/* How many LP can the party afford ? */
			Party_gold = Get_party_gold();
			if (Place_price)
			{
				Max_delta_LP = min((UNSHORT) Max_delta_LP,
				 (Party_gold / Place_price));
			}

			/* Any ? */
		   if (Max_delta_LP > 0)
			{
				#if FALSE
				/* Yes -> Was a second selector text given ? */
				if (Place_value != 0xFF)
				{
					/* Yes -> Get text file address */
					Text_ptr = MEM_Claim_pointer(Place_text_handle);

					/* Find text block */
					Text_ptr = Find_text_block(Text_ptr, Place_value);

					/* How many life-points exactly ? */
					Delta_LP = (UNSHORT) Input_number(1, 0, (SILONG) Max_delta_LP,
					 Text_ptr);

					MEM_Free_pointer(Place_text_handle);
				}
				else
				#endif
				{
					/* No -> How many LP exactly ? */
					Delta_LP = (UNSHORT) Input_number(1, 0,
					 (SILONG) Max_delta_LP, System_text_ptrs[560]);
				}

				/* Any ? */
				if (Delta_LP)
				{
					/* Yes -> Calculate total price */
					Total_price = Delta_LP * Place_price;

					/* Does the player want to buy ? */
					if (Place_buy_requester(Total_price))
					{
						/* Yes -> Heal LP */
						Do_healing(Place_victim_member_nr, Delta_LP);

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
							Do_text_window(System_text_ptrs[561]);
						}
					}
				}
			}
			else
			{
				/* Not enough money -> Insult the player */
				Do_text_window(System_text_ptrs[555]);
			}
		}
	}
	else
	{
		/* Everyone healthy -> Insult the player */
		Do_text_window(System_text_ptrs[626]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Heal_LP_member_evaluator
 * FUNCTION  : Check if a party member's LP can be healed
 *              (member select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 13:57
 * LAST      : 14.09.95 20:33
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Message nr / 0 = member is OK
 *              / 0xFFFF = member is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Heal_LP_member_evaluator(MEM_HANDLE Char_handle)
{
	UNSHORT Message_nr;

	/* Completely healthy ? */
	if (Get_LP(Char_handle) >= Get_max_LP(Char_handle))
	{
		/* Yes -> "Cannot heal" */
		Message_nr = 559;
	}
	else
	{
		/* No -> Member is OK */
		Message_nr = 0;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Heal_condition_place_handler
 * FUNCTION  : Heal condition place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 21:30
 * LAST      : 13.10.95 13:41
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Heal_condition_place_handler(void)
{
	UNLONG Total_price;
	UNSHORT Selected_condition;
	UNSHORT Selected_party_member;
	UNBYTE *Text_ptr;

	/* Was a selector text given ? */
	if (Place_selector_text_block_nr != 0xFF)
	{
		/* Yes -> Get text file address */
		Text_ptr = MEM_Claim_pointer(Place_text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Place_value);

		/* Heal which party member ? */
		Selected_party_member = Select_party_member(Text_ptr,
		 Heal_condition_member_evaluator, 0);

		MEM_Free_pointer(Place_text_handle);
	}
	else
	{
		/* No -> Heal which party member ? */
		Selected_party_member = Select_party_member(System_text_ptrs[563],
		 Heal_condition_member_evaluator, 0);
	}

	/* Anyone selectable ? */
	if (Selected_party_member)
	{
		/* Yes -> Anyone selected ? */
		if (Selected_party_member != 0xFFFF)
		{
			/* Yes -> Set place "victim" data */
			Place_victim_handle		= Party_char_handles[Selected_party_member - 1];
			Place_victim_member_nr	= Selected_party_member;

			/* Set text subject */
			Subject_char_handle = Place_victim_handle;

			#if FALSE
			/* Was a second selector text given ? */
			if (Place_value != 0xFF)
			{
				/* Yes -> Get text file address */
				Text_ptr = MEM_Claim_pointer(Place_text_handle);

				/* Find text block */
				Text_ptr = Find_text_block(Text_ptr, Place_value);

				/* Heal which condition ? */
				Selected_condition = Do_select_window
				(
					Text_ptr,
					Heal_condition_evaluator,
					Draw_condition,
					MAX_CONDITIONS,
					120,
					12,
					(UNBYTE *) &Place_victim_handle
				);

				MEM_Free_pointer(Place_text_handle);
			}
			else
			#endif
			{
				/* No -> Heal which condition ? */
				Selected_condition = Do_select_window
				(
					System_text_ptrs[608],
					Heal_condition_evaluator,
					Draw_condition,
					MAX_CONDITIONS,
					120,
					12,
					(UNBYTE *) &Place_victim_handle
				);
			}

			/* Any selected ? */
		   if (Selected_condition != 0xFFFF)
			{
				/* Yes -> Calculate total price */
				Total_price = (Heal_condition_prices[Selected_condition] *
				 Place_price) / 100;

				/* Does the player want to buy ? */
				if (Place_buy_requester(Total_price))
				{
					/* Yes -> Heal condition */
					Clear_condition(Place_victim_handle, Selected_condition);

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
						Do_text_window(System_text_ptrs[625]);
					}
				}
			}
		}
	}
	else
	{
		/* Everyone healthy -> Insult the player */
		Do_text_window(System_text_ptrs[626]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Heal_condition_member_evaluator
 * FUNCTION  : Check if a party member's conditions can be healed
 *              (member select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.09.95 15:44
 * LAST      : 03.09.95 15:44
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Message nr / 0 = member is OK
 *              / 0xFFFF = member is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Heal_condition_member_evaluator(MEM_HANDLE Char_handle)
{
	UNSHORT Message_nr;

	/* Any conditions ? */
	if (Get_conditions(Char_handle))
	{
		/* Yes -> Member is "OK" */
		Message_nr = 0;
	}
	else
	{
		/* No -> "Cannot heal" */
		Message_nr = 559;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Heal_condition_evaluator
 * FUNCTION  : Evaluator for healing conditions (select window).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 21:48
 * LAST      : 07.10.95 23:05
 * INPUTS    : UNSHORT Index - Selectable index (0...).
 *             UNBYTE *Data - Pointer to data passed to select window.
 * RESULT    : UNSHORT : Blocked message number / 0 = entry is OK /
 *              0xFFFF = entry is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Heal_condition_evaluator(UNSHORT Index, UNBYTE *Data)
{
	MEM_HANDLE Char_handle;
	UNSHORT Conditions;
	UNSHORT Message_nr;

	/* Can this condition be healed ? */
	if (Heal_condition_prices[Index])
	{
		/* Yes -> Get handle of victim character data */
		Char_handle = *((MEM_HANDLE *) Data);

		/* Get conditions of victim */
		Conditions = Get_conditions(Char_handle);

		/* Does the victim have this condition ? */
		if (Conditions & (1 << Index))
		{
			/* Yes -> Entry is OK */
			Message_nr = 0;
		}
		else
		{
			/* No -> Entry is absent */
			Message_nr = 0xFFFF;
		}
	}
	else
	{
		/* No -> Entry is absent */
		Message_nr = 0xFFFF;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_condition
 * FUNCTION  : Heal condition draw function (select window).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 21:50
 * LAST      : 03.09.95 15:29
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             struct Object *Object - Pointer to object data.
 *             UNSHORT Index - Selectable index (0...).
 *             UNBYTE *Data - Pointer to data passed to select window.
 *             BOOLEAN Blocked - Set if the entry is blocked.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_condition(struct OPM *OPM, struct Object *Object, UNSHORT Index,
 UNBYTE *Data, BOOLEAN Blocked)
{
	if (Blocked)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print condition name */
	Print_string(OPM, Object->X + 2, Object->Y + 1,
	 System_text_ptrs[609 + Index]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_cursed_item_place_handler
 * FUNCTION  : Remove cursed items place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 13:51
 * LAST      : 13.10.95 13:41
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_cursed_item_place_handler(void)
{
	UNSHORT Selected_party_member;

	/* Heal which party member ? */
	Selected_party_member = Select_party_member(System_text_ptrs[745],
	 Remove_cursed_item_member_evaluator, 0);

	/* Anyone selectable ? */
	if (Selected_party_member)
	{
		/* Yes -> Anyone selected ? */
		if (Selected_party_member != 0xFFFF)
		{
			/* Yes -> Set place "victim" data */
			Place_victim_handle		= Party_char_handles[Selected_party_member - 1];
			Place_victim_member_nr	= Selected_party_member;

			/* Set text subject */
			Subject_char_handle = Place_victim_handle;

			/* Yes -> Does the player want to buy ? */
			if (Place_buy_requester(Place_price))
			{
				/* Yes -> Remove cursed items */
				Remove_cursed_items(Place_victim_handle);

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
					Do_text_window(System_text_ptrs[562]);
				}
			}
		}
	}
	else
	{
		/* No -> Insult the player */
		Do_text_window(System_text_ptrs[744]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_cursed_item_member_evaluator
 * FUNCTION  : Check if a party member carries any cursed items
 *              (member select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 19:58
 * LAST      : 10.10.95 19:58
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Message nr / 0 = member is OK
 *              / 0xFFFF = member is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Remove_cursed_item_member_evaluator(MEM_HANDLE Char_handle)
{
	UNSHORT Message_nr;

	/* Has any cursed items ? */
	if (Has_cursed_items(Char_handle))
	{
		/* Yes -> Member is OK */
		Message_nr = 0;
	}
	else
	{
		/* No -> "Cannot heal" */
		Message_nr = 559;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Examine_item_place_handler
 * FUNCTION  : Examine item place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 13:53
 * LAST      : 17.08.95 14:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Examine_item_place_handler(void)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNLONG Total_price;
	UNSHORT Selected_item_index;
	UNBYTE *Text_ptr;

	/* Was a selector text given ? */
	if (Place_selector_text_block_nr != 0xFF)
	{
		/* Yes -> Get text file address */
		Text_ptr = MEM_Claim_pointer(Place_text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Place_value);

		/* Examine which item ? */
		Selected_item_index = Select_party_member_item
		(
			Place_victim_member_nr,
			Text_ptr,
			Examine_item_evaluator
		);

		MEM_Free_pointer(Place_text_handle);
	}
	else
	{
		/* No -> Examine which item ? */
		Selected_item_index = Select_party_member_item
		(
			Place_victim_member_nr,
			System_text_ptrs[564],
			Examine_item_evaluator
		);
	}

	/* Any item selected ? */
	if (Selected_item_index != 0xFFFF)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Place_victim_handle);

		/* Get item data */
		if (Selected_item_index < ITEMS_ON_BODY)
		{
			Item_data = Get_item_data(&(Char->Body_items[Selected_item_index - 1]));
		}
		else
		{
			Item_data = Get_item_data(&(Char->Backpack_items[Selected_item_index -
			 ITEMS_ON_BODY - 1]));
		}

		/* Calculate total price */
		Total_price = (Item_data->Price * Place_price) / 100;

		/* Does the player want to buy ? */
		if (Place_buy_requester(Total_price))
		{
			/* Yes -> Examine item */
			if (Selected_item_index < ITEMS_ON_BODY)
			{
				Char->Body_items[Selected_item_index - 1].Flags |= MAGIC_CHECK;
			}
			else
			{
				Char->Backpack_items[Selected_item_index - ITEMS_ON_BODY - 1].Flags |= MAGIC_CHECK;
			}

			/* Show */

			/* Was an action text given ? */
			if (Place_action_text_block_nr != 0xFF)
			{
				/* Yes -> Print it */
				Do_text_file_window(Place_text_handle,	Place_action_text_block_nr);
			}
			else
			{
				/* No -> Print standard text */
				Do_text_window(System_text_ptrs[566]);
			}
		}
		MEM_Free_pointer(Place_victim_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Examine_item_evaluator
 * FUNCTION  : Check if an item can be examined (item select window evaluator).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 14:09
 * LAST      : 17.08.95 14:09
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Examine_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet)
{
	UNSHORT Message_nr;

	/* Has the item already been examined ? */
	if (Packet->Flags & MAGIC_CHECK)
	{
		/* Yes -> "Has already been examined" */
		Message_nr = 565;
	}
	else
	{
		/* No -> Item is OK */
		Message_nr = 0;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Recharge_item_place_handler
 * FUNCTION  : Recharge item place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 14:23
 * LAST      : 13.10.95 13:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Recharge_item_place_handler(void)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	struct Item_packet *Packet;
	UNLONG Party_gold;
	UNLONG Total_price;
	SISHORT Max_delta_charges;
	UNSHORT Selected_item_index;
	UNSHORT Delta_charges;
	UNBYTE *Text_ptr;

	/* Was a selector text given ? */
	if (Place_selector_text_block_nr != 0xFF)
	{
		/* Yes -> Get text file address */
		Text_ptr = MEM_Claim_pointer(Place_text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Place_value);

		/* Recharge which item ? */
		Selected_item_index = Select_party_member_item
		(
			Place_victim_member_nr,
			Text_ptr,
			Recharge_item_evaluator
		);

		MEM_Free_pointer(Place_text_handle);
	}
	else
	{
		/* No -> Recharge which item ? */
		Selected_item_index = Select_party_member_item
		(
			Place_victim_member_nr,
			System_text_ptrs[569],
			Recharge_item_evaluator
		);
	}

	/* Any item selected ? */
	if (Selected_item_index != 0xFFFF)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Place_victim_handle);

		/* Get item packet */
		if (Selected_item_index < ITEMS_ON_BODY)
		{
			Packet = &(Char->Body_items[Selected_item_index - 1]);
		}
		else
		{
			Packet = &(Char->Backpack_items[Selected_item_index -
			 ITEMS_ON_BODY - 1]);
		}

		/* Get item data */
 		Item_data = Get_item_data(Packet);

		/* Calculate how many charges can be added */
		Max_delta_charges = Item_data->Max_charges - Packet->Charges;

		/* How many charges can the party afford ? */
		Party_gold = Get_party_gold();
		if (Place_price)
		{
			Max_delta_charges = min((UNSHORT) Max_delta_charges,
			 (Party_gold / Place_price));
		}

		/* Any ? */
		if (Max_delta_charges > 0)
		{
			#if FALSE
			/* Yes -> Was a second selector text given ? */
			if (Place_value != 0xFF)
			{
				/* Yes -> Get text file address */
				Text_ptr = MEM_Claim_pointer(Place_text_handle);

				/* Find text block */
				Text_ptr = Find_text_block(Text_ptr, Place_value);

				/* How many exactly ? */
				Delta_charges = (UNSHORT) Input_number
				(
					1,
					0,
					(SILONG) Max_delta_charges,
					Text_ptr
				);

				MEM_Free_pointer(Place_text_handle);
			}
			else
			#endif
			{
				/* No -> How many exactly ? */
				Delta_charges = (UNSHORT) Input_number
				(
					1,
					0,
					(SILONG) Max_delta_charges,
					System_text_ptrs[570]
				);
			}

			/* Any ? */
			if (Delta_charges)
			{
				/* Yes -> Calculate total price */
				Total_price = Delta_charges * Place_price;

				/* Does the player want to buy ? */
				if (Place_buy_requester(Total_price))
				{
					/* Yes -> Recharge */
					Packet->Charges += Delta_charges;

					/* Count up */
					Packet->Nr_enchantments++;

					/* Show */

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
						Do_text_window(System_text_ptrs[571]);
					}
				}
			}
		}
		else
		{
			/* Not enough money -> Insult the player */
			Do_text_window(System_text_ptrs[555]);
		}

		Free_item_data();
		MEM_Free_pointer(Place_victim_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Recharge_item_evaluator
 * FUNCTION  : Check if an item can be recharged (item select window evaluator).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 14:27
 * LAST      : 17.08.95 14:33
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Recharge_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr = 0;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is this a magical item ? */
	if (Item_data->Spell_nr)
	{
		/* Yes -> Is it already fully charged ? */
		if (Packet->Charges >= Item_data->Max_charges)
		{
			/* Yes -> "Already fully charged" */
			Message_nr = 567;
		}
		else
		{
			/* No -> Has the maximum number of enchantments been reached ? */
			if (Packet->Nr_enchantments >= Item_data->Max_enchantments)
			{
				/* Yes -> "Cannot be enchanted anymore" */
				Message_nr = 568;
			}
		}
	}
	else
	{
		/* No -> "Item is not magical" */
		Message_nr = 93;
	}

	Free_item_data();

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Rent_room_place_handler
 * FUNCTION  : Rent room place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 14:33
 * LAST      : 13.10.95 13:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Rent_room_place_handler(void)
{
	UNLONG Total_price;
	UNSHORT Total_members;

	/* Count alive party members */
	Total_members = Count_alive_party_members();

	/* Any ? */
	if (Total_members)
	{
		/* Yes -> Calculate total price */
		Total_price = Total_members * Place_price;

		/* Does the player want to buy ? */
		if (Place_buy_requester(Total_price))
		{
			/* Yes -> Was an action text given ? */
			if (Place_action_text_block_nr != 0xFF)
			{
				/* Yes -> Print it */
				Do_text_file_window(Place_text_handle,
					Place_action_text_block_nr);
			}
			else
			{
				/* No -> Print standard text */
				Do_text_window(System_text_ptrs[602]);
			}

			/* Do Camp screen (Inn mode) */
			Enter_Camp(TRUE);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merchant_place_handler
 * FUNCTION  : Merchant place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 16:54
 * LAST      : 17.08.95 16:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merchant_place_handler(void)
{
	Enter_Merchant();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Buy_food_place_handler
 * FUNCTION  : Buy food place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:39
 * LAST      : 13.10.95 13:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Buy_food_place_handler(void)
{
	UNLONG Party_gold;
	UNLONG Total_price;
	UNSHORT Max_delta_food;
	UNSHORT Delta_food;
	UNBYTE *Text_ptr;

	/* How many rations can the party afford ? */
	Party_gold = Get_party_gold();
	if (Place_price)
	{
		Max_delta_food = Party_gold / Place_price;
	}
	else
	{
		Max_delta_food = 32767;
	}

	/* Any ? */
	if (Max_delta_food)
	{
		/* Yes -> Was a selector text given ? */
		if (Place_selector_text_block_nr != 0xFF)
		{
			/* Yes -> Get text file address */
			Text_ptr = MEM_Claim_pointer(Place_text_handle);

			/* Find text block */
			Text_ptr = Find_text_block(Text_ptr, Place_selector_text_block_nr);

			/* How many food rations exactly ? */
			Delta_food = (UNSHORT) Input_number
			(
				1,
				0,
				(SILONG) Max_delta_food,
				Text_ptr
			);

			MEM_Free_pointer(Place_text_handle);
		}
		else
		{
			/* No -> How many food rations exactly ? */
			Delta_food = (UNSHORT) Input_number
			(
				1,
				0,
				(SILONG) Max_delta_food,
				System_text_ptrs[588]
			);
		}

		/* Any ? */
		if (Delta_food)
		{
			/* Yes -> Calculate total price */
			Total_price = Delta_food * Place_price;

			/* Does the player want to buy ? */
			if (Place_buy_requester(Total_price))
			{
				/* Yes -> Give food */
				Set_party_food(Get_party_food() + Delta_food);

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
					Do_text_window(System_text_ptrs[589]);
				}
			}
		}
	}
	else
	{
		/* Not enough money -> Insult the player */
		Do_text_window(System_text_ptrs[555]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Buy_transport_place_handler
 * FUNCTION  : Buy transport place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:39
 * LAST      : 17.08.95 10:39
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Buy_transport_place_handler(void)
{
	// NOT IMPLEMENTED
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Learn_spell_place_handler
 * FUNCTION  : Learn spell place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.09.95 16:20
 * LAST      : 13.10.95 13:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Learn_spell_place_handler(void)
{
	UNLONG Total_price;
	UNSHORT Selected_party_member = 0xFFFF;
	UNSHORT Selected_spell;
	UNSHORT Counter;
	UNSHORT i;

	/* Find a party member who knows this spell class */
	Counter = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Alive and kicking ? */
			if (Character_alive(Party_char_handles[i]))
			{
				/* Yes -> Knows this spell class ? */
				if (Spell_class_known(Party_char_handles[i], Place_value))
				{
					/* Yes -> "Select" this member */
					Selected_party_member = i + 1;

					/* Count up */
					Counter++;
				}
			}
		}
	}

	/* Does anyone know this spell class ? */
	if (Counter && (Selected_party_member != 0xFFFF))
	{
		/* Yes -> Set place "victim" data */
		Place_victim_handle		= Party_char_handles[Selected_party_member - 1];
		Place_victim_member_nr	= Selected_party_member;

		/* Set text subject */
		Subject_char_handle = Place_victim_handle;

		/* Learn which spell ? */
		Selected_spell = Do_select_window
		(
			System_text_ptrs[631],
			Learn_spell_evaluator,
			Draw_learn_spell,
			Spells_per_class[Place_value],
			120,
			12,
			(UNBYTE *) &Place_victim_handle
		);

		/* Any selected ? */
		if (Selected_spell != 0xFFFF)
		{
			/* Yes -> Calculate REAL spell index */
			Selected_spell++;

			/* Calculate total price */
			Total_price = Place_price * Get_spell_level(Place_value,
				Selected_spell);

			/* Does the player want to buy ? */
			if (Place_buy_requester(Total_price))
			{
				/* Yes -> Learn spell */
				Learn_spell(Place_victim_handle, Place_value, Selected_spell);

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
					Do_text_window(System_text_ptrs[190]);
				}
			}
		}
	}
	else
	{
		/* No-one selectable -> Insult player */
		Do_text_window(System_text_ptrs[628]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Learn_spell_evaluator
 * FUNCTION  : Evaluator for learning spells (select window).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.09.95 16:22
 * LAST      : 01.10.95 17:12
 * INPUTS    : UNSHORT Index - Selectable index (0...).
 *             UNBYTE *Data - Pointer to data passed to select window.
 * RESULT    : UNSHORT : Blocked message number / 0 = entry is OK /
 *              0xFFFF = entry is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Learn_spell_evaluator(UNSHORT Index, UNBYTE *Data)
{
	UNSHORT Message_nr;

	/* Does this spell exist ? */
	if (Spell_exists(Place_value, Index + 1))
	{
		/* Yes -> Is this spell already known ? */
		if (Spell_known(Place_victim_handle, Place_value, Index + 1))
		{
			/* Yes -> "Spell already known" */
			Message_nr = 188;
		}
		else
		{
			/* No -> Is this spell's level too high ? */
			if (Get_spell_level(Place_value, Index + 1) >
			 Get_level(Place_victim_handle))
			{
				/* Yes -> "Level is too high" */
				Message_nr = 189;
			}
			else
			{
				/* No -> Entry is OK */
				Message_nr = 0;
			}
		}
	}
	else
	{
		/* No -> Remove entry */
		Message_nr = 0xFFFF;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_learn_spell
 * FUNCTION  : Learn spell draw function (select window).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.09.95 16:23
 * LAST      : 03.09.95 16:23
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             struct Object *Object - Pointer to object data.
 *             UNSHORT Index - Selectable index (0...).
 *             UNBYTE *Data - Pointer to data passed to select window.
 *             BOOLEAN Blocked - Set if the entry is blocked.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_learn_spell(struct OPM *OPM, struct Object *Object, UNSHORT Index,
 UNBYTE *Data, BOOLEAN Blocked)
{
	UNCHAR String[100];

	if (Blocked)
		Set_ink(RED_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Build string */
	_bprintf
	(
		String,
		100,
		"%s (%u)",
		Get_spell_name(Place_value, Index + 1),
		Get_spell_level(Place_value, Index + 1)
	);

	/* Print spell name and level */
	Print_string(OPM, Object->X + 2, Object->Y + 1, String);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Repair_item_place_handler
 * FUNCTION  : Repair item place handler.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:39
 * LAST      : 13.10.95 13:35
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Repair_item_place_handler(void)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNLONG Total_price;
	UNSHORT Selected_item_index;
	UNBYTE *Text_ptr;

	/* Was a selector text given ? */
	if (Place_selector_text_block_nr != 0xFF)
	{
		/* Yes -> Get text file address */
		Text_ptr = MEM_Claim_pointer(Place_text_handle);

		/* Find text block */
		Text_ptr = Find_text_block(Text_ptr, Place_value);

		/* Repair which item ? */
		Selected_item_index = Select_party_member_item
		(
			Place_victim_member_nr,
			Text_ptr,
			Repair_item_evaluator
		);

		MEM_Free_pointer(Place_text_handle);
	}
	else
	{
		/* No -> Repair which item ? */
		Selected_item_index = Select_party_member_item
		(
			Place_victim_member_nr,
			System_text_ptrs[591],
			Repair_item_evaluator
		);
	}

	/* Any item selected ? */
	if (Selected_item_index != 0xFFFF)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Place_victim_handle);

		/* Get item data */
		if (Selected_item_index < ITEMS_ON_BODY)
		{
			Item_data = Get_item_data(&(Char->Body_items[Selected_item_index - 1]));
		}
		else
		{
			Item_data = Get_item_data(&(Char->Backpack_items[Selected_item_index -
			 ITEMS_ON_BODY - 1]));
		}

		/* Calculate total price */
		Total_price = (Item_data->Price * Place_price) / 100;

		/* Does the player want to buy ? */
		if (Place_buy_requester(Total_price))
		{
			/* Yes -> Repair item */
			if (Selected_item_index < ITEMS_ON_BODY)
			{
				Char->Body_items[Selected_item_index - 1].Flags &= ~BROKEN_ITEM;
			}
			else
			{
				Char->Backpack_items[Selected_item_index - ITEMS_ON_BODY - 1].Flags &= ~BROKEN_ITEM;
			}

			/* Show */
			// EFFECT

			/* Was an action text given ? */
			if (Place_action_text_block_nr != 0xFF)
			{
				/* Yes -> Print it */
				Do_text_file_window(Place_text_handle,	Place_action_text_block_nr);
			}
			else
			{
				/* No -> Print standard text */
				Do_text_window(System_text_ptrs[592]);
			}
		}
		MEM_Free_pointer(Place_victim_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Repair_item_evaluator
 * FUNCTION  : Check if an item can be repaired (item select window evaluator).
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.95 11:27
 * LAST      : 24.08.95 11:27
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Repair_item_evaluator(struct Character_data *Char, struct Item_packet *Packet)
{
	UNSHORT Message_nr;

	/* Is this item broken ? */
	if (Packet->Flags & BROKEN_ITEM)
	{
		/* No -> Item is OK */
		Message_nr = 0;
	}
	else
	{
		/* Yes -> "Is not broken!" */
		Message_nr = 590;
	}

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Place_buy_requester
 * FUNCTION  : Ask if the player wants to buy a place service.
 * FILE      : PLACES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 11:23
 * LAST      : 13.10.95 13:39
 * INPUTS    : UNLONG Price - Price of service.
 * RESULT    : BOOLEAN : Player wants to buy.
 * BUGS      : No known.
 * NOTES     : - This function will check whether the party has enough gold
 *             - When the player has decided to buy, the gold will be
 *              subtracted automatically and the success flag will be set.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Place_buy_requester(UNLONG Price)
{
	BOOLEAN Result;
	UNLONG Subblock_offsets[2];
	UNLONG Party_gold;
	UNCHAR String[200];
	UNBYTE *Text_ptr;

	/* Is the price zero ? */
	if (Price)
	{
		/* No -> Store price for PRIC text command */
		Place_total_price = Price;

		/* Get party gold */
		Party_gold = Get_party_gold();

		/* Does the player have enough money ? */
		if (Price <= Party_gold)
		{
			/* Yes -> Was a text block number given ? */
			if (Place_buy_req_text_block_nr != 0xFF)
			{
				/* Yes -> Get text file address */
				Text_ptr = MEM_Claim_pointer(Place_text_handle);

				/* Find text block */
				Text_ptr = Find_text_block(Text_ptr, Place_buy_req_text_block_nr);

				/* Build sub-block catalogue */
				Build_subblock_catalogue(Text_ptr, Subblock_offsets, 2);

				/* Button texts included ? */
				if (Subblock_offsets[0] && Subblock_offsets[1])
				{
					/* Yes -> Do requester with custom button texts */
					Result = Boolean_requester_with_buttons(Text_ptr, Text_ptr +
					 Subblock_offsets[0], Text_ptr + Subblock_offsets[1]);
				}
				else
				{
					/* No -> Do requester with default button texts */
					Result = Boolean_requester(Text_ptr);
				}
				MEM_Free_pointer(Place_text_handle);
			}
			else
			{
				/* No -> Build standard buy requester string */
				_bprintf
				(
					String,
					200,
					System_text_ptrs[557],
					Price / 10,
					Price % 10
				);

				/* Do requester */
				Result = Boolean_requester(String);
			}
		}
		else
		{
			/* No -> Insult the player */
			Do_text_window(System_text_ptrs[555]);

			Result = FALSE;
		}
	}
	else
	{
		/* Yes -> True */
		Result = TRUE;
	}

	/* Accepted ? */
	if (Result)
	{
		/* Yes -> Remove gold */
		Set_party_gold(max(0, Get_party_gold() - Price));

		/* Set success flag */
		Set_success_flag();
	}

	return Result;
}

