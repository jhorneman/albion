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

/* 2D map window parameters */
#define MAP_2D_X					(0)
#define MAP_2D_Y					(0)

#define MAX_MAPBUF_WIDTH 		(24)		/* Scroll margin is included ! */
#define MAX_MAPBUF_HEIGHT		(13)

#define MAPBUF_DEPTH				(4)

/*
 ** Structure definitions **************************************************
 */

/* 2D objects */
struct Object_2D {
	UNSHORT Flags;
	SISHORT Index;
	SILONG X, Y;
	UNSHORT Width, Height;
	UNSHORT Level;
	MEM_HANDLE Graphics_handle;
	UNLONG Graphics_offset;
};

/* Bitmasks for Object_2D.Flags */
#define ADD_ICONS_OVER_OBJECT		(1 << 0)

/*
 ** External global variables **********************************************
 */

extern struct OPM *Current_2D_OPM;

extern struct Scroll_data Scroll_2D;

extern UNSHORT Mapbuf_X, Mapbuf_Y;

extern UNSHORT Mapbuf_width, Mapbuf_height;
extern UNSHORT Map_2D_width, Map_2D_height;

/*
 ** Prototypes *************************************************************
 */

void Prepare_2D_display(void);

/* 2D map display functions */
void Draw_2D_map(void);
void Redraw_2D_objects(void);

void Draw_2D_scroll_buffer(void);

void Redraw_2D_map(void);

void Update_2D_map_area(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
void Update_2D_screen_area(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);

/* 2D object list management functions */
void Clear_2D_object_list(void);
void Add_2D_object(struct Object_2D *Object);

/* 2D map scroll unit update function */
void Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

/* 2D camera access functions */
void Set_2D_camera_position(UNLONG New_X, UNLONG New_Y);
void Get_2D_camera_position(UNLONG *X_ptr, UNLONG *Y_ptr);

void Set_2D_camera_target_position(UNLONG New_X, UNLONG New_Y);
void Get_2D_camera_target_position(UNLONG *X_ptr, UNLONG *Y_ptr);

void Move_2D_camera(SILONG dX, SILONG dY);

void Lock_2D_camera(void);
void Unlock_2D_camera(void);

/* Camera support functions */
void Calculate_2D_camera_target_position(void);

#endif

