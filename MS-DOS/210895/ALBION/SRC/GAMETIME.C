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
#include <GAMEVAR.H>
#include <EVELOGIC.H>

/* prototypes */

void Next_step(void);
void Next_hour(void);
void Next_day(void);
void Next_month(void);
void Next_year(void);

/* global variables */

static struct Time_frame Ship_time = {
	12,
	30,
	24,
	60,
	48
};

static struct Time_frame Albion_time = {
	12,
	30,
	24,
	60,
	48
};

UNSHORT Current_step;
UNLONG Current_substep;
UNSHORT Steps_per_day;
struct Time_frame *Current_time_frame = NULL;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_time
 * FUNCTION  : Initialize time.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 13:23
 * LAST      : 14.12.94 13:23
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
	/* No time frame */
	Current_time_frame = NULL;
	Steps_per_day = 0;

	/* Reset time (just in case) */
	Current_step = 0;
	Current_substep = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map_time
 * FUNCTION  : Initialize time per map.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 13:35
 * LAST      : 14.12.94 13:35
 * INPUTS    : UNSHORT Map_flags - Map flags.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_map_time(UNSHORT Map_flags)
{
	struct Time_frame *New_time_frame;
	UNLONG Minutes;

	/* What time frame should we use ? */
	if (Map_flags & PLANET_SPACESHIP)
		New_time_frame = &Ship_time;
	else
		New_time_frame = &Albion_time;

	/* Are we now in the correct time-frame ? */
	if (Current_time_frame != New_time_frame)
	{
		/* No -> Must we convert ? */
		if (Current_time_frame)
		{
			/* Yes -> Calculate current minute of this year */
			Minutes = (((((PARTY_DATA.Month * Current_time_frame->Days_per_month)
			 + PARTY_DATA.Day) * Current_time_frame->Hours_per_day) +
			 PARTY_DATA.Hour) * Current_time_frame->Minutes_per_hour) +
			 PARTY_DATA.Minute;

			/* Convert current time to new time frame */
			PARTY_DATA.Minute = Minutes % New_time_frame->Minutes_per_hour;
			Minutes /= New_time_frame->Minutes_per_hour;

			PARTY_DATA.Hour = Minutes % New_time_frame->Hours_per_day;
			Minutes /= New_time_frame->Hours_per_day;

			PARTY_DATA.Day = Minutes % New_time_frame->Days_per_month;
			Minutes /= New_time_frame->Days_per_month;

			PARTY_DATA.Month = Minutes % New_time_frame->Months_per_year;
		}

		/* Set new time frame */
		Current_time_frame = New_time_frame;

		/* Calculate steps per day */
		Steps_per_day = Current_time_frame->Steps_per_hour *
		 Current_time_frame->Hours_per_day;

		/* Calculate current step */
		Current_step = (PARTY_DATA.Hour * Current_time_frame->Steps_per_hour)
		 + (PARTY_DATA.Minute * Current_time_frame->Steps_per_hour) / 60;
		Current_substep = 0;
	}
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
 * LAST      : 25.07.95 18:58
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

	/* Execute every-step event chains */
	Execute_chains_in_map(EVERY_STEP_TRIGGER);

	/* Execute normal map events again */
	Execute_normal_map_events();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Next_hour
 * FUNCTION  : Next hour logic.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 14:04
 * LAST      : 14.12.94 14:04
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
	/* Update */
	PARTY_DATA.Hour++;

	/* Next day ? */
	if (PARTY_DATA.Hour == Current_time_frame->Hours_per_day)
	{
		/* Yes */
		PARTY_DATA.Hour = 0;
		Current_step = 0;

		/* Next day */
		Next_day();
	}

	/* Execute every-hour event chains */
	Execute_chains_in_map(EVERY_HOUR_TRIGGER);
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

