/************
 * NAME     : COMBAT.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMBAT.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>

#include <SORT.H>
#include <FINDCOL.H>
#include <TEXT.H>

#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <COMBAT.H>
#include <COMBVAR.H>
#include <COMACTS.H>
#include <COMSHOW.H>
#include <COMOBS.H>
#include <TACTICAL.H>
#include <EVELOGIC.H>
#include <MAGIC.H>
#include <APRES.H>
#include <XFTYPES.H>
#include <SOUND.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <ITMLOGIC.H>
#include <COLOURS.H>
#include <TEXTWIN.H>
#include <MONLOGIC.H>
#include <MAP.H>

/* defines */

#define MAX_COMBAT_BACKGROUNDS	(19)

#define MAX_COMBAT_UPDATES			(15)

#define COMBAT_ANIM_DELAY			(3)

#define COMBAT_PAL_NR				(50)

#define ENVIRONMENT_SPEED			(51)
#define TEMPORARY_EFFECTS_SPEED	(0)

/* prototypes */

void Combat_ModInit(void);
void Combat_ModExit(void);

void Init_Combat_display(void);
void Exit_Combat_display(void);
void Update_Combat_display(void);

void Reset_combat_actions(void);

void Handle_combat_environment_events(void);
void Handle_combat_temporary_effects_event(void);

void Determine_monster_actions(void);

void Determine_uncontrollable_actions(void);

void Select_uncontrollable_action(struct Combat_participant *Part);
void Select_panicked_action(struct Combat_participant *Part);
void Select_crazy_action(struct Combat_participant *Part);

void Try_crazy_move(struct Combat_participant *Part);
void Try_crazy_attack(struct Combat_participant *Part);

void Create_sorted_combat_event_list(void);

void Swap_combat_events(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_combat_events(UNLONG A, UNLONG B, UNBYTE *Data);

BOOLEAN Init_party_combat_data(void);
void Exit_party_combat_data(void);

BOOLEAN Init_monster_combat_data(UNSHORT Monster_group_nr);
void Exit_monster_combat_data(void);

MEM_HANDLE Clone_monster_data(MEM_HANDLE Original_data_handle);

void Do_monster_appear_animations(void);

/* global variables */

static struct Module Combat_Mod = {
	LOCAL_MOD, SCREEN_MOD, COMBAT_SCREEN,
	Update_display,
	Combat_ModInit,
	Combat_ModExit,
	Init_Combat_display,
	Exit_Combat_display,
	Update_Combat_display
};

BOOLEAN In_Combat = FALSE;
BOOLEAN Fighting = FALSE;
BOOLEAN End_combat_flag;
BOOLEAN Show_monster_LP_flag;
BOOLEAN Party_has_advanced;

UNLONG Combat_square_width = COMBAT_SQUARE_WIDTH;
UNLONG Combat_square_depth = COMBAT_SQUARE_DEPTH;

MEM_HANDLE Combat_background_handle;

UNSHORT Tactical_window_object;

static UNSHORT Combat_background_number;
static UNSHORT Monster_group_number;

UNSHORT Combat_status;
UNLONG Added_experience_points;
UNSHORT Nr_monsters;

static UNLONG Combat_timer;
UNSHORT Nr_combat_updates;
BOOLEAN First_combat_update;

static SISHORT Combat_remaining_ticks = 0;
static UNSHORT Combat_animation_timer = 0;

/* Text command variables */
struct Combat_participant *Current_acting_part;
struct Combat_participant *Current_victim_part;
UNSHORT Combat_text_damage;
UNSHORT Combat_text_weapon_item_index;

/* Combat palette table */
static UNSHORT Combat_palettes[MAX_COMBAT_BACKGROUNDS] = {
	23, 24, 27, 32, 33,
	34, 35, 36, 37, 38,
	39, 40, 41, 42, 43,
	44, 52, 53, 54
};

/* List of combat events, sorted on speed */
static UNSHORT Nr_combat_events;
static struct Combat_event Combat_event_list[MAX_COMBAT_EVENTS];

/* Party member combat participant array */
struct Combat_participant Party_parts[6];

/* Monster combat participant array */
struct Combat_participant Monster_parts[MONSTERS_PER_GROUP];

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_Combat
 * FUNCTION  : Enter the combat screen.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 10:55
 * LAST      : 27.10.95 17:45
 * INPUTS    : UNSHORT Monster_group_nr - Monster group number.
 *             UNSHORT Combat_background_nr - Combat background number.
 *             UNSHORT NPC_index - NPC index / 0xFFFF.
 * RESULT    : UNSHORT : Combat status.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Enter_Combat(UNSHORT Monster_group_nr, UNSHORT Combat_background_nr,
 UNSHORT NPC_index)
{
	/* Night combat background table */
	static const UNSHORT Night_combat_background_table[MAX_COMBAT_BACKGROUNDS] = {
		 0,  0, 17,  0,  0,
		 0,  0,  0,  0,  0,
		 0, 18,  0, 13, 19,
		 0,  0,  0,  0
	};

	UNLONG Bit_index;
	UNSHORT Map_nr;

	/* Store map number */
	Map_nr = PARTY_DATA.Map_nr;

	/* Legal combat background number ? */
	if (!Combat_background_nr ||
	 (Combat_background_nr > MAX_COMBAT_BACKGROUNDS))
	{
		/* No -> Use default */
		Combat_background_nr = 1;
	}

	/* Is it night ? */
	if ((PARTY_DATA.Hour < MORNING_TIME) ||
	 (PARTY_DATA.Hour >= EVENING_TIME))
	{
		/* Yes -> Is there a night version of this background ? */
		if (Night_combat_background_table[Combat_background_nr - 1])
		{
			/* Yes -> Use it */
			Combat_background_nr = Night_combat_background_table[Combat_background_nr - 1];
		}
	}

	/* Legal combat background number ? */
	if (!Combat_background_nr ||
	 (Combat_background_nr > MAX_COMBAT_BACKGROUNDS))
	{
		/* No -> Use default */
		Combat_background_nr = 1;
	}

	/* Store combat parameters */
	Monster_group_number			= Monster_group_nr;
	Combat_background_number	= Combat_background_nr;

	/* Still in move mode ? */
	if (Move_mode_flag)
	{
		/* Yes -> Exit move mode */
		Pop_module();

		/* Clear flag again just to be sure */
		Move_mode_flag = FALSE;
	}

	/* Enter combat screen */
	Exit_display();
	Push_module(&Combat_Mod);

	/* Restart ? */
	if (!Restart_game_flag)
	{
		/* No -> Act depending on combat status */
		switch (Combat_status)
		{
			case COMBAT_PARTY_WON:
			{
				/* Combat with NPC ? */
				if (NPC_index != 0xFFFF)
				{
					/* Yes -> Calculate bit index */
					Bit_index = ((Map_nr - 1) * NPCS_PER_MAP) + NPC_index;

					/* Disable the NPC */
					Write_bit_array(CD_BIT_ARRAY, Bit_index, SET_BIT_ARRAY);
				}

				/* Enter Apres combat screen */
				Enter_Apres_combat();
				break;
			}
			case COMBAT_PARTY_LOST:
			{
				/* Game over */
				Game_over();
				break;
			}
			case COMBAT_PARTY_FLED:
			{
				/* Re-draw the screen */
				Init_display();

				/* Clear combat conditions */
				Clear_combat_conditions();

				/* Set monster move delay */
				Monster_move_delay = MONSTER_DELAY_TIME;
				break;
			}
			case COMBAT_ENDED:
			{
				/* Re-draw the screen */
				Init_display();

				/* Clear combat conditions */
				Clear_combat_conditions();
				break;
			}
		}
	}

	/* Destroy Apres combat item pool */
	Exit_Apres_pool();

	return Combat_status;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Combat_ModInit
 * FUNCTION  : Initialize Combat module.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 18:09
 * LAST      : 20.10.95 20:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Combat_DisInit isn't called here!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Combat_ModInit(void)
{
	static struct Event_action Combat_start_action =
	{
		0,	NO_ACTOR_TYPE, 0,
		COMBAT_START_ACTION, 0, 0,
		NULL, NULL, NULL
	};

	BOOLEAN Result;
	UNSHORT Palette_nr;
	UNSHORT i, j;

	/* Clear combat matrix */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			BASEMEM_FillMemByte((UNBYTE *) &Combat_matrix[i][j],
			 sizeof(struct Tactical_square), 0);
		}
	}

	/* Clear variables */
	Added_experience_points	= 0;

	Combat_remaining_ticks = 0;
	Combat_animation_timer = 0;

	/* Clear flags */
	End_combat_flag		= FALSE;
	Show_monster_LP_flag	= FALSE;
	Party_has_advanced	= FALSE;

	/* Stop music (to facilitate loading) */
	SOUND_Located_sound_effects_off();
	SOUND_Play_song(0);
	SOUND_Play_ambient_song(0);

	/* Load combat background */
	Combat_background_handle = Load_subfile(COMBAT_BACKGROUND,
	 Combat_background_number);
	if (!Combat_background_handle)
	{
		Error(ERROR_FILE_LOAD);
		Pop_module();
		return;
	}

	/* Determine palette number */
	Palette_nr = Combat_palettes[Combat_background_number - 1];

	/* Initialize COMbat OBject system */
	Result = Init_COMOB_system(Palette_nr);
	if (!Result)
	{
		Pop_module();
		Exit_program();
		return;
	}

	/* Initialize party combat data */
	Init_party_combat_data();
	if (!Result)
	{
		Pop_module();
		Exit_program();
		return;
	}

	/* Initialize monster combat data */
	Result = Init_monster_combat_data(Monster_group_number);
	if (!Result)
	{
		Pop_module();
		Exit_program();
		return;
	}

	/* Initialize Apres pool */
	Init_Apres_pool();

	/* Set combat status */
	Combat_status = COMBAT_RUNNING;

	/* In combat */
	In_Combat = TRUE;

	/* Set control status */
	Update_control_status();

	/* Clear actions */
	Reset_combat_actions();

	/* Initialize tactical window background management */
	Init_tactical_window_background();

	/* Initialize combat timer */
	Combat_timer = SYSTEM_GetTicks();

	/* Start combat music */
	SOUND_Play_song(COMBAT_MUSIC);

	/* Load palette */
	Load_palette(Palette_nr);

	/* Draw COMbat OBjects */
	Draw_combat_screen();

	/* Combat starts event logic */
	Perform_action(&Combat_start_action);

	/* Let monsters appear */
	Do_monster_appear_animations();

	/* Add tactical window object */
	Tactical_window_object = Add_object
	(
		Earth_object,
		&Tactical_window_Class,
		NULL,
		TACTICAL_X,
		TACTICAL_Y,
		TACTICAL_WIDTH,
		TACTICAL_HEIGHT
	);

	/* Draw */
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);

	Update_screen();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Combat_ModExit
 * FUNCTION  : Exit Combat module.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.05.95 09:59
 * LAST      : 25.10.95 13:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Combat_ModExit(void)
{
	UNSHORT i;

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Remove all temporary effects */
			Remove_all_temporary_effects(&(Party_parts[i]));
		}
	}

	/* Exit COMbat OBject system */
	Exit_COMOB_system();

	/* Exit party and monster data */
	Exit_party_combat_data();
	Exit_monster_combat_data();

	/* Exit tactical window background management */
	Exit_tactical_window_background();

	/* Free other memory */
	MEM_Free_memory(Combat_background_handle);

	/* No longer in combat */
	In_Combat = FALSE;

	/* Set map music */
	SOUND_Located_sound_effects_on();
	Set_music(Current_map_music_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Combat_display
 * FUNCTION  : Initialize Combat display.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 15:41
 * LAST      : 20.10.95 20:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_Combat_display(void)
{
	UNSHORT Palette_nr;

	/* Determine palette number */
	Palette_nr = Combat_palettes[Combat_background_number - 1];

	/* Load palette */
	Load_palette(Palette_nr);

	/* Draw COMbat OBjects */
	Draw_combat_screen();

	/* Add tactical window object */
	Tactical_window_object = Add_object
	(
		Earth_object,
		&Tactical_window_Class,
		NULL,
		TACTICAL_X,
		TACTICAL_Y,
		TACTICAL_WIDTH,
		TACTICAL_HEIGHT
	);

	/* Draw */
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Combat_display
 * FUNCTION  : Exit Combat display.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.03.95 15:42
 * LAST      : 14.03.95 15:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_Combat_display(void)
{
	/* Delete interface objects */
	Delete_object(Tactical_window_object);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_Combat_display
 * FUNCTION  : Update Combat display.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 11:59
 * LAST      : 16.03.95 13:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_Combat_display(void)
{
	UNLONG T;

	/* Clear update counter */
	Nr_combat_updates = 0;

	/* Set flag */
	First_combat_update = TRUE;

	if (Fighting)
	{
		/* Re-draw combat screen */
		Draw_combat_screen();

		/* Get elapsed time and add to remaining ticks */
		T = SYSTEM_GetTicks();

		if (T >= Combat_timer)
			Combat_remaining_ticks += T - Combat_timer + 1;
		else
			Combat_remaining_ticks += Combat_timer - T + 1;

		Combat_timer = T;

		/* Adjust remaining ticks if too high */
		if (Combat_remaining_ticks > 32000)
			Combat_remaining_ticks -= 32000;

		/* Time to update ? */
		while (Combat_remaining_ticks > 2)
		{
			/* Yes -> Update COMOBs */
			Update_COMOBs();

			/* Clear flag */
			First_combat_update = FALSE;

			/* Time to animate ? */
			Combat_animation_timer++;
			if (Combat_animation_timer == COMBAT_ANIM_DELAY)
			{
				/* Yes -> Update combat animations */
				Update_combat_animations();

				/* Animate all COMOBs */
				Animate_all_COMOBs();

				/* Reset timer */
				Combat_animation_timer = 0;
			}

			/* Count up */
			if (Nr_combat_updates < MAX_COMBAT_UPDATES)
				Nr_combat_updates++;

			/* Count down */
			Combat_remaining_ticks -= 3;
		}
	}
	else
	{
		/* Reset animation timer */
		Combat_timer = SYSTEM_GetTicks();
		Combat_remaining_ticks = 0;
	}

	/* End of combat ? */
	if (End_combat_flag)
	{
		/* Yes -> Exit */
		Exit_display();
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Combat_round
 * FUNCTION  : Execute a round of combat.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 14:39
 * LAST      : 16.10.95 17:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Combat_round(void)
{
/*	struct Event_action Combat_round_start_action =
	{
		0,	NO_ACTOR_TYPE, 0,
		EVERY_ROUND_ACTION, 0, 0,
		NULL, NULL, NULL
	}; */
	struct Combat_participant *Part;
	BOOLEAN Battle_ended = FALSE;
	UNSHORT i;

	/* The party may advance again */
	Party_has_advanced = FALSE;

	/* The fight is on! */
	Fighting = TRUE;

	/* Delete interface objects */
	Delete_object(Tactical_window_object);

	/* Re-draw the screen */
	Update_screen();

	/* Combat round starts event logic */
//	Perform_action(&Combat_round_start_action);

	/* Determine actions of all non-party members */
	Determine_monster_actions();
	Determine_uncontrollable_actions();

	/* Create a sorted combat event list */
	Create_sorted_combat_event_list();

	/* Handle combat event list */
	for (i=0;i<Nr_combat_events;i++)
	{
		/* Act depending on combat event type */
		switch (Combat_event_list[i].Type)
		{
			/* Participant event */
			case PART_COMBAT_EVENT:
			{
				/* Get participant data */
				Part = Combat_event_list[i].Part;

				/* Any action ? */
				if (Part->Current_action != NO_COMACT)
				{
					/* Yes -> Monster ? */
					if (Part->Type == MONSTER_PART_TYPE)
					{
						/* Monster -> Wait for animation to end */
						Wait_4_combat_animation(Part);
					}

					/* Is this character capable of this action ? */
					if (Get_conditions(Part->Char_handle) &
					 Combat_action_table[Part->Current_action].Condition_mask)
					{
						/* No -> Reset action */
						Part->Current_action = NO_COMACT;
					}
					else
					{
						/* Yes -> Set current acting participant data */
						Current_acting_part = Part;

						/* Execute action (if any) */
						if (Combat_action_table[Part->Current_action].Handler)
							(Combat_action_table[Part->Current_action].Handler)(Part);

						/* Clear current acting participant data */
						Current_acting_part = NULL;
					}
				}
				break;
			}
			/* Environment event */
			case ENVIRONMENT_COMBAT_EVENT:
			{
				/* Handle environment events */
				Handle_combat_environment_events();
				break;
			}
			/* Temporary effects event */
			case TEMPORARY_EFFECTS_COMBAT_EVENT:
			{
				/* Handle temporary events */
				Handle_combat_temporary_effects_event();
				break;
			}
		}

		/* Remove incapable participants */
		Update_control_status();

		/* Redraw */
		Draw_combat_screen();

		/* End of battle ? */
		Battle_ended = End_of_battle();
		if (Battle_ended)
		{
			/* Yes -> Break */
			break;
		}
	}

	/* Wait for a short while so effects may end */
	for (i=0;i<10;i++)
		Update_screen();

	Fighting = FALSE;

	Update_screen();

	/* Add tactical window object */
	Tactical_window_object = Add_object
	(
		Earth_object,
		&Tactical_window_Class,
		NULL,
		TACTICAL_X,
		TACTICAL_Y,
		TACTICAL_WIDTH,
		TACTICAL_HEIGHT
	);

	/* Check all actions */
	Check_party_combat_actions();

	/* Draw */
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);

	/* Clear permanent text window */
	Clear_permanent_text();

	/* End of battle ? */
	if (Battle_ended)
	{
		/* Yes -> Exit combat screen */
		End_combat_flag = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : End_of_battle
 * FUNCTION  : Determine if the battle has ended.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 10:41
 * LAST      : 08.03.95 10:41
 * INPUTS    : None.
 * RESULT    : BOOLEAN : End of battle.
 * BUGS      : No known.
 * NOTES     : - This function will set the combat status variable.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
End_of_battle(void)
{
	BOOLEAN Result = FALSE;

	/* Any party members left ? */
	if (Count_remaining_party_members())
	{
		/* Yes -> All monsters gone ? */
		if (!Count_remaining_monsters())
		{
			/* Yes -> Party won! */
			Combat_status = COMBAT_PARTY_WON;

			/* End of battle */
			Result = TRUE;
		}
	}
	else
	{
		/* No -> Did any flee ? */
		if (Count_fled_party_members())
		{
			/* Yes -> Party fled! */
			Combat_status = COMBAT_PARTY_FLED;

			/* End of battle */
			Result = TRUE;
		}
		else
		{
			/* No -> Party lost! */
			Combat_status = COMBAT_PARTY_LOST;

			/* End of battle */
			Result = TRUE;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_control_status
 * FUNCTION  : Update the control status of all current combat participants.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 10:11
 * LAST      : 12.10.95 22:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the participant is removed, but is still in the speed
 *              list, Remove_participant() makes sure everything works out
 *              fine because it clears the participant's action.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_control_status(void)
{
	struct Combat_participant *Part;
	UNSHORT Conditions;
	UNSHORT i, j;

	/* Search the matrix for participants */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here in the matrix ? */
			if (Combat_matrix[i][j].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Get conditions of participant */
				Conditions = Get_conditions(Part->Char_handle);

				/* Party or monster ? */
				if (Part->Type == PARTY_PART_TYPE)
				{
					/* Party -> Can participate ? */
					if (Conditions & FIGHTING_MASK)
					{
						/* No -> Remove participant */
						Remove_participant(Part);
					}
					else
					{
						/* Yes -> Controllable ? */
						if (Conditions & CONTROL_MASK)
						{
							/* No -> Is this new information ? */
							if (!(Part->Flags & PART_UNCONTROL))
							{
								/* Yes -> Set flag */
								Part->Flags |= PART_UNCONTROL;

								/* Reset action */
								Part->Current_action = NO_COMACT;
							}
						}
					}
				}
				else
				{
					/* Monster -> Can participate ? */
					if (Conditions & FIGHTING_MASK)
					{
						/* No -> Remove participant */
						Remove_participant(Part);
					}
					else
					{
						/* Yes -> Controllable ? */
						if (Conditions & CONTROL_MASK)
						{
							/* No -> Is this new information ? */
							if (!(Part->Flags & PART_UNCONTROL))
							{
								/* Yes -> Set flag */
								Part->Flags |= PART_UNCONTROL;

								/* Reset action */
								Part->Current_action = NO_COMACT;
							}
						}
						else
						{
							/* Yes -> Clear flag */
							Part->Flags &= ~PART_UNCONTROL;
						}
					}
				}
			}
		}
	}

	/* Is the active member still controllable ? */
	if (Party_parts[PARTY_DATA.Active_member - 1].Flags & PART_UNCONTROL)
	{
		/* No -> Select a new active member */
		Select_new_active_member();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_participant
 * FUNCTION  : Remove a participant from combat.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 10:49
 * LAST      : 25.10.95 13:16
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the participant is removed, but is still in the speed
 *              list, Remove_participant() makes sure everything works out
 *              fine because it clears the participant's action.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_participant(struct Combat_participant *Part)
{
	/* Is the participant still in the matrix ? */
	if ((Part->Tactical_X != 0xFFFF) && (Part->Tactical_Y != 0xFFFF))
	{
		/* Yes -> Is the participant inside the combat matrix ? */
		if ((Part->Tactical_X < NR_TACTICAL_COLUMNS) &&
		 (Part->Tactical_Y < NR_TACTICAL_ROWS))
		{
			/* Yes -> Remove it from the matrix */
			Combat_matrix[Part->Tactical_Y][Part->Tactical_X].Part = NULL;
		}

		/* Clear tactical coordinates */
		Part->Tactical_X = 0xFFFF;
		Part->Tactical_Y = 0xFFFF;
	}

	/* Indicate the participant has been removed */
	Part->Flags |= PART_REMOVED;

	/* Reset action */
	Part->Current_action = NO_COMACT;

	/* Remove all temporary effects */
	Remove_all_temporary_effects(Part);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_combat_actions
 * FUNCTION  : Reset the actions of all participants.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 11:02
 * LAST      : 08.03.95 11:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will clear the actions, but not the targets.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_combat_actions(void)
{
	struct Combat_participant *Part;
	UNSHORT i, j;

	/* Search the matrix for participants */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here in the matrix ? */
			if (Combat_matrix[i][j].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Clear action */
				Part->Current_action = NO_COMACT;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle combat environment events.
 * FUNCTION  : Handle_combat_environment_events
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.06.95 15:04
 * LAST      : 23.10.95 22:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_combat_environment_events(void)
{
	UNSHORT X, Y;

	/* Check combat matrix */
	for (Y=0;Y<NR_TACTICAL_ROWS;Y++)
	{
		for (X=0;X<NR_TACTICAL_COLUMNS;X++)
		{
			/* Is there a trap AND a participant here in the matrix ? */
			if (Is_trap(X, Y) && Combat_matrix[Y][X].Part)
			{
				/* Yes -> Tell the player */
 				Print_combat_message
				(
					Combat_matrix[Y][X].Part,
					Combat_matrix[Y][X].Part,
					0,
					414
				);

				/* Execute trap */
				(Combat_matrix[Y][X].Trap_data.Trap_handler)
				(
					Combat_matrix[Y][X].Part,
				 	Combat_matrix[Y][X].Trap_data.Trap_strength,
				 	Combat_matrix[Y][X].Trap_data.Trap_COMOB
				);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle combat temporary effects event.
 * FUNCTION  : Handle_combat_temporary_effects_event
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.06.95 13:33
 * LAST      : 25.10.95 10:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_combat_temporary_effects_event(void)
{
	static const UNSHORT Effect_end_message_nrs[MAX_TEMP_EFFECTS] = {
		737, 740, 629, 738
	};

	struct Combat_participant *Part;
	UNSHORT Timer;
	UNSHORT i, j, k;

	/* Check combat matrix */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Get participant data */
			Part = Combat_matrix[i][j].Part;

			/* Is there a participant here in the matrix ? */
			if (Part)
			{
				/* Yes -> Check for temporary effects */
				for (k=0;k<MAX_TEMP_EFFECTS;k++)
				{
					/* Does the participant have this effect ? */
					Timer = Part->Temp_effect_data[k].Timer;
					if (Timer)
					{
						/* Yes -> Implement effect */
						switch (k)
						{
							/* Hurried */
							case HURRY_TEMP_EFFECT:
							{
								break;
							}
							/* Frozen */
							case FROZEN_TEMP_EFFECT:
							{
								break;
							}
							/* Blinded */
							case BLINDED_TEMP_EFFECT:
							{
								break;
							}
							/* Berserk */
							case BERSERKER_TEMP_EFFECT:
							{
								break;
							}
						}

 						/* Count down */
						Timer--;

						/* End of effect ? */
						if (Timer)
						{
							/* No -> Store new counter */
							Part->Temp_effect_data[k].Timer = Timer;
						}
						else
						{
							/* Yes -> Tell the player */
							Print_part_message(Part, Effect_end_message_nrs[k]);

							/* Remove effect
							  (timer will be cleared by Remove_temporary_effect) */
							Remove_temporary_effect(Part, k);
						}
					}
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_temporary_effect
 * FUNCTION  : Add a temporary effect to a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.06.95 13:39
 * LAST      : 25.10.95 16:48
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Effect_type - Effect type.
 *             UNSHORT Strength - Strength of effect (1...100).
 *             UNSHORT Duration - Duration of effect in combat rounds
 *              (for strength 100%).
 *             struct COMOB *Effect_COMOB - Pointer to effect COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will modify the duration of the effect
 *              depending on the strength.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_temporary_effect(struct Combat_participant *Part, UNSHORT Effect_type,
 UNSHORT Strength, UNSHORT Duration, struct COMOB *Effect_COMOB)
{
	static const UNSHORT Effect_add_message_nrs[MAX_TEMP_EFFECTS] = {
		419, 739, 627, 422
	};

	/* Exit if this an illegal effect type */
	if (Effect_type >= MAX_TEMP_EFFECTS)
		return;

	/* Modify duration */
	Duration = max((Strength * Duration) / 100, 1) + 1;

	/* Is this effect active ? */
	if (Part->Temp_effect_data[Effect_type].Timer)
	{
		/* Yes -> Delete effect COMOB */
		Delete_COMOB(Effect_COMOB);
	}
	else
	{
		/* No -> Initialize effect */
		switch(Effect_type)
		{
			/* Hurried */
			case HURRY_TEMP_EFFECT:
			{
				/* Double attacks per round */
				Part->Flags |= PART_HURRIED;
				break;
			}
			/* Frozen */
			case FROZEN_TEMP_EFFECT:
			{
				/* Lame victim */
				Set_condition(Part->Char_handle, LAMED);
				break;
			}
			/* Blinded */
			case BLINDED_TEMP_EFFECT:
			{
				/* Blind victim */
				Set_condition(Part->Char_handle, BLIND);
				break;
			}
			/* Berserk */
			case BERSERKER_TEMP_EFFECT:
			{
				struct Character_data *Char;
				UNSHORT Value;

				/* Remove 25% from LP */
				Value = Get_LP(Part->Char_handle);
				Value = (Value * 25) / 100;
				Do_combat_damage(Part, Value);

				/* Increase strength */
				Value = Get_real_attribute(Part->Char_handle, STRENGTH);
				Value = (Value * 150) / 100;
				Set_attribute(Part->Char_handle, STRENGTH, Value);

				/* Increase close-range attack skill */
				Value = Get_real_skill(Part->Char_handle, CLOSE_RANGE_ATTACK);
				Value = (Value * 150) / 100;
				Set_skill(Part->Char_handle, CLOSE_RANGE_ATTACK, Value);

				/* Increase long-range attack skill */
				Value = Get_real_skill(Part->Char_handle, LONG_RANGE_ATTACK);
				Value = (Value * 150) / 100;
				Set_skill(Part->Char_handle, LONG_RANGE_ATTACK, Value);

				/* Increase critical hit skill */
				Value = Get_real_skill(Part->Char_handle, CRITICAL_HIT);
				Value = (Value * 150) / 100;
				Set_skill(Part->Char_handle, CRITICAL_HIT, Value);

				/* Increase damage */
				Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);
				Char->xDamage = (Char->xDamage * 150) / 100;
				MEM_Free_pointer(Part->Char_handle);

				break;
			}
		}

		/* Tell the player */
		Print_part_message(Part, Effect_add_message_nrs[Effect_type]);

		/* Store pointer to effect COMOB */
		Part->Temp_effect_data[Effect_type].Effect_COMOB = Effect_COMOB;
	}

	/* Set the effect's duration */
	Part->Temp_effect_data[Effect_type].Timer = Duration;

	/* Set the effect's strength */
	Part->Temp_effect_data[Effect_type].Strength = Strength;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_temporary_effect
 * FUNCTION  : Remove a temporary effect from a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.06.95 13:40
 * LAST      : 25.10.95 11:02
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Effect_type - Effect type.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The removing of the effect will be shown.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_temporary_effect(struct Combat_participant *Part, UNSHORT Effect_type)
{
	/* Exit if this an illegal effect type */
	if (Effect_type >= MAX_TEMP_EFFECTS)
		return;

	/* Is this effect active ? */
	if (Part->Temp_effect_data[Effect_type].Timer)
	{
		/* Yes -> Remove it */
		Part->Temp_effect_data[Effect_type].Timer		= 0;
		Part->Temp_effect_data[Effect_type].Strength	= 0;

		/* Exit effect */
		switch(Effect_type)
		{
			/* Hurried */
			case HURRY_TEMP_EFFECT:
			{
				/* Attacks per round are no longer doubled */
				Part->Flags &= ~PART_HURRIED;
				break;
			}
			/* Frozen */
			case FROZEN_TEMP_EFFECT:
			{
				/* Remove lamed */
				Clear_condition(Part->Char_handle, LAMED);
				break;
			}
			/* Blinded */
			case BLINDED_TEMP_EFFECT:
			{
				/* Remove blindness */
				Clear_condition(Part->Char_handle, BLIND);
				break;
			}
			/* Berserk */
			case BERSERKER_TEMP_EFFECT:
			{
				struct Character_data *Char;
				UNSHORT Value;

				/* Decrease strength skill */
				Value = Get_real_attribute(Part->Char_handle, STRENGTH);
				Value = (Value * 100) / 150;
				Set_attribute(Part->Char_handle, STRENGTH, Value);

				/* Decrease close-range attack skill */
				Value = Get_real_skill(Part->Char_handle, CLOSE_RANGE_ATTACK);
				Value = (Value * 100) / 150;
				Set_skill(Part->Char_handle, CLOSE_RANGE_ATTACK, Value);

				/* Decrease long-range attack skill */
				Value = Get_real_skill(Part->Char_handle, LONG_RANGE_ATTACK);
				Value = (Value * 100) / 150;
				Set_skill(Part->Char_handle, LONG_RANGE_ATTACK, Value);

				/* Decrease critical hit skill */
				Value = Get_real_skill(Part->Char_handle, CRITICAL_HIT);
				Value = (Value * 100) / 150;
				Set_skill(Part->Char_handle, CRITICAL_HIT, Value);

				/* Decrease damage */
				Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);
				Char->xDamage = (Char->xDamage * 100) / 150;
				MEM_Free_pointer(Part->Char_handle);

				break;
			}
		}

		/* Delete effect COMOB (if any) */
		if (Part->Temp_effect_data[Effect_type].Effect_COMOB)
			Delete_COMOB(Part->Temp_effect_data[Effect_type].Effect_COMOB);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_all_temporary_effects
 * FUNCTION  : Remove all temporary effects from a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.06.95 13:41
 * LAST      : 08.06.95 13:41
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes that Remove_temporary_effect will
 *              check whether the effect is actually active.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_all_temporary_effects(struct Combat_participant *Part)
{
	UNSHORT i;

	/* Remove all effects */
	for (i=0;i<MAX_TEMP_EFFECTS;i++)
	{
		Remove_temporary_effect(Part, i);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Determine_monster_actions
 * FUNCTION  : Determine monster actions.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 17:14
 * LAST      : 07.03.95 17:14
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Determine_monster_actions(void)
{
	UNSHORT i;

	for (i=0;i<Nr_monsters;i++)
	{
		/* Anyone there ? */
		if (Monster_parts[i].Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Controllable ? */
			if (!(Monster_parts[i].Flags & PART_UNCONTROL))
			{
				/* Yes -> Select an action */
				Select_monster_action(&Monster_parts[i]);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Determine_uncontrollable_actions
 * FUNCTION  : Determine uncontrollable participants' actions.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 17:00
 * LAST      : 07.03.95 17:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Determine_uncontrollable_actions(void)
{
	UNSHORT i;

	/* Check party members for uncontrollables */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Party_parts[i].Type == PARTY_PART_TYPE)
		{
			/* Yes -> Uncontrollable ? */
			if (Party_parts[i].Flags & PART_UNCONTROL)
			{
				/* Yes -> Select an action */
				Select_uncontrollable_action(&Party_parts[i]);
			}
		}
	}

	/* Check monsters for uncontrollables */
	for (i=0;i<Nr_monsters;i++)
	{
		/* Anyone there ? */
		if (Monster_parts[i].Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Uncontrollable ? */
			if (Monster_parts[i].Flags & PART_UNCONTROL)
			{
				/* Yes -> Select an action */
				Select_uncontrollable_action(&Monster_parts[i]);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_uncontrollable_action
 * FUNCTION  : Select an action for an uncontrollable participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 20:14
 * LAST      : 06.10.95 20:14
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_uncontrollable_action(struct Combat_participant *Part)
{
	UNSHORT Conditions;

	/* Clear action */
	Part->Current_action = NO_COMACT;

	/* Get conditions */
	Conditions = Get_conditions(Part->Char_handle);

	/* Asleep ? */
	if (!(Conditions & (1 << ASLEEP)))
	{
		/* No -> Panicked ? */
		if (Conditions & (1 << PANICKED))
		{
			/* Yes -> Select panicked action */
			Select_panicked_action(Part);
		}
		else
		{
			/* No -> Insane ? */
			if (Conditions & (1 << INSANE))
			{
				/* Yes -> Select insane action */
				Select_crazy_action(Part);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_panicked_action
 * FUNCTION  : Select an action for a panicked participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 20:19
 * LAST      : 06.10.95 20:19
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_panicked_action(struct Combat_participant *Part)
{
	UNLONG Movement_range_mask;
	UNSHORT Nr_steps;
	UNSHORT X, Y;
	UNSHORT Tactic_index;
	UNSHORT Step_index;
	UNSHORT New_X;

	/* Is the participant inside the combat matrix ? */
	if ((Part->Tactical_X >= NR_TACTICAL_COLUMNS) ||
	 (Part->Tactical_Y >= NR_TACTICAL_ROWS))
	{
		/* No -> Exit */
		return;
	}

	/* Is this participant a monster or a party member ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party member -> Is in the bottom row / can flee ? */
		if ((Part->Tactical_Y == 4) &&
		 !(Get_conditions(Part->Char_handle) & FLEE_MASK))
		{
			/* Yes -> Flee */
			Part->Current_action = FLEE_COMACT;
		}
		else
		{
			/* No -> Get movement range */
			Movement_range_mask = Get_movement_range(Part) &
			 ~(Get_occupied_move_targets(Part));

			/* Any ? */
			if (Movement_range_mask)
			{
				/* Yes -> Clear number of moves */
				Part->Target.Move_target_data.Nr_moves = 0;

				/* Get number of moves of participant */
				Nr_steps = Get_nr_moves(Part);

				/* Get coordinates of party member */
				X = Part->Tactical_X;
				Y = Part->Tactical_Y;

				/* Calculate tactical square index */
				Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

				/* Try to retreat */
				Step_index = 0;
				while (Step_index < Nr_steps)
				{
					/* In the bottom row ? */
					if (Y == 4)
					{
						/* Yes -> Exit */
						break;
					}
					else
					{
						/* No -> Moving down possible ? */
						if (Movement_range_mask &
						 (1L << (Tactic_index + NR_TACTICAL_COLUMNS)))
						{
							/* Yes -> Move down */
							Y++;
						}
						else
						{
							/* No -> Is a diagonal move down possible ? */
							New_X = Make_defensive_horizontal_move
							(
								X,
								Y + 1,
								Movement_range_mask
							);
							if (X != New_X)
							{
								/* Yes -> Move diagonally */
								X = New_X;
								Y++;
							}
							else
							{
								/* No -> Is a horizontal move possible ? */
								New_X = Make_defensive_horizontal_move
								(
									X,
									Y,
									Movement_range_mask
								);
								if (X != New_X)
								{
									/* Yes -> Move horizontally */
									X = New_X;
								}
								else
								{
									/* No -> Exit */
									break;
								}
							}
						}
					}

					/* No turning back! */
					Movement_range_mask &= ~(1L << Tactic_index);

					/* Calculate new tactical square index */
					Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

					/* Store */
					Part->Target.Move_target_data.Target_square_indices[Step_index] = Tactic_index;

					/* Count up */
					Step_index++;
				}

				/* Were any moves entered ? */
				if (Step_index)
				{
					/* Yes -> Set action */
					Part->Current_action = MOVE_COMACT;

					/* Store number of moves */
					Part->Target.Move_target_data.Nr_moves = Step_index;
				}
			}
		}
	}
	else
	{
		/* Monster -> Select normal defensive action */
		Select_defensive_monster_action(Part);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_crazy_action
 * FUNCTION  : Select an action for a crazy participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 20:25
 * LAST      : 06.10.95 20:25
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_crazy_action(struct Combat_participant *Part)
{
	/* Move or attack ? */
	if ((rand() % 8) >= 4)
	{
		/* Move -> Try */
		Try_crazy_move(Part);

		/* Success ? */
		if (Part->Current_action == NO_COMACT)
		{
			/* No -> Try attack */
			Try_crazy_attack(Part);
		}
	}
	else
	{
		/* Attack -> Try */
		Try_crazy_attack(Part);

		/* Success ? */
		if (Part->Current_action == NO_COMACT)
		{
			/* No -> Try move */
			Try_crazy_move(Part);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_crazy_move
 * FUNCTION  : Try to move a crazy participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 20:25
 * LAST      : 06.10.95 20:25
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Try_crazy_move(struct Combat_participant *Part)
{
	UNLONG Movement_range_mask;
	UNSHORT Target_square_index;

	/* Get movement range */
	Movement_range_mask = Get_movement_range(Part) &
	 ~(Get_occupied_move_targets(Part));

	/* Any ? */
	if (Movement_range_mask)
	{
		/* Yes -> Select a target */
		Target_square_index = Choose_monster_target(Movement_range_mask);

		/* Any target selected ? */
		if (Target_square_index != 0xFFFF)
		{
			/* Yes -> Insert action and target data */
			Part->Current_action = MOVE_COMACT;
			Part->Target.Move_target_data.Nr_moves = 1;
			Part->Target.Move_target_data.Target_square_indices[0] = Target_square_index;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_crazy_attack
 * FUNCTION  : Try to let a crazy participant attack.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.10.95 20:37
 * LAST      : 06.10.95 20:43
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Try_crazy_attack(struct Combat_participant *Part)
{
	UNLONG Attack_mask;
	UNSHORT Action = NO_COMACT;
	UNSHORT Target_square_index = 0xFFFF;
	UNSHORT Weapon_slot_index;
	UNSHORT Old_part_type;

	/* Search the participant's body for long-range weapons */
	Weapon_slot_index = Search_body_for_item_type
	(
		Part->Char_handle,
		LONG_RANGE_IT
	);

	/* Is the participant carrying a long-range weapon ? */
	if (Weapon_slot_index != 0xFFFF)
	{
		/* Yes -> Does it have the right ammo ? */
		if (Check_if_long_range_weapon_has_ammo
		(
			Part,
			Weapon_slot_index
		))
		{
			/* Yes -> Get potential long-range targets
			  (temporarily changing the participant type so everyone's an enemy) */
			Old_part_type = Part->Type;
			Part->Type = CRAZY_PART_TYPE;
			Attack_mask = Get_long_range_targets(Part);
			Part->Type = Old_part_type;

			/* Any ? */
			if (Attack_mask)
			{
				/* Yes -> Select one */
				Target_square_index = Choose_monster_target(Attack_mask);

				/* Long-range attack */
				Action = LONG_RANGE_COMACT;
			}
		}
	}
	else
	{
		/* No -> Search the participant's body for close-range weapons */
		Weapon_slot_index = Search_body_for_item_type
		(
			Part->Char_handle,
			CLOSE_RANGE_IT
		);

		/* Get potential close-range targets
		  (temporarily changing the participant type so everyone's an enemy) */
		Old_part_type = Part->Type;
		Part->Type = CRAZY_PART_TYPE;
		Attack_mask = Get_close_range_targets(Part);
		Part->Type = Old_part_type;

		/* Any ? */
		if (Attack_mask)
		{
			/* Yes -> Select one */
			Target_square_index = Choose_monster_target(Attack_mask);

			/* Close-range attack */
			Action = CLOSE_RANGE_COMACT;
		}
	}

	/* Any action / any target selected ? */
	if ((Action != NO_COMACT) && (Target_square_index != 0xFFFF))
	{
		/* Yes -> Insert action and target data */
		Part->Current_action = Action;
		Part->Target.Attack_target_data.Target_square_index		= Target_square_index;
		Part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_slot_index;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Create_sorted_combat_event_list
 * FUNCTION  : Create a sorted list with all combat events.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:56
 * LAST      : 07.03.95 16:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Create_sorted_combat_event_list(void)
{
	UNSHORT Speed;
	UNSHORT i;

	/* Empty the combat event list */
	Nr_combat_events = 0;

	/* Insert party member actions in combat event list */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Party_parts[i].Type == PARTY_PART_TYPE)
		{
			/* Yes -> Get party member's speed factor */
			Speed = Get_attribute(Party_parts[i].Char_handle, SPEED);

			/* Insert participant data in combat event list */
			Combat_event_list[Nr_combat_events].Type	= PART_COMBAT_EVENT;
			Combat_event_list[Nr_combat_events].Speed	= Speed;
			Combat_event_list[Nr_combat_events].Part	= &Party_parts[i];

			/* Count up */
			Nr_combat_events++;
		}
	}

	/* Insert monster actions in combat event list */
	for (i=0;i<Nr_monsters;i++)
	{
		/* Anyone there ? */
		if (Monster_parts[i].Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Get monster's speed factor */
			Speed = Get_attribute(Monster_parts[i].Char_handle, SPEED);

			/* Insert participant data in combat event list */
			Combat_event_list[Nr_combat_events].Type	= PART_COMBAT_EVENT;
			Combat_event_list[Nr_combat_events].Speed	= Speed;
			Combat_event_list[Nr_combat_events].Part	= &Monster_parts[i];

			/* Count up */
			Nr_combat_events++;
		}
	}

	/* Insert environment event in combat event list */
	Combat_event_list[Nr_combat_events].Type	= ENVIRONMENT_COMBAT_EVENT;
	Combat_event_list[Nr_combat_events].Speed	= ENVIRONMENT_SPEED;
	Combat_event_list[Nr_combat_events].Part	= NULL;

	/* Count up */
	Nr_combat_events++;

	/* Insert temporary effects event in combat event list */
	Combat_event_list[Nr_combat_events].Type	= TEMPORARY_EFFECTS_COMBAT_EVENT;
	Combat_event_list[Nr_combat_events].Speed	= TEMPORARY_EFFECTS_SPEED;
	Combat_event_list[Nr_combat_events].Part	= NULL;

	/* Count up */
	Nr_combat_events++;

	/* Sort combat event list */
	Shellsort
	(
		Swap_combat_events,
		Compare_combat_events,
		Nr_combat_events,
		NULL
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_combat_events
 * FUNCTION  : Swap two combat events (for sorting).
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:57
 * LAST      : 07.03.95 16:57
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_combat_events(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Combat_event T;

	memcpy((UNBYTE *) &T, (UNBYTE *) &Combat_event_list[A],
	 sizeof(struct Combat_event));
	memcpy((UNBYTE *) &Combat_event_list[A], (UNBYTE *) &Combat_event_list[B],
	 sizeof(struct Combat_event));
	memcpy((UNBYTE *) &Combat_event_list[B], (UNBYTE *) &T,
	 sizeof(struct Combat_event));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_combat_events
 * FUNCTION  : Compare two combat events (for sorting).
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:58
 * LAST      : 07.03.95 16:58
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_combat_events(UNLONG A, UNLONG B, UNBYTE *Data)
{
	return(Combat_event_list[A].Speed < Combat_event_list[B].Speed);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_party_combat_data
 * FUNCTION  : Init party combat data.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 10:40
 * LAST      : 23.10.95 21:13
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Party combat data was initialized successfully.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_party_combat_data(void)
{
	BOOLEAN Result;
	UNSHORT Position;
	UNSHORT X, Y;
	UNSHORT i;

	/* Turn party members into combat participants */
	Result = TRUE;
	for (i=0;i<6;i++)
	{
		/* Clear party member participant data */
		BASEMEM_FillMemByte((UNBYTE *) &Party_parts[i],
		 sizeof(struct Combat_participant), 0);

		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Capable of fighting ? */
			if (!(Get_conditions(Party_char_handles[i]) & FIGHTING_MASK))
			{
				/* Yes -> Set participant data */
				Party_parts[i].Type			= PARTY_PART_TYPE;
				Party_parts[i].Number		= i + 1;

				Party_parts[i].Char_handle	= Party_char_handles[i];

				/* Get battle order position */
				Position = PARTY_DATA.Battle_order[i] +
				 (3 * NR_TACTICAL_COLUMNS);

				/* Calculate tactical coordinates */
				X = Position % NR_TACTICAL_COLUMNS;
				Y = Position / NR_TACTICAL_COLUMNS;

				/* Set tactical coordinates */
				Party_parts[i].Tactical_X = X;
				Party_parts[i].Tactical_Y = Y;

				/* Insert party member in combat matrix */
				Combat_matrix[Y][X].Part = &Party_parts[i];

				/* Load tactical icon graphics */
				Party_parts[i].Tactical_icon_handle = Load_subfile(TACTICAL_ICONS,
				 PARTY_DATA.Member_nrs[i]);
				if (!Party_parts[i].Tactical_icon_handle)
				{
					Error(ERROR_FILE_LOAD);
					Result = FALSE;
					break;
				}
			}
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_party_combat_data
 * FUNCTION  : Exit party combat data.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.07.95 13:27
 * LAST      : 17.07.95 13:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_party_combat_data(void)
{
	UNSHORT i;

	/* Free party member tactical icons */
	for (i=0;i<6;i++)
	{
		/* Any there ? */
		if (Party_parts[i].Tactical_icon_handle)
		{
			/* Yes -> Free tactical icon graphics memory */
			MEM_Free_memory(Party_parts[i].Tactical_icon_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_monster_combat_data
 * FUNCTION  : Init monster combat data.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 18:29
 * LAST      : 24.10.95 17:32
 * INPUTS    : UNSHORT Monster_group_nr - Monster group number.
 * RESULT    : BOOLEAN : Monster data was initialized successfully.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_monster_combat_data(UNSHORT Monster_group_nr)
{
	struct Character_data *Char;
	MEM_HANDLE Monster_group_handle;
	MEM_HANDLE Monster_char_data_handles[MONSTERS_PER_GROUP];
	BOOLEAN Success;
	BOOLEAN Result;
	UNSHORT *Monster_group;
	UNSHORT Batch[MONSTERS_PER_GROUP];
	UNSHORT Counter;
	UNSHORT Index;
	UNSHORT X, Y;
	UNSHORT i;

	/* Clear monster participant data */
	for (i=0;i<MONSTERS_PER_GROUP;i++)
	{
		BASEMEM_FillMemByte((UNBYTE *) &Monster_parts[i],
		 sizeof(struct Combat_participant), 0);

		Monster_parts[i].Type = EMPTY_PART_TYPE;
	}

	/* Load monster group data */
	Monster_group_handle = Load_subfile(MONSTER_GROUP, Monster_group_nr);
	if (!Monster_group_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}
	Monster_group = (UNSHORT *) MEM_Claim_pointer(Monster_group_handle);

	/* Build monster character data batch */
	Counter = 0;
	for (i=0;i<MONSTERS_PER_GROUP;i++)
	{
		/* Anything there ? */
		if (Monster_group[i])
		{
			/* Yes -> Insert in batch */
			Batch[Counter] = Monster_group[i];

			/* Count up */
			Counter++;
		}
	}

	/* Store counter */
	Nr_monsters = Counter;

	/* Any monsters ? */
	if (!Nr_monsters)
	{
		/* No -> Error */
		Error(ERROR_EMPTY_MONSTER_GROUP);

		/* Destroy monster group data */
		MEM_Free_pointer(Monster_group_handle);
		MEM_Free_memory(Monster_group_handle);

		return FALSE;
	}

	/* Load batch of monster character data */
	Success = Load_full_batch(MONSTER_CHAR, Nr_monsters, &(Batch[0]),
	 &(Monster_char_data_handles[0]));
	if (!Success)
	{
		Error(ERROR_FILE_LOAD);

		return FALSE;
	}

	/* Turn monsters into combat participants */
	Index = 0;
	for (i=0;i<MONSTERS_PER_GROUP;i++)
	{
		/* Anything there ? */
		if (Monster_group[i])
		{
			/* Yes -> Set participant data */
			Monster_parts[Index].Type		= MONSTER_PART_TYPE;
			Monster_parts[Index].Number	= Index + 1;

			/* Set monster show set and tactic index */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Monster_char_data_handles[Index]);

			Monster_parts[Index].Show_set_index	= (UNSHORT) Char->Show_set_index;
			Monster_parts[Index].Tactic_index	= (UNSHORT) Char->Tactic_index;

			MEM_Free_pointer(Monster_char_data_handles[Index]);

			/* If show set index is zero, set to one (default for monsters) */
			if (!Monster_parts[Index].Show_set_index)
			{
				Monster_parts[Index].Show_set_index = 1;
			}

			/* Clone monster data */
			Monster_parts[Index].Char_handle =
			 Clone_monster_data(Monster_char_data_handles[Index]);

			/* Destroy original monster data */
			MEM_Free_memory(Monster_char_data_handles[Index]);

			/* Give monster close- and long-range capabilities */
			Monster_parts[Index].Capabilities |= PART_CLOSE_CAP | PART_LONG_CAP;

			/* Know any spells ? */
			if (Character_has_magical_abilities(Monster_parts[Index].Char_handle))
			{
				/* Yes -> Give capability */
				Monster_parts[Index].Capabilities |= PART_MAGIC_CAP;
			}

			/* Calculate tactical coordinates */
			X = i % NR_TACTICAL_COLUMNS;
			Y = i / NR_TACTICAL_COLUMNS;

			/* Set tactical coordinates */
			Monster_parts[Index].Tactical_X = X;
			Monster_parts[Index].Tactical_Y = Y;

			/* Insert monster in combat matrix */
			Combat_matrix[Y][X].Part = &Monster_parts[Index];

			/* Next monster participant */
			Index++;
		}
	}

	/* Destroy monster group data */
	MEM_Free_pointer(Monster_group_handle);
	MEM_Free_memory(Monster_group_handle);

	/* Initialize show monsters */
	Result = TRUE;
	for (i=0;i<Nr_monsters;i++)
	{
		/* Init */
		Combat_show(&Monster_parts[i], SHOW_INIT, NULL);

		/* Error ? */
		if (!Monster_parts[i].Graphics_handle)
		{
			/* Yes -> Remove participant */
			Monster_parts[i].Type = EMPTY_PART_TYPE;

			/* Exit */
			Result = FALSE;
			break;
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_monster_combat_data
 * FUNCTION  : Exit monster combat data.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.07.95 13:26
 * LAST      : 17.07.95 13:26
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_monster_combat_data(void)
{
	UNSHORT i;

	/* Free monster memory */
	for (i=0;i<Nr_monsters;i++)
	{
		/* Anyone there ? */
		if (Monster_parts[i].Type != EMPTY_PART_TYPE)
		{
			/* Yes -> Free graphics memory */
			Combat_show(&Monster_parts[i], SHOW_EXIT, NULL);
		}

		/* Free cloned character data */
		MEM_Free_memory(Monster_parts[i].Char_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clone_monster_data
 * FUNCTION  : Create clone of monster character data
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 11:54
 * LAST      : 20.10.95 15:58
 * INPUTS    : MEM_HANDLE Original_data_handle - Handle of original character
 *              data.
 * RESULT    : MEM_HANDLE : Handle of cloned data.
 * BUGS      : No known.
 * NOTES     : - This function duplicates the monster character data, enters
 *              the maxima for skills, attributes, LP and PP and randomizes
 *              the normal values for these by -5% to +5%.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Clone_monster_data(MEM_HANDLE Original_data_handle)
{
	MEM_HANDLE Cloned_data_handle;
	SISHORT Value;
	SISHORT Maximum;
	UNSHORT i;
	UNBYTE *Source;
	UNBYTE *Target;

	/* Allocate cloned data */
	Cloned_data_handle = MEM_Allocate_memory(sizeof(struct Character_data));

	/* Copy original to cloned data */
	Source = MEM_Claim_pointer(Original_data_handle);
	Target = MEM_Claim_pointer(Cloned_data_handle);

	memcpy(Target, Source, sizeof(struct Character_data));

	MEM_Free_pointer(Cloned_data_handle);
	MEM_Free_pointer(Original_data_handle);

	/* Enter attribute maxima & randomize attributes
	  (one less because age is unimportant) */
	for (i=0;i<MAX_ATTRS - 1;i++)
	{
		/* Get current attribute value */
		Maximum = Get_real_attribute(Cloned_data_handle, i);

		/* Enter attribute maximum */
		Set_max_attribute(Cloned_data_handle, i, Maximum);

		/* Randomize attribute value */
	 	Value = (Maximum * (100 + (rand() % 11) - 5)) / 100;

		/* Clip attribute value */
		Value = max(min(Value, 0), Maximum);

		/* Store new attribute value */
		Set_attribute(Cloned_data_handle, i, Value);
	}

	/* Enter skill maxima & randomize skills */
	for (i=0;i<MAX_SKILLS;i++)
	{
		/* Get current skill value */
		Maximum = Get_real_skill(Cloned_data_handle, i);

		/* Enter as skill maximum */
		Set_max_skill(Cloned_data_handle, i, Maximum);

		/* Randomize skill value */
	 	Value = (Maximum * (100 + (rand() % 11) - 5)) / 100;

		/* Clip skill value */
		Value = max(min(Value, 0), Maximum);

		/* Store new skill value */
		Set_skill(Cloned_data_handle, i, Value);
	}

	/* Get current LP value */
	Value = Get_LP(Cloned_data_handle);

	/* Set LP maximum */
	Set_max_LP(Cloned_data_handle, Value);

	/* Randomize LP value */
	Value = (Value * (100 + (rand() % 11) - 5)) / 100;

	/* Set new LP value */
	Set_LP(Cloned_data_handle, Value);

	/* Get current SP value */
	Value = Get_SP(Cloned_data_handle);

	/* Set SP maximum */
	Set_max_SP(Cloned_data_handle, Value);

	return Cloned_data_handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_monster_appear_animations
 * FUNCTION  : Show each monster's appear animation.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 11:54
 * LAST      : 21.09.95 15:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_monster_appear_animations(void)
{
	UNSHORT i;

	Fighting = TRUE;

	/* Check all monsters */
	for (i=0;i<Nr_monsters;i++)
	{
		/* Anyone there ? */
		if (Monster_parts[i].Type == MONSTER_PART_TYPE)
		{
			/* Yes */
			Combat_show(&Monster_parts[i], SHOW_APPEAR, NULL);
		}
	}

	/* Wait for a short while */
	for (i=0;i<20;i++)
		Update_screen();

	Fighting = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_combat_animation
 * FUNCTION  : Set a participant's animation.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 19:38
 * LAST      : 27.10.95 14:59
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Anim_type - Animation type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_combat_animation(struct Combat_participant *Part, UNSHORT Anim_type)
{
	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Has this animation ? */
		if (Has_combat_animation(Part, Anim_type))
		{
			/* Yes -> Set animation type */
			Part->Anim_type = Anim_type;
		}

		/* Reset animation */
		Part->Anim_index = 0;
		Part->Flags &= ~PART_WAVEDIR;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Has_combat_animation
 * FUNCTION  : Check if a participant has a certain combat animation.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 11:38
 * LAST      : 10.05.95 11:38
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Anim_type - Animation type.
 * RESULT    : BOOLEAN : Has animation.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Has_combat_animation(struct Combat_participant *Part, UNSHORT Anim_type)
{
	struct Character_data *Char;
	BOOLEAN Result = FALSE;

	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Get monster character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		/* Has animation ? */
		if (Char->Anim_lengths[Anim_type])
			Result = TRUE;

		MEM_Free_pointer(Part->Char_handle);
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_combat_animation_length
 * FUNCTION  : Get the length of a participant's combat animation.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 22:39
 * LAST      : 23.10.95 22:39
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Anim_type - Animation type.
 * RESULT    : UNSHORT : Animation length (taking wave animations into account).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_combat_animation_length(struct Combat_participant *Part, UNSHORT Anim_type)
{
	struct Character_data *Char;
	UNSHORT Nr_frames = 0;

	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Get monster character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		/* Get number of frames for movement animation */
		Nr_frames = (UNSHORT) Char->Anim_lengths[Anim_type];

		/* Any animation / wave animation ? */
		if (Nr_frames && (Char->Anim_motion & (1 << Anim_type)))
		{
			/* Yes -> More frames */
			Nr_frames += Nr_frames - 2;
		}

		MEM_Free_pointer(Part->Char_handle);
	}

	/* At least one animation frame */
	return max(Nr_frames, 1);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_combat_animation
 * FUNCTION  : Wait for the end of a combat animation.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 10:53
 * LAST      : 07.03.95 10:53
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_combat_animation(struct Combat_participant *Part)
{
	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Wait for animation to end */
		while (Part->Anim_type != NO_COMANIM)
		{
			Update_screen();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_4_half_combat_animation
 * FUNCTION  : Wait for until the first half of a combat animation has passed.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 22:46
 * LAST      : 23.10.95 22:49
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_4_half_combat_animation(struct Combat_participant *Part)
{
	Wait_4_combat_animation(Part);

	#if FALSE
	struct Character_data *Char;
	UNSHORT Anim_type;
	UNSHORT Nr_frames;

	/* Is a monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Get animation type */
		Anim_type = Part->Anim_type;

		/* Any animation ? */
		if (Anim_type != NO_COMANIM)
		{
			/* Yes -> Get monster character data */
			Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

			/* Get number of frames for movement animation */
			Nr_frames = (UNSHORT) Char->Anim_lengths[Anim_type];

			/* Wave or circle animation ? */
			if (Char->Anim_motion & (1 << Anim_type))
			{
				/* Wave */
				if (Nr_frames)
					Nr_frames -= 1;
			}
			else
			{
				/* Circle */
				Nr_frames /= 2;
			}

			MEM_Free_pointer(Part->Char_handle);

			/* Wait for half of animation */
			while(Part->Anim_index < Nr_frames)
			{
				Update_screen();
			}
		}
	}
	#endif
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_combat_animations
 * FUNCTION  : Update combat animations.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 10:56
 * LAST      : 07.03.95 10:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_combat_animations(void)
{
	UNSHORT i;

	/* Animate all monsters */
	for (i=0;i<Nr_monsters;i++)
	{
		/* Still there ? */
		if (Monster_parts[i].Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Hand-animated ? */
			if (!(Monster_parts[i].Flags & PART_HAND_ANIMATED))
			{
				/* No -> Update monster animation */
				Update_monster_animation(&Monster_parts[i]);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_monster_animation
 * FUNCTION  : Update a monster's animation.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 11:00
 * LAST      : 07.03.95 11:00
 * INPUTS    : struct Combat_participant *Monster_part - Pointer to
 *              participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_monster_animation(struct Combat_participant *Monster_part)
{
	struct Character_data *Char;
	UNSHORT Anim_type;
	SISHORT Frame_index;
	UNSHORT Nr_frames;
	UNSHORT New_frame;

	/* Get animation type */
	Anim_type = Monster_part->Anim_type;

	/* Any animation ? */
	if (Anim_type != NO_COMANIM)
	{
		/* Yes -> Get frame index */
		Frame_index = Monster_part->Anim_index;

		/* Get monster character data */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Monster_part->Char_handle);

		/* Get number of frames for current animation type */
		Nr_frames = (UNSHORT) Char->Anim_lengths[Anim_type];

		/* Any frames ? */
		if (Nr_frames)
		{
			/* Yes -> Wave or circle animation ? */
			if ((Char->Anim_motion & (1 << Anim_type)) && (Nr_frames > 2))
			{
				/* Wave -> Back or forth ? */
				if (Monster_part->Flags & PART_WAVEDIR)
				{
					/* Back -> Previous frame */
					Frame_index--;

					/* Reached first frame ? */
					if (Frame_index < 1)
					{
						/* Yes -> Forth next time */
						Monster_part->Flags &= ~PART_WAVEDIR;

						/* End animation */
						Monster_part->Anim_type = NO_COMANIM;
					}
				}
				else
				{
					/* Forth -> Next frame */
					Frame_index++;

					/* Reached last frame ? */
					if (Frame_index == Nr_frames)
					{
						/* Yes -> Reverse direction */
						Monster_part->Flags |= PART_WAVEDIR;
						Frame_index -= 2;
					}
  				}
			}
			else
			{
				/* Circle -> Next frame */
				Frame_index++;

				/* Reached last frame ? */
				if (Frame_index >= Nr_frames)
				{
					/* Yes -> End animation */
					Monster_part->Anim_type = NO_COMANIM;
				}
				else
				{
					/* No -> Get new frame */
					New_frame = (UNSHORT) Char->Anim_sequences[Anim_type][Frame_index];
				}
			}
		}
		else
		{
			/* No -> End animation */
			Monster_part->Anim_type = NO_COMANIM;
		}

		/* Has the animation ended ? */
		if (Monster_part->Anim_type == NO_COMANIM)
		{
			/* Yes -> Switch to default frame */
			New_frame = Monster_part->Default_frame;

			/* Reset frame index */
			Frame_index = 0;
		}
		else
		{
			/* No -> Get new frame */
			New_frame = (UNSHORT) Char->Anim_sequences[Anim_type][Frame_index];
		}

		MEM_Free_pointer(Monster_part->Char_handle);

		/* Set new animation index */
		Monster_part->Anim_index = Frame_index;

		/* Set new animation frame */
		Monster_part->Main_COMOB->Frame = New_frame * 2;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_close_range_targets
 * FUNCTION  : Make bitlist of close-range attack targets.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 20:43
 * LAST      : 12.04.95 20:43
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to attacker
 *              participant data.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_close_range_targets(struct Combat_participant *Attacker_part)
{
	return (Do_close_range_targets
	(
		Attacker_part->Tactical_X,
		Attacker_part->Tactical_Y,
		Attacker_part->Type
	));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_close_range_targets
 * FUNCTION  : Make bitlist of close-range attack targets.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 20:49
 * LAST      : 12.04.95 20:49
 * INPUTS    : UNSHORT Tactical_X - Current tactical X-coordinate.
 *             UNSHORT Tactical_Y - Current tactical Y-coordinate.
 *             UNSHORT Attacker_type - Attacker participant type.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Do_close_range_targets(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNSHORT Attacker_type)
{
	struct Combat_participant *Part;
	UNLONG Mask = 0;
	SISHORT X, Y;
	UNSHORT i;

	/* Are the source coordinates inside the combat matrix ? */
	if ((Tactical_X >= NR_TACTICAL_COLUMNS) ||
	 (Tactical_Y >= NR_TACTICAL_ROWS))
	{
		/* No -> Exit */
		return 0;
	}

	/* Look around participant */
	for(i=0;i<8;i++)
	{
		/* Get coordinates */
		X = Tactical_X + Offsets8[i][0];
		Y = Tactical_Y + Offsets8[i][1];

		/* Inside the combat matrix ? */
		if ((X >= 0) &&
		 (X < NR_TACTICAL_COLUMNS) &&
		 (Y >= 0) &&
		 (Y < NR_TACTICAL_ROWS))
		{
			/* Yes -> Anything here in the matrix ? */
			if (Combat_matrix[Y][X].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[Y][X].Part;

				/* Is enemy ? */
				if (Part->Type && (Part->Type != Attacker_type))
				{
					/* Yes -> Mark */
					Mask |= (1L << (Y * NR_TACTICAL_COLUMNS + X));
				}
			}
		}
	}
	return Mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_long_range_targets
 * FUNCTION  : Make bitlist of long-range attack targets.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 15:50
 * LAST      : 12.04.95 15:50
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to attacker
 *              participant data.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_long_range_targets(struct Combat_participant *Attacker_part)
{
	struct Combat_participant *Part;
	UNLONG Mask = 0;
	UNSHORT i, j;

	/* Search the matrix for participants */
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here in the matrix ? */
			if (Combat_matrix[i][j].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Is enemy ? */
				if (Part->Type && (Part->Type != Attacker_part->Type))
				{
					/* Yes -> Mark */
					Mask |= (1L << (i * NR_TACTICAL_COLUMNS + j));
				}
			}
		}
	}
	return Mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_nr_moves
 * FUNCTION  : Calculate number of moves of a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.09.95 12:50
 * LAST      : 20.09.95 12:50
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNSHORT : Number of moves.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_nr_moves(struct Combat_participant *Part)
{
	return (min(max(Get_attribute(Part->Char_handle, SPEED) /
	 MOVE_SPEED_FACTOR, 1), MAX_COMBAT_MOVES));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_movement_range
 * FUNCTION  : Determine movement range.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.04.95 22:09
 * LAST      : 30.10.95 04:48
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_movement_range(struct Combat_participant *Part)
{
	BOOLEAN CA_not_completed;
	UNLONG Move_mask;
	UNLONG Allowed_mask;
	SISHORT X, Y;
	UNSHORT Nr_steps;
	UNSHORT Cell1, Cell2;
	UNSHORT i, j, k;
	UNBYTE Movement_cells[NR_TACTICAL_ROWS][NR_TACTICAL_COLUMNS];

	/* Is the participant inside the combat matrix ? */
	if ((Part->Tactical_X >= NR_TACTICAL_COLUMNS) ||
	 (Part->Tactical_Y >= NR_TACTICAL_ROWS))
	{
		/* No -> Exit */
		return 0;
	}

	/* Clear automatus */
	for(i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for(j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			Movement_cells[i][j] = 0;
		}
	}

	/* Get number of moves of participant */
	Nr_steps = Get_nr_moves(Part);

	/* Initialise automatus */
	Movement_cells[Part->Tactical_Y][Part->Tactical_X] = Nr_steps + 1;

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
		Allowed_mask = COMBAT_PARTY_MASK;
	else
		Allowed_mask = COMBAT_MONSTER_MASK;

	/* Do cellular automatus */
	do
	{
		/* Clear flag */
		CA_not_completed = FALSE;

		/* Check each cell */
		for(i=0;i<NR_TACTICAL_ROWS;i++)
		{
			for(j=0;j<NR_TACTICAL_COLUMNS;j++)
			{
				/* Is this cell allowed ? */
				if (Allowed_mask & (1L << (i * NR_TACTICAL_COLUMNS + j)))
				{
					/* Yes -> Is this cell empty ? */
					Cell1 = Movement_cells[i][j];
					if (Cell1)
					{
						/* No -> Subtract one */
						Cell1--;

						/* Look around */
						for(k=0;k<8;k++)
						{
							/* Get coordinates */
							X = j + Offsets8[k][0];
							Y = i + Offsets8[k][1];

							/* Inside the combat matrix ? */
							if ((X >= 0) &&
							 (X < NR_TACTICAL_COLUMNS) &&
							 (Y >= 0) &&
							 (Y < NR_TACTICAL_ROWS))
							{
								/* Yes -> Is this cell allowed ? */
								if (Allowed_mask & (1L << ((Y * NR_TACTICAL_COLUMNS) + X)))
								{
									/* Yes -> Get cell */
									Cell2 = Movement_cells[Y][X];

									/* Going up-hill ? */
									if (Cell2 < Cell1)
									{
										/* No -> Spread! */
										Movement_cells[Y][X] = Cell1;

										/* Set flag */
										CA_not_completed = TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	} while (CA_not_completed);

	/* Build movement mask */
	Move_mask = 0;
	for(i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for(j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Nothing here in the matrix / cell filled ? */
			if (!Combat_matrix[i][j].Part && Movement_cells[i][j])
			{
				/* Yes -> Mark */
				Move_mask |= (1L << ((i * NR_TACTICAL_COLUMNS) + j));
			}
		}
	}

	return Move_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_occupied_move_targets
 * FUNCTION  : Make bitlist of occupied movement targets.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.04.95 10:39
 * LAST      : 18.09.95 14:50
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * NOTES     : - This function only works for party members.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_occupied_move_targets(struct Combat_participant *Part)
{
	struct Combat_participant *Current;
	UNLONG Move_mask = 0;
	UNSHORT Target_square_index;
	UNSHORT i, j;

	/* Check matrix */
	for(i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for(j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here ? */
			Current = Combat_matrix[i][j].Part;
			if (Current)
			{
				/* Yes -> Is self ? */
				if (Current != Part)
				{
					/* No -> Is friend ? */
					if (Current->Type == Part->Type)
					{
						/* Yes -> About to move ? */
						if (Current->Current_action == MOVE_COMACT)
						{
							/* Yes -> Get movement target square index */
							Target_square_index = Current->Target.Move_target_data.Target_square_indices[Current->Target.Move_target_data.Nr_moves - 1];

							/* Mark target */
							Move_mask |= (1L << Target_square_index);
						}
					}
				}
			}
		}
	}

	return Move_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_party_targets
 * FUNCTION  : Make bitlist of party member targets.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.04.95 11:16
 * LAST      : 19.04.95 11:16
 * INPUTS    : None.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_party_targets(void)
{
	struct Combat_participant *Part;
	UNLONG Mask = 0;
	UNSHORT i, j;

	/* Search the matrix for participants */
	for (i=3;i<5;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here in the matrix ? */
			if (Combat_matrix[i][j].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Is party member ? */
				if (Part->Type == PARTY_PART_TYPE)
				{
					/* Yes -> Mark */
					Mask |= (1L << (i * NR_TACTICAL_COLUMNS + j));
				}
			}
		}
	}
	return Mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_monster_targets
 * FUNCTION  : Make bitlist of monster targets.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.04.95 11:23
 * LAST      : 19.04.95 11:23
 * INPUTS    : None.
 * RESULT    : UNLONG : Bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_monster_targets(void)
{
	struct Combat_participant *Part;
	UNLONG Mask = 0;
	UNSHORT i, j;

	/* Search the matrix for participants */
	for (i=0;i<4;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anything here in the matrix ? */
			if (Combat_matrix[i][j].Part)
			{
				/* Yes -> Get participant data */
				Part = Combat_matrix[i][j].Part;

				/* Is monster ? */
				if (Part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Mark */
					Mask |= (1L << (i * NR_TACTICAL_COLUMNS + j));
				}
			}
		}
	}
	return Mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_combat_message
 * FUNCTION  : Print a combat message.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.03.95 16:20
 * LAST      : 12.03.95 16:20
 * INPUTS    : struct Combat_participant *Acting_part - Pointer to acting
 *              participant's data.
 *             struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Damage - Amount of damage.
 *             UNSHORT Message_nr - Message number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This central function handles all text output during combat.
 *              It sets the global variables which are read by the {DAMG}-,
 *              {WEAP}-, {COMB}- and {VICT}-text commands. Any information
 *              not appropriate to / needed by the current message should be
 *              set to NULL or 0.
 *              By centralizing the access to these global variables it is
 *              guaranteed that they always contain a correct value.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_combat_message(struct Combat_participant *Acting_part, struct
 Combat_participant *Victim_part, UNSHORT Damage, UNSHORT Message_nr)
{
	/* Set global combat text parameters */
	Current_acting_part = Acting_part;
	Current_victim_part = Victim_part;
	Combat_text_damage = Damage;

	if (Acting_part)
		Combat_text_weapon_item_index = Acting_part->Weapon_item_index;
	else
		Combat_text_weapon_item_index = 0;

	/* Print message */
	Set_permanent_message_nr(Message_nr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_part_message
 * FUNCTION  : Print a combat message referring to a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.05.95 17:31
 * LAST      : 05.05.95 17:31
 * INPUTS    : struct Combat_participant *Part - Pointer to participant's
 *              data.
 *             UNSHORT Message_nr - Message number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_part_message(struct Combat_participant *Part, UNSHORT Message_nr)
{
	UNCHAR String[300];
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	/* Get character name */
	Get_char_name(Part->Char_handle, Name);

	/* Build message */
	_bprintf
	(
		String,
		300,
		System_text_ptrs[Message_nr],
		Name
	);

	/* Print message */
	Set_permanent_text(String);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_combat_damage
 * FUNCTION  :	Do damage to a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 17:46
 * LAST      : 27.10.95 18:13
 * INPUTS    : struct Combat_participant *Part - Pointer to participant's
 *              data.
 *             UNSHORT Damage - Amount of damage.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_combat_damage(struct Combat_participant *Part, UNSHORT Damage)
{
	static struct Event_action Part_damage_action;
	union Combat_show_parms P;
	SISHORT LP;

	/* Can be damaged ? */
	if (Get_conditions(Part->Char_handle) & DAMAGE_MASK)
	{
		/* No -> Exit */
		return;
	}

	/* Clear event action data */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &Part_damage_action,
		sizeof(struct Event_action),
		0
	);

	/* Any damage done ? */
	if (Damage)
	{
		/* Yes -> Wake up */
		Clear_condition(Part->Char_handle, ASLEEP);
	}

	/* Show damage in tactical window */
	Part->Damage					= Damage;
	Part->Damage_display_timer	= DAMAGE_DISPLAY_INTERVAL;

	/* Show damage */
	P.Damage_parms.Damage = Damage;
	Combat_show(Part, SHOW_DAMAGE, &P);

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Play sound effect */
		SOUND_Play_sound_effect(268, 100, 100, 50, 0);

		/* Print "<victim> receives <damage> damage! */
		Print_combat_message
		(
			NULL,
			Part,
			Damage,
			450
		);

		/* Do damage */
		Do_damage(Part->Number, Damage);

		/* "Dead" ? */
		if (Get_conditions(Part->Char_handle) & DEAD_MASK)
		{
			/* Yes -> Write event action data */
			Part_damage_action.Actor_type		= PARTY_ACTOR_TYPE;
			Part_damage_action.Actor_index	= Part->Number;
			Part_damage_action.Action_type	= PART_DIES_ACTION;

			Perform_action(&Part_damage_action);

			/* Abort attack */
			Abort_attack = TRUE;
		}
		else
		{
			/* No -> Write event action data */
			Part_damage_action.Actor_type		= PARTY_ACTOR_TYPE;
			Part_damage_action.Actor_index	= Part->Number;
			Part_damage_action.Action_type	= PART_IS_HIT_ACTION;

			Perform_action(&Part_damage_action);
		}
	}
	else
	{
		/* Monster -> Play sound effect */
		SOUND_Play_sound_effect(268, 100, 60, 50, 15000);

		/* Print "<victim> receives <damage> damage! */
		Print_combat_message
		(
			NULL,
			Part,
			Damage,
			450
		);

		/* Decrease life points */
		LP = Get_LP(Part->Char_handle);
		LP = max(LP - Damage, 0);
		Set_LP(Part->Char_handle, LP);

		/* "Dead" ? */
		if (!LP)
		{
			/* Yes -> Write event action data */
			Part_damage_action.Actor_type		= MONSTER_ACTOR_TYPE;
			Part_damage_action.Actor_index	= Part->Number;
			Part_damage_action.Action_type	= PART_DIES_ACTION;

			Perform_action(&Part_damage_action);

			/* Show monster death */
			Combat_show(Part, SHOW_DEATH, NULL);

			/* Kill monster */
			Kill_participant(Part);

			/* Abort attack */
			Abort_attack = TRUE;
		}
		else
		{
			/* No -> Write event action data */
			Part_damage_action.Actor_type		= MONSTER_ACTOR_TYPE;
			Part_damage_action.Actor_index	= Part->Number;
			Part_damage_action.Action_type	= PART_IS_HIT_ACTION;

			Perform_action(&Part_damage_action);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Kill_participant
 * FUNCTION  : Kill a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 20:22
 * LAST      : 07.06.95 14:07
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Death is NOT shown!
 *             - A monster's items, gold and food are put in the Apres pool.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Kill_participant(struct Combat_participant *Part)
{
	struct Character_data *Char;
	UNSHORT i;

	/* Is monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		/* Put items in Apres pool */
		for (i=0;i<ITEMS_ON_BODY;i++)
		{
			if (!Packet_empty(&(Char->Body_items[i])))
			{
				Store_item_in_Apres_pool(&(Char->Body_items[i]));
			}
		}
		for (i=0;i<ITEMS_PER_CHAR;i++)
		{
			if (!Packet_empty(&(Char->Backpack_items[i])))
			{
				Store_item_in_Apres_pool(&(Char->Backpack_items[i]));
			}
		}

		/* Put gold and food in Apres pool */
		Put_gold_in_Apres_pool(Char->Char_gold);
		Put_food_in_Apres_pool(Char->Char_food);

		MEM_Free_pointer(Part->Char_handle);
	}

	/* Destroy participant */
	Destroy_participant(Part);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Destroy_participant
 * FUNCTION  : Destroy a combat participant.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 20:22
 * LAST      : 07.06.95 14:08
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Death is NOT shown!
 *             - A monster's items, gold and food are NOT put in the Apres
 *              pool.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Destroy_participant(struct Combat_participant *Part)
{
	struct Character_data *Char;

	/* Kill! */
	Set_condition(Part->Char_handle, UNCONSCIOUS);

	/* Clear number of life-points */
	Set_LP(Part->Char_handle, 0);

	/* Is monster ? */
	if (Part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Add EP */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		Added_experience_points += (UNLONG) Char->Experience_bonus;

		MEM_Free_pointer(Part->Char_handle);
	}

	/* Remove incapable participants */
	Update_control_status();

	/* Show */
//	Update_screen();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Install_trap
 * FUNCTION  : Install a trap.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 10:12
 * LAST      : 07.07.95 10:12
 * INPUTS    : UNSHORT Tactical_X - Tactical X-coordinate.
 *             UNSHORT Tactical_Y - Tactical Y-coordinate.
 *             Trap_handler Handler - Pointer to trap handler function.
 *             UNSHORT Strength - Spell strength.
 *             struct COMOB *Trap_COMOB - Pointer to trap COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Install_trap(UNSHORT Tactical_X, UNSHORT Tactical_Y, Trap_handler Handler,
UNSHORT Strength, struct COMOB *Trap_COMOB)
{
	/* Are the source coordinates inside the combat matrix ? */
	if ((Tactical_X < NR_TACTICAL_COLUMNS) &&
	 (Tactical_Y < NR_TACTICAL_ROWS))
	{
		/* Yes -> Set trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_handler	= Handler;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_strength	= Strength;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_COMOB		= Trap_COMOB;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_trap
 * FUNCTION  : Remove a trap.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 10:13
 * LAST      : 07.07.95 10:13
 * INPUTS    : UNSHORT Tactical_X - Tactical X-coordinate.
 *             UNSHORT Tactical_Y - Tactical Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_trap(UNSHORT Tactical_X, UNSHORT Tactical_Y)
{
	struct COMOB *COMOB;

	/* Are the source coordinates inside the combat matrix ? */
	if ((Tactical_X < NR_TACTICAL_COLUMNS) &&
	 (Tactical_Y < NR_TACTICAL_ROWS))
	{
		/* Yes -> Is there a trap COMOB ? */
		COMOB = Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_COMOB;
		if (COMOB)
		{
			/* Yes -> Remove it */
			Delete_COMOB(COMOB);
		}

		/* Remove trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_handler	= NULL;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_strength	= 0;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_COMOB		= NULL;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Is_trap
 * FUNCTION  : Is there a trap here?
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.07.95 10:16
 * LAST      : 07.07.95 10:16
 * INPUTS    : UNSHORT Tactical_X - Tactical X-coordinate.
 *             UNSHORT Tactical_Y - Tactical Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Is_trap(UNSHORT Tactical_X, UNSHORT Tactical_Y)
{
	BOOLEAN Is_trap_flag = FALSE;

	/* Are the source coordinates inside the combat matrix ? */
	if ((Tactical_X < NR_TACTICAL_COLUMNS) &&
	 (Tactical_Y < NR_TACTICAL_ROWS))
	{
		/* Yes -> Is there a trap here ? */
		if (Combat_matrix[Tactical_Y][Tactical_X].Trap_data.Trap_handler)
		{
			/* Yes */
			Is_trap_flag = TRUE;
		}
	}

	return Is_trap_flag;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_remaining_monsters
 * FUNCTION  : Count the monsters remaining in combat.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 21:00
 * LAST      : 23.10.95 21:00
 * INPUTS    : None.
 * RESULT    : UNSHORT : Remaining monsters.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_remaining_monsters(void)
{
	UNSHORT Remaining_monsters;
	UNSHORT X, Y;

	/* Check combat matrix */
	Remaining_monsters = 0;
	for (Y=0;Y<NR_TACTICAL_ROWS;Y++)
	{
		for (X=0;X<NR_TACTICAL_COLUMNS;X++)
		{
			/* Anyone there ? */
			if (Combat_matrix[Y][X].Part)
			{
				/* Yes -> Monster ? */
				if (Combat_matrix[Y][X].Part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Count up */
					Remaining_monsters++;
				}
			}
		}
	}

	return Remaining_monsters;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_remaining_party_members
 * FUNCTION  : Count the party members remaining in combat.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 21:01
 * LAST      : 23.10.95 21:20
 * INPUTS    : None.
 * RESULT    : UNSHORT : Remaining party members.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_remaining_party_members(void)
{
	UNSHORT Remaining_members;
	UNSHORT X, Y;

	/* Check combat matrix */
	Remaining_members = 0;
	for (Y=0;Y<NR_TACTICAL_ROWS;Y++)
	{
		for (X=0;X<NR_TACTICAL_COLUMNS;X++)
		{
			/* Anyone there ? */
			if (Combat_matrix[Y][X].Part)
			{
				/* Yes -> Party member ? */
				if (Combat_matrix[Y][X].Part->Type == PARTY_PART_TYPE)
				{
					/* Yes -> Capable of fighting ? */
					if (!(Get_conditions(Combat_matrix[Y][X].Part->Char_handle) &
					 ((1 << UNCONSCIOUS) | (1 << INSANE))))
					{
						/* Yes -> Count up */
						Remaining_members++;
					}
				}
			}
		}
	}

	return Remaining_members;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_fled_party_members
 * FUNCTION  : Count the party members that have fled.
 * FILE      : COMBAT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 21:01
 * LAST      : 23.10.95 21:01
 * INPUTS    : None.
 * RESULT    : UNSHORT : Fled party members.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_fled_party_members(void)
{
	UNSHORT Fled_members;
	UNSHORT i;

	/* Check all party members */
	Fled_members = 0;
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Party_parts[i].Type == PARTY_PART_TYPE)
		{
			/* Yes -> Fled ? */
			if (Get_conditions(Party_parts[i].Char_handle) & (1 << FLEEING))
			{
				/* Yes -> Count up */
				Fled_members++;
			}
		}
	}

	return Fled_members;
}

