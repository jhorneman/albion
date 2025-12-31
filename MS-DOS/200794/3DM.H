
#define SQUARE_SIZE_3D		(1024)		/* In 3D units */
#define SQUARE_SIZE_CM		(512)			/* In cm */
#define ANGLE_STEPS			(2048)		/* Equals 360 degrees */

struct Interface_3DM {
	UNLONG Flags;					/* See below */
	UNSHORT Player_X[3];			/* 32:16 bit player coordinates in 3D units */
	UNSHORT Player_Z[3];
	UNLONG Camera_angle;			/* 16:16 bit */

	UNSHORT Camera_height;		/* In cm */
	SISHORT Horizon_Y;			/* In pixels measured from the middle of the 3D-window */

	UNSHORT Window_3D_width;	/* In pixels */
	UNSHORT Window_3D_height;

	UNSHORT Wall_height;			/* In cm */

	struct Object_group_3D *Object_groups;
	struct Floor_3D *Floors;	/* Probably not needed */
	struct Object_3D *Objects;
	struct Wall_3D **Wall_pointers;	/* Pointer to list of pointers to wall data */

	struct NPC_3D *NPCs;			/* Pointer to NPC list */
	UNSHORT Nr_NPCs;				/* Number of NPCs in list (max. 32) */

	struct Square_3D *Map_pointer;
	UNSHORT Map_width;			/* In map squares */
	UNSHORT Map_height;

	UNBYTE *Shade_table;
	UNSHORT Shade_factor;

	UNBYTE *Background_graphics;
	UNSHORT Background_width;	/* In pixels */
	UNSHORT Background_height;

	struct OPM *Target_OPM;
};

#define DUNGEON_OR_CITY		(1<<0)

#define FIRST_WALL			(101)

struct Square_3D {
	UNBYTE Wall_layer;			/*	0 = empty,
											1...FIRST_WALL-1 = object group number,
											FIRST_WALL...255 = wall number */
	UNBYTE Floor_layer;			/*	0 = empty, 1...255 = floor number */
	UNBYTE Ceiling_layer;		/*	0 = empty, 1...255 = floor (!) number */
};

#define OBJECTS_PER_GROUP	(8)

struct Object_group_3D {
	UNSHORT Automapper_icon;	/* Number of automapper icon */
	struct Object_in_group_3D[OBJECTS_PER_GROUP];	/* The objects in this group */
};

struct Object_in_group_3D {
	SISHORT X,Y,Z;					/* Coordinates of object in map square (0...1023) */
	UNSHORT Number;				/* Object number (0 = no object) */
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
	UNSHORT NPC_nr;
};

UNBYTE *Get_object_graphics(UNSHORT NPC_nr, UNSHORT Object_nr);
UNBYTE *Get_floor_graphics(UNSHORT Floor_nr);
UNBYTE *Get_wall_graphics(UNSHORT Wall_nr);
UNBYTE *Get_wall_shade_table(UNSHORT Wall_nr);
UNBYTE *Get_overlay_graphics(UNSHORT Wall_nr, UNSHORT Overlay_index);

