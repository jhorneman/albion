/************
 * NAME     : 3D_MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 11-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP_3D_H
#define MAP_3D_H

#include <BBDEF.H>
#include <CONTROL.H>
#include <3DCOMMON.H>

/*
 ** Defines ****************************************************************
 */

/* Maximum 3D map size : 127 x 127 */

#define OBJECTS_PER_GROUP	(8)
#define FIRST_WALL			(101)

#define ROTATE_3D_SPEED		(ANGLE_STEPS / 16)

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
#define WALL_NO_SHADING			(1L << 1)	/* 0 = shade, 1 = don't shade */
#define WALL_VISION_BLOCKED	(1L << 2)
#define MASKED_WALL				(1L << 5)	/* 0 = block, 1 = mask */
#define TRANSPARENT_WALL		(1L << 6)	/* Has priority over previous flag */
#define WALL_NO_VANISH			(1L << 7)	/* 0 = vanish, 1 = don't vanish */

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

/*
 ** External global variables **********************************************
 */

extern struct Module M3_Mod;

extern UNSHORT Window_3D_size;

extern UNSHORT Window_3D_X, Window_3D_Y;

extern struct Interface_3DM I3DM;

extern SILONG yawSIN;
extern SILONG yawCOS;

/*
 ** Prototypes *************************************************************
 */

void M3_MainLoop(void);

void Display_3D_map(void);

void Render_3D_map(void);

void Set_3D_window_size(UNSHORT Size);

#endif

