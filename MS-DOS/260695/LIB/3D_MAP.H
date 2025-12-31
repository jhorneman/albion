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

#define MAX_3D_SHADE_LEVELS	(63)

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

struct Texture_3D {
	UNSHORT X;
	UNSHORT Width,Height;
	union {
		MEM_HANDLE Handle;
		struct Texture_3D **Ptr;
	} p;
};

/*
 ** External global variables **********************************************
 */

extern struct Module M3_Mod;

extern MEM_HANDLE Lab_data_handle;
extern UNSHORT Nr_of_object_groups, Nr_of_floors, Nr_of_objects, Nr_of_walls;

extern struct Texture_3D Floor_textures[FLOORS_MAX][8];
extern UNLONG Wall_offsets[WALLS_MAX];

extern SILONG yawSIN;
extern SILONG yawCOS;

/*
 ** Prototypes *************************************************************
 */

/* 3D map object methods */
UNLONG Touched_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Left_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Right_3D_object(struct Object *Object, union Method_parms *P);
UNLONG DLeft_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_3D_object(struct Object *Object, union Method_parms *P);

/* 3D map module */
void M3_ModInit(void);
void M3_ModExit(void);
void Init_3D_display(void);
void Exit_3D_display(void);
void M3_Main_loop(void);
void Display_3D_map(void);

void Error_3DM(UNSHORT Error_code);

/* 3D map data preparation */
void Load_3D_floors(struct Lab_data *Lab_ptr, struct Floor_3D *Floor_ptr);
MEM_HANDLE Allocate_floor_buffer(void);
void Load_3D_textures(struct Object_3D *Object_ptr, struct Wall_3D *Wall_ptr);
void Copy_texture(struct Texture_3D *Texture, UNBYTE *Source);
void Copy_texture2(struct Texture_3D *Texture, UNBYTE *Source);
void Position_3D_textures(SISHORT Start, SISHORT End, UNSHORT Height);
void Swap_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_3D_textures2(UNLONG A, UNLONG B, UNBYTE *Data);
void Load_3D_overlays(struct Wall_3D *Wall_ptr);

void Calculate_shade_tables_for_transparent_walls(void);
void Put_unanimated_overlays_on_unanimated_walls(void);

/* 3DM interface functions */
UNBYTE *Get_object_graphics(UNSHORT NPC_nr, UNSHORT Object_nr, UNSHORT Hash_nr);
UNBYTE *Get_floor_graphics(UNSHORT Floor_nr, UNSHORT Hash_nr, UNSHORT Resolution);
UNBYTE *Get_wall_graphics(UNSHORT Wall_nr, UNSHORT Hash_nr);
UNBYTE *Get_wall_shade_table(UNSHORT Wall_nr);

void Put_overlay_on_wall(struct Overlay_3D *Overlay, UNBYTE *Wall_graphics,
 UNBYTE *Overlay_graphics);
void Remove_overlay_from_wall(struct Overlay_3D *Overlay,
 UNBYTE *Source_wall_graphics, UNBYTE *Target_wall_graphics);
void Colour_cycle_forward_3D(UNSHORT Start, UNSHORT Size);

void Calculate_shade_table(struct BBPALETTE *Pal, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Number);
void Relocate_Shade_table(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target,
 UNLONG Size);

/* BOOLEAN Texture_vanish_test(UNSHORT Width, UNSHORT Height, UNBYTE *Ptr); */

void Convert_lab_data(UNBYTE *Ptr, MEM_HANDLE Handle);
void Calculate_shade_function(UNSHORT A_value, UNSHORT B_value);
void Initialize_3D_position(void);
void Rotate_3D(SISHORT dAlpha);
void Add_3D_vector(SILONG dX, SILONG dY);
void Set_3D_window_size(UNSHORT Width, UNSHORT Height);

/* 3D map move mode */
void MM3_Main_loop(void);
void Init_MM3(void);
void Exit_MM3(void);
void Mouse_move_3D(void);

UNSHORT Get_3D_mouse_state(SISHORT X, SISHORT Y);

/* 3D movement and collision detection */
void Make_3D_move(SILONG dX, SILONG dY);
BOOLEAN Try_3D_move(SILONG X, SILONG Y, UNSHORT NPC_index, UNSHORT Travel_mode);
BOOLEAN Movement_check_3D(SISHORT X, SISHORT Y, UNSHORT Travel_mode);
BOOLEAN Check_3D_object_group_collision(SISHORT Square_X, SISHORT Square_Z,
 SISHORT Group_X, SISHORT Group_Z, UNSHORT Travel_mode,
 struct Object_group_3D *Object_group, struct Object_3D *Object_base);

/* Coordinate conversion functions */
void Map_to_dungeon(UNSHORT Map_X, UNSHORT Map_Y, UNLONG *Dungeon_X_ptr,
 UNLONG *Dungeon_Y_ptr);
void Dungeon_to_map(UNLONG Dungeon_X, UNLONG Dungeon_Y, UNSHORT *Map_X_ptr,
 UNSHORT *Map_Y_ptr);

#endif

