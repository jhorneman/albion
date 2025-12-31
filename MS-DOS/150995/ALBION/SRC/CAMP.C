/************
 * NAME     : CAMP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 17-08-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MERCHANT.H
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

#define CAMP_PICTURE		(1)
#define INN_PICTURE		(1)

#define OUTSIDE_RECUPERATION_PERCENTAGE	(50)

/* prototypes */

/* Camp module functions */
void Camp_ModInit(void);

/* Camp support functions */
void Recuperate(UNSHORT Member_index, UNSHORT Recuperation_percentage);

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
 * LAST      : 01.09.95 13:58
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
	/* Store flag */
	CAMP_Inn_flag = Inn_flag;

	/* Enter Camp screen */
	Exit_display();
	Push_module(&Camp_Mod);
	Init_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Camp_ModInit
 * FUNCTION  : Initialize Camp module.
 * FILE      : CAMP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.09.95 13:58
 * LAST      : 01.09.95 13:58
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
	UNSHORT i, j;

	/* Load small picture */
//	Small_picture_handle = Load_subfile(PICTURE, MERCHANT_PICTURE);

	/* Get dimensions of small picture */
//	Get_LBM_dimensions(Small_picture_handle, &Small_picture_width,
//	 &Small_picture_height);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);

	/* Draw camp picture */
//	Display_LBM(&Main_OPM, (Screen_width - Small_picture_width) / 2,
//	 (Panel_Y - Small_picture_height) / 2, Small_picture_handle);

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

	/* Clear counter */
	PARTY_DATA.Camp_counter = 0;

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

	/* Clear counter again */
	PARTY_DATA.Camp_counter = 0;

	/* Increase hour */
	for (i=0;i<Nr_hours;i++)
	{
		for (j=0;j<Current_time_frame->Steps_per_hour;j++)
		{
			Next_step();
		}
	}

	/* Get party food */
	Party_food = Get_party_food();

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Alive and kicking ? */
			if (Character_alive(Party_char_handles[i + 1]))
			{
				/* Yes -> In an Inn ? */
				if (CAMP_Inn_flag)
				{
					/* Yes -> Recuperate */
					Recuperate(i + 1, Place_value);
				}
				else
				{
					/* No -> Is there any food ? */
					if (Party_food)
					{
						/* Yes -> Subtract one ration */
						Party_food -= 1;

						/* Recuperate */
						Recuperate(i + 1, OUTSIDE_RECUPERATION_PERCENTAGE);
					}
					else
					{
						/* No -> "No food" */
						Subject_char_handle = Party_char_handles[i];
						Do_text_window(System_text_ptrs[606]);
					}
				}
			}
		}
	}

	/* Set new party food */
	Set_party_food(Party_food);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);

//	MEM_Free_memory(Small_picture_handle);

	/* Reset palette blending */
	Reset_map_palette_blending();

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
 * LAST      : 01.09.95 16:46
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
	UNSHORT LP;
	UNSHORT SP;
	UNSHORT Gained_LP;
	UNSHORT Gained_SP;

	Char_handle = Party_char_handles[Member_index - 1];

	/* Get LP */
	LP = Get_LP(Char_handle);

	/* Increase LP */
	Gained_LP = max(1, ((LP * Recuperation_percentage) / 100));

	Set_LP(Char_handle, LP + Gained_LP);

	// EFFECT

	/* Does this person know any spells ? */
	if (Character_has_magical_abilities(Char_handle))
	{
		/* Yes -> Get SP */
		SP = Get_SP(Char_handle);

		/* Increase SP */
		Gained_SP = max(1, ((SP * Recuperation_percentage) / 100));

		Set_SP(Char_handle, SP + Gained_SP);

		//EFFECT
	}
}

