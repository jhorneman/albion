/************
 * NAME     : MAP_PUM.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 10-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <BBDEF.H>

#include <3D_SEL.H>
#include <MAP_PUM.H>
#include <MAP.H>
#include <POPUP.H>
#include <GAMETEXT.H>
#include <EVELOGIC.H>
#include <PRTLOGIC.H>
#include <NPCS.H>
#include <FONT.H>
#include <XFTYPES.H>
#include <GAMEVAR.H>
#include <DIALOGUE.H>
#include <STATAREA.H>
#include <TEXTWIN.H>
#include <ITEMSEL.H>
#include <AUTOMAP.H>
#include <CAMP.H>
#include <QUERMOD.H>
#include <MAINMENU.H>
#include <BOOLREQ.H>
#include <INPUTNR.H>
#include <GAMETIME.H>

/*
 ** Prototypes *************************************************************
 */

/* Map pop-up menu functions */
void Map_PUM_evaluator(struct PUM *PUM);

void PUM_Map_Talk(UNLONG Data);
void PUM_Map_Examine(UNLONG Data);
void PUM_Map_Manipulate(UNLONG Data);
void PUM_Map_Use_item(UNLONG Data);
void PUM_Map_Take_item(UNLONG Data);

void PUM_Map_Change_transport(UNLONG Data);
void PUM_Map_Enter_automapper(UNLONG Data);
void PUM_Map_Build_camp(UNLONG Data);
void PUM_Map_Wait(UNLONG Data);
void PUM_Map_Main_menu(UNLONG Data);

/*
 ** Global variables *******************************************************
 */

/* Current map selection data */
struct Map_selection_data Current_map_selection_data;

/* Map pop-up menu entries */
static struct PUME Map_PUMEs[] = {
	{PUME_AUTO_CLOSE,			0,	6,		PUM_Map_Talk},
	{PUME_AUTO_CLOSE,			0,	7,		PUM_Map_Examine},
	{PUME_AUTO_CLOSE,			0,	8,		PUM_Map_Manipulate},
	{PUME_AUTO_CLOSE,			0,	9,		PUM_Map_Use_item},
	{PUME_AUTO_CLOSE,			0,	74,	PUM_Map_Take_item},
	{PUME_NOT_SELECTABLE,	0,	0,		NULL},
	{PUME_AUTO_CLOSE,			0,	11,	PUM_Map_Change_transport},
	{PUME_AUTO_CLOSE,			0,	130,	PUM_Map_Enter_automapper},
	{PUME_AUTO_CLOSE,			0,	12,	PUM_Map_Build_camp},
	{PUME_AUTO_CLOSE,			0,	723,	PUM_Map_Wait},
	{PUME_AUTO_CLOSE,			0,	696,	PUM_Map_Main_menu}
};

/* Map pop-up menu */
struct PUM Map_PUM = {
	11,
	NULL,
	0,
	Map_PUM_evaluator,
	Map_PUMEs
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Map_PUM_evaluator
 * FUNCTION  : Evaluate map pop-up menu.
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 12:30
 * LAST      : 21.10.95 14:00
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Map_PUM_evaluator(struct PUM *PUM)
{
	struct PUME *PUMES;
	UNLONG Distance;
	UNSHORT State;
	UNSHORT Trigger_modes;

	PUMES = PUM->PUME_list;

	/* Set default pop-up menu title */
	PUM->Title = System_text_ptrs[14];

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Was an NPC selected ? */
		if (Current_map_selection_data.NPC_index != 0xFFFF)
		{
			/* Yes -> Set pop-up menu title */
			PUM->Title = System_text_ptrs[513];
		}

		/* Get map selection data */
		State				= Current_map_selection_data.State;
		Trigger_modes	= Current_map_selection_data.Trigger_modes;
		Distance			= Current_map_selection_data.Distance;

		/* Speak trigger-mode set ? */
		if (Trigger_modes & (1 << SPEAK_TRIGGER))
		{
			/* Yes -> Is there an active member ? */
			if (Member_present(PARTY_DATA.Active_member))
			{
				/* Yes -> Is Mellthas the active character ? */
				if (PARTY_DATA.Member_nrs[PARTY_DATA.Active_member - 1] == MELLTHAS_CHAR)
				{
					/* Yes -> Block PUM entry */
					PUMES[0].Flags |= PUME_BLOCKED;
					PUMES[0].Blocked_message_nr = 735;
				}
			}
			else
			{
				/* No -> Is this too far away for talking ? */
				if ((Distance > MAX_TALK_DISTANCE) && !Cheat_mode)
				{
					/* Yes -> Block PUM entry */
					PUMES[0].Flags |= PUME_BLOCKED;
					PUMES[0].Blocked_message_nr = 536;
				}
			}
		}
		else
		{
			/* No -> Speaking is not possible */
			PUMES[0].Flags |= PUME_ABSENT;
		}

		/* Examine trigger-mode set ? */
		if (Trigger_modes & (1 << EXAMINE_TRIGGER))
		{
			/* Yes -> Is this too far away for examining ? */
			if ((Distance > MAX_EXAMINE_DISTANCE) && !Cheat_mode)
			{
				/* Yes -> Block PUM entry */
				PUMES[1].Flags |= PUME_BLOCKED;
				PUMES[1].Blocked_message_nr = 536;
			}
		}
		else
		{
			/* No -> Examining is not possible */
			PUMES[1].Flags |= PUME_ABSENT;
		}

		/* Touch trigger-mode set ? */
		if (Trigger_modes & (1 << TOUCH_TRIGGER))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if ((Distance > MAX_MANIPULATE_DISTANCE) && !Cheat_mode)
			{
				/* Yes -> Block PUM entry */
				PUMES[2].Flags |= PUME_BLOCKED;
				PUMES[2].Blocked_message_nr = 536;
			}
			else
			{
				/* No -> Blocked ? */
				if (State == BLOCKED_MAP_SELECTION)
				{
					/* Yes -> Block PUM entry */
					PUMES[2].Flags |= PUME_BLOCKED;
					PUMES[2].Blocked_message_nr = 537;
				}
			}
		}
		else
		{
			/* No -> Touching is not possible */
			PUMES[2].Flags |= PUME_ABSENT;
		}

		/* Use item trigger-mode set ? */
		if (Trigger_modes & (1 << USE_ITEM_TRIGGER))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if ((Distance > MAX_MANIPULATE_DISTANCE) && !Cheat_mode)
			{
				/* Yes -> Block PUM entry */
				PUMES[3].Flags |= PUME_BLOCKED;
				PUMES[3].Blocked_message_nr = 536;
			}
			else
			{
				/* No -> Blocked ? */
				if (State == BLOCKED_MAP_SELECTION)
				{
					/* Yes -> Block PUM entry */
					PUMES[3].Flags |= PUME_BLOCKED;
					PUMES[3].Blocked_message_nr = 537;
				}
			}
		}
		else
		{
			/* No -> Using item is not possible */
			PUMES[3].Flags |= PUME_ABSENT;
		}

		/* Take trigger-mode set ? */
		if (Trigger_modes & (1 << TAKE_TRIGGER))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if ((Distance > MAX_MANIPULATE_DISTANCE) && !Cheat_mode)
			{
				/* Yes -> Block PUM entry */
				PUMES[4].Flags |= PUME_BLOCKED;
				PUMES[4].Blocked_message_nr = 536;
			}
			else
			{
				/* No -> Blocked ? */
				if ((State == BLOCKED_MAP_SELECTION) && !Cheat_mode)
				{
					/* Yes -> Block PUM entry */
					PUMES[4].Flags |= PUME_BLOCKED;
					PUMES[4].Blocked_message_nr = 537;
				}
				else
				{
					/* No -> Is the active character overweight ? */
					if (Character_is_overweight(Active_char_handle))
					{
						/* Yes -> Block PUM entry */
						PUMES[4].Flags |= PUME_BLOCKED;
						PUMES[4].Blocked_message_nr = 489;
					}
					else
					{
						/* No -> Is there a free slot in the active character's
						 backpack ? */
						if (Char_inventory_full(Active_char_handle))
						{
							/* No -> Block PUM entry */
							PUMES[4].Flags |= PUME_BLOCKED;
							PUMES[4].Blocked_message_nr = 490;
						}
					}
				}
			}
		}
		else
		{
			/* No -> Taking is not possible */
			PUMES[4].Flags |= PUME_ABSENT;
		}

		/* Cannot change transportation */
		PUMES[6].Flags |= PUME_ABSENT;
	}
	else
	{
		/* 2D -> Search for NPCs */
		Current_map_selection_data.NPC_index = Search_2D_NPC
		(
			Current_map_selection_data.Map_X,
			Current_map_selection_data.Map_Y,
			0xFFFF
		);

		/* Is there an NPC here ? */
		if (Current_map_selection_data.NPC_index != 0xFFFF)
		{
			/* Yes -> Has event ? */
			if (VNPCs[Current_map_selection_data.NPC_index].First_block_nr != 0xFFFF)
			{
				/* Yes -> Get triggermodes */
				Trigger_modes = VNPCs[Current_map_selection_data.NPC_index].Trigger_modes;

				/* Set pop-up menu title */
				PUM->Title = System_text_ptrs[513];
			}
			else
			{
				/* No -> Can this NPC be talked to ? */
				if ((VNPCs[Current_map_selection_data.NPC_index].NPC_type != MONSTER_NPC) &&
				 (VNPCs[Current_map_selection_data.NPC_index].NPC_type != OBJECT_NPC))
				{
					/* Yes -> Use standard "triggermodes" */
					Trigger_modes = (1 << SPEAK_TRIGGER);

					/* Set pop-up menu title */
					PUM->Title = System_text_ptrs[513];
				}
				else
				{
					/* No -> Ignore this NPC */
					Current_map_selection_data.NPC_index = 0xFFFF;

					/* Get trigger modes for this position */
					Trigger_modes = Get_event_triggers
					(
						Current_map_selection_data.Map_X,
						Current_map_selection_data.Map_Y,
						0xFFFF
					);
				}
			}
		}
		else
		{
			/* No -> Get trigger modes for this position */
			Trigger_modes = Get_event_triggers
			(
				Current_map_selection_data.Map_X,
				Current_map_selection_data.Map_Y,
				0xFFFF
			);
		}

		/* Change PUM entries depending on triggermodes */
		if (!(Trigger_modes & (1 << SPEAK_TRIGGER)))
			PUMES[0].Flags |= PUME_ABSENT;

		if (!(Trigger_modes & (1 << EXAMINE_TRIGGER)))
			PUMES[1].Flags |= PUME_ABSENT;

		if (!(Trigger_modes & (1 << TOUCH_TRIGGER)))
			PUMES[2].Flags |= PUME_ABSENT;

		if (!(Trigger_modes & (1 << USE_ITEM_TRIGGER)))
			PUMES[3].Flags |= PUME_ABSENT;

		/* Speak trigger-mode set ? */
		if (Trigger_modes & (1 << SPEAK_TRIGGER))
		{
			/* Yes -> Is there an active member ? */
			if (Member_present(PARTY_DATA.Active_member))
			{
				/* Yes -> Is Mellthas the active character ? */
				if (PARTY_DATA.Member_nrs[PARTY_DATA.Active_member - 1] == MELLTHAS_CHAR)
				{
					/* Yes -> Block PUM entry */
					PUMES[0].Flags |= PUME_BLOCKED;
					PUMES[0].Blocked_message_nr = 735;
				}
			}
		}

		/* Take trigger-mode set ? */
		if (Trigger_modes & (1 << TAKE_TRIGGER))
		{
			/* Yes -> Is the active character overweight ? */
			if (Character_is_overweight(Active_char_handle))
			{
				/* Yes -> Block PUM entry */
				PUMES[4].Flags |= PUME_BLOCKED;
				PUMES[4].Blocked_message_nr = 489;
			}
			else
			{
				/* No -> Is there a free slot in the active character's
				 backpack ? */
				if (Char_inventory_full(Active_char_handle))
				{
					/* No -> Block PUM entry */
					PUMES[4].Flags |= PUME_BLOCKED;
					PUMES[4].Blocked_message_nr = 490;
				}
			}
		}
		else
		{
			/* No -> Taking is not possible */
			PUMES[4].Flags |= PUME_ABSENT;
		}

		/* Is the party on foot ? */
		if (PARTY_DATA.Travel_mode == ON_FOOT)
		{
			/* Yes -> Is there a transportation here ? */
			if (!(Seek_transport
			(
				PARTY_DATA.Map_nr,
				PARTY_DATA.X,
				PARTY_DATA.Y
			)))
			{
				/* No */
				PUMES[6].Flags |= PUME_ABSENT;
			}
		}
		else
		{
			/* No -> Can the party go on foot here ? */
			if (Get_location_status
			(
				PARTY_DATA.X,
				PARTY_DATA.Y,
				0xFFFF
			) & BLOCKED_TRAVELMODES)
			{
				/* Yes */
				PUMES[6].Flags |= PUME_ABSENT;
			}
		}

		/* Cannot enter automapper */
		PUMES[7].Flags |= PUME_ABSENT;
	}

	/* In a city / in a house / in the spaceship ? */
	if ((Current_map_type == CITY_MAP) ||
	 (Current_map_type == INTERIOR_MAP) ||
	 Spaceship_map)
	{
		/* Yes -> Cannot camp */
		PUMES[8].Flags |= PUME_ABSENT;
	}
	else
	{
		/* No -> Is a monster watching ? */
		if (Monster_is_watching)
		{
			/* Yes -> Block camp */
			PUMES[8].Flags |= PUME_BLOCKED;
			PUMES[8].Blocked_message_nr = 601;
		}
		else
		{
			/* No -> Is it too early ? */
			if (PARTY_DATA.Camp_counter < CAMP_INTERVAL)
			{
				/* Yes -> Block camp */
				PUMES[8].Flags |= PUME_BLOCKED;
				PUMES[8].Blocked_message_nr = 603;
			}
		}
	}

	/* Is a city map ? */
	if (Current_map_type == CITY_MAP)
	{
		/* Yes -> Is a monster watching ? */
		if (Monster_is_watching)
		{
			/* Yes -> Block waiting */
			PUMES[9].Flags |= PUME_BLOCKED;
			PUMES[9].Blocked_message_nr = 601;
		}
	}
	else
	{
		/* No -> Cannot wait */
		PUMES[9].Flags |= PUME_ABSENT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Talk
 * FUNCTION  : Talk (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 19.10.95 20:25
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Talk(UNLONG Data)
{
	MEM_HANDLE NPC_char_handle;
	UNSHORT NPC_index;
	UNSHORT Selected_word;
	UNSHORT Char_file_type;
	UNSHORT State;

	/* Get NPC index */
	NPC_index = Current_map_selection_data.NPC_index;

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger talk event for NPC */
			Trigger_NPC_event_chain(NPC_index, SPEAK_TRIGGER, 0);
		}
		else
		{
			/* No -> Is this a person ? */
			if ((VNPCs[NPC_index].NPC_type == PARTY_NPC) ||
			 (VNPCs[NPC_index].NPC_type == NPC_NPC))
			{
				/* Yes -> Is this person asleep ? */
				if (!_3D_map)
				{
					State = VNPCs[NPC_index].Data._2D_NPC_data.Move.State;
					if ((State == SLEEPING1_STATE) || (State == SLEEPING2_STATE))
					{
						/* Yes -> "Do not disturb!" */
						Set_permanent_message_nr(539);

						/* Exit */
						return;
					}
				}

				/* Turn towards NPC */
				Turn_towards_NPC(NPC_index);

				/* Long or short dialogue ? */
				if (VNPCs[NPC_index].Flags & NPC_SHORT_DIALOGUE)
				{
					/* Short -> Set ink colour */
					Set_ink(GOLD_TEXT);

					/* Do short dialogue */
					Do_text_file_window(Map_text_handle, VNPCs[NPC_index].Number);
				}
				else
				{
					/* Long -> Determine character data type
					 (At this point I can be sure that only these two types
					 of NPC are possible.) */
					switch (VNPCs[NPC_index].NPC_type)
					{
						case PARTY_NPC:
						{
							Char_file_type = PARTY_CHAR;
							break;
						}
						case NPC_NPC:
						{
							Char_file_type = NPC_CHAR;
							break;
						}
					}

					/* Load NPC character data */
					NPC_char_handle = Load_subfile
					(
						Char_file_type,
						VNPCs[NPC_index].Number
					);

					if (!NPC_char_handle)
					{
						Error(ERROR_FILE_LOAD);

						Exit_program();

						return;
					}

					/* Do any of the known languages of the active party
						member match with the NPC's ?  */
					if ((Get_known_languages(Active_char_handle) &
					 Get_known_languages(NPC_char_handle)) || Cheat_mode)
					{
						/* Yes -> Destroy NPC character data */
						MEM_Free_memory(NPC_char_handle);

						/* Do dialogue */
						/* (the first two NPC types match exactly with the
						    character types) */
						Do_Dialogue
						(
							NORMAL_DIALOGUE,
							(UNSHORT) VNPCs[NPC_index].NPC_type,
							VNPCs[NPC_index].Number,
							NPC_index
						);
					}
					else
					{
						/* No -> Destroy NPC character data */
						MEM_Free_memory(NPC_char_handle);

						/* "No comprendo, signor." */
						Set_permanent_message_nr(540);
					}
				}
			}
		}
	}
	else
	{
		/* No -> Select a word from the word list */
		Selected_word = Select_word();

		/* Any selected ? */
		if (Selected_word != 0xFFFF)
		{
			/* Yes -> Trigger talk event for map */
			Trigger_map_event_chain
			(
				Current_map_selection_data.Map_X,
				Current_map_selection_data.Map_Y,
				0xFFFF,
				SPEAK_TRIGGER,
				Selected_word
			);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Examine
 * FUNCTION  : Examine (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 22.10.95 18:55
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Examine(UNLONG Data)
{
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;

	/* Get NPC index */
	NPC_index = Current_map_selection_data.NPC_index;

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger examine event for NPC */
			Trigger_NPC_event_chain(NPC_index, EXAMINE_TRIGGER, 0);

			/* There was an NPC event */
			NPC_event = TRUE;
		}
	}

	/* Was there an NPC event ? */
	if (!NPC_event)
	{
		/* No -> Trigger examine event for map */
		Trigger_map_event_chain
		(
			Current_map_selection_data.Map_X,
			Current_map_selection_data.Map_Y,
			0xFFFF,
			EXAMINE_TRIGGER,
			0
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Manipulate
 * FUNCTION  : Manipulate (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 22.10.95 18:55
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Manipulate(UNLONG Data)
{
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;

	/* Get NPC index */
	NPC_index = Current_map_selection_data.NPC_index;

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger touch event for NPC */
			Trigger_NPC_event_chain(NPC_index, TOUCH_TRIGGER, 0);

			/* There was an NPC event */
			NPC_event = TRUE;
		}
	}

	/* Was there an NPC event ? */
	if (!NPC_event)
	{
		/* No -> Trigger touch event for map */
		Trigger_map_event_chain
		(
			Current_map_selection_data.Map_X,
			Current_map_selection_data.Map_Y,
			0xFFFF,
			TOUCH_TRIGGER,
			0
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Use_item
 * FUNCTION  : Use item (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 22.10.95 18:56
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Use_item(UNLONG Data)
{
	struct Character_data *Char;
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;
	UNSHORT Selected_item_slot_index;
	UNSHORT Used_item_index;

	/* Select item to be used */
	Selected_item_slot_index = Select_party_member_item
	(
		PARTY_DATA.Active_member,
		System_text_ptrs[430],
		NULL
	);

	/* Any selected ? */
	if (Selected_item_slot_index != 0xFFFF)
	{
		/* Yes -> Get selected item index */
		Char = (struct Character_data *) MEM_Claim_pointer(Active_char_handle);

		if (Selected_item_slot_index <= ITEMS_ON_BODY)
		{
			Used_item_index = Char->Body_items[Selected_item_slot_index - 1].Index;
		}
		else
		{
			Used_item_index = Char->Backpack_items[Selected_item_slot_index -
			 ITEMS_ON_BODY - 1].Index;
		}

		MEM_Free_pointer(Active_char_handle);

		/* Store item use data */
		Used_item_char_handle	= Active_char_handle;
		Used_item_slot_index		= Selected_item_slot_index;

		/* Get NPC index */
		NPC_index = Current_map_selection_data.NPC_index;

		/* Is there an NPC here ? */
		if (NPC_index != 0xFFFF)
		{
			/* Yes -> Has event ? */
			if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
			{
				/* Yes -> Trigger use item event for NPC */
				Trigger_NPC_event_chain
				(
					NPC_index,
					USE_ITEM_TRIGGER,
					Used_item_index
				);

				/* There was an NPC event */
				NPC_event = TRUE;
			}
		}

		/* Was there an NPC event ? */
		if (!NPC_event)
		{
			/* No -> Trigger use item event for map */
			Trigger_map_event_chain
			(
				Current_map_selection_data.Map_X,
				Current_map_selection_data.Map_Y,
				0xFFFF,
				USE_ITEM_TRIGGER,
				Used_item_index
			);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Take_item
 * FUNCTION  : Take item (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 10:46
 * LAST      : 22.10.95 18:56
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Take_item(UNLONG Data)
{
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;

	/* Get NPC index */
	NPC_index = Current_map_selection_data.NPC_index;

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger take event for NPC */
			Trigger_NPC_event_chain(NPC_index, TAKE_TRIGGER, 0);

			/* There was an NPC event */
			NPC_event = TRUE;
		}
	}

	/* Was there an NPC event ? */
	if (!NPC_event)
	{
		/* No -> Trigger take event for map */
		Trigger_map_event_chain
		(
			Current_map_selection_data.Map_X,
			Current_map_selection_data.Map_Y,
			0xFFFF,
			TAKE_TRIGGER,
			0
		);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Change_transport
 * FUNCTION  : Change transport (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:57
 * LAST      : 28.12.94 16:57
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Change_transport(UNLONG Data)
{
	// NOT IMPLEMENTED
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Enter_automapper
 * FUNCTION  : Enter the automapper (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 16:30
 * LAST      : 31.08.95 12:53
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Enter_automapper(UNLONG Data)
{
	Enter_Automap(0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Build_camp
 * FUNCTION  : Build a camp (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 14:15
 * LAST      : 19.10.95 14:32
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Build_camp(UNLONG Data)
{
	/* Are you sure ? */
	if (Boolean_requester(System_text_ptrs[726]))
	{
		/* Yes -> Camp */
		Enter_Camp(FALSE);

		/* Reset palette blending */
		Reset_map_palette_blending();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Wait
 * FUNCTION  : Wait (map pop-up menu).
 * FILE      : MAP_PUM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.10.95 21:37
 * LAST      : 10.10.95 00:53
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Wait(UNLONG Data)
{
	UNSHORT Nr_hours;

	/* Wait for how long ? */
	Nr_hours = (UNSHORT) Input_number
	(
		1,
		0,
		24,
		System_text_ptrs[724]
	);

	/* Any ? */
	if (Nr_hours)
	{
		/* Yes -> Wait */
		Wait_x_hours(Nr_hours);

		/* Reset palette blending */
		Reset_map_palette_blending();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Main_menu
 * FUNCTION  : Enter main menu (map pop-up menu).
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.09.95 23:00
 * LAST      : 02.10.95 17:48
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Main_menu(UNLONG Data)
{
	Enter_Main_menu(MAIN_MENU_MAP);
}

