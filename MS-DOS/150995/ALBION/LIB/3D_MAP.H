/************
 * NAME     : 3D_MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP3D_H
#define MAP3D_H

#include <BBDEF.H>
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

/* Maximum 3D map size : 127 x 127 */

#define OBJECTS_PER_GROUP	(8)
#define FIRST_WALL			(101)

/*
 ** Structure definitions **************************************************
 */

struct Lab_data {
	UNSHORT Wall_height;					/* In cm */
	SISHORT Horizon_Y;
	UNSHORT Shade_table_nr;
	UNSHORT Background_graphics_nr;
	SISHORT Background_offset;
	UNSHORT Shade_factor;
	UNSHORT Target_R;
	UNSHORT Target_G;
	UNSHORT Target_B;
	UNSHORT Floor_colour;
	UNSHORT Sky_colour;
	UNSHORT Function_A;
	UNSHORT Function_B;
	UNSHORT Square_size_cm_log;
	UNSHORT Background_angle_factor;
	UNSHORT View_depth;
	UNSHORT Flags;
	UNSHORT Shade_deviation;
	UNSHORT _4_future_use_;
	UNSHORT Nr_object_groups;
};

/* Bitmasks for Lab_data.Flags */
#define LAB_DATA_INIT		(1)

/* 3D map square */
struct Square_3D {
	UNBYTE Wall_layer;			/*	0 = empty,
											1...FIRST_WALL-1 = object group number,
											FIRST_WALL...255 = wall number */
	UNBYTE Floor_layer;			/*	0 = empty, 1...255 = floor number */
	UNBYTE Ceiling_layer;		/*	0 = empty, 1...255 = floor (!) number */
};

struct Object_in_group_3D {
	SISHORT X, Z, Y;				/* Coordinates of object in map square in cm */
	UNSHORT Number;				/* Object number (0 = no object) */
};

struct Object_group_3D {
	UNSHORT Automapper_icon;	/* Number of automapper icon */
	struct Object_in_group_3D Group[OBJECTS_PER_GROUP];	/* The objects in this group */
};

struct Floor_3D {
	UNLONG Flags;
	UNBYTE Nr_frames;				/* Number of animation frames (1...8) */
	UNBYTE Colour;
	UNSHORT Graphics_nr;			/* Number of floor graphics file */
	UNSHORT Height;				/* Height (position!) in cm */
};

/* Bitmasks for Floor_3D.Flags */
#define FLOOR_NO_SHADING		(1<<1)	/* 0 = shade, 1 = don't shade */
#define FLOOR_NO_VANISH			(1<<7)	/* 0 = vanish, 1 = don't vanish */

struct Object_3D {
	UNLONG Flags;
	UNSHORT Graphics_nr;			/* Number of object graphics file */
	UNBYTE Nr_frames;				/* Number of animation frames (1...8) */
	UNBYTE _pad01;
	UNSHORT Graphics_width;		/* In pixels */
	UNSHORT Graphics_height;
	UNSHORT Object_width;		/* In cm */
	UNSHORT Object_height;
};

/* Bitmasks for Object_3D.Flags */
#define OBJECT_NO_SHADING		(1<<1)	/* 0 = shade, 1 = don't shade */
#define OBJECT_DISTORTION		(1<<2)	/* 0 = normal, 1 = distort */
#define OBJECT_NO_VANISH		(1<<7)	/* 0 = vanish, 1 = don't vanish */

struct Wall_3D {
	UNLONG Flags;
	UNSHORT Graphics_nr;			/* Number of wall graphics file */
	UNBYTE Nr_frames;				/* Number of animation frames (1...8) */
	UNBYTE Automapper_icon;
	UNSHORT Transparent_colour;
	UNSHORT Graphics_width;		/* In pixels */
	UNSHORT Graphics_height;
	UNSHORT Nr_overlays;
};

/* Bitmasks for Wall_3D.Flags */
#define WALL_NO_SHADING			(1<<1)	/* 0 = shade, 1 = don't shade */
#define WALL_VISION_BLOCKED	(1<<2)
#define MASKED_WALL				(1<<5)	/* 0 = block, 1 = mask */
#define TRANSPARENT_WALL		(1<<6)	/* Has priority over previous flag */
#define WALL_NO_VANISH			(1<<7)	/* 0 = vanish, 1 = don't vanish */

struct Overlay_3D {
	UNSHORT Graphics_nr;			/* Number of overlay graphics file */
	UNBYTE Nr_frames;				/* Number of animation frames (1...8) */
	UNBYTE Mask_flag;				/* 0 = block, 1 = mask */
	UNSHORT X, Y;					/* In pixels */
	UNSHORT Graphics_width;		/* In pixels */
	UNSHORT Graphics_height;
};

struct NPC_3D {
	UNLONG X, Z;					/* In cm */
	UNSHORT Group_nr;
	UNSHORT NPC_nr;				/* 1... */
};

/* 3D movement data */
struct Movement_3D {
	SILONG X, Y;					/* Changed */
	SILONG dX, dY;					/* Changed */
	UNSHORT Travel_mode;			/* Input */
	UNSHORT NPC_index;			/* Input */
};

/*
 ** External global variables **********************************************
 */

extern struct Module M3_Mod;

extern UNSHORT Window_3D_X, Window_3D_Y;

extern struct Interface_3DM I3DM;

extern SILONG yawSIN;
extern SILONG yawCOS;

/*
 ** Prototypes *************************************************************
 */

void Display_3D_map(void);

void Initialize_3D_position(void);

/* 3D movement and collision detection */
BOOLEAN Make_3D_move(struct Movement_3D *Move);

BOOLEAN Movement_check_3D(SISHORT Map_X, SISHORT Map_Y, UNSHORT NPC_index,
 UNSHORT Travel_mode);

/* Coordinate conversion functions */
void Map_to_3D(SISHORT Map_X, SISHORT Map_Y, UNLONG *X_3D_ptr,
 UNLONG *Y_3D_ptr);
void _3D_to_map(UNLONG X_3D, UNLONG Y_3D, SISHORT *Map_X_ptr,
 SISHORT *Map_Y_ptr);

#endif

