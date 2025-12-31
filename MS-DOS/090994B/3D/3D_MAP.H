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

/*
 ** Defines ****************************************************************
 */

// map max 127 x 127

#define FLOORS_MAX			( 50)
#define OBJECTS_MAX			(100)
#define WALLS_MAX			( 60)
#define OVERLAYS_MAX			(  8)	/* Per wall */
#define TEXTURES_MAX			(200)

#define OBJECTS_PER_GROUP		(  8)
#define FIRST_WALL			(101)

/*
 ** Structure definitions **************************************************
 */

typedef	struct

	{	UNSHORT Wall_height_cm;
		UNSHORT Horizon_Y;
		UNSHORT Shade_table_nr;
		UNSHORT Background_graphics_nr;
		UNSHORT Nr_object_groups;
	}
	Lab_data;



/* 3D map square */

typedef	struct

	{	UNBYTE	Wall_layer;		/* 0 = empty,
						   1...FIRST_WALL-1 = object group number,
						   FIRST_WALL...255 = wall number */
		UNBYTE	Floor_layer;		/* 0 = empty, 1...255 = floor number */
		UNBYTE	Ceiling_layer;		/* 0 = empty, 1...255 = floor (!) number */
	}
	Square_3D;


typedef	struct

	{	SISHORT	X,Y,Z;			/* Coordinates of object in map square (0...1023) */
		UNSHORT	Number;			/* Object number (0 = no object) */
	}
	Object_in_group_3D;


typedef struct

	{	UNSHORT		   Automapper_icon;		/* Number of automapper icon */
		Object_in_group_3D Group[ OBJECTS_PER_GROUP];	/* The objects in this group */
	}
	Object_group_3D;


typedef struct

	{	UNLONG	Flags;
		UNBYTE	Nr_frames;		/* Number of animation frames (1...8) */
		UNBYTE	_pad01;
		UNSHORT	Graphics_nr;		/* Number of floor graphics file */
	}
	Floor_3D;


typedef struct

	{	UNLONG	Flags;
		UNSHORT	Graphics_nr;		/* Number of object graphics file */
		UNBYTE	Nr_frames;		/* Number of animation frames (1...8) */
		UNBYTE	_pad01;
		UNSHORT	Graphics_width;		/* In pixels */
		UNSHORT	Graphics_height;
		UNSHORT	Object_width;		/* In cm */
		UNSHORT	Object_height;
	}
	Object_3D;


/* Bitmasks for Object_3D.Flags */

#define OBJECT_DISTORTION		(1 << 2)	/* 0 = normal, 1 = distort */

typedef struct

	{	UNLONG	Flags;
		UNSHORT	Graphics_nr;		/* Number of wall graphics file */
		UNBYTE	Nr_frames;		/* Number of animation frames (1...8) */
		UNBYTE	Automapper_icon;
		UNSHORT	Transparent_colour;
		UNSHORT	Graphics_width;		/* In pixels */
		UNSHORT	Graphics_height;
		UNSHORT	Nr_overlays;
	}
	Wall_3D;



/* Bitmasks for Wall_3D.Flags */

#define MASKED_WALL			(1 << 5)	/* 0 = block, 1 = mask */
#define TRANSPARENT_WALL		(1 << 6)	/* Has priority over previous flag */



typedef	struct

	{	UNSHORT	Graphics_nr;		/* Number of overlay graphics file */
		UNBYTE	Nr_frames;		/* Number of animation frames (1...8) */
		UNBYTE	Mask_flag;		/* 0 = block, 1 = mask */
		UNSHORT	X,Y;			/* In pixels */
		UNSHORT	Graphics_width;		/* In pixels */
		UNSHORT	Graphics_height;
	}
	Overlay_3D;


typedef	struct

	{	UNLONG	X,Z;			/* In 3D units */
		UNSHORT	Group_nr;
		UNSHORT	NPC_nr;
	}
	NPC_3D;


/*
 ** External global variables **********************************************
 */

// extern struct Module M3_Mod;


/*
 ** Prototypes *************************************************************
 */

void	Init_3D_map( void);
void	Exit_3D_map( void);
void	Display_3D_map( void);
void	Update_3D_map( void);

void	Init_3D_object( struct Object *Object, union Method_parms *P);
void	Exit_3D_object( struct Object *Object, union Method_parms *P);
void	Handle_keys_3D( struct Object *Object, union Method_parms *P);

void	Load_3D_floors( Floor_3D *Floor_ptr);
void	Load_3D_textures( Object_3D *Object_ptr, Wall_3D *Wall_ptr);
void	Copy_texture( struct Texture_3D *Texture, UNBYTE *Source);
void	Position_3D_textures( UNSHORT Start, UNSHORT End, UNSHORT Height);
void	Swap_3D_textures( UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN	Compare_3D_textures( UNLONG A, UNLONG B, UNBYTE *Data);
void	Load_3D_overlays( Wall_3D *Wall_ptr);

UNBYTE	FAR *Get_object_graphics( UNSHORT NPC_nr, UNSHORT Object_nr, UNSHORT Hash_nr);
UNBYTE	FAR *Get_floor_graphics( UNSHORT Floor_nr, UNSHORT Hash_nr);
UNBYTE	FAR *Get_wall_graphics( UNSHORT Wall_nr, UNSHORT Hash_nr);
UNBYTE	FAR *Get_wall_shade_table( UNSHORT Wall_nr);

void	Put_overlay_on_wall( Wall_3D *Wall, Overlay_3D *Overlay,
		UNBYTE *Wall_graphics, UNBYTE *Overlay_graphics);

#endif

