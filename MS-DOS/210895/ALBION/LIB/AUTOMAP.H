/************
 * NAME     : AUTOMAP.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef AUTOMAP_H
#define AUTOMAP_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Enter automap function */
void Enter_Automap(void);
BOOLEAN Do_enter_automap(struct Event_action *Action);

void Check_for_new_goto_points(void);

/* Automap management functions */
void Init_automap(void);
void Exit_automap(void);
void Save_automap(void);
void Update_automap(void);

#endif

