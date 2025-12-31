/************
 * NAME     : 2D_MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP2D_H
#define MAP2D_H

#include <BBDEF.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

#define FIRST_ICON_SET_SIZE	(1536)

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
#define UNDERLAY_PRIORITY		(0x00000002)
#define OVERLAY					(0x00000004)
#define ICON_HEIGHT				(0x00000060)
#define ICON_HEIGHT_B			(5)
#define BLOCKED_DIRECTIONS		(0x00000780)
#define BLOCKED_DIRECTIONS_B	(7)
#define RIGHT_SLOPE				(0x00080000)
#define LEFT_SLOPE				(0x00100000)
#define PERSON_INVISIBLE		(0x00200000)
#define ADD_NEXT_ICON			(0x00400000)

/*
 ** External global variables **********************************************
 */

extern struct Module M2_Mod;
extern UNBYTE *Icon_graphics[2];
extern struct Icon_2D_data *Icon_data[2];
extern MEM_HANDLE Icondata_handle[];

extern struct Scroll_data Scroll_2D;

/*
 ** Prototypes *************************************************************
 */

void Init_2D_object(struct Object *Object, union Method_parms *P);
void Exit_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Custommouse_2D(struct Object *Object, union Method_parms *P);
void Customkeys_2D(struct Object *Object, union Method_parms *P);

void Init_2D_map(void);
void Exit_2D_map(void);
void M2_Main_loop(void);
void Display_2D_map(void);

void Make_2D_move(UNSHORT Direction);
BOOLEAN Try_2D_move(UNSHORT X, UNSHORT Y, UNSHORT Direction);
BOOLEAN Movement_check_2D(UNSHORT X, UNSHORT Y);
BOOLEAN Do_check_2D(UNSHORT X, UNSHORT Y);
void Move_2D(SISHORT dX, SISHORT dY);

#endif

