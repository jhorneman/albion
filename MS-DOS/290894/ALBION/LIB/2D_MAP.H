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
#define MAX_2D_OBJECTS			(50)

/* 2D map window parameters */
#define MAP2D_X					(0)
#define MAP2D_Y					(0)
#define MAP2D_WIDTH				(360)
//#define MAP2D_HEIGHT				(192)
#define MAP2D_HEIGHT				(240)

#define MAPBUF_WIDTH				(24)		/* Scroll margin is included ! */
#define MAPBUF_HEIGHT			(16)
//(13)

#define MAPBUF_DEPTH				(4)

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

/* 2D objects */
struct Object_2D {
	UNSHORT Index;
	SILONG X,Y;
	UNSHORT Width, Height;
	UNSHORT Level;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;
};

/* 2D rectangles */
struct Rectangle_2D {
	SISHORT X,Y;
	SISHORT X2,Y2;
};

/*
 ** External global variables **********************************************
 */

extern struct Module M2_Mod;

/*
 ** Prototypes *************************************************************
 */

void Init_2D_map(void);
void Exit_2D_map(void);

void Update_2D_map(void);
void Display_2D_map(void);

void Handle_keys_2D(struct Object *Object, union Method_parms *P);

void Init_2D_object(struct Object *Object, union Method_parms *P);
void Exit_2D_object(struct Object *Object, union Method_parms *P);

void Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

void Do_2D_map_display(void);
void Display_underlay(UNSHORT Underlay_nr, UNSHORT X, UNSHORT Y);
void Display_overlay(UNSHORT Overlay_nr, UNSHORT X, UNSHORT Y);
void Display_2D_object(struct Object_2D *Object);

void Swap_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);

void Set_animation_update(void);
void Set_update_rectangle(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
void Set_erase_rectangles(void);
void Fill_map_buffer(void);

#endif

