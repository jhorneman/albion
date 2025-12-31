/************
 * NAME     : CAMP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 17-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : CAMP.H
 ************/

/* includes */

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
#include <CAMP.H>
#include <PRTLOGIC.H>
#include <EVELOGIC.H>
#include <GAMETEXT.H>
#include <BOOLREQ.H>
#include <COLOURS.H>
#include <XFTYPES.H>
#include <PLACES.H>
#include <STATAREA.H>
#include <TEXTWIN.H>
#include <GAMETIME.H>

/* defines */

#define CAMP_PICTURE		(31)

#define RATIONS_PER_CAMP						(2)
#define OUTSIDE_RECUPERATION_PERCENTAGE	(50)

/* prototypes */

BOOLEAN Do_enter_camp(struct Event_action *Action);

/* Camp module functions */
void Camp_ModInit(void);

/* global variables */

static BOOLEAN CAMP_Inn_flag;

/* Module */
static struct Module Camp_Mod = {
	LOCAL_MOD, SCREEN_MOD, CAMP_SCREEN,
	NULL,
	Camp_ModInit,
	NULL,
	NULL,
	NULL,
	NULL
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_Camp
 * FUNCTION  : Enter Camp screen.
 * FILE      : CAMP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 13:58
 * LAST      : 02.10.95 21:24
 * INPUTS    : BOOLEAN Inn_flag - Inn.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_Camp(BOOLEAN Inn_flag)
{
	static struct Event_action Enter_camp_action =
	{
		0,	NO_ACTOR_TYPE, 0,
		MAKE_CAMP_ACTION, 0, 0,
		Do_enter_camp, NULL, NULL
	};

	/* Store flag */
	CAMP_Inn_flag = Inn_flag;

	/* Check events */
	Perform_action(&Enter_camp_action);
}

BOOLEAN
Do_enter_camp(struct Event_action *Action)
{
	/* Enter Camp screen */
	Exit_display();
	Push_module(&Camp_Mod);
	Init_display();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Camp_ModInit
 * FUNCTION  : Initialize Camp module.
 * FILE      : CAMP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 13:58
 * LAST      : 07.10.95 20:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Camp_ModInit(void)
{
	UNLONG Party_food;
	UNSHORT Nr_hours;
	UNSHORT i;

	/* Load small picture */
	Load_small_picture(CAMP_PICTURE);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);

	/* Draw picture */
	Display_small_picture_centered
	(
		&Main_OPM,
		0,
		0,
		360,
		Panel_Y
	);

	Switch_screens();

	/* De-exhaust the party */
	for (i=0;i<6;i++)
	{
		/* Anyone here ? */
		if (Member_present(i + 1))
		{
			/* Yes -> De-exhaust */
			De_exhaust_party_member(i + 1);
		}
	}

	/* Not in a dungeon and camping at night ? */
	if ((Current_map_type != DUNGEON_MAP) &&
	 ((PARTY_DATA.Hour < MORNING_TIME) || (PARTY_DATA.Hour >= EVENING_TIME)))
	{
		/* Yes -> "Rest until dawn" */
		Do_text_window(System_text_ptrs[604]);

		/* Calculate number of hours */
		if (PARTY_DATA.Hour < MORNING_TIME)
		{
			Nr_hours = DAWN_TIME - PARTY_DATA.Hour;
		}
		else
		{
			Nr_hours = (24 - PARTY_DATA.Hour) + DAWN_TIME;
		}
	}
	else
	{
		/* No -> "Rest x hours" */
		Do_text_window(System_text_ptrs[605]);

		/* Set number of hours */
		Nr_hours = CAMP_DURATION;
	}

	/* Execute diseased logic */
	Do_diseased_logic();

	/* Get party food */
	Party_food = Get_party_food();

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> In an Inn ? */
			if (CAMP_Inn_flag)
			{
				/* Yes -> Recuperate */
				Recuperate(i + 1, Place_value);
			}
			else
			{
				/* No -> Is there enough food ? */
				if (Party_food >= RATIONS_PER_CAMP)
				{
					/* Yes -> Subtract rations */
					Party_food -= RATIONS_PER_CAMP;

					/* Recuperate */
					Recuperate(i + 1, OUTSIDE_RECUPERATION_PERCENTAGE);
				}
				else
				{
					/* No -> "Not enough food" */
					Subject_char_handle = Party_char_handles[i];
					Do_text_window(System_text_ptrs[606]);
				}
			}
		}
	}

	/* Set new party food */
	Set_party_food(Party_food);

	/* Clear counter */
	PARTY_DATA.Camp_counter = 0;

	/* Wait */
	Wait_x_hours(Nr_hours);

	/* Clear counter again */
	PARTY_DATA.Camp_counter = 0;

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);

	/* Reset palette blending */
	Reset_map_palette_blending();

	Destroy_small_picture();

	Pop_module();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Recuperate.
 * FUNCTION  : Let a party member recuperate while camping.
 * FILE      : CAMP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 16:46
 * LAST      : 05.10.95 20:34
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 *             UNSHORT Recuperation_percentage - Recuperation percentage.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes the party member exists.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Recuperate(UNSHORT Member_index, UNSHORT Recuperation_percentage)
{
	MEM_HANDLE Char_handle;
	SISHORT SP;
	SISHORT Max_LP;
	SISHORT Max_SP;
	SISHORT Gained_LP;
	SISHORT Gained_SP;

	Char_handle = Party_char_handles[Member_index - 1];

	/* Get maximum LP */
	Max_LP = Get_max_LP(Char_handle);

	/* Increase LP */
	Gained_LP = max(1, ((Max_LP * Recuperation_percentage) / 100));

	/* Give stamina bonus */
	Gained_LP += Get_attribute(Char_handle, STAMINA) / 15;

	Do_healing(Member_index, Gained_LP);

	// EFFECT

	/* Does this person know any spells ? */
	if (Character_has_magical_abilities(Char_handle))
	{
		/* Yes -> Get SP and maximum SP */
		SP			= Get_SP(Char_handle);
		Max_SP	= Get_max_SP(Char_handle);

		/* Increase SP */
		Gained_SP = max(1, ((Max_SP * Recuperation_percentage) / 100));

		/* Give magical talent bonus */
		Gained_SP += Get_attribute(Char_handle, MAGIC_TALENT) / 15;

		Set_SP(Char_handle, SP + Gained_SP);

		//EFFECT
	}
}

