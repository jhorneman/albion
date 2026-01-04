static struct Time_frame Ship_time = {
	12,
	30,
	24,
	60,
	48
};

struct Time_frame *Current_time_frame = NULL;

// In Init_time :

	Current_time_frame	= NULL;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map_time
 * FUNCTION  : Initialize time per map.
 * FILE      : GAMETIME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 13:35
 * LAST      : 25.08.95 16:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_map_time(void)
{
	struct Time_frame *New_time_frame;
	UNLONG Minutes;

	/* What time frame should we use ? */
	if (Spaceship_map)
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
		Current_step = (PARTY_DATA.Hour * Current_time_frame->Steps_per_hour) +
		 (PARTY_DATA.Minute * Current_time_frame->Steps_per_hour) / 60;
		Current_substep = 0;
	}
}

