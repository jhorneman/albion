/************
 * NAME     : 2D_MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP_2D__H
#define MAP_2D__H

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

/* 2D movement data */
struct Movement_2D {
	UNSHORT Flags;					/* See below */
	UNSHORT X, Y;					/* Changed */
	UNSHORT Direction;			/* Input */
	SISHORT dX, dY;				/* Output */
	UNSHORT View_direction;		/* Changed */
	UNSHORT Slide_direction;	/* Internal */
	UNSHORT Reverse_slide;		/* Internal */
	UNSHORT Travel_mode;			/* Input */
	UNSHORT NPC_index;			/* Input */
	UNSHORT State;					/* Changed */
};

/* Bitmasks for Movement_2D.Flags */
#define UPDATE_AFTER_MOVE		(1 << 0)		/* Output */
#define DONT_SUCK					(1 << 1)    /* Internal */

/*
 ** External global variables **********************************************
 */

extern struct Module M2_Mod;

extern UNLONG Party_object_X, Party_object_Y;

extern UNSHORT _2D_speed;
extern UNSHORT Preview_direction;

extern UNBYTE *Icon_graphics_ptr;
extern struct Icon_2D_data *Icon_data_ptr;

extern MEM_HANDLE Icondata_handle;
extern MEM_HANDLE Blocklist_handle;

extern UNSHORT Party_animation[];

extern struct Scroll_data Scroll_2D;

extern BOOLEAN Big_guy;

/*
 ** Prototypes *************************************************************
 */

void Display_2D_map(void);

/* 2D movement and collision detection */
void Make_2D_move(UNSHORT Direction);

BOOLEAN Try_2D_move(struct Movement_2D *Move);

void Move_2D(SISHORT dX, SISHORT dY, BOOLEAN Update);

UNSHORT Search_2D_party_member(UNSHORT X, UNSHORT Y);

void Convert_2D_map_to_screen_coordinates(UNSHORT Map_X, UNSHORT Map_Y,
 UNSHORT *Screen_X, UNSHORT *Screen_Y);

#endif

