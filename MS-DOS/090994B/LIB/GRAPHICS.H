/************
 * NAME     : XFTYPES.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 27-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <BBDEF.H>
#include <BBOPM.H>

/*
 ** Defines ****************************************************************
 */

#define DEFAULT_MPTR			(0)
#define FORWARD_MPTR			(1)
#define BACKWARD_MPTR		(2)
#define LEFT3D_MPTR			(3)
#define RIGHT3D_MPTR			(4)
#define TURNLEFT_MPTR		(5)
#define TURNRIGHT_MPTR		(6)
#define TURN180LEFT_MPTR	(7)
#define TURN180RIGHT_MPTR	(8)
#define UP2D_MPTR				(9)
#define DOWN2D_MPTR			(10)
#define LEFT2D_MPTR			(11)
#define RIGHT2D_MPTR			(12)
#define UPLEFT_MPTR			(13)
#define UPRIGHT_MPTR			(14)
#define DOWNRIGHT_MPTR		(15)
#define DOWNLEFT_MPTR		(16)

/*
 ** Global variables *******************************************************
 */

extern struct Mouse_pointer Mouse_pointers[];

#endif

