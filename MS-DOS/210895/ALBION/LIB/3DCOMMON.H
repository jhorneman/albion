/************
 * NAME     : 3DCOMMON.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 16-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMMON3D_H
#define COMMON3D_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <3D_MAP.H>

/*
 ** Defines ****************************************************************
 */

#define ANGLE_STEPS			(16384)		/* Equals 360 degrees */

/*
 ** Structure definitions **************************************************
 */

struct Interface_3DM {
	UNLONG Flags;						/* See below */
	UNSHORT Player_X[3];				/* 32:16 bit player coordinates in cm */
	UNSHORT Player_Z[3];				/* (frac:low:high) */
	UNLONG Camera_angle;				/* 16:16 bit */

	UNSHORT Camera_height;			/* In cm */
	SISHORT Horizon_Y;				/* In pixels measured from the middle of the 3D-window */
	SISHORT Horizon_offset;
	UNSHORT Horizon_angle_factor;	/* Factor (360 / f) */

	UNBYTE Sky_colour;				/* Farbe des Himmel-Bereichs (Åber dem Horizont) */
	UNBYTE Floor_colour;				/* Farbe des Boden-Bereichs (unter dem Horizont) */

	UNSHORT Window_3D_width;		/* In pixels */
	UNSHORT Window_3D_height;

	UNSHORT Wall_height;	 			/* In cm */
	UNSHORT Square_size;				/* In cm */

	struct Object_group_3D *Object_groups;
	struct Floor_3D *Floors;
	struct Object_3D *Objects;
	struct Wall_3D **Wall_pointers;	/* Pointer to list of pointers to wall data */

	struct NPC_3D *NPCs;				/* Pointer to NPC list */
	UNSHORT Nr_NPCs;					/* Number of NPCs in list (max. 32) */

	struct Square_3D *Map_pointer;

	UNSHORT Default_colour;
	UNSHORT pad;

	UNBYTE *RunLengthCode;			/* Pointer to the run-length code of the recently selected object */

	UNBYTE *Shade_table;
	UNSHORT Shade_factor;
	SISHORT Shade_offsets[4];		/* North, East, South, West */

	UNBYTE *Background_graphics;	/* NULL means no background */
	UNSHORT Background_width;		/* In pixels */
	UNSHORT Background_height;

	struct OPM *Target_OPM;

	SIBYTE Shade_function[1024];
};

/* Bitmasks for Interface_3DM.Flags */
#define DUNGEON_OR_CITY			(1<<0)
#define NONLINEAR_BACKGROUND	(1<<1)



/* DEKLARATION DER OBJECT-IDENTIFY-STRUKTUR */
/* ---------------------------------------- */

#define	T_NOTHING	0
#define	T_OBJECT		1
#define	T_NPC			2
#define	T_WALL		3
#define	T_SKY			4
#define	T_FLOOR		5

struct Identify_3D
{
	UNSHORT	testColor;  /* INPUT: "UNBENUTZTE" TEST-FARBE */
	UNSHORT	screenX;    /* INPUT: 2D-Screen-X-Koordinate */
	UNSHORT	screenY;    /* INPUT: 2D-Screen-Y-Koordinate */
	UNBYTE 	type;	    	/* RESULT: TYP DES GEFUNDENEN OBJEKTES (siehe oben) */
	UNBYTE 	id_nr;	   /* RESULT: ID INNERHALB EINER OBJECT/NPC-GROUP */
	UNBYTE 	mapX;	    	/* RESULT: X-POSITION IN DER MAP */
	UNBYTE 	mapY;	    	/* RESULT: Y-POSITION IN DER MAP */
};


/*
 ** External global variables **********************************************
 */

extern struct Interface_3DM I3DM;

#endif

