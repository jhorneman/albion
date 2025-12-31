/************
 * NAME     : INVLEFT.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 9-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INVLEFT_H
#define INVLEFT_H

#include <BBDEF.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

/* Left inventory types */
#define INVLEFT_STATUS_1	(1)
#define INVLEFT_STATUS_2	(2)
#define INVLEFT_STATUS_3	(3)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern struct Object_class InvLeft1_Class;
extern struct Object_class InvLeft2_Class;
extern struct Object_class InvLeft3_Class;

/*
 ** Prototypes *************************************************************
 */

#endif

