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

extern UNSHORT Current_step;
extern UNLONG Current_substep;
extern UNSHORT Steps_per_day;
extern struct Time_frame *Current_time_frame;

/*
 ** Prototypes *************************************************************
 */

void Init_time(void);
void Init_map_time(UNSHORT Map_flags);

void Update_time(UNSHORT Nr_substeps);

void Next_step(void);
void Next_hour(void);
void Next_day(void);
void Next_month(void);
void Next_year(void);

#endif

