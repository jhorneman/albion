/************
 * NAME     : 3D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 11-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <ALBION.H>

#include <XLOAD.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>

#include <CONTROL.H>
#include <MAP.H>
#include <AUTOMAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <GAMETIME.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <3DM.H>
#include <XFTYPES.H>
#include <COLOURS.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <POPUP.H>
#include <GAMETEXT.H>
#include <MUSIC.H>
#include <EVELOGIC.H>
#include <SPECITEM.H>
#include <MAP_PUM.H>
#include <MAGIC.H>
#include <MAINMENU.H>
#include <3D_MOVE.H>
#include <3D_SEL.H>

/* defines */

#define BASE_SHADE_FACTOR		(3000)
#define MAX_SHADE_FACTOR		(4096 - BASE_SHADE_FACTOR)

#define MIN_3D_WINDOW_WIDTH	(120)
#define MAX_3D_WINDOW_WIDTH	(360)
#define MIN_3D_WINDOW_HEIGHT	(64)
#define MAX_3D_WINDOW_HEIGHT	(192)

/* structure definitions */

/* prototypes */

/* 3D map module functions */
void M3_ModInit(void);
void M3_ModExit(void);
void M3_DisInit(void);
void M3_DisExit(void);

BOOLEAN Init_3DM_library(void);

/* 3D map object methods */
UNLONG Touched_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Left_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Right_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_3D_object(struct Object *Object, union Method_parms *P);

void Error_3DM(UNSHORT Error_code);

/* Shade factor functions */
void Init_shade_factor(void);
SISHORT Get_target_shade_factor(void);
void Update_shade_factor(void);

/* global variables */

/* 3D map module */
struct Module M3_Mod =
{
	0, SCREEN_MOD, MAP_3D_SCREEN,
	M3_MainLoop,
	M3_ModInit,
	M3_ModExit,
	M3_DisInit,
	M3_DisExit,
	Update_map_display
};

/* 3D map window method list */
static struct Method M3_methods[] = {
	{ TOUCHED_METHOD,		Touched_3D_object },
	{ LEFT_METHOD,			Left_3D_object },
	{ RIGHT_METHOD,		Right_3D_object },
	{ CUSTOMKEY_METHOD,	Customkeys_3D_object },
	{ 0, NULL}
};

/* 3D map window object class description */
static struct Object_class M3_Class = {
	0, sizeof(struct Object),
	&M3_methods[0]
};

static UNSHORT M3_Object;

static UNSHORT Corner_state_3D;

UNSHORT Window_3D_size;

UNSHORT Window_3D_X, Window_3D_Y;

static SISHORT Shade_offsets[4] = {-20, -1, -18, 0};

/* Shade factor variables */
static SISHORT Current_shade_factor;
static SISHORT Max_shade_factor;

struct Interface_3DM I3DM;

static struct OPM M3_OPM;

/* Error handling data */
static UNCHAR _3DM_library_name[] = "3DM";

static struct Error_message _3DM_errors[] = {
	{ERR_3DM_NOT_INITIALIZED,	"3DM not initialized."},
	{ERR_3DM_SQUARE_SIZE, 		"Illegal square size."},
	{ERR_3DM_WINDOW_SIZE, 		"Illegal window size."},
	{ERR_3DM_HEIGHT, 				"Illegal camera height."},
	{ERR_3DM_MAP, 					"Illegal map pointer."},
	{ERR_3DM_MAP_SIZE, 			"Illegal map size."},
	{ERR_3DM_WALL_HEIGHT, 		"Illegal wall height."},
	{ERR_3DM_DATA, 				"Illegal data in interface."},
	{ERR_3DM_SHADE_TABLE, 		"Illegal shade table."},
	{ERR_3DM_RATIO, 				"Illegal aspect ratio."},
	{ERR_VIEW_DEPTH, 				"Illegal view depth."},
	{0, NULL}
};

#ifdef PREP_3D_DEBUG_FLAG
extern UNLONG Actual_size;
extern UNLONG Converted_size;
extern UNSHORT Nr_textures;
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_ModInit
 * FUNCTION  : Initialize 3D map module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:00
 * LAST      : 10.10.95 17:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M3_ModInit(void)
{
	BOOLEAN Result;
	UNSHORT i;

	/* Clear corner state */
	Corner_state_3D = 0xFFFF;

	/* Initialize data */
	Result = Init_3D_data();
	if (!Result)
	{
		Exit_program();
		return;
	}

	/* Initialize 3DM library */
	Result = Init_3DM_library();
	if (!Result)
	{
		Exit_program();
		return;
	}

	/* Initialize located effects */
	Map_square_size = I3DM.Square_size;
	Initialize_located_effects(12 * Map_square_size);

	/* Set NPC reaction radius */
	NPC_reaction_radius = (I3DM.Square_size * 2) / 3;

	/* Initialize shade factor */
	Init_shade_factor();

	/* Loading a game ? */
	if (Loading_game)
	{
		/* Yes -> Copy extra information from party data */
		for (i=0;i<3;i++)
		{
			I3DM.Player_X[i] = PARTY_DATA.Player_X[i];
			I3DM.Player_Z[i] = PARTY_DATA.Player_Z[i];
		}

		I3DM.Camera_angle		= PARTY_DATA.Camera_angle;
		I3DM.Camera_height 	= PARTY_DATA.Camera_height;
		I3DM.Horizon_Y			= PARTY_DATA.Horizon_Y;
	}
	else
	{
		/* No -> Initialize party position */
		Initialize_3D_position();
	}

	/* Initialize NPCs */
	Init_3D_NPCs();
	Display_3D_NPCs();
	I3DM.Nr_NPCs = Nr_3D_NPCs;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_ModExit
 * FUNCTION  : Exit 3D map module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.94 15:40
 * LAST      : 23.08.95 12:37
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M3_ModExit(void)
{
	/* Destroy OPM */
	OPM_Del(&M3_OPM);

	/* Close 3DM */
	Exit_3DM();

	/* Exit 3D data */
	Exit_3D_data();

	Exit_3D_NPCs();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_DisInit
 * FUNCTION  : Initialize 3D map display.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 15:29
 * LAST      : 10.10.95 18:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M3_DisInit(void)
{
	if (!Map_display_initialized)
	{
		/* Initialize map display */
		Init_map_display();

		/* Set 3D window size */
		Set_3D_window_size(Window_3D_size);

		/* Re-draw slab */
		OPM_CopyOPMOPM
		(
			&Slab_OPM,
			&Main_OPM,
			0,
			0,
			360,
			min(Panel_Y, 192),
			0,
			0
		);

		/* Window less than full screen ? */
		if ((I3DM.Window_3D_width < Screen_width) &&
		 (I3DM.Window_3D_height < Panel_Y))
		{
			/* Yes -> Draw border around 3D window */
			Draw_deep_border
			(
				&Main_OPM,
				Window_3D_X - 1,
				Window_3D_Y - 1,
				I3DM.Window_3D_width + 2,
				I3DM.Window_3D_height + 2
			);
		}

		/* Add OPM to update list */
		Add_update_OPM(&M3_OPM);

		/* Add interface object */
		M3_Object = Add_object
		(
			Earth_object,
			&M3_Class,
			NULL,
			Window_3D_X,
			Window_3D_Y,
			I3DM.Window_3D_width,
			I3DM.Window_3D_height
		);

		Map_display_initialized = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_DisExit
 * FUNCTION  : Exit 3D map display.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 15:29
 * LAST      : 13.08.95 11:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M3_DisExit(void)
{
	if (Map_display_initialized)
	{
		/* Delete interface object */
		Delete_object(M3_Object);

		/* Remove OPM from update list */
		Remove_update_OPM(&M3_OPM);

		/* Exit map display */
		Exit_map_display();

		Map_display_initialized = FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_MainLoop
 * FUNCTION  : 3D map main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.94 14:14
 * LAST      : 28.07.95 17:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Update_time must come at the end because it may trigger
 *              events.
 *             - This function is called from other main loop functions!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M3_MainLoop(void)
{
	/* Display map */
	Update_display();

	/* Update NPC positions */
	Handle_3D_NPCs();

	/* Update time */
	Update_time(Update_duration * 10);

	#ifdef PREP_3D_DEBUG_FLAG
	OPM_FillBox(&Status_area_OPM, 181, 3, 120, 40, BLACK);

	xprintf(&Status_area_OPM, 182, 4, "Actual : %lu", Actual_size);
	xprintf(&Status_area_OPM, 182, 14, "Converted : %lu", Converted_size);
	xprintf(&Status_area_OPM, 182, 24, "Textures : %u", Nr_textures);
	#endif
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_3D_map
 * FUNCTION  : Display the 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.94 10:15
 * LAST      : 07.10.95 18:58
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
	UNSHORT Width, Height;

	/* Update map stuff */
	Update_map_stuff();

	/* Set 3D position of listener */
	Set_listener_position
	(
		*((UNLONG *) &(I3DM.Player_X[1])),
		(Map_height * Map_square_size) - *((UNLONG *) &(I3DM.Player_Z[1])),
		(((I3DM.Camera_angle / 65536) * 360) / ANGLE_STEPS)
	);

	/* Set view direction */
	PARTY_DATA.View_direction = ((((((0 - (I3DM.Camera_angle / 65536)) -
	 (ANGLE_STEPS / 8)) & (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 4)) + 1) &
	 0x0003);

	/* Update shade factor */
	Update_shade_factor();

	/* Put overlays on walls */
	Put_overlays_on_walls();

	/* Render 3D map */
	Render_3D_map();

	/* Is the mouse in a corner ? */
	if (Corner_state_3D != 0xFFFF)
	{
		/* Yes -> Get dimensions of display window */
		Width		= I3DM.Window_3D_width;
		Height	= I3DM.Window_3D_height;

		/* Act on depending on corner state */
		switch (Corner_state_3D)
		{
			/* Top left */
			case 0:
			{
				Put_masked_block
				(
					&M3_OPM,
					0,
					0,
					16,
					16,
					&(Left_90_symbol[0])
				);
				break;
			}
			/* Top right */
			case 1:
			{
				Put_masked_block
				(
					&M3_OPM,
					Width - 16,
					0,
					16,
					16,
					&(Right_90_symbol[0])
				);
				break;
			}
			/* Bottom left */
			case 2:
			{
				Put_masked_block
				(
					&M3_OPM,
					0,
					Height - 16,
					16,
					16,
					&(Left_180_symbol[0])
				);
				break;
			}
			/* Bottom right */
			case 3:
			{
				Put_masked_block
				(
					&M3_OPM,
					Width - 16,
					Height - 16,
					16,
					16,
					&(Right_180_symbol[0])
				);
				break;
			}
		}
	}

	/* Display special items in 3D */
	Display_special_items_3D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Render_3D_map
 * FUNCTION  : Render the 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.07.95 16:11
 * LAST      : 31.08.95 11:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Render_3D_map(void)
{
	struct Map_data *Map_ptr;
	struct Lab_data *Lab_ptr;
	struct Wall_3D *Wall_ptrs[MAX_WALLS];
	UNSHORT Err;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Display NPCs */
	Display_3D_NPCs();
	I3DM.Nr_NPCs = Nr_3D_NPCs;

	/* Set pointers to various parts of the lab data */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Ptr = (UNBYTE *) (Lab_ptr + 1);

	I3DM.Object_groups = ((struct Object_group_3D *) Ptr);

	Ptr += (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;

	I3DM.Floors = (struct Floor_3D *) Ptr;

	Ptr += (Nr_of_floors * sizeof(struct Floor_3D)) + 2;

	I3DM.Objects = (struct Object_3D *) Ptr;

	/* Build list of pointers to wall data */
	for (i=0;i<Nr_of_walls;i++)
	{
		Wall_ptrs[i] = (struct Wall_3D *) ((UNLONG) Lab_ptr + Wall_offsets[i]);
	}
	I3DM.Wall_pointers = &Wall_ptrs[0];

	/* Set pointer to map, shade table and background graphics */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	I3DM.Map_pointer = (struct Square_3D *) ((UNBYTE *) Map_ptr +
	 Map_layers_offset);

	I3DM.Shade_table = Get_shade_table_address();

	I3DM.Background_graphics = MEM_Claim_pointer(Background_gfx_handle) +
	 sizeof(struct Gfx_header);

	/* Set current shade factor */
	if (Cheat_mode)
	{
		I3DM.Shade_factor = (UNSHORT)(MAX_SHADE_FACTOR - Max_shade_factor);
	}
	else
	{
		I3DM.Shade_factor = (UNSHORT)(MAX_SHADE_FACTOR -
		 max(min(Current_shade_factor, MAX_SHADE_FACTOR - 1), 0));
	}

	/* Set shade table */
	Err = Set_3DM_ShadeTable(I3DM.Shade_table);
	if (Err)
	{
		Error_3DM(Err);
		Exit_program();
	}

	/* Call 3DM */
	Err = Render_3DM_View();
	if (Err)
	{
		Error_3DM(Err);
		Exit_program();
	}
	M3_OPM.status |= OPMSTAT_CHANGED;

	/* Free pointers */
	MEM_Free_pointer(Background_gfx_handle);
	MEM_Free_pointer(Map_handle);
	MEM_Free_pointer(Lab_data_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_3DM_library
 * FUNCTION  : Initialize 3DM library.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.07.95 14:05
 * LAST      : 10.10.95 18:41
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_3DM_library(void)
{
	struct Lab_data *Lab_ptr;
	UNSHORT View_depth;
	UNSHORT Err;
	UNSHORT i;

	/* Initialize 3DM interface */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	I3DM.Flags 						= 0;
	I3DM.Camera_height 			= 165;		// Write function !!!

	I3DM.Horizon_Y 				= Lab_ptr->Horizon_Y;
	I3DM.Horizon_offset 			= Lab_ptr->Background_offset;

	I3DM.Sky_colour 				= Lab_ptr->Sky_colour;
	I3DM.Floor_colour				= Lab_ptr->Floor_colour;

	I3DM.Square_size 				= (1 << Lab_ptr->Square_size_cm_log);

	I3DM.Wall_height 				= Lab_ptr->Wall_height;

	I3DM.Default_colour 			= Find_closest_colour(Lab_ptr->Target_R,
										  Lab_ptr->Target_G, Lab_ptr->Target_B);

	I3DM.Shade_table 				= Get_shade_table_address();

	I3DM.Shade_factor 			= 0;

	I3DM.Horizon_angle_factor	= Lab_ptr->Background_angle_factor;

	I3DM.Target_OPM 				= &M3_OPM;

	for (i=0;i<4;i++)
	{
		I3DM.Shade_offsets[i]	= (Shade_offsets[i] *
										  Lab_ptr->Shade_deviation) / 100;
	}

	/* Get view depth */
	View_depth = Lab_ptr->View_depth;

	MEM_Free_pointer(Lab_data_handle);

	/* Initialize NPC interface */
	I3DM.NPCs = NPCs_3D;
	I3DM.Nr_NPCs = 0;

	/* Is there a background ? */
	if (Background_gfx_handle)
	{
		struct Gfx_header *Gfx;

		/* Yes -> Set  size of background graphics */
		Gfx = (struct Gfx_header *) MEM_Claim_pointer(Background_gfx_handle);

		I3DM.Background_width = Gfx->Width;
		I3DM.Background_height = Gfx->Height;

		MEM_Free_pointer(Background_gfx_handle);
	}
	else
	{
		/* No */
		I3DM.Background_width = 0;
		I3DM.Background_height = 0;
	}

	/* Set interface flags (flags must be cleared !) */
	if ((Current_map_type == CITY_MAP) ||
	 (Current_map_type == WILDERNESS_MAP))
	{
		I3DM.Flags |= DUNGEON_OR_CITY;
	}

	/* Set 3D window size */
	Set_3D_window_size(Window_3D_size);

	/* Set 3DM aspect ratio */
	Err = Set_3DM_AspectRatio(111);
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

	/* Set 3DM square size */
	Err = Set_3DM_SquareSize(I3DM.Square_size);
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

	/* Set 3DM map dimensions */
	Err = Set_3DM_Map(Map_width, Map_height, I3DM.Wall_height);
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

	/* Set 3DM shade table address */
	Err = Set_3DM_ShadeTable(I3DM.Shade_table);
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

	/* Initialize 3DM */
	Err = Init_3DM();
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

	/* Set 3DM view depth */
	Err = Set_3DM_ViewDepth(View_depth);
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touched_3D_object
 * FUNCTION  : Touched method of 3D map object.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 11:05
 * LAST      : 07.10.95 19:18
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Touched_3D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Q;

	/* Update highlighting */
	Normal_touched(Object, P);

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state2(Mouse_X, Mouse_Y);

	/* Is the mouse in a corner ? */
	if (Q >= 256)
	{
		/* Yes -> Set 3D corner state */
		Corner_state_3D = Q - 256;

		/* Change mouse pointer accordingly */
		Change_mouse(&(Mouse_pointers[DEFAULT_MPTR]));
	}
	else
	{
		/* No -> Clear corner state */
		Corner_state_3D = 0xFFFF;

		/* Change mouse pointer accordingly */
		Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_3D_object
 * FUNCTION  : Left method of 3D map object.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 11:05
 * LAST      : 06.10.95 22:23
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_3D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Q;
	UNSHORT i;

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state2(Mouse_X, Mouse_Y);

	/* Is the mouse in a corner ? */
	if (Q >= 256)
	{
		/* Yes -> Act depending on corner state */
		Q -= 256;
		switch (Q)
		{
			/* Top left */
			case 0:
			{
				/* 90 degrees turn to the left */
				for (i=0;i<FULLTURN_STEPS/2-1;i++)
				{
					Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
					Update_display();
					Switch_screens();
				}
				Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
				break;
			}
			/* Top right */
			case 1:
			{
				/* 90 degrees turn to the right */
		  		for (i=0;i<FULLTURN_STEPS/2-1;i++)
				{
					Rotate_3D(-ANGLE_STEPS / (FULLTURN_STEPS * 2));
					Update_display();
					Switch_screens();
				}
				Rotate_3D(-ANGLE_STEPS / (FULLTURN_STEPS * 2));
				break;
			}
			/* Bottom left */
			case 2:
			{
				/* 180 degrees turn to the left */
		  		for (i=0;i<FULLTURN_STEPS-1;i++)
				{
					Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
					Update_display();
					Switch_screens();
				}
				Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
				break;
			}
			/* Bottom right */
			case 3:
			{
				/* 180 degrees turn to the right */
	 	  		for (i=0;i<FULLTURN_STEPS-1;i++)
				{
					Rotate_3D(-ANGLE_STEPS / (FULLTURN_STEPS * 2));
					Update_display();
					Switch_screens();
				}
				Rotate_3D(-ANGLE_STEPS / (FULLTURN_STEPS * 2));
				break;
			}
		}
	}
	else
	{
		/* No -> Change mouse pointer accordingly */
		Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

		/* Is the mouse pointer inside the 3D map window ? */
		if (Q < 12)
		{
			/* Yes -> Enter move mode */
			Push_module(&Move_mouse_M3_Mod);
		}
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_3D_object
 * FUNCTION  : Right method of 3D map object.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.11.94 16:18
 * LAST      : 20.07.95 14:10
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_3D_object(struct Object *Object, union Method_parms *P)
{
	/* Select a map element */
	Push_module(&Select_M3_Mod);

	Update_screen();

	/* Do map pop-up menu */
	Do_PUM(Mouse_X + 16, Mouse_Y - 40, &Map_PUM);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_3D_object
 * FUNCTION  : Custom keys method of 3D map object.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:03
 * LAST      : 04.10.95 22:34
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Reacted to key.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_3D_object(struct Object *Object, union Method_parms *P)
{
	BOOLEAN Result = TRUE;
	UNSHORT Key_code;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	switch (Key_code)
	{
		/* Enter main menu */
		case BLEV_ESC:
		{
			Enter_Main_menu(MAIN_MENU_MAP);
			break;
		}
		/* Enter automapper */
		case BLEV_TAB:
		{
			Enter_Automap(0);
			break;
		}
		/* Move */
		case BLEV_UP:
		case BLEV_DOWN:
		case BLEV_LEFT:
		case BLEV_RIGHT:
		{
			/* Enter move mode */
			Push_module(&Move_key_M3_Mod);
			break;
		}
		/* Look up */
		case BLEV_PGUP:
		{
			/* Look up */
			if (I3DM.Horizon_Y < (I3DM.Window_3D_height / 2))
			{
				I3DM.Horizon_Y += I3DM.Window_3D_height / 4;
			}
			break;
		}
		/* Look down */
		case BLEV_PGDN:
		{
			/* Look down */
			if (I3DM.Horizon_Y > (0 - (I3DM.Window_3D_height / 2)))
			{
				I3DM.Horizon_Y -= I3DM.Window_3D_height / 4;
			}
			break;
		}
		/* No reaction */
		default:
		{
			Result = FALSE;
			break;
		}
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Error_3DM
 * FUNCTION  : Report a 3DM error.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 12:12
 * LAST      : 15.09.94 12:12
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : 3DM.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Error_3DM(UNSHORT Error_code)
{
	struct Error_report Report;

	/* Build error report */
	Report.Code = Error_code;
	Report.Messages = &(_3DM_errors[0]);

	/* Push error on the error stack */
	ERROR_PushError(Print_error, _3DM_library_name,
	 sizeof(struct Error_report), (UNBYTE *) &Report);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_3D_window_size
 * FUNCTION  : Set 3D window size.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.09.94 16:46
 * LAST      : 10.10.95 17:17
 * INPUTS    : UNSHORT Size - New window size (0...100%)
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_3D_window_size(UNSHORT Size)
{
	UNSHORT Width;
	UNSHORT Height;
	UNSHORT Err;

	/* Store new size */
	Window_3D_size = Size;

	/* Calculate width and height */
	Width = MIN_3D_WINDOW_WIDTH + ((MAX_3D_WINDOW_WIDTH -
	 MIN_3D_WINDOW_WIDTH) * Size) / 100;
	Height = MIN_3D_WINDOW_HEIGHT + ((MAX_3D_WINDOW_HEIGHT -
	 MIN_3D_WINDOW_HEIGHT) * Size) / 100;

	/* Set dimensions in interface */
	I3DM.Window_3D_width		= Width;
	I3DM.Window_3D_height	= Height;

	/* Calculate new coordinates */
	Window_3D_X = (Screen_width - Width) / 2;
	Window_3D_Y = (Panel_Y - Height) / 2;

	/* Create new OPM */
	OPM_Del(&M3_OPM);
	OPM_CreateVirtualOPM
	(
		&Main_OPM,
		&M3_OPM,
		Window_3D_X,
		Window_3D_Y,
		Width,
		Height
	);

	/* Inform 3DM */
	Err = Set_3DM_WindowSize(Width, Height);
	if (Err)
	{
		Error_3DM(Err);
		Exit_program();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_shade_factor
 * FUNCTION  : Initialise shade factor.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 15:11
 * LAST      : 01.10.95 00:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_shade_factor(void)
{
	struct Lab_data *Lab_ptr;

	/* Get maximum shade factor */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Max_shade_factor = max(0, MAX_SHADE_FACTOR -
	 (SISHORT) Lab_ptr->Shade_factor);

	MEM_Free_pointer(Lab_data_handle);

	/* Calculate target shade factor and set current shade factor to target */
	Current_shade_factor = Get_target_shade_factor();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_target_shade_factor
 * FUNCTION  : Calculate target shade factor.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 15:12
 * LAST      : 01.10.95 00:05
 * INPUTS    : None.
 * RESULT    : SISHORT : Target shade factor.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SISHORT
Get_target_shade_factor(void)
{
	SISHORT Target_shade_factor = 0;
	SILONG Light_strength;
	SILONG F;

	/* Act depending on current light state */
	switch (Current_map_light_state)
	{
		/* Always light */
		case ALWAYS_LIGHT:
		{
			Target_shade_factor = Max_shade_factor;
			break;
		}
		/* Always dark */
		case ALWAYS_DARK:
		{
			/* Get light strength */
			Light_strength = (SILONG) min(max(Get_light_spell_strength(),
			 Calculate_party_light()), 100);

			/* Is the active member an Iskai ? */
			if (Get_race(Active_char_handle) == ISKAI_RACE)
			{
				/* Yes -> Add 25% */
				Light_strength = min(Light_strength + 25, 100);
			}

			/* Calculate target shade factor, using the following formula :

							 2
			           -x             F
			    y = ( ----- + x ) * ----
						  200           50

				y = Target shade factor (0 ... Maximum shade factor)
				x = Light strength (0 ... 100)
				F = Maximum shade factor

			 This creates a non-linear translation which compensates for the
			 3D distortion. A linear translation would be:

				y = ( F * x )
					  -------
						 100

			 */

			/* Compress maximum to 75% */
			F = (SILONG) Max_shade_factor;
			F = (F * 3) / 4;

			/* Calculate target shade factor */
			Target_shade_factor = (SISHORT) (((Light_strength * Light_strength) /
			 -200) + Light_strength) * (F / 50);

			/* Boost up by 25% */
			Target_shade_factor += (Max_shade_factor / 4);

			break;
		}
		/* Changing light */
		case CHANGING_LIGHT:
		{
			Target_shade_factor = Max_shade_factor;
			break;
		}
	}

	return Target_shade_factor;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_shade_factor
 * FUNCTION  : Update shade factor.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.08.95 15:13
 * LAST      : 02.09.95 15:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_shade_factor(void)
{
	SISHORT Delta;
	SISHORT Target_shade_factor;

	/* Calculate target shade factor */
	Target_shade_factor = Get_target_shade_factor();

	/* Calculate difference */
	Delta = Target_shade_factor - Current_shade_factor;

	/* Any difference ? */
	if (Delta)
	{
		/* Yes -> Change current shade factor */
		if (abs(Delta) < 20)
		{
			Current_shade_factor += Delta;
		}
		else
		{
			Current_shade_factor += sgn(Delta) * 20;
		}
	}
}

