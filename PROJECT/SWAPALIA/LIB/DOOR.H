/************
 * NAME     : DOOR.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef DOOR_H
#define DOOR_H

#include <BBDEF.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

/* Door status */
#define EXIT_DOOR_STATUS		(0)	/* Player left the door screen */
#define TRAP_DOOR_STATUS		(1)	/* Player activated a trap */
#define OPENED_DOOR_STATUS		(2)	/* Player opened the door */
#define IS_OPEN_DOOR_STATUS	(3)	/* Door was already open */

/* Lock status */
#define LOCK_STATUS_OK					(0)		/* Initial status */
#define LOCK_STATUS_PICKED				(1)		/* Player picked the lock */
#define LOCK_STATUS_UNLOCKED			(2)		/* Player unlocked the lock */
#define LOCK_STATUS_TRAP_ACTIVATED	(3)		/* Player activated a trap */

/*
 ** Structure definitions **************************************************
 */

/* Lock OID */
struct Lock_OID {
	UNSHORT Locked_percentage;
	UNSHORT Key_index;
	BOOLEAN Lock_has_trap;
	UNSHORT *Lock_result_ptr;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Lock_Class;

/*
 ** Prototypes *************************************************************
 */

UNSHORT Do_Door(UNSHORT Locked_percentage, UNSHORT Key_index,
 UNSHORT Lock_type, UNSHORT First_text_block_nr, UNSHORT Opened_text_block_nr,
 UNSHORT Door_index, BOOLEAN Has_trap);

#endif

