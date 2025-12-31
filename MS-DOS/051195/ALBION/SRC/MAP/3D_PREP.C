/************
 * NAME     : 3D_PREP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 19.07.95 13:44
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <string.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <SORT.H>
#include <FINDCOL.H>

#include <CONTROL.H>
#include <GAMEVAR.H>
#include <MAP.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <ANIMATE.H>
#include <XFTYPES.H>
#include <COLOURS.H>
#include <GRAPHICS.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_3D_SHADE_LEVELS	(63)

#define MAX_3D_OBJECTS			(100)
#define OVERLAYS_PER_WALL		(8)			/* Per wall */
#define MAX_3D_TEXTURES			(300)

/*
 ** Prototypes *************************************************************
 */

/* 3D map data preparation */
BOOLEAN Load_3D_floors(struct Lab_data *Lab_ptr, struct Floor_3D *Floor_ptr);

MEM_HANDLE Allocate_floor_buffer(void);

BOOLEAN Load_3D_textures(struct Object_3D *Object_ptr,
 struct Wall_3D *Wall_ptr);

void Copy_texture(struct Texture_3D *Texture, UNBYTE *Source);
void Copy_texture2(struct Texture_3D *Texture, UNBYTE *Source);

void Position_3D_textures(SISHORT Start, SISHORT End, UNSHORT Height);

void Swap_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_3D_textures2(UNLONG A, UNLONG B, UNBYTE *Data);

BOOLEAN Load_3D_overlays(struct Wall_3D *Wall_ptr);

void Init_shade_tables_for_transparent_walls(void);
void Exit_shade_tables_for_transparent_walls(void);
void Calculate_shade_tables_for_transparent_walls(void);

void Put_unanimated_overlays_on_unanimated_walls(void);

void Put_overlay_on_wall(struct Overlay_3D *Overlay, UNBYTE *Wall_graphics,
 UNBYTE *Overlay_graphics);

void Remove_overlay_from_wall(struct Overlay_3D *Overlay,
 UNBYTE *Source_wall_graphics, UNBYTE *Target_wall_graphics);

void Calculate_shade_table(struct BBPALETTE *Pal, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Number);

/* BOOLEAN Texture_vanish_test(UNSHORT Width, UNSHORT Height, UNBYTE *Ptr); */

void Calculate_shade_function(UNSHORT A_value, UNSHORT B_value);

/*
 ** Global variables *******************************************************
 */

/* 3D floor buffer file type */
static UNCHAR Floor_buffer_fname[] = "Floor buffer";

static struct File_type Floor_buffer_ftype = {
	NULL,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Floor_buffer_fname
};

/* 3D texture buffer file type */
static UNCHAR Texture_buffer_fname[] = "Texture buffer";

static struct File_type Texture_buffer_ftype = {
	MEM_Relocate,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Texture_buffer_fname
};

/* 3D shade table file type */
static UNCHAR Shade_table_fname[] = "Shade table";

static struct File_type Shade_table_ftype = {
	Relocate_on_256_byte_boundary,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Shade_table_fname
};

/* 3D wall shade table file type */
static UNCHAR Wall_shade_table_fname[] = "Wall shade table";

static struct File_type Wall_shade_table_ftype = {
	Relocate_on_256_byte_boundary,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Wall_shade_table_fname
};

UNSHORT Nr_3D_object_groups;
UNSHORT Nr_3D_floors;
UNSHORT Nr_3D_objects;
UNSHORT Nr_3D_walls;

static UNSHORT Nr_3D_floor_buffers;
static UNSHORT Nr_3D_textures;
static UNSHORT Nr_3D_texture_buffers;

#ifdef PREP_3D_DEBUG_FLAG
UNLONG Actual_size;
UNLONG Converted_size;
UNSHORT Nr_textures;
#endif

MEM_HANDLE Lab_data_handle;

MEM_HANDLE Shade_table_handle;
MEM_HANDLE Background_gfx_handle;

BOOLEAN Shade_tables_are_valid;

static MEM_HANDLE Floor_buffer_handles[MAX_3D_FLOORS * 2];
static MEM_HANDLE Texture_buffer_handles[MAX_3D_TEXTURES];
static MEM_HANDLE Overlay_handles[MAX_3D_WALLS][8];
static MEM_HANDLE Wall_shade_tables[MAX_3D_WALLS];

struct Texture_3D Floor_textures[MAX_3D_FLOORS][8];

static struct Texture_3D Textures[(MAX_3D_OBJECTS * 8) + (MAX_3D_WALLS * 9)];
static struct Texture_3D *Object_textures[MAX_3D_OBJECTS][8];
static struct Texture_3D *Wall_textures[MAX_3D_WALLS][9];

UNLONG Wall_offsets[MAX_3D_WALLS];
static UNBYTE Wall_states[MAX_3D_WALLS];

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_3D_data
 * FUNCTION  : Initialize 3D data.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.07.95 13:53
 * LAST      : 14.10.95 22:35
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_3D_data(void)
{
	struct Map_data *Map_ptr;
	struct Lab_data *Lab_ptr;
	struct Floor_3D *Floor_ptr;
	struct Object_3D *Object_ptr;
	struct Wall_3D *Wall_ptr;
	struct Wall_3D *Work_ptr;
	struct Overlay_3D *Overlay_ptr;
	BOOLEAN Result;
	UNSHORT Lab_data_index;
	UNSHORT Nr;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Shade tables are no longer valid */
	Shade_tables_are_valid = FALSE;

	/* Get LAByrinth data index */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	Lab_data_index = Map_ptr->LAB_DATA_NR;
	MEM_Free_pointer(Map_handle);

	/* Load LAByrinth data */
	Lab_data_handle = Load_subfile(LAB_DATA, Lab_data_index);

	/* Get pointers to various parts of the lab data */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Nr_3D_object_groups = Lab_ptr->Nr_object_groups;

	Ptr = (UNBYTE *) Lab_ptr + sizeof(struct Lab_data)
	 + Nr_3D_object_groups * sizeof(struct Object_group_3D);

	Nr_3D_floors = *((UNSHORT *) Ptr);
	Ptr += 2;

	Floor_ptr = (struct Floor_3D *) Ptr;

	Ptr += (Nr_3D_floors * sizeof(struct Floor_3D));

	Nr_3D_objects = *((UNSHORT *) Ptr);
	Ptr += 2;

	Object_ptr = (struct Object_3D *) Ptr;

	Ptr += Nr_3D_objects * sizeof(struct Object_3D);

	Nr_3D_walls = *((UNSHORT *) Ptr);
	Ptr += 2;

	Wall_ptr = (struct Wall_3D *) Ptr;

	/* Check lab data entries */
	if (Nr_3D_floors > MAX_3D_FLOORS)
	{
		Error(ERROR_TOO_MANY_3D_FLOORS);
		return FALSE;
	}

	if (Nr_3D_objects > MAX_3D_OBJECTS)
	{
		Error(ERROR_TOO_MANY_3D_OBJECTS);
		return FALSE;
	}

	if (Nr_3D_floors > MAX_3D_FLOORS)
	{
		Error(ERROR_TOO_MANY_3D_FLOORS);
		return FALSE;
	}

	/* Build list of offsets to wall data */
	Work_ptr = Wall_ptr;
	for (i=0;i<Nr_3D_walls;i++)
	{
		/* Store pointer to wall data */
		Wall_offsets[i] = (UNLONG) Work_ptr - (UNLONG) Lab_ptr;

		/* Skip overlay data */
		Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
		Overlay_ptr += Work_ptr->Nr_overlays;
		Work_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/* Initialise shade table */
	Shade_table_handle = MEM_Do_allocate(65 * 256, (UNLONG) 0,
	 &Shade_table_ftype);

	/* Initialise shade tables for transparent walls */
	Init_shade_tables_for_transparent_walls();

	/* Load background graphics */
	Nr = Lab_ptr->Background_graphics_nr;
	if (Nr)
		Background_gfx_handle = Load_subfile(BACKGROUND_GFX, Nr);
	else
		Background_gfx_handle = NULL;

	/* Load floor, ceiling, object, overlay and wall graphics */
	Result = Load_3D_floors(Lab_ptr, Floor_ptr);
	if (!Result)
		return FALSE;

	Result = Load_3D_textures(Object_ptr, Wall_ptr);
	if (!Result)
		return FALSE;

	Result = Load_3D_overlays(Wall_ptr);
	if (!Result)
		return FALSE;

	/* Put un-animated overlays on un-animated walls */
	Put_unanimated_overlays_on_unanimated_walls();

	/* Calculate shade function */
	Calculate_shade_function(Lab_ptr->Function_A, Lab_ptr->Function_B);

	MEM_Free_pointer(Lab_data_handle);

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_3D_data
 * FUNCTION  : Exit 3D data.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.07.95 14:12
 * LAST      : 09.10.95 12:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_3D_data(void)
{
	UNSHORT i, j;

	/* Free floor and ceiling graphics */
	for (i=0;i<Nr_3D_floor_buffers;i++)
		MEM_Free_memory(Floor_buffer_handles[i]);

	/* Free object and wall graphics */
	for (i=0;i<Nr_3D_texture_buffers;i++)
		MEM_Free_memory(Texture_buffer_handles[i]);

	/* Free overlay graphics */
	for (i=0;i<Nr_3D_walls;i++)
	{
		for (j=0;j<OVERLAYS_PER_WALL;j++)
		{
			MEM_Free_memory(Overlay_handles[i][j]);
		}
	}

	/* Free transparent wall shade tables */
	Exit_shade_tables_for_transparent_walls();

	/* Free background graphics */
	if (Background_gfx_handle)
		MEM_Free_memory(Background_gfx_handle);

	/* Free shade table */
	MEM_Free_memory(Shade_table_handle);

	/* Free LAByrinth data */
	MEM_Free_memory(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_all_shade_tables
 * FUNCTION  : Make all shade tables.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.10.95 12:21
 * LAST      : 09.10.95 18:59
 * INPUTS    : UNSHORT Target_R - Target red value for main shade table.
 *             UNSHORT Target_G - Target green value for main shade table.
 *             UNSHORT Target_B - Target blue value for main shade table.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function can also be called when the palette has
 *              changed.
 *             - This function assumes all shade tables have been initialized.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_all_shade_tables(UNSHORT Target_R, UNSHORT Target_G, UNSHORT Target_B)
{
	/* Are the shade tables still valid ? */
	if (!Shade_tables_are_valid)
	{
		/* No -> Install wait mouse pointer */
		Push_mouse(&(Mouse_pointers[WAIT_MPTR]));

		/* City or wilderness map ? */
		if ((Current_map_type == CITY_MAP) ||
		 (Current_map_type == WILDERNESS_MAP))
		{
			/* Yes -> Calculate shade table for 256 colours */
			Calculate_shade_table
			(
				&Palette,
				Target_R,
				Target_G,
				Target_B,
				256
			);
		}
		else
		{
			/* No -> Calculate shade table for 192 colours */
			Calculate_shade_table
			(
				&Palette,
				Target_R,
				Target_G,
				Target_B,
				192
			);
		}

		/* Calculate shade tables for transparent walls */
		Calculate_shade_tables_for_transparent_walls();

		/* Remove mouse pointer */
		Pop_mouse();

		/* Shade tables are now valid */
		Shade_tables_are_valid = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_3D_floors
 * FUNCTION  : Load and convert floors and ceilings.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:10
 * LAST      : 02.08.95 19:11
 * INPUTS    : struct Floor_3D *Floor_ptr - Pointer to floor data.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_3D_floors(struct Lab_data *Lab_ptr, struct Floor_3D *Floor_ptr)
{
	MEM_HANDLE Handles[MAX_3D_FLOORS];
	BOOLEAN Result;
	UNSHORT Batch[MAX_3D_FLOORS];
	UNSHORT Index;
	UNSHORT i,j;
	UNBYTE *Source, *Target;

	/* Build floor batch */
	for (i=0;i<Nr_3D_floors;i++)
		Batch[i] = Floor_ptr[i].Graphics_nr;

	/* Load batch */
	Result = Load_full_batch(FLOOR_3D, Nr_3D_floors, &Batch[0], &Handles[0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Convert floors and ceilings */
	Index = 0;

	Nr_3D_floor_buffers = 1;
	Floor_buffer_handles[0] = Allocate_floor_buffer();
	Target = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Floor_buffer_handles[0])
	 + 256) & 0xffffff00);

	/* Do all floors */
	for (i=0;i<Nr_3D_floors;i++)
	{
		/* Get pointer to floor graphics */
		Source = MEM_Claim_pointer(Handles[i]);

		#if FALSE
		/* Has this lab-data already been initialized ? */
		if (!(Lab_ptr->Flags & LAB_DATA_INIT))
		{
			/* No -> Find floor colour */
			Floor_ptr[i].Colour = Find_floor_colour(&Palette, 0, 256, Source);
		}
		#endif

		/* Do all frames */
		for (j=0;j<Floor_ptr[i].Nr_frames;j++)
		{
			/* Copy and convert floor frame */
			{
				UNSHORT k,l;
				UNBYTE *Ptr;

				for (k=0;k<64;k++)
				{
					Ptr = Target + (Index * 64) + 63 - k;
					for (l=0;l<64;l++)
					{
						*Ptr = *Source++;
						Ptr += 256;
					}
				}
			}

			/* Initialize texture data */
			Floor_textures[i][j].X			= Index * 64;
			Floor_textures[i][j].p.Handle	= Floor_buffer_handles[Nr_3D_floor_buffers - 1];

			/* Time for a new buffer ? */
			Index++;
			if (Index == 4)
			{
				/* Yes -> Allocate a new one */
				MEM_Free_pointer(Floor_buffer_handles[Nr_3D_floor_buffers - 1]);

				Nr_3D_floor_buffers++;

				Floor_buffer_handles[Nr_3D_floor_buffers - 1] =
				 Allocate_floor_buffer();

				Target = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Floor_buffer_handles[Nr_3D_floor_buffers - 1])
				 + 256) & 0xffffff00);

				Index = 0;
			}
		}

		/* Discard floor */
		MEM_Free_pointer(Handles[i]);
		MEM_Free_memory(Handles[i]);
	}

	/* Ready */
	MEM_Free_pointer(Floor_buffer_handles[Nr_3D_floor_buffers - 1]);

	#if FALSE
	/* Is this lab-data already initialized ? */
	if (!(Lab_ptr->Flags & LAB_DATA_INIT))
	{
		/* No -> It is now */
		Lab_ptr->Flags |= LAB_DATA_INIT;
	}
	#endif

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Allocate_floor_buffer
 * FUNCTION  : Allocate a floor buffer.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.10.94 13:50
 * LAST      : 18.10.94 13:50
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Handle of floor buffer.
 * BUGS      : - This is a recursive function.
 * NOTES     : - This function makes sure the buffer does not cross a 64K
 *              boundary.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Allocate_floor_buffer(void)
{
	MEM_HANDLE Handle1, Handle2;
	UNLONG Ptr;

	Handle1 = MEM_Do_allocate(65 * 256, 0, &Floor_buffer_ftype);

	for(;;)
	{
		Ptr = ((UNLONG) MEM_Claim_pointer(Handle1) + 256) & 0xffffff00;

		if (((Ptr + 64 * 256 - 1) >> 16) != (Ptr >> 16))
		{
			MEM_Free_pointer(Handle1);

			Handle2 = Allocate_floor_buffer();

			MEM_Kill_memory(Handle1);

			Handle1 = Handle2;
		}
		else
			break;
	}

	MEM_Free_pointer(Handle1);

	return Handle1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_3D_textures
 * FUNCTION  : Load and convert object and wall textures.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:10
 * LAST      : 14.10.95 22:24
 * INPUTS    : struct Object_3D *Object_ptr - Pointer to object data.
 *             struct Wall_3D *Wall_ptr - Pointer to wall data.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_3D_textures(struct Object_3D *Object_ptr, struct Wall_3D *Wall_ptr)
{
	struct Wall_3D *Work_ptr;
	struct Overlay_3D *Overlay_ptr;
	BOOLEAN Result;
	UNSHORT i, j;
	UNSHORT Q;

	#ifdef PREP_3D_DEBUG_FLAG
	/* Clear */
	Actual_size = 0;
	Converted_size = 0;
	Nr_textures = 0;
	#endif

	/*** Build list of textures that must be converted ***/
	Nr_3D_textures = 0;
	Nr_3D_texture_buffers = 0;

	/* First the objects */
	for (i=0;i<Nr_3D_objects;i++)
	{
		/* Do each animation frame */
		for (j=0;j<Object_ptr[i].Nr_frames;j++)
		{
			/* Create texture data */
			Textures[Nr_3D_textures].Width	= Object_ptr[i].Graphics_height;
			Textures[Nr_3D_textures].Height	= Object_ptr[i].Graphics_width;
			Textures[Nr_3D_textures].p.Ptr	= &Object_textures[i][j];

			/* Count up */
			Nr_3D_textures++;

			/* Too many textures ? */
			if (Nr_3D_textures > MAX_3D_TEXTURES)
			{
				/* Yes -> Report error */
				Error(ERROR_TOO_MANY_3D_TEXTURES);

				return FALSE;
			}
		}
	}

	/* Then the walls */
	Work_ptr = Wall_ptr;
	for (i=0;i<Nr_3D_walls;i++)
	{
		Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);

		/* Is this wall animated ? */
		Q = 0;
		if (Work_ptr->Nr_frames > 1)
			Q = 1;

		/* Does this wall have overlays ? */
		if (Work_ptr->Nr_overlays)
		{
			/* Yes */
			Q += 2;

			/* Are any of them animated ? */
			for (j=0;j<Work_ptr->Nr_overlays;j++)
			{
				if (Overlay_ptr[j].Nr_frames > 1)
				{
					Q += 2;
					break;
				}
			}
		}

		/* Store wall state */
		Wall_states[i] = Q;

		/* Is an extra frame needed ? */
		if ((Q == 2) || (Q >= 4))
		{
			/* Yes -> Create texture data for one extra texture */
			Textures[Nr_3D_textures].Width	= Work_ptr->Graphics_height;
			Textures[Nr_3D_textures].Height	= Work_ptr->Graphics_width;
			Textures[Nr_3D_textures].p.Ptr	= &Wall_textures[i][8];

			/* Count up */
			Nr_3D_textures++;

			/* Too many textures ? */
			if (Nr_3D_textures > MAX_3D_TEXTURES)
			{
				/* Yes -> Report error */
				Error(ERROR_TOO_MANY_3D_TEXTURES);

				return FALSE;
			}
		}

		/* Are the other frames needed ? */
		if (Q != 2)
		{
			/* Yes -> Do each animation frame */
			for (j=0;j<Work_ptr->Nr_frames;j++)
			{
				/* Create texture data */
				Textures[Nr_3D_textures].Width	= Work_ptr->Graphics_height;
				Textures[Nr_3D_textures].Height	= Work_ptr->Graphics_width;
				Textures[Nr_3D_textures].p.Ptr	= &Wall_textures[i][j];

				/* Count up */
				Nr_3D_textures++;

				/* Too many textures ? */
				if (Nr_3D_textures > MAX_3D_TEXTURES)
				{
					/* Yes -> Report error */
					Error(ERROR_TOO_MANY_3D_TEXTURES);

					return FALSE;
				}
			}
		}

		/* Skip overlay data */
		Overlay_ptr += Work_ptr->Nr_overlays;
		Work_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/*** Sort textures on height, then width ***/
	Shellsort
	(
		Swap_3D_textures,
		Compare_3D_textures,
		Nr_3D_textures,
		(UNBYTE *) (UNLONG) 0
	);

	/*** Get pointers to textures ***/
	{
		struct Texture_3D **P;

		for (i=0;i<Nr_3D_textures;i++)
		{
			P = Textures[i].p.Ptr;
			*P = &(Textures[i]);
		}
	}

	/*** Build texture buffers ***/
	{
		UNSHORT Index1, Index2;
		UNSHORT Height1, Height2;

		Index1 = 0;
		Index2 = 0;

		Height1 = Textures[0].Height;
		if (Height1 < 8)
			Height1 = 8;
		Height2 = Height1;

		/* Inspect all textures */
		for(;;)
		{
			/* Inspect the next texture */
			Index2++;

			/* Is the height difference greater than the first height OR
			 have all the textures been inspected ? */
			if ((Index2 >= Nr_3D_textures) ||
			 ((Textures[Index2].Height - Height1) > Height1))
			{
				/* Yes -> Determine the positions for all textures until now */
				Position_3D_textures(Index1, Index2 - 1, Height2);

				/* Exit if all textures have been inspected */
				if (Index2 >= Nr_3D_textures)
					break;

				/* Continue */
				Height1 = Height2;
				Index1 = Index2;
			}

			/* Get new height */
			Height2 = Textures[Index2].Height;
		}
	}

	/*** Load and convert the object textures ***/
	{
		MEM_HANDLE Handles[MAX_3D_OBJECTS];
		UNLONG Size;
		UNSHORT Batch[MAX_3D_OBJECTS];
		UNBYTE *Ptr;

		/* Build object batch */
		for (i=0;i<Nr_3D_objects;i++)
			Batch[i] = Object_ptr[i].Graphics_nr;

		/* Load object graphics */
		Result = Load_partial_batch(OBJECT_3D, Nr_3D_objects, &Batch[0],
		 &Handles[0]);
		if (!Result)
		{
			Error(ERROR_FILE_LOAD);
			return FALSE;
		}

		/* Convert and copy object graphics */
		for (i=0;i<Nr_3D_objects;i++)
		{
			Size = Object_ptr[i].Graphics_width * Object_ptr[i].Graphics_height;
			Ptr = MEM_Claim_pointer(Handles[i]);

			/* Copy each animation frame */
			for (j=0;j<Object_ptr[i].Nr_frames;j++)
			{
				/* Copy animation frame */
				Copy_texture2(Object_textures[i][j], Ptr);
				Ptr += Size;
			}

			/* Destroy object graphics */
			MEM_Free_pointer(Handles[i]);
			MEM_Free_memory(Handles[i]);
		}
	}

	/*** Load and convert the wall textures ***/
	{
		MEM_HANDLE Handles[MAX_3D_WALLS];
		UNLONG Size;
		UNSHORT Batch[MAX_3D_WALLS];
		UNBYTE *Ptr;

		/* Build wall batch */
		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_3D_walls;i++)
		{
			Batch[i] = Work_ptr->Graphics_nr;

			/* Skip overlay data */
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Overlay_ptr += Work_ptr->Nr_overlays;
			Work_ptr = (struct Wall_3D *) Overlay_ptr;
		}

		/* Load wall graphics */
		Result = Load_partial_batch(WALL_3D, Nr_3D_walls, &Batch[0],
		 &Handles[0]);
		if (!Result)
		{
			Error(ERROR_FILE_LOAD);
			return FALSE;
		}

		/* Convert and copy wall graphics */
		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_3D_walls;i++)
		{
			Size = Work_ptr->Graphics_width * Work_ptr->Graphics_height;
			Ptr = MEM_Claim_pointer(Handles[i]);
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Q = Wall_states[i];

			/* Is an extra frame needed ? */
			if ((Q == 2) || (Q >= 4))
			{
				/* Yes -> Copy one extra frame */
				Copy_texture(Wall_textures[i][8], Ptr);
			}

			/* Are the other frames needed ? */
			if (Q != 2)
			{
				/* Yes -> Copy each animation frame */
				for (j=0;j<Work_ptr->Nr_frames;j++)
				{
					Copy_texture(Wall_textures[i][j], Ptr);
					Ptr += Size;
				}
			}

			/* Destroy wall graphics */
			MEM_Free_pointer(Handles[i]);
			MEM_Free_memory(Handles[i]);

			/* Skip overlay data */
			Overlay_ptr += Work_ptr->Nr_overlays;
			Work_ptr = (struct Wall_3D *) Overlay_ptr;
		}
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Copy_texture
 * FUNCTION  : Copy and convert a texture.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.94 17:55
 * LAST      : 12.08.94 17:55
 *	INPUTS    : struct Texture_3D *Texture - Pointer to texture data.
 *             UNBYTE *Source - Pointer to unconverted graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Copy_texture(struct Texture_3D *Texture, UNBYTE *Source)
{
	UNSHORT i, j;
	UNBYTE *Target, *Ptr;

	#ifdef PREP_3D_DEBUG_FLAG
	Actual_size += (Texture->Width * Texture->Height);
	#endif

	/* Get pointer to texture buffer */
	Target = MEM_Claim_pointer(Texture->p.Handle) + Texture->X;

	/* Copy and convert texture */
	for (i=0;i<Texture->Height;i++)
	{
		Ptr = Target;
		for (j=0;j<Texture->Width;j++)
		{
			*Ptr++ = *Source++;
		}
		Target += 256;
	}

	/* Ready */
	MEM_Free_pointer(Texture->p.Handle);
}

void
Copy_texture2(struct Texture_3D *Texture, UNBYTE *Source)
{
	UNSHORT i, j;
	UNBYTE *Target, *Ptr;

	#ifdef PREP_3D_DEBUG_FLAG
	Actual_size += (Texture->Width * Texture->Height);
	#endif

	/* Get pointer to texture buffer */
	Target = MEM_Claim_pointer(Texture->p.Handle) + Texture->X;

	/* Copy and convert texture */
	for (i=0;i<Texture->Height;i++)
	{
		Ptr = Target;
		for (j=0;j<Texture->Width;j++)
		{
			*Ptr++ = *Source;
			Source += Texture->Height;
		}
		Source -= (Texture->Height * Texture->Width);
		Source++;
		Target += 256;
	}

	/* Ready */
	MEM_Free_pointer(Texture->p.Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Position_3D_textures
 * FUNCTION  : Determine the position of 3D textures.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 17:17
 * LAST      : 15.10.95 19:28
 *	INPUTS    : SISHORT Start - Index of first texture to be positioned.
 *             SISHORT End - Index of last texture to be positioned.
 *             UNSHORT Height - Height of the texture buffers.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Position_3D_textures(SISHORT Start, SISHORT End, UNSHORT Height)
{
	MEM_HANDLE Handle;
	UNSHORT X;

	/* Sort textures on width */
	Shellsort
	(
		Swap_3D_textures,
		Compare_3D_textures2,
		End - Start + 1,
		(UNBYTE *) ((UNLONG) Start)
	);

	/* Get pointers to textures */
	{
		struct Texture_3D **P;
		UNSHORT i;

		for (i=Start;i<=End;i++)
		{
			P = Textures[i].p.Ptr;
			*P = &(Textures[i]);
		}
  	}

	/* Position textures */
	for(;;)
	{
		/* Make a new texture buffer */
		Handle = MEM_Do_allocate
		(
			256 * (UNLONG) Height,
			0,
			&Texture_buffer_ftype
		);

		/* Success ? */
		if (!Handle)
		{
			/* No -> Error */
			Error(ERROR_OUT_OF_MEMORY);
			return;
		}

		/* Clear texture buffer */
		MEM_Clear_memory(Handle);

		#ifdef PREP_3D_DEBUG_FLAG
		Converted_size += Height * 256;
		Nr_textures++;
		#endif

		/* Increase */
		Texture_buffer_handles[Nr_3D_texture_buffers] = Handle;
		Nr_3D_texture_buffers++;

		/* Position as many large textures as possible */
		X = 0;
		while ((X + Textures[End].Width) <= 256)
		{
			/* Insert texture data */
			Textures[End].X = X;
			Textures[End].p.Handle = Handle;

			/* Increase X-coordinate in texture buffer */
			X += Textures[End].Width;

			/* Next texture */
			End--;
			if (End < Start)
				break;
		}

		if (Start <= End)
		{
			/* Position as many small textures as possible */
			while ((X + Textures[Start].Width) <= 256)
			{
				/* Insert texture data */
				Textures[Start].X = X;
				Textures[Start].p.Handle = Handle;

				/* Increase X-coordinate in texture buffer */
				X += Textures[Start].Width;

				/* Next texture */
				Start++;
				if (Start > End)
					break;
			}
		}

		/* Exit if all textures have been copied */
		if (Start > End)
			return;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_3D_textures
 * FUNCTION  : Swap two 3D textures (for sorting).
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 15:17
 * LAST      : 11.08.94 15:17
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Texture_3D T;
	UNLONG Start;

	/* Get offset */
	Start = (UNLONG) Data;

	/* Add to indices */
	A += Start;
	B += Start;

	/* Swap texture data */
	memcpy (&T, Textures+A, sizeof(struct Texture_3D));
	memcpy (Textures+A, Textures+B, sizeof(struct Texture_3D));
	memcpy (Textures+B, &T, sizeof(struct Texture_3D));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_3D_textures
 * FUNCTION  : Compare two 3D textures (for sorting).
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 15:17
 * LAST      : 11.08.94 15:17
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 *	RESULT    : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_3D_textures(UNLONG A, UNLONG B, UNBYTE *Data)
{
	UNLONG Start;

	/* Get offset */
	Start = (UNLONG) Data;

	/* Add to indices */
	A += Start;
	B += Start;

	/* Compare texture data */
/*	if (Textures[A].Height == Textures[B].Height)
		return(Textures[A].Width > Textures[B].Width);
	else */
		return(Textures[A].Height > Textures[B].Height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_3D_textures2
 * FUNCTION  : Compare two 3D textures (for sorting).
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 17:37
 * LAST      : 14.12.94 17:37
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 *	RESULT    : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_3D_textures2(UNLONG A, UNLONG B, UNBYTE *Data)
{
	UNLONG Start;

	/* Get offset */
	Start = (UNLONG) Data;

	/* Add to indices */
	A += Start;
	B += Start;

	/* Compare texture data */
	return(Textures[A].Width > Textures[B].Width);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_3D_overlays
 * FUNCTION  : Load and convert overlays.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.94 15:30
 * LAST      : 02.08.95 19:13
 * INPUTS    : struct Wall_3D *Wall_ptr - Pointer to wall data.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_3D_overlays(struct Wall_3D *Wall_ptr)
{
	struct Overlay_3D *Overlay_ptr;
	BOOLEAN Result;
	UNSHORT Batch[MAX_3D_WALLS][OVERLAYS_PER_WALL];
	UNSHORT i, j;

	/* Build overlay batch */
	for (i=0;i<Nr_3D_walls;i++)
	{
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);

		/* Do each overlay */
		for (j=0;j<Wall_ptr->Nr_overlays;j++)
		{
			Batch[i][j] = Overlay_ptr[j].Graphics_nr;
		}

		/* Clear the rest of the batch */
		for (;j<OVERLAYS_PER_WALL;j++)
		{
			Batch[i][j] = 0;
		}

		/* Skip overlay data */
		Overlay_ptr += Wall_ptr->Nr_overlays;
		Wall_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/* Load overlay graphics */
	Result = Load_partial_batch
	(
		OVERLAY_3D,
		Nr_3D_walls * OVERLAYS_PER_WALL,
		&Batch[0][0],
		&Overlay_handles[0][0]
	);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_shade_tables_for_transparent_walls
 * FUNCTION  : Initialise shade tables for transparent walls.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.10.95 23:20
 * LAST      : 08.10.95 23:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes the wall offsets table has been filled
 *              with meaningful values.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_shade_tables_for_transparent_walls(void)
{
	struct Wall_3D *Wall_ptr;
	MEM_HANDLE Handle;
	UNSHORT i;
	UNBYTE *Lab_ptr;

	/* Check each wall */
	Lab_ptr = MEM_Claim_pointer(Lab_data_handle);
	for (i=0;i<Nr_3D_walls;i++)
	{
		/* Get pointer to wall data */
		Wall_ptr = (struct Wall_3D *)(Lab_ptr + Wall_offsets[i]);

		/* Is this a transparent wall ? */
		if (Wall_ptr->Flags & TRANSPARENT_WALL)
		{
			/* Yes -> Allocate space for shade table */
			Handle = MEM_Do_allocate
			(
				2 * 256,
				(UNLONG) i,
				&Wall_shade_table_ftype
			);

			/* Success ? */
			if (!Handle)
			{
				/* No -> Report error */
				Error(ERROR_OUT_OF_MEMORY);
				Exit_program();
				break;
			}

			/* Store handle */
			Wall_shade_tables[i] = Handle;
		}
		else
		{
			/* Not transparent -> Clear handle */
			Wall_shade_tables[i] = NULL;
		}
	}
	MEM_Free_pointer(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_shade_tables_for_transparent_walls
 * FUNCTION  : Exit shade tables for transparent walls.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.10.95 23:22
 * LAST      : 08.10.95 23:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_shade_tables_for_transparent_walls(void)
{
	UNSHORT i;

	/* Check all walls */
	for (i=0;i<Nr_3D_walls;i++)
	{
		/* Does a shade table exist ? */
		if (Wall_shade_tables[i])
		{
			/* Yes -> Free memory */
			MEM_Free_memory(Wall_shade_tables[i]);

			/* Mark table as non-existent */
			Wall_shade_tables[i] = NULL;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_shade_tables_for_transparent_walls
 * FUNCTION  : Calculate shade tables for transparent walls.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.02.95 13:35
 * LAST      : 08.10.95 23:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes the wall offsets table has been filled
 *              with meaningful values.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_shade_tables_for_transparent_walls(void)
{
	struct Wall_3D *Wall_ptr;
	UNSHORT Target_colour_index;
	UNSHORT Target_R, Target_G, Target_B;
	UNSHORT R, G, B;
	UNSHORT i, j;
	UNBYTE *Lab_ptr;
	UNBYTE *Ptr;

	/* Check each wall */
	Lab_ptr = MEM_Claim_pointer(Lab_data_handle);
	for (i=0;i<Nr_3D_walls;i++)
	{
		/* Get pointer to wall data */
		Wall_ptr = (struct Wall_3D *)(Lab_ptr + Wall_offsets[i]);

		/* Is this a transparent wall ? */
		if (Wall_ptr->Flags & TRANSPARENT_WALL)
		{
			/* Yes -> Get target colour */
			Target_colour_index = Wall_ptr->Transparent_colour;

			Target_R = Palette.color[Target_colour_index].red;
			Target_G = Palette.color[Target_colour_index].green;
			Target_B = Palette.color[Target_colour_index].blue;

			/* Make shading table */
			Ptr = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Wall_shade_tables[i]) +
			 256) & 0xffffff00);

			for (j=0;j<256;j++)
			{
				/* Get the current colour */
				R = Palette.color[j].red;
				G = Palette.color[j].green;
				B = Palette.color[j].blue;

				/* Interpolate towards the target colour */
				R = ((R - Target_R) * 32) / 64 + Target_R;
				G = ((G - Target_G) * 32) / 64 + Target_G;
				B = ((B - Target_B) * 32) / 64 + Target_B;

				/* Find the closest matching colour in the palette */
				*Ptr++ = Find_closest_colour(R, G, B);
			}

			MEM_Free_pointer(Wall_shade_tables[i]);
		}
	}
	MEM_Free_pointer(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_unanimated_overlays_on_unanimated_walls
 * FUNCTION  : Put un-animated overlays on un-animated walls.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.02.95 13:41
 * LAST      : 29.07.95 16:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function must be called after the textures have been
 *              loaded.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_unanimated_overlays_on_unanimated_walls(void)
{
	struct Texture_3D *Texture;
	struct Wall_3D *Wall_ptr;
	struct Overlay_3D *Overlay_ptr;

	UNSHORT i, j, k;

	UNBYTE *Wall_graphics;
	UNBYTE *Overlay_graphics;

	/* Get pointer to wall data */
	Wall_ptr = (struct Wall_3D *) (MEM_Claim_pointer(Lab_data_handle) +
	 Wall_offsets[0]);

	/* Check all walls */
	for (i=0;i<Nr_3D_walls;i++)
	{
		/* Get pointer to overlay data for this wall */
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);

		/* Act depending on wall state */
		switch (Wall_states[i])
		{
			/* Un-animated overlays on un-animated wall */
			case 2:
			{
				/* Put all the overlays on the wall */
				Texture = Wall_textures[i][8];
				Wall_graphics = MEM_Claim_pointer(Texture->p.Handle) +
				 Texture->X;

				for (j=0;j<Wall_ptr->Nr_overlays;j++)
				{
					Overlay_graphics = MEM_Claim_pointer(Overlay_handles[i][j]);

					Put_overlay_on_wall(&(Overlay_ptr[j]), Wall_graphics,
					 Overlay_graphics);

					MEM_Free_pointer(Overlay_handles[i][j]);
				}

				MEM_Free_pointer(Texture->p.Handle);

				break;
			}
			/* Un-animated overlays on animated wall */
			case 3:
			{
				/* Yes -> Put all the overlays on the walls */
				for (k=0;k<Wall_ptr->Nr_frames;k++)
				{
					Texture = Wall_textures[i][k];
					Wall_graphics = MEM_Claim_pointer(Texture->p.Handle) +
					 Texture->X;

					for (j=0;j<Wall_ptr->Nr_overlays;j++)
					{
						Overlay_graphics = MEM_Claim_pointer(Overlay_handles[i][j]);

						Put_overlay_on_wall(&(Overlay_ptr[j]), Wall_graphics,
						 Overlay_graphics);

						MEM_Free_pointer(Overlay_handles[i][j]);
					}

					MEM_Free_pointer(Texture->p.Handle);
				}
				break;
			}
		}

		/* Skip overlay data */
		Overlay_ptr += Wall_ptr->Nr_overlays;
		Wall_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	MEM_Free_pointer(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_overlays_on_walls
 * FUNCTION  : Put possibly animated overlays on possibly animated walls.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.07.95 15:30
 * LAST      : 29.07.95 16:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_overlays_on_walls(void)
{
	struct Texture_3D *Source_texture;
	struct Texture_3D *Target_texture;
	struct Wall_3D *Wall_ptr;
	struct Overlay_3D *Overlay_ptr;

	UNSHORT Frame;
	UNSHORT i, j, k;

	UNBYTE *Source_wall_graphics;
	UNBYTE *Target_wall_graphics;
	UNBYTE *Overlay_graphics;

	/* Get pointer to wall data */
	Wall_ptr = (struct Wall_3D *) (MEM_Claim_pointer(Lab_data_handle) +
	 Wall_offsets[0]);

	/* Check all walls */
	for (i=0;i<Nr_3D_walls;i++)
	{
		/* Get pointer to overlay data for this wall */
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);

		if (Wall_states[i] >= 4)
		{
			/* Get source and target wall graphics */
			Frame = Get_animation_frame(Wall_ptr->Flags, Wall_ptr->Nr_frames, i);

			Source_texture = Wall_textures[i][Frame];
			Source_wall_graphics = MEM_Claim_pointer(Source_texture->p.Handle)
			 + Source_texture->X;

			Target_texture = Wall_textures[i][8];
			Target_wall_graphics = MEM_Claim_pointer(Target_texture->p.Handle)
			 + Target_texture->X;

			/* Is this wall animated ? */
			if (Wall_ptr->Nr_frames > 1)
			{
				/* Yes -> Copy a new frame to the work frame */
				for (j=0;j<Wall_ptr->Graphics_width;j++)
				{
					for (k=0;k<Wall_ptr->Graphics_height;k++)
					{
						*Target_wall_graphics++ = *Source_wall_graphics++;
					}
					Source_wall_graphics += 256 - Wall_ptr->Graphics_height;
					Target_wall_graphics += 256 - Wall_ptr->Graphics_height;
				}
			}
			else
			{
				/* No -> Remove all current overlays */
				for (j=0;j<Wall_ptr->Nr_overlays;j++)
				{
					Remove_overlay_from_wall(&(Overlay_ptr[j]),
					 Source_wall_graphics, Target_wall_graphics);
				}
			}

			MEM_Free_pointer(Source_texture->p.Handle);

			/* Put all the overlays on the walls */
			for (j=0;j<Wall_ptr->Nr_overlays;j++)
			{
				Frame = Get_animation_frame(0, Overlay_ptr[j].Nr_frames,
				 (i * 8) + j);

				Overlay_graphics = MEM_Claim_pointer(Overlay_handles[i][j])
				 + Frame * (Overlay_ptr[j].Graphics_width
				 * Overlay_ptr[j].Graphics_height);

				Put_overlay_on_wall(&(Overlay_ptr[j]), Target_wall_graphics,
				 Overlay_graphics);

				MEM_Free_pointer(Overlay_handles[i][j]);
			}

			MEM_Free_pointer(Target_texture->p.Handle);
		}

		/* Skip overlay data */
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
		Overlay_ptr += Wall_ptr->Nr_overlays;
		Wall_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	MEM_Free_pointer(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_object_graphics
 * FUNCTION  : Return pointer to object graphics.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 16:27
 * LAST      : 20.10.95 18:19
 * INPUTS    : UNSHORT NPC_nr - NPC index (0xFFFF for normal objects).
 *             UNSHORT Object_nr - Object number (1...).
 *             UNSHORT Hash_nr - Number used for animation hashing.
 * RESULT    : UNBYTE * : Pointer to object graphics.
 * BUGS      : No known.
 * NOTES     : - Due to a bug in 3DM, the NPC index is one too high.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_object_graphics(UNSHORT NPC_nr, UNSHORT Object_nr, UNSHORT Hash_nr)
{
	struct Object_3D *Object;
	struct Texture_3D *Texture;
	UNSHORT Frame;

	/* Exit if object number is illegal */
	if (Object_nr > Nr_3D_objects)
		return 0;

	/* Get object data */
	Object = (I3DM.Objects) + Object_nr - 1;

	/* Is an NPC ? */
	if (NPC_nr == 0xFFFF)
	{
		/* No -> Get animation frame */
		Frame = Get_animation_frame
		(
			Object->Flags,
			Object->Nr_frames,
			Hash_nr
		);

		/* Get texture data */
		Texture = Object_textures[Object_nr-1][Frame];

		/* Return pointer to texture */
		return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
	}
	else
	{
		/* Yes */
		/* (correction for 3DM bug) */
		NPC_nr -= 1;

		/* Animate always ? */
		if (VNPCs[NPC_nr].Flags & NPC_ANIMATE_ALWAYS)
		{
			/* Yes -> Set next animation frame */
			Frame = Get_animation_frame
			(
				Object->Flags,
				Object->Nr_frames,
				Hash_nr
			);
		}
		else
		{
			/* No -> Did this NPC move ? */
			if (VNPCs[NPC_nr].Flags & NPC_MOVED)
			{
				/* Yes -> Set next animation frame */
				Frame = Get_animation_frame
				(
					Object->Flags,
					Object->Nr_frames,
					Hash_nr
				);
			}
			else
			{
				/* No -> Use the middle frame */
				Frame = Object->Nr_frames / 2;

				/* Legal frame index ? */
				if (Frame >= Object->Nr_frames)
				{
					/* No -> Use frame 0 */
					Frame = 0;
				}
			}
		}

		/* Store animation frame index */
		VNPCs[NPC_nr].Frame = Frame;

		/* Get texture data */
		Texture = Object_textures[Object_nr-1][Frame];

		/* Return pointer to texture */
		return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_floor_graphics
 * FUNCTION  : Return pointer to floor graphics.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 16:48
 * LAST      : 18.08.94 16:48
 * INPUTS    : UNSHORT Floor_nr - Floor number (1...).
 *             UNSHORT Hash_nr - Number used for animation hashing.
 *             UNSHORT Resolution - 0 = normal, 1 = defocussed.
 * RESULT    : UNBYTE * : Pointer to floor graphics.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_floor_graphics(UNSHORT Floor_nr, UNSHORT Hash_nr, UNSHORT Resolution)
{
	struct Floor_3D *Floor;
	struct Texture_3D *Texture;
	UNSHORT Frame;

	if (Floor_nr > Nr_3D_floors)
		return 0;

	/* Get floor data */
	Floor = (I3DM.Floors) + Floor_nr - 1;

	/* Get animation frame */
	Frame = Get_animation_frame(Floor->Flags, Floor->Nr_frames, Hash_nr);

	/* Get texture data */
	Texture = &Floor_textures[Floor_nr-1][Frame];

	/* Return pointer to texture */
	return((UNBYTE *) ((((UNLONG) MEM_Get_pointer(Texture->p.Handle) +
	 256) & 0xffffff00) + Texture->X));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_wall_graphics
 * FUNCTION  : Return pointer to wall graphics.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 16:56
 * LAST      : 18.08.94 16:56
 * INPUTS    : UNSHORT Wall_nr - Wall number (1...).
 *             UNSHORT Hash_nr - Number used for animation hashing.
 * RESULT    : UNBYTE * : Pointer to wall graphics.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_wall_graphics(UNSHORT Wall_nr, UNSHORT Hash_nr)
{
	struct Wall_3D *Wall;
	struct Texture_3D *Texture;
	UNSHORT Frame, Q;

	if (Wall_nr > Nr_3D_walls)
		return 0;

	/* Get wall data */
	Wall = I3DM.Wall_pointers[Wall_nr-1];

	/* Select frame depending on wall state */
	Q = Wall_states[Wall_nr-1];
	if ((Q == 2) || (Q >= 4))
	{
		Frame = 8;
	}
	else
	{
		/* Get animation frame */
		Frame = Get_animation_frame(Wall->Flags, Wall->Nr_frames, Hash_nr);
	}

	/* Get texture data */
	Texture = Wall_textures[Wall_nr-1][Frame];

	/* Return pointer to texture */
	return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_wall_shade_table
 * FUNCTION  : Return pointer to wall shade table.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 17:47
 * LAST      : 18.08.94 17:47
 * INPUTS    : UNSHORT Wall_nr - Wall number (1...).
 * RESULT    : UNBYTE * : Pointer to shade table / 0 if inappropriate.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_wall_shade_table(UNSHORT Wall_nr)
{
	/* Return pointer to shading table */
	return((UNBYTE *) (((UNLONG) MEM_Get_pointer(Wall_shade_tables[Wall_nr-1]) +
	 256) & 0xffffff00));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_overlay_on_wall
 * FUNCTION  : Put an overlay on a wall.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 18:00
 * LAST      : 18.08.94 18:00
 * INPUTS    : struct Overlay_3D *Overlay - Pointer to overlay data.
 *             UNBYTE *Wall_graphics - Pointer to wall graphics.
 *             UNBYTE *Overlay_graphics - Pointer to overlay graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This routine assumes that the overlay will fit on the wall.
 *              No clipping or boundary checks are performed.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_overlay_on_wall(struct Overlay_3D *Overlay,
 UNBYTE *Wall_graphics, UNBYTE *Overlay_graphics)
{
	UNSHORT i, j;
	UNBYTE c;
	UNBYTE *Ptr;

	/* Calculate destination address */
	Wall_graphics += Overlay->Y + (Overlay->X * 256);

	/* Block or mask ? */
	if (Overlay->Mask_flag)
	{
		/* Masked -> Put overlay on wall */
		for (i=0;i<Overlay->Graphics_width;i++)
		{
			Ptr = Wall_graphics;
			for (j=0;j<Overlay->Graphics_height;j++)
			{
				c = *Overlay_graphics++;
				if (c)
					*Ptr = c;
				Ptr++;
			}
			Wall_graphics += 256;
		}
	}
	else
	{
	 	/* Blocked -> Put overlay on wall */
		for (i=0;i<Overlay->Graphics_width;i++)
		{
			Ptr = Wall_graphics;
			for (j=0;j<Overlay->Graphics_height;j++)
			{
				*Ptr++ = *Overlay_graphics++;
			}
			Wall_graphics += 256;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_overlay_from_wall
 * FUNCTION  : Remove an overlay from a wall.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.09.94 11:57
 * LAST      : 02.09.94 11:57
 * INPUTS    : struct Overlay_3D *Overlay - Pointer to overlay data.
 *             UNBYTE *Source_wall_graphics - Pointer to source wall graphics.
 *             UNBYTE *Target_wall_graphics - Pointer to target wall graphics.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This routine assumes that the overlay will fit on the wall.
 *              No clipping or boundary checks are performed.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_overlay_from_wall(struct Overlay_3D *Overlay,
 UNBYTE *Source_wall_graphics, UNBYTE *Target_wall_graphics)
{
	UNSHORT i, j;

	/* Calculate destination address */
	Source_wall_graphics += Overlay->Y + (Overlay->X * 256);
	Target_wall_graphics += Overlay->Y + (Overlay->X * 256);

	/* Remove overlay from wall */
	for (i=0;i<Overlay->Graphics_width;i++)
	{
		for (j=0;j<Overlay->Graphics_height;j++)
		{
			*Target_wall_graphics++ = *Source_wall_graphics++;
		}
		Source_wall_graphics += 256 - Overlay->Graphics_height;
		Target_wall_graphics += 256 - Overlay->Graphics_height;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Colour_cycle_forward_3D
 * FUNCTION  : Cycle colours forward in the 3D shading table.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.94 11:12
 * LAST      : 29.07.95 16:21
 * INPUTS    : UNSHORT Start - Number of first colour (0...255).
 *             UNSHORT Size - Number of colours to cycle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Colour_cycle_forward_3D(UNSHORT Start, UNSHORT Size)
{
	UNSHORT i, j;
	UNBYTE Last;
	UNBYTE *Ptr;

	/* Is this a 3D map ? */
	if (_3D_map)
	{
		/* Yes -> Get pointer to shade table */
		Ptr = Get_shade_table_address() + Start;

		for (i=0;i<MAX_3D_SHADE_LEVELS;i++)
		{
			/* Save the last colour in the range */
			Last = Ptr[Size-1];

			/* Copy all colours forward */
			for (j=Size-1;j>0;j--)
			{
				Ptr[j] = Ptr[j-1];
			}

			/* Insert last colour at the start */
			Ptr[0] = Last;

			/* Next shade table */
			Ptr += 256;
		}
	}
	else
	{
		/* No -> Error */
		Error(ERROR_3D_COLOUR_CYCLING_IN_2D_MAP);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_shade_table
 * FUNCTION  : Calculate a shade table for the 3D map.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 12:33
 * LAST      : 29.07.95 16:21
 * INPUTS    : struct BBPALETTE *Pal - Pointer to colour palette.
 *             UNSHORT Target_R - Target red value.
 *             UNSHORT Target_G - Target green value.
 *             UNSHORT Target_B - Target blue value.
 *             UNSHORT Number - Number of colours that must be shaded.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes that an appropriately sized buffer has
 *              already been allocated.
 *             - This function does not claim the shade table. Instead it
 *              assumes the table won't be moved by Find_closest_colour
 *              (which is correct).
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_shade_table(struct BBPALETTE *Pal, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Number)
{
	SISHORT R, G, B;
	UNSHORT i, j;
	UNBYTE *Ptr;

	Ptr = Get_shade_table_address();

	/* Write level 0 (no shading) */
	for (j=0;j<256;j++)
		*Ptr++ = j;

	/* Write level 1 to second but last */
	for (i=1;i<MAX_3D_SHADE_LEVELS;i++)
	{
		/* First the colours that must be shaded */
		for (j=0;j<Number;j++)
		{
			/* Get the current colour */
			R = Pal->color[j].red;
			G = Pal->color[j].green;
			B = Pal->color[j].blue;

			/* Interpolate towards the target colour */
			R = ((R - Target_R) * (64 - i)) / 64 + Target_R;
			G = ((G - Target_G) * (64 - i)) / 64 + Target_G;
			B = ((B - Target_B) * (64 - i)) / 64 + Target_B;

			/* Find the closest matching colour in the palette */
			*Ptr++ = Find_closest_colour(R, G, B);
		}

		/* The last colours remain the same */
		for (;j<256;j++)
			*Ptr++ = j;
	}

	/* Write last level (black) */
	for (j=0;j<Number;j++)
		*Ptr++ = 0;

	/* The last colours remain the same */
	for (;j<256;j++)
		*Ptr++ = j;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Relocate_on_256_byte_boundary
 * FUNCTION  : Relocate a memory block on a 256 byte boundary.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.11.94 11:18
 * LAST      : 29.07.95 16:26
 * INPUTS    : MEM_HANDLE Handle - Handle of memory block.
 *             UNBYTE *Source - Pointer to source.
 *             UNBYTE *Target - Pointer to target.
 *             UNLONG Size - Size of memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes that the memory block has 256 bytes of
 *              "padding" around it.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Relocate_on_256_byte_boundary(MEM_HANDLE Handle, UNBYTE *Source,
 UNBYTE *Target, UNLONG Size)
{
	UNBYTE *Source_ptr;
	UNBYTE *Target_ptr;

	/* Get pointers to source and target, on 256 byte boundaries */
	Source_ptr = (UNBYTE *) (((UNLONG) Source + 256) & 0xffffff00);
	Target_ptr = (UNBYTE *) (((UNLONG) Target + 256) & 0xffffff00);

	/* Copy the memory block, minus 256 bytes */
	BASEMEM_CopyMem(Source_ptr, Target_ptr, Size - 256);
}

#if FALSE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Texture_vanish_test
 * FUNCTION  : Test if a texture may vanish when shaded away.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.12.94 16:41
 * LAST      : 09.12.94 16:42
 * INPUTS    : UNSHORT Width - Width of texture.
 *             UNSHORT Height - Height of texture.
 *             UNBYTE *Ptr - Pointer to texture.
 * RESULT    : BOOLEAN : TRUE (texture may NOT vanish) or FALSE (texture may
 *              vanish).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Texture_vanish_test(UNSHORT Width, UNSHORT Height, UNBYTE *Ptr)
{
	BOOLEAN Result = FALSE;
	UNLONG i;

	/* Are all colours shaded anyway ? */
	if ((Map_type == CITY_MAP) || (Map_type == WILDERNESS_MAP))
	{
		/* Yes -> Don't bother */
		return FALSE;
	}

	/* Check entire texture */
	for (i=0;i<Width * Height;i++)
	{
		/* Does this pixel have a non-shaded colour ? */
		if (*Ptr++ >= 192)
		{
			/* Yes -> Don't vanish */
			Result = TRUE;
			break;
		}
	}

	return Result;
}
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_lab_data
 * FUNCTION  : Convert 3D LAByrinth data (file converter function).
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 13:09
 * LAST      : 29.08.95 13:38
 * INPUTS    : UNBYTE *Ptr - Pointer to data.
 *             MEM_HANDLE Handle - Handle of data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Convert_lab_data(UNBYTE *Ptr, MEM_HANDLE Handle)
{
	struct Lab_data *Lab_ptr;
	struct Object_group_3D *Object_group_ptr;
	SISHORT f;
	UNSHORT i, j;

	Lab_ptr = (struct Lab_data *) Ptr;

	/* Calculate conversion factor */
	f = 9 - Lab_ptr->Square_size_cm_log;

	/* Is conversion necessary ? */
	if (f)
	{
		/* Yes -> Convert all object groups */
		Object_group_ptr = (struct Object_group_3D *) (Lab_ptr + 1);

		/* Scale up or down ? */
		if (f > 0)
		{
			/* Up -> Calculate scaling factor */
			f = (1 << f);

			/* Do all object groups */
			for (i=0;i<Lab_ptr->Nr_object_groups;i++)
			{
				/* Do each object in this group */
				for (j=0;j<OBJECTS_PER_GROUP;j++)
				{
					/* Scale coordinates */
					Object_group_ptr->Group[j].X /= f;
					Object_group_ptr->Group[j].Y /= f;
					Object_group_ptr->Group[j].Z /= f;
				}
				/* Next object group */
				Object_group_ptr++;
			}
		}
		else
		{
			/* Down -> Calculate scaling factor */
			f = (1 << (0 - f));

			/* Do all object groups */
			for (i=0;i<Lab_ptr->Nr_object_groups;i++)
			{
				/* Do each object in this group */
				for (j=0;j<OBJECTS_PER_GROUP;j++)
				{
					/* Scale coordinates */
					Object_group_ptr->Group[j].X *= f;
					Object_group_ptr->Group[j].Y *= f;
					Object_group_ptr->Group[j].Z *= f;
				}
				/* Next object group */
				Object_group_ptr++;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_shade_function
 * FUNCTION  : Calculate a shade function for the 3D map.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 13:54
 * LAST      : 07.09.94 13:54
 * INPUTS    : UNSHORT A_value - A part of shade function.
 *             UNSHORT B_value - B part of shade function.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_shade_function(UNSHORT A_value, UNSHORT B_value)
{
	SISHORT Value;
	UNSHORT i, Counter;
	SIBYTE *Ptr;

	/* Get pointer to shade function */
	Ptr = &(I3DM.Shade_function[0]);

	/* Part A : No shading */
	for (i=0;i<A_value;i++)
		*Ptr++ = 0;

	/* Part B : Linear slope */
	Value = 0;
	Counter = A_value;

	for (;;)
	{
		/* Write slope step */
		for (i=0;i<B_value;i++)
		{
			/* Write shade value */
			*Ptr++ = Value;

			/* Count up */
			Counter++;

			/* Exit if function is full */
			if (Counter >= 1024)
			{
				Value = MAX_3D_SHADE_LEVELS;
				break;
			}
		}
		/* Exit if value has reached maximum */
		if (Value >= MAX_3D_SHADE_LEVELS)
			break;

		/* Increase value */
		Value++;
	}

	/* Part C : Shading is set to maximum */
	for (i=0;i<1024 - Counter;i++)
		*Ptr++ = MAX_3D_SHADE_LEVELS;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_shade_table_address
 * FUNCTION  : Get the address of the 3D shade table.
 * FILE      : 3D_PREP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.07.95 14:20
 * LAST      : 29.07.95 16:24
 * INPUTS    : None.
 * RESULT    : UNBYTE * : Pointer to shade table.
 * BUGS      : No known.
 * NOTES     : - The handle is not claimed, so it needn't be freed afterwards.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_shade_table_address(void)
{
	return (UNBYTE *) (((UNLONG) MEM_Get_pointer(Shade_table_handle) +
	 256) & 0xffffff00);
}

