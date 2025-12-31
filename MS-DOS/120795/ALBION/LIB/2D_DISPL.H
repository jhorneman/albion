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

#define MAX_2D_OBJECTS			(120)

/* 2D map window parameters */
#define MAP_2D_X					(0)
#define MAP_2D_Y					(0)

#define MAPBUF_WIDTH_MAX 		(24)		/* Scroll margin is included ! */
#define MAPBUF_HEIGHT_MAX		(13)

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
#define ADD_ICONS_OVER_OBJECT		(1<<0)

/* 2D rectangles */
struct Rectangle_2D {
	SISHORT X, Y;
	SISHORT X2, Y2;
};

/*
 ** External global variables **********************************************
 */

extern struct OPM *Current_2D_OPM;

extern UNSHORT Mapbuf_X, Mapbuf_Y;
extern UNLONG Camera_2D_X, Camera_2D_Y;

extern UNSHORT Mapbuf_width, Mapbuf_height;
extern UNSHORT Map_2D_width, Map_2D_height;

/*
 ** Prototypes *************************************************************
 */

/* Camera management functions */
void Prepare_2D_display(void);
void Set_2D_camera(void);
void Initialize_2D_camera(void);
void Calculate_2D_camera_position(UNLONG *X_ptr, UNLONG *Y_ptr);

/* 2D map scroll unit update function */
void Update_2D_unit(struct Scroll_data *Scroll, UNSHORT Buffer_X,
 UNSHORT Buffer_Y, UNSHORT Playfield_X, UNSHORT Playfield_Y);

/* 2D map display functions */
void Draw_2D_map(UNSHORT Display_X, UNSHORT Display_Y);
void Draw_2D_scroll_buffer(UNSHORT Display_X, UNSHORT Display_Y);

void Display_extra_icons(UNSHORT Underlay_nr, UNSHORT Overlay_nr, UNSHORT X,
 UNSHORT Y);
void Display_underlay(UNSHORT Underlay_nr, UNSHORT X, UNSHORT Y);
void Display_overlay(UNSHORT Overlay_nr, UNSHORT X, UNSHORT Y);
void Display_2D_object(struct Object_2D *Object);

/* 2D object sort functions */
void Swap_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_2D_objects(UNLONG A, UNLONG B, UNBYTE *Data);

/* 2D map display support functions */
void Set_animation_update(void);
void Set_update_rectangle(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height);
void Redraw_2D_map(void);
void Set_erase_rectangles(void);
void Fill_map_buffer(void);

/* 2D object list management functions */
void Clear_2D_object_list(void);
void Add_2D_object(struct Object_2D *Object);

/* Diagnostic functions */
void Display_underlay_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNLONG Flags);
void Display_overlay_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNLONG Flags);
void Display_event_diagnostics(struct OPM *OPM, UNSHORT X, UNSHORT Y,
 UNSHORT Map_X, UNSHORT Map_Y);

#endif

