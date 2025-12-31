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

// map max 127 x 127

#define FLOORS_MAX			(50)
#define OBJECTS_MAX			(100)
#define WALLS_MAX				(60)
#define OVERLAYS_MAX			(8)			/* Per wall */
#define TEXTURES_MAX			(200)

#define OBJECTS_PER_GROUP	(8)
#define FIRST_WALL			(101)
#define FULLTURN_STEPS		(16)

/*
 ** Structure definitions **************************************************
 */

struct Lab_data {
	UNSHORT Wall_height_cm;
	UNSHORT Horizon_Y;
	UNSHORT Shade_table_nr;
	UNSHORT Background_graphics_nr;
	UNSHORT Nr_object_groups;
};

/* 3D map square */
struct Square_3D {
	UNBYTE Wall_layer;			/*	0 = empty,
											1...FIRST_WALL-1 = object group number,
											FIRST_WALL...255 = wall number */
	UNBYTE Floor_layer;			/*	0 = empty, 1...255 = floor number */
	UNBYTE Ceiling_layer;		/*	0 = empty, 1...255 = floor (!) number */
};

struct Object_in_group_3D {
	SISHORT X,Z,Y;					/* Coordinates of object in map square in cm */
	UNSHORT Number;				/* Object number (0 = no object) */
};

struct Object_group_3D {
	UNSHORT Automapper_icon;	/* Number of automapper icon */
	struct Object_in_group_3D Group[OBJECTS_PER_GROUP];	/* The objects in this group */
};

struct Floor_3D {
	UNLONG Flags;
	UNBYTE Nr_frames;				/* Number of animation frames (1...8) */
	UNBYTE _pad01;
	UNSHORT Graphics_nr;			/* Number of floor graphics file */
};

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
#define OBJECT_DISTORTION		(1<<2)	/* 0 = normal, 1 = distort */

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
#define MASKED_WALL				(1<<5)	/* 0 = block, 1 = mask */
#define TRANSPARENT_WALL		(1<<6)	/* Has priority over previous flag */

struct Overlay_3D {
	UNSHORT Graphics_nr;			/* Number of overlay graphics file */
	UNBYTE Nr_frames;				/* Number of animation frames (1...8) */
	UNBYTE Mask_flag;				/* 0 = block, 1 = mask */
	UNSHORT X,Y;					/* In pixels */
	UNSHORT Graphics_width;		/* In pixels */
	UNSHORT Graphics_height;
};

struct NPC_3D {
	UNLONG X,Z;						/* In 3D units */
	UNSHORT Group_nr;
	UNSHORT NPC_nr;				/* 1... */
};

/*
 ** External global variables **********************************************
 */

extern struct Module M3_Mod;

extern MEM_HANDLE Lab_data_handle;
extern UNSHORT Nr_of_object_groups, Nr_of_floors, Nr_of_objects, Nr_of_walls;

extern SILONG yawSIN;
extern SILONG yawCOS;

/*
 ** Prototypes *************************************************************
 */

void Init_3D_object(struct Object *Object, union Method_parms *P);
void Exit_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Custommouse_3D(struct Object *Object, union Method_parms *P);
void Customkeys_3D(struct Object *Object, union Method_parms *P);

void Init_3D_map(void);
void Exit_3D_map(void);
void M3_Main_loop(void);
void Display_3D_map(void);

void Load_3D_floors(struct Floor_3D *Floor_ptr);
void Load_3D_textures(struct Object_3D *Object_ptr, struct Wall_3D *Wall_ptr);
void Copy_texture(struct Texture_3D *Texture, UNBYTE *Source);
void Copy_texture2(struct Texture_3D *Texture, UNBYTE *Source);
void Position_3D_textures(SISHORT Start, SISHORT End, UNSHORT Height);
void Swap_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data);
void Load_3D_overlays(struct Wall_3D *Wall_ptr);

UNBYTE *Get_object_graphics(UNSHORT NPC_nr, UNSHORT Object_nr, UNSHORT Hash_nr);
UNBYTE *Get_floor_graphics(UNSHORT Floor_nr, UNSHORT Hash_nr);
UNBYTE *Get_wall_graphics(UNSHORT Wall_nr, UNSHORT Hash_nr);
UNBYTE *Get_wall_shade_table(UNSHORT Wall_nr);

void Put_overlay_on_wall(struct Overlay_3D *Overlay, UNBYTE *Wall_graphics,
 UNBYTE *Overlay_graphics);
void Remove_overlay_from_wall(struct Overlay_3D *Overlay,
 UNBYTE *Source_wall_graphics, UNBYTE *Target_wall_graphics);
void Colour_cycle_forward_3D(UNSHORT Start, UNSHORT Size);

void Calculate_shade_table(struct BBPALETTE *Palette, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Number);
UNBYTE Find_closest_colour(struct BBPALETTE *Palette, UNSHORT Start, UNSHORT Number,
 UNSHORT Red, UNSHORT Green, UNSHORT Blue);
void Convert_lab_data(UNBYTE *Ptr, MEM_HANDLE Handle);
void Calculate_shade_function(void);
void Initialize_3D_position(void);
void Rotate_3D(SISHORT dAlpha);
void Add_3D_vector(SILONG dX, SILONG dY);

#endif

