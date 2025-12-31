/************
 * NAME     : GAMETIME.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 14-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>

#include <PRTLOGIC.H>
#include <GAMETIME.H>
#include <MAP.H>
#include <NPCS.H>
#include <GAME.H>
#include <MAGIC.H>
#include <GAMEVAR.H>
#include <EVELOGIC.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <SAVELOAD.H>

/* prototypes */

void Next_hour(void);
void Next_day(void);
void Next_month(void);
void Next_year(void);

/* global variables */

static struct Time_frame Albion_time = {
	12,
	30,
	24,
	60,
	48
};

BOOLEAN Waiting_flag;

UNSHORT Current_step;
UNLONG Current_substep;
UNSHORT Steps_per_day;
struct Time_frame *Current_time_frame = &Albion_time;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_time
 * FUNCTION  : Initialize time.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 13:23
 * LAST      : 04.10.95 13:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_time(void)
{
	/* Calculate steps per day */
	Steps_per_day = Current_time_frame->Steps_per_hour *
	 Current_time_frame->Hours_per_day;

	/* Loading a game / right version ? */
	if (Loading_game && (Current_save_game_version_nr > 75))
	{
		/* Yes -> Get current step */
		Current_step = PARTY_DATA.Current_step;

		/* Get current substep */
		Current_substep = PARTY_DATA.Current_substep;
	}
	else
	{
		/* No -> Reset time */
		Reset_time();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_time
 * FUNCTION  : Reset time.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 13:18
 * LAST      : 04.10.95 13:18
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_time(void)
{
	/* Calculate current step */
	Current_step = (PARTY_DATA.Hour * Current_time_frame->Steps_per_hour) +
	 (PARTY_DATA.Minute * Current_time_frame->Steps_per_hour) / 60;

	/* Clear current substep */
	Current_substep = 0;

	/* Remember the time */
	Last_NPC_step = Current_step;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_time
 * FUNCTION  : Update time.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 13:23
 * LAST      : 22.07.95 18:04
 * INPUTS    : UNSHORT Nr_substeps - Number of sub-steps that have passed,
 *              times 100.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The number of sub-steps may have any value. Time will be
 *              updated accordingly.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_time(UNSHORT Nr_substeps)
{
	/* Update */
	Current_substep += Nr_substeps;

	/* Next step ? */
	while (Current_substep >= SUBSTEPS_PER_STEP)
	{
		/* Yes */
		Current_substep -= SUBSTEPS_PER_STEP;

		/* Next step */
		Next_step();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Next_step.
 * FUNCTION  : Next step logic.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 14:03
 * LAST      : 05.09.95 21:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Next_step(void)
{
	UNSHORT Old_map_nr;

	/* Get current map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Update temporary spells */
	Update_temporary_spells();

	/* Next step */
	Current_step++;

	/* Calculate current minute */
	PARTY_DATA.Minute = ((Current_step % Current_time_frame->Steps_per_hour) *
	 60) / Current_time_frame->Steps_per_hour;

	/* Next hour ? */
	if (!(Current_step % Current_time_frame->Steps_per_hour))
	{
		/* Yes -> Next hour */
		Next_hour();
	}

	/* Still in the same map ? */
	if (Old_map_nr == PARTY_DATA.Map_nr)
	{
		/* Yes -> Execute every-step event chains */
		Execute_chains_in_map(EVERY_STEP_TRIGGER);

		/* Still in the same map ? */
		if (Old_map_nr == PARTY_DATA.Map_nr)
		{
			/* Yes -> Execute normal map events again */
			Execute_normal_map_events();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wait_x_hours
 * FUNCTION  : Wait a number of hours.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.10.95 21:28
 * LAST      : 09.10.95 21:23
 * INPUTS    : UNSHORT Nr_hours - Number of hours.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wait_x_hours(UNSHORT Nr_hours)
{
	UNSHORT Old_map_nr;
	UNSHORT i, j;

	/* Set waiting flag */
	Waiting_flag = TRUE;

	/* Install wait mouse pointer */
	Push_mouse(&(Mouse_pointers[WAIT_MPTR]));

	/* Get current map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	for (i=0;i<Nr_hours;i++)
	{
		for (j=0;j<Current_time_frame->Steps_per_hour;j++)
		{
			/* Update temporary spells */
			Update_temporary_spells();

			/* Next step */
			Current_step++;

			/* Calculate current minute */
			PARTY_DATA.Minute = ((Current_step % Current_time_frame->Steps_per_hour) *
			 60) / Current_time_frame->Steps_per_hour;

			/* Next hour ? */
			if (!(Current_step % Current_time_frame->Steps_per_hour))
			{
				/* Yes -> Next hour */
				Next_hour();

				/* Still in the same map ? */
				if (Old_map_nr != PARTY_DATA.Map_nr)
					break;
			}
		}
		/* Still in the same map ? */
		if (Old_map_nr != PARTY_DATA.Map_nr)
			break;
	}

	/* Remove mouse pointer */
	Pop_mouse();

	/* Clear waiting flag */
	Waiting_flag = FALSE;

	/* Remember the time */
	Last_NPC_step = Current_step;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Next_hour
 * FUNCTION  : Next hour logic.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 14:04
 * LAST      : 02.09.95 19:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Next_hour(void)
{
	UNSHORT Old_map_nr;

	/* Get current map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Update party light */
	Update_party_light();

	/* Update */
	PARTY_DATA.Hour++;
	PARTY_DATA.Camp_counter++;

	/* Execute poison logic */
	Do_poison_logic();

	/* Execute camp logic */
	Do_camp_logic();

	/* Next day ? */
	if (PARTY_DATA.Hour == Current_time_frame->Hours_per_day)
	{
		/* Yes */
		PARTY_DATA.Hour = 0;
		Current_step = 0;

		/* Next day */
		Next_day();
	}

	/* Still in the same map ? */
	if (Old_map_nr == PARTY_DATA.Map_nr)
	{
		/* Yes -> Execute every-hour event chains */
		Execute_chains_in_map(EVERY_HOUR_TRIGGER);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Next_day
 * FUNCTION  : Next day logic.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 14:05
 * LAST      : 14.12.94 14:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Next_day(void)
{
	/* Update */
	PARTY_DATA.Day++;

	/* Next month ? */
	if (PARTY_DATA.Day == Current_time_frame->Days_per_month)
	{
		/* Yes */
		PARTY_DATA.Day = 0;

		/* Next month */
		Next_month();
	}

	/* Execute every-day event chains */
	Execute_chains_in_map(EVERY_DAY_TRIGGER);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Next_month
 * FUNCTION  : Next month logic.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 14:06
 * LAST      : 14.12.94 14:06
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Next_month(void)
{
	/* Update */
	PARTY_DATA.Month++;

	/* Next year ? */
	if (PARTY_DATA.Month == Current_time_frame->Months_per_year)
	{
		/* Yes */
		PARTY_DATA.Month = 0;

		/* Next year */
		Next_year();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Next_year
 * FUNCTION  : Next year logic.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 14:07
 * LAST      : 14.12.94 14:07
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Next_year(void)
{
	/* Update */
	PARTY_DATA.Year++;
	PARTY_DATA.Relative_year++;
}

