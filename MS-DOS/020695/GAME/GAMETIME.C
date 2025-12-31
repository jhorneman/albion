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

#include <GAMETIME.H>
#include <MAP.H>
#include <NPCS.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <EVELOGIC.H>

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
	40,
	18,
	60,
	48
};

UNSHORT Current_step;
UNSHORT Current_substep;
UNSHORT Steps_per_day;
struct Time_frame *Time = NULL;

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
	Time = NULL;
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
	struct Time_frame *Target;
	UNLONG Minutes;

	/* What time frame should we use ? */
	if (Map_flags & PLANET_SPACESHIP)
		Target = &Ship_time;
	else
		Target = &Albion_time;

	/* Are we now in the correct time-frame ? */
	if (Time != Target)
	{
		/* No -> Must we convert ? */
		if (Time)
		{
			/* Yes -> Calculate current minute of this year */
			Minutes = (((((PARTY_DATA.Month * Time->Days_per_month)
			 + PARTY_DATA.Day) * Time->Hours_per_day) + PARTY_DATA.Hour)
			 * Time->Minutes_per_hour) + PARTY_DATA.Minute;

			/* Convert current time to new time frame */
			PARTY_DATA.Minute = Minutes % Target->Minutes_per_hour;
			Minutes /= Target->Minutes_per_hour;

			PARTY_DATA.Hour = Minutes % Target->Hours_per_day;
			Minutes /= Target->Hours_per_day;

			PARTY_DATA.Day = Minutes % Target->Days_per_month;
			Minutes /= Target->Days_per_month;

			PARTY_DATA.Month = Minutes % Target->Months_per_year;
		}

		/* Set new time frame */
		Time = Target;

		/* Calculate steps per day */
		Steps_per_day = Time->Steps_per_hour * Time->Hours_per_day;

		/* Calculate current step */
		Current_step = (PARTY_DATA.Hour * Time->Steps_per_hour)
		 + (PARTY_DATA.Minute * Time->Steps_per_hour) / 60;
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
 * LAST      : 14.12.94 13:23
 * INPUTS    : UNSHORT Nr_substeps - Number of sub-steps that have passed.
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
 * LAST      : 14.12.94 14:03
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
	PARTY_DATA.Minute = ((Current_step % Time->Steps_per_hour) * 60)
	 / Time->Steps_per_hour;

	/* Next hour ? */
	if (!(Current_step % Time->Steps_per_hour))
	{
		/* Yes -> Next hour */
		Next_hour();
	}

	/* Execute every-step event chains */
	Execute_chains_in_map(EVERY_STEP_TRIGGER);
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
	if (PARTY_DATA.Hour == Time->Hours_per_day)
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
	if (PARTY_DATA.Day == Time->Days_per_month)
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
	if (PARTY_DATA.Month == Time->Months_per_year)
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

