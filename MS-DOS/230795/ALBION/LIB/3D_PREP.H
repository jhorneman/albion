/************
 * NAME     : 3D_PREP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 19-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef PREP3D_H
#define PREP3D_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define FLOORS_MAX			(50)
#define WALLS_MAX				(60)

/*
 ** Structure definitions **************************************************
 */

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

extern UNSHORT Nr_of_object_groups;
extern UNSHORT Nr_of_floors;
extern UNSHORT Nr_of_objects;
extern UNSHORT Nr_of_walls;

extern MEM_HANDLE Lab_data_handle;
extern MEM_HANDLE Shade_table_handle;
extern MEM_HANDLE Background_gfx_handle;

extern struct Texture_3D Floor_textures[FLOORS_MAX][8];
extern UNLONG Wall_offsets[WALLS_MAX];

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_3D_data(void);
void Exit_3D_data(void);

void Put_overlays_on_walls(void);

void Colour_cycle_forward_3D(UNSHORT Start, UNSHORT Size);

/* 3DM interface functions */
UNBYTE *Get_object_graphics(UNSHORT NPC_nr, UNSHORT Object_nr, UNSHORT Hash_nr);
UNBYTE *Get_floor_graphics(UNSHORT Floor_nr, UNSHORT Hash_nr, UNSHORT Resolution);
UNBYTE *Get_wall_graphics(UNSHORT Wall_nr, UNSHORT Hash_nr);
UNBYTE *Get_wall_shade_table(UNSHORT Wall_nr);

void Relocate_Shade_table(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target,
 UNLONG Size);

void Convert_lab_data(UNBYTE *Ptr, MEM_HANDLE Handle);

UNBYTE *Get_shade_table_address(void);

#endif

