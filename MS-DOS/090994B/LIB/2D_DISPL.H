/************
 * NAME     : 2D_DISPL.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef DISPLAY2D_H
#define DISPLAY2D_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_2D_OBJECTS			(50)

/* 2D map window parameters */
#define MAP2D_X					(0)
#define MAP2D_Y					(0)
#define MAP2D_WIDTH				(360)
#define MAP2D_HEIGHT				(192)

#define MAPBUF_WIDTH				(24)		/* Scroll margin is included ! */
#define MAPBUF_HEIGHT			(13)

#define MAPBUF_DEPTH				(4)

/*
 ** Structure definitions **************************************************
 */

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

extern struct Object_2D *Object_2D_list[];
extern UNSHORT Nr_2D_objects;
extern UNSHORT Camera_2D_X, Camera_2D_Y;

/*
 ** Prototypes *************************************************************
 */

void Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

void Draw_2D_map(void);
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

