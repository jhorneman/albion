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

/*
 ** Defines ****************************************************************
 */

#define FIRST_ICON_SET_SIZE	(1536)
#define MAX_2D_OBJECTS			(50)
#define MAP2D_WIDTH				(360)
#define MAP2D_HEIGHT				(192)
#define MAPBUF_WIDTH				(24)		/* Scroll margin is included ! */
#define MAPBUF_HEIGHT			(13)
#define MAPBUF_DEPTH				(4)

/*
 ** Structure definitions **************************************************
 */

/* 2D icon data */
typedef struct {
	UNLONG Flags;
	UNSHORT First_frame;
	UNBYTE Nr_frames;
	UNBYTE _pad01;
} Icon_2D_data;

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
typedef struct {
	UNSHORT Index;
	SILONG X,Y;
	UNSHORT Width, Height;
	UNSHORT Level;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;
} Object_2D;

/* 2D rectangles */
typedef struct {
	SISHORT X,Y;
	SISHORT X2,Y2;
} Rectangle_2D;

/*
 ** Prototypes *************************************************************
 */

void Init_2D_map(void);
void Exit_2D_map(void);
void Update_2D_map(void);
void Display_2D_map(void);

void Handle_keys_2D(UNSHORT Key_code);

void Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

void Set_update_rectangle(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
void Set_erase_rectangles(void);

void Yagga(void);

void Swap_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);

#endif

