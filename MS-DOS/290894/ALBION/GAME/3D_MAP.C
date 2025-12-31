/************
 * NAME     : 3D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 11.08.94 11:58
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <GFXFUNC.H>
#include <SORT.H>

#include <ALBION.H>
#include <CONTROL.H>
#include <MAP.H>
#include <GAME.H>
#include <3D_MAP.H>
#include <3DCOMMON.H>
#include <3DM.H>
#include <ANIMATE.H>
#include <XFTYPES.H>

/* structure definitions */
struct Texture_3D {
	UNSHORT X;
	UNSHORT Width,Height;
	union {
		MEM_HANDLE Handle;
		struct Texture_3D **Ptr;
	} p;
};

/* global variables */

struct Module M3_Mod =
{
	0, SCREEN_MOD,
	Update_3D_map,
	Init_3D_map,
	Exit_3D_map,
	NULL,
	NULL,
	Display_3D_map
};

struct Method M3_methods[] = {
	{ INIT_METHOD, Init_3D_object },
	{ EXIT_METHOD, Exit_3D_object },
	{ RIGHT_METHOD, Exit_program },
	{ CUSTOMKEY_METHOD, Handle_keys_3D }
};

struct Object_class M3_Class = {
	sizeof(struct Object),
	&M3_methods[0]
};

UNSHORT M3_Object;

UNSHORT Nr_of_object_groups, Nr_of_floors, Nr_of_objects, Nr_of_walls;
UNSHORT Nr_of_floor_buffers, Nr_of_textures, Nr_of_texture_buffers;

MEM_HANDLE Lab_data_handle, Shade_table_handle, Background_gfx_handle;
MEM_HANDLE Floor_buffer_handles[FLOORS_MAX * 2];
MEM_HANDLE Texture_buffer_handles[TEXTURES_MAX];
MEM_HANDLE Overlay_handles[WALLS_MAX][8];
MEM_HANDLE Wall_shade_tables[WALLS_MAX];

struct Texture_3D Floor_textures[FLOORS_MAX][8];
struct Texture_3D Textures[(OBJECTS_MAX * 8) + (WALLS_MAX * 9)];
struct Texture_3D *Object_textures[OBJECTS_MAX][8];
struct Texture_3D *Wall_textures[WALLS_MAX][9];

struct Interface_3DM I3DM;

struct OPM M3_OPM;

void
Init_3D_object(struct Object *Object, union Method_parms *P)
{
	Object->Rect.left += P->Rect.left;
	Object->Rect.top += P->Rect.top;
	Object->Rect.width = P->Rect.width;
	Object->Rect.height = P->Rect.height;
}

void
Exit_3D_object(struct Object *Object, union Method_parms *P)
{
}

void
Handle_keys_3D(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;

	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		case BLEV_ESC:
			Quit_program = TRUE;
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_3D_map
 * FUNCTION  : Initialize the 3D map module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:00
 * LAST      : 11.08.94 12:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_3D_map(void)
{
	struct Lab_data *Lab_ptr;
	struct Floor_3D *Floor_ptr;
	struct Object_3D *Object_ptr;
	struct Wall_3D *Wall_ptr;
	UNSHORT i,j;
	UNBYTE *Ptr;

	OPM_CreateVirtualOPM(&Main_OPM, &M3_OPM, 0,0,360,192);
	Push_root(&M3_OPM);

	{
		struct BBRECT x = {0,0,360,192};

		M3_Object = Add_object(0, &M3_Class, (UNBYTE *) &x);
	}

	// Load map texts


	/* Load LAByrinth data */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	i = Map_ptr->LAB_DATA_NR;
	MEM_Free_pointer(Map_handle);

	Lab_data_handle = XLD_Load_subfile(&Xftypes[LAB_DATA], i);

	/* Get pointers to various parts of the lab data */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Nr_of_object_groups = Lab_ptr->Nr_object_groups;

	Ptr = (UNBYTE *) Lab_ptr + sizeof(struct Lab_data)
		+ Nr_of_object_groups * sizeof(struct Object_group_3D);

	Nr_of_floors = *((UNSHORT *) Ptr);
	Ptr += 2;

	Floor_ptr = (struct Floor_3D *) Ptr;

	Ptr += (Nr_of_floors * sizeof(struct Floor_3D));

	Nr_of_objects = *((UNSHORT *) Ptr);
	Ptr += 2;

	Object_ptr = (struct Object_3D *) Ptr;

	Ptr += Nr_of_objects * sizeof(struct Object_3D);

	Nr_of_walls = *((UNSHORT *) Ptr);
	Ptr += 2;

	Wall_ptr = (struct Wall_3D *) Ptr;

	/* Load shade table */
/*	Shade_table_handle = XLD_Load_subfile(&Xftypes[SHADE_TABLE],
	 Lab_ptr->Shade_table_nr); */

	Shade_table_handle = MEM_Allocate_memory(64*256);

	Ptr = MEM_Claim_pointer(Shade_table_handle);

	for (i=0;i<64;i++)
	{
		for (j=0;j<256;j++)
		{
			*Ptr++ = j;
		}
	}

	MEM_Free_pointer(Shade_table_handle);

	/* Load background graphics */
	i = Lab_ptr->Background_graphics_nr;
	if (i)
		Background_gfx_handle = XLD_Load_subfile(&Xftypes[BACKGROUND_GFX], i);
	else
		Background_gfx_handle = NULL;

	/* Load floor, ceiling, object, overlay and wall graphics */
	Load_3D_floors(Floor_ptr);
	Load_3D_textures(Object_ptr, Wall_ptr);
	Load_3D_overlays(Wall_ptr);

	/* Put un-animated overlays on un-animated walls */
	{
		struct Wall_3D *Work_ptr;
		struct Overlay_3D *Overlay_ptr;
		MEM_HANDLE Wall_handle;
		UNBYTE *Wall_graphics, *Overlay_graphics;
		UNSHORT Q;

		Work_ptr = (struct Wall_3D *) Ptr;
		for (i=0;i<Nr_of_walls;i++)
		{
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);

			/* Is this wall un-animated and does it have overlays ? */
			if ((Work_ptr->Nr_frames == 1) && Work_ptr->Nr_overlays)
			{
				/* Yes -> Are any of the overlays animated ? */
				Q = 0;
				for (j=0;j<Work_ptr->Nr_overlays;j++)
				{
					if (Overlay_ptr[j].Nr_frames > 1)
						Q++;
				}

				if (!Q)
				{
					/* Yes -> Put all the overlays on the walls */
					Wall_handle = (Wall_textures[i][8])->p.Handle;
					Wall_graphics = MEM_Claim_pointer(Wall_handle);

					for (j=0;j<Work_ptr->Nr_overlays;j++)
					{
						Overlay_graphics = MEM_Claim_pointer(Overlay_handles[i][j]);

						Put_overlay_on_wall(Work_ptr, &(Overlay_ptr[j]), Wall_graphics,
						 Overlay_graphics);

						MEM_Free_pointer(Overlay_handles[i][j]);
					}

					MEM_Free_pointer(Wall_handle);
				}
			}

			/* Skip overlay data */
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Overlay_ptr += Work_ptr->Nr_overlays;
			Work_ptr = (struct Wall_3D *) Overlay_ptr;
		}
	}

	/* Calculate shade tables for walls */

	/* Initialize 3DM interface */
	I3DM.Flags = 0;
	// Init coordinates and angle
	{
		UNLONG *Urg;

		Urg = (UNLONG *) &I3DM.Player_X[0];

		*Urg = PARTY_DATA.X * SQUARE_SIZE_3D;

		I3DM.Player_X[2] = 0;

		Urg = (UNLONG *) &I3DM.Player_Z[0];

		*Urg = PARTY_DATA.Y * SQUARE_SIZE_3D;

		I3DM.Player_Z[2] = 0;
	}
	I3DM.Camera_angle = 0;

	// Init horizon and camera height
	I3DM.Camera_height = 165;
	I3DM.Horizon_Y = Lab_ptr->Horizon_Y;

	// Set window width and height
	I3DM.Window_3D_width = 360;
	I3DM.Window_3D_height = 200;

	// Set wall height and square size in cm
	I3DM.Wall_height = Lab_ptr->Wall_height_cm;

	// Call NPC Init -> Set number of NPCs and pointer to list
	// Set map width and height
	// Set shade factor
	I3DM.Shade_factor = 0;
	// Set size of background graphics

	I3DM.Target_OPM = &M3_OPM;

	MEM_Free_pointer(Lab_data_handle);

//

	/* Initialize 3DM */
//Set window size
//Set square size
//	Init_3DM();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_3D_map
 * FUNCTION  : Exit the 3D map module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.94 15:40
 * LAST      : 15.08.94 15:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_3D_map(void)
{
	UNSHORT i,j;

	/* Free floor and ceiling graphics */
	for (i=0;i<Nr_of_floor_buffers;i++)
		MEM_Free_memory(Floor_buffer_handles[i]);

	/* Free object and wall graphics */
	for (i=0;i<Nr_of_texture_buffers;i++)
		MEM_Free_memory(Texture_buffer_handles[i]);

	/* Free overlay graphics */
	for (i=0;i<Nr_of_walls;i++)
		for (j=0;j<OVERLAYS_MAX;j++)
			MEM_Free_memory(Overlay_handles[i][j]);

	/* Free background graphics and shade table */
	if (Background_gfx_handle)
		MEM_Free_memory(Background_gfx_handle);
	MEM_Free_memory(Shade_table_handle);

	/* Free LAByrinth data */
	MEM_Free_memory(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_3D_map
 * FUNCTION  : Update the 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.94 10:15
 * LAST      : 16.08.94 10:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_3D_map(void)
{
	struct Map_data *Map_ptr;
	struct Lab_data *Lab_ptr;
	struct Floor_3D *Floor_ptr;
	struct Object_3D *Object_ptr;
	struct Wall_3D *Wall_ptrs[WALLS_MAX], *Work_ptr;
	struct Overlay_3D *Overlay_ptr;
	UNSHORT i,j;
	UNBYTE *Ptr;

	// Call NPC handler -> Update NPC list

	/* Set pointers to various parts of the lab data */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Ptr = (UNBYTE *) (Lab_ptr + 1);

	I3DM.Object_groups = (struct Object_group_3D *) Ptr;

	Ptr += (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;

	I3DM.Floors = (struct Floor_3D *) Ptr;

	Ptr += (Nr_of_floors * sizeof(struct Floor_3D)) + 2;

	I3DM.Objects = (struct Object_3D *) Ptr;

	Ptr += (Nr_of_objects * sizeof(struct Object_3D)) + 2;

	/* Build list of pointers to wall data */
	Work_ptr = (struct Wall_3D *) Ptr;
	for (i=0;i<Nr_of_walls;i++)
	{
		/* Store pointer to wall data */
		Wall_ptrs[i] = Work_ptr;

		/* Skip overlay data */
		Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
		Overlay_ptr += Work_ptr->Nr_overlays;
		Work_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/* Store pointer to list */
	I3DM.Wall_pointers = &Wall_ptrs[0];

	/* Set pointer to map, shade table and background graphics */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	I3DM.Map_pointer = (struct Square_3D *) (Map_ptr + 1);
	I3DM.Shade_table = MEM_Claim_pointer(Shade_table_handle);
	I3DM.Background_graphics = MEM_Claim_pointer(Background_gfx_handle);

	// Put overlays on walls

	/* Call 3DM */
//	RenderView();

	/* Exit */
	MEM_Free_pointer(Background_gfx_handle);
	MEM_Free_pointer(Shade_table_handle);
	MEM_Free_pointer(Map_handle);
	MEM_Free_pointer(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_3D_map
 * FUNCTION  : Display the 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.94 10:15
 * LAST      : 16.08.94 10:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_3D_map(void)
{
	DSA_CopyOPMToScreen(&Screen, &M3_OPM, 0, 0, DSA_CMOPMTS_ALWAYS);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_3D_floors
 * FUNCTION  : Load and convert floors and ceilings.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:10
 * LAST      : 11.08.94 12:10
 * INPUTS    : struct Floor_3D *Floor_ptr - Pointer to floor data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Load_3D_floors(struct Floor_3D *Floor_ptr)
{
	MEM_HANDLE Handles[FLOORS_MAX];
	UNSHORT Batch[FLOORS_MAX];
	UNSHORT Index;
	UNSHORT i,j;
	UNBYTE *Source, *Target;

	/* Build floor batch */
	for (i=0;i<Nr_of_floors;i++)
		Batch[i] = Floor_ptr[i].Graphics_nr;

	/* Load batch */
	XLD_Load_batch(&Xftypes[FLOOR_3D], Nr_of_floors, &Batch[0], &Handles[0]);

	/* Convert floors and ceilings */
	Nr_of_floor_buffers = 0;
	Index = 0;
	Floor_buffer_handles[0] = MEM_Allocate_memory(64 * 256);
	Target = MEM_Claim_pointer(Floor_buffer_handles[0]);

	for (i=0;i<Nr_of_floors;i++)
	{
		Source = MEM_Claim_pointer(Handles[i]);

		for (j=0;j<Floor_ptr[i].Nr_frames;j++)
		{
			/* Copy and convert floor frame */
			{
				UNSHORT k,l;
				UNBYTE *Ptr;

				Ptr = Target + (Index * 64);
				for (k=0;k<64;k++)
				{
					for (l=0;l<64;l++)
					{
						*Ptr++ = *Source++;
					}
					Ptr += 192;					/* 256 - 64 */
				}
			}

			/* Initialize texture data */
			Floor_textures[i][j].X = Index * 64;
			Floor_textures[i][j].p.Handle = Floor_buffer_handles[Nr_of_floor_buffers];

			/* Time for a new buffer ? */
			Index++;
			if (Index == 4)
			{
				/* Yes -> Allocate a new one */
				MEM_Free_pointer(Floor_buffer_handles[Nr_of_floor_buffers]);

				Nr_of_floor_buffers++;

				Floor_buffer_handles[Nr_of_floor_buffers] = MEM_Allocate_memory(64 * 256);
				Target = MEM_Claim_pointer(Floor_buffer_handles[Nr_of_floor_buffers]);

				Index = 0;
			}
		}

		/* Discard floor */
		MEM_Free_pointer(Handles[i]);
		MEM_Free_memory(Handles[i]);
	}

	/* Ready */
	MEM_Free_pointer(Floor_buffer_handles[Nr_of_floor_buffers]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_3D_textures
 * FUNCTION  : Load and convert object and wall textures.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:10
 * LAST      : 11.08.94 12:10
 * INPUTS    : struct Object_3D *Object_ptr - Pointer to object data.
 *             struct Wall_3D *Wall_ptr - Pointer to wall data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Load_3D_textures(struct Object_3D *Object_ptr, struct Wall_3D *Wall_ptr)
{
	struct Wall_3D *Work_ptr;
	struct Overlay_3D *Overlay_ptr;
	UNSHORT i,j;
	UNSHORT Q;

	/*** Build list of textures that must be converted ***/
	Nr_of_textures = 0;
	Nr_of_texture_buffers = 0;

	/* First the objects */
	for (i=0;i<Nr_of_objects;i++)
	{
		/* Do each animation frame */
		for (j=0;j<Object_ptr[i].Nr_frames;j++)
		{
			/* Create texture data */
			Textures[Nr_of_textures].Width = Object_ptr[i].Graphics_width;
			Textures[Nr_of_textures].Height = Object_ptr[i].Graphics_height;
			Textures[Nr_of_textures].p.Ptr = &Object_textures[i][j];

			Object_textures[i][j] = &Textures[Nr_of_textures];

			Nr_of_textures++;
		}
	}

	/* Then the walls */
	Work_ptr = Wall_ptr;
	for (i=0;i<Nr_of_walls;i++)
	{
		Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);

		/* Does this wall have overlays ? */
		Q = 0;
		if (Work_ptr->Nr_overlays)
		{
			/* Yes -> Create texture data for one extra texture */
			Textures[Nr_of_textures].Width = Work_ptr->Graphics_width;
			Textures[Nr_of_textures].Height = Work_ptr->Graphics_height;
			Textures[Nr_of_textures].p.Ptr = &Wall_textures[i][8];

			Wall_textures[i][8] = &Textures[Nr_of_textures];

			Nr_of_textures++;

			/* Are any of the overlays animated ? */
			for (j=0;j<Work_ptr->Nr_overlays;j++)
			{
				if (Overlay_ptr[j].Nr_frames == 1)
					Q++;
			}
		}

		/* Is this wall animated or does it only have un-animated overlays ? */
		if (!Q || (Work_ptr->Nr_frames > 1))
		{
			/* Yes -> Do each animation frame */
			for (j=0;j<Work_ptr->Nr_frames;j++)
			{
				/* Create texture data */
				Textures[Nr_of_textures].Width = Work_ptr->Graphics_width;
				Textures[Nr_of_textures].Height = Work_ptr->Graphics_height;
				Textures[Nr_of_textures].p.Ptr = &Wall_textures[i][j];

				Wall_textures[i][j] = &Textures[Nr_of_textures];

				Nr_of_textures++;
			}
		}

		/* Skip overlay data */
		Overlay_ptr += Work_ptr->Nr_overlays;
		Work_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/*** Sort textures on height, then width ***/
	Shellsort(Swap_3D_textures, Compare_3D_textures, Nr_of_textures, (UNBYTE *) &Textures[0]);

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
		while (TRUE)
		{
			/* Inspect the next texture */
			Index2++;

			/* Is the height difference greater than the first height OR
			 have all the textures been inspected ? */
			if ((Index2 >= Nr_of_textures)
				|| ((Textures[Index2].Height - Height1) > Height1))
			{
				/* Yes -> Determine the positions for all textures until now */
				Position_3D_textures(Index1, Index2 - 1, Height2);

				/* Exit if all textures have been inspected */
				if (Index2 >= Nr_of_textures)
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
		MEM_HANDLE Handles[OBJECTS_MAX];
		UNLONG Size;
		UNSHORT Batch[OBJECTS_MAX];
		UNBYTE *Ptr;

		/* Build object batch */
		for (i=0;i<Nr_of_objects;i++)
			Batch[i] = Object_ptr[i].Graphics_nr;

		/* Load object graphics */
		XLD_Load_batch(&Xftypes[OBJECT_3D], Nr_of_objects, &Batch[0], &Handles[0]);

		/* Convert and copy object graphics */
		for (i=0;i<Nr_of_objects;i++)
		{
			Size = Object_ptr[i].Graphics_width * Object_ptr[i].Graphics_height;
			Ptr = MEM_Claim_pointer(Handles[i]);

			/* Copy each animation frame */
			for (j=0;j<Object_ptr[i].Nr_frames;j++)
			{
				Copy_texture(Object_textures[i][j], Ptr);
				Ptr += Size;
			}

			/* Destroy object graphics */
			MEM_Free_pointer(Handles[i]);
			MEM_Free_memory(Handles[i]);
		}
	}

	/*** Load and convert the wall textures ***/
	{
		MEM_HANDLE Handles[WALLS_MAX];
		UNLONG Size;
		UNSHORT Batch[WALLS_MAX];
		UNBYTE *Ptr;

		/* Build wall batch */
		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_of_walls;i++)
		{
			Batch[i] = Work_ptr[i].Graphics_nr;

			/* Skip overlay data */
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Overlay_ptr += Work_ptr->Nr_overlays;
			Work_ptr = (struct Wall_3D *) Overlay_ptr;
		}

		/* Load wall graphics */
		XLD_Load_batch(&Xftypes[WALL_3D], Nr_of_walls, &Batch[0], &Handles[0]);

		/* Convert and copy wall graphics */
		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_of_walls;i++)
		{
			Size = Work_ptr[i].Graphics_width * Work_ptr[i].Graphics_height;
			Ptr = MEM_Claim_pointer(Handles[i]);
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);

			/* Does this wall have overlays ? */
			Q = 0;
			if (Work_ptr->Nr_overlays)
			{
				/* Yes -> Copy one extra frame */
				Copy_texture(Wall_textures[i][8], Ptr);

					/* Are any of the overlays animated ? */
					for (j=0;j<Work_ptr->Nr_overlays;j++)
					{
						if (Overlay_ptr[j].Nr_frames == 1)
							Q++;
					}
			}

			/* Is this wall animated or does it only have un-animated overlays ? */
			if (!Q || (Work_ptr->Nr_frames > 1))
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Copy_texture
 * FUNCTION  : Copy and convert a texture.
 * FILE      : 3D_MAP.C
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
	UNSHORT i,j;
	UNBYTE *Target, *Ptr;

	/* Get pointer to texture buffer */
	Target = MEM_Claim_pointer(Texture->p.Handle) + Texture->X;

	/* Copy and convert texture */
	for (i=0;i<Texture->Width;i++)
	{
		Ptr = Target;
		for (j=0;j<Texture->Height;j++)
		{
			*Ptr++ = *Source++;
		}
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
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 17:17
 * LAST      : 11.08.94 17:17
 *	INPUTS    : UNSHORT Start - Index of first texture to be copied.
 *             UNSHORT End - Index of last texture to be stored.
 *             UNSHORT Height - Height of the texture buffers.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Position_3D_textures(UNSHORT Start, UNSHORT End, UNSHORT Height)
{
	MEM_HANDLE Handle;
	UNSHORT X;
	UNBYTE *Ptr;

	while (TRUE)
	{
		/* Make a new texture buffer */
		Handle = MEM_Allocate_memory((UNLONG) Height * 256);

		Texture_buffer_handles[Nr_of_texture_buffers] = Handle;
		Nr_of_texture_buffers++;

		Ptr = MEM_Claim_pointer(Handle);
		X = 0;

		/* Position as many large textures as possible */
		while ((End >= Start) && (X + Textures[End].Width <= 256))
		{
			/* Insert texture data */
			Textures[End].X = X;
			Textures[End].p.Handle = Handle;

			/* Increase X-coordinate in texture buffer */
			X += Textures[End].Width;

			/* Next texture */
			End--;
		}

		/* Position as many small textures as possible */
		while ((Start <= End) && (X + Textures[Start].Width > 256))
		{
			/* Insert texture data */
			Textures[Start].X = X;
			Textures[Start].p.Handle = Handle;

			/* Increase X-coordinate in texture buffer */
			X += Textures[Start].Width;

			/* Next texture */
			Start++;
		}

		/* Next texture buffer */
		MEM_Free_pointer(Handle);

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
 * FILE      : 3D_MAP.C
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
	struct Texture_3D *List, T, **P;

	List = (struct Texture_3D *) Data;

	memcpy (&T, &List[A], sizeof(struct Texture_3D));
	memcpy (&List[A], &List[B], sizeof(struct Texture_3D));
	memcpy (&List[B], &T, sizeof(struct Texture_3D));

	P = List[A].p.Ptr;
	*P = &List[B];

	P = List[B].p.Ptr;
	*P = &List[A];
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_3D_textures
 * FUNCTION  : Compare two 3D textures (for sorting).
 * FILE      : 3D_MAP.C
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
	struct Texture_3D *List, *T;

	List = (struct Texture_3D *) Data;

	if (List[A].Height == List[B].Height)
		return(List[A].Width > List[B].Width);
	else
		return(List[A].Height > List[B].Height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_3D_overlays
 * FUNCTION  : Load and convert overlays.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.94 15:30
 * LAST      : 15.08.94 15:30
 * INPUTS    : struct Wall_3D *Wall_ptr - Pointer to wall data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Load_3D_overlays(struct Wall_3D *Wall_ptr)
{
	struct Overlay_3D *Overlay_ptr;
	UNSHORT Batch[WALLS_MAX][OVERLAYS_MAX];
	UNSHORT i,j;

	/* Build overlay batch */
	for (i=0;i<Nr_of_walls;i++)
	{
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);

		/* Do each overlay */
		for (j=0;j<Wall_ptr->Nr_overlays;j++)
		{
			Batch[i][j] = Overlay_ptr[j].Graphics_nr;
		}

		/* Clear the rest of the batch */
		for (;j<OVERLAYS_MAX;j++)
		{
			Batch[i][j] = 0;
		}

		/* Skip overlay data */
		Overlay_ptr += Wall_ptr->Nr_overlays;
		Wall_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/* Load overlay graphics */
	XLD_Load_batch(&Xftypes[OVERLAY_3D], Nr_of_walls * OVERLAYS_MAX,
	 &Batch[0], &Overlay_handles[0][0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_object_graphics
 * FUNCTION  : Return pointer to object graphics.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 16:27
 * LAST      : 18.08.94 16:27
 * INPUTS    : UNSHORT NPC_nr - NPC index (0xFFFF for normal objects).
 *             UNSHORT Object_nr - Object number (1...).
 *             UNSHORT Hash_nr - Number used for animation hashing.
 * RESULT    : UNBYTE * : Pointer to object graphics.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_object_graphics(UNSHORT NPC_nr, UNSHORT Object_nr, UNSHORT Hash_nr)
{
	struct Object_3D *Object;
	struct Texture_3D *Texture;
	MEM_HANDLE Handle;
	UNSHORT Frame;

	/* Get pointer to object data */
	Object = (I3DM.Objects) + Object_nr - 1;

	if (NPC_nr == 0xFFFF)
	{
		Frame = Get_animation_frame(Object->Flags, Object->Nr_frames, Hash_nr);

		Texture = Object_textures[Object_nr-1][Frame];

		return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
	}
	else
		return (0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_floor_graphics
 * FUNCTION  : Return pointer to floor graphics.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 16:48
 * LAST      : 18.08.94 16:48
 * INPUTS    : UNSHORT Floor_nr - Floor number (1...).
 *             UNSHORT Hash_nr - Number used for animation hashing.
 * RESULT    : UNBYTE * : Pointer to floor graphics.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Get_floor_graphics(UNSHORT Floor_nr, UNSHORT Hash_nr)
{
	struct Floor_3D *Floor;
	struct Texture_3D *Texture;
	MEM_HANDLE Handle;
	UNSHORT Frame;

	/* Get pointer to floor data */
	Floor = (I3DM.Floors) + Floor_nr - 1;

	Frame = Get_animation_frame(Floor->Flags, Floor->Nr_frames, Hash_nr);

	Texture = &Floor_textures[Floor_nr-1][Frame];

	return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_wall_graphics
 * FUNCTION  : Return pointer to wall graphics.
 * FILE      : 3D_MAP.C
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
	MEM_HANDLE Handle;
	UNSHORT Frame;

	/* Get pointer to wall data */
	Wall = I3DM.Wall_pointers[Wall_nr-1];

	/* Does this wall have overlays ? */
	if (Wall->Nr_overlays)
		/* Yes -> Use last frame, which has been precalculated */
		Frame = 8;
	else
		/* No -> Just take the right frame */
		Frame = Get_animation_frame(Wall->Flags, Wall->Nr_frames, Hash_nr);

	Texture = Wall_textures[Wall_nr-1][Frame];

	return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_wall_shade_table
 * FUNCTION  : Return pointer to wall shade table.
 * FILE      : 3D_MAP.C
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
	return(0);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_overlay_on_wall
 * FUNCTION  : Put an overlay on a wall.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.08.94 18:00
 * LAST      : 18.08.94 18:00
 * INPUTS    : struct Wall_3D *Wall - Pointer to wall data.
 *             struct Overlay_3D *Overlay - Pointer to overlay data.
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
Put_overlay_on_wall(struct Wall_3D *Wall, struct Overlay_3D *Overlay,
 UNBYTE *Wall_graphics, UNBYTE *Overlay_graphics)
{
	UNSHORT i,j;
	UNBYTE *Ptr, c;

	/* Calculate destination address */
	Wall_graphics += Overlay->X + (Overlay->Y * Wall->Graphics_width);

	/* Block or mask ? */
	if (Overlay->Mask_flag)
	{
		/* Put overlay on wall (masked) */
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
	 	/* Put overlay on wall (blocked) */
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

