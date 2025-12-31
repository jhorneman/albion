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

#define FIRST_ICON_SET_SIZE	(1536)

#define PARTY_PATH_LENGTH		(256)

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
	UNSHORT Flags;
	UNSHORT X, Y;
	UNSHORT Direction;
	SISHORT dX, dY;
	UNSHORT View_direction;
	UNSHORT Slide_direction;
	UNSHORT Reverse_slide;
	UNSHORT Travel_mode;
	UNSHORT NPC_index;
	UNSHORT State;
};

/* Bitmasks for Movement_2D.Flags */
#define UPDATE_AFTER_MOVE		(1<<0)
#define DONT_SUCK					(1<<1)

/*
 ** External global variables **********************************************
 */

extern struct Module M2_Mod;

extern UNLONG Party_object_X, Party_object_Y;

extern UNSHORT _2D_speed;
extern UNSHORT Preview_direction;

extern UNBYTE *Icon_graphics[2];
extern struct Icon_2D_data *Icon_data[2];

extern MEM_HANDLE Icondata_handle[];
extern MEM_HANDLE Blocklist_handle[];

extern UNSHORT Party_animation[];

extern struct Scroll_data Scroll_2D;

extern BOOLEAN Big_guy;

/*
 ** Prototypes *************************************************************
 */

/* 2D map object methods */
UNLONG Touched_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Left_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Right_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_2D_object(struct Object *Object, union Method_parms *P);

/* 2D map module */
void M2_ModInit(void);
void M2_ModExit(void);
void Init_2D_display(void);
void Exit_2D_display(void);
void M2_Main_loop(void);

void Display_2D_map(void);

/* 2D map move mode */
void Move_M2_Main_loop(void);
void Mouse_move_2D(void);
void Init_Move_M2(void);
void Exit_Move_M2(void);

/* 2D map selection mode */
void Select_M2_Main_loop(void);
void Display_M2_selection(UNSHORT X, UNSHORT Y);
void Init_Select_M2(void);
void Exit_Select_M2(void);

UNSHORT Get_2D_mouse_state(SISHORT X, SISHORT Y);

/* 2D movement and collision detection */
void Make_2D_move(UNSHORT Direction);

BOOLEAN Try_2D_move(struct Movement_2D *Move);

BOOLEAN Special_2D_high_check(UNSHORT X, UNSHORT Y);
BOOLEAN Special_2D_low_check(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY);

BOOLEAN Double_movement_check_2D(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY,
 UNSHORT Travel_mode, UNSHORT NPC_index);
BOOLEAN Extra_vertical_movement_check_2D(UNSHORT X, UNSHORT Y,
 UNSHORT NPC_index);

BOOLEAN Do_check_2D(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY,
 UNSHORT Travel_mode, UNSHORT NPC_index);
BOOLEAN Check_2D_NPC_collision(UNSHORT X, UNSHORT Y, UNSHORT NPC_index);

BOOLEAN Sucking_chairs_2D(struct Movement_2D *Move);

void Move_2D(SISHORT dX, SISHORT dY, BOOLEAN Update);
void Update_2D_party_position(BOOLEAN Movement);
void Store_2D_movement(SISHORT dX, SISHORT dY);
void Display_2D_party(void);

UNSHORT Search_2D_party_member(UNSHORT X, UNSHORT Y);

void Convert_2D_map_to_screen_coordinates(UNSHORT Map_X, UNSHORT Map_Y,
 UNSHORT *Screen_X, UNSHORT *Screen_Y);

#endif

