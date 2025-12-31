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

#define MAP_DATA		(0)
#define ICON_DATA		(1)
#define ICON_GFX		(2)
#define PALETTE		(3)
#define BASEPAL		(4)
#define SLAB			(5)
#define PARTY_GFX		(6)

/*
 ** Global variables *******************************************************
 */

extern struct XFile_type Xftypes[];

#endif

