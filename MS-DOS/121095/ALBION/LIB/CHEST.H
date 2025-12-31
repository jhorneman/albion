/************
 * NAME     : CHEST.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef CHEST_H
#define CHEST_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* Chest status */
#define EXIT_CLOSED_CHEST_STATUS	(0)	/* Player left the closed chest screen */
#define TRAP_CHEST_STATUS			(1)	/* Player activated a trap */
#define OPENED_CHEST_STATUS		(2)	/* Player opened the chest */
#define EXIT_OPEN_CHEST_STATUS	(3)	/* Player left the open chest screen */
#define JUNKPILE_EMPTY_STATUS		(4)	/* Player emptied a junkpile */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

UNSHORT Do_Chest(UNSHORT Locked_percentage, UNSHORT Key_index,
 UNSHORT Flags, UNSHORT First_text_block_nr, UNSHORT Opened_text_block_nr,
 UNSHORT Chest_index, BOOLEAN Has_trap);

#endif

