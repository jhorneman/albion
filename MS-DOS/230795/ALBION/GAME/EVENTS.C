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

#include <FLC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <CONTROL.H>
#include <EVELOGIC.H>
#include <ITMLOGIC.H>
#include <EVENTS.H>
#include <STATAREA.H>
#include <CHEST.H>
#include <DOOR.H>
#include <FONT.H>
#include <DIALOGUE.H>
#include <INVITEMS.H>
#include <INPUT.H>
#include <MUSIC.H>
#include <COMBAT.H>
#include <XFTYPES.H>
#include <GRAPHICS.H>

/* global variables */

UNSHORT Shortcut_map_X = 36;
UNSHORT Shortcut_map_Y = 12;

Event_handler Event_handlers[MAX_EVENT_TYPES] = {
	Map_Exit_event, Door_event, Chest_event, Text_event,
	Spinner_event, Trap_event, Change_Used_Item_event, Datachange_event,
	Change_Icon_event, Encounter_event, Place_action_event, Query_event,
	Modify_event, NULL, Signal_event, Clone_Automap_event,
	Sound_event, Start_Dialogue_event, Create_Trans_event, Execute_event,
	Remove_Member_event, End_Dialogue_event, Wipe_event, Play_Animation_event,
	Offset_event, Pause_event, Single_Item_Chest_event, Ask_Surrender_event,
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
			Do_normal_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Teleporter */
		case TELEPORTER_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = TELEPORT_MX_MODE;

			/* Do map exit */
			Do_normal_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Trapdoor up */
		case TRAPDOOR_UP_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = TRAPDOOR_UP_MX_MODE;

			/* Do map exit */
			Do_normal_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Jump */
		case JUMP_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = JUMP_MX_MODE;

			/* Do map exit */
			Do_normal_map_exit(New_X, New_Y, New_VD, New_map_nr);

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
			Do_normal_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Fall down mode */
		case FALL_DOWN_MX_TYPE:
		{
			/* Set special map exit mode */
			Special_map_exit_mode = FALL_DOWN_MX_MODE;

			/* Do map exit */
			Do_normal_map_exit(New_X, New_Y, New_VD, New_map_nr);

			break;
		}
		/* Chapter screen */
		case CHAPTER_SCREEN_MX_TYPE:
		{
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
 * NAME      : Do_normal_map_exit
 * FUNCTION  : Do a normal map exit.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.06.95 14:52
 * LAST      : 04.07.95 17:08
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
Do_normal_map_exit(UNSHORT New_X, UNSHORT New_Y, UNSHORT New_VD,
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

		/* Exit current map */
		Exit_map();

		/* Set new position */
		PARTY_DATA.X = New_X;
		PARTY_DATA.Y = New_Y;
		PARTY_DATA.View_direction = New_VD;
		PARTY_DATA.Map_nr = New_map_nr;

		/* Initialize new map */
		Result = Init_map();
		if (!Result)
		{
			Exit_program();
		}
	}
	else
	{
		/* No -> Change position */
		Change_position(New_X, New_Y, New_VD);
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

			/* Save door in bit array */
			Write_bit_array(DOOR_BIT_ARRAY, (UNLONG) Event->Word_6,
			 SET_BIT_ARRAY);

			break;
		}
		/* The door was already open */
		case IS_OPEN_DOOR_STATUS:
		{
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
 * LAST      : 30.06.95 18:44
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

	/* Set ink _*/
	Set_ink(GOLD_TEXT);

	/* What kind of text ? */
	switch (Event->Byte_1)
	{
		/* Normal text */
		case NORMAL_TEXT_TYPE:
		{
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
		/* Text spoken by an NPC */
		case NPC_SPOKEN_TEXT_TYPE:
		{
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
		/* Multiple-choice text */
		case MULTI_CHOICE_TEXT_TYPE:
		{
			struct Dialogue_node New_node;

			/* Build new dialogue node */
			New_node.Text_handle = Text_handle;
			New_node.Text_block_nr = Block_nr;
			New_node.Flags = 0;

			/* Set current dialogue node */
			Set_current_dialogue_node(&New_node);

			/* Break current event context */
			Break_current_context();

			break;
		}
		/* Place title text */
		case PLACE_TITLE_TEXT_TYPE:
		{
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
			New_node.Text_handle = Text_handle;
			New_node.Text_block_nr = Block_nr;
			New_node.Flags = DIALOGUE_NO_DEFAULT;

			/* Set current dialogue node */
			Set_current_dialogue_node(&New_node);

			/* Break current event context */
			Break_current_context();

			break;
		}
		/* Text spoken by the active party member */
		case ACTIVE_SPOKEN_TEXT_TYPE:
		{
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
		/* Multiple-choice default text */
		case MULTI_CHOICE_DEFAULT_TEXT_TYPE:
		{
			struct Dialogue_node New_node;

			/* Erase default dialogue node ? */
			if (Block_nr == 0xFF)
			{
				/* Yes */
				New_node.Text_handle = NULL;
			}
			else
			{
				/* No */
				New_node.Text_handle = Text_handle;
			}

			/* Build new dialogue node */
			New_node.Text_block_nr = Block_nr;
			New_node.Flags = 0;

			/* Set default dialogue node */
			Set_default_dialogue_node(&New_node);

			/* Break current event context */
			Break_current_context();

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
				if (PARTY_DATA.Member_nrs[i])
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
			UNSHORT i;

			for (i=0;i<6;i++)
			{
				/* Is this the right member ? */
				if (PARTY_DATA.Member_nrs[i] == (UNSHORT) Event->Byte_5)
				{
					/* Yes -> Trap party member */
					Do_trap(i + 1, Event);
					break;
				}
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
 * NAME      : Change_Used_Item_event
 * FUNCTION  : Change Used Item event handler.
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
Change_Used_Item_event(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Datachange_event
 * FUNCTION  : Datachange event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:36
 * LAST      : 29.12.94 11:37
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

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Whose data is changed ? */
	switch (Event->Byte_3)
	{
		/* Active party member */
		case ACTIVE_MEMBER_DC:
		{
			Do_datachange(Active_char_handle, Event);
			break;
		}
		/* Entire party */
		case ENTIRE_PARTY_DC:
		{
			UNSHORT i;

			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (PARTY_DATA.Member_nrs[i])
				{
					/* Yes -> Change party member's data */
					Do_datachange(Party_char_handles[i], Event);
				}
			}

			break;
		}
		/* A single party member */
		case SINGLE_MEMBER_DC:
		{
			UNSHORT i;

			/* Search party */
			for (i=0;i<6;i++)
			{
				/* Is this the right member ? */
				if (PARTY_DATA.Member_nrs[i] == (UNSHORT) Event->Byte_5)
				{
					/* Yes -> Change party member's data */
					Do_datachange(Party_char_handles[i], Event);
					break;
				}
			}

			break;
		}
		/* Current combat participant */
		case CURRENT_PART_DC:
		{
			break;
		}
		/* Current combat participant's victim */
		case CURRENT_PART_VICTIM_DC:
		{
			break;
		}
		/* NPC */
		case NPC_DC:
		{
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_datachange
 * FUNCTION  : Actually change data.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:57
 * LAST      : 31.03.95 15:23
 * INPUTS    : MEM_HANDLE Char_handle - Handle of victim's character data.
 *             struct Event_block *Event - Pointer to current event block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_datachange(MEM_HANDLE Char_handle, struct Event_block *Event)
{
	struct Character_data *Char;
	UNSHORT Modify_mode, Index, Number;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get data-change parameters */
	Modify_mode = (UNSHORT) Event->Byte_2;
	Index = Event->Word_6;
	Number = Event->Word_8;

	/* Random number ? */
	if (Event->Byte_4)
	{
		/* Yes -> Randomize */
		Number = rand() % Number + 1;
	}

	/* Change which data ? */
	switch (Event->Byte_1)
	{
		/* Attribute */
		case ATTRIBUTE_DC_TYPE:
		{
			struct Attribute *Attr;

			/* Get attribute data */
			Attr = &(Char->Attributes[Index]);

			/* Already too high ? */
			if (Attr->Normal + Attr->Magic < Attr->Maximum)
			{
				/* No -> Modify */
				Attr->Normal = (UNSHORT) Modify_amount((UNLONG) (Attr->Normal +
				 Attr->Magic), (UNLONG) Attr->Maximum, Modify_mode,
				 (UNLONG) Number) - Attr->Magic;
			}
			break;
		}
		/* Skill */
		case SKILL_DC_TYPE:
		{
			struct Skill *Skill;

			/* Get skill data */
			Skill = &(Char->Skills[Index]);

			/* Already too high ? */
			if (Skill->Normal + Skill->Magic < Skill->Maximum)
			{
				/* No -> Modify */
				Skill->Normal = (UNSHORT) Modify_amount((UNLONG) (Skill->Normal +
				 Skill->Magic), (UNLONG) Skill->Maximum, Modify_mode,
				 (UNLONG) Number) - Skill->Magic;
			}
			break;
		}
		/* Life-points */
		case LP_DC_TYPE:
		{
			UNSHORT New_LP;

			/* Calculate new LP */
			New_LP = (UNSHORT) Modify_amount((UNLONG) Get_LP(Char_handle),
			 32767, Modify_mode, (UNLONG) Number);

			/* Set */
			Set_LP(Char_handle, New_LP);

			break;
		}
		/* Spell-points */
		case SP_DC_TYPE:
		{
			UNSHORT New_SP;

			/* Calculate new SP */
			New_SP = (UNSHORT) Modify_amount((UNLONG) Get_SP(Char_handle),
			 32767, Modify_mode, (UNLONG) Number);

			/* Set */
			Set_SP(Char_handle, New_SP);

			break;
		}
		/* Attacker per round */
		case APR_DC_TYPE:
		{
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
					Clear_condition(Char_handle, Index);
					break;
				/* Set bit */
				case SET_MODIFY_MODE:
					Set_condition(Char_handle, Index);
					break;
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
					if (Get_conditions(Char_handle) & (1 << Index))
						Clear_condition(Char_handle, Index);
					else
						Set_condition(Char_handle, Index);
					break;
			}
			break;
		}
		/* Character level */
		case LEVEL_DC_TYPE:
		{
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
					Char->Known_languages &= ~(1 << Index);
					break;
				/* Set bit */
				case SET_MODIFY_MODE:
					Char->Known_languages |= (1 << Index);
					break;
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
					Char->Known_languages ^= (1 << Index);
					break;
			}
			break;
		}
		/* Experience points */
		case EP_DC_TYPE:
		{
			UNLONG EP;

			/* Calculate new EP */
			EP = Modify_amount(Get_EP(Char_handle), 0x7FFFFFFF, Modify_mode,
			 (UNLONG) Number);

			/* Set */
			Set_EP(Char_handle, EP);

			break;
		}
		/* Training points */
		case TP_DC_TYPE:
		{
			Char->Training_points = Modify_amount((UNLONG) Char->Training_points,
			 0xFFFF, Modify_mode, (UNLONG) Number);
			break;
		}
		/* Small portrait */
		case PORTRAIT_DC_TYPE:
		{
			break;
		}
		/* Full-body picture */
		case FULL_BODY_PIC_DC_TYPE:
		{
			break;
		}
		/* Event set #1 */
		case EVENT_SET_1_DC_TYPE:
		{
			break;
		}
		/* Event set #2 */
		case EVENT_SET_2_DC_TYPE:
		{
			break;
		}
		/* 2D graphics */
		case GFX_2D_DC_TYPE:
		{
			break;
		}
		/* Tactical icon */
		case TACTICAL_ICON_DC_TYPE:
		{
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
					Char->xKnown_spells[Index] &= ~(1 << Number);
					break;
				/* Set bit */
				case SET_MODIFY_MODE:
					Char->xKnown_spells[Index] |= (1 << Number);
					break;
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
					Char->xKnown_spells[Index] ^= (1 << Number);
					break;
			}
			break;
		}
		/* Maximum life-points */
		case MAX_LP_DC_TYPE:
		{
			Char->xLife_points_maximum = (UNSHORT) Modify_amount((UNLONG)
			 Char->xLife_points_maximum, 0x7FFF, Modify_mode, Number);
			break;
		}
		/* Maximum spell-points */
		case MAX_SP_DC_TYPE:
		{
			Char->xSpell_points_maximum = (UNSHORT) Modify_amount((UNLONG)
			 Char->xSpell_points_maximum, 0x7FFF, Modify_mode, Number);
			break;
		}
		/* Carried items */
		case ITEMS_DC_TYPE:
		{
			Modify_carried_items(Char_handle, Index, Number, Modify_mode);
			break;
		}
		/* Carried gold */
		case GOLD_DC_TYPE:
		{
			Char->Char_gold = (UNSHORT) Modify_amount((UNLONG)
			 Char->Char_gold, 32767, Modify_mode, Number);
			break;
		}
		/* Carried food */
		case FOOD_DC_TYPE:
		{
			Char->Char_food = (UNSHORT) Modify_amount((UNLONG)
			 Char->Char_food, 32767, Modify_mode, Number);
			break;
		}
		/* Morale / courage */
		case MORALE_DC_TYPE:
		{
			break;
		}
	}

	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_Icon_event
 * FUNCTION  : Change Icon event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 17:06
 * LAST      : 29.12.94 17:06
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
	UNSHORT Type, Sub_type, Value, Map_nr;
	UNBYTE *Map_data;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get current event block */
	Event = &(Context->Data);

	/* Get parameters */
	Type = (UNSHORT) Event->Byte_4;
	Sub_type = (UNSHORT) Event->Byte_5;
	Value = Event->Word_6;
	Map_nr = Event->Word_8;

	/* Is change of event entry ? */
	if (Type == EVENT_ENTRY_CHANGE)
	{
		/* Yes -> Turn chain number into first block number */
		Map_data = MEM_Claim_pointer(Map_handle);
		Value = Get_event_first_block_nr(Map_data, Value);
		MEM_Free_pointer(Map_handle);
	}

	/* Relative coordinates ? */
	if (Event->Byte_3 & 0x01)
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
	if (!(Event->Byte_3 & 0x02))
	{
		/* Yes */
		Add_modification(X, Y, Type, Sub_type, Value, Map_nr);
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
 * LAST      : 30.06.95 14:42
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
	UNSHORT Monster_group_nr, Combat_background_nr;

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

	/* Was a combat background given ? */
	if (!Combat_background_nr)
	{
		/* No -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

		/* Is a map context ? */
		if (Context->Source == MAP_EVENT_SOURCE)
		{
			/* Yes -> Get combat background from current location */
			Combat_background_nr = (Get_location_status(PARTY_DATA.X,
			 PARTY_DATA.Y, 0xFFFF) & COMBAT_BACKGROUND_NR) >>
			 COMBAT_BACKGROUND_B;
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
Place_action_event(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Query_event
 * FUNCTION  : Query event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:29
 * LAST      : 01.07.95 15:29
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
			Result = Character_in_party(Number);
			break;
		}
		case ITEM_CARRIED_QM_TYPE:
		{
			UNSHORT Nr_items, i;

			/* Count items */
			Nr_items = 0;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (PARTY_DATA.Member_nrs[i])
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

			/* Get current event context */
			Context = &(Event_context_stack[Event_context_stack_index]);

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Was the right item used ? */
				Result = (BOOLEAN)(Context->Source_data.Map_source.Value
				 == Number);
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
		case INTERNAL_QM_TYPE:
		{
			/* Read bit from internal party-data flags */
			Result = (BOOLEAN) PARTY_DATA.Internal_flags & (1 << Number);
			break;
		}
		case LIGHT_STATE_QM_TYPE:
		{
			break;
		}
		case DIRECTION_QM_TYPE:
		{
			/* Compare view direction */
			Result = Query_compare(PARTY_DATA.View_direction, Query_mode, Number);
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
							/* Yes -> Make this word known */
							Write_bit_array(KNOWN_WORDS_BIT_ARRAY, Word_instances[i],
							 SET_BIT_ARRAY);

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
			struct Character_data *Char;
			UNSHORT Gold, i;

			/* Count gold */
			Gold = 0;
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (PARTY_DATA.Member_nrs[i])
				{
					/* Yes -> Count party member's gold coins */
					Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);

					Gold += Char->Char_gold;

					MEM_Free_pointer(Party_char_handles[i]);
				}
			}

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
				if (PARTY_DATA.Member_nrs[i])
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
			UNSHORT i;

			/* Search party */
			Result = FALSE;
			for (i=0;i<6;i++)
			{
				/* Is this the right character ? */
				if (PARTY_DATA.Member_nrs[i] == Number)
				{
					/* Yes -> Check if party member is conscious */
					if (!(Get_conditions(Party_char_handles[i]) & DEAD_MASK))
					 Result = TRUE;
					break;
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
				if (PARTY_DATA.Member_nrs[i])
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
				if (PARTY_DATA.Member_nrs[i])
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
				if (PARTY_DATA.Member_nrs[i])
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
			/* Check active party member */
			Result = (PARTY_DATA.Active_member == Number);
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
			if (Number < BYTE_COUNTERS_MAX)
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
		case CHECK_ACTION_MONSTER_QM_TYPE:
		{
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
		case BATTLE_SITUATION_QM_TYPE:
		{
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
		case RELATIVE_WEIGHT_QM_TYPE:
		{
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

			/* Input quantity */
			Quantity = (UNLONG) Input_number(0, 0, 9999, System_text_ptrs[201]);

			/* Compare quantity */
			Result = Query_compare(Quantity, Query_mode, Number);
			break;
		}
		case VISIBLE_QM_TYPE:
		{
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
 * FILE      : EVENTS.C
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
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:05
 * LAST      : 29.12.94 11:05
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
	UNSHORT Number, Extra, Modify_mode;

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
		/* Items carried by the party */
		case ITEM_CARRIED_QM_TYPE:
		{
			Modify_items_carried_by_party(Number, Extra, Modify_mode);
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
		/* Internal flags bit array */
		case INTERNAL_QM_TYPE:
		{
			/* Which modify mode ? */
			switch (Modify_mode)
			{
				/* Clear bit */
				case CLEAR_MODIFY_MODE:
					PARTY_DATA.Internal_flags &= ~(1 << Number);
					break;
				/* Set bit */
				case SET_MODIFY_MODE:
					PARTY_DATA.Internal_flags |= (1 << Number);
					break;
				/* Toggle bit */
				case TOGGLE_MODIFY_MODE:
					PARTY_DATA.Internal_flags ^= (1 << Number);
					break;
			}
			break;
		}
		/* Current light state */
		case LIGHT_STATE_QM_TYPE:
		{
			break;
		}
		/* View direction */
		case DIRECTION_QM_TYPE:
		{
			break;
		}
		/* Gold carried by the party */
		case GOLD_CARRIED_QM_TYPE:
		{
			break;
		}
		/* Food carried by the party */
		case FOOD_CARRIED_QM_TYPE:
		{
			break;
		}
		/* Currently active member */
		case ACTIVE_MEMBER_QM_TYPE:
		{
			break;
		}
		/* Byte counter array */
		case BYTE_COUNTER_QM_TYPE:
		{
			/* Legal byte-counter index ? */
			if (Number < BYTE_COUNTERS_MAX)
			{
				/* Yes -> Modify byte counter */
				PARTY_DATA.Byte_counters[Number] =
					(UNBYTE) Modify_amount((UNLONG) PARTY_DATA.Byte_counters[Number],
					(UNLONG) 255, Modify_mode, (UNLONG) Extra);
			}
			break;
		}
		/* 3D shading factor */
		case SHADING_FACTOR_QM_TYPE:
		{
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_amount
 * FUNCTION  : Modify an amount.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 11:15
 * LAST      : 29.12.94 11:15
 * INPUTS    : UNLONG Amount - Original amount.
 *             UNLONG Maximum - Maximum amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 *             UNLONG Number - Number that is used to modify amount.
 * RESULT    : UNLONG : Modified amount.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Modify_amount(UNLONG Amount, UNLONG Maximum, UNSHORT Modify_mode,
 UNLONG Number)
{
	/* Which modify mode ? */
	switch (Modify_mode)
	{
		/* Clear */
		case CLEAR_MODIFY_MODE:
			Amount = 0;
			break;
		/* Set to maximum */
		case SET_MODIFY_MODE:
			Amount = Maximum;
			break;
		/* Load with value */
		case LOAD_MODIFY_MODE:
			Amount = Number;
			break;
		/* Increase */
		case INC_MODIFY_MODE:
			Amount += Number;
			if (Amount > Maximum)
				Amount = Maximum;
			break;
		/* Decrease */
		case DEC_MODIFY_MODE:
			if (Amount < Number)
				Amount = 0;
			else
				Amount -= Number;
			break;
		/* Increase by percentage */
		case INCPERC_MODIFY_MODE:
			Number = (Number * Amount) / 100;
			Amount += Number;
			if (Amount > Maximum)
				Amount = Maximum;
			break;
		/* Decrease by percentage */
		case DECPERC_MODIFY_MODE:
			Number = (Number * Amount) / 100;
			if (Amount < Number)
				Amount = 0;
			else
				Amount -= Number;
			break;
	}

	return Amount;
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
			case NPC_CHAR_TYPE:
			{
				Signal_action.Actor_type = DIALOGUE_ACTOR_TYPE;
				Signal_action.Actor_index = Context->Source_data.Set_source.Char_index;
				break;
			}
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
 * LAST      : 13.02.95 12:17
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
			UNSHORT X, Y;

			/* Is a map context ? */
			if (Context->Source == MAP_EVENT_SOURCE)
			{
				/* Yes -> Relative to event coordinates */
				X = Context->Source_data.Map_source.X;
				Y = Context->Source_data.Map_source.Y;

				/* Install sound effect */
				Add_located_effect(Number, (UNSHORT) Event->Byte_3,
				 (UNSHORT) Event->Byte_4, (UNSHORT) Event->Byte_5, Event->Word_6,
				 (SILONG) X * Map_square_size, (SILONG) Y * Map_square_size);
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
Remove_Member_event(void)
{
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
 * LAST      : 03.05.95 20:22
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

	/* What kind of wipe ? */
	switch (Event->Byte_1)
	{
		/* Plain switch */
		case 0:
		{
			/* Update the screen */
	   	Update_screen();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_Animation_event
 * FUNCTION  : Play Animation event handler.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 12:17
 * LAST      : 27.04.95 12:17
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
	#ifdef URGLE
	struct Event_block *Event;
	struct Flic_playback FLC;
	MEM_HANDLE Flic_handle;
	SISHORT X, Y;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Prepare screen */
	Prepare_screen();

	/* Get coordinates */
	X = (SISHORT) Event->Word_6;
	Y = (SISHORT) Event->Word_8;

	/* Relative coordinates ? */
	if (Event->Byte_3 & 0x02)
	{
		#ifdef URGLE
		/* Yes -> Get current event context */
		Context = &(Event_context_stack[Event_context_stack_index]);

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
		#endif
	}

	/* Load flic */
	Flic_handle = Load_subfile(FLIC, (UNSHORT) Event->Byte_1);

	Ptr = MEM_Claim_pointer(Flic_handle);

	FLC.Output_frame = &Main_OPM;
	FLC.Playback_X = X;
	FLC.Playback_Y = Y;
	FLC.Input_buffer = Ptr;
	FLC.Palette = &Palette;
	FLC.Screen = &Screen;
	FLC.Black_colour_index = BLACK;

	for (i=0;i<(UNSHORT) Event->Byte_2;i++)
	{
		FLC_Start_flic_playback(&FLC);
		DSA_ActivatePal(&Screen);

		for (j=1;j<FLC.Nr_frames;j++)
		{
			FLC_Playback_flic_frame();
			DSA_ActivatePal(&Screen);

			Update_screen();
		}

		FLC_Stop_flic_playback();
	}
	#endif
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
 * LAST      : 02.06.95 14:54
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
	UNSHORT Counter;

	/* Get current event block */
	Event = &(Event_context_stack[Event_context_stack_index].Data);

	/* Get number of ticks */
	Counter = (UNSHORT) Event->Byte_1;

	/* Wait for user ? */
	if (!Counter)
	{
		/* Yes */
		Wait_4_user();
	}
	else
	{
		/* No */
		SYSTEM_WaitTicks((UNLONG) Counter);
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
 * LAST      : 19.06.95 15:08
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
	UNSHORT Source_X, Source_Y;
	UNSHORT Temp_X, Temp_Y;
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
			Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);

			/* Calculate source coordinates */
			Convert_2D_map_to_screen_coordinates(Context->Source_data.Map_source.X,
			 Context->Source_data.Map_source.Y, &Temp_X, &Temp_Y);

			/* Inside the 2D map window ? */
			if ((Temp_X != 0xFFFF) && (Temp_Y != 0xFFFF))
			{
				/* Yes -> Accept these values */
				Source_X = Temp_X;
				Source_Y = Temp_Y;
			}
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

			HDOB.Draw_mode = HDOB_MASK;
			HDOB.X = Source_X;
			HDOB.Y = Source_Y;
			HDOB.Width = 16;
			HDOB.Height = 16;
			HDOB.Nr_frames = (UNSHORT) Item_data->Nr_frames;
			HDOB.Graphics_handle = Item_graphics_handle;
			HDOB.Graphics_offset = 256 * Item_data->Pic_nr;

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
			break;
		}
		/* Food */
		case 2:
		{
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
			Draw_2D_scroll_buffer(MAP_2D_X, MAP_2D_Y);
			Switch_screens();
		}
	}

	/* Indicate success of event */
	Set_success_flag();
}

#if FALSE
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
#endif

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
Do_Script_event(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_carried_items
 * FUNCTION  : Modify the number of a certain item carried by a character.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 00:05
 * LAST      : 04.07.95 19:06
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Item_index - Index of item.
 *             UNSHORT Number - Number that is used to modify amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 * RESULT    : None.
 * NOTES     : - This function may give a character more items than he or she
 *              can carry. Also, if the character does not have enough room
 *              in his or her backpack, the function will not go into an
 *              endless loop.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Modify_carried_items(MEM_HANDLE Char_handle, UNSHORT Item_index,
 UNSHORT Number, UNSHORT Modify_mode)
{
	struct Character_data *Char;
	struct Item_packet Add_packet;
	UNSHORT Old_amount;
	UNSHORT New_amount;
	UNSHORT Counter;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Count current number of items */
	Old_amount = Count_items_in_backpack(Char_handle, Item_index);

	/* Calculate new amount */
	New_amount = (UNSHORT) Modify_amount((UNLONG) Old_amount, 0xFFFFFFFF,
	 Modify_mode, (UNLONG) Number);

	/* Any change ? */
	if (Old_amount == New_amount)
		return;

	/* Yes -> More or less ? */
	if (Old_amount > New_amount)
	{
		/* Less -> Remove items */
		Counter = Old_amount - New_amount;

		/* Check backpack items */
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
					/* Yes -> Break */
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
	else
	{
		/* More -> Add items */
		Counter = New_amount - Old_amount;

		/* Clear packet */
		Clear_packet(&Add_packet);

		while (Counter)
		{
			/* Set item index */
			Add_packet.Index = Item_index;

			/* Too much for one packet ? */
			if (Counter > 99)
			{
				/* Yes -> First do batch of 99 items */
				Add_packet.Quantity = 99;

				/* Count down */
				Counter -= 99;
			}
			else
			{
				/* No -> Do all items */
				Add_packet.Quantity = Counter;

				/* Count down */
				Counter = 0;
			}

			/* Try to move packet to backpack */
			Auto_move_packet_to_backpack(Char_handle, &Add_packet);

			#if FALSE
			/* Anything left in the packet ? */
			if (!Packet_empty(&Add_packet))
			{
				/* Yes -> Count up */
				Counter += (UNSHORT) Add_packet.Quantity;
			}
			#endif
		}
	}
	MEM_Free_pointer(Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Modify_items_carried_by_party
 * FUNCTION  : Modify the number of a certain item carried by the party.
 * FILE      : EVENTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 10:49
 * LAST      : 28.04.95 10:49
 * INPUTS    : UNSHORT Item_index - Index of item.
 *             UNSHORT Number - Number that is used to modify amount.
 *             UNSHORT Modify_mode - Modify mode (from event data).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Modify_items_carried_by_party(UNSHORT Item_index, UNSHORT Number,
 UNSHORT Modify_mode)
{
	struct Character_data *Char;
	struct Item_packet Add_packet;
	BOOLEAN Success;
	UNSHORT Nr_items[6] = {0, 0, 0, 0, 0, 0 };
	UNSHORT Old_amount = 0;
	UNSHORT New_amount;
	UNSHORT Counter;
	UNSHORT Subtract;
	UNSHORT i, j;

	/* Count amount of items currently carried by each party member */
	Old_amount = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Count party member's items */
			Nr_items[i] = Count_items_on_body(Party_char_handles[i], Number);
			Nr_items[i] += Count_items_in_backpack(Party_char_handles[i], Number);
		}

		/* Increase total amount */
		Old_amount += Nr_items[i];
	}

	/* Calculate new amount */
	New_amount = (UNSHORT) Modify_amount((UNLONG) Old_amount, 0xFFFFFFFF,
	 Modify_mode, (UNLONG) Number);

	/* Any change ? */
	if (Old_amount == New_amount)
		return;

	/* Yes -> More or less ? */
	if (Old_amount > New_amount)
	{
		/* Less -> Remove items */
		Counter = Old_amount - New_amount;

		/* Check party */
		for (i=0;i<6;i++)
		{
			/* Anyone there ? */
			if (PARTY_DATA.Member_nrs[i])
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
						Remove_item(Party_char_handles[i], j + 1, 1);

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
							Remove_item(Party_char_handles[i], ITEMS_ON_BODY + j + 1,
							 1);

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
					/* Yes -> Break */
					break;
				}
			}
		}
	}
	else
	{
		/* More -> Add items */
		Counter = New_amount - Old_amount;

		/* Clear packet */
		Clear_packet(&Add_packet);

		while(Counter)
		{
			/* No success yet */
			Success = FALSE;

			/* Give one item to each party member */
			for (i=0;i<6;i++)
			{
				/* Anyone there ? */
				if (PARTY_DATA.Member_nrs[i])
				{
					/* Yes -> Initialize packet */
					Add_packet.Index = Item_index;
					Add_packet.Quantity = 1;

					/* Try to move packet to backpack */
					Auto_move_packet_to_backpack(Party_char_handles[i],
					 &Add_packet);

					/* Anything left in the packet ? */
					if (Packet_empty(&Add_packet))
					{
						/* No -> Count down */
						Counter--;

						/* Indicate success */
						Success = TRUE;
					}

					/* Satisfied ? */
					if (!Counter)
					{
						/* Yes -> Break */
						break;
					}
				}
			}

			/* No luck at all ? */
			if (!Success)
			{
				/* Yes -> Then don't bother */
				break;
			}
		}
	}
}

