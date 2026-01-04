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

#define MAX_UPDATE_DEPTH		(10)

/* Automapper function flags */
#define SHOW_NPCS_FUNCTION			(1 << 0)
#define SHOW_MONSTERS_FUNCTION 	(1 << 1)
#define SHOW_TRAPS_FUNCTION 		(1 << 2)
#define SHOW_HIDDEN_FUNCTION 		(1 << 3)		// NOT USED

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
void Enter_Automap(UNSHORT Function_flags);

void Check_for_new_goto_points(void);

/* Automap management functions */
void Init_automap(void);
void Exit_automap(void);
void Save_automap(void);

void Update_automap(UNSHORT Update_depth);

#endif

