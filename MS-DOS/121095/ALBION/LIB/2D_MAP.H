/************
 * NAME     : 2D_MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP_2D_H
#define MAP_2D_H

#include <BBDEF.H>
#include <CONTROL.H>
#include <SCROLL.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* 2D map square */
struct Square_2D {
	UNBYTE m[3];
};

/* 2D icon data */
struct Icon_2D_data {
	UNLONG Flags;
	UNSHORT First_frame;
	UNBYTE Nr_frames;
	UNBYTE _pad01;
};

/* Bit masks for Icon_2D_data.Flags */
#define UNDERLAY_PRIORITY		(1 << 1)
#define OVERLAY					(1 << 2)
#define ICON_HEIGHT				(0x00000060)
#define ICON_HEIGHT_B			(5)
#define BLOCKED_DIRECTIONS		(0x00000780)
#define BLOCKED_DIRECTIONS_B	(7)
#define CLONE_ICON				(1 << 18)
#define RIGHT_SLOPE				(1 << 19)
#define LEFT_SLOPE				(1 << 20)
#define ICON_INVISIBLE			(1 << 21)
#define ADD_NEXT_ICON			(1 << 22)

/* 2D cut-map blocks */
struct Cut_block_2D {
	UNBYTE Width, Height;
	/* Followed by Square_2D[Height][Width] */
};

/*
 ** External global variables **********************************************
 */

extern struct Module M2_Mod;

extern UNLONG Party_object_X, Party_object_Y;

extern MEM_HANDLE Icondata_handle;
extern MEM_HANDLE Icongfx_handle;
extern MEM_HANDLE Blocklist_handle;

extern BOOLEAN Big_guy;

/*
 ** Prototypes *************************************************************
 */

/* 2D map module */
void M2_MainLoop(void);

void Display_2D_map(void);

UNSHORT Search_2D_party_member(UNSHORT X, UNSHORT Y);

void Convert_2D_map_to_screen_coordinates(UNSHORT Map_X, UNSHORT Map_Y,
 SISHORT *Screen_X, SISHORT *Screen_Y);

/* Coordinate conversion functions */
void Map_to_2D(SISHORT Map_X, SISHORT Map_Y, UNLONG *X_2D_ptr,
 UNLONG *Y_2D_ptr);
void _2D_to_map(UNLONG X_2D, UNLONG Y_2D, SISHORT *Map_X_ptr,
 SISHORT *Map_Y_ptr);

void Initialize_2D_position(void);

#endif

