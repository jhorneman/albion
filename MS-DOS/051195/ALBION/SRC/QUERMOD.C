/************
 * NAME     : QUERMOD.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : QUERMOD.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <CONTROL.H>
#include <EVELOGIC.H>
#include <ITMLOGIC.H>
#include <EVENTS.H>
#include <QUERMOD.H>
#include <STATAREA.H>
#include <CHEST.H>
#include <DOOR.H>
#include <FONT.H>
#include <DIALOGUE.H>
#include <INVENTO.H>
#include <INVITEMS.H>
#include <INPUT.H>
#include <SOUND.H>
#include <COMBAT.H>
#include <XFTYPES.H>
#include <GRAPHICS.H>
#include <TEXTWIN.H>
#include <BOOLREQ.H>
#include <INPUTNR.H>
#include <SCRIPT.H>
#include <2D_MAP.H>
#include <MAP_PUM.H>
#include <ICCHANGE.H>

/* defines */

/* Datachange data types */
#define ATTRIBUTE_DC_TYPE			(0)
#define SKILL_DC_TYPE				(1)
#define LP_DC_TYPE					(2)
#define SP_DC_TYPE					(3)
#define APR_DC_TYPE					(4)
#define CONDITIONS_DC_TYPE			(5)
#define LEVEL_DC_TYPE				(6)
#define LANGUAGES_DC_TYPE			(7)
#define EP_DC_TYPE					(8)
#define TP_DC_TYPE					(9)
#define PORTRAIT_DC_TYPE			(10)
#define FULL_BODY_PIC_DC_TYPE		(11)
#define EVENT_SET_1_DC_TYPE		(12)
#define EVENT_SET_2_DC_TYPE		(13)
#define GFX_2D_DC_TYPE				(14)
#define TACTICAL_ICON_DC_TYPE		(15)
#define SPELL_DC_TYPE				(16)
#define MAX_LP_DC_TYPE				(17)
#define MAX_SP_DC_TYPE				(18)
#define ITEMS_DC_TYPE				(19)
#define GOLD_DC_TYPE					(20)
#define FOOD_DC_TYPE					(21)
#define MORALE_DC_TYPE				(22)

/* Datachange change types */
#define ACTIVE_MEMBER_DC			(0)
#define ENTIRE_PARTY_DC				(1)
#define SINGLE_MEMBER_DC			(2)
#define CURRENT_PART_DC				(3)
#define CURRENT_PART_VICTIM_DC	(4)
#define NPC_DC							(5)
#define INVENTORY_MEMBER_DC		(6)
#define SUBJECT_DC					(7)

/* Query and Modify types */
#define QUEST_BIT_QM_TYPE					(0)
#define EVENT_SAVE_BIT_QM_TYPE			(1)
#define DOOR_BIT_QM_TYPE					(2)
#define CHEST_BIT_QM_TYPE					(3)
#define CD_BIT_QM_TYPE						(4)
#define CHAR_IN_PARTY_QM_TYPE				(5)
#define ITEM_CARRIED_QM_TYPE				(6)
#define ITEM_USED_QM_TYPE					(7)
#define WORD_BIT_QM_TYPE					(8)
#define SUCCESS_QM_TYPE						(9)
#define WAITING_QM_TYPE	 					(10)
#define LIGHT_STATE_QM_TYPE				(11)
#define DIRECTION_QM_TYPE					(12)
#define WORD_SPOKEN_QM_TYPE				(13)
#define TRIGGERING_NPC_QM_TYPE			(14)
#define GOLD_CARRIED_QM_TYPE				(15)
#define FOOD_CARRIED_QM_TYPE				(16)
#define CHANCE_QM_TYPE						(17)
#define HOUR_QM_TYPE							(18)
#define ACTIVE_LEVEL_QM_TYPE				(19)
#define TRIGGER_MODE_QM_TYPE				(20)
#define MEMBER_CONSCIOUS_QM_TYPE			(21)
#define SEX_IN_PARTY_QM_TYPE				(22)
#define CLASS_IN_PARTY_QM_TYPE			(23)
#define RACE_IN_PARTY_QM_TYPE				(24)
#define ACTIVE_LANGUAGES_QM_TYPE			(25)
#define ACTIVE_MEMBER_QM_TYPE				(26)
#define DAY_QM_TYPE							(27)
#define BYTE_COUNTER_QM_TYPE				(28)
#define MAP_NR_QM_TYPE						(29)
#define STEP_QM_TYPE							(30)
#define REQUESTER_QM_TYPE					(31)
#define CHECK_ACTION_MONSTER_QM_TYPE	(32)
#define VISIBLE_QM_TYPE						(33)
#define CHECK_DIALOGUE_ACTION_QM_TYPE	(34)
#define BATTLE_SITUATION_QM_TYPE			(35)
#define ITEM_TYPE_USED_QM_TYPE			(36)
#define RELATIVE_WEIGHT_QM_TYPE			(37)
#define PROBE_SKILL_QM_TYPE				(38)
#define PROBE_ATTRIBUTE_QM_TYPE			(39)
#define _2D_MAP_QM_TYPE						(40)
#define X_QM_TYPE								(41)
#define Y_QM_TYPE								(42)
#define NUMBER_REQ_QM_TYPE					(43)

/* Modify modes */
#define CLEAR_MODIFY_MODE			(0)
#define SET_MODIFY_MODE				(1)
#define TOGGLE_MODIFY_MODE	  		(2)
#define LOAD_MODIFY_MODE			(3)
#define INC_MODIFY_MODE				(4)
#define DEC_MODIFY_MODE				(5)
#define INCPERC_MODIFY_MODE		(6)
#define DECPERC_MODIFY_MODE		(7)

/* Query modes */
#define LT_QUERY_MODE	(1)
#define LE_QUERY_MODE	(2)
#define EQ_QUERY_MODE	(3)
#define GE_QUERY_MODE	(4)
#define GT_QUERY_MODE	(5)

/* prototypes */

void Do_datachange(MEM_HANDLE Char_handle, UNSHORT Member_nr,
 struct Event_block *Event);

BOOLEAN Query_compare(UNSHORT Value, UNSHORT Query_mode, UNSHORT Number);

SILONG Modify_amount(SILONG Amount, SILONG Maximum, UNSHORT Modify_mode,
 SILONG Number);

BOOLEAN Is_modify_possible(SILONG Amount, SILONG Maximum, UNSHORT Modify_mode,
 SILONG Number);

SILONG Modify_amount_from_maximum(SILONG Amount, SILONG Maximum,
 UNSHORT Modify_mode, SILONG Number);

BOOLEAN Modify_carried_items(MEM_HANDLE Char_handle, UNSHORT Item_index,
 UNSHORT Number, UNSHORT Modify_mode);

BOOLEAN Modify_items_carried_by_party(UNSHORT Item_index, UNSHORT Number,
 UNSHORT Modify_mode);

/* global variables */

/* (needed for Change used item event) */
MEM_HANDLE Used_item_char_handle;
UNSHORT Used_item_slot_index;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_Used_Item_event
 * FUNCTION  : Change Used Item event handler.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 23:24
 * LAST      : 12.10.95 15:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_Used_Item_event(void)
{
	struct Event_action *Event_action;
	struct Event_context *Context;
	struct Event_block *Event;
	struct Character_data *Char;
	struct Item_packet *Packet;
	UNSHORT Used_item_index;
	BOOLEAN Change_is_possible = TRUE;
	BOOLEAN Result;
	UNCHAR String[300];
	UNCHAR Char_name[CHAR_NAME_LENGTH + 1];

	/* Clear success flag */
	Clear_success_flag();

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

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

	/* Does it contain more than one item ? */
	if (Packet->Quantity > 1)
	{
		/* Yes -> Does this character have any free slots ? */
		if (Inventory_full
		(
			&(Char->Backpack_items[0]),
			ITEMS_PER_CHAR
		))
		{
			/* No -> The used item cannot be changed */
			Change_is_possible = FALSE;

			/* Tell the player */
			Get_char_name(Used_item_char_handle, Char_name);
			_bprintf
			(
				String,
				300,
				System_text_ptrs[752],
				Char_name
			);
			Set_permanent_text(String);
		}
	}
	MEM_Free_pointer(Used_item_char_handle);

	/* Is changing possible ? */
	if (Change_is_possible)
	{
		/* Yes -> Determine used item index */
		Used_item_index = 0xFFFF;
		switch (Context->Source)
		{
			/* Map source */
			case MAP_EVENT_SOURCE:
			{
				/* Use item trigger ? */
				if (Context->Source_data.Map_source.Trigger == USE_ITEM_TRIGGER)
				{
					/* Yes -> Get used item index from trigger data */
					Used_item_index = Context->Source_data.Map_source.Value;
				}
				break;
			}
			/* Event set source */
			case SET_EVENT_SOURCE:
			{
				/* Get current event action data */
				Event_action = &(Event_action_stack[Event_action_stack_index]);

				/* Use item action ? */
				if (Event_action->Action_type == USE_ITEM_ACTION)
				{
					/* Yes -> Get used item index from action data */
					Used_item_index = Event_action->Action_value;
				}
				break;
			}
		}

		/* Could the used item index be determined ? */
		if (Used_item_index != 0xFFFF)
		{
			/* Yes -> Remove one item of the type that was used */
			Result = Modify_carried_items
			(
				Used_item_char_handle,
				Used_item_index,
				1,
				DEC_MODIFY_MODE
			);

			/* Success ? */
			if (Result)
			{
				/* Yes -> Add one item of the new type */
				Result = Modify_carried_items
				(
					Used_item_char_handle,
					Event->Word_6,
					1,
					INC_MODIFY_MODE
				);

				/* Success ? */
				if (Result)
				{
					/* Yes -> Set success flag */
					Set_success_flag();
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Datachange_event
 * FUNCTION  : Datachange event handler.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:36
 * LAST      : 16.10.95 12:07
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Datachange_event(void)
{
	struct Event_block *Event;
	MEM_HANDLE Char_handle;
	UNSHORT Member_index;
	UNSHORT Char_nr;
	UNSHORT i;

	/* Clear success flag */
	Clear_success_flag();

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get character index */
	Char_nr = (UNSHORT) Event->Byte_5;

	/* Whose data is changed ? */
	switch (Event->Byte_3)
	{
		/* Active party member */
		case ACTIVE_MEMBER_DC:
		{
			/* Is there an active member ? */
			if (PARTY_DATA.Active_member != 0xFFFF)
			{
				/* Yes -> Change data */
				Do_datachange
				(
					Active_char_handle,
					PARTY_DATA.Active_member,
					Event
				);
			}
			break;
		}
		/* Entire party */
		case ENTIRE_PARTY_DC:
		{
			/* Check all party members */
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Change party member's data */
					Do_datachange
					(
						Party_char_handles[i],
						i + 1,
						Event
					);

					/* Abort if Game Over */
					if (Game_over_flag)
						break;
				}
			}

			break;
		}
		/* A single party member */
		case SINGLE_MEMBER_DC:
		{
			/* Find character in party */
			Member_index = Find_character_in_party(Char_nr);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Change party member's data */
				Do_datachange
				(
					Party_char_handles[Member_index - 1],
					Member_index,
					Event
				);
			}
			else
			{
				/* No -> Load character data */
				Char_handle = Load_subfile(PARTY_CHAR, Char_nr);

				/* Success ? */
				if (Char_handle)
				{
					/* Yes -> Change data */
					Do_datachange
					(
						Char_handle,
						0xFFFF,
						Event
					);
				}
				else
				{
					/* No -> Report and exit */
					Error(ERROR_FILE_LOAD);

					Exit_program();
				}

				/* Save character data */
				Save_subfile(Char_handle, PARTY_CHAR, Char_nr);

				/* Free memory */
				MEM_Kill_memory(Char_handle);
			}
			break;
		}
		/* Current combat participant */
		case CURRENT_PART_DC:
		{
			/* In combat ? */
			if (In_Combat)
			{
				/* Yes -> Is the currently acting combat participant a party
				  member ? */
				if (Current_acting_part->Type == PARTY_PART_TYPE)
				{
					/* Yes -> Change data */
					Do_datachange
					(
						Current_acting_part->Char_handle,
						Current_acting_part->Number,
						Event
					);
				}
				else
				{
					/* No -> Change data */
					Do_datachange
					(
						Current_acting_part->Char_handle,
						0xFFFF,
						Event
					);
				}
			}
			break;
		}
		/* Current combat participant's victim */
		case CURRENT_PART_VICTIM_DC:
		{
			/* In combat ? */
			if (In_Combat)
			{
				/* Yes -> Is the current victim combat participant a party
				  member ? */
				if (Current_victim_part->Type == PARTY_PART_TYPE)
				{
					/* Yes -> Change data */
					Do_datachange
					(
						Current_victim_part->Char_handle,
						Current_victim_part->Number,
						Event
					);
				}
				else
				{
					/* No -> Change data */
					Do_datachange
					(
						Current_victim_part->Char_handle,
						0xFFFF,
						Event
					);
				}
			}
			break;
		}
		/* NPC */
		case NPC_DC:
		{
			/* Load character data */
			Char_handle = Load_subfile(NPC_CHAR, Char_nr);

			/* Success ? */
			if (Char_handle)
			{
				/* Yes -> Change data */
				Do_datachange
				(
					Char_handle,
					0xFFFF,
					Event
				);
			}
			else
			{
				/* No -> Report and exit */
				Error(ERROR_FILE_LOAD);

				Exit_program();
			}

			/* Save character data */
			Save_subfile(Char_handle, NPC_CHAR, Char_nr);

			/* Free memory */
			MEM_Kill_memory(Char_handle);

			break;
		}
		/* Inventory party member */
		case INVENTORY_MEMBER_DC:
		{
			/* In Inventory screen ? */
			if (In_Inventory)
			{
				/* Yes -> Change data */
				Do_datachange
				(
					Inventory_char_handle,
					Inventory_member,
					Event
				);
			}
			break;
		}
		/* The current subject */
		case SUBJECT_DC:
		{
			/* Is there a subject (the most rudimentary check) */
			if (Subject_char_handle)
			{
				/* Yes -> Find character in party */
				Member_index = Find_char_data_in_party(Subject_char_handle);

				/* Found ? */
				if (Member_index != 0xFFFF)
				{
					/* Yes -> Change party member's data */
					Do_datachange
					(
						Subject_char_handle,
						Member_index,
						Event
					);
				}
				else
				{
					/* No -> Change subject's data */
					Do_datachange
					(
						Subject_char_handle,
						0xFFFF,
						Event
					);
				}
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_datachange
 * FUNCTION  : Actually change data.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:57
 * LAST      : 15.10.95 18:14
 * INPUTS    : MEM_HANDLE Char_handle - Handle of victim's character data.
 *             UNSHORT Member_nr - Member index (1...6) / 0xFFFF = no party
 *              member.
 *             struct Event_block *Event - Pointer to current event block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_datachange(MEM_HANDLE Char_handle, UNSHORT Member_nr,
 struct Event_block *Event)
{
	struct Character_data *Char;
	BOOLEAN Result;
	UNSHORT Modify_mode;
	UNSHORT Index;
	UNSHORT Number;

	/* Set success flag */
	Set_success_flag();

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get data-change parameters */
	Modify_mode	= (UNSHORT) Event->Byte_2;
	Index			= Event->Word_6;
	Number		= Event->Word_8;

	/* Random number ? */
	if (Event->Byte_4)
	{
		/* Yes -> Randomize */
		Number = (rand() % Number) + 1;
	}

	/* Change which data ? */
	switch (Event->Byte_1)
	{
		/* Attribute */
		case ATTRIBUTE_DC_TYPE:
		{
			/* Modify */
			Set_attribute
			(
				Char_handle,
				Index,
				(SISHORT) Modify_amount_from_maximum
				(
					(SILONG) Get_real_attribute(Char_handle, Index),
					(SILONG) Get_max_attribute(Char_handle, Index),
					Modify_mode,
					(SILONG) Number
				)
			);
			break;
		}
		/* Skill */
		case SKILL_DC_TYPE:
		{
			/* Modify */
			Set_skill
			(
				Char_handle,
				Index,
				(SISHORT) Modify_amount_from_maximum
				(
					(SILONG) Get_real_skill(Char_handle, Index),
					(SILONG) Get_max_skill(Char_handle, Index),
					Modify_mode,
					(SILONG) Number
				)
			);
			break;
		}
		/* Life-points */
		case LP_DC_TYPE:
		{
			SISHORT Old_LP;
			SISHORT New_LP;

			/* Get old LP */
			Old_LP = Get_LP(Char_handle);

			/* Calculate new LP */
			New_LP = (SISHORT) Modify_amount_from_maximum
			(
				(SILONG) Old_LP,
				(SILONG) Get_max_LP(Char_handle),
				Modify_mode,
				(SILONG) Number
			);

			/* Is this a party member ? */
			if (Member_nr != 0xFFFF)
			{
				/* Yes -> Have the LP changed ? */
				if (Old_LP != New_LP)
				{
					/* Yes -> Damage or healing ? */
					if (Old_LP > New_LP)
					{
						/* Damage -> Do damage */
						Do_damage(Member_nr, Old_LP - New_LP);
					}
					else
					{
						/* Healing -> Do healing */
						Do_healing(Member_nr, New_LP - Old_LP);
					}
				}
			}
			else
			{
				/* No party member -> Just set new LP */
				Set_LP(Char_handle, New_LP);
			}
			break;
		}
		/* Spell-points */
		case SP_DC_TYPE:
		{
			/* Set new SP */
			Set_SP
			(
				Char_handle,
				(SISHORT) Modify_amount_from_maximum
				(
					(SILONG) Get_SP(Char_handle),
					(SILONG) Get_max_SP(Char_handle),
					Modify_mode,
					(SILONG) Number
				)
			);
			break;
		}
		/* Conditions */
		case CONDITIONS_DC_TYPE:
		{
			/* Which modify mode ? */
			switch (Modify_mode)
			{
				/* Clear bit */
				case CLEAR_MODIFY_MODE:
				{
					Clear_condition(Char_handle, Index);
					break;
				}
				/* Set bit */
				case SET_MODIFY_MODE:
				{
					Set_condition(Char_handle, Index);
					break;
				}
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
				{
					if (Get_conditions(Char_handle) & (1 << Index))
						Clear_condition(Char_handle, Index);
					else
						Set_condition(Char_handle, Index);
					break;
				}
			}
			break;
		}
		/* Languages */
		case LANGUAGES_DC_TYPE:
		{
			/* Which modify mode ? */
			switch (Modify_mode)
			{
				/* Clear bit */
				case CLEAR_MODIFY_MODE:
				{
					Char->Known_languages &= ~(1 << Index);
					break;
				}
				/* Set bit */
				case SET_MODIFY_MODE:
				{
					Char->Known_languages |= (1 << Index);
					break;
				}
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
				{
					Char->Known_languages ^= (1 << Index);
					break;
				}
			}
			break;
		}
		/* Experience points */
		case EP_DC_TYPE:
		{
			UNLONG EP;

			/* Calculate new EP */
			EP = Modify_amount
			(
				Get_EP(Char_handle),
				0x7FFFFFFF,
				Modify_mode,
				(SILONG) Number
			);

			/* Set */
			Set_EP(Char_handle, EP);

			break;
		}
		/* Training points */
		case TP_DC_TYPE:
		{
			UNSHORT TP;

			/* Calculate new TP */
			TP = Modify_amount
			(
				(SILONG) Get_TP(Char_handle),
				0x7FFF,
				Modify_mode,
				(SILONG) Number
			);

			/* Set */
			Set_TP(Char_handle, TP);

			break;
		}
		/* Event set #1 */
		case EVENT_SET_1_DC_TYPE:
		{
			/* Set new event set index */
			Set_event_set_index(Char_handle, 0, Number);
			break;
		}
		/* Event set #2 */
		case EVENT_SET_2_DC_TYPE:
		{
			/* Set new event set index */
			Set_event_set_index(Char_handle, 1, Number);
			break;
		}
		/* Spell */
		case SPELL_DC_TYPE:
		{
			/* Which modify mode ? */
			switch (Modify_mode)
			{
				/* Clear bit */
				case CLEAR_MODIFY_MODE:
				{
					Char->xKnown_spells[Index] &= ~(1L << Number);
					break;
				}
				/* Set bit */
				case SET_MODIFY_MODE:
				{
					Char->xKnown_spells[Index] |= (1L << Number);
					break;
				}
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
				{
					Char->xKnown_spells[Index] ^= (1L << Number);
					break;
				}
			}
			break;
		}
		/* Maximum life-points */
		case MAX_LP_DC_TYPE:
		{
			Set_max_LP
			(
				Char_handle,
				(SISHORT) Modify_amount
				(
					(SILONG) Get_real_max_LP(Char_handle),
					0x7FFF,
					Modify_mode,
					(SILONG) Number
				)
			);
			break;
		}
		/* Maximum spell-points */
		case MAX_SP_DC_TYPE:
		{
			Set_max_SP
			(
				Char_handle,
				(SISHORT) Modify_amount
				(
					(SILONG) Get_real_max_SP(Char_handle),
					0x7FFF,
					Modify_mode,
					(SILONG) Number
				)
			);
			break;
		}
		/* Carried items */
		case ITEMS_DC_TYPE:
		{
			Result = Modify_carried_items
			(
				Char_handle,
				Index,
				Number,
				Modify_mode
			);

			/* Success ? */
			if (!Result)
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}

			break;
		}
		/* Carried gold */
		case GOLD_DC_TYPE:
		{
			UNSHORT New_amount;

			/* Is this modification possible ? */
			if (Is_modify_possible
			(
				(SILONG) Char->Char_gold,
				32767,
				Modify_mode,
				Number
			))
			{
				/* Yes -> Modify */
				New_amount = (SISHORT) Modify_amount
				(
					(SILONG) Char->Char_gold,
					32767,
					Modify_mode,
					Number
				);

				/* In dialogue / not in combat ? */
				if (In_Dialogue && !In_Combat)
				{
					/* Yes -> Give or take ? */
					if (New_amount > Char->Char_gold)
					{
						/* Take -> Show moving gold */
						Show_moving_gold
						(
							360 - 36 - 8 + 12,
							5 + 8,
							10 + 12,
							5 + 8
						);
					}
					else
					{
						/* Give -> Show moving gold */
						Show_moving_gold
						(
							10 + 12,
							5 + 8,
							360 - 36 - 8 + 12,
							5 + 8
						);
					}
				}

				/* Set new amount of gold */
				Char->Char_gold = New_amount;
			}
			else
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}
			break;
		}
		/* Carried food */
		case FOOD_DC_TYPE:
		{
			UNSHORT New_amount;

			/* Is this modification possible ? */
			if (Is_modify_possible
			(
				(SILONG) Char->Char_food,
				32767,
				Modify_mode,
				Number
			))
			{
				/* Yes -> Modify */
				New_amount = (SISHORT) Modify_amount
				(
					(SILONG) Char->Char_food,
					32767,
					Modify_mode,
					Number
				);

				/* In dialogue / not in combat ? */
				if (In_Dialogue && !In_Combat)
				{
					/* Yes -> Give or take ? */
					if (New_amount > Char->Char_food)
					{
						/* Take -> Show moving food */
						Show_moving_food
						(
							360 - 36 - 8 + 12,
							5 + 8,
							10 + 12,
							5 + 8
						);
					}
					else
					{
						/* Give -> Show moving food */
						Show_moving_food
						(
							10 + 12,
							5 + 8,
							360 - 36 - 8 + 12,
							5 + 8
						);
					}
				}

				/* Set new amount of food */
				Char->Char_food = New_amount;
			}
			else
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}
			break;
		}
	}

	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Query_event
 * FUNCTION  : Query event handler.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:29
 * LAST      : 06.10.95 22:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Query_event(void)
{
	struct Event_block *Event;
	BOOLEAN Result = TRUE;
	UNSHORT Number;
	UNSHORT Extra;
	UNSHORT Query_mode;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get query parameters */
	Query_mode = (UNSHORT) Event->Byte_2;
	Extra = (UNSHORT) Event->Byte_3;
	Number = Event->Word_6;

	/* What kind of query ? */
	switch (Event->Byte_1)
	{
		case QUEST_BIT_QM_TYPE:
		{
			/* Read bit from quest bit array */
			Result = Read_bit_array(QUEST_BIT_ARRAY, Number);
			break;
		}
		case EVENT_SAVE_BIT_QM_TYPE:
		{
			UNLONG Bit_number;

			/* Use on current map ? */
			if (!Number)
			{
				Number = PARTY_DATA.Map_nr;
			}

			/* Calculate bit number */
			Bit_number = ((Number - 1) * EVENTS_PER_MAP) + Extra;

			/* Read bit from event save bit array */
			Result = Read_bit_array(EVENT_SAVE_BIT_ARRAY, Bit_number);
			break;
		}
		case DOOR_BIT_QM_TYPE:
		{
			/* Read bit from door bit array */
			Result = Read_bit_array(DOOR_BIT_ARRAY, Number);
			break;
		}
		case CHEST_BIT_QM_TYPE:
		{
			/* Read bit from chest bit array */
			Result = Read_bit_array(CHEST_BIT_ARRAY, Number);
			break;
		}
		case CD_BIT_QM_TYPE:
		{
			UNLONG Bit_number;

			/* Use on current map ? */
			if (!Number)
			{
				Number = PARTY_DATA.Map_nr;
			}

			/* Calculate bit number */
			Bit_number = ((Number - 1) * NPCS_PER_MAP) + Extra;

			/* Read bit from CD bit array */
			Result = Read_bit_array(CD_BIT_ARRAY, Bit_number);
			break;
		}
		case CHAR_IN_PARTY_QM_TYPE:
		{
			/* Check */
			Result = (Find_character_in_party(Number) != 0xFFFF);
			break;
		}
		case ITEM_CARRIED_QM_TYPE:
		{
			SISHORT Nr_items;
			UNSHORT i;

			/* Count items */
			Nr_items = 0;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Count party member's items */
					Nr_items += Count_items_on_body(Party_char_handles[i],
					 Number);
					Nr_items += Count_items_in_backpack(Party_char_handles[i],
					 Number);
				}
			}

			/* Compare */
			Result = Query_compare(Nr_items, Query_mode, Extra);
			break;
		}
		case ITEM_USED_QM_TYPE:
		{
			struct Event_context *Context;

			/* Default is FALSE */
			Result = FALSE;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Use item trigger ? */
				if (Context->Source_data.Map_source.Trigger == USE_ITEM_TRIGGER)
				{
					/* Yes -> Was the right item used ? */
					Result = (BOOLEAN)(Context->Source_data.Map_source.Value == Number);
					if (Result)
					{
						/* Yes -> Destroy used item */
						Destroy_used_item();
					}
					else
					{
						/* No -> Negative chain given ? */
						if (Event->Word_8 == 0xFFFF)
						{
							/* No -> Print standard "no can do" message */
							Set_permanent_message_nr(431);
						}
					}
				}
			}
			break;
		}
		case WORD_BIT_QM_TYPE:
		{
			/* Read bit from known words bit array */
			Result = Read_bit_array(KNOWN_WORDS_BIT_ARRAY, Number);
			break;
		}
		case SUCCESS_QM_TYPE:
		{
			struct Event_context *Context;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Get success flag */
			Result = (BOOLEAN) (Context->Flags & SUCCESS_FLAG);

			break;
		}
		case WAITING_QM_TYPE:
		{
			/* Check if the party is waiting */
			Result = Waiting_flag;
			break;
		}
		case LIGHT_STATE_QM_TYPE:
		{
			/* Compare map light state */
			Result = Query_compare(Current_map_light_state, Query_mode,
			 Number);
			break;
		}
		case DIRECTION_QM_TYPE:
		{
			/* Check party ? */
			if (Extra == 255)
			{
				/* Yes -> Compare party view direction */
				Result = Query_compare(PARTY_DATA.View_direction, Query_mode,
				 Number);
			}
			else
			{
				/* No -> 2D map ? */
				if (!_3D_map)
				{
					/* Yes -> Does this NPC exist ? */
					if (NPC_present(Extra))
					{
						/* Yes -> Compare NPC view direction */
						Result = Query_compare(VNPCs[Extra].Data._2D_NPC_data.Move.View_direction,
						 Query_mode, Number);
					}
					else
					{
						/* No -> False */
						Result = FALSE;
					}
				}
				else
				{
					/* No -> False */
					Result = FALSE;
				}
			}
			break;
		}
		case WORD_SPOKEN_QM_TYPE:
		{
			struct Event_context *Context;
			UNSHORT Word_instances[MAX_WORD_INSTANCES];
			UNSHORT Nr_instances;
			UNSHORT i;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Default is no reaction */
				Result = FALSE;

				/* Was a known word entered ? */
				if (Context->Source_data.Map_source.Value != 0xFFFE)
				{
					/* Yes -> Get references to selected word */
					Nr_instances = Search_references_to_merged_word(Context->Source_data.Map_source.Value,
					 Word_instances);

					/* Check all instances */
					for (i=0;i<Nr_instances;i++)
					{
						/* Is this the right word ? */
						if (Word_instances[i] == Number)
						{
							/* Yes -> Make all references known */
							for (i=0;i<Nr_instances;i++)
							{
								/* Mark this word as known */
								Write_bit_array
								(
									KNOWN_WORDS_BIT_ARRAY,
									Word_instances[i],
									SET_BIT_ARRAY
								);
							}

							/* Success */
							Result = TRUE;
							break;
						}
					}
				}

				/* Was the right word spoken ? */
				if (!Result)
				{
					/* No -> Negative chain given ? */
					if (Event->Word_8 == 0xFFFF)
					{
						/* No -> Print standard "no effect" message */
						Set_permanent_message_nr(432);
					}
				}
			}
			else
			{
				/* No -> False */
				Result = FALSE;
			}
			break;
		}
		case TRIGGERING_NPC_QM_TYPE:
		{
			struct Event_context *Context;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Did this NPC trigger this event ? */
				Result = (BOOLEAN)(Context->Source_data.Map_source.Value
				 == Number);
			}
			else
			{
				/* No -> False */
				Result = FALSE;
			}
			break;
		}
		case GOLD_CARRIED_QM_TYPE:
		{
			UNLONG Gold;

			/* Get party gold */
			Gold = Get_party_gold();

			/* Compare */
			Result = Query_compare(Gold, Query_mode, Number);
			break;
		}
		case FOOD_CARRIED_QM_TYPE:
		{
			struct Character_data *Char;
			UNSHORT Food, i;

			/* Count food */
			Food = 0;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Count party member's food rations */
					Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

					Food += Char->Char_food;

					MEM_Free_pointer(Party_char_handles[i]);
				}
			}

			/* Compare */
			Result = Query_compare(Food, Query_mode, Number);
			break;
		}
		case CHANCE_QM_TYPE:
		{
			/* Compare chance */
			Result = Query_compare(rand() % 100, Query_mode, Number);
			break;
		}
		case HOUR_QM_TYPE:
		{
			/* Compare hour X of Y */
			Result = Query_compare(PARTY_DATA.Hour % Extra, Query_mode,
			 Number);
			break;
		}
		case ACTIVE_LEVEL_QM_TYPE:
		{
			/* Compare active member's level */
			Result = Query_compare(Get_level(Active_char_handle), Query_mode,
			 Number);
			break;
		}
		case TRIGGER_MODE_QM_TYPE:
		{
			struct Event_context *Context;
			UNSHORT Trigger_mode;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Get trigger mode */
				Trigger_mode = Context->Source_data.Map_source.Trigger;

				/* Compare */
				Result = (Trigger_mode == Number);
			}
			else
			{
				/* No -> No trigger mode */
				Result = FALSE;
			}
			break;
		}
		case MEMBER_CONSCIOUS_QM_TYPE:
		{
			UNSHORT Member_index;

			Result = FALSE;

			/* Find character in party */
			Member_index = Find_character_in_party(Number);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Is this character capable ? */
				if (Character_capable(Party_char_handles[Member_index - 1]))
				{
					Result = TRUE;
				}
			}
			break;
		}
		case SEX_IN_PARTY_QM_TYPE:
		{
			UNSHORT i;

			/* Check party */
			Result = FALSE;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Right sex ? */
					if (Get_sex(Party_char_handles[i]) == Number)
					{
						/* Yes -> Exit */
						Result = TRUE;
						break;
					}
				}
			}
			break;
		}
		case CLASS_IN_PARTY_QM_TYPE:
		{
			UNSHORT i;

			/* Check party */
			Result = FALSE;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Right class ? */
					if (Get_class(Party_char_handles[i]) == Number)
					{
						/* Yes -> Exit */
						Result = TRUE;
						break;
					}
				}
			}
			break;
		}
		case RACE_IN_PARTY_QM_TYPE:
		{
			UNSHORT i;

			/* Check party */
			Result = FALSE;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Right race ? */
					if (Get_race(Party_char_handles[i]) == Number)
					{
						/* Yes -> Exit */
						Result = TRUE;
						break;
					}
				}
			}
			break;
		}
		case ACTIVE_LANGUAGES_QM_TYPE:
		{
			struct Character_data *Char;
			UNSHORT Languages;

			/* Get active member's languages */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Active_char_handle);

			Languages = Char->Known_languages;

			MEM_Free_pointer(Active_char_handle);

			/* Read bit from languages */
			Result = (BOOLEAN) Languages & (1 << Number);
			break;
		}
		case ACTIVE_MEMBER_QM_TYPE:
		{
			UNSHORT Member_index;

			Result = FALSE;

			/* Find character in party */
			Member_index = Find_character_in_party(Number);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Is active member ? */
				Result = (Member_index == PARTY_DATA.Active_member);
			}
			break;
		}
		case DAY_QM_TYPE:
		{
			/* Compare day X of Y */
			Result = Query_compare(PARTY_DATA.Day % Extra, Query_mode,
			 Number);
			break;
		}
		case BYTE_COUNTER_QM_TYPE:
		{
			/* Legal byte-counter index ? */
			if (Number < MAX_BYTE_COUNTERS)
			{
				/* Yes -> Compare byte counter */
				Result = Query_compare(PARTY_DATA.Byte_counters[Number],
				 Query_mode, Extra);
			}
			break;
		}
		case MAP_NR_QM_TYPE:
		{
			/* Compare map number */
			Result = Query_compare(PARTY_DATA.Map_nr, Query_mode, Number);
			break;
		}
		case STEP_QM_TYPE:
		{
			/* Compare step X of steps per day */
			Result = Query_compare(Current_step % Steps_per_day, Query_mode,
			 Number);
			break;
		}
		case REQUESTER_QM_TYPE:
		{
			struct Event_context *Context;
			MEM_HANDLE Text_handle;
			UNLONG Subblock_offsets[2];
			UNBYTE *Text_ptr;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Get memory handle of text file */
			Text_handle = Context->Text_handle;

			/* Get text file address */
			Text_ptr = MEM_Claim_pointer(Text_handle);

			/* Find text block */
			Text_ptr = Find_text_block(Text_ptr, Number);

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

			MEM_Free_pointer(Text_handle);
			break;
		}
		case VISIBLE_QM_TYPE:
		{
			SILONG Light_strength;

			/* Default is visible */
			Result = TRUE;

			/* Is the current map always dark ? */
			if (Current_map_light_state == ALWAYS_DARK)
			{
				/* Yes -> Get light strength */
				Light_strength = (SILONG) min(max(Get_light_spell_strength(),
				 Calculate_party_light()), 100);

				/* Not visible if light strength below 25% */
				if (Light_strength < 25)
					Result = FALSE;
			}
			break;
		}
		case CHECK_DIALOGUE_ACTION_QM_TYPE:
		{
			struct Event_action *Event_action;
			struct Event_context *Context;
			UNSHORT Value = 0;

			/* Default is FALSE */
			Result = FALSE;

			/* Get current event action data */
			Event_action = &(Event_action_stack[Event_action_stack_index]);

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a set context ? */
			if (Context->Source == SET_EVENT_SOURCE)
			{
				/* Yes -> Act depending on action type */
				switch(Event_action->Action_type)
				{
					case ASK_ABOUT_ACTION:
					{
						/* Get merged word index */
						Value = Get_merged_word_index(Event_action->Action_value);
						break;
					}
					case GIVE_ITEM_ACTION:
					{
						/* Get item index */
						Value = Event_action->Action_value;
						break;
					}
					case SELECT_ANSWER_ACTION:
					{
						/* Build value from text block and MC index */
						Value = Event_action->Action_value * 256 +
						 Event_action->Action_extra;
						break;
					}
				}

				/* Check dialogue log */
				Result = Find_dialogue_log_entry(Dialogue_char_type,
				 Dialogue_char_index, Event_action->Action_type, Value);
			}
			break;
		}
		case ITEM_TYPE_USED_QM_TYPE:
		{
			struct Event_context *Context;
			struct Item_packet Packet;
			UNSHORT Type;

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Build item packet */
				Build_item_packet(&Packet, Context->Source_data.Map_source.Value,
				 1);

				/* Get item type */
				Type = (UNSHORT) (Get_item_data(&Packet))->Type;
				Free_item_data();

				/* Was the right item type used ? */
				Result = (BOOLEAN)(Type == Number);
				if (Result)
				{
					/* Yes -> Destroy used item */

				}
				else
				{
					/* No -> Negative chain given ? */
					if (Event->Word_8 == 0xFFFF)
					{
						/* No -> Print standard "no can do" message */
						Set_permanent_message_nr(431);
					}
				}
			}
			else
			{
				/* No -> False */
				Result = FALSE;
			}
			break;
		}
		case PROBE_SKILL_QM_TYPE:
		{
			UNSHORT Skill;

			/* Get active member's skill */
			Skill = Get_skill(Active_char_handle, Extra);

			/* Compare skill */
			Result = Query_compare(Skill, Query_mode, Number);
			break;
		}
		case PROBE_ATTRIBUTE_QM_TYPE:
		{
			UNSHORT Attribute;

			/* Get active member's attribute */
			Attribute = Get_attribute(Active_char_handle, Extra);

			/* Compare attribute */
			Result = Query_compare(Attribute, Query_mode, Number);
			break;
		}
		case _2D_MAP_QM_TYPE:
		{
			/* Is this a 2D map ? */
			Result = !_3D_map;
			break;
		}
		case X_QM_TYPE:
		{
			/* Check party ? */
			if (Extra == 255)
			{
				/* Yes -> Compare party X-coordinate */
				Result = Query_compare(PARTY_DATA.X, Query_mode, Number);
			}
			else
			{
				/* No -> Does this NPC exist ? */
				if (NPC_present(Extra))
				{
					/* Yes -> Compare NPC X-coordinate */
					Result = Query_compare(VNPCs[Extra].Map_X, Query_mode,
					 Number);
				}
				else
				{
					/* No -> False */
					Result = FALSE;
				}
			}
			break;
		}
		case Y_QM_TYPE:
		{
			/* Check party ? */
			if (Extra == 255)
			{
				/* Yes -> Compare party Y-coordinate */
				Result = Query_compare(PARTY_DATA.Y, Query_mode, Number);
			}
			else
			{
				/* No -> Does this NPC exist ? */
				if (NPC_present(Extra))
				{
					/* Yes -> Compare NPC Y-coordinate */
					Result = Query_compare(VNPCs[Extra].Map_Y, Query_mode,
					 Number);
				}
				else
				{
					/* No -> False */
					Result = FALSE;
				}
			}
			break;
		}
		case NUMBER_REQ_QM_TYPE:
		{
			UNLONG Quantity;

			/* Cheat mode ? */
			if (Cheat_mode)
			{
				/* Yes -> Yay! */
				Result = TRUE;
			}
			else
			{
				/* No -> Input quantity */
				Quantity = (UNLONG) Input_number(0, 0, 9999,
				 System_text_ptrs[201]);

				/* Compare quantity */
				Result = Query_compare(Quantity, Query_mode, Number);
			}
			break;
		}
	}

	/* Query false ? */
	if (!Result)
	{
		/* Yes -> Follow negative chain */
		Event->Next_event_nr = Event->Word_8;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Query_compare
 * FUNCTION  : Compare query value with given number.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 19:17
 * LAST      : 28.12.94 19:17
 * INPUTS    : UNSHORT Value - Value (depends on query type).
 *             UNSHORT Query_mode - Query mode (from event data).
 *             UNSHORT Number - Number that value will be compared with
 *              (from event data).
 * RESULT    : BOOLEAN : Result of comparison.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Query_compare(UNSHORT Value, UNSHORT Query_mode, UNSHORT Number)
{
	BOOLEAN Result = FALSE;

	/* Which query mode ? */
	switch (Query_mode)
	{
		/* Less than */
		case LT_QUERY_MODE:
		{
			Result = (Value < Number);
			break;
		}
		/* Less than or equal */
		case LE_QUERY_MODE:
		{
			Result = (Value <= Number);
			break;
		}
		/* Equal */
		case EQ_QUERY_MODE:
		{
			Result = (Value == Number);
			break;
		}
		/* Greater than or equal */
		case GE_QUERY_MODE:
		{
			Result = (Value >= Number);
			break;
		}
		/* Greater than */
		case GT_QUERY_MODE:
		{
			Result = (Value > Number);
			break;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_event
 * FUNCTION  : Modify event handler.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:05
 * LAST      : 11.10.95 15:49
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Modify_event(void)
{
	struct Event_block *Event;
	BOOLEAN Result;
	UNSHORT Number;
	UNSHORT Extra;
	UNSHORT Modify_mode;

	/* Set success flag */
	Set_success_flag();

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get modify parameters */
	Modify_mode = (UNSHORT) Event->Byte_2;
	Extra = (UNSHORT) Event->Byte_3;
	Number = Event->Word_6;

	/* What kind of modify ? */
	switch (Event->Byte_1)
	{
		/* Quest bit array */
		case QUEST_BIT_QM_TYPE:
		{
			/* Modify bit in quest bit array */
			Write_bit_array(QUEST_BIT_ARRAY, Number, Modify_mode);
			break;
		}
		/* Event save bit array */
		case EVENT_SAVE_BIT_QM_TYPE:
		{
			UNLONG Bit_number;

			/* Use on current map ? */
			if (!Number)
			{
				Number = PARTY_DATA.Map_nr;
			}

			/* Calculate bit number */
			Bit_number = ((Number - 1) * EVENTS_PER_MAP) + Extra;

			/* Modify bit in event save bit array */
			Write_bit_array(EVENT_SAVE_BIT_ARRAY, Bit_number, Modify_mode);
			break;
		}
		/* Door bit array */
		case DOOR_BIT_QM_TYPE:
		{
			/* Modify bit in door bit array */
			Write_bit_array(DOOR_BIT_ARRAY, Number, Modify_mode);
			break;
		}
		/* Chest bit array */
		case CHEST_BIT_QM_TYPE:
		{
			/* Modify bit in chest bit array */
			Write_bit_array(CHEST_BIT_ARRAY, Number, Modify_mode);
			break;
		}
		/* Character Deleted bit array */
		case CD_BIT_QM_TYPE:
		{
			UNLONG Bit_number;

			/* Use on current map ? */
			if (!Number)
			{
				Number = PARTY_DATA.Map_nr;
			}

			/* Calculate bit number */
			Bit_number = ((Number - 1) * NPCS_PER_MAP) + Extra;

			/* Modify bit in CD bit array */
			Write_bit_array(CD_BIT_ARRAY, Bit_number, Modify_mode);
			break;
		}
		/* Character in party */
		case CHAR_IN_PARTY_QM_TYPE:
		{
			/* Add a party member */
			Result = Add_party_member(Number, 0xFFFF);

			/* Success ? */
			if (!Result)
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}
			break;
		}
		/* Items carried by the party */
		case ITEM_CARRIED_QM_TYPE:
		{
			Result = Modify_items_carried_by_party(Number, Extra, Modify_mode);

			/* Success ? */
			if (!Result)
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}
			break;
		}
		/* Word bit array */
		case WORD_BIT_QM_TYPE:
		{
			/* Modify bit in known words bit array */
			Write_bit_array(KNOWN_WORDS_BIT_ARRAY, Number, Modify_mode);

			/* In Dialogue ? */
			if (In_Dialogue)
			{
				/* Yes -> Modify in new words bit array as well */
				Write_bit_array(NEW_WORDS_BIT_ARRAY, Number, Modify_mode);
			}
			break;
		}
		/* Current light state */
		case LIGHT_STATE_QM_TYPE:
		{
			/* Set new map light state */
			Current_map_light_state = Number;
			break;
		}
		/* Gold carried by the party */
		case GOLD_CARRIED_QM_TYPE:
		{
			UNLONG Party_gold;

			/* Get party gold */
			Party_gold = Get_party_gold();

			/* Is this modification possible ? */
			if (Is_modify_possible
			(
				Party_gold,
				6 * 32767,
				Modify_mode,
				(UNLONG) Number
			))
			{
				/* Yes -> Modify this amount */
				Party_gold = Modify_amount
				(
					Party_gold,
					6 * 32767,
					Modify_mode,
					(UNLONG) Number
				);

				/* Set party gold */
				Set_party_gold(Party_gold);
			}
			else
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}
			break;
		}
		/* Food carried by the party */
		case FOOD_CARRIED_QM_TYPE:
		{
			UNLONG Party_food;

			/* Get party food */
			Party_food = Get_party_food();

			/* Is this modification possible ? */
			if (Is_modify_possible
			(
				Party_food,
				6 * 32767,
				Modify_mode,
				(UNLONG) Number
			))
			{
				/* Yes -> Modify this amount */
				Party_food = Modify_amount
				(
					Party_food,
					6 * 32767,
					Modify_mode,
					(UNLONG) Number
				);

				/* Set party food */
				Set_party_food(Party_food);
			}
			else
			{
				/* No -> Clear success flag */
				Clear_success_flag();
			}
			break;
		}
		/* Currently active member */
		case ACTIVE_MEMBER_QM_TYPE:
		{
			UNSHORT Member_index;

			/* Clear success flag */
			Clear_success_flag();

			/* Find character in party */
			Member_index = Find_character_in_party(Number);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Try to activate */
				Result = Activate_party_member(Member_index);

				/* Success ? */
				if (Result)
				{
					/* Yes -> Set success flag */
					Set_success_flag();
				}
			}
			break;
		}
		/* Byte counter array */
		case BYTE_COUNTER_QM_TYPE:
		{
			/* Legal byte-counter index ? */
			if (Number < MAX_BYTE_COUNTERS)
			{
				/* Yes -> Modify byte counter */
				PARTY_DATA.Byte_counters[Number] = (UNBYTE) Modify_amount
				(
					(UNLONG) PARTY_DATA.Byte_counters[Number],
					(UNLONG) 255,
					Modify_mode,
					(UNLONG) Extra
				);
			}
			break;
		}
		/* Hour */
		case HOUR_QM_TYPE:
		{
			struct Event_context *Context;
			UNSHORT Old_map_nr;

			/* Act depending on modify mode */
			switch (Modify_mode)
			{
				/* Load */
				case LOAD_MODIFY_MODE:
				{
					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Set new hour */
					PARTY_DATA.Hour = Number % Current_time_frame->Hours_per_day;

					/* Re-initialise time */
					Reset_time();

					/* Reset palette blending */
					Reset_map_palette_blending();

					break;
				}
				/* Increase */
				case INC_MODIFY_MODE:
				{
					/* Get current map number */
					Old_map_nr = PARTY_DATA.Map_nr;

					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Wait */
					Wait_x_hours(Number);

					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Still in the same map ? */
					if (Old_map_nr != PARTY_DATA.Map_nr)
					{
						/* No -> Get current event context */
						Context = &(Event_context_stack[Event_context_stack_index]);

						/* Was this event chain located in the map data ? */
						if (Context->Source == MAP_EVENT_SOURCE)
						{
							/* Yes -> Break current event context */
							Break_current_context();
						}
					}

					/* Reset palette blending */
					Reset_map_palette_blending();

					break;
				}
			}
			break;
		}
		/* Day */
		case DAY_QM_TYPE:
		{
			struct Event_context *Context;
			UNSHORT Old_map_nr;
			UNSHORT i;

			/* Act depending on modify mode */
			switch (Modify_mode)
			{
				/* Load */
				case LOAD_MODIFY_MODE:
				{
					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Set new day */
					PARTY_DATA.Day = Number % Current_time_frame->Days_per_month;

					/* Re-initialise time */
					Reset_time();

					/* Reset palette blending */
					Reset_map_palette_blending();

					break;
				}
				/* Increase */
				case INC_MODIFY_MODE:
				{
					/* Get current map number */
					Old_map_nr = PARTY_DATA.Map_nr;

					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Increase day */
					for (i=0;i<Number;i++)
					{
						Wait_x_hours(Current_time_frame->Hours_per_day);
					}

					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Still in the same map ? */
					if (Old_map_nr != PARTY_DATA.Map_nr)
					{
						/* No -> Get current event context */
						Context = &(Event_context_stack[Event_context_stack_index]);

						/* Was this event chain located in the map data ? */
						if (Context->Source == MAP_EVENT_SOURCE)
						{
							/* Yes -> Break current event context */
							Break_current_context();
						}
					}

					/* Reset palette blending */
					Reset_map_palette_blending();

					break;
				}
			}
			break;
		}
		/* Step */
		case STEP_QM_TYPE:
		{
			struct Event_context *Context;
			UNSHORT Old_map_nr;
			UNSHORT i;

			/* Act depending on modify mode */
			switch (Modify_mode)
			{
				/* Load */
				case LOAD_MODIFY_MODE:
				{
					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Set new step */
					Current_step = Number % Current_time_frame->Steps_per_hour;

					/* Re-initialise time */
					Reset_time();

					/* Reset palette blending */
					Reset_map_palette_blending();

					break;
				}
				/* Increase */
				case INC_MODIFY_MODE:
				{
					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Get current map number */
					Old_map_nr = PARTY_DATA.Map_nr;

					/* Increase step */
					for (i=0;i<Number;i++)
					{
						Next_step();
					}

					/* Clear camp counter */
					PARTY_DATA.Camp_counter = 0;

					/* Still in the same map ? */
					if (Old_map_nr != PARTY_DATA.Map_nr)
					{
						/* No -> Get current event context */
						Context = &(Event_context_stack[Event_context_stack_index]);

						/* Was this event chain located in the map data ? */
						if (Context->Source == MAP_EVENT_SOURCE)
						{
							/* Yes -> Break current event context */
							Break_current_context();
						}
					}

					/* Reset palette blending */
					Reset_map_palette_blending();

					break;
				}
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_amount
 * FUNCTION  : Modify an amount.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:15
 * LAST      : 29.12.94 11:15
 * INPUTS    : SILONG Amount - Original amount.
 *             SILONG Maximum - Maximum amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 *             SILONG Number - Number that is used to modify amount.
 * RESULT    : SILONG : Modified amount.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG
Modify_amount(SILONG Amount, SILONG Maximum, UNSHORT Modify_mode,
 SILONG Number)
{
	/* Which modify mode ? */
	switch (Modify_mode)
	{
		/* Clear */
		case CLEAR_MODIFY_MODE:
		{
			Amount = 0;
			break;
		}
		/* Set to maximum */
		case SET_MODIFY_MODE:
		{
			Amount = Maximum;
			break;
		}
		/* Load with value */
		case LOAD_MODIFY_MODE:
		{
			Amount = Number;
			break;
		}
		/* Increase */
		case INC_MODIFY_MODE:
		{
			Amount += Number;

			if (Amount > Maximum)
				Amount = Maximum;

			break;
		}
		/* Decrease */
		case DEC_MODIFY_MODE:
		{
			if (Amount < Number)
				Amount = 0;
			else
				Amount -= Number;

			break;
		}
		/* Increase by percentage */
		case INCPERC_MODIFY_MODE:
		{
			Number = (Number * Amount) / 100;

			Amount += Number;

			if (Amount > Maximum)
				Amount = Maximum;

			break;
		}
		/* Decrease by percentage */
		case DECPERC_MODIFY_MODE:
		{
			Number = (Number * Amount) / 100;

			if (Amount < Number)
				Amount = 0;
			else
				Amount -= Number;

			break;
		}
	}

	return Amount;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Is_modify_possible
 * FUNCTION  : Check if a modification is possible.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 18:30
 * LAST      : 02.09.95 18:30
 * INPUTS    : SILONG Amount - Original amount.
 *             SILONG Maximum - Maximum amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 *             SILONG Number - Number that is used to modify amount.
 * RESULT    : BOOLEAN : Possible.
 * BUGS      : No known.
 * NOTES     : - This function does not need to use signed variables.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Is_modify_possible(SILONG Amount, SILONG Maximum, UNSHORT Modify_mode,
 SILONG Number)
{
	BOOLEAN Result = TRUE;

	/* Which modify mode ? */
	switch (Modify_mode)
	{
		/* Load with value */
		case LOAD_MODIFY_MODE:
		{
			if (Number > Maximum)
				Result = FALSE;

			break;
		}
		/* Increase */
		case INC_MODIFY_MODE:
		{
			if ((Amount + Number) > Maximum)
				Result = FALSE;

			break;
		}
		/* Decrease */
		case DEC_MODIFY_MODE:
		{
			if (Amount < Number)
				Result = FALSE;

			break;
		}
		/* Increase by percentage */
		case INCPERC_MODIFY_MODE:
		{
			Number = (Number * Amount) / 100;

			if ((Amount + Number) > Maximum)
				Result = FALSE;

			break;
		}
		/* Decrease by percentage */
		case DECPERC_MODIFY_MODE:
		{
			Number = (Number * Amount) / 100;

			if (Amount < Number)
				Result = FALSE;

			break;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_amount_from_maximum
 * FUNCTION  : Modify an amount, using the maximum value as a reference for
 *              relative modifications.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.95 10:35
 * LAST      : 02.09.95 18:32
 * INPUTS    : SILONG Amount - Original amount.
 *             SILONG Maximum - Maximum amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 *             SILONG Number - Number that is used to modify amount.
 * RESULT    : SILONG : Modified amount.
 * BUGS      : No known.
 * NOTES     : - This function does not need to use signed variables.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SILONG
Modify_amount_from_maximum(SILONG Amount, SILONG Maximum, UNSHORT Modify_mode,
 SILONG Number)
{
	/* Which modify mode ? */
	switch (Modify_mode)
	{
		/* Clear */
		case CLEAR_MODIFY_MODE:
		{
			Amount = 0;
			break;
		}
		/* Set to maximum */
		case SET_MODIFY_MODE:
		{
			Amount = Maximum;
			break;
		}
		/* Load with value */
		case LOAD_MODIFY_MODE:
		{
			Amount = min(Maximum, Number);
			break;
		}
		/* Increase */
		case INC_MODIFY_MODE:
		{
			Amount += Number;

			if (Amount > Maximum)
				Amount = Maximum;

			break;
		}
		/* Decrease */
		case DEC_MODIFY_MODE:
		{
			if (Amount < Number)
				Amount = 0;
			else
				Amount -= Number;

			break;
		}
		/* Increase by percentage */
		case INCPERC_MODIFY_MODE:
		{
			Number = (Number * Maximum) / 100;

			Amount += Number;

			if (Amount > Maximum)
				Amount = Maximum;

			break;
		}
		/* Decrease by percentage */
		case DECPERC_MODIFY_MODE:
		{
			Number = (Number * Maximum) / 100;

			if (Amount < Number)
				Amount = 0;
			else
				Amount -= Number;

			break;
		}
	}

	return Amount;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_carried_items
 * FUNCTION  : Modify the number of a certain item carried by a character.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 00:05
 * LAST      : 23.10.95 22:00
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Item_index - Index of item.
 *             UNSHORT Number - Number that is used to modify amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 * RESULT    : BOOLEAN : Success.
 * NOTES     : - This function may give a character more items than he or she
 *              can carry. Also, if the character does not have enough room
 *              in his or her backpack, the function will not go into an
 *              endless loop.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Modify_carried_items(MEM_HANDLE Char_handle, UNSHORT Item_index,
 UNSHORT Number, UNSHORT Modify_mode)
{
	struct Character_data *Char;
	struct Item_packet Add_packet;
	BOOLEAN Success;
	BOOLEAN Result;
	SILONG Old_amount;
	SILONG New_amount;
	SILONG Attempted_amount;
	SILONG Counter;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Count current number of items */
	Old_amount = (SILONG) Count_items_on_body(Char_handle, Item_index) +
	 (SILONG) Count_items_in_backpack(Char_handle, Item_index);

	/* Is this modification possible ? */
	if (!Is_modify_possible
	(
		Old_amount,
		0x7FFFFFFF,
		Modify_mode,
		(SILONG) Number
	))
	{
		/* No -> Exit */
		return FALSE;
	}

	/* Calculate new amount */
	New_amount = Modify_amount
	(
		Old_amount,
		0x7FFFFFFF,
		Modify_mode,
		(SILONG) Number
	);

	/* Any change ? */
	if (Old_amount == New_amount)
		return TRUE;

	/* Yes -> No success yet */
	Success = FALSE;

	/* More or less ? */
	if (Old_amount > New_amount)
	{
		/* Less -> Remove items */
		Counter = Old_amount - New_amount;

		/* Check body items */
		for (i=0;i<ITEMS_ON_BODY;i++)
		{
			/* Slot not empty / right item ? */
			if ((!Packet_empty(&(Char->Body_items[i]))) &&
			 (Char->Body_items[i].Index == Item_index))
			{
				/* Yes -> Remove it */
				Remove_item(Char_handle, i + 1, 1);

				/* Count down */
				Counter--;
			}

			/* Satisfied ? */
			if (!Counter)
			{
				/* Yes -> Success! */
				Success = TRUE;
				break;
			}
		}

		/* Is there any need to check the backpack ? */
		if (Counter)
		{
			/* Yes -> Check backpack items */
			for (i=0;i<ITEMS_PER_CHAR;i++)
			{
				/* Slot not empty / right item ? */
				while ((!Packet_empty(&(Char->Backpack_items[i]))) &&
				 (Char->Backpack_items[i].Index == Item_index))
				{
					/* Yes -> Remove it */
					Remove_item(Char_handle, ITEMS_ON_BODY + i + 1, 1);

					/* Count down */
					Counter--;

					/* Satisfied ? */
					if (!Counter)
					{
						/* Yes -> Success! */
						Success = TRUE;
						break;
					}
				}

				/* Satisfied ? */
				if (!Counter)
				{
					/* Yes -> Break */
					break;
				}
			}
		}

		/* In dialogue / not in combat ? */
		if (In_Dialogue && !In_Combat)
		{
			/* Yes -> Success ? */
			if (Success)
			{
				/* Yes -> Build item packet */
				Build_item_packet
				(
					&Add_packet,
					Item_index,
					Old_amount - New_amount
				);

				/* Show moving items */
				Show_moving_item
				(
					&Add_packet,
					10 + 12,
					5 + 8,
					360 - 36 - 8 + 12,
					5 + 8
				);
			}
		}
	}
	else
	{
		/* More -> Add items */
		Counter = New_amount - Old_amount;

		while (Counter)
		{
			/* Too much for one packet ? */
			if (Counter > 99)
			{
				/* Yes -> First do batch of 99 items */
				Attempted_amount = 99;
			}
			else
			{
				/* No -> Do all items */
				Attempted_amount = Counter;
			}

			/* Build packet */
			Build_item_packet
			(
				&Add_packet,
				Item_index,
				(UNSHORT) Attempted_amount
			);

			/* Count down */
			Counter -= Attempted_amount;

			/* Try to move packet to backpack */
			Auto_move_packet_to_backpack(Char_handle, &Add_packet);

			/* Anything left in the packet ? */
			if (!Packet_empty(&Add_packet))
			{
				/* Yes -> Is this exactly the amount we tried to add ? */
				if ((SILONG) Add_packet.Quantity == Attempted_amount)
				{
					/* Yes -> Try to free a slot */
					Result = Free_item_slot_in_inventory(Char_handle);

					/* Success ? */
					if (Result)
					{
						/* Yes -> Add back to counter */
						Counter += (SILONG) Add_packet.Quantity;
					}
					else
					{
						/* No -> Error */
						break;
					}
				}
				else
				{
					/* No -> Add back to counter */
					Counter += (SILONG) Add_packet.Quantity;
				}
			}

			/* Some items were added */
			Success = TRUE;
		}

		/* In dialogue / not in combat ? */
		if (In_Dialogue && !In_Combat)
		{
			/* Yes -> Success ? */
			if (Success)
			{
				/* Yes -> Build item packet */
				Build_item_packet
				(
					&Add_packet,
					Item_index,
					New_amount - Old_amount
				);

				/* Show moving items */
				Show_moving_item
				(
					&Add_packet,
					360 - 36 - 8 + 12,
					5 + 8,
					10 + 12,
					5 + 8
				);
			}
		}
	}
	MEM_Free_pointer(Char_handle);

	return Success;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_items_carried_by_party
 * FUNCTION  : Modify the number of a certain item carried by the party.
 * FILE      : QUERMOD.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 10:49
 * LAST      : 15.10.95 22:20
 * INPUTS    : UNSHORT Item_index - Index of item.
 *             UNSHORT Number - Number that is used to modify amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Modify_items_carried_by_party(UNSHORT Item_index, UNSHORT Number,
 UNSHORT Modify_mode)
{
	struct Character_data *Char;
	struct Item_packet Add_packet;
	BOOLEAN Success;
	BOOLEAN Result;
	SILONG Nr_items[6];
	SILONG Old_amount;
	SILONG New_amount;
	SILONG Counter;
	SILONG Subtract;
	UNSHORT i, j;

	/* Count amount of items currently carried by each party member */
	Old_amount = 0;
	for (i=0;i<6;i++)
	{
		/* Clear */
		Nr_items[i] = 0;

		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Count party member's items */
			Nr_items[i] = (SILONG) Count_items_on_body
			(
				Party_char_handles[i],
				Item_index
			);
			Nr_items[i] += (SILONG) Count_items_in_backpack
			(
				Party_char_handles[i],
				Item_index
			);

			/* Increase total amount */
			Old_amount += Nr_items[i];
		}
	}

	/* Is this modification possible ? */
	if (!Is_modify_possible
	(
		Old_amount,
		0x7FFFFFFF,
		Modify_mode,
		(SILONG) Number
	))
	{
		/* No -> Exit */
		return FALSE;
	}

	/* Calculate new amount */
	New_amount = Modify_amount
	(
		Old_amount,
		0x7FFFFFFF,
		Modify_mode,
		(SILONG) Number
	);

	/* Any change ? */
	if (Old_amount == New_amount)
		return TRUE;

	/* Yes -> No success yet */
	Success = FALSE;

	/* More or less ? */
	if (Old_amount > New_amount)
	{
		/* Less -> Remove items */
		Counter = Old_amount - New_amount;

		/* Check party */
		for (i=0;i<6;i++)
		{
			/* Anyone there ? */
			if (Member_present(i + 1))
			{
				/* Yes -> How many should be removed from this character ? */
				if (Counter > Nr_items[i])
					Subtract = Nr_items[i];
				else
					Subtract = Counter;

				/* Get character data */
				Char = (struct Character_data *)
				 MEM_Claim_pointer(Party_char_handles[i]);

				/* Check body items */
				for (j=0;j<ITEMS_ON_BODY;j++)
				{
					/* Slot not empty / right item ? */
					if ((!Packet_empty(&(Char->Body_items[j]))) &&
					 (Char->Body_items[j].Index == Item_index))
					{
						/* Yes -> Remove it */
						Remove_item
						(
							Party_char_handles[i],
							j + 1,
							1
						);

						/* Count down */
						Subtract--;
						Counter--;

						/* Satisfied ? */
						if (!Subtract || !Counter)
						{
							/* Yes -> Break */
							break;
						}
					}
				}

				/* Satisfied ? */
				if (Subtract)
				{
					/* No -> Check backpack items */
					for (j=0;j<ITEMS_PER_CHAR;j++)
					{
						/* Slot not empty / right item ? */
						while ((!Packet_empty(&(Char->Backpack_items[j]))) &&
						 (Char->Backpack_items[j].Index == Item_index))
						{
							/* Yes -> Remove it */
							Remove_item
							(
								Party_char_handles[i],
								ITEMS_ON_BODY + j + 1,
								1
							);

							/* Count down */
							Subtract--;
							Counter--;

							/* Satisfied ? */
							if (!Subtract || !Counter)
							{
								/* Yes -> Break */
								break;
							}
						}

						/* Satisfied ? */
						if (!Subtract || !Counter)
						{
							/* Yes -> Break */
							break;
						}
					}
				}

				MEM_Free_pointer(Party_char_handles[i]);

				/* Error ? */
				if (Subtract)
				{
					/* Yes -> Absorb */
					Counter += Subtract;
				}

				/* Satisfied completely ? */
				if (!Counter)
				{
					/* Yes -> Success! */
					Success = TRUE;
					break;
				}
			}
		}

		/* In dialogue / not in combat ? */
		if (In_Dialogue && !In_Combat)
		{
			/* Yes -> Success ? */
			if (Success)
			{
				/* Yes -> Build item packet */
				Build_item_packet
				(
					&Add_packet,
					Item_index,
					Old_amount - New_amount
				);

				/* Show moving items */
				Show_moving_item
				(
					&Add_packet,
					10 + 12,
					5 + 8,
					360 - 36 - 8 + 12,
					5 + 8
				);
			}
		}
	}
	else
	{
		/* More -> Add items */
		Counter = New_amount - Old_amount;

		while (Counter)
		{
			/* Give one item to each party member */
			Success = FALSE;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Build packet */
					Build_item_packet(&Add_packet, Item_index, 1);

					/* Try to move packet to backpack */
					Auto_move_packet_to_backpack(Party_char_handles[i],
					 &Add_packet);

					/* Anything left in the packet ? */
					if (Packet_empty(&Add_packet))
					{
						/* No -> Count down */
						Counter--;

						/* Satisfied ? */
						if (!Counter)
						{
							/* Yes -> Indicate success */
							Success = TRUE;
							break;
						}
					}
				}
			}

			/* Any luck at all ? */
			if (!Success)
			{
				/* No -> Try to free a slot */
				Result = Free_item_slot_in_party();

				/* Success ? */
				if (!Result)
				{
					/* No -> Error */
					break;
				}
			}
		}

		/* In dialogue / not in combat ? */
		if (In_Dialogue && !In_Combat)
		{
			/* Yes -> Success ? */
			if (Success)
			{
				/* Yes -> Build item packet */
				Build_item_packet
				(
					&Add_packet,
					Item_index,
					New_amount - Old_amount
				);

				/* Show moving items */
				Show_moving_item
				(
					&Add_packet,
					360 - 36 - 8 + 12,
					5 + 8,
					10 + 12,
					5 + 8
				);
			}
		}
	}

	return Success;
}

