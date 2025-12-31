/************
 * NAME     : XFTYPES.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 27-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef XFTYPES_H
#define XFTYPES_H

#include <BBDEF.H>
#include <BBMEM.H>
#include <XLOAD.H>

/*
 ** Defines ****************************************************************
 */

#define MAP_DATA			(0)
#define ICON_DATA			(1)
#define ICON_GFX			(2)
#define PALETTE			(3)
#define BASEPAL			(4)
#define SLAB				(5)
#define PART1_GFX			(6)
#define PART2_GFX			(7)
#define LAB_DATA			(8)
#define WALL_3D			(9)
#define OBJECT_3D			(10)
#define OVERLAY_3D		(11)
#define FLOOR_3D			(12)
#define SHADE_TABLE		(13)
#define BACKGROUND_GFX	(14)
#define FONTS				(15)
#define BLOCK_LIST		(16)

/*
 ** Global variables *******************************************************
 */

extern struct XFile_type Xftypes[];

#endif

