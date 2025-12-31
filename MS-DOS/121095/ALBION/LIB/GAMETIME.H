/************
 * NAME     : GAMETIME.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 14-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GAMETIME_H
#define GAMETIME_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define SUBSTEPS_PER_STEP	(16 * 100)

/*
 ** Structure definitions **************************************************
 */

struct Time_frame {
	UNSHORT Months_per_year;
	UNSHORT Days_per_month;
	UNSHORT Hours_per_day;
	UNSHORT Minutes_per_hour;
	UNSHORT Steps_per_hour;
};

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Waiting_flag;

extern UNSHORT Current_step;
extern UNLONG Current_substep;

extern UNSHORT Steps_per_day;
extern struct Time_frame *Current_time_frame;

/*
 ** Prototypes *************************************************************
 */

void Init_time(void);

void Reset_time(void);

void Update_time(UNSHORT Nr_substeps);

void Next_step(void);

void Wait_x_hours(UNSHORT Nr_hours);

#endif

