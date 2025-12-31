/************
 * NAME     : GOLDFOOD.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 6-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GOLDFOOD_H
#define GOLDFOOD_H

#include <BBDEF.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

#define CHAR_GOLD_TYPE		(0)
#define CHAR_FOOD_TYPE		(1)
#define CHEST_GOLD_TYPE		(2)
#define CHEST_FOOD_TYPE		(3)

/*
 ** Structure definitions **************************************************
 */

/* Gold and food OID */
struct Gold_food_OID {
	UNSHORT Type;					/* See above */
	MEM_HANDLE Data_handle;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Gold_food_Class;

/*
 ** Prototypes *************************************************************
 */

#endif

