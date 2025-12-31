/************
 * NAME     : EVELOGIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 27-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : EVELOGIC.H
 ************/

/* includes */

#include <string.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBERROR.H>

#include <TEXT.H>

#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <CONTROL.H>
#include <EVELOGIC.H>
#include <ITMLOGIC.H>
#include <EVENTS.H>
#include <DIALOGUE.H>
#include <DIAGNOST.H>
#include <COMBAT.H>
#include <MAGIC.H>

/* prototypes */

/* Map event logic support functions */
struct Map_event_entry *Get_event_entry_at_position(UNBYTE *Map_data,
 SISHORT X, SISHORT Y);

UNSHORT Get_event_chain_nr_at_position(UNBYTE *Map_data, SISHORT X,
 SISHORT Y);

/* global variables */

static BOOLEAN Dialogue_set_reacted;

/* Event action stack */
struct Event_action Event_action_stack[EVENT_ACTIONS_MAX];
UNSHORT Event_action_stack_index = 0;

/* Event context stack */
struct Event_context Event_context_stack[EVENT_CONTEXTS_MAX];
UNSHORT Event_context_stack_index = 0;

/* Event set lists */
static struct Event_set_list_entry Standard_event_set_list[] = {
	{MAP_SET, 0},
	{ACTING_MEMBER_SET, 0},
	{NON_ACTING_MEMBER_SET, 1},
	{NON_ACTING_MEMBER_SET, 2},
	{NON_ACTING_MEMBER_SET, 3},
	{NON_ACTING_MEMBER_SET, 4},
	{NON_ACTING_MEMBER_SET, 5},
	{NON_ACTING_MEMBER_SET, 6},
	{ITEM_SET, 0},
	{MAGIC_SET, 0},
	{DEFAULT_SET, 0},
	{0xFFFF, 0}
};

static struct Event_set_list_entry Dialogue_event_set_list[] = {
	{DIALOGUE_SET, 0},
	{0xFFFF, 0}
};

static struct Event_set_list_entry Combat_event_set_list[] = {
	{MAP_SET, 0},
	{ACTING_PART_SET, 0},
	{FRIEND_PART_SET, 1},
	{FRIEND_PART_SET, 2},
	{FRIEND_PART_SET, 3},
	{FRIEND_PART_SET, 4},
	{FRIEND_PART_SET, 5},
	{FRIEND_PART_SET, 6},
	{FRIEND_PART_SET, 7},
	{FRIEND_PART_SET, 8},
	{FRIEND_PART_SET, 9},
	{FRIEND_PART_SET, 10},
	{FRIEND_PART_SET, 11},
	{FRIEND_PART_SET, 12},
	{FRIEND_PART_SET, 13},
	{FRIEND_PART_SET, 14},
	{FRIEND_PART_SET, 15},
	{FRIEND_PART_SET, 16},
	{FRIEND_PART_SET, 17},
	{FRIEND_PART_SET, 18},
	{ENEMY_PART_SET, 1},
	{ENEMY_PART_SET, 2},
	{ENEMY_PART_SET, 3},
	{ENEMY_PART_SET, 4},
	{ENEMY_PART_SET, 5},
	{ENEMY_PART_SET, 6},
	{ENEMY_PART_SET, 7},
	{ENEMY_PART_SET, 8},
	{ENEMY_PART_SET, 9},
	{ENEMY_PART_SET, 10},
	{ENEMY_PART_SET, 11},
	{ENEMY_PART_SET, 12},
	{ENEMY_PART_SET, 13},
	{ENEMY_PART_SET, 14},
	{ENEMY_PART_SET, 15},
	{ENEMY_PART_SET, 16},
	{ENEMY_PART_SET, 17},
	{ENEMY_PART_SET, 18},
	{ITEM_SET, 0},
	{MAGIC_SET, 0},
	{DEFAULT_SET, 0},
	{0xFFFF, 0}
};

/* Event set handler lists */
static Set_handler Set_handlers[] = {
	Handle_default_set,
	Handle_map_set,
	Handle_item_set,
	Handle_magic_set,
	Handle_dialogue_set,
	Handle_acting_member_set,
	Handle_non_acting_member_set,
	Handle_acting_part_set,
	Handle_friend_part_set,
	Handle_enemy_part_set
};

/* Chain catalogue data */
static UNLONG Catalogued_map_chain_counters[MAX_TRIGGER_MODES];
static MEM_HANDLE Chain_catalogue_handles[MAX_TRIGGER_MODES];

/* Error handling data */
static UNCHAR _Event_library_name[] = "Events";

static struct Error_message _Event_errors[] = {
	{ERROR_ILLEGAL_EVENT_BLOCK,	"Illegal event block."},
	{ERROR_ENDLESS_LOOP,				"Endless loop in event chain."},
	{ERROR_ILLEGAL_EVENT_TYPE,		"Illegal event type."},
	{0, NULL}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Perform_dialogue_action
 * FUNCTION  :	Perform an event-invoking action in a dialogue.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 19:05
 * LAST      : 27.06.95 19:05
 * INPUTS    : struct Event_action *New - Pointer to event action data.
 * RESULT    : BOOLEAN : TRUE (the dialogue partner reacted to this action).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Perform_dialogue_action(struct Event_action *New)
{
	/* Clear flag */
	Dialogue_set_reacted = FALSE;

	/* Just perform the action */
	Perform_action(New);

	/* Return a special flag set by the dialogue event set handler */
	return Dialogue_set_reacted;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Perform_action
 * FUNCTION  :	Perform an event-invoking action.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 17:11
 * LAST      : 27.10.94 17:11
 * INPUTS    : struct Event_action *New - Pointer to event action data.
 * RESULT    : BOOLEAN : TRUE (executed) or FALSE (didn't execute).
 * BUGS      : No known.
 * NOTES     : - It is assumed that this action will execute by default.
 *             - The Event_set_ptr entry need not be set. This function will
 *              determine the current context and set the pointer itself.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Perform_action(struct Event_action *New)
{
	struct Event_action *Event_action;
	BOOLEAN Result;

	/* Make a new event action */
	Event_action = Push_event_action();

	/* Copy input data */
	memcpy((UNBYTE *) Event_action, (UNBYTE *) New, sizeof(struct Event_action));

	/* Determine event set list */
	/* In combat ? */
	if (In_Combat)
	{
		/* Yes -> Use combat set list */
		Event_action->Event_set_ptr = &(Combat_event_set_list[0]);
	}
	else
	{
		/* No -> In dialogue ? */
		if (In_Dialogue)
		{
			/* Yes -> Use dialogue set list */
			Event_action->Event_set_ptr = &(Dialogue_event_set_list[0]);
		}
		else
		{
			/* No -> Use standard set list */
			Event_action->Event_set_ptr = &(Standard_event_set_list[0]);
		}
	}

	/* Execute */
	Result = Execute_event_action();

	/* Remove event action */
	Pop_event_action();

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_event_action
 * FUNCTION  :	Execute the current event action.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 10:11
 * LAST      : 01.03.95 14:18
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (executed) or FALSE (didn't execute).
 * BUGS      : No known.
 * NOTES     : - This function must be re-entrant.
 *             - The Execute and Signal events will call this function.
 *             - This function assumes that the Event_action structure has been
 *              initialized properly.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Execute_event_action(void)
{
	struct Event_action *Event_action;
	struct Event_set_list_entry *Event_set_ptr;
	UNSHORT Type, Value;
	BOOLEAN Result = FALSE;

	Event_action = &(Event_action_stack[Event_action_stack_index]);

	for (;;)
	{
		/* Get pointer to event set */
		Event_set_ptr = Event_action->Event_set_ptr;

		/* End of list ? */
		if (Event_set_ptr->Type == 0xFFFF)
		{
			/* Yes -> Has the action been forbidden or executed ? */
			if (!(Event_action->Flags & HAVE_EXECUTED)
			 && !(Event_action->Flags & FORBIDDEN))
			{
				/* No -> Execute action (if any) */
				if (Event_action->Action_ptr)
				{
					/* Yes -> Execute. Successful ? */
					if ((Event_action->Action_ptr)(Event_action))
					{
						/* Yes -> Indicate the action was successful */
						Event_action->Flags |= ACTION_SUCCESSFUL;
					}
				}

				/* Indicate the action was executed */
				Event_action->Flags |= HAVE_EXECUTED;
			}
			break;
		}

		/* No -> Get current set data */
		Type = Event_set_ptr->Type;
		Value = Event_set_ptr->Value;

		/* Store pointer to next event set */
		Event_set_ptr++;
		Event_action->Event_set_ptr = Event_set_ptr;

		/* Handle event set */
		Set_handlers[Type](Value);
		/* It is possible that during the execution of this event set, this
		 same function has been called again (and again), and the "deeper"
		 event sets have already been checked. */

		/* Has the action already been executed at a deeper nesting level ? */
		if (Event_action->Flags & HAVE_EXECUTED)
		{
			/* Yes -> The other event sets have already been checked */
			break;
		}
	}

	/* Executed ? */
	if (Event_action->Flags & HAVE_EXECUTED)
	{
		/* Yes -> Action successful ? */
		if (Event_action->Flags & ACTION_SUCCESSFUL)
		{
			/* Yes -> Success ! */
			Result = TRUE;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_default_set
 * FUNCTION  : Handle the default event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.10.94 15:36
 * LAST      : 30.07.95 19:57
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_default_set(UNSHORT Value)
{
	struct Event_action *Event_action;
	struct Event_context *Context;
	UNSHORT i;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Has a chain already been found ? */
	if (!(Event_action->Flags & FOUND_CHAIN))
	{
		/* No -> Search default event set */
		i = Search_event_set(Default_event_set_handle, 0xFFFF);

		/* Found anything ? */
		if (i != 0xFFFF)
		{
			/* Yes -> Make a new context */
			Context = Push_event_context();

			/* Prepare it */
			Prepare_event_set_context(Default_event_set_handle, i, Context,
			 NULL);

			Context->Text_handle = Default_event_text_handle;

			Context->Source_data.Set_source.Char_type			= 0xFFFF;
			Context->Source_data.Set_source.Char_index		= 0;
			Context->Source_data.Set_source.Char_set_index	= 0;
			Context->Source_data.Set_source.Set_index			= DEFAULT_EVENT_SET;

			/* Execute it */
			Execute_event_chain();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_map_set
 * FUNCTION  : Handle action events from the map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 12:13
 * LAST      : 24.07.95 19:07
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     :
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_map_set(UNSHORT Value)
{
	struct Event_action *Event_action;
	struct Event_context *Context;
	struct Map_event_entry *Entry_data;
	struct Event_block *Base;
	struct Event_block *Block;
	struct Event_block *Found_block = NULL;
	UNSHORT X, Y;
	UNSHORT i, j;
	UNSHORT Nr_entries = 0;
	UNSHORT First_block_nr = 0xFFFF;
	UNSHORT Chain_nr;
	UNBYTE *Map_ptr;
	UNBYTE *Ptr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return;

	/* Get initial event coordinates */
	X = PARTY_DATA.X;
	Y = PARTY_DATA.Y;

	/* Search back through the stack */
	i = Event_context_stack_index;
	while (i > 0)
	{
		/* Is this a map context ? */
		Context = &(Event_context_stack[i]);
		if (Context->Source == MAP_EVENT_SOURCE)
		{
			/* Yes -> Use the coordinates instead, as it is likely that the
			 player is now in a chest or door screen. */
			X = Context->Source_data.Map_source.X;
			Y = Context->Source_data.Map_source.Y;
			break;
		}
	}

	/* Get current event action data */
	Event_action = (struct Event_action *) &(Event_action_stack[Event_action_stack_index]);

	/* Get pointer to map data & event blocks */
	Map_ptr = MEM_Claim_pointer(Map_handle);
	Base = (struct Event_block *) (Map_ptr + Event_data_offset);

	/* Search NPCs for events */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (NPC_present(i))
		{
			/* Yes -> Right coordinates / right trigger mode / has event ? */
			if ((VNPCs[i].Map_X == X) && (VNPCs[i].Map_Y == Y) &&
			 (VNPCs[i].Trigger_modes & (1 << ACTION_TRIGGER)) &&
			 (VNPCs[i].First_block_nr != 0xFFFF))
			{
				/* Yes -> Get event chain index */
				j = VNPCs[i].First_block_nr;
				Chain_nr = Get_event_chain_nr(j);

				/* Has this event been saved ? */
				if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
				{
					/* No -> Get event data */
					Block = &(Base[j]);

					/* Is action event ? */
					if (Block->Type != ACTION_EVENT)
						continue;

					/* Right action type ? */
					if (Block->Byte_1 != Event_action->Action_type)
						continue;

					/* Yes -> Right extra value ? */
					if (Block->Byte_3 != Event_action->Action_extra)
					{
						/* No -> Any extra value ? */
						if (Block->Byte_3 != ACTION_ANY_VALUE_B)
							continue;

						/* No -> Right value ? */
						if (Block->Word_6 == Event_action->Action_value)
						{
							/* Yes -> Store, but search onward */
							First_block_nr = j;
							Found_block = Block;
						}
						else
						{
							/* No -> Any value ? */
							if (Block->Word_6 == ACTION_ANY_VALUE_W)
							{
								/* Yes -> Already found a block ? */
								if (Found_block)
								{
									/* Yes -> Is that one more specific than this one ? */
									if ((Found_block->Byte_3 != ACTION_ANY_VALUE_B) ||
									 (Found_block->Word_6 != ACTION_ANY_VALUE_W))
										continue;
								}

								/* No -> Store, but search onward */
								First_block_nr = j;
								Found_block = Block;
							}
							else
								continue;
						}
					}
					else
					{
						/* Yes -> Right value ? */
						if (Block->Word_6 == Event_action->Action_value)
						{
							/* Yes -> Found it */
							First_block_nr = j;
							X = 0;
							Y = 0;
							break;
						}
						else
						{
							/* No -> Any value ? */
							if (Block->Word_6 == ACTION_ANY_VALUE_W)
							{
								/* No -> Store, but search onward */
								First_block_nr = j;
								Found_block = Block;
							}
						}
					}
				}
			}
		}
	}

	/* Found something ? */
	if (First_block_nr == 0xFFFF)
	{
		/* No -> Get pointer to event entry data */
		Ptr = Map_ptr + Event_entry_offset;

		/* Find the event entry-list for the desired Y-coordinate */
		for (i=0;i<=Y;i++)
		{
			Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
		}
		Nr_entries = *((UNSHORT *) Ptr);
		Ptr += 2;
		Entry_data = (struct Map_event_entry *) Ptr;

		/* Search the event entry with the desired X-coordinate */
		for (i=0;i<Nr_entries;i++)
		{
			/* Right X-coordinate / right trigger mode / not deleted ? */
			if ((Entry_data[i].X == X) &&
			 (Entry_data[i].Trigger_modes & (1 << ACTION_TRIGGER)) &&
			 (Entry_data[i].First_block_nr != 0xFFFF))
			{
				/* Yes -> Get event chain index */
				j = Entry_data[i].First_block_nr;
				Chain_nr = Get_event_chain_nr(j);

				/* Has this event been saved ? */
				if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
				{
					/* Yes -> Get event data */
					Block = &(Base[j]);

					/* Is action event ? */
					if (Block->Type != ACTION_EVENT)
						continue;

					/* Right action type ? */
					if (Block->Byte_1 != Event_action->Action_type)
						continue;

					/* Yes -> Right extra value ? */
					if (Block->Byte_3 != Event_action->Action_extra)
					{
						/* No -> Any extra value ? */
						if (Block->Byte_3 != ACTION_ANY_VALUE_B)
							continue;

						/* No -> Right value ? */
						if (Block->Word_6 == Event_action->Action_value)
						{
							/* Yes -> Store, but search onward */
							First_block_nr = j;
							Found_block = Block;
						}
						else
						{
							/* No -> Any value ? */
							if (Block->Word_6 == ACTION_ANY_VALUE_W)
							{
								/* Yes -> Already found a block ? */
								if (Found_block)
								{
									/* Yes -> Is that one more specific than this one ? */
									if ((Found_block->Byte_3 != ACTION_ANY_VALUE_B) ||
										(Found_block->Word_6 != ACTION_ANY_VALUE_W))
										continue;
								}

								/* No -> Store, but search onward */
								First_block_nr = j;
								Found_block = Block;
							}
							else
								continue;
						}
					}
					else
					{
						/* Yes -> Right value ? */
						if (Block->Word_6 == Event_action->Action_value)
						{
							/* Yes -> Found it */
							First_block_nr = j;
							break;
						}
						else
						{
							/* No -> Any value ? */
							if (Block->Word_6 == ACTION_ANY_VALUE_W)
							{
								/* No -> Store, but search onward */
								First_block_nr = j;
								Found_block = Block;
							}
						}
					}
				}
			}
		}

		/* Found something ? */
		if (First_block_nr == 0xFFFF)
		{
			/* No -> Get pointer to event entry data */
			Ptr = Map_ptr + Event_entry_offset;

			/* Search the global events */
			Nr_entries = *((UNSHORT *) Ptr);
			Ptr += 2;
			Entry_data = (struct Map_event_entry *) Ptr;

			for (i=0;i<Nr_entries;i++)
			{
				/* Right trigger mode ? */
				if (Entry_data[i].Trigger_modes & (1 << ACTION_TRIGGER))
				{
					/* Yes -> Get event chain index */
					j = Entry_data[i].First_block_nr;
					Chain_nr = Get_event_chain_nr(j);

					/* Has this event been saved ? */
					if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
					{
						/* No -> Get event data */
						Block = &(Base[j]);

						/* Is action event ? */
						if (Block->Type != ACTION_EVENT)
							continue;

						/* Right action type ? */
						if (Block->Byte_1 != Event_action->Action_type)
							continue;

						/* Yes -> Right extra value ? */
						if (Block->Byte_3 != Event_action->Action_extra)
						{
							/* No -> Any extra value ? */
							if (Block->Byte_3 != ACTION_ANY_VALUE_B)
								continue;

							/* No -> Right value ? */
							if (Block->Word_6 == Event_action->Action_value)
							{
								/* Yes -> Store, but search onward */
								First_block_nr = j;
								Found_block = Block;
							}
							else
							{
								/* No -> Any value ? */
								if (Block->Word_6 == ACTION_ANY_VALUE_W)
								{
									/* Yes -> Already found a block ? */
									if (Found_block)
									{
										/* Yes -> Is that one more specific than this
										 one ? */
										if ((Found_block->Byte_3 != ACTION_ANY_VALUE_B) ||
										 (Found_block->Word_6 != ACTION_ANY_VALUE_W))
											continue;
									}

									/* No -> Store, but search onward */
									First_block_nr = j;
									Found_block = Block;
								}
								else
									continue;
							}
						}
						else
						{
							/* Yes -> Right value ? */
							if (Block->Word_6 == Event_action->Action_value)
							{
								/* Yes -> Found it */
								First_block_nr = j;
								X = 0;
								Y = 0;
								break;
							}
							else
							{
								/* No -> Any value ? */
								if (Block->Word_6 == ACTION_ANY_VALUE_W)
								{
									/* No -> Store, but search onward */
									First_block_nr = j;
									Found_block = Block;
								}
							}
						}
					}
				}
			}
		}
	}

	/* Get event chain index */
	Chain_nr = Get_event_chain_nr(First_block_nr);

	MEM_Free_pointer(Map_handle);

	/* Exit if nothing was found */
	if (First_block_nr == 0xFFFF)
		return;

	/* Set flag */
	Event_action->Flags |= FOUND_CHAIN;

	/* Make a new context */
	Context = Push_event_context();

	/* Initialize context */
	Context->Source 			= MAP_EVENT_SOURCE;
	Context->Set_handle 		= Map_handle;
	Context->Text_handle		= Map_text_handle;
	Context->Chain_nr 		= Chain_nr;
	Context->Chain_block_nr = First_block_nr;
	Context->Block_nr 		= First_block_nr;
	Context->Max_blocks 		= Nr_event_blocks;
	Context->Base 				= Event_data_offset;
	Context->Validator 		= Map_context_validator;

	Context->Source_data.Map_source.Map_nr 	= PARTY_DATA.Map_nr;
	Context->Source_data.Map_source.Trigger	= ACTION_TRIGGER;
	Context->Source_data.Map_source.Value 		= 0;
	Context->Source_data.Map_source.X 			= X;
	Context->Source_data.Map_source.Y 			= Y;

	/* Execute it */
	Execute_event_chain();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_item_set
 * FUNCTION  : Handle the item event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.11.94 11:55
 * LAST      : 30.07.95 19:57
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_item_set(UNSHORT Value)
{
	struct Event_context *Context;
	UNSHORT i;

	/* Search item event set */
	i = Search_event_set(Item_event_set_handle, 0xFFFF);

	/* Found anything ? */
	if (i != 0xFFFF)
	{
		/* Yes -> Make a new context */
		Context = Push_event_context();

		/* Prepare it */
		Prepare_event_set_context(Item_event_set_handle, i, Context, NULL);

		Context->Text_handle = Item_event_text_handle;

		Context->Source_data.Set_source.Char_type			= 0xFFFF;
		Context->Source_data.Set_source.Char_index		= 0;
		Context->Source_data.Set_source.Char_set_index	= 0;
		Context->Source_data.Set_source.Set_index			= ITEM_EVENT_SET;

		/* Execute it */
		Execute_event_chain();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_magic_set
 * FUNCTION  : Handle the magic event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.11.94 11:54
 * LAST      : 30.07.95 19:57
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_magic_set(UNSHORT Value)
{
	struct Event_context *Context;
	UNSHORT i;

	/* Search magic event set */
	i = Search_event_set(Magic_event_set_handle, 0xFFFF);

	/* Found anything ? */
	if (i != 0xFFFF)
	{
		/* Yes -> Make a new context */
		Context = Push_event_context();

		/* Prepare it */
		Prepare_event_set_context(Magic_event_set_handle, i, Context, NULL);

		Context->Text_handle = Magic_event_text_handle;

		Context->Source_data.Set_source.Char_type			= 0xFFFF;
		Context->Source_data.Set_source.Char_index		= 0;
		Context->Source_data.Set_source.Char_set_index	= 0;
		Context->Source_data.Set_source.Set_index			= MAGIC_EVENT_SET;

		/* Execute it */
		Execute_event_chain();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_dialogue_set
 * FUNCTION  : Handle the dialogue partner's event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.11.94 12:00
 * LAST      : 31.07.95 10:35
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_dialogue_set(UNSHORT Value)
{
	struct Event_context *Context;
	UNSHORT i, j;

	/* Is the dialogue character capable of acting ? */
	if ((Dialogue_type != RIDDLEMOUTH_DIALOGUE) &&
	 !(Character_capable(Dialogue_char_handle)))
		return;

	/* Search two sets */
	for (j=0;j<2;j++)
	{
		/* Search event set */
		i = Search_event_set(Dialogue_event_set_handles[j], ME_ACTOR_FLAG);

		/* Found anything ? */
		if (i != 0xFFFF)
		{
			/* Yes -> Set special flag to indicate the dialogue partner
			 reacted */
			Dialogue_set_reacted = TRUE;

			/* Make a new context */
			Context = Push_event_context();

			/* Prepare it */
			Prepare_event_set_context(Dialogue_event_set_handles[j], i, Context,
			 Dialogue_context_validator);

			Context->Text_handle = Dialogue_event_text_handles[j];

			Context->Source_data.Set_source.Char_type			= Dialogue_char_type;
			Context->Source_data.Set_source.Char_index		= Dialogue_char_index;
			Context->Source_data.Set_source.Char_set_index	= j;
			Context->Source_data.Set_source.Set_index			=
			 Get_event_set_index(Dialogue_char_handle, j);

	 		/* Execute it */
			Execute_event_chain();
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_acting_member_set
 * FUNCTION  : Handle the acting member's event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 12:21
 * LAST      : 30.12.94 12:21
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_acting_member_set(UNSHORT Value)
{
	/* Is the active party member capable of acting ? */
	if (Character_capable(Active_char_handle))
	{
		/* Yes -> Search event sets of active member */
		Search_member_sets(PARTY_DATA.Active_member, ME_ACTOR_FLAG);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_non_acting_member_set
 * FUNCTION  : Handle a non-acting member's event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 12:21
 * LAST      : 30.12.94 12:21
 * INPUTS    : UNSHORT Value - Member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume it is not being called in combat.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_non_acting_member_set(UNSHORT Value)
{
	struct Event_action *Event_action;

	/* Exit if this member does not exist */
	if (!PARTY_DATA.Member_nrs[Value - 1])
		return;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Did this party member act ? */
	if (Event_action->Actor_index != Value)
	{
		/* No -> Are we talking to this party member ? */
		if ((!In_Dialogue) || (Dialogue_char_type != PARTY_CHAR_TYPE) ||
		 (Dialogue_char_index != PARTY_DATA.Member_nrs[Value-1]))
		{
			/* No -> Is this party member capable of acting ? */
			if (Character_capable(Party_char_handles[Value-1]))
			{
				/* Yes -> Search this member's set for friend actions */
				Search_member_sets(Value, FRIEND_ACTOR_FLAG);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_acting_part_set
 * FUNCTION  : Handle the acting participant's event set.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.01.95 12:14
 * LAST      : 25.01.95 12:14
 * INPUTS    : UNSHORT Value - Can be ignored for this set.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_acting_part_set(UNSHORT Value)
{
	struct Event_action *Event_action;
	UNSHORT Index;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Get the index of the current actor */
	Index = Event_action->Actor_index;

	/* What type of actor ? */
	switch (Event_action->Actor_type)
	{
		/* Party member */
		case PARTY_ACTOR_TYPE:
		{
			/* Is the acting party member capable of acting ? */
			if (Character_capable(Party_char_handles[Index-1]))
			{
				/* Yes -> Search the acting member's set */
				Search_member_sets(Index, ME_ACTOR_FLAG);
			}
			break;
		}
		/* Monster */
		case MONSTER_ACTOR_TYPE:
		{
			/* Is the acting monster capable of acting ? */
			if (Character_capable(Party_char_handles[Index-1]))
			{
				/* Yes -> Search the acting member's set */
				Search_monster_sets(Index, ME_ACTOR_FLAG);
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_friend_part_set
 * FUNCTION  : Handle the event set of a friend of the acting participant.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.01.95 12:16
 * LAST      : 25.01.95 12:16
 * INPUTS    : UNSHORT Value - Participant index (1...18).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_friend_part_set(UNSHORT Value)
{
	struct Event_action *Event_action;
	UNSHORT Index;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Get the index of the current actor */
	Index = Event_action->Actor_index;

	/* What type of actor ? */
	switch (Event_action->Actor_type)
	{
		/* Party member */
		case PARTY_ACTOR_TYPE:
		{
			/* Is this a monster index ? */
			if (Value <= 6)
			{
				/* No -> Does this member exist ? */
				if (PARTY_DATA.Member_nrs[Value - 1])
				{
					/* Yes -> Is this the acting member ? */
					if (Value != Index)
					{
						/* No -> Is this party member capable of acting ? */
						if (Character_capable(Party_char_handles[Value - 1]))
						{
							/* Yes -> Search this member's set for friend events */
							Search_member_sets(Value, FRIEND_ACTOR_FLAG);
						}
					}
				}
			}
			break;
		}
		/* Monster */
		case MONSTER_ACTOR_TYPE:
		{
			/* Does this monster exist ? */
			if (PARTY_DATA.Member_nrs[Value - 1])
			{
				/* Yes -> Is this the acting monster ? */
				if (Value != Index)
				{
					/* No -> Is this monster capable of acting ? */
					if (Character_capable(Party_char_handles[Value - 1]))
					{
						/* Yes -> Search this monster's set for friend events */
						Search_monster_sets(Value, FRIEND_ACTOR_FLAG);
					}
				}
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_enemy_part_set
 * FUNCTION  : Handle the event set of an enemy of the acting participant.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.01.95 12:16
 * LAST      : 26.01.95 13:37
 * INPUTS    : UNSHORT Value - Participant index (1...18).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_enemy_part_set(UNSHORT Value)
{
	struct Event_action *Event_action;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* What type of actor ? */
	switch (Event_action->Actor_type)
	{
		/* Party member -> Check monster */
		case PARTY_ACTOR_TYPE:
		{
			/* Does this monster exist ? */
			if (PARTY_DATA.Member_nrs[Value - 1])
			{
				/* Yes -> Is this monster capable of acting ? */
				if (Character_capable(Party_char_handles[Value - 1]))
				{
					/* Yes -> Search this monster's set for enemy events */
					Search_monster_sets(Value, ENEMY_ACTOR_FLAG);
				}
			}
			break;
		}
		/* Monster -> Check party member */
		case MONSTER_ACTOR_TYPE:
		{
			/* Is this a monster index ? */
			if (Value <= 6)
			{
				/* No -> Does this member exist ? */
				if (PARTY_DATA.Member_nrs[Value - 1])
				{
					/* Yes -> Is this party member capable of acting ? */
					if (Character_capable(Party_char_handles[Value - 1]))
					{
						/* Yes -> Search this member's set for enemy events */
						Search_member_sets(Value, ENEMY_ACTOR_FLAG);
					}
				}
			}
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_member_sets
 * FUNCTION  : Search a party member's event sets.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 12:25
 * LAST      : 31.07.95 10:35
 * INPUTS    : UNSHORT Member_nr - Member index (1...6).
 *             UNSHORT Actor_type - Actor type flag (0xFFFF = doesn't matter).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the action does not have an actor, this function will
 *              automatically set the actor type to 0xFFFF.
 *             - This function assumes the party member with this index exists.
 * SEE ALSO  : EVELOGIC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Search_member_sets(UNSHORT Member_nr, UNSHORT Actor_type)
{
	struct Event_action *Event_action;
	struct Event_context *Context;
	UNSHORT i, j;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Was there an actor ? */
	if (Event_action->Actor_type == NO_ACTOR_TYPE)
	{
		/* No -> Actor type doesn't matter */
		Actor_type = 0xFFFF;
	}

	/* Search two sets */
	for (j=0;j<2;j++)
	{
		/* Search event set */
		i = Search_event_set(Party_event_set_handles[Member_nr-1][j],
		 Actor_type);

		/* Found anything ? */
		if (i != 0xFFFF)
		{
			/* Yes -> Make a new context */
			Context = Push_event_context();

			/* Prepare it */
			Prepare_event_set_context(Party_event_set_handles[Member_nr-1][j],
			 i, Context, Party_member_context_validator);

			Context->Text_handle = Party_event_text_handles[Member_nr-1][j];

			Context->Source_data.Set_source.Char_type			= PARTY_CHAR_TYPE;
			Context->Source_data.Set_source.Char_index		= PARTY_DATA.Member_nrs[Member_nr - 1];
			Context->Source_data.Set_source.Char_set_index	= j;
			Context->Source_data.Set_source.Set_index			=
			 Get_event_set_index(Party_char_handles[Member_nr-1], j);

			/* Execute it */
			Execute_event_chain();
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_monster_sets
 * FUNCTION  : Search a monster's event sets.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.01.95 13:19
 * LAST      : 26.01.95 13:19
 * INPUTS    : UNSHORT Monster_nr - Monster index (1...18).
 *             UNSHORT Actor_type - Actor type flag (0xFFFF = doesn't matter).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the action does not have an actor, this function will
 *              automatically set the actor type to 0xFFFF.
 *             - This function assumes the monster with this index exists.
 * SEE ALSO  : EVELOGIC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Search_monster_sets(UNSHORT Monster_nr, UNSHORT Actor_type)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_event_set_context
 * FUNCTION  : Prepare an event set context.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.10.94 15:42
 * LAST      : 30.07.95 19:51
 * INPUTS    : MEM_HANDLE Set_handle - Event set memory handle.
 *             UNSHORT Chain_nr - Event chain number.
 *             struct Event_context *Context - Pointer to new event context.
 *             Context_validator Validator - Pointer to context validator.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will also set a flag in the current Event
 *              Action data to indicate that a chain was found.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Prepare_event_set_context(MEM_HANDLE Set_handle, UNSHORT Chain_nr,
 struct Event_context *Context, Context_validator Validator)
{
	struct Event_action *Event_action;
	struct Standard_event_set *Set;
	UNSHORT *Ptr;
	UNSHORT Nr_chains;

	/* Get current event action data */
	Event_action = &(Event_action_stack[Event_action_stack_index]);

	/* Set flag */
	Event_action->Flags |= FOUND_CHAIN;

	/* Set data in context */
	Context->Source		= SET_EVENT_SOURCE;
	Context->Set_handle	= Set_handle;
	Context->Chain_nr		= Chain_nr;
	Context->Max_blocks	= 0xFFFF;
	Context->Validator	= Validator;

	/* Get event set data */
	Set = (struct Standard_event_set *) MEM_Claim_pointer(Set_handle);

	/* Get number of chains */
	Nr_chains = Set->Nr_chains;

	/* Calculate event base */
	Context->Base = sizeof(struct Standard_event_set) + (Nr_chains * 2);

	/* Get first block number */
	Ptr = (UNSHORT *) (Set + 1);
	Context->Block_nr			= Ptr[Chain_nr];
	Context->Chain_block_nr	= Context->Block_nr;

	MEM_Free_pointer(Set_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_event_set
 * FUNCTION  : Search an event set for a certain action event.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.10.94 15:32
 * LAST      : 14.07.95 10:49
 * INPUTS    : MEM_HANDLE Set_handle - Event set memory handle.
 *             UNSHORT Actor_type - Actor type flag (0xFFFF = doesn't matter).
 * RESULT    : UNSHORT : Chain number (0..., 0xFFFF = not found).
 * BUGS      : No known.
 * NOTES     : - This function will search the entire event set for the most
 *              specific event chain matching the input values.
 *             - It will use the action parameters from the current event
 *              action.
 *             - This will only work for "pure" event sets.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_event_set(MEM_HANDLE Set_handle, UNSHORT Actor_type)
{
	struct Event_action *Event_action;
	struct Standard_event_set *Set;
	struct Event_block *Base;
	struct Event_block *Ptr;
	struct Event_block *Found_block = NULL;
	UNSHORT Nr_chains;
	UNSHORT i;
	UNSHORT Found_chain = 0xFFFF;
	UNSHORT *Entry_list;

	/* Get current event action data */
	Event_action = (struct Event_action *)
	 &(Event_action_stack[Event_action_stack_index]);

	/* Any event set ? */
	if (Set_handle)
	{
		/* Yes -> Get event set data */
		Set = (struct Standard_event_set *) MEM_Claim_pointer(Set_handle);

		/* Get number of chains */
		Nr_chains = Set->Nr_chains;

		/* Get event chain entry list */
		Entry_list = (UNSHORT *) (Set + 1);

		/* Get first event block */
		Base = (struct Event_block *) (Entry_list + Nr_chains);

		/* Search all chains */
		for (i=0;i<Nr_chains;i++)
		{
			/* Find start of chain */
			Ptr = Base + Entry_list[i];

			/* Is action event ? */
			if (Ptr->Type != ACTION_EVENT)
				continue;

			/* Yes -> Are we looking for an action by a person ? */
			if (Actor_type != 0xFFFF)
			{
				/* Yes -> Right set type ? */
				if (!(Ptr->Byte_2 & (1 << Actor_type)))
					continue;
			}

			/* Right action type ? */
			if (Ptr->Byte_1 != Event_action->Action_type)
				continue;

			/* Yes -> Right extra value ? */
			if (Ptr->Byte_3 != Event_action->Action_extra)
			{
				/* No -> Any extra value ? */
				if (Ptr->Byte_3 != ACTION_ANY_VALUE_B)
					continue;

				/* No -> Right value ? */
				if (Ptr->Word_6 == Event_action->Action_value)
				{
					/* Yes -> Store, but search onward */
					Found_chain = i;
					Found_block = Ptr;
				}
				else
				{
					/* No -> Any value ? */
					if (Ptr->Word_6 == ACTION_ANY_VALUE_W)
					{
						/* Yes -> Already found a block ? */
						if (Found_block)
						{
							/* Yes -> Is that one more specific than this one ? */
							if ((Found_block->Byte_3 != ACTION_ANY_VALUE_B) ||
							 (Found_block->Word_6 != ACTION_ANY_VALUE_W))
								continue;
						}

						/* No -> Store, but search onward */
						Found_chain = i;
						Found_block = Ptr;
					}
					else
						continue;
				}
			}
			else
			{
				/* Yes -> Right value ? */
				if (Ptr->Word_6 == Event_action->Action_value)
				{
					/* Yes -> Found it */
					Found_chain = i;
					break;
				}
				else
				{
					/* No -> Any value ? */
					if (Ptr->Word_6 == ACTION_ANY_VALUE_W)
					{
						/* No -> Store, but search onward */
						Found_chain = i;
						Found_block = Ptr;
					}
				}
			}
		}

		MEM_Free_pointer(Set_handle);
	}

	return Found_chain;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_event_action
 * FUNCTION  :	Push a new event action on the stack.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 10:01
 * LAST      : 28.10.94 10:01
 * INPUTS    : None.
 * RESULT    : struct Event_action * : Pointer to new event action structure.
 * BUGS      : No known.
 * NOTES     : - The new event action structure will be cleared.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Event_action *
Push_event_action(void)
{
	UNBYTE *Ptr = NULL;

	/* Is there room on the stack ? */
	if (Event_action_stack_index < EVENT_ACTIONS_MAX)
	{
		/* Yes -> Increase stack index */
		Event_action_stack_index++;

		/* Clear new event action */
		Ptr = (UNBYTE *) &(Event_action_stack[Event_action_stack_index]);

		BASEMEM_FillMemByte(Ptr, sizeof(struct Event_action), 0);
	}

	return (struct Event_action *) Ptr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_event_action
 * FUNCTION  :	Pop an event action from the stack.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 10:08
 * LAST      : 28.10.94 10:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_event_action(void)
{
	/* Is the stack empty ? */
	if (Event_action_stack_index)
	{
		/* No -> Decrease stack index */
		Event_action_stack_index--;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_event_action
 * FUNCTION  :	Reset the event action stack.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 10:09
 * LAST      : 28.10.94 10:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_event_action(void)
{
	UNSHORT i;
	UNBYTE *Ptr;

	/* Reset stack */
	Event_action_stack_index = 0;

	/* Clear new event action data */
	Ptr = (UNBYTE *) &(Event_action_stack[Event_action_stack_index]);

	for (i=0;i<sizeof(struct Event_action);i++)
	{
		*(Ptr + i) = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_event_chain
 * FUNCTION  : Execute the current event chain (the one on the event context
 *              stack).
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 10:09
 * LAST      : 15.08.95 15:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes that the following entries of the
 *              current context have been set : Event_handle, Event_base and
 *              Event_block_nr.
 *             - The Event_data entry will be set by this routine.
 *             - The event context will automatically be popped once the
 *              chain has been executed. Pop_event_context requires the
 *              Context_validator entry for the previous event context.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_event_chain(void)
{
	struct Event_context *Context;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get first block number */
	i = Context->Block_nr;

	/* End of chain ? */
	if (i != 0xFFFF)
	{
		/* No -> Copy first event block */
		Ptr = MEM_Claim_pointer(Context->Set_handle) + Context->Base
		 + (i * sizeof(struct Event_block));
		memcpy(&(Context->Data), Ptr, sizeof(struct Event_block));
		MEM_Free_pointer(Context->Set_handle);

		/* Clear flags */
		Context->Flags = 0;

		for(;;)
		{
			if (Diagnostic_mode == 2)
				Display_event_data(Context);

			/* Get current event type */
			i = Context->Data.Type;

			/* Is legal ? */
			if (i && (i <= MAX_EVENT_TYPES))
			{
				/* Yes -> Execute */
				if (Event_handlers[i-1])
					Event_handlers[i-1]();
			}
			else
			{
				/* No -> Error */
				Event_error(ERROR_ILLEGAL_EVENT_TYPE);
				break;
			}

			/* Break event chain / exit program ? */
			if ((Context->Flags & BREAK_EVENT_CHAIN) || Quit_program)
			{
				/* Yes */
				break;
			}

			/* No -> Next event ? */
			if (!Chain_to_next_event())
			{
				/* No */
				break;
			}
		}
	}

	/* Pop context */
	Pop_event_context();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Chain_to_next_event
 * FUNCTION  : Follow the chain to the next event in the current event chain.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 15:45
 * LAST      : 28.10.94 15:45
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - There is a next event, FALSE - End of chain.
 * BUGS      : No known.
 * NOTES     : - This function re-sets [ Event_block_nr ] and [ Event_data ].
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Chain_to_next_event(void)
{
	struct Event_context *Context;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get next block number */
	i = Context->Data.Next_event_nr;

	/* Get current block number */
	j = Context->Block_nr;

	/* End of chain ? */
	if (i == 0xFFFF)
	{
		/* Yes */
		Context->Block_nr = 0xFFFF;

		/* Exit */
		return FALSE;
	}
	else
	{
		/* No -> */
/*		i += Context->Chain_block_nr; */

		/* Illegal block number ? */
		if (i >= Context->Max_blocks)
		{
			/* Yes -> Error */
			Event_error(ERROR_ILLEGAL_EVENT_BLOCK);
			return FALSE;
		}

		/* No -> Endless loop ? */
		if (i == j)
		{
			/* Yes -> Error */
			Event_error(ERROR_ENDLESS_LOOP);
			return FALSE;
		}

		/* No -> Store new block number */
		Context->Block_nr = i;

		/* Copy event block */
		Ptr = MEM_Claim_pointer(Context->Set_handle) + Context->Base
		 + (i * sizeof(struct Event_block));
		memcpy(&(Context->Data), Ptr, sizeof(struct Event_block));
		MEM_Free_pointer(Context->Set_handle);

		return TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_event_context
 * FUNCTION  :	Push a new event context on the stack.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 17:15
 * LAST      : 28.10.94 17:15
 * INPUTS    : None.
 * RESULT    : struct Event_context * : Pointer to new event context.
 * BUGS      : No known.
 * NOTES     : - The new event context will be cleared.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Event_context *
Push_event_context(void)
{
	/* Is there room on the stack ? */
	if (Event_context_stack_index < EVENT_CONTEXTS_MAX)
	{
		/* Yes -> Increase stack index */
		Event_context_stack_index++;

		/* Clear new event_context */
		BASEMEM_FillMemByte((UNBYTE *) &(Event_context_stack[Event_context_stack_index]),
		 sizeof(struct Event_context), 0);

		return &Event_context_stack[Event_context_stack_index];
	}
	else
		return NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_event_context
 * FUNCTION  :	Pop an event context from the stack.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 17:16
 * LAST      : 28.10.94 17:16
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_event_context(void)
{
	struct Event_context *Old;

	/* Is the stack empty ? */
	if (Event_context_stack_index)
	{
		/* No -> Decrease stack index */
		Event_context_stack_index--;

		/* Must the old context be validated ? */
		Old = &(Event_context_stack[Event_context_stack_index]);
		if (Old->Validator)
		{
			/* Yes -> Is it still valid ? */
			if (!(Old->Validator)(Old))
			{
				/* No -> Break */
				Old->Flags |= BREAK_EVENT_CHAIN;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_event_context
 * FUNCTION  :	Reset the event context stack.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.94 17:17
 * LAST      : 28.10.94 17:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_event_context(void)
{
	/* Reset stack */
	Event_context_stack_index = 0;

	/* Clear new event_context */
	BASEMEM_FillMemByte((UNBYTE *) &(Event_context_stack[0]),
	 sizeof(struct Event_context), 0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Break_current_context
 * FUNCTION  :	Break the current event context.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 10:42
 * LAST      : 30.12.94 10:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Break_current_context(void)
{
	struct Event_context *Context;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Set break flag */
	Context->Flags |= BREAK_EVENT_CHAIN;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_success_flag
 * FUNCTION  : Set the success flag for the current context.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:29
 * LAST      : 21.04.95 11:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_success_flag(void)
{
	struct Event_context *Context;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Set success flag */
	Context->Flags |= SUCCESS_FLAG;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_success_flag
 * FUNCTION  : Clear the success flag for the current context.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:30
 * LAST      : 21.04.95 11:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_success_flag(void)
{
	struct Event_context *Context;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Clear success flag */
	Context->Flags &= ~SUCCESS_FLAG;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_current_event
 * FUNCTION  : Save the current event chain.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 13:15
 * LAST      : 07.07.95 13:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_current_event(void)
{
	struct Event_context *Context;
	UNLONG Bit_nr;

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Current context is a map set ? */
	if (Context->Source == MAP_EVENT_SOURCE)
	{
		/* Yes ->  Calculate bit number */
		Bit_nr = ((Context->Source_data.Map_source.Map_nr - 1) *
		 EVENTS_PER_MAP) + Context->Chain_nr;

		/* Modify bit in event save bit array */
		Write_bit_array(EVENT_SAVE_BIT_ARRAY, Bit_nr, SET_BIT_ARRAY);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Event_is_saved
 * FUNCTION  : Check if an event is saved.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 13:15
 * LAST      : 07.07.95 13:15
 * INPUTS    : UNSHORT Map_nr - Map number.
 *             UNSHORT Chain_nr - Chain number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only works for map event chains.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Event_is_saved(UNSHORT Map_nr, UNSHORT Chain_nr)
{
	UNLONG Bit_nr;

	/* Build bit number */
	Bit_nr = ((Map_nr - 1) * EVENTS_PER_MAP) + Chain_nr;

	/* Has this event been saved ? */
	return (Read_bit_array(EVENT_SAVE_BIT_ARRAY, Bit_nr));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Trigger_map_event_chain
 * FUNCTION  :	Try to trigger an event chain in the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.12.94 11:52
 * LAST      : 10.08.95 18:00
 * INPUTS    : UNSHORT X - Map X-coordinate (0 for global event).
 *             UNSHORT Y - Map Y-coordinate (0 for global event).
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 *             UNSHORT Trigger_mode - Trigger mode.
 *             UNSHORT Value - Extra value needed for some triggermodes.
 * RESULT    : BOOLEAN : TRUE (something happened) or FALSE (nothing happened).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Trigger_map_event_chain(UNSHORT X, UNSHORT Y, UNSHORT NPC_index,
 UNSHORT Trigger_mode, UNSHORT Value)
{
	struct Event_context *Context;
	UNSHORT First_block_nr;

	/* Search chain */
	First_block_nr = Search_triggered_event(X, Y, NPC_index, Trigger_mode);

	/* Exit if nothing was found */
	if (First_block_nr == 0xFFFF)
		return FALSE;

	/* Make a new context */
	Context = Push_event_context();

	/* Initialize context */
	Context->Source			= MAP_EVENT_SOURCE;
	Context->Set_handle		= Map_handle;
	Context->Text_handle		= Map_text_handle;
	Context->Chain_nr			= Get_event_chain_nr(First_block_nr);
	Context->Chain_block_nr	= First_block_nr;
	Context->Block_nr			= First_block_nr;
	Context->Max_blocks		= Nr_event_blocks;
	Context->Base				= Event_data_offset;
	Context->Validator		= Map_context_validator;

	Context->Source_data.Map_source.Map_nr		= PARTY_DATA.Map_nr;
	Context->Source_data.Map_source.Trigger	= Trigger_mode;
	Context->Source_data.Map_source.Value		= Value;
	Context->Source_data.Map_source.X			= X;
	Context->Source_data.Map_source.Y			= Y;

	/* Execute it */
	Execute_event_chain();

	/* Exit */
	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Trigger_NPC_event_chain
 * FUNCTION  :	Try to trigger an event chain for an NPC in the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.04.95 18:10
 * LAST      : 29.07.95 19:47
 * INPUTS    : UNSHORT NPC_index - Index of NPC.
 *             UNSHORT Trigger_mode - Trigger mode.
 *             UNSHORT Value - Extra value needed for some triggermodes.
 * RESULT    : BOOLEAN : TRUE (something happened) or FALSE (nothing happened).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Trigger_NPC_event_chain(UNSHORT NPC_index, UNSHORT Trigger_mode,
 UNSHORT Value)
{
	struct Event_context *Context;
	BOOLEAN Result = FALSE;
	UNSHORT First_block_nr;

	/* Get first block number */
	First_block_nr = VNPCs[NPC_index].First_block_nr;

	/* Does this NPC have an event ? */
	if (First_block_nr != 0xFFFF)
	{
		/* Yes -> Does this NPC have the right trigger-mode ? */
		if (VNPCs[NPC_index].Trigger_modes & (1 << Trigger_mode))
		{
			/* Yes -> Make a new context */
			Context = Push_event_context();

			/* Initialize context */
			Context->Source			= MAP_EVENT_SOURCE;
			Context->Set_handle		= Map_handle;
			Context->Text_handle		= Map_text_handle;
			Context->Chain_nr			= Get_event_chain_nr(First_block_nr);
			Context->Chain_block_nr	= First_block_nr;
			Context->Block_nr			= First_block_nr;
			Context->Max_blocks		= Nr_event_blocks;
			Context->Base				= Event_data_offset;
			Context->Validator		= Map_context_validator;

			Context->Source_data.Map_source.Map_nr		= PARTY_DATA.Map_nr;
			Context->Source_data.Map_source.Trigger	= Trigger_mode;
			Context->Source_data.Map_source.Value		= Value;
			Context->Source_data.Map_source.X			= VNPCs[NPC_index].Map_X;
			Context->Source_data.Map_source.Y			= VNPCs[NPC_index].Map_Y;

			/* Execute it */
			Execute_event_chain();

			/* Indicate success */
			Result = TRUE;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_triggered_event
 * FUNCTION  :	Search for an event chain in the current map with a
 *              certain trigger mode.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 10:45
 * LAST      : 29.07.95 19:27
 * INPUTS    : UNSHORT X - Map X-coordinate (0 for global event).
 *             UNSHORT Y - Map Y-coordinate (0 for global event).
 *             UNSHORT NPC_index - Index of NPC (0...) / 0xFFFF for party.
 *             UNSHORT Trigger_mode - Trigger mode.
 * RESULT    : UNSHORT : First event block index (0...) /
 *              0xFFFF means nothing was found.
 * BUGS      : No known.
 * NOTES     : - This function will fail if the number of NPCs per map grows
 *              larger than 32.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_triggered_event(UNSHORT X, UNSHORT Y, UNSHORT NPC_index,
 UNSHORT Trigger_mode)
{
	UNSHORT Value;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0xFFFF;

	/* Search NPC events */
	Value = Search_triggered_NPC_event(X, Y, NPC_index, Trigger_mode);

	/* Found something ? */
	if (Value == 0xFFFF)
	{
		/* No -> Search map events */
		Value = Search_triggered_map_event(X, Y, Trigger_mode);
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_triggered_NPC_event
 * FUNCTION  :	Search for an event chain in the current map with a
 *              certain trigger mode.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 10:45
 * LAST      : 29.07.95 19:47
 * INPUTS    : UNSHORT X - Map X-coordinate (0 for global event).
 *             UNSHORT Y - Map Y-coordinate (0 for global event).
 *             UNSHORT NPC_index - Index of NPC (0...) / 0xFFFF for party.
 *             UNSHORT Trigger_mode - Trigger mode.
 * RESULT    : UNSHORT : First event block index (0...) /
 *              0xFFFF means nothing was found.
 * BUGS      : No known.
 * NOTES     : - Event chains deleted by Icon Changes will automatically
 *              return 0xFFFF.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_triggered_NPC_event(UNSHORT X, UNSHORT Y, UNSHORT NPC_index,
 UNSHORT Trigger_mode)
{
	UNSHORT Chain_nr;
	UNSHORT First_block_nr = 0xFFFF;
	UNSHORT i;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0xFFFF;

	/* Search NPCs for events */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself ? */
		if ((NPC_present(i)) && (i != NPC_index))
		{
			/* Yes -> Right coordinates / right trigger mode / has event ? */
			if ((VNPCs[i].Map_X == X) &&
			 (VNPCs[i].Map_Y == Y) &&
			 (VNPCs[i].Trigger_modes & (1 << Trigger_mode)) &&
			 (VNPCs[i].First_block_nr != 0xFFFF))
			{
				/* Yes -> Get event chain index */
				First_block_nr = VNPCs[i].First_block_nr;
				Chain_nr = Get_event_chain_nr(First_block_nr);

				/* Has this event been saved ? */
				if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
				{
					/* No -> Found a chain */
					break;
				}
				else
				{
					/* Yes -> Keep searching */
					First_block_nr = 0xFFFF;
				}
			}
		}
	}

	return First_block_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_triggered_map_event
 * FUNCTION  :	Search for an event chain in the current map with a
 *              certain trigger mode.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.04.95 18:12
 * LAST      : 29.07.95 19:48
 * INPUTS    : UNSHORT X - Map X-coordinate (0 for global event).
 *             UNSHORT Y - Map Y-coordinate (0 for global event).
 *             UNSHORT Trigger_mode - Trigger mode.
 * RESULT    : UNSHORT : First event block index (0...) /
 *              0xFFFF means nothing was found.
 * BUGS      : No known.
 * NOTES     : - Event chains deleted by Icon Changes will automatically
 *              return 0xFFFF.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_triggered_map_event(UNSHORT X, UNSHORT Y, UNSHORT Trigger_mode)
{
	struct Map_event_entry *Entry_data;
	UNSHORT Nr_entries = 0;
	UNSHORT Chain_nr;
	UNSHORT First_block_nr = 0xFFFF;
	UNSHORT i;
	UNBYTE *Map_ptr;
	UNBYTE *Ptr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0xFFFF;

	/* Get pointer to map data */
	Map_ptr = MEM_Claim_pointer(Map_handle);

	/* Get pointer to event entry data */
	Ptr = Map_ptr + Event_entry_offset;

	/* Find the event entry-list for the desired Y-coordinate */
	for (i=0;i<Y;i++)
	{
		Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
	}
	Nr_entries = *((UNSHORT *) Ptr);
	Ptr += 2;
	Entry_data = (struct Map_event_entry *) Ptr;

	/* Global event ? */
	if (X || Y)
	{
		/* No -> Search the event entry with the desired X-coordinate */
		for (i=0;i<Nr_entries;i++)
		{
			/* Right X-coordinate / right trigger mode ? */
			if ((Entry_data[i].X == X) &&
			 (Entry_data[i].Trigger_modes & (1 << Trigger_mode)))
			{
				/* Yes -> Get event chain index */
				First_block_nr = Entry_data[i].First_block_nr;
				Chain_nr = Get_event_chain_nr(First_block_nr);

				/* Has this event been saved ? */
				if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
				{
					/* No -> Found a chain */
					break;
				}
				else
				{
					/* Yes -> Keep searching */
					First_block_nr = 0xFFFF;
				}
			}
		}
	}
	else
	{
		/* Yes -> Search the event entry with the desired trigger-mode */
		for (i=0;i<Nr_entries;i++)
		{
			/* Right trigger mode ? */
			if (Entry_data[i].Trigger_modes & (1 << Trigger_mode))
			{
				/* Yes -> Get event chain index */
				First_block_nr = Entry_data[i].First_block_nr;
				Chain_nr = Get_event_chain_nr(First_block_nr);

				/* Has this event been saved ? */
				if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
				{
					/* No -> Found a chain */
					break;
				}
				else
				{
					/* Yes -> Keep searching */
					First_block_nr = 0xFFFF;
				}
			}
		}
	}
	MEM_Free_pointer(Map_handle);

	return First_block_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event_triggers
 * FUNCTION  :	Get the event triggers of a position in the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.04.95 17:07
 * LAST      : 04.07.95 12:26
 * INPUTS    : UNSHORT X - Map X-coordinate (1...).
 *             UNSHORT Y - Map Y-coordinate (1...).
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : UNSHORT : List of trigger modes.
 * BUGS      : No known.
 * NOTES     : - Getting event triggers of global events is meaningless.
 *             - This function will fail if the number of NPCs per map grows
 *              larger than 32.
 *             - This function will examine the first event block in any
 *              found chain so that Offset events will be correctly handled.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_event_triggers(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	UNSHORT Value;

	/* Exit if the map isn't initialized, or if coordinates are zero */
	if (!Map_initialized || !X || !Y)
		return 0;

	/* Check NPC events */
	Value = Get_NPC_event_triggers(X, Y, NPC_index);

	/* Found something ? */
	if (!Value)
	{
		/* No -> Check map events */
		Value = Get_map_event_triggers(X, Y);
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_NPC_event_triggers
 * FUNCTION  :	Get the event triggers of NPC's for a position in the current
 *              map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.04.95 16:59
 * LAST      : 29.07.95 20:01
 * INPUTS    : UNSHORT X - Map X-coordinate (1...).
 *             UNSHORT Y - Map Y-coordinate (1...).
 *             UNSHORT NPC_index - Index of NPC (0...) / 0xFFFF for party.
 * RESULT    : UNSHORT : List of trigger modes.
 * BUGS      : No known.
 * NOTES     : - This function will examine the first event block in any
 *              found chain so that Offset events will be correctly handled.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_NPC_event_triggers(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	struct Event_block *Block;
	UNSHORT Value = 0;
	UNSHORT Chain_nr;
	UNSHORT First_block_nr = 0;
	UNSHORT i;
	UNBYTE *Map_ptr;

	/* Exit if the map isn't initialized, or if coordinates are zero */
	if (!Map_initialized || !X || !Y)
		return 0;

	/* Search NPCs for events */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself ? */
		if ((NPC_present(i)) && (i != NPC_index))
		{
			/* Yes -> Right coordinates / has event ? */
			if ((VNPCs[i].Map_X == X) && (VNPCs[i].Map_Y == Y) &&
			 (VNPCs[i].First_block_nr != 0xFFFF))
			{
				/* Yes -> Get event chain index */
				First_block_nr = VNPCs[i].First_block_nr;
				Chain_nr = Get_event_chain_nr(First_block_nr);

				/* Has this event been saved ? */
				if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
				{
					/* No -> Found a chain */
					Value = VNPCs[i].Trigger_modes;
					break;
				}
			}
		}
	}

	/* Found something ? */
	if (Value)
	{
		/* Yes -> Examine the first event block in the chain */
		Map_ptr = MEM_Claim_pointer(Map_handle);

		Block = (struct Event_block *) (Map_ptr + Event_data_offset +
		 (First_block_nr * sizeof(struct Event_block)));

		/* Is this an Offset event ? */
		if (Block->Type == OFFSET_EVENT)
		{
			/* Yes -> Relative coordinates ? */
			if (Block->Byte_3 & 0x01)
			{
				/* Yes -> Add relative coordinates */
				X += (SISHORT) ((SIBYTE) Block->Byte_1);
				Y += (SISHORT) ((SIBYTE) Block->Byte_2);
			}
			else
			{
				/* No -> Get absolute coordinates */
				X = (UNSHORT) Block->Byte_1;
				Y = (UNSHORT) Block->Byte_2;
			}

			/* Recurse */
			Value = Get_NPC_event_triggers(X, Y, NPC_index);
		}

		MEM_Free_pointer(Map_handle);
	}

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_map_event_triggers
 * FUNCTION  :	Get the event triggers of a position in the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.04.95 17:01
 * LAST      : 29.07.95 19:50
 * INPUTS    : UNSHORT X - Map X-coordinate (1...).
 *             UNSHORT Y - Map Y-coordinate (1...).
 * RESULT    : UNSHORT : List of trigger modes.
 * BUGS      : No known.
 * NOTES     : - Getting event triggers of global events is meaningless.
 *             - This function will examine the first event block in any
 *              found chain so that Offset events will be correctly handled.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_map_event_triggers(UNSHORT X, UNSHORT Y)
{
	struct Map_event_entry *Entry_data;
	struct Event_block *Block;
	UNSHORT Nr_entries = 0;
	UNSHORT Value = 0;
	UNSHORT Chain_nr;
	UNSHORT First_block_nr = 0;
	UNSHORT i;
	UNBYTE *Map_ptr;
	UNBYTE *Ptr;

	/* Exit if the map isn't initialized, or if coordinates are zero */
	if (!Map_initialized || !X || !Y)
		return 0;

	/* Get pointer to map data */
	Map_ptr = MEM_Claim_pointer(Map_handle);

	/* Get pointer to event entry data */
	Ptr = Map_ptr + Event_entry_offset;

	/* Find the event entry-list for the desired Y-coordinate */
	for (i=0;i<Y;i++)
	{
		Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
	}
	Nr_entries = *((UNSHORT *) Ptr);
	Ptr += 2;
	Entry_data = (struct Map_event_entry *) Ptr;

	/* Search the event entry with the desired X-coordinate */
	for (i=0;i<Nr_entries;i++)
	{
		/* Right X-coordinate / not deleted ? */
		if ((Entry_data[i].X == X) && (Entry_data[i].First_block_nr
		 != 0xFFFF))
		{
			/* Yes -> Get event chain index */
			First_block_nr = Entry_data[i].First_block_nr;
			Chain_nr = Get_event_chain_nr(First_block_nr);

			/* Has this event been saved ? */
			if (!Event_is_saved(PARTY_DATA.Map_nr, Chain_nr))
			{
				/* No -> Found a chain */
				Value = Entry_data[i].Trigger_modes;
				break;
			}
		}
	}

	/* Found something ? */
	if (Value)
	{
		/* Yes -> Examine the first event block in the chain */
		Block = (struct Event_block *) (Map_ptr + Event_data_offset +
		 (First_block_nr * sizeof(struct Event_block)));

		/* Is this an Offset event ? */
		if (Block->Type == OFFSET_EVENT)
		{
			/* Yes -> Relative coordinates ? */
			if (Block->Byte_3 & 0x01)
			{
				/* Yes -> Add relative coordinates */
				X += (SISHORT) ((SIBYTE) Block->Byte_1);
				Y += (SISHORT) ((SIBYTE) Block->Byte_2);
			}
			else
			{
				/* No -> Get absolute coordinates */
				X = (UNSHORT) Block->Byte_1;
				Y = (UNSHORT) Block->Byte_2;
			}

			/* Recurse */
			Value = Get_map_event_triggers(X, Y);
		}
	}
	MEM_Free_pointer(Map_handle);

	return Value;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_chains_in_map
 * FUNCTION  :	Search the current map for event chains which have a given
 *              trigger-mode and execute these chains.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.04.95 18:03
 * LAST      : 30.07.95 19:34
 * INPUTS    : UNSHORT Trigger_mode - Trigger mode index.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - NPCs will also be checked.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_chains_in_map(UNSHORT Trigger_mode)
{
	struct Event_context *Context;
	struct Map_event_entry *Entry_data;
	UNLONG Offset;
	UNSHORT Nr_entries = 0;
	UNSHORT Map_nr;
	UNSHORT Chain_nr;
	UNSHORT First_block_nr;
	UNSHORT Y;
	UNSHORT i, j;
	UNBYTE *Map_ptr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return;

	/* Have the map events been changed ? */
	if (Map_events_changed)
	{
		/* Yes -> Update event catalogues */
		Update_event_chain_catalogues();

		/* Clear flag */
		Map_events_changed = FALSE;
	}

	/* Have any chains been catalogued for this triggermode ? */
	if (Catalogued_map_chain_counters[Trigger_mode])
	{
		/* Yes -> Execute catalogued chains */
		Execute_catalogued_chains_in_map(Trigger_mode);
	}
	else
	{
		/* Get current map number */
		Map_nr = PARTY_DATA.Map_nr;

		/* Search NPCs for events */
		for (i=0;i<NPCS_PER_MAP;i++)
		{
			/* Anyone there ? */
			if (NPC_present(i))
			{
				/* Yes -> Has event / right trigger mode / event not deleted ? */
				if ((VNPCs[i].Trigger_modes & (1 << Trigger_mode)) &&
				 (VNPCs[i].First_block_nr != 0xFFFF))
				{
					/* Yes -> Get first block index */
					First_block_nr = VNPCs[i].First_block_nr;

					/* Get event chain index */
					Chain_nr = Get_event_chain_nr(First_block_nr);

					/* Has this event been saved ? */
					if (!Event_is_saved(Map_nr, Chain_nr))
					{
						/* No -> Make a new context */
						Context = Push_event_context();

						/* Initialize context */
						Context->Source 			= MAP_EVENT_SOURCE;
						Context->Set_handle 		= Map_handle;
						Context->Text_handle 	= Map_text_handle;
						Context->Chain_nr			= Chain_nr;
						Context->Chain_block_nr = First_block_nr;
						Context->Block_nr 		= First_block_nr;
						Context->Max_blocks 		= Nr_event_blocks;
						Context->Base 				= Event_data_offset;
						Context->Validator 		= Map_context_validator;

						Context->Source_data.Map_source.Map_nr 	= Map_nr;
						Context->Source_data.Map_source.Trigger 	= Trigger_mode;
						Context->Source_data.Map_source.Value 		= 0;
						Context->Source_data.Map_source.X 			= 0;
						Context->Source_data.Map_source.Y 			= 0;

						/* Execute it */
						Execute_event_chain();

						/* Still in the same map ? */
						if (Map_nr != PARTY_DATA.Map_nr)
						{
							/* No -> Break */
							break;
						}
					}
				}
			}
		}

		/* In a different map ? */
		if (Map_nr != PARTY_DATA.Map_nr)
		{
			/* Yes -> Exit */
			return;
		}

		/* Get pointer to map data */
		Map_ptr = MEM_Claim_pointer(Map_handle);

		/* Get pointer to event entry data */
		Offset = Event_entry_offset;

		/* Search all Y-coordinates (including global events) */
		for (Y=0;Y<=Map_height;Y++)
		{
			Nr_entries = *((UNSHORT *) (Map_ptr + Offset));
			Offset += 2;
			Entry_data = (struct Map_event_entry *) (Map_ptr + Offset);

			/* Search all entries */
			for (j=0;j<Nr_entries;j++)
			{
				/* Right trigger mode / not deleted ? */
				if ((Entry_data[j].Trigger_modes & (1 << Trigger_mode)) &&
				 (Entry_data[j].First_block_nr != 0xFFFF))
				{
					/* Yes -> Get first block index */
					First_block_nr = Entry_data[j].First_block_nr;

					/* Get event chain index */
					Chain_nr = Get_event_chain_nr(First_block_nr);

					/* Has this event been saved ? */
					if (!Event_is_saved(Map_nr, Chain_nr))
					{
						/* No -> Make a new context */
						Context = Push_event_context();

						/* Initialize context */
						Context->Source 			= MAP_EVENT_SOURCE;
						Context->Set_handle 		= Map_handle;
						Context->Text_handle 	= Map_text_handle;
						Context->Chain_nr 		= Chain_nr;
						Context->Chain_block_nr	= First_block_nr;
						Context->Block_nr 		= First_block_nr;
						Context->Max_blocks 		= Nr_event_blocks;
						Context->Base 				= Event_data_offset;
						Context->Validator 		= Map_context_validator;

						Context->Source_data.Map_source.Map_nr 	= Map_nr;
						Context->Source_data.Map_source.Trigger 	= Trigger_mode;
						Context->Source_data.Map_source.Value 		= 0;
						Context->Source_data.Map_source.X 			= Entry_data[j].X;
						Context->Source_data.Map_source.Y 			= Y;

						/* Execute it */
						Execute_event_chain();

						/* Still in the same map ? */
						if (Map_nr == PARTY_DATA.Map_nr)
						{
							/* No -> Break */
							break;
						}
					}
				}
			}
			/* Still in the same map ? */
			if (Map_nr != PARTY_DATA.Map_nr)
				break;

			/* Yes -> Next Y */
			Offset += Nr_entries * sizeof(struct Map_event_entry);
		}

		MEM_Free_pointer(Map_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_catalogued_chains_in_map
 * FUNCTION  :	Execute catalogued event chains in the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.05.95 11:43
 * LAST      : 29.07.95 19:39
 * INPUTS    : UNSHORT Trigger_mode - Trigger mode index.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - NPCs will also be checked.
 *             - This function assumes the events have been catalogued.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_catalogued_chains_in_map(UNSHORT Trigger_mode)
{
	struct Catalogued_map_chain *Catalogue;
	struct Event_context *Context;
	UNLONG Nr_catalogued_chains;
	UNLONG i;
	UNSHORT Map_nr;
	UNSHORT Chain_nr;
	UNSHORT First_block_nr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return;

	/* Get event catalogue data */
	Nr_catalogued_chains = Catalogued_map_chain_counters[Trigger_mode];

	Catalogue = (struct Catalogued_map_chain *)
	 MEM_Claim_pointer(Chain_catalogue_handles[Trigger_mode]);

	/* Get current map number */
	Map_nr = PARTY_DATA.Map_nr;

	/* Search NPCs for events */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (NPC_present(i))
		{
			/* Yes -> Has event / right trigger mode / event not deleted ? */
			if ((VNPCs[i].Trigger_modes & (1 << Trigger_mode)) &&
			 (VNPCs[i].First_block_nr != 0xFFFF))
			{
				/* Yes -> Get first block index */
				First_block_nr = VNPCs[i].First_block_nr;

				/* Get event chain index */
				Chain_nr = Get_event_chain_nr(First_block_nr);

				/* Has this event been saved ? */
				if (!Event_is_saved(Map_nr, Chain_nr))
				{
					/* No -> Make a new context */
					Context = Push_event_context();

					/* Initialize context */
					Context->Source 			= MAP_EVENT_SOURCE;
					Context->Set_handle 		= Map_handle;
					Context->Text_handle 	= Map_text_handle;
					Context->Chain_nr 		= Chain_nr;
					Context->Chain_block_nr	= First_block_nr;
					Context->Block_nr 		= First_block_nr;
					Context->Max_blocks 		= Nr_event_blocks;
					Context->Base 				= Event_data_offset;
					Context->Validator 		= Map_context_validator;

					Context->Source_data.Map_source.Map_nr 	= Map_nr;
					Context->Source_data.Map_source.Trigger	= Trigger_mode;
					Context->Source_data.Map_source.Value 		= 0;
					Context->Source_data.Map_source.X 			= 0;
					Context->Source_data.Map_source.Y 			= 0;

					/* Execute it */
					Execute_event_chain();

					/* Still in the same map ? */
					if (Map_nr != PARTY_DATA.Map_nr)
					{
						/* No -> Break */
						break;
					}
				}
			}
		}
	}

	/* In a different map ? */
	if (Map_nr != PARTY_DATA.Map_nr)
	{
		/* Yes -> Exit */
		return;
	}

	/* Go through all catalogued chains */
	for (i=0;i<Nr_catalogued_chains;i++)
	{
		/* Not deleted ? */
		if (Catalogue[i].First_block_nr != 0xFFFF)
		{
			/* Yes -> Get first block index */
			First_block_nr = Catalogue[i].First_block_nr;

			/* Get event chain index */
			Chain_nr = Get_event_chain_nr(First_block_nr);

			/* Has this event been saved ? */
			if (!Event_is_saved(Map_nr, Chain_nr))
			{
				/* No -> Make a new context */
				Context = Push_event_context();

				/* Initialize context */
				Context->Source			= MAP_EVENT_SOURCE;
				Context->Set_handle		= Map_handle;
				Context->Text_handle		= Map_text_handle;
				Context->Chain_nr			= Chain_nr;
				Context->Chain_block_nr	= First_block_nr;
				Context->Block_nr			= First_block_nr;
				Context->Max_blocks		= Nr_event_blocks;
				Context->Base				= Event_data_offset;
				Context->Validator		= Map_context_validator;

				Context->Source_data.Map_source.Map_nr		= Map_nr;
				Context->Source_data.Map_source.Trigger	= Trigger_mode;
				Context->Source_data.Map_source.Value		= 0;
				Context->Source_data.Map_source.X			= Catalogue[i].X;
				Context->Source_data.Map_source.Y			= Catalogue[i].Y;

				/* Execute it */
				Execute_event_chain();

				/* Still in the same map ? */
				if (Map_nr != PARTY_DATA.Map_nr)
				{
					/* No -> Break */
					break;
				}
			}
		}
	}
	MEM_Free_pointer(Chain_catalogue_handles[Trigger_mode]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Catalogue_event_chains
 * FUNCTION  : Catalogue all event chains of a certain type in the current
 *              map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.05.95 14:48
 * LAST      : 19.05.95 14:48
 * INPUTS    : UNSHORT Trigger_mode - Trigger mode index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Catalogue_event_chains(UNSHORT Trigger_mode)
{
	struct Catalogued_map_chain *Catalogue;

	/* Legal triggermode ? */
	if (Trigger_mode < MAX_TRIGGER_MODES)
	{
		/* Yes -> Does a catalogue of this triggermode already exist ? */
		if (Catalogued_map_chain_counters[Trigger_mode])
		{
			/* Yes -> Destroy buffer */
			MEM_Free_memory(Chain_catalogue_handles[Trigger_mode]);
		}

		/* Count number of chains with this triggermode */
		Catalogued_map_chain_counters[Trigger_mode] =
		 Catalogue_chains_in_map(Trigger_mode, NULL);

		/* Any ? */
		if (Catalogued_map_chain_counters[Trigger_mode])
		{
			/* Yes -> Allocate buffer for catalogue */
			Chain_catalogue_handles[Trigger_mode] =
			 MEM_Allocate_memory(Catalogued_map_chain_counters[Trigger_mode] *
			 sizeof(struct Catalogued_map_chain));

			/* Catalogue event chains */
			Catalogue = (struct Catalogued_map_chain *)
			 MEM_Claim_pointer(Chain_catalogue_handles[Trigger_mode]);

			Catalogue_chains_in_map(Trigger_mode, Catalogue);

			MEM_Free_pointer(Chain_catalogue_handles[Trigger_mode]);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Catalogue_chains_in_map
 * FUNCTION  :	Search the current map for event chains which have a given
 *              trigger-mode and catalogue these chains.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.05.95 11:43
 * LAST      : 19.05.95 11:43
 * INPUTS    : UNSHORT Trigger_mode - Trigger mode index.
 *             struct Catalogued_map_chain *Catalogue - Pointer to buffer
 *              where the chains will be catalogued / NULL.
 * RESULT    : UNLONG : Number of chains found.
 * BUGS      : No known.
 * NOTES     : - NPCs will NOT be checked.
 *             - If the Catalogue-pointer is NULL, the chains will be counted
 *              and not catalogued.
 *             - It is assumed the catalogue buffer is big enough.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Catalogue_chains_in_map(UNSHORT Trigger_mode,
 struct Catalogued_map_chain *Catalogue)
{
	struct Map_event_entry *Entry_data;
	UNLONG Offset;
	UNLONG Counter = 0;
	UNSHORT Nr_entries = 0;
	UNSHORT i, j;
	UNBYTE *Map_ptr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0;

	/* Get pointer to map data */
	Map_ptr = MEM_Claim_pointer(Map_handle);

	/* Get pointer to event entry data */
	Offset = Event_entry_offset;

	/* Search all Y-coordinates (including global events) */
	for (i=0;i<=Map_height;i++)
	{
		Nr_entries = *((UNSHORT *) (Map_ptr + Offset));
		Offset += 2;
		Entry_data = (struct Map_event_entry *) (Map_ptr + Offset);

		/* Search all entries */
		for (j=0;j<Nr_entries;j++)
		{
			/* Right trigger mode ? */
			if (Entry_data[j].Trigger_modes & (1 << Trigger_mode))
			{
				/* Yes -> Catalogue ? */
				if (Catalogue)
				{
					/* Yes -> Store data */
					Catalogue[Counter].X						= Entry_data[j].X;
					Catalogue[Counter].Y						= i;
					Catalogue[Counter].First_block_nr	= Entry_data[j].First_block_nr;
				}

				/* Count up */
				Counter++;
			}
		}

		/* Next Y */
		Offset += Nr_entries * sizeof(struct Map_event_entry);
	}
	MEM_Free_pointer(Map_handle);

	return Counter;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_event_chain_catalogues
 * FUNCTION  :	Destroy all event chain catalogues of the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.05.95 14:52
 * LAST      : 19.05.95 14:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_event_chain_catalogues(void)
{
	UNSHORT i;

	/* Check all triggermodes */
	for (i=0;i<MAX_TRIGGER_MODES;i++)
	{
		/* Any chains catalogued for this triggermode ? */
		if (Catalogued_map_chain_counters[i])
		{
			/* Yes -> Destroy buffer */
			MEM_Free_memory(Chain_catalogue_handles[i]);

			/* Clear counter */
			Catalogued_map_chain_counters[i] = 0;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_event_chain_catalogues
 * FUNCTION  :	Update all event chain catalogues of the current map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.05.95 18:45
 * LAST      : 20.05.95 18:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_event_chain_catalogues(void)
{
	UNSHORT i;

	/* Check all triggermodes */
	for (i=0;i<MAX_TRIGGER_MODES;i++)
	{
		/* Any chains catalogued for this triggermode ? */
		if (Catalogued_map_chain_counters[i])
		{
			/* Yes -> Build new catalogue */
			Catalogue_event_chains(i);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Map_context_validator
 * FUNCTION  :	Determine if a map event context is still valid.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 10:23
 * LAST      : 30.12.94 10:23
 * INPUTS    : struct Event_context *Context.
 * RESULT    : BOOLEAN : TRUE (valid) or FALSE (invalid).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Map_context_validator(struct Event_context *Context)
{
	/* Still in the same map ? */
	return (Context->Source_data.Map_source.Map_nr == PARTY_DATA.Map_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_context_validator
 * FUNCTION  : Determine if the dialogue partner event context is still valid.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.01.95 14:45
 * LAST      : 31.07.95 10:31
 * INPUTS    : struct Event_context *Context - Pointer to event context.
 * RESULT    : BOOLEAN : TRUE (valid) or FALSE (invalid).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Dialogue_context_validator(struct Event_context *Context)
{
	/* Still in the same event set ? */
	if (Get_event_set_index(Dialogue_char_handle,
	 Context->Source_data.Set_source.Char_set_index) !=
	 Context->Source_data.Set_source.Set_index)
	{
		/* No -> Exit */
		return FALSE;
	}

	/* Talking with a riddlemouth ? */
	if (Dialogue_type == RIDDLEMOUTH_DIALOGUE)
	{
		/* Yes -> OK */
		return TRUE;
	}
	else
	{
		/* No -> Check if the character is capapble */
		return(Character_capable(Dialogue_char_handle));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_member_context_validator
 * FUNCTION  : Determine if a party member's event context is still valid.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.01.95 13:08
 * LAST      : 31.07.95 10:31
 * INPUTS    : struct Event_context *Context - Pointer to event context.
 * RESULT    : BOOLEAN : TRUE (valid) or FALSE (invalid).
 * BUGS      : No known.
 * NOTES     : - This function will work for any event set except those
 *              belonging to the current dialogue partner.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_member_context_validator(struct Event_context *Context)
{
	BOOLEAN Result = TRUE;
	UNSHORT Char_index;
	UNSHORT Member_index;

	/* Get character index */
	Char_index = Context->Source_data.Set_source.Char_index;

	/* What type of character ? */
	switch (Context->Source_data.Set_source.Char_type)
	{
		/* Party character */
		case PARTY_CHAR_TYPE:
		{
			/* Find character in party */
			Member_index = Find_character_in_party(Char_index);

			/* Found ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Still in the same event set ? */
				if (Get_event_set_index(Party_char_handles[Member_index - 1],
				 Context->Source_data.Set_source.Char_set_index) !=
				 Context->Source_data.Set_source.Set_index)
				{
					/* No -> Exit */
					return FALSE;
				}

				/* Yes -> Is this character capable ? */
				Result = Character_capable(Party_char_handles[Member_index - 1]);
			}
			else
			{
				/* No */
				Result = FALSE;
			}
			break;
		}
		/* Monster character */
		case MONSTER_CHAR_TYPE:
		{
			break;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event_first_block_nr
 * FUNCTION  : Get the number of the first block of a map event chain.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 13:43
 * LAST      : 12.08.95 13:56
 * INPUTS    : UNSHORT Chain_nr - Event chain number.
 * RESULT    : UNSHORT : Number of first event block (0xFFFF = not found).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_event_first_block_nr(UNSHORT Chain_nr)
{
	UNSHORT Nr_chains;
	UNSHORT *Ptr;
	UNSHORT First_block_nr = 0xFFFF;

	/* Get pointer to event chain data */
	Ptr = (UNSHORT *)(MEM_Claim_pointer(Map_handle) + Event_chain_offset);

 	/* Get number of chains in this map */
	Nr_chains = (_3D_map) ? EVENTS_PER_3D_MAP : EVENTS_PER_MAP;

	/* Legal chain number ? */
	if (Chain_nr < Nr_chains)
	{
		/* Yes -> Get first block number */
		First_block_nr = Ptr[Chain_nr];
	}

	MEM_Free_pointer(Map_handle);

	return First_block_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event_chain_nr
 * FUNCTION  : Get the number of an event chain.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.01.95 12:20
 * LAST      : 29.07.95 19:43
 * INPUTS    : UNSHORT First_block_nr - Number of first event block.
 * RESULT    : UNSHORT : Event chain number (0xFFFF = not found).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_event_chain_nr(UNSHORT First_block_nr)
{
	UNSHORT Nr_chains;
	UNSHORT Chain_nr = 0xFFFF;
	UNSHORT i;
	UNSHORT *Ptr;

	/* Get pointer to event chain data */
	Ptr = (UNSHORT *)(MEM_Claim_pointer(Map_handle) + Event_chain_offset);

	/* Get number of chains in this map */
	Nr_chains = (_3D_map) ? EVENTS_PER_3D_MAP : EVENTS_PER_MAP;

	/* Check all chains */
	for (i=0;i<Nr_chains;i++)
	{
		if (Ptr[i] == First_block_nr)
		{
			Chain_nr = i;
			break;
		}
	}

	MEM_Free_pointer(Map_handle);

	return Chain_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event_entry_at_position
 * FUNCTION  : Get the event entry at a certain position in a map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 19:13
 * LAST      : 07.02.95 19:13
 * INPUTS    : SISHORT X - X-coordinate (1...).
 *             SISHORT Y - Y-coordinate (1...).
 *             UNBYTE *Map_data - Pointer to start of map data.
 * RESULT    : struct Map_event_entry * : Entry (NULL = not found).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Map_event_entry *
Get_event_entry_at_position(UNBYTE *Map_data, SISHORT X, SISHORT Y)
{
	struct Map_event_entry *Entry_data;
	struct Map_event_entry *Output = NULL;
	UNSHORT Nr_entries = 0;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X >= Map_width) || (Y < 1) || (Y >= Map_height))
		return NULL;

	/* Get pointer to event entry data */
	Ptr = Map_data + Event_entry_offset;

	/* Find the event entry-list for the desired Y-coordinate */
	for (i=0;i<Y;i++)
	{
		Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
	}
	Nr_entries = *((UNSHORT *) Ptr);
	Ptr += 2;
	Entry_data = (struct Map_event_entry *) Ptr;

	/* Search the event entry with the desired X-coordinate */
	for (i=0;i<Nr_entries;i++)
	{
		/* Right X-coordinate ? */
		if (Entry_data[i].X == X)
		{
			/* Yes -> Found a chain */
			Output = &(Entry_data[i]);
			break;
		}
	}

	return Output;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_event_chain_nr_at_position
 * FUNCTION  : Get the event chain number at a certain position in a map.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.02.95 11:12
 * LAST      : 29.07.95 19:52
 * INPUTS    : SISHORT X - X-coordinate (1...).
 *             SISHORT Y - Y-coordinate (1...).
 *             UNBYTE *Map_data - Pointer to start of map data.
 * RESULT    : UNSHORT : Event chain number (0xFFFF = not found).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_event_chain_nr_at_position(UNBYTE *Map_data, SISHORT X, SISHORT Y)
{
	struct Map_event_entry *Entry;

	/* Get the event entry at the desired position */
	Entry = Get_event_entry_at_position(Map_data, X, Y);

	/* Any entry ? */
	if (Entry)
	{
		/* Yes -> Get the event chain number */
		return(Get_event_chain_nr(Entry->First_block_nr));
	}
	else
	{
		/* No */
		return 0xFFFF;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Event_error
 * FUNCTION  : Report an event error.
 * FILE      : EVELOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.02.95 11:00
 * LAST      : 24.02.95 11:00
 * INPUTS    : UNSHORT Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : EVELOGIC.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Event_error(UNSHORT Error_code)
{
	struct Error_report Report;

	/* Build error report */
	Report.Code = Error_code;
	Report.Messages = &(_Event_errors[0]);

	/* Push error on the error stack */
	ERROR_PushError(Print_error, _Event_library_name,
	 sizeof(struct Error_report), (UNBYTE *) &Report);
}



void
Display_event_data(struct Event_context *Context)
{
	static UNCHAR Trigger_mode_names[MAX_TRIGGER_MODES][20] = {
		"Normal",
		"Examine",
		"Touch",
		"Speak",
		"Use item",
		"Map init",
		"Every step",
		"Every hour",
		"Every day",
		"Default",
		"Action",
		"NPC",
		"Take"
	};
	static UNCHAR Event_type_names[MAX_EVENT_TYPES][20] = {
		"Map exit",
		"Door",
		"Chest",
		"Text",
		"Spinner",
		"Trap",
		"Change used item",
		"Datachange",
		"Change icon",
		"Encounter",
		"Place action",
		"Query",
		"Modify",
		"Action",
		"Signal",
		"Clone automap",
		"Sound",
		"Start dialogue",
		"Create transport",
		"Execute",
		"Remove party member",
		"End dialogue",
		"Wipe",
		"Play animation",
		"Offset",
		"Pause",
		"Simple chest",
		"Ask surrender",
		"Do script"
	};

	UNSHORT Trigger_mode = 0;
	UNSHORT Y;

	/* Is a map context ? */
	if (Context->Source == MAP_EVENT_SOURCE)
	{
		/* Yes -> Get trigger mode */
		Trigger_mode = Context->Source_data.Map_source.Trigger;
	}

	/* Exit if this is an EVERY STEP event chain */
	if (Trigger_mode == EVERY_STEP_TRIGGER)
		return;

	/* Else print event diagnostics */
	Init_dprint(0, 0, 150, 200);

	switch (Context->Source)
	{
		case MAP_EVENT_SOURCE:
		{
			xprintf(&Diag_OPM, 0, 0, "Source : Map");

			xprintf(&Diag_OPM, 10, 10, "Map nr : %u",
			 Context->Source_data.Map_source.Map_nr);
			xprintf(&Diag_OPM, 10, 20, "Trigger : %s",
			 &(Trigger_mode_names[Context->Source_data.Map_source.Trigger][0]));
 			xprintf(&Diag_OPM, 10, 30, "Value : %u",
			 Context->Source_data.Map_source.Value);
			xprintf(&Diag_OPM, 10, 40, "X : %u", Context->Source_data.Map_source.X);
 			xprintf(&Diag_OPM, 10, 50, "Y : %u", Context->Source_data.Map_source.Y);

			Y = 60;

			break;
		}
		case SET_EVENT_SOURCE:
		{
			xprintf(&Diag_OPM, 0, 0, "Source : Event set");

			switch(Context->Source_data.Set_source.Char_type)
			{
				case PARTY_CHAR_TYPE:
				{
					xprintf(&Diag_OPM, 10, 10, "Character type : party member");
					break;
				}
				case NPC_CHAR_TYPE:
				{
					xprintf(&Diag_OPM, 10, 10, "Character type : NPC");
					break;
				}
				case MONSTER_CHAR_TYPE:
				{
					xprintf(&Diag_OPM, 10, 10, "Character type : monster");
					break;
				}
				case 0xFFFF:
				{
					xprintf(&Diag_OPM, 10, 10, "Character type : none");
					break;
				}
			}

			xprintf(&Diag_OPM, 10, 20, "Character index : %u",
			 Context->Source_data.Set_source.Char_index);
			xprintf(&Diag_OPM, 10, 30, "Character set index : %u",
			 Context->Source_data.Set_source.Char_set_index);
			xprintf(&Diag_OPM, 10, 40, "Event set index : %u",
			 Context->Source_data.Set_source.Set_index);

			Y = 50;

			break;
		}
		case INLINE_EVENT_SOURCE:
		{
			xprintf(&Diag_OPM, 0, 0, "Source : Inline");

			Y = 10;

			break;
		}
	}

	xprintf(&Diag_OPM, 0, Y, "Chain number : %u", Context->Chain_nr);
	xprintf(&Diag_OPM, 0, Y + 10, "Block number : %u", Context->Block_nr);

	xprintf(&Diag_OPM, 0, Y + 20, "Type : %s",
	 &(Event_type_names[Context->Data.Type - 1][0]));

	xprintf(&Diag_OPM, 0, Y + 30, "Byte 1 : %u", Context->Data.Byte_1);
	xprintf(&Diag_OPM, 0, Y + 40, "Byte 2 : %u", Context->Data.Byte_2);
	xprintf(&Diag_OPM, 0, Y + 50, "Byte 3 : %u", Context->Data.Byte_3);
	xprintf(&Diag_OPM, 0, Y + 60, "Byte 4 : %u", Context->Data.Byte_4);
	xprintf(&Diag_OPM, 0, Y + 70, "Byte 5 : %u", Context->Data.Byte_5);
	xprintf(&Diag_OPM, 0, Y + 80, "Word 6 : %u", Context->Data.Word_6);
	xprintf(&Diag_OPM, 0, Y + 90, "Word 8 : %u", Context->Data.Word_8);

	xprintf(&Diag_OPM, 0, Y + 100, "Next : %u", Context->Data.Next_event_nr);

	Update_screen();

	Wait_4_user();

	Exit_dprint();
}

