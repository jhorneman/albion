/************
 * NAME     : EVENTS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : EVENTS.H
 ************/

/* includes */

#include <stdlib.h>

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
#include <MUSIC.H>
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
#include <PLACES.H>

/* prototypes */

void Map_Exit_event(void);
void Do_map_exit(UNSHORT New_X, UNSHORT New_Y, UNSHORT New_VD,
 UNSHORT New_map_nr);

void Door_event(void);

void Chest_event(void);

void Text_event(void);

void Spinner_event(void);

void Trap_event(void);
void Do_trap(UNSHORT Member_nr, struct Event_block *Event);

void Change_Icon_event(void);

void Encounter_event(void);

void Place_action_event(void);

void Signal_event(void);

void Clone_Automap_event(void);

void Sound_event(void);

void Start_Dialogue_event(void);

void Create_Trans_event(void);

void Execute_event(void);

void Remove_Member_event(void);

void End_Dialogue_event(void);

void Wipe_event(void);

void Play_Animation_event(void);

void Offset_event(void);

void Pause_event(void);

void Single_Item_Chest_event(void);

void Ask_Surrender_event(void);

void Do_Script_event(void);

/* global variables */

UNSHORT Shortcut_map_X = 36;
UNSHORT Shortcut_map_Y = 12;

/* Event handler list */
Event_handler Event_handlers[MAX_EVENT_TYPES] = {
	Map_Exit_event,
	Door_event,
	Chest_event,
	Text_event,
	Spinner_event,
	Trap_event,
	Change_Used_Item_event,
	Datachange_event,
	Change_Icon_event,
	Encounter_event,
	Place_action_event,
	Query_event,
	Modify_event,
	NULL,
	Signal_event,
	Clone_Automap_event,
	Sound_event,
	Start_Dialogue_event,
	Create_Trans_event,
	Execute_event,
	Remove_Member_event,
	End_Dialogue_event,
	Wipe_event,
	Play_Animation_event,
	Offset_event,
	Pause_event,
	Single_Item_Chest_event,
	Ask_Surrender_event,
	Do_Script_event
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Map_Exit_event
 * FUNCTION  : Map Exit event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 10:06
 * LAST      : 15.06.95 14:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Map_Exit_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	UNSHORT New_X, New_Y;
	UNSHORT New_VD;
	UNSHORT New_map_nr;
	UNSHORT Old_map_nr;
	UNSHORT MX_type;

	/* Get current map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get map exit type */
	MX_type = (UNSHORT) Event->Byte_5;

	/* Relative X-coordinate ? */
	if ((MX_type == NORMAL_X_REL_MX_TYPE) ||
	 (MX_type == NORMAL_XY_REL_MX_TYPE))
	{
		/* Yes -> Get new X-coordinate */
		New_X = PARTY_DATA.X + (SISHORT) ((SIBYTE) Event->Byte_1);
	}
	else
	{
		/* No -> Get new X-coordinate */
		New_X = (UNSHORT) Event->Byte_1;

		/* Keep current X-coordinate ? */
		if (!New_X)
		{
			/* Yes */
			New_X = PARTY_DATA.X;
		}
	}

	/* Relative Y-coordinate ? */
	if ((MX_type == NORMAL_Y_REL_MX_TYPE) ||
	 (MX_type == NORMAL_XY_REL_MX_TYPE))
	{
		/* Yes -> Get new Y-coordinate */
		New_Y = PARTY_DATA.Y + (SISHORT) ((SIBYTE) Event->Byte_1);
	}
	else
	{
		/* No -> Get new Y-coordinate */
		New_Y = (UNSHORT) Event->Byte_2;

		/* Keep current Y-coordinate ? */
		if (!New_Y)
		{
			/* Yes */
			New_Y = PARTY_DATA.Y;
		}
	}

	/* Get map exit parameters */
	New_VD = (UNSHORT) Event->Byte_3;
	New_map_nr = Event->Word_6;

	/* Keep current view direction ? */
	if (New_VD == 0xFF)
	{
		/* Yes */
		New_VD = PARTY_DATA.View_direction;
	}

	/* What kind of map exit ? */
	switch (MX_type)
	{
		/* Normal map exit */
		case NORMAL_MX_TYPE:
		case NORMAL_X_REL_MX_TYPE:
		case NORMAL_Y_REL_MX_TYPE:
		case NORMAL_XY_REL_MX_TYPE:
		{
			/* Do map exit */
			Do_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Teleporter */
		case TELEPORTER_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = TELEPORT_MX_MODE;

			/* Do map exit */
			Do_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Trapdoor up */
		case TRAPDOOR_UP_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = TRAPDOOR_UP_MX_MODE;

			/* Do map exit */
			Do_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Jump */
		case JUMP_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = JUMP_MX_MODE;

			/* Do map exit */
			Do_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* End-sequence */
		case END_SEQUENCE_MX_TYPE:
		{
			break;
		}
		/* Trapdoor down */
		case TRAPDOOR_DOWN_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = TRAPDOOR_DOWN_MX_MODE;

			/* Do map exit */
			Do_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Fall down mode */
		case FALL_DOWN_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = FALL_DOWN_MX_MODE;

			/* Do map exit */
			Do_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Chapter screen */
		case CHAPTER_SCREEN_MX_TYPE:
		{
			// NOT IMPLEMENTED
			break;
		}
	}

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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_map_exit
 * FUNCTION  : Do a map exit.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.06.95 14:52
 * LAST      : 01.09.95 11:26
 * INPUTS    : UNSHORT New_X - New X-coordinate.
 *             UNSHORT New_Y - New Y-coordinate.
 *             UNSHORT New_VD - New view direction.
 *             UNSHORT New_map_nr - New map number / 0 = no change.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_map_exit(UNSHORT New_X, UNSHORT New_Y, UNSHORT New_VD,
 UNSHORT New_map_nr)
{
	BOOLEAN Result;

	/* Different map ? */
	if (New_map_nr)
	{
		/* Yes -> Are we leaving the shortcut map ? */
		if (PARTY_DATA.Map_nr == SHORTCUT_MAP_NR)
		{
			/* Yes -> Store previous coordinates */
			Shortcut_map_X = Old_position.Map_X;
			Shortcut_map_Y = Old_position.Map_Y;
		}

		/* Are we in a dialogue ? */
		if (In_Dialogue)
		{
			/* Yes -> Exit */
			Exit_display();
			Pop_module();
		}

		/* Exit current map */
		Exit_map();

		/* Jump or teleport ? */
		if ((Special_map_exit_mode != TELEPORT_MX_MODE) &&
		 (Special_map_exit_mode != JUMP_MX_MODE))
		{
			/* No -> Display slab */
			OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
			Switch_screens();
		}

		/* Set new position */
		PARTY_DATA.X					= New_X;
		PARTY_DATA.Y					= New_Y;
		PARTY_DATA.View_direction	= New_VD;
		PARTY_DATA.Map_nr				= New_map_nr;

		/* Initialize new map */
		Result = Init_map();
		if (!Result)
		{
			Exit_program();
		}
	}
	else
	{
		/* No -> Jump or teleport ? */
		if ((Special_map_exit_mode != TELEPORT_MX_MODE) &&
		 (Special_map_exit_mode != JUMP_MX_MODE))
		{
			/* No -> Display slab */
			OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
			Switch_screens();
		}

		/* Change position */
		Change_position(New_X, New_Y, New_VD);

		/* Execute normal map events */
		Execute_normal_map_events();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Door_event
 * FUNCTION  : Door event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.01.95 11:01
 * LAST      : 11.07.95 13:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Door_event(void)
{
	struct Event_block *Event;
	UNSHORT Door_status;

	/* Clear success flag */
	Clear_success_flag();

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Enter Door screen */
	Door_status = Do_Door((UNSHORT) Event->Byte_1,
	 (UNSHORT) Event->Byte_2,
	 (UNSHORT) Event->Byte_3,
	 (UNSHORT) Event->Byte_4,
	 (UNSHORT) Event->Byte_5,
	 Event->Word_6,
	 (BOOLEAN) (Event->Word_8 != 0xFFFF));

	/* Act depending on door status */
	switch(Door_status)
	{
		/* Player left the door screen */
		case EXIT_DOOR_STATUS:
		{
			break;
		}
		/* Player activated a trap */
		case TRAP_DOOR_STATUS:
		{
			/* Follow negative chain */
			Event->Next_event_nr = Event->Word_8;

			break;
		}
		/* Player opened the door */
		case OPENED_DOOR_STATUS:
		{
			/* Yay! Set success flag */
			Set_success_flag();

			/* Any bit given ? */
			if (Event->Word_6 != 0xFFFF)
			{
				/* Yes -> Save door in bit array */
				Write_bit_array(DOOR_BIT_ARRAY, (UNLONG) Event->Word_6,
				 SET_BIT_ARRAY);
			}

			break;
		}
		/* The door was already open */
		case IS_OPEN_DOOR_STATUS:
		{
			/* Yay! Set success flag */
			Set_success_flag();

			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Chest_event
 * FUNCTION  : Chest event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.07.95 18:47
 * LAST      : 12.07.95 12:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Chest_event(void)
{
	struct Event_block *Event;
	UNSHORT Chest_status;

	/* Clear success flag */
	Clear_success_flag();

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Enter Chest screen */
	Chest_status = Do_Chest((UNSHORT) Event->Byte_1,
	 (UNSHORT) Event->Byte_2,
	 (UNSHORT) Event->Byte_3,
	 (UNSHORT) Event->Byte_4,
	 (UNSHORT) Event->Byte_5,
	 Event->Word_6,
	 (BOOLEAN) (Event->Word_8 != 0xFFFF));

	/* Act depending on chest status */
	switch(Chest_status)
	{
		/* Player left the closed chest screen */
		case EXIT_CLOSED_CHEST_STATUS:
		{
			break;
		}
		/* Player activated a trap */
		case TRAP_CHEST_STATUS:
		{
			/* Follow negative chain */
			Event->Next_event_nr = Event->Word_8;

			break;
		}
		/* Player opened the chest */
		case OPENED_CHEST_STATUS:
		{
			/* Yay! Set success flag */
			Set_success_flag();

			/* Save chest in bit array */
			Write_bit_array(CHEST_BIT_ARRAY, (UNLONG) Event->Word_6,
			 SET_BIT_ARRAY);

			break;
		}
		/* Player left the open chest screen */
		case EXIT_OPEN_CHEST_STATUS:
		{
			break;
		}
		/* Player emptied a junkpile */
		case JUNKPILE_EMPTY_STATUS:
		{
			/* Yay! Set success flag */
			Set_success_flag();

			/* Save ? */
			if (!(Event->Byte_3 & 0x02))
			{
				/* Yes -> Save event */
				Save_current_event();
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Text_event
 * FUNCTION  : Text event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.02.95 17:52
 * LAST      : 28.07.95 13:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Text_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	MEM_HANDLE Text_handle;
	UNSHORT Char_nr;
	UNSHORT Block_nr;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

	/* Get text parameters */
	Char_nr = (UNSHORT) Event->Byte_4;
	Block_nr = (UNSHORT) Event->Byte_5;

	/* Get memory handle of text file */
	Text_handle = Context->Text_handle;

	/* What kind of text ? */
	switch (Event->Byte_1)
	{
		/* Normal text */
		case NORMAL_TEXT_TYPE:
		{
			/* Set ink colour */
			Set_ink(SILVER_TEXT);

			/* In a dialogue ? */
			if (In_Dialogue)
			{
				/* Yes -> Display text in dialogue window */
				Display_text_in_dialogue(Text_handle, Block_nr);
			}
			else
			{
				/* No -> Do normal text window */
				Do_text_file_window(Text_handle, Block_nr);
			}

			break;
		}
		/* Visual text */
		case VISUAL_TEXT_TYPE:
		{
			/* Set ink colour */
			Set_ink(SILVER_TEXT);

			/* Can the party see ? */
			if (Party_can_see())
			{
				/* Yes -> Do text window */
				Do_text_file_window(Text_handle, Block_nr);
			}
			break;
		}
		/* Text spoken by a party member */
		case PARTY_SPOKEN_TEXT_TYPE:
		{
			UNSHORT Member_index;

			/* Set ink colour */
			Set_ink(GOLD_TEXT);

			/* Find character in party */
			Member_index = Find_character_in_party(Char_nr);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Is this character capable ? */
				if (Character_capable(Party_char_handles[Member_index - 1]))
				{
					/* Yes -> Do text window */
					Do_text_file_window_with_symbol
					(
						Text_handle,
						Block_nr,
						PORTRAIT_WIDTH,
						PORTRAIT_HEIGHT,
						Small_portrait_handles[Member_index - 1],
						0
					);
				}
			}
			break;
		}
		/* Text spoken by an NPC */
		case NPC_SPOKEN_TEXT_TYPE:
		{
			struct Character_data *Char;
			MEM_HANDLE NPC_char_handle;
			MEM_HANDLE NPC_portrait_handle;

			/* Set ink colour */
			Set_ink(GOLD_TEXT);

			/* Load NPC character data */
			NPC_char_handle = Load_subfile(NPC_CHAR, Char_nr);
			if (!NPC_char_handle)
			{
				Error(ERROR_FILE_LOAD);
				break;
			}

			/* Get pointer to character data */
			Char = (struct Character_data *) MEM_Claim_pointer(NPC_char_handle);

			/* Load portrait */
			NPC_portrait_handle = Load_subfile(SMALL_PORTRAIT,
			 (UNSHORT) Char->Portrait_nr);

			MEM_Free_pointer(NPC_char_handle);
			MEM_Free_memory(NPC_char_handle);

			if (!NPC_portrait_handle)
			{
				Error(ERROR_FILE_LOAD);
				break;
			}

			/* Do text window */
			Do_text_file_window_with_symbol
			(
				Text_handle,
				Block_nr,
				PORTRAIT_WIDTH,
				PORTRAIT_HEIGHT,
				NPC_portrait_handle,
				0
			);

			/* Destroy portrait */
			MEM_Free_memory(NPC_portrait_handle);

			break;
		}
		/* Multiple-choice text */
		case MULTI_CHOICE_TEXT_TYPE:
		{
			struct Dialogue_node New_node;

			/* Build new dialogue node */
			New_node.Text_handle		= Text_handle;
			New_node.Text_block_nr	= Block_nr;
			New_node.Flags				= 0;

			/* Set current dialogue node */
			Set_current_dialogue_node(&New_node);

			break;
		}
		/* Place title text */
		case PLACE_TITLE_TEXT_TYPE:
		{
			// NOT IMPLEMENTED
			break;
		}
		/* Text for the permanent text window */
		case PERMANENT_TEXT_TYPE:
		{
			UNBYTE *Ptr;

			/* Get text file address */
			Ptr = MEM_Claim_pointer(Text_handle);

			/* Find text block */
			Ptr = Find_text_block(Ptr, Block_nr);

			/* Print text */
			Set_permanent_text(Ptr);

			MEM_Free_pointer(Text_handle);

			break;
		}
		/* Text displayed in combat */
		case COMBAT_TEXT_TYPE:
		{
			/* Do text window */
			Do_text_file_window(Text_handle, Block_nr);

			break;
		}
		/* Text displayed in combat, but only in slow mode */
		case SLOW_COMBAT_TEXT_TYPE:
		{
			/* Do text window */
			Do_text_file_window(Text_handle, Block_nr);

			break;
		}
		/* Multiple-choice text, without standard options */
		case MULTI_CHOICE_NO_STANDARD_TEXT_TYPE:
		{
			struct Dialogue_node New_node;

			/* Build new dialogue node */
			New_node.Text_handle		= Text_handle;
			New_node.Text_block_nr	= Block_nr;
			New_node.Flags				= DIALOGUE_NO_DEFAULT;

			/* Set current dialogue node */
			Set_current_dialogue_node(&New_node);

			break;
		}
		/* Text spoken by the active party member */
		case ACTIVE_SPOKEN_TEXT_TYPE:
		{
			/* Set ink colour */
			Set_ink(GOLD_TEXT);

			/* Is the active party member capable ? */
			if (Character_capable(Active_char_handle))
			{
				/* Yes -> Do text window */
				Do_text_file_window_with_symbol
				(
					Text_handle,
					Block_nr,
					PORTRAIT_WIDTH,
					PORTRAIT_HEIGHT,
					Small_portrait_handles[PARTY_DATA.Active_member - 1],
					0
				);
			}
			break;
		}
		/* Multiple-choice default text */
		case MULTI_CHOICE_DEFAULT_TEXT_TYPE:
		{
			struct Dialogue_node New_node;

			/* Erase default dialogue node ? */
			if (Block_nr == 0xFF)
			{
				/* Yes */
				New_node.Text_handle	= NULL;
			}
			else
			{
				/* No */
				New_node.Text_handle	= Text_handle;
			}

			/* Build new dialogue node */
			New_node.Text_block_nr	= Block_nr;
			New_node.Flags				= 0;

			/* Set default dialogue node */
			Set_default_dialogue_node(&New_node);

			break;
		}
		/* Multiple-choice default text, without standard options */
		case MULTI_CHOICE_DEFAULT_NO_STANDARD_TEXT_TYPE:
		{
			struct Dialogue_node New_node;

			/* Erase default dialogue node ? */
			if (Block_nr == 0xFF)
			{
				/* Yes */
				New_node.Text_handle	= NULL;
			}
			else
			{
				/* No */
				New_node.Text_handle	= Text_handle;
			}

			/* Build new dialogue node */
			New_node.Text_block_nr	= Block_nr;
			New_node.Flags				= DIALOGUE_NO_DEFAULT;

			/* Set default dialogue node */
			Set_default_dialogue_node(&New_node);

			break;
		}
		/* Replace the default multiple-choice options */
		case REPLACE_DEFAULT_MULTI_CHOICE_TYPE:
		{
			/* Go back to normal ? */
			if (Block_nr == 0xFF)
			{
				/* Yes */
				Set_standard_MC_entry_text_block(NULL, 0);
			}
			else
			{
				/* No */
				Set_standard_MC_entry_text_block(Text_handle, Block_nr);
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Spinner_event
 * FUNCTION  : Spinner event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Spinner_event(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Trap_event
 * FUNCTION  : Trap event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:21
 * LAST      : 21.04.95 11:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Trap_event(void)
{
	struct Event_block *Event;

	/* Clear success flag */
	Clear_success_flag();

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Whose data is changed ? */
	switch (Event->Byte_3)
	{
		/* Active party member */
		case 0:
		{
			/* Trap active member */
			Do_trap(PARTY_DATA.Active_member, Event);

			break;
		}
		/* Entire party */
		case 1:
		{
			UNSHORT i;

			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (Member_present(i + 1))
				{
					/* Yes -> Trap party member */
					Do_trap(i + 1, Event);
				}
			}
			break;
		}
		/* A single party member */
		case 2:
		{
			UNSHORT Member_index;

			/* Find character in party */
			Member_index = Find_character_in_party((UNSHORT) Event->Byte_5);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
			 	/* Yes -> Trap party member */
				Do_trap(Member_index, Event);
			}
			break;
		}
	}

	/* Check active member */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_trap
 * FUNCTION  : Actually execute a trap.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:21
 * LAST      : 21.04.95 11:21
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 *             struct Event_block *Event - Pointer to current event block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes the party member index is legal.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_trap(UNSHORT Member_nr, struct Event_block *Event)
{
	MEM_HANDLE Char_handle;

	/* Get character handle */
	Char_handle = Party_char_handles[Member_nr - 1];

	/* Does this character have the right sex / is alive ? */
	if ((Event->Byte_2 & (1 << Get_sex(Char_handle))) &&
	 (!(Get_conditions(Char_handle) & DEAD_MASK)))
	{
		/* Yes -> Indicate success of trap event */
	  	Set_success_flag();

		/* Lucky ? */
		if (Probe_attribute(Char_handle, LUCK))
		{
			/* Yes -> Tell the player */

		}
		else
		{
			/* No -> Condition trap ? */
			if (Event->Byte_1 < MAX_CONDITIONS)
			{
				/* Yes -> Set condition */
				Set_condition(Char_handle, (UNSHORT) Event->Byte_1);
			}

			/* Damage trap ? */
			if (Event->Word_6)
			{
				/* Yes -> Do damage */
				Do_damage(Member_nr, Get_rnd_50_100(Event->Word_6));
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_Icon_event
 * FUNCTION  : Change Icon event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 17:06
 * LAST      : 03.09.95 19:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_Icon_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	SISHORT X, Y;
	UNSHORT Type;
	UNSHORT Sub_type;
	UNSHORT Value;
	UNSHORT Map_nr;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

	/* Get parameters */
	Type		= (UNSHORT) Event->Byte_4;
	Sub_type	= (UNSHORT) Event->Byte_5;
	Value		= Event->Word_6;
	Map_nr	= Event->Word_8;

	/* Is change of event entry ? */
	if (Type == EVENT_ENTRY_CHANGE)
	{
		/* Yes -> Turn chain number into first block number */
		Value = Get_event_first_block_nr(Value);
	}

	/* Relative coordinates / not an NPC change ? */
	if ((Event->Byte_3 & 0x01) &&
	 (Type != NPC_MOVE_CHANGE) &&
	 (Type != NPC_GFX_CHANGE))
	{
		/* Yes -> Get relative coordinates */
		X = (SISHORT) ((SIBYTE) Event->Byte_1);
		Y = (SISHORT) ((SIBYTE) Event->Byte_2);

		/* Is a map context ? */
		if (Context->Source == MAP_EVENT_SOURCE)
		{
			/* Yes -> Relative to event coordinates */
			X += Context->Source_data.Map_source.X;
			Y += Context->Source_data.Map_source.Y;
		}
		else
		{
			/* No -> Relative to party coordinates */
			X += PARTY_DATA.X;
			Y += PARTY_DATA.Y;
		}
	}
	else
	{
		/* No -> Get absolute coordinates */
		X = (SISHORT) Event->Byte_1;
		Y = (SISHORT) Event->Byte_2;
	}

	/* Use current map ? */
	if (!Map_nr)
	{
		/* Yes -> Load number */
		Map_nr = PARTY_DATA.Map_nr;
	}

	/* Save icon change ? */
	if (Event->Byte_3 & 0x02)
	{
		/* No -> Save in temporary list */
		Add_temporary_modification(X, Y, Type, Sub_type, Value, Map_nr);
	}
	else
	{
		/* Yes -> Save in permanent list */
		Add_permanent_modification(X, Y, Type, Sub_type, Value, Map_nr);
	}

	/* In the current map ? */
	if (Map_nr == PARTY_DATA.Map_nr)
	{
		/* Yes -> Change icon */
		Do_change_icon(X, Y, Type, Sub_type, Value);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Encounter_event
 * FUNCTION  : Encounter event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 13:02
 * LAST      : 26.07.95 11:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Encounter_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	UNSHORT Monster_group_nr;
	UNSHORT Combat_background_nr;

	/* In combat ? */
	if (In_Combat)
	{
		/* Yes -> Error */
		Error(ERROR_ILLEGAL_COMBAT_RECURSION);
		return;
	}

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get parameters */
	Monster_group_nr = Event->Word_6;
	Combat_background_nr = Event->Word_8;

	/* Any combat background given ? */
	if (!Combat_background_nr)
	{
		/* No -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

		/* Is a map context ? */
		if (Context->Source == MAP_EVENT_SOURCE)
		{
			/* Yes -> Get combat background number */
			Combat_background_nr = Get_combat_background_nr();
		}
		else
		{
			/* No -> Oops! */
			Combat_background_nr = 1;
		}
	}

	/* Enter combat screen */
	Enter_Combat(Monster_group_nr, Combat_background_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Place_Action_event
 * FUNCTION  : Place Action event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.95 21:37
 * LAST      : 16.08.95 21:37
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Place_action_event(void)
{
	/* Do place action */
	Do_place_action();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Signal_event
 * FUNCTION  : Signal event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 10:46
 * LAST      : 25.04.95 11:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Signal_event(void)
{
	static struct Event_action Signal_action =
	{
		0,	0, 0,
		SIGNAL_ACTION, 0, 0,
		NULL, NULL, NULL
	};
	struct Event_context *Context;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Set default action data */
	Signal_action.Actor_type = NO_ACTOR_TYPE;
	Signal_action.Actor_index = 0;

	/* Current context is an event set ? */
	if (Context->Source == SET_EVENT_SOURCE)
	{
		/* Yes -> Copy actor data */
		switch (Context->Source_data.Set_source.Char_type)
		{
			/* Party character */
			case PARTY_CHAR_TYPE:
			{
				/* Talking to this character ? */

				{
					/* No -> */
					Signal_action.Actor_type = PARTY_ACTOR_TYPE;
					Signal_action.Actor_index =
					 Context->Source_data.Set_source.Char_index;
				}
				break;
			}
			/* NPC character */
			case NPC_CHAR_TYPE:
			{
				Signal_action.Actor_type = DIALOGUE_ACTOR_TYPE;
				Signal_action.Actor_index = Context->Source_data.Set_source.Char_index;
				break;
			}
			/* Monster character */
			case MONSTER_CHAR_TYPE:
			{
				Signal_action.Actor_type = MONSTER_ACTOR_TYPE;
				Signal_action.Actor_index = Context->Source_data.Set_source.Char_index;
				break;
			}
		}
	}

	/* Set signal number */
	Signal_action.Action_value = (UNSHORT) Context->Data.Byte_1;

	/* Signal */
	Perform_action(&Signal_action);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clone_Automap_event
 * FUNCTION  : Clone Automap event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 13:21
 * LAST      : 13.07.95 16:03
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clone_Automap_event(void)
{
	struct Event_block *Event;

	MEM_HANDLE Source_map_handle;
	MEM_HANDLE Target_map_handle;

	MEM_HANDLE Source_automap_handle;

	struct Map_data *Source_map;
	struct Map_data *Target_map;

	struct Goto_point *Source_Goto_points;
	struct Goto_point *Target_Goto_points;

	UNSHORT Source_map_nr;
	UNSHORT Target_map_nr;

	UNSHORT Source_nr_Goto_points;
	UNSHORT Target_nr_Goto_points;

	UNSHORT *WPtr;
	UNSHORT i, j;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get parameters */
	Source_map_nr = Event->Word_6;
	Target_map_nr = Event->Word_8;

	/* Legal clone parameters ? */
	if (Source_map_nr == Target_map_nr)
	{
		/* No -> Error! */
		Error(ERROR_CLONE_SAME_MAPS);
		return;
	}

	/* Load source map */
	Source_map_handle = Load_subfile(MAP_DATA, Source_map_nr);
	if (!Source_map_handle)
	{
		Error(ERROR_FILE_LOAD);
		return;
	}

	/* Load target map */
	Target_map_handle = Load_subfile(MAP_DATA, Target_map_nr);
	if (!Target_map_handle)
	{
		/* Error -> Destroy source map memory */
		MEM_Free_memory(Source_map_handle);

		/* Error! */
		Error(ERROR_FILE_LOAD);
		return;
	}

	/* Get source and target map data */
	Source_map = (struct Map_data *) MEM_Claim_pointer(Source_map_handle);
	Target_map = (struct Map_data *) MEM_Claim_pointer(Target_map_handle);

	/* Do both maps have the same size ? */
	if ((Source_map->Width != Target_map->Width) ||
	 (Source_map->Height != Target_map->Height))
	{
		/* No -> Destroy map memory */
		MEM_Free_memory(Source_map_handle);
		MEM_Free_memory(Target_map_handle);

		/* Error! */
		Error(ERROR_CLONE_MAPS_NOT_SAME_SIZE);
		return;
	}

	/* Are both maps of the same type ? */
	if (Source_map->Display_type != Target_map->Display_type)
	{
		/* No -> Destroy map memory */
		MEM_Free_memory(Source_map_handle);
		MEM_Free_memory(Target_map_handle);

		/* Error! */
		Error(ERROR_CLONE_MAPS_NOT_SAME_TYPE);
		return;
	}

	/* Are both 3D maps ? */
	if (Source_map->Display_type == 1)
	{
		/* Yes -> Load source automap */
		Source_automap_handle = Load_subfile(AUTOMAP, Source_map_nr);
		if (!Source_automap_handle)
		{
			/* Error -> Destroy map memory */
			MEM_Free_memory(Source_map_handle);
			MEM_Free_memory(Target_map_handle);

			/* Error! */
			Error(ERROR_FILE_LOAD);
			return;
		}

		/* Save over target automap */
		Save_subfile(Source_automap_handle, AUTOMAP, Target_map_nr);

		/* Destroy automap memory */
		MEM_Free_memory(Source_automap_handle);

		/* Find goto-point data in source map */
		WPtr = Find_Goto_point_data_in_map(Source_map);
		Source_nr_Goto_points = *WPtr++;
		Source_Goto_points = (struct Goto_point *) WPtr;

		WPtr = Find_Goto_point_data_in_map(Target_map);
		Target_nr_Goto_points = *WPtr++;
		Target_Goto_points = (struct Goto_point *) WPtr;

		/* Scan all source goto-points */
		for (i=0;i<Source_nr_Goto_points;i++)
		{
			/* Scan all target goto-points */
			for (j=0;j<Target_nr_Goto_points;j++)
			{
				/* Is at same location as source goto-point ? */
				if ((Source_Goto_points[i].X == Target_Goto_points[j].X) &&
					(Source_Goto_points[i].Y == Target_Goto_points[j].Y))
				{
					/* Yes -> Is the source goto-point known ? */
					if (Read_bit_array(GOTO_POINTS_BIT_ARRAY,
					 Source_Goto_points[i].Bit_nr))
					{
						/* Yes -> Make target goto-point known as well */
						Write_bit_array(GOTO_POINTS_BIT_ARRAY,
						 Target_Goto_points[j].Bit_nr, SET_BIT_ARRAY);
					}
					else
					{
						/* No -> Make target goto-point unknown as well */
						Write_bit_array(GOTO_POINTS_BIT_ARRAY,
						 Target_Goto_points[j].Bit_nr, CLEAR_BIT_ARRAY);
					}

					/* Stop scanning target goto-points */
					break;
				}
			}
		}
	}

	/* Clone event save bits */
	for (i=0;i<EVENTS_PER_MAP;i++)
	{
		/* Copy bit from the event save bit array */
		if (Read_bit_array(EVENT_SAVE_BIT_ARRAY,
		 (Source_map_nr * EVENTS_PER_MAP) + i))
		{
			Write_bit_array(EVENT_SAVE_BIT_ARRAY,
			 (Target_map_nr * EVENTS_PER_MAP) + i, SET_BIT_ARRAY);
		}
		else
		{
			Write_bit_array(EVENT_SAVE_BIT_ARRAY,
			 (Target_map_nr * EVENTS_PER_MAP) + i, CLEAR_BIT_ARRAY);
		}
	}

	/* Clone character deleted bits */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Copy bit from the character deleted bit array */
		if (Read_bit_array(CD_BIT_ARRAY, (Source_map_nr * NPCS_PER_MAP) + i))
		{
			Write_bit_array(CD_BIT_ARRAY, (Target_map_nr * NPCS_PER_MAP) + i,
			 SET_BIT_ARRAY);
		}
		else
		{
			Write_bit_array(CD_BIT_ARRAY, (Target_map_nr * NPCS_PER_MAP) + i,
			 CLEAR_BIT_ARRAY);
		}
	}

	/* Clone modifications */
	Clone_modifications(Source_map_nr, Target_map_nr);

	/* Destroy map memory */
	MEM_Free_memory(Source_map_handle);
	MEM_Free_memory(Target_map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sound_event
 * FUNCTION  : Sound event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.02.95 12:17
 * LAST      : 25.07.95 16:03
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Sound_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	UNSHORT Number;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

	/* Get sound parameter */
	Number = (UNSHORT) Event->Byte_2;

	/* What kind of sound ? */
	switch (Event->Byte_1)
	{
		/* Change main song */
		case MAIN_SONG_SOUND_TYPE:
		{
			/* Use map music ? */
			if (Number == 0xFFFF)
			{
				/* Yes */
				Number = Current_map_music_nr;
			}

			/* Change main song */
			Play_song(Number);

			break;
		}
		/* Play sound effect */
		case EFFECT_SOUND_TYPE:
		{
			/* Play sound effect */
			Play_sound_effect(Number, (UNSHORT) Event->Byte_3,
			 (UNSHORT) Event->Byte_4, (UNSHORT) Event->Byte_5, Event->Word_6);

			break;
		}
		/* Change ambient song */
		case AMBIENT_SONG_SOUND_TYPE:
		{
			/* Change ambient song */
			Play_ambient_song(Number);

			break;
		}
		/* Play jingle */
		case JINGLE_SOUND_TYPE:
		{
			/* Play jingle */
			Play_jingle(Number);

			break;
		}
		/* Install located sound-effect */
		case LOCATED_EFFECT_SOUND_TYPE:
		{
			SILONG X, Y;

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Use event coordinates */
				X = (Map_square_size / 2) + Map_square_size *
				 (SILONG) Context->Source_data.Map_source.X;
				Y = (Map_square_size / 2) + Map_square_size *
				 (SILONG) Context->Source_data.Map_source.Y;

				/* Install or remove ? */
				if (Number)
				{
					/* Install -> Install sound effect */
					Add_located_effect(Number, (UNSHORT) Event->Byte_3,
					 (UNSHORT) Event->Byte_4, (UNSHORT) Event->Byte_5,
					 Event->Word_6, X, Y);
				}
				else
				{
					/* Remove -> Remove all sound effects at this position */
					Remove_located_effects_at(X, Y);
				}
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_Dialogue_event
 * FUNCTION  : Start Dialogue event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.04.95 11:44
 * LAST      : 30.06.95 14:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Start_Dialogue_event(void)
{
	struct Event_block *Event;

	/* In a dialogue ? */
	if (In_Dialogue)
	{
		/* Yes -> Error */
		Error(ERROR_ILLEGAL_DIALOGUE_RECURSION);
		return;
	}

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Start dialogue */
	Do_Dialogue((UNSHORT) Event->Byte_2, (UNSHORT) Event->Byte_1,
	 Event->Word_6, 0xFFFF);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Create_Trans_event
 * FUNCTION  : Create Transport event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 13:08
 * LAST      : 08.03.95 13:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Create_Trans_event(void)
{
	struct Event_block *Event;
	SISHORT X, Y;
	UNSHORT Transport_type;
	UNSHORT Map_nr;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get parameters */
	X = (SISHORT) Event->Byte_1;
	Y = (SISHORT) Event->Byte_2;
	Transport_type = (UNSHORT) Event->Byte_3;
	Map_nr = Event->Word_6;

	/* Use current coordinates ? */
	if (!X && !Y)
	{
		/* Yes -> Get current coordinates */
		X = PARTY_DATA.X;
		Y = PARTY_DATA.Y;
	}

	/* Use current map ? */
	if (!Map_nr)
	{
		/* Yes -> Load number */
		Map_nr = PARTY_DATA.Map_nr;
	}

	/* Create transport */
	Create_transport(Map_nr, X, Y, Transport_type);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_event
 * FUNCTION  : Execute event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:07
 * LAST      : 14.07.95 14:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_event(void)
{
	struct Event_action *Event_action;
	struct Event_block *Event;
	BOOLEAN Result;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Execute or forbid ? */
	if (Event->Byte_1)
	{
		/* Forbid -> Indicate the action is forbidden */
		Event_action->Flags |= FORBIDDEN;
	}
	else
	{
		/* Execute -> Indicate the action is NOT forbidden */
		Event_action->Flags &= ~FORBIDDEN;

		/* Go one level deeper */
		Result = Execute_event_action();

		/* Action executed ? */
		if (!Result)
		{
			/* No -> Follow negative chain */
			Event->Next_event_nr = Event->Word_8;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_Member_event
 * FUNCTION  : Remove Member event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.08.95 11:03
 * LAST      : 15.09.95 15:28
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_Member_event(void)
{
	struct Event_block *Event;
	struct Character_data *Char;
	UNSHORT Member_index;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Find character in party */
	Member_index = Find_character_in_party((UNSHORT) Event->Byte_1);

	/* Found ? */
	if (Member_index != 0xFFFF)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Party_char_handles[Member_index - 1]);

		/* Act depending on remove mode */
		switch (Event->Byte_2)
		{
			case 0:
			{
				/* New CD bit number ? */
				if (Event->Word_6 != 0xFFFF)
				{
					/* Set CD bit number */
					Char->Destination_CD_bit = Event->Word_6;
				}
				break;
			}
			case 1:
			{
				/* New CD bit number ? */
				if (Event->Word_6 != 0xFFFF)
				{
					/* Set CD bit number */
					Char->Destination_CD_bit = Event->Word_6;
				}
				break;
			}
			case 2:
			{
				/* Set CD bit number */
				Char->Destination_CD_bit = 0xFFFF;
				break;
			}
			case 3:
			{
				/* Set CD bit number */
				Char->Destination_CD_bit = 0xFFFF;
				break;
			}
		}

		MEM_Free_pointer(Party_char_handles[Member_index - 1]);

		/* Remove party member */
		Remove_party_member(Member_index);

		/* Are we talking to this party member ? */
		if (In_Dialogue && (Dialogue_char_type == PARTY_CHAR_TYPE) &&
		 (Dialogue_char_index == (UNSHORT) Event->Byte_1))
		{
			/* Yes -> The dialogue partner is now no longer in the party */
			Dialogue_partner_in_party = FALSE;

			/* End dialogue */
			End_dialogue_flag = TRUE;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : End_Dialogue_event
 * FUNCTION  : End Dialogue event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 17:06
 * LAST      : 25.06.95 17:06
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
End_Dialogue_event(void)
{
	/* End dialogue */
	End_dialogue_flag = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wipe_event
 * FUNCTION  : Wipe event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 20:22
 * LAST      : 11.08.95 19:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wipe_event(void)
{
	struct Event_block *Event;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Wipe */
	Wipe((UNSHORT) Event->Byte_1);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_Animation_event
 * FUNCTION  : Play Animation event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 12:17
 * LAST      : 11.08.95 14:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_Animation_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	SISHORT Playback_X, Playback_Y;
	SISHORT Screen_X, Screen_Y;
	UNSHORT Map_X, Map_Y;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get playback coordinates */
	Playback_X = (SISHORT) Event->Word_6;
	Playback_Y = (SISHORT) Event->Word_8;

	/* Relative coordinates ? */
	if (Event->Byte_3 & 0x02)
	{
		/* Yes -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

		/* Is a 2D map ? */
		if (!_3D_map)
		{
			/* Yes -> Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Relative to event coordinates */
				Map_X = Context->Source_data.Map_source.X;
				Map_Y = Context->Source_data.Map_source.Y;
			}
			else
			{
				/* No -> Relative to party coordinates */
				Map_X = PARTY_DATA.X;
				Map_Y = PARTY_DATA.Y;
			}

			/* Convert to screen coordinates */
			Convert_2D_map_to_screen_coordinates(Map_X, Map_Y, &Screen_X,
			 &Screen_Y);

			/* Add to playback coordinates */
			Playback_X += Screen_X;
			Playback_Y += Screen_Y;
		}
	}

	/* Play flic */
	Play_animation((UNSHORT) Event->Byte_1, Playback_X, Playback_Y,
	 (UNSHORT) Event->Byte_2, (UNSHORT) Event->Byte_4);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Offset_event
 * FUNCTION  : Offset event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.02.95 10:15
 * LAST      : 17.02.95 10:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Offset_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	SISHORT X, Y;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

	/* Relative coordinates ? */
	if (Event->Byte_3 & 0x01)
	{
		/* Yes -> Get relative coordinates */
		X = (SISHORT) ((SIBYTE) Event->Byte_1);
		Y = (SISHORT) ((SIBYTE) Event->Byte_2);

		/* Is this a map context ? */
		if (Context->Source == MAP_EVENT_SOURCE)
		{
			/* Yes -> Relative to event coordinates */
			X += Context->Source_data.Map_source.X;
			Y += Context->Source_data.Map_source.Y;
		}
		else
		{
			/* No -> Relative to party coordinates */
			X += PARTY_DATA.X;
			Y += PARTY_DATA.Y;
		}
	}
	else
	{
		/* No -> Get absolute coordinates */
		X = (SISHORT) Event->Byte_1;
		Y = (SISHORT) Event->Byte_2;
	}

	/* Trigger other event */
	Trigger_map_event_chain(X, Y, 0xFFFF,
	 Context->Source_data.Map_source.Trigger,
	 Context->Source_data.Map_source.Value);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pause_event
 * FUNCTION  : Pause event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.06.95 14:54
 * LAST      : 11.08.95 21:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pause_event(void)
{
	struct Event_block *Event;
	UNLONG Duration;
	UNLONG T;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get number of ticks */
	Duration = (UNLONG) Event->Byte_1;

	/* Wait for user ? */
	if (!Duration)
	{
		/* Yes */
		Wait_4_user();
	}
	else
	{
		/* No -> Get time */
		T = SYSTEM_GetTicks();

		/* Wait */
		while (SYSTEM_GetTicks() < T + Duration)
		{
			Update_display();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Single_Item_Chest_event
 * FUNCTION  : Single Item Chest event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.06.95 15:08
 * LAST      : 25.08.95 11:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Single_Item_Chest_event(void)
{
	struct Event_context *Context;
	struct Event_block *Event;
	struct Item_packet Packet;
	struct Item_data *Item_data;
	struct HDOB HDOB;
	struct Object *Object;
	SISHORT Source_X, Source_Y;
	UNSHORT Target_X, Target_Y;
	UNSHORT HDOB_nr;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

	/* Set default source coordinates */
	Source_X = (Screen_width - 16) / 2;
	Source_Y = (Panel_Y - 16) / 2;

	/* Is a map context ? */
	if (Context->Source == MAP_EVENT_SOURCE)
	{
		/* Yes -> 2D map ? */
		if (!_3D_map)
		{
			/* Yes -> Prepare screen */
			Current_2D_OPM = &Main_OPM;
			Draw_2D_scroll_buffer();

			/* Calculate source coordinates */
			Convert_2D_map_to_screen_coordinates(Context->Source_data.Map_source.X,
			 Context->Source_data.Map_source.Y, &Source_X, &Source_Y);
		}
	}

	/* Calculate target coordinates */
	Object = Get_object_data(Party_member_objects[PARTY_DATA.Active_member - 1]);
	Target_X = Object->Rect.left + (Object->Rect.width - 16) / 2;
	Target_Y = Object->Rect.top + (Object->Rect.height - 16) / 2;

	/* What kind of chest ? */
	switch (Event->Byte_1)
	{
		/* Item */
		case 0:
		{
			/* Build item packet */
			Build_item_packet(&Packet, Event->Word_6, Event->Word_8);

			/* Get item data */
			Item_data = Get_item_data(&Packet);

			/* Initialize HDOB data */
			BASEMEM_FillMemByte((UNBYTE *) &HDOB, sizeof(struct HDOB), 0);

			HDOB.Draw_mode			= HDOB_MASK;
			HDOB.X					= Source_X;
			HDOB.Y					= Source_Y;
			HDOB.Width				= 16;
			HDOB.Height				= 16;
			HDOB.Nr_frames			= (UNSHORT) Item_data->Nr_frames;
			HDOB.Graphics_handle	= Item_graphics_handle;
			HDOB.Graphics_offset	= 256 * Item_data->Pic_nr;

			Free_item_data();

			/* Add HDOB */
			HDOB_nr = Add_HDOB(&HDOB);

			/* Move HDOB towards target */
			Move_HDOB(HDOB_nr, Target_X, Target_Y, 12);

			/* Remove HDOB */
			Delete_HDOB(HDOB_nr);

			/* Give item to character */
			Put_item_in_character(Active_char_handle, 0,
			 (UNSHORT) Packet.Quantity, &Packet);

			break;
		}
		/* Gold */
		case 1:
		{
			/* Initialize HDOB data */
			BASEMEM_FillMemByte((UNBYTE *) &HDOB, sizeof(struct HDOB), 0);

			HDOB.Draw_mode			= HDOB_MASK;
			HDOB.X					= Source_X;
			HDOB.Y					= Source_Y;
			HDOB.Width				= 12;
			HDOB.Height				= 10;
			HDOB.Nr_frames			= 1;
			HDOB.Graphics_handle	= NULL;
			HDOB.Graphics_offset	= (UNLONG) &(Gold_symbol[0]);

			/* Add HDOB */
			HDOB_nr = Add_HDOB(&HDOB);

			/* Move HDOB towards target */
			Move_HDOB(HDOB_nr, Target_X, Target_Y, 12);

			/* Remove HDOB */
			Delete_HDOB(HDOB_nr);

			/* Give gold */
			Set_party_gold(Get_party_gold() + (UNLONG) Event->Word_8);

			break;
		}
		/* Food */
		case 2:
		{
			/* Initialize HDOB data */
			BASEMEM_FillMemByte((UNBYTE *) &HDOB, sizeof(struct HDOB), 0);

			HDOB.Draw_mode			= HDOB_MASK;
			HDOB.X					= Source_X;
			HDOB.Y					= Source_Y;
			HDOB.Width				= 20;
			HDOB.Height				= 10;
			HDOB.Nr_frames			= 1;
			HDOB.Graphics_handle	= NULL;
			HDOB.Graphics_offset	= (UNLONG) &(Food_symbol[0]);

			/* Add HDOB */
			HDOB_nr = Add_HDOB(&HDOB);

			/* Move HDOB towards target */
			Move_HDOB(HDOB_nr, Target_X, Target_Y, 12);

			/* Remove HDOB */
			Delete_HDOB(HDOB_nr);

			/* Give food */
			Set_party_food(Get_party_food() + (UNLONG) Event->Word_8);

			break;
		}
	}

	/* Is a map context ? */
	if (Context->Source == MAP_EVENT_SOURCE)
	{
		/* Yes -> 2D map ? */
		if (!_3D_map)
		{
			/* Yes -> Prepare screen */
			Current_2D_OPM = NULL;
			Draw_2D_scroll_buffer();
			Switch_screens();
		}
	}

	/* Indicate success of event */
	Set_success_flag();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Ask_Surrender_event
 * FUNCTION  : Ask Surrender event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Ask_Surrender_event(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_Script_event
 * FUNCTION  : Do Script event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 13:02
 * LAST      : 11.08.95 12:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_Script_event(void)
{
	struct Event_block *Event;
	UNSHORT Old_map_nr;

	/* Get current map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Execute script */
	Execute_script(Event->Word_6);

	/* Still in the same map ? */
	if (Old_map_nr != PARTY_DATA.Map_nr)
	{
		/* No -> Break current event context */
		Break_current_context();
	}
}

