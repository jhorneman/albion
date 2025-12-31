/************
 * NAME     : PRTLOGIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-7-95
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>
#include <TEXT.H>

#include <MAIN.H>
#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <EVELOGIC.H>
#include <ITMLOGIC.H>
#include <PRTLOGIC.H>
#include <2D_MAP.H>
#include <XFTYPES.H>
#include <SAVELOAD.H>
#include <FONT.H>
#include <STATAREA.H>
#include <MUSIC.H>
#include <USERFACE.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <COMBAT.H>
#include <DIALOGUE.H>
#include <GRAPHICS.H>
#include <MEMBRSEL.H>

/* prototypes */

/* global variables */

/* Party data handles */
MEM_HANDLE Party_char_handles[6];
MEM_HANDLE Small_portrait_handles[6];

MEM_HANDLE Active_char_handle;

MEM_HANDLE Party_event_set_handles[6][2];
MEM_HANDLE Party_event_text_handles[6][2];

MEM_HANDLE Default_event_set_handle;
MEM_HANDLE Default_event_text_handle;

struct Party_data PARTY_DATA;

UNLONG Body_items_offset;
UNLONG Backpack_items_offset;

static UNSHORT Level_factors[LEVEL_MAX];
static UNLONG Level_start_EP[CLASSES_MAX] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1,	1, 1, 1, 1,
	1, 1, 1, 1,	1, 1, 1, 1, 1, 1,
	1, 1
};

UNSHORT Compass_bit_coordinates[60][2];

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_game_data
 * FUNCTION  : Initialize game data.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:42
 * LAST      : 12.09.95 21:56
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_game_data(void)
{
	BOOLEAN Result;

	/* Calculate data offsets */
	Body_items_offset = (UNLONG) offsetof(struct Character_data, Body_items[0]);
	Backpack_items_offset = (UNLONG) offsetof(struct Character_data, Backpack_items[0]);

	/* Initialize game texts */
	Init_game_texts();

	/* Load default event-set and -texts */
	Default_event_set_handle = Load_subfile(EVENT_SET, DEFAULT_EVENT_SET);
	if (!Default_event_set_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	Default_event_text_handle = Load_subfile(EVENT_TEXT, DEFAULT_EVENT_SET);
	if (!Default_event_text_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load all item data */
	Result = Init_item_data();
	if (!Result)
		return FALSE;

	/* Load all magic data */
	Result = Init_magic_data();
	if (!Result)
		return FALSE;

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_game_data
 * FUNCTION  : Delete game data.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:42
 * LAST      : 29.06.95 18:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_game_data(void)
{
	/* Exit game texts */
	Exit_game_texts();

	/* Destroy default event-set and -texts */
	MEM_Free_memory(Default_event_set_handle);
	MEM_Free_memory(Default_event_text_handle);

	/* Destroy item data */
	Exit_item_data();

	/* Destroy magic data */
	Exit_magic_data();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_game_tables
 * FUNCTION  : Calculate tables needed by the game.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.05.95 12:20
 * LAST      : 10.08.95 14:40
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Calculate_game_tables(void)
{
	UNSHORT Old;
	UNSHORT New;
	UNSHORT Index;
	UNSHORT i;

	/* Calculate positions of compass bit */
	Index = 0;
	for (i=0;i<360;i+=6)
	{
		Compass_bit_coordinates[Index][0] = 12 + 11 *
		 sin(((double) i * 2 * PI) / 360);

		Compass_bit_coordinates[Index][1] = 12 - 11 *
		 cos(((double) i * 2 * PI) / 360);

		Index++;
	}

	/* Calculate level factors */
	Old = 0;
	for (i=1;i<=LEVEL_MAX;i++)
	{
		New = i + Old;
		Level_factors[i - 1] = New;
		Old = New;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_party_data
 * FUNCTION  : Initialize the party members' data.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:22
 * LAST      : 04.09.95 18:24
 * INPUTS    : UNSHORT Load_game_nr - Number of saved game that should be
 *              loaded / 0 for restart.
 *             BOOLEAN No_load_flag - Set if file parts should not be changed.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_party_data(UNSHORT Load_game_nr, BOOLEAN No_load_flag)
{
	struct Character_data *Char;
	BOOLEAN Result;
	UNSHORT Batch[12];
	UNSHORT i;

	/* Select and load game state */
	if (Load_game_nr)
	{
		Result = Load_game_state(Load_game_nr, No_load_flag);
		if (!Result)
			return FALSE;
	}
	else
	{
	 	Load_initial_game_state();
	}

	/* Build party character data batch */
	for (i=0;i<6;i++)
	{
		Batch[i] = PARTY_DATA.Member_nrs[i];
	}

	/* Load party character data */
	Result = Load_partial_batch(PARTY_CHAR, 6, &Batch[0],
	 &Party_char_handles[0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Set active character handle */
	Active_char_handle = Party_char_handles[PARTY_DATA.Active_member - 1];

	/* Build small portraits batch */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Put portrait number into batch */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Party_char_handles[i]);

			Batch[i] = (UNSHORT) Char->Portrait_nr;

			MEM_Free_pointer(Party_char_handles[i]);
		}
		else
		{
			/* No */
			Batch[i] = 0;
		}
	}

	/* Load small portraits */
	Result = Load_partial_batch(SMALL_PORTRAIT, 6, &Batch[0],
	 &Small_portrait_handles[0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Build party event-set and -text batch */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Put event set numbers into batch */
			Batch[2 * i]		= Get_event_set_index(Party_char_handles[i], 0);
			Batch[2 * i + 1]	= Get_event_set_index(Party_char_handles[i], 1);
		}
		else
		{
			/* No */
			Batch[2 * i]		= 0;
			Batch[2 * i + 1]	= 0;
		}
	}

	/* Load event sets */
	Result = Load_partial_batch(EVENT_SET, 12, &Batch[0],
	 &Party_event_set_handles[0][0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load event texts */
	Result = Load_partial_batch(EVENT_TEXT, 12, &Batch[0],
	 &Party_event_text_handles[0][0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_party_data
 * FUNCTION  : Delete all party members' data.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:22
 * LAST      : 14.11.94 10:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_party_data(void)
{
	UNSHORT i;

	/* Destroy party members' data */
	for (i=0;i<6;i++)
	{
		MEM_Free_memory(Party_char_handles[i]);
		MEM_Free_memory(Small_portrait_handles[i]);
		MEM_Free_memory(Party_event_set_handles[i][0]);
		MEM_Free_memory(Party_event_set_handles[i][1]);
		MEM_Free_memory(Party_event_text_handles[i][0]);
		MEM_Free_memory(Party_event_text_handles[i][1]);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_bit_array
 * FUNCTION  : Clear a bit array.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 16:25
 * LAST      : 05.07.95 10:19
 * INPUTS    : UNSHORT Array_type - Array type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_bit_array(UNSHORT Array_type)
{
	UNLONG Max = 0;
	UNSHORT i;
	UNBYTE *Array = NULL;

	/* Which bit array ? */
	switch (Array_type)
	{
		/* Quest bits */
		case QUEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Quest;
			Max = QUEST_MAX;
			break;
		}
		/* Chest bits */
		case CHEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Chest_open;
			Max = CHESTS_MAX;
			break;
		}
		/* Door bits */
		case DOOR_BIT_ARRAY:
		{
			Array = PARTY_DATA.Door_open;
			Max = DOORS_MAX;
			break;
		}
		/* Character Deleted bits */
		case CD_BIT_ARRAY:
		{
			Array = PARTY_DATA.CD;
			Max = CD_MAX;
			break;
		}
		/* Event save bits */
		case EVENT_SAVE_BIT_ARRAY:
		{
			Array = PARTY_DATA.Event;
			Max = EVENT_MAX;
			break;
		}
		/* Known word bits */
		case KNOWN_WORDS_BIT_ARRAY:
		{
			Array = Known_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* New word bits */
		case NEW_WORDS_BIT_ARRAY:
		{
			Array = New_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* Goto-point bits */
		case GOTO_POINTS_BIT_ARRAY:
		{
			Array = PARTY_DATA.Goto_points;
			Max = GOTO_POINTS_MAX;
			break;
		}
		/* Error */
		default:
		{
			/* Report error */
			Error(ERROR_BIT_ARRAY_TYPE);
			return;
		}
	}

	/* Legal array ? */
	if (Array)
	{
		/* Yes -> Clear bit array */
		for (i=0;i<((Max + 7) / 8);i++)
			Array[i] = 0;
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_BIT_ARRAY_ILLEGAL_ARRAY);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Read_bit_array
 * FUNCTION  : Read a bit from a bit array.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:53
 * LAST      : 05.07.95 10:19
 * INPUTS    : UNSHORT Array_type - Array type.
 *             UNLONG Bit_number - Bit number.
 * RESULT    : BOOLEAN : TRUE (bit was set) or FALSE (bit was cleared).
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Read_bit_array(UNSHORT Array_type, UNLONG Bit_number)
{
	UNLONG Max = 0;
	UNBYTE *Array = NULL;

	/* Which bit array ? */
	switch (Array_type)
	{
		/* Quest bits */
		case QUEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Quest;
			Max = QUEST_MAX;
			break;
		}
		/* Chest bits */
		case CHEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Chest_open;
			Max = CHESTS_MAX;
			break;
		}
		/* Door bits */
		case DOOR_BIT_ARRAY:
		{
			Array = PARTY_DATA.Door_open;
			Max = DOORS_MAX;
			break;
		}
		/* Character Deleted bits */
		case CD_BIT_ARRAY:
		{
			Array = PARTY_DATA.CD;
			Max = CD_MAX;
			break;
		}
		/* Event save bits */
		case EVENT_SAVE_BIT_ARRAY:
		{
			Array = PARTY_DATA.Event;
			Max = EVENT_MAX;
			break;
		}
		/* Known word bits */
		case KNOWN_WORDS_BIT_ARRAY:
		{
			Array = Known_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* New word bits */
		case NEW_WORDS_BIT_ARRAY:
		{
			Array = New_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* Goto-point bits */
		case GOTO_POINTS_BIT_ARRAY:
		{
			Array = PARTY_DATA.Goto_points;
			Max = GOTO_POINTS_MAX;
			break;
		}
		/* Error */
		default:
		{
			/* Report error */
			Error(ERROR_BIT_ARRAY_TYPE);
			return FALSE;
		}
	}

	/* Legal array ? */
	if (Array)
	{
		/* Yes -> Bit number in range ? */
		if (Bit_number < Max)
		{
			/* Yes -> Read bit */
			return((BOOLEAN) ((Array[Bit_number >> 3]) &
			 (1 << (Bit_number & 0x0007))));
		}
		else
		{
			/* No -> Report error */
			Error(ERROR_BIT_ARRAY_OUT_OF_RANGE);

			/* False */
			return FALSE;
		}
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_BIT_ARRAY_ILLEGAL_ARRAY);

		/* False */
		return FALSE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_bit_array
 * FUNCTION  : Write a bit from a bit array.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:53
 * LAST      : 05.07.95 10:19
 * INPUTS    : UNSHORT Array_type - Array type.
 *             UNLONG Bit_number - Bit number.
 *             UNSHORT Write_mode - Write mode.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Write_bit_array(UNSHORT Array_type, UNLONG Bit_number, UNSHORT Write_mode)
{
	UNLONG Max = 0;
	UNBYTE *Array = NULL;

	/* Which bit array ? */
	switch (Array_type)
	{
		/* Quest bits */
		case QUEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Quest;
			Max = QUEST_MAX;
			break;
		}
		/* Chest bits */
		case CHEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Chest_open;
			Max = CHESTS_MAX;
			break;
		}
		/* Door bits */
		case DOOR_BIT_ARRAY:
		{
			Array = PARTY_DATA.Door_open;
			Max = DOORS_MAX;
			break;
		}
		/* Character Deleted bits */
		case CD_BIT_ARRAY:
		{
			Array = PARTY_DATA.CD;
			Max = CD_MAX;
			break;
		}
		/* Event save bits */
		case EVENT_SAVE_BIT_ARRAY:
		{
			Array = PARTY_DATA.Event;
			Max = EVENT_MAX;
			break;
		}
		/* Known word bits */
		case KNOWN_WORDS_BIT_ARRAY:
		{
			Array = Known_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* New word bits */
		case NEW_WORDS_BIT_ARRAY:
		{
			Array = New_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* Goto-point bits */
		case GOTO_POINTS_BIT_ARRAY:
		{
			Array = PARTY_DATA.Goto_points;
			Max = GOTO_POINTS_MAX;
			break;
		}
		/* Error */
		default:
		{
			/* Report error */
			Error(ERROR_BIT_ARRAY_TYPE);
			return;
		}
	}

	/* Legal array ? */
	if (Array)
	{
		/* Yes -> Bit number in range ? */
		if (Bit_number < Max)
		{
			/* Yes -> Which write mode ? */
			switch (Write_mode)
			{
				/* Set bit */
				case SET_BIT_ARRAY:
				{
					Array[Bit_number >> 3] |= (1 << (Bit_number & 0x0007));
					break;
				}
				/* Clear bit */
				case CLEAR_BIT_ARRAY:
				{
					Array[Bit_number >> 3] &= ~(1 << (Bit_number & 0x0007));
					break;
				}
				/* Toggle bit */
				case TOGGLE_BIT_ARRAY:
				{
					Array[Bit_number >> 3] ^= (1 << (Bit_number & 0x0007));
					break;
				}
				/* Error */
				default:
				{
					/* Report error and exit */
					Error(ERROR_BIT_ARRAY_ACCESS_TYPE);
					return;
				}
			}

			/* Demon logic */
			/* Which bit array ? */
			switch (Array_type)
			{
				/* Character Deleted bits */
				case CD_BIT_ARRAY:
				{
					/* Update present status of all NPCs */
					Update_NPC_present_status();
					break;
				}
			}
		}
		else
		{
			/* No -> Report error */
			Error(ERROR_BIT_ARRAY_OUT_OF_RANGE);
		}
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_BIT_ARRAY_ILLEGAL_ARRAY);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_new_active_member
 * FUNCTION  : Select a new active party member.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 15:32
 * LAST      : 16.08.95 20:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_new_active_member(void)
{
	UNSHORT Member_index;

	/* No active member */
	PARTY_DATA.Active_member = 0;

	/* Select a party member (aborting not possible) */
	/* (this function will exit immediately if no party members can be
		 active. The result will be 0) */
	Member_index = Select_party_member(System_text_ptrs[545],
	 Can_be_active_member_evaluator, 541);

	/* Anyone selected ? */
	if (Member_index && (Member_index != 0xFFFF))
	{
		/* Yes -> Activate this member */
		Activate_party_member(Member_index);
	}
	else
	{
		/* No -> Game over */
		Game_over();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Game_over
 * FUNCTION  : Game over.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 20:52
 * LAST      : 16.08.95 20:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Game_over(void)
{
	Exit_program();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Activate_party_member
 * FUNCTION  : Activate a party member.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 10:52
 * LAST      : 10.08.95 15:20
 * INPUTS    : UNSHORT Member_nr - Member index (1...6).
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Activate_party_member(UNSHORT Member_nr)
{
	static struct Event_action Activate_party_member_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		MAKE_ACTIVE_ACTION, 0, 0,
		Do_activate_party_member, NULL, NULL
	};

	BOOLEAN Result;

	/* Anyone there ? */
	if (!PARTY_DATA.Member_nrs[Member_nr-1])
		return FALSE;

	/* Exit if this member cannot be active */
	if (Get_conditions(Party_char_handles[Member_nr - 1]) & ACTIVE_MASK)
		return FALSE;

	/* Exit if this member is already active */
	if (PARTY_DATA.Active_member == Member_nr)
		return TRUE;

	/* Are we in dialogue ? */
	if (In_Dialogue)
	{
		/* Yes -> With this party member ? */
		if ((Dialogue_char_type == PARTY_CHAR_TYPE) &&
		 (Dialogue_char_index == PARTY_DATA.Member_nrs[Member_nr-1]))
		{
			/* Yes -> Exit */
			return FALSE;
		}
		else
		{
			/* Do any of the known languages of the new party member match with
			 the dialogue partner's ? */
			if (!(Get_known_languages(Party_char_handles[Member_nr - 1]) &
			 Get_known_languages(Dialogue_char_handle)) && !Cheat_mode)
			{
				/* No -> "No comprendo, signor." */
				Set_permanent_message_nr(543);

				/* Exit */
				return FALSE;
			}
		}
	}

	/* Try to activate this party member */
	Activate_party_member_action.Actor_index = Member_nr;
	Result = Perform_action(&Activate_party_member_action);

	return Result;
}

BOOLEAN
Do_activate_party_member(struct Event_action *Action)
{
	UNSHORT Member_nr;

	/* Get member index */
	Member_nr = Action->Actor_index;

	/* Make this member active */
	Active_char_handle = Party_char_handles[Member_nr - 1];
	PARTY_DATA.Active_member = Member_nr;

	/* Do stuff when in combat */

	return TRUE;
}

/*
	move.b	d5,Active_handle		; Store
	move.w	d7,Active_member
	jsr	Check_active_member		; Update status
	tst.b	Battling			; In combat ?
	beq.s	.No2
	lea.l	Combat_party,a0		; Set active participant
	subq.w	#1,d7
	mulu.w	#Participant_data_size,d7
	add.l	d7,a0
	move.l	a0,Active_participant
.No2:	moveq.l	#0,d6			; Success !
.Exit:	Free	d5
.Exit2:	tst.w	d6			; Any luck ?
	movem.l	(sp)+,d0/d1/d5-d7/a0/a1
	rts
*/

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_character_in_party
 * FUNCTION  : Find a character in the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 17:12
 * LAST      : 24.07.95 10:35
 * INPUTS    : UNSHORT Char_index - Character index.
 * RESULT    : UNSHORT : Index of character in party (1...6) / 0xFFFF =
 *              character is not in party.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Find_character_in_party(UNSHORT Char_index)
{
	UNSHORT Result = 0xFFFF;
	UNSHORT i;

	/* Search party */
	for (i=0;i<6;i++)
	{
		/* Is this the right character ? */
		if (PARTY_DATA.Member_nrs[i] == Char_index)
		{
			/* Yes */
			Result = i + 1;
			break;
		}
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_alive
 * FUNCTION  : Check if a character is alive.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 20:59
 * LAST      : 16.08.95 20:59
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE (alive) or FALSE (dead).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_alive(MEM_HANDLE Char_handle)
{
	if (Get_conditions(Char_handle) & DEAD_MASK)
		return FALSE;
	else
		return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_capable
 * FUNCTION  : Check if a character is capable.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.01.95 10:23
 * LAST      : 26.01.95 10:23
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE (capable) or FALSE (incapable).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_capable(MEM_HANDLE Char_handle)
{
	if (Get_conditions(Char_handle) & INCAPABLE_MASK)
		return FALSE;
	else
		return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_can_see
 * FUNCTION  : Check if the party can see.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 16:39
 * LAST      : 07.02.95 16:39
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (can see) or FALSE (cannot see).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_can_see(void)
{
	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_is_full
 * FUNCTION  : Check if the party is full.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.07.95 17:35
 * LAST      : 03.07.95 17:35
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (is full) or FALSE (is not full).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_is_full(void)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Check party */
	for (i=0;i<6;i++)
	{
		/* Free slot ? */
		if (!PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Party is not full */
			Result = FALSE;

			/* Exit */
			break;
		}
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_has_magical_abilities
 * FUNCTION  : Check if a character has magical abilities.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 14:45
 * LAST      : 04.04.95 14:45
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_has_magical_abilities(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes */
		Result = TRUE;
	}
	else
	{
		/* No -> Check */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Any spell classes known ? */
		Result = (BOOLEAN)(Char->xKnown_spell_classes);

		MEM_Free_pointer(Char_handle);
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_knows_spells
 * FUNCTION  : Check if a character knows any spells.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 14:12
 * LAST      : 05.04.95 14:12
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_knows_spells(MEM_HANDLE Char_handle)
{
	BOOLEAN Result = FALSE;
	UNSHORT i, j;

	/* Does the character have any magical abilities ? */
	if (Character_has_magical_abilities(Char_handle))
	{
		/* Yes -> Check all spells */
		for (i=0;i<SPELL_CLASSES_MAX;i++)
		{
			/* Check all spells in this class */
			for (j=0;j<SPELLS_PER_CLASS;j++)
			{
				/* Is this spell known ? */
				if (Spell_known(Char_handle, i, j))
				{
					/* Yes! */
					Result = TRUE;
					break;
				}
			}

			/* Know any spells yet ? */
			if (Result)
				break;
		}
	}
	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_is_overweight
 * FUNCTION  : Check if a character is overweight.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 11:07
 * LAST      : 20.06.95 11:07
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_is_overweight(MEM_HANDLE Char_handle)
{
	/* Carrying too much ? */
	if (Get_carried_weight(Char_handle) >= ((UNLONG) Get_attribute(Char_handle,
	 STRENGTH) * 1000))
		return TRUE;
	else
		return FALSE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Probe
 * FUNCTION  : Probe a value. A random number between 0 and the probe range
 *              is determined. If this number is smaller or equal to the
 *              probe value, the probe was successful.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 12:49
 * LAST      : 13.03.95 12:49
 * INPUTS    : SISHORT Value - Probe value.
 *             UNSHORT Range - Probe range.
 * RESULT    : BOOLEAN : Probe success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Probe(SISHORT Value, UNSHORT Range)
{
	/* Zero or lower ? */
	if (Value < 0)
		return FALSE;

	/* Equal to or above probe range ? */
	if (Value >= Range)
		return TRUE;

	/* Probe */
	return((rand() % Range) <= Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Probe_attribute
 * FUNCTION  : Probe a character's attribute.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 12:52
 * LAST      : 13.03.95 12:52
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Attribute_nr - Attribute index.
 * RESULT    : BOOLEAN : Probe success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Probe_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr)
{
	return(Probe(Get_attribute(Char_handle, Attribute_nr), 100));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Probe_skill
 * FUNCTION  : Probe a character's skill.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 12:53
 * LAST      : 13.03.95 12:53
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Skill_nr - Skill index.
 * RESULT    : BOOLEAN : Probe success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Probe_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr)
{
	return(Probe(Get_skill(Char_handle, Skill_nr), 100));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_rnd_50_100
 * FUNCTION  : Get random value between 50% of 100% of input value.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 13:52
 * LAST      : 31.03.95 13:52
 * INPUTS    : UNSHORT Value - Input value.
 * RESULT    : UNSHORT : Return value.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_rnd_50_100(UNSHORT Value)
{
	return((((rand() % 51) + 50) * Value) / 100);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_attribute
 * FUNCTION  : Get a character's attribute.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:44
 * LAST      : 14.09.95 20:48
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Attribute_nr - Attribute index (1...).
 * RESULT    : UNSHORT : Attribute value.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr)
{
	struct Character_data *Char;
	UNSHORT Value = 0;

	/* Legal attribute number ? */
	if ((Attribute_nr > 0) && (Attribute_nr <= ATTRS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Get attribute value */
		Value = Char->Attributes[Attribute_nr - 1].Normal;

		MEM_Free_pointer(Char_handle);
	}

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_attribute
 * FUNCTION  : Set a character's attribute.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:21
 * LAST      : 31.03.95 14:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Attribute_nr - Attribute index (1...).
 *             SISHORT Value - New value.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr, SISHORT Value)
{
	struct Character_data *Char;

	/* Legal attribute number ? */
	if ((Attribute_nr > 0) && (Attribute_nr <= ATTRS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Set attribute value */
		Char->Attributes[Attribute_nr - 1].Normal = Value;

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_skill
 * FUNCTION  : Get a character's skill.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:45
 * LAST      : 14.09.95 20:49
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Skill_nr - Skill index (1...).
 * RESULT    : UNSHORT : Skill value.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr)
{
	struct Character_data *Char;
	UNSHORT Value = 0;

	/* Legal skill number ? */
	if ((Skill_nr > 0) && (Skill_nr <= SKILLS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Get skill value */
		Value = Char->Skills[Skill_nr - 1].Normal;

		MEM_Free_pointer(Char_handle);
	}

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_skill
 * FUNCTION  : Set a character's skill.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:22
 * LAST      : 31.03.95 14:22
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Skill_nr - Skill index (1...).
 *             SISHORT Value - New value.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr, SISHORT Value)
{
	struct Character_data *Char;

	/* Legal skill number ? */
	if ((Skill_nr > 0) && (Skill_nr <= SKILLS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Set skill value */
		Char->Skills[Skill_nr - 1].Normal = Value;

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_condition
 * FUNCTION  : Set a character's body condition.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:23
 * LAST      : 08.03.95 16:23
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Condition_nr - Body condition index.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr)
{
	struct Character_data *Char;

	/* Legal condition ? */
	if (Condition_nr  < MAX_CONDITIONS)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Set condition */
		Char->xBody_conditions |= (1 << Condition_nr);

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_condition
 * FUNCTION  : Clear a character's body condition.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:24
 * LAST      : 08.03.95 16:24
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Condition_nr - Body condition index.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr)
{
	struct Character_data *Char;

	/* Legal condition ? */
	if (Condition_nr  < MAX_CONDITIONS)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Clear condition */
		Char->xBody_conditions &= ~(1 << Condition_nr);

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_conditions
 * FUNCTION  : Get a character's body conditions.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 18:25
 * LAST      : 07.03.95 18:25
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Body conditions bit-list.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_conditions(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get conditions */
	Value = Char->xBody_conditions;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_char_name
 * FUNCTION  : Get a character's name.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:51
 * LAST      : 13.03.95 16:51
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNCHAR *Name - Pointer to name buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The buffer should be at least CHAR_NAME_LENGTH + 1 bytes long.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_char_name(MEM_HANDLE Char_handle, UNCHAR *Name)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Copy character name */
//	strcpy(Name, Char->xName[Language]);
	strcpy(Name, Char->xName[0]);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_class
 * FUNCTION  : Get a character's class.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:09
 * LAST      : 04.04.95 13:09
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Class.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_class(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get class */
	Value = (UNSHORT) Char->xClass;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_level
 * FUNCTION  : Get a character's level.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:12
 * LAST      : 04.04.95 13:12
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Level.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_level(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get level */
	Value = (UNSHORT) Char->xLevel;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_sex
 * FUNCTION  : Get a character's sex.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:21
 * LAST      : 04.04.95 13:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Sex.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_sex(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get sex */
	Value = (UNSHORT) Char->xSex;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_race
 * FUNCTION  : Get a character's race.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:22
 * LAST      : 04.04.95 13:22
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Race.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_race(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get race */
	Value = (UNSHORT) Char->xRace;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_known_languages
 * FUNCTION  : Get a character's known languages list.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.07.95 13:33
 * LAST      : 24.07.95 13:33
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Known languages bitlist.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_known_languages(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get known languages */
	Value = (UNSHORT) Char->Known_languages;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_LP
 * FUNCTION  : Get a character's life points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:27
 * LAST      : 31.03.95 14:27
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Life points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_LP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get life points */
	Value = Char->xLife_points;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_max_LP
 * FUNCTION  : Get a character's maximum life points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 20:10
 * LAST      : 14.09.95 20:10
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Maximum life points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_max_LP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get life points */
	Value = Char->xLife_points_maximum;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_SP
 * FUNCTION  : Get a character's spell points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:28
 * LAST      : 31.03.95 14:28
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Spell points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_SP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get spell points */
	Value = Char->xSpell_points;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_max_SP
 * FUNCTION  : Get a character's maximum spell points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 20:11
 * LAST      : 14.09.95 20:11
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Maximum spell points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_max_SP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get spell points */
	Value = Char->xSpell_points_maximum;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_LP
 * FUNCTION  : Set a character's life points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.95 14:04
 * LAST      : 14.09.95 20:17
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT New_LP - New amount of life points.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may execute no logic concerning life or
 *              death of a character.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_LP(MEM_HANDLE Char_handle, UNSHORT New_LP)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set life points */
	Char->xLife_points = min(New_LP, Get_max_LP(Char_handle));

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_max_LP
 * FUNCTION  : Set a character's maximum life points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 20:25
 * LAST      : 14.09.95 20:25
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT New_max_LP - New maximum amount of life points.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may execute no logic concerning life or
 *              death of a character.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_max_LP(MEM_HANDLE Char_handle, UNSHORT New_max_LP)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set maximum life points */
	Char->xLife_points_maximum = min(New_max_LP, 0x7FFF);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_SP
 * FUNCTION  : Set a character's spell points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.95 14:05
 * LAST      : 14.09.95 20:17
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT New_SP - New amount of spell points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_SP(MEM_HANDLE Char_handle, UNSHORT New_SP)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set spell points */
	Char->xSpell_points = min(New_SP, Get_max_SP(Char_handle));

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_max_SP
 * FUNCTION  : Set a character's maximum spell points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.09.95 20:26
 * LAST      : 14.09.95 20:26
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT New_max_SP - New maximum amount of spell points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_max_SP(MEM_HANDLE Char_handle, UNSHORT New_max_SP)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set maximum spell points */
	Char->xSpell_points_maximum = min(New_max_SP, 0x7FFF);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_EP
 * FUNCTION  : Get a character's experience points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 15:19
 * LAST      : 31.03.95 15:19
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNLONG : Experience points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_EP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNLONG Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get experience points */
	Value = Char->xExperience_points;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_EP
 * FUNCTION  : Set a character's experience points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 15:20
 * LAST      : 31.03.95 15:20
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNLONG Value - New experience points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_EP(MEM_HANDLE Char_handle, UNLONG Value)
{
	struct Character_data *Char;
	UNLONG Old_value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get old value */
	Old_value = Char->xExperience_points;

	/* Set experience points */
	Char->xExperience_points = Value;

	MEM_Free_pointer(Char_handle);

	/* Increase ? */
	if (Value > Old_value)
	{
		/* Yes -> */
	}

	/* Check member levels */
	Check_member_levels();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_TP
 * FUNCTION  : Get a character's training points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:52
 * LAST      : 17.08.95 10:52
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Experience points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_TP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get training points */
	Value = Char->xTraining_points;

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_TP
 * FUNCTION  : Set a character's training points.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.08.95 10:52
 * LAST      : 17.08.95 10:52
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Value - New training points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_TP(MEM_HANDLE Char_handle, UNSHORT Value)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set training points */
	Char->xTraining_points = Value;

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_carried_weight
 * FUNCTION  : Get the weight carried by a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 12:25
 * LAST      : 03.04.95 12:25
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNLONG : Weight carried by character (in grammes).
 * BUGS      : No known.
 * NOTES     : - Although this function should be used to access the carried
 *              weight, it will still set the Carried_weight variable, just
 *              in case.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_carried_weight(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNLONG Weight;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Calculate weight */
	Weight = 0;

	/* Add weight of body items */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything in body slot ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes -> Add weight */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			Weight += (UNLONG) Item_data->Weight;

			Free_item_data();
		}
	}

	/* Add weight of backpack items */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Anything in backpack slot ? */
		if (!Packet_empty(&(Char->Backpack_items[i])))
		{
			/* Yes -> Add weight */
			Item_data = Get_item_data(&(Char->Backpack_items[i]));

			Weight += (UNLONG) Item_data->Weight *
			 (UNSHORT) Char->Backpack_items[i].Quantity;

			Free_item_data();
		}
	}

	/* Add weight of gold */
	Weight += (UNLONG) Char->Char_gold * GOLD_WEIGHT;

	/* Add weight of food */
	Weight += (UNLONG) Char->Char_food * FOOD_WEIGHT;

	/* Write carried weight */
	Char->xCarried_weight = Weight;

	MEM_Free_pointer(Char_handle);

	return Weight;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_nr_occupied_hands
 * FUNCTION  : Get the number of occupied hands of a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:48
 * LAST      : 03.04.95 14:48
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Number of occupied hands.
 * BUGS      : No known.
 * NOTES     : - Although this function should be used to access the number
 *              of occupied hands, it will still set the Hands_occupied
 *              variable, just in case.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_nr_occupied_hands(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT Nr_hands;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Clear counter */
	Nr_hands = 0;

	/* Anything in the right hand? */
	if (!Packet_empty(&(Char->Body_items[RIGHT_HAND - 1])))
	{
		/* Yes -> Add number of hands to counter */
		Item_data = Get_item_data(&(Char->Body_items[RIGHT_HAND - 1]));
		Nr_hands += Item_data->Hand_use;
		Free_item_data();
	}

	/* Anything in the right hand? */
	if (!Packet_empty(&(Char->Body_items[LEFT_HAND - 1])))
	{
		/* Yes -> Add number of hands to counter */
		Item_data = Get_item_data(&(Char->Body_items[LEFT_HAND - 1]));
		Nr_hands += Item_data->Hand_use;
		Free_item_data();
	}

	/* Write number of occupied hands */
	Char->xHands_occupied = Nr_hands;

	MEM_Free_pointer(Char_handle);

	return Nr_hands;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event_set_index
 * FUNCTION  : Get the event set index of a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.07.95 20:43
 * LAST      : 30.07.95 20:43
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Char_set_index - Index of set per character (0...1).
 * RESULT    : UNSHORT : Event set index.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_event_set_index(MEM_HANDLE Char_handle, UNSHORT Char_set_index)
{
	struct Character_data *Char;
	UNSHORT Set_index;

	/* Exit if the set index is illegal */
	if (Char_set_index >= 2)
		return 0;

	/* Get event set index */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	Set_index = Char->xEvent_set_nrs[Char_set_index];

	MEM_Free_pointer(Char_handle);

	return Set_index;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_event_set_index
 * FUNCTION  : Set the event set index of a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.07.95 20:46
 * LAST      : 30.07.95 20:46
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Char_set_index - Index of set per character (0...1).
 *             UNSHORT Set_index - New event set index.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_event_set_index(MEM_HANDLE Char_handle, UNSHORT Char_set_index,
 UNSHORT Set_index)
{
	struct Character_data *Char;

	/* Exit if the set index is illegal */
	if (Char_set_index >= 2)
		return;

	/* Set event set index */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	Char->xEvent_set_nrs[Char_set_index] = Set_index;

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Spell_class_known
 * FUNCTION  : Check if a character knows a certain spell class.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:05
 * LAST      : 02.08.95 11:20
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 * RESULT    : BOOLEAN : Known.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Spell_class_known(MEM_HANDLE Char_handle, UNSHORT Class_nr)
{
	struct Character_data *Char;
	BOOLEAN Result = TRUE;

	/* Exit if the spell class number is illegal */
	if (Class_nr >= SPELL_CLASSES_MAX)
		return FALSE;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Spell class known ? */
	Result = (BOOLEAN)(Char->xKnown_spell_classes & (1 << Class_nr));

	MEM_Free_pointer(Char_handle);

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Spell_known
 * FUNCTION  : Check if a character knows a certain spell.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 12:37
 * LAST      : 02.08.95 11:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : BOOLEAN : Known.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Spell_known(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Character_data *Char;
	BOOLEAN Result = TRUE;

	/* Exit if the spell class number is illegal */
	if (Class_nr >= SPELL_CLASSES_MAX)
		return FALSE;

	/* Exit if the spell number is illegal */
	if (!Spell_nr || (Spell_nr > Spells_per_class[Class_nr]))
		return FALSE;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Cheat mode / party character ? */
	if (!Cheat_mode || (Char->Type != PARTY_CHAR_TYPE))
	{
		/* No -> Spell known ? */
		Result = (BOOLEAN)(Char->xKnown_spells[Class_nr] & (1 << Spell_nr));
	}

	MEM_Free_pointer(Char_handle);

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_strength
 * FUNCTION  : Get the strength of a certain spell of a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 12:46
 * LAST      : 02.08.95 11:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNLONG : Spell strength (0...10000).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_spell_strength(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Character_data *Char;
	UNLONG Value = 100 * 100;

	/* Exit if the spell class number is illegal */
	if (Class_nr >= SPELL_CLASSES_MAX)
		return 0;

	/* Exit if the spell number is too high */
	if (Spell_nr > Spells_per_class[Class_nr])
		return 0;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Cheat mode / party character ? */
	if (!Cheat_mode || (Char->Type != PARTY_CHAR_TYPE))
	{
		/* No -> Get spell strength */
		Value = (UNLONG) Char->xSpell_capabilities[Class_nr][Spell_nr - 1];
	}

	MEM_Free_pointer(Char_handle);

	return Value;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_spell_strength
 * FUNCTION  : Get the strength of a certain spell of a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 12:46
 * LAST      : 02.08.95 11:52
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 *             UNLONG Spell_strength - Spell strength (0...10000).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_spell_strength(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr,
 UNLONG Spell_strength)
{
	struct Character_data *Char;

	/* Exit if the spell class number is illegal */
	if (Class_nr >= SPELL_CLASSES_MAX)
		return;

	/* Exit if the spell number is too high */
	if (!Spell_nr || (Spell_nr > Spells_per_class[Class_nr]))
		return;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set spell strength */
	Char->xSpell_capabilities[Class_nr][Spell_nr - 1] =
	 (UNSHORT) Spell_strength;

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_damage
 * FUNCTION  : Get the damage a character can do.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:03
 * LAST      : 24.05.95 11:47
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Damage.
 * BUGS      : No known.
 * NOTES     : - This function should be used to access the amount of
 *              damage. The character data variable should NOT be accessed!
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_damage(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i;
	SISHORT Damage;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Calculate initial damage */
	Damage = Char->xDamage + Char->xDamage_magic;

	/* Add damage of body items */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything in body slot ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			/* Cursed ? */
			if (Char->Body_items[i].Flags & CURSED)
			{
				/* Yes -> Subtract damage value of item */
				Damage -= (UNSHORT) Item_data->Damage_pts;
			}
			else
			{
				/* No -> Add damage value of item */
				Damage += (UNSHORT) Item_data->Damage_pts;
			}

			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);

	return((UNSHORT) min(Damage, 0));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_protection
 * FUNCTION  : Get the protection a character has.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:06
 * LAST      : 24.05.95 11:47
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Protection.
 * BUGS      : No known.
 * NOTES     : - This function should be used to access the amount of
 *              protection. The character data variable should NOT be
 *              accessed!
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_protection(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i;
	SISHORT Protection;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Calculate initial protection */
	Protection = Char->xProtection + Char->xProtection_magic;

	/* Add protection of body items */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything in body slot ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			/* Cursed ? */
			if (Char->Body_items[i].Flags & CURSED)
			{
				/* Yes -> Subtract protection value of item */
				Protection -= (UNSHORT) Item_data->Protection_pts;
			}
			else
			{
				/* No -> Add protection value of item */
				Protection += (UNSHORT) Item_data->Protection_pts;
			}

			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);

	return((UNSHORT) min(Protection, 0));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_damage
 * FUNCTION  : Do damage to a party member.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 10:45
 * LAST      : 10.08.95 11:05
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 *             UNSHORT Damage - Amount of damage.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_damage(UNSHORT Member_nr, UNSHORT Damage)
{
	SISHORT Old_LP;
	SISHORT New_LP;

	/* Cheat mode ? */
	if (!Cheat_mode)
	{
		/* No -> Anyone there ? */
		if (Member_present(Member_nr))
		{
			/* Yes -> Decrease life points */
			Old_LP = (SISHORT) Get_LP(Party_char_handles[Member_nr - 1]);

			New_LP = max(Old_LP - Damage, 0);

			Set_LP(Party_char_handles[Member_nr - 1], (UNSHORT) New_LP);

			/* "Dead" ? */
			if (!New_LP)
			{
				/* Yes -> Kill party member */
				Kill_party_member(Member_nr);
			}
			else
			{
				/* No -> Show damage */
				Show_party_member_damage(Member_nr, Old_LP - New_LP);
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_party_member_damage
 * FUNCTION  : Show party member receiving damage.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 11:01
 * LAST      : 10.08.95 11:01
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 *             UNSHORT Damage - Amount of damage.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_party_member_damage(UNSHORT Member_nr, UNSHORT Damage)
{
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_party_member_healing
 * FUNCTION  : Show party member healing.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 11:07
 * LAST      : 10.08.95 11:07
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 *             UNSHORT Healing - Amount of healing (= negative damage).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_party_member_healing(UNSHORT Member_nr, UNSHORT Healing)
{
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Kill_party_member
 * FUNCTION  : "Kill" a party member.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 10:53
 * LAST      : 21.04.95 10:53
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Kill_party_member(UNSHORT Member_nr)
{
	/* Anyone there ? */
	if (Member_present(Member_nr))
	{
		/* Yes -> "Kill" */
		Set_condition(Party_char_handles[Member_nr - 1], UNCONSCIOUS);

		/* Clear life-points */
		Set_LP(Party_char_handles[Member_nr - 1], 0);

		/* Show "death" */

		/* Was this the active party member / not in combat ? */
		if ((Member_nr == PARTY_DATA.Active_member) && (!In_Combat))
		{
			/* Yes -> Select a new active member */
 			Select_new_active_member();
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Revive_party_member
 * FUNCTION  : "Revive" a party member.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 11:05
 * LAST      : 10.08.95 11:05
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Revive_party_member(UNSHORT Member_nr)
{
	/* Anyone there ? */
	if (Member_present(Member_nr))
	{
		/* Yes -> "Revive" */
		Clear_condition(Party_char_handles[Member_nr - 1], UNCONSCIOUS);

		/* Show "revival" */

	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : De_exhaust_party_member
 * FUNCTION  : De-exhaust a party member.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 15:30
 * LAST      : 01.09.95 15:30
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
De_exhaust_party_member(UNSHORT Member_nr)
{
	struct Character_data *Char;
	MEM_HANDLE Char_handle;
	UNSHORT i;

	/* Anyone there ? */
	if (Member_present(Member_nr))
	{
		/* Yes */
		Char_handle = Party_char_handles[Member_nr - 1];

		/* Exhausted ? */
		if (Get_conditions(Char_handle) & (1 <<EXHAUSTED))
		{
			/* Yes -> Remove exhaustion */
			Clear_condition(Char_handle, EXHAUSTED);

			Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

			/* Restore attributes */
			for (i=0;i<ATTRS_MAX;i++)
			{
				if (Char->Attributes[i].Normal < Char->Attributes[i].Backup)
					Char->Attributes[i].Normal = Char->Attributes[i].Backup;
			}

			/* Restore skills */
			for (i=0;i<SKILLS_MAX;i++)
			{
				if (Char->Skills[i].Normal < Char->Skills[i].Backup)
					Char->Skills[i].Normal = Char->Skills[i].Backup;
			}

			MEM_Free_pointer(Char_handle);
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_member_levels
 * FUNCTION  : Check if any party member's should advance to the next level.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 13:17
 * LAST      : 31.03.95 13:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function should be called whenever a party member's
 *              experience points are changed.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Check_member_levels(void)
{
	UNLONG Current_EP;
	UNLONG Start_EP;
	UNLONG Next_EP;
	UNSHORT Levels[6];
	UNSHORT Current_level;
	UNSHORT i, j;

	/* Check entire party */
	for (i=0;i<6;i++)
	{
		/* Clear level */
		Levels[i] = 1;

		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Is this character alive ? */
			if (!(Get_conditions(Party_char_handles[i]) & DEAD_MASK))
			{
				/* Yes -> Get current experience points */
				Current_EP = Get_EP(Party_char_handles[i]);

				/* Get start EP */
				Start_EP = Level_start_EP[Get_class(Party_char_handles[i])];

				/* While the maximum level hasn't been reached yet */
				while (Levels[i] < LEVEL_MAX)
				{
					/* Calculate the EP needed for the next level */
					/* (the levels start at 1 !) */
					Next_EP = Start_EP * Level_factors[Levels[i]];

					/* Enough for the next level ? */
					if (Current_EP >= Next_EP)
					{
						/* Yes -> Increase level */
						Levels[i]++;
					}
					else
					{
						/* No -> Break off */
						break;
					}
				}

				/* Any changes ? */
				Current_level = Get_level(Party_char_handles[i]);
				if (Levels[i] != Current_level)
				{
					/* Yes -> Increase or decrease ? */
					if (Levels[i] > Current_level)
					{
						/* Increase */
						for (j=0;j<Levels[i] - Current_level;j++)
						{
							Increase_character_level(Party_char_handles[i]);
						}
					}
				}
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Increase_character_level
 * FUNCTION  : Increase the level of a character.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:03
 * LAST      : 14.09.95 20:28
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only increases the level and character values
 *              and does not display anything.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Increase_character_level(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT New_APR = 0;
	UNSHORT Extra_LP;
	UNSHORT Extra_SP;
	UNSHORT Extra_TP;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Increase character level */
	Char->xLevel += 1;

	/* Is the APR factor zero ? */
	if (Char->Special[0])
	{
		/* No -> Calculate new attacks per round */
		New_APR = (UNSHORT) Get_level(Char_handle) / Char->Special[0];
	}

	/* Store new APR */
	Char->Attacks_per_round = max(1, New_APR);

	/* Calculate extra LP & LP max */
	Extra_LP = Get_rnd_50_100(Char->Special[1]);

	/* Add */
	Set_max_LP(Char_handle, Get_max_LP(Char_handle) + Extra_LP);
	Set_LP(Char_handle, Get_LP(Char_handle) + Extra_LP);

	/* Does this character have magical abilities ? */
	if (Character_has_magical_abilities(Char_handle))
	{
		/* Yes -> Calculate extra SP & SP max */
		Extra_SP = Get_rnd_50_100(Char->Special[2]);

		/* Add intelligence bonus */
		Extra_SP += Get_attribute(Char_handle, INTELLIGENCE) / 20;

		/* Add */
		Set_max_SP(Char_handle, Get_max_SP(Char_handle) + Extra_SP);
		Set_SP(Char_handle, Get_SP(Char_handle) + Extra_SP);
	}

	/* Calculate extra training points */
	Extra_TP = Get_rnd_50_100(Char->Special[4]);

	/* Add */
	Set_TP(Char_handle, Get_TP(Char_handle) + Extra_TP);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Char_inventory_empty
 * FUNCTION  : Check if a character's inventory is empty.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 14:35
 * LAST      : 27.04.95 14:35
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : BOOLEAN : Character's inventory is empty.
 * BUGS      : No known.
 * NOTES     : - Both body and backpack items are checked.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Char_inventory_empty(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check if character's inventory is empty */
	Result = (Inventory_empty(&(Char->Body_items[0]), ITEMS_ON_BODY)) |
	 (Inventory_empty(&(Char->Backpack_items[0]), ITEMS_PER_CHAR));

	MEM_Free_pointer(Char_handle);

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Char_inventory_full
 * FUNCTION  : Check if a character's inventory is full.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 14:38
 * LAST      : 27.04.95 14:38
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : BOOLEAN : Character's inventory is full.
 * BUGS      : No known.
 * NOTES     : - Only backpack items are checked.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Char_inventory_full(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check if character's inventory is empty */
	Result = Inventory_full(&(Char->Backpack_items[0]), ITEMS_PER_CHAR);

	MEM_Free_pointer(Char_handle);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_items_on_body
 * FUNCTION  : Count all the items of a certain type on a character's
 *              body.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 10:43
 * LAST      : 28.04.95 10:43
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Item_index - Index of item that must be counted.
 * RESULT    : UNSHORT : Counter.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_items_on_body(MEM_HANDLE Char_handle, UNSHORT Item_index)
{
 	struct Character_data *Char;
	UNSHORT Counter = 0;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check all item slots on body */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is this the right item ? */
		if (Char->Body_items[i].Index == Item_index)
		{
			/* Yes -> Count up */
			Counter++;
		}
	}

	MEM_Free_pointer(Char_handle);

	return Counter;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_items_in_backpack
 * FUNCTION  : Count all the items of a certain type in a character's
 *              backpack.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 23:11
 * LAST      : 27.04.95 23:11
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Item_index - Index of item that must be counted.
 * RESULT    : UNSHORT : Counter.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_items_in_backpack(MEM_HANDLE Char_handle, UNSHORT Item_index)
{
 	struct Character_data *Char;
	UNSHORT Counter = 0;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check all item slots in backpack */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Is this the right item ? */
		if (Char->Backpack_items[i].Index == Item_index)
		{
			/* Yes -> Count up */
			Counter += (UNSHORT) Char->Backpack_items[i].Quantity;
		}
	}

	MEM_Free_pointer(Char_handle);

	return Counter;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Magical_item_evaluator
 * FUNCTION  : Check if item is magical (item select window evaluator).
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:31
 * LAST      : 27.04.95 15:31
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Magical_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Magical item ? */
	if (Item_data->Spell_nr)
	{
		/* Yes -> Any charges left ? */
		if (Packet->Charges)
		{
			/* Yes -> Item is OK */
			Message_nr = 0;
		}
		else
		{
			/* No -> "No more charges" */
			Message_nr = 94;
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
 * NAME      : Vital_item_evaluator
 * FUNCTION  : Check if item is vital for solving the game (item select
 *              window evaluator).
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.07.95 17:20
 * LAST      : 03.07.95 17:20
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Vital_item_evaluator(struct Character_data *Char, struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr = 0;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is the item undroppable ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes -> "Item is vital" */
		Message_nr = 511;
	}

	Free_item_data();

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Alive_member_evaluator
 * FUNCTION  : Check if a party member is alive (member select window evaluator).
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 15:07
 * LAST      : 16.08.95 21:01
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Message nr / 0 = member is OK
 *              / 0xFFFF = member is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Alive_member_evaluator(MEM_HANDLE Char_handle)
{
	/* Is this party member alive ? */
	if (Character_alive(Char_handle))
	{
		/* Yes -> Member is OK */
		return 0;
	}
	else
	{
		/* No -> "Member's dead!" */
		return 127;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Can_be_active_member_evaluator
 * FUNCTION  : Check if a party member can be active (member select window evaluator).
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 15:21
 * LAST      : 10.08.95 15:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Message nr / 0 = member is OK
 *              / 0xFFFF = member is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Can_be_active_member_evaluator(MEM_HANDLE Char_handle)
{
	/* Is this party member alive ? */
	if (Get_conditions(Char_handle) & ACTIVE_MASK)
	{
		/* No -> "Member cannot be active!" */
		return 544;
	}
	else
	{
		/* Yes -> Member is OK */
		return 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_party_member
 * FUNCTION  : Remove a party member from the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 15:25
 * LAST      : 30.06.95 15:25
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_party_member(UNSHORT Member_index)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Party_char_handles[Member_index - 1]);

	/* Does this person have a CD bit number ? */
	if (Char->Destination_CD_bit != 0xFFFF)
	{
		/* Yes -> Un-delete NPC */
		Write_bit_array(CD_BIT_ARRAY, (UNLONG) Char->Destination_CD_bit,
		 CLEAR_BIT_ARRAY);
	}

	MEM_Free_pointer(Party_char_handles[Member_index - 1]);

	/* Save character data */
	Save_subfile(Party_char_handles[Member_index - 1], PARTY_CHAR,
	 PARTY_DATA.Member_nrs[Member_index - 1]);

	/* Free memory */
	MEM_Kill_memory(Party_char_handles[Member_index - 1]);
	MEM_Free_memory(Small_portrait_handles[Member_index - 1]);
	MEM_Free_memory(Party_event_set_handles[Member_index - 1][0]);
	MEM_Free_memory(Party_event_set_handles[Member_index - 1][1]);
	MEM_Free_memory(Party_event_text_handles[Member_index - 1][0]);
	MEM_Free_memory(Party_event_text_handles[Member_index - 1][1]);

	/* Remove party member */
	PARTY_DATA.Member_nrs[Member_index - 1] = 0;

	/* Reset the status area */
	Reset_status_area();

	/* Was this the active member ? */
	if (PARTY_DATA.Active_member == Member_index)
	{
		/* Yes -> Select a new active member */
		Select_new_active_member();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_party_member
 * FUNCTION  : Add a party member to the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 15:26
 * LAST      : 22.08.95 11:36
 * INPUTS    : UNSHORT Char_index - Party character index.
 *             UNSHORT NPC_index - NPC index (0...) / 0xFFFF = no NPC.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Add_party_member(UNSHORT Char_index, UNSHORT NPC_index)
{
	struct Character_data *Char;
	MEM_HANDLE Char_handle;
	BOOLEAN Result;
	UNLONG CD_bit_nr;
	SISHORT i;
	UNSHORT New_member_index = 0;
	UNSHORT Occupied_battle_order_slots;
	UNSHORT Batch[2];

	/* Find a free slot in the party */
	for (i=0;i<6;i++)
	{
		if (!PARTY_DATA.Member_nrs[i])
		{
			New_member_index = i + 1;
			break;
		}
	}

	/* Found one ? */
	if (!New_member_index)
	{
		/* No -> Error */
		Error(ERROR_NO_PLACE_IN_PARTY);
		return FALSE;
	}

	/* Load character data */
	Char_handle = Load_subfile(PARTY_CHAR, Char_index);
	if (!Char_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Store character data handle */
	Party_char_handles[New_member_index - 1] = Char_handle;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Load small portrait */
	Small_portrait_handles[New_member_index - 1] =
	 Load_subfile(SMALL_PORTRAIT, (UNSHORT) Char->Portrait_nr);

	if (!Small_portrait_handles[New_member_index - 1])
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Build event set batch */
	Batch[0] = Get_event_set_index(Char_handle, 0);
	Batch[1] = Get_event_set_index(Char_handle, 1);

	/* Load event sets */
	Result = Load_partial_batch(EVENT_SET, 2, Batch,
	 &Party_event_set_handles[New_member_index - 1][0]);

	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load event texts */
	Result = Load_partial_batch(EVENT_TEXT, 2, Batch,
	 &Party_event_text_handles[New_member_index - 1][0]);

	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Was this an NPC ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Does this person already have a CD bit number ? */
		if (Char->Destination_CD_bit == 0xFFFF)
		{
			/* No -> Calculate CD bit number */
			CD_bit_nr = ((PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP) + NPC_index;

			/* Store */
			Char->Destination_CD_bit = CD_bit_nr;
		}
		else
		{
			/* Yes -> Get it */
			CD_bit_nr = Char->Destination_CD_bit;
		}

		/* Delete NPC */
		Write_bit_array(CD_BIT_ARRAY, CD_bit_nr, SET_BIT_ARRAY);
	}

	MEM_Free_pointer(Char_handle);

	/* Build list of occupied battle order slots */
	Occupied_battle_order_slots = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			Occupied_battle_order_slots |= (1 << PARTY_DATA.Battle_order[i]);
		}
	}

	/* Search for a free battle order slot */
	for (i=11;i>=0;i--)
	{
		if (!(Occupied_battle_order_slots & (1 << i)))
		{
			PARTY_DATA.Battle_order[New_member_index - 1] = i;
			break;
		}
	}

	/* Insert character in party */
	PARTY_DATA.Member_nrs[New_member_index - 1] = Char_index;

	/* Reset the status area */
	Reset_status_area();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_party_gold
 * FUNCTION  : Get the amount of gold coins carried by the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 20:05
 * LAST      : 18.08.95 20:05
 * INPUTS    : None.
 * RESULT    : UNLONG : Party gold.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_party_gold(void)
{
	struct Character_data *Char;
	UNLONG Party_gold = 0;
	UNSHORT i;

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

			/* Add member's gold to pool */
			Party_gold += (UNLONG) Char->Char_gold;

			MEM_Free_pointer(Party_char_handles[i]);
		}
	}

	return Party_gold;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_party_gold
 * FUNCTION  : Set the amount of gold coins carried by the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.95 20:07
 * LAST      : 18.08.95 21:31
 * INPUTS    : UNLONG New_party_gold - New party gold.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_party_gold(UNLONG New_party_gold)
{
	struct Character_data *Char;
	SILONG Delta_gold;
	SILONG Delta_gold_per_member;
	UNSHORT Total_members;
	UNSHORT Members_left;
	UNSHORT Fit;
	UNSHORT i;

	/* Count party members */
	Total_members = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Count up */
			Total_members++;
		}
	}

	/* Exit if there are no party members (unlikely) */
	if (!Total_members)
		return;

	/* Determine the gold difference */
	Delta_gold = New_party_gold - Get_party_gold();

	/* Any difference ? */
	if (Delta_gold)
	{
		/* Yes -> Increase or decrease ? */
		if (Delta_gold > 0)
		{
			/* Increase -> Try to give all the gold */
			while (Total_members && Delta_gold > 0)
			{
				/* Check all party members */
				Members_left = Total_members;
				for (i=0;i<6;i++)
				{
					/* Anyone there ? */
					if (Member_present(i + 1))
					{
						/* Yes -> Determine the gold difference per party member */
						Delta_gold_per_member = Delta_gold / Members_left;

						/* Get character data */
						Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

						/* How much gold can this party member take ? */
						Fit = 32767 - Char->Char_gold;

						/* Any ? */
						if (Fit)
						{
							/* Yes -> Give as much gold as possible */
							if (Fit > Delta_gold_per_member)
							{
								Char->Char_gold += (UNSHORT) Delta_gold_per_member;
								Delta_gold -= Delta_gold_per_member;
							}
							else
							{
								Char->Char_gold += Fit;
								Delta_gold -= Fit;
							}

							/* Count down */
							Members_left--;
						}

						MEM_Free_pointer(Party_char_handles[i]);

						/* Exit if all members have been checked */
						/* (must happen in order to avoid divides by zero!) */
						if (!Members_left)
							break;
					}
				}

				/* Was any gold given during this pass ? */
				if (Total_members == Members_left)
				{
					/* No -> Exit */
					/* (this is the unlikely situation where the party has so
					    much gold they can't possible carry it all. Capitalist
					    pigs!)
					 */
					break;
				}
				else
				{
					/* Yes -> Try again, but for less party members */
					Total_members = Members_left;
				}
			}
		}
		else
		{
			/* Decrease -> Try to take all the gold */
			Delta_gold = 0 - Delta_gold;

			while (Total_members && Delta_gold > 0)
			{
				/* Check all party members */
				Members_left = Total_members;
				for (i=0;i<6;i++)
				{
					/* Anyone there ? */
					if (Member_present(i + 1))
					{
						/* Yes -> Determine the gold difference per party member */
						Delta_gold_per_member = Delta_gold / Members_left;

						/* Get character data */
						Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

						/* Can this party member give any gold ? */
						if (Char->Char_gold)
						{
							/* Yes -> Take as much gold as possible */
							if (Char->Char_gold > Delta_gold_per_member)
							{
								Char->Char_gold -= (UNSHORT) Delta_gold_per_member;
								Delta_gold -= Delta_gold_per_member;
							}
							else
							{
								Delta_gold -= Char->Char_gold;
								Char->Char_gold = 0;
							}

							/* Count down */
							Members_left--;
						}

						MEM_Free_pointer(Party_char_handles[i]);

						/* Exit if all members have been checked */
						/* (must happen in order to avoid divides by zero!) */
						if (!Members_left)
							break;
					}
				}

				/* Was any gold taken during this pass ? */
				if (Members_left == Total_members)
				{
					/* No -> Exit */
					/* (this is the unlikely situation where the party has less
					    gold than they have spent. Society is to blame!)
					 */
					break;
				}
				else
				{
					/* Yes -> Try again, but for less party members */
					Total_members = Members_left;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_party_food
 * FUNCTION  : Get the amount of food rations carried by the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.95 11:13
 * LAST      : 22.08.95 11:13
 * INPUTS    : None.
 * RESULT    : UNLONG : Party food.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_party_food(void)
{
	struct Character_data *Char;
	UNLONG Party_food = 0;
	UNSHORT i;

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

			/* Add member's food to pool */
			Party_food += (UNLONG) Char->Char_food;

			MEM_Free_pointer(Party_char_handles[i]);
		}
	}

	return Party_food;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_party_food
 * FUNCTION  : Set the amount of food rations carried by the party.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.95 11:13
 * LAST      : 22.08.95 11:13
 * INPUTS    : UNLONG New_party_food - New party food.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_party_food(UNLONG New_party_food)
{
	struct Character_data *Char;
	SILONG Delta_food;
	SILONG Delta_food_per_member;
	UNSHORT Total_members;
	UNSHORT Members_left;
	UNSHORT Fit;
	UNSHORT i;

	/* Count party members */
	Total_members = Count_alive_party_members();
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Count up */
			Total_members++;
		}
	}

	/* Exit if there are no party members (unlikely) */
	if (!Total_members)
		return;

	/* Determine the food difference */
	Delta_food = New_party_food - Get_party_food();

	/* Any difference ? */
	if (Delta_food)
	{
		/* Yes -> Increase or decrease ? */
		if (Delta_food > 0)
		{
			/* Increase -> Try to give all the food */
			while (Total_members && Delta_food > 0)
			{
				/* Check all party members */
				Members_left = Total_members;
				for (i=0;i<6;i++)
				{
					/* Anyone there ? */
					if (Member_present(i + 1))
					{
						/* Yes -> Determine the food difference per party member */
						Delta_food_per_member = Delta_food / Members_left;

						/* Get character data */
						Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

						/* How much food can this party member take ? */
						Fit = 32767 - Char->Char_food;

						/* Any ? */
						if (Fit)
						{
							/* Yes -> Give as much food as possible */
							if (Fit > Delta_food_per_member)
							{
								Char->Char_food += (UNSHORT) Delta_food_per_member;
								Delta_food -= Delta_food_per_member;
							}
							else
							{
								Char->Char_food += Fit;
								Delta_food -= Fit;
							}

							/* Count down */
							Members_left--;
						}

						MEM_Free_pointer(Party_char_handles[i]);

						/* Exit if all members have been checked */
						/* (must happen in order to avoid divides by zero!) */
						if (!Members_left)
							break;
					}
				}

				/* Was any food given during this pass ? */
				if (Total_members == Members_left)
				{
					/* No -> Exit */
					/* (this is the unlikely situation where the party has so
					    much food they can't possible carry it all. Capitalist
					    pigs!)
					 */
					break;
				}
				else
				{
					/* Yes -> Try again, but for less party members */
					Total_members = Members_left;
				}
			}
		}
		else
		{
			/* Decrease -> Try to take all the food */
			Delta_food = 0 - Delta_food;

			while (Total_members && Delta_food > 0)
			{
				/* Check all party members */
				Members_left = Total_members;
				for (i=0;i<6;i++)
				{
					/* Anyone there ? */
					if (Member_present(i + 1))
					{
						/* Yes -> Determine the food difference per party member */
						Delta_food_per_member = Delta_food / Members_left;

						/* Get character data */
						Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

						/* Can this party member give any food ? */
						if (Char->Char_food)
						{
							/* Yes -> Take as much food as possible */
							if (Char->Char_food > Delta_food_per_member)
							{
								Char->Char_food -= (UNSHORT) Delta_food_per_member;
								Delta_food -= Delta_food_per_member;
							}
							else
							{
								Delta_food -= Char->Char_food;
								Char->Char_food = 0;
							}

							/* Count down */
							Members_left--;
						}

						MEM_Free_pointer(Party_char_handles[i]);

						/* Exit if all members have been checked */
						/* (must happen in order to avoid divides by zero!) */
						if (!Members_left)
							break;
					}
				}

				/* Was any food taken during this pass ? */
				if (Members_left == Total_members)
				{
					/* No -> Exit */
					/* (this is the unlikely situation where the party has less
					    food than they have spent. Society is to blame!)
					 */
					break;
				}
				else
				{
					/* Yes -> Try again, but for less party members */
					Total_members = Members_left;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_party_members
 * FUNCTION  : Count the number of party members.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 14:44
 * LAST      : 01.09.95 14:44
 * INPUTS    : None.
 * RESULT    : UNSHORT : Number of members.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_party_members(void)
{
	UNSHORT Total_members;
	UNSHORT i;

	/* Check party */
	Total_members = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Count up */
			Total_members++;
		}
	}

	return Total_members;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_alive_party_members
 * FUNCTION  : Count the number of party members who are alive.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 14:39
 * LAST      : 01.09.95 14:39
 * INPUTS    : None.
 * RESULT    : UNSHORT : Number of members.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_alive_party_members(void)
{
	UNSHORT Total_members;
	UNSHORT i;

	/* Check party */
	Total_members = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Alive and kicking ? */
			if (Character_alive(Party_char_handles[i]))
			{
				/* Yes -> Count up */
				Total_members++;
			}
		}
	}

	return Total_members;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_party_light
 * FUNCTION  : Calculate the amount of light the party is generating because
 *              of light-giving items they are carrying.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 19:10
 * LAST      : 02.09.95 19:10
 * INPUTS    : None.
 * RESULT    : UNSHORT : Amount of light (0...100).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Calculate_party_light(void)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNLONG Total_light;
	UNSHORT Max_light;
	UNSHORT Nr_lights;
	UNSHORT Light;
	UNSHORT Party_light;
	UNSHORT Mean_light;
	UNSHORT i, j;

	/* Clear */
	Total_light		= 0;
	Max_light		= 0;
	Nr_lights		= 0;
	Party_light		= 0;

	/* Check the party */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

			/* Check all body items */
			for (j=0;j<ITEMS_ON_BODY;j++)
			{
				/* Anything there ? */
				if (!Packet_empty(&(Char->Body_items[j])))
				{
					/* Yes -> Get item data */
					Item_data = Get_item_data(&(Char->Body_items[j]));

					/* Is a LIGHT item ? */
					if (Item_data->Type == LIGHT_IT)
					{
						/* Yes -> Get light */
						Light += (UNSHORT) Item_data->Misc[0];

						/* Higher than current maximum ? */
						if (Light > Max_light)
						{
							/* Yes -> Set new maximum */
							Max_light = Light;
						}

						/* Add to total light */
						Total_light += (UNLONG) Light;

						/* Count up */
						Nr_lights++;
					}
					Free_item_data();
				}
			}

			/* Check all backpack items */
			for (j=0;j<ITEMS_PER_CHAR;j++)
			{
				/* Anything there ? */
				if (!Packet_empty(&(Char->Backpack_items[j])))
				{
					/* Yes -> Get item data */
					Item_data = Get_item_data(&(Char->Backpack_items[j]));

					/* Is a LIGHT item ? */
					if (Item_data->Type == LIGHT_IT)
					{
						/* Yes -> Get light */
						Light += (UNSHORT) Item_data->Misc[0];

						/* Higher than current maximum ? */
						if (Light > Max_light)
						{
							/* Yes -> Set new maximum */
							Max_light = Light;
						}

						/* Add to total light */
						Total_light += (UNLONG) Light;

						/* Count up */
						Nr_lights++;
					}
					Free_item_data();
				}
			}
			MEM_Free_pointer(Party_char_handles[i]);
		}
	}

	/* Any lights ? */
	if (Nr_lights)
	{
		/* Yes -> Calculate mean light */
		Mean_light = (UNSHORT)(Total_light / (UNLONG) Nr_lights);

		/* Calculate party light */
		Party_light = min(100, Max_light + (Mean_light / 2));
	}

	return Party_light;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_party_light
 * FUNCTION  : Update the light-giving items the party is carrying.
 * FILE      : PRTLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.95 19:10
 * LAST      : 02.09.95 19:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_party_light(void)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i, j;

	/* Check the party */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

			/* Check all body items */
			for (j=0;j<ITEMS_ON_BODY;j++)
			{
				/* Anything there ? */
				if (!Packet_empty(&(Char->Body_items[j])))
				{
					/* Yes -> Get item data */
					Item_data = Get_item_data(&(Char->Body_items[j]));

					/* Is a LIGHT item ? */
					if (Item_data->Type == LIGHT_IT)
					{
						/* Yes -> Infinite charges ? */
						if (Char->Body_items[j].Charges < 255)
						{
							/* No -> Count down */
							Char->Body_items[j].Charges--;

							/* All gone ? */
							if (!Char->Body_items[j].Charges)
							{
								/* Yes -> Destroy item */
								Remove_item(Party_char_handles[i], j, 1);
							}
						}
					}
					Free_item_data();
				}
			}

			/* Check all backpack items */
			for (j=0;j<ITEMS_PER_CHAR;j++)
			{
				/* Anything there ? */
				if (!Packet_empty(&(Char->Backpack_items[j])))
				{
					/* Yes -> Get item data */
					Item_data = Get_item_data(&(Char->Backpack_items[j]));

					/* Is a LIGHT item ? */
					if (Item_data->Type == LIGHT_IT)
					{
						/* Yes -> Infinite charges ? */
						if (Char->Backpack_items[j].Charges < 255)
						{
							/* No -> Count down */
							Char->Backpack_items[j].Charges--;

							/* All gone ? */
							if (!Char->Backpack_items[j].Charges)
							{
								/* Yes -> Destroy item */
								Remove_item(Party_char_handles[i],
								 ITEMS_ON_BODY + j, 1);
							}
						}
					}
					Free_item_data();
				}
			}
			MEM_Free_pointer(Party_char_handles[i]);
		}
	}
}

