/************
 * NAME     : 3D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 11.08.94 11:58
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>

#include <CONTROL.H>
#include <MAP.H>
#include <AUTOMAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <GAMETIME.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <3DM.H>
#include <XFTYPES.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <POPUP.H>
#include <GAMETEXT.H>
#include <MUSIC.H>
#include <EVELOGIC.H>

/* defines */

#define FULLTURN_STEPS		(16)

/* structure definitions */

struct Collision_3D {
	SISHORT dX, dY;
	UNSHORT Bit_mask;
};

/* prototypes */

/* 3D map module functions */
void M3_ModInit(void);
void M3_ModExit(void);
void M3_DisInit(void);
void M3_DisExit(void);
void M3_MainLoop(void);

void Render_3D_map(void);

BOOLEAN Init_3DM_library(void);

/* 3D map object methods */
UNLONG Touched_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Left_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Right_3D_object(struct Object *Object, union Method_parms *P);
UNLONG DLeft_3D_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_3D_object(struct Object *Object, union Method_parms *P);

void Error_3DM(UNSHORT Error_code);

void Rotate_3D(SISHORT dAlpha);
void Add_3D_vector(SILONG dX, SILONG dY);
void Set_3D_window_size(UNSHORT Width, UNSHORT Height);

/* 3D map move mode */
void Move_M3_MainLoop(void);
void Move_M3_ModInit(void);
void Move_M3_ModExit(void);
void Mouse_move_3D(void);

/* 3D map select mode */
void Select_M3_MainLoop(void);
void Select_M3_ModInit(void);
void Select_M3_ModExit(void);

void Interpret_3D_identification_data(void);
UNLONG Get_distance_to_selected_map_square(void);
BOOLEAN Check_line_to_selected_map_square(void);

UNSHORT Get_3D_mouse_state(SISHORT X, SISHORT Y);

/* 3D map movement / collision detection functions */
void Make_3D_party_move(SILONG dX, SILONG dY);

BOOLEAN Try_3D_move(SILONG X, SILONG Y, UNSHORT NPC_index,
 UNSHORT Travel_mode);

BOOLEAN Check_3D_object_group_collision(SISHORT Square_X, SISHORT Square_Z,
 SISHORT Group_X, SISHORT Group_Z, UNSHORT Travel_mode,
 struct Object_group_3D *Object_group, struct Object_3D *Object_base);

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

/* 3D map move mode module */
static struct Module Move_M3_Mod =
{
	NO_INPUT_HANDLING, MODE_MOD, MAP_3D_SCREEN,
	Move_M3_MainLoop,
	Move_M3_ModInit,
	Move_M3_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 3D map select mode module */
static struct Module Select_M3_Mod = {
	LOCAL_MOD | NO_INPUT_HANDLING, MODE_MOD, NO_SCREEN,
	Select_M3_MainLoop,
	Select_M3_ModInit,
	Select_M3_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 3D map window method list */
static struct Method M3_methods[] = {
	{ TOUCHED_METHOD, Touched_3D_object },
	{ LEFT_METHOD, Left_3D_object },
	{ RIGHT_METHOD, Right_3D_object },
	{ DLEFT_METHOD, DLeft_3D_object },
	{ CUSTOMKEY_METHOD, Customkeys_3D_object },
	{ 0, NULL}
};

/* 3D map window object class description */
static struct Object_class M3_Class = {
	0, sizeof(struct Object),
	&M3_methods[0]
};

static UNSHORT M3_Object;

static UNSHORT Mice_3D[] = {
	TURNLEFT_MPTR, FORWARD_MPTR, TURNRIGHT_MPTR,
	LEFT3D_MPTR, POP_UP_MPTR, RIGHT3D_MPTR,
	TURN180LEFT_MPTR, BACKWARD_MPTR, TURN180RIGHT_MPTR,
	DEFAULT_MPTR			// Outside of the map window
};

static UNSHORT Window_3D_X, Window_3D_Y;

static SISHORT Shade_offsets[4] = {-20, -1, -18, 0};

struct Interface_3DM I3DM;

static struct Identify_3D Identify_data;

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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_ModInit
 * FUNCTION  : Initialize 3D map module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 12:00
 * LAST      : 19.07.95 14:10
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

	/* Set dimensions in interface */
	I3DM.Window_3D_width = Screen_width;
	I3DM.Window_3D_height = Panel_Y;

	/* Calculate coordinates */
	Window_3D_X = (Screen_width - I3DM.Window_3D_width) / 2;
	Window_3D_Y = (Panel_Y - I3DM.Window_3D_height) / 2;

	/* Make OPM */
	OPM_CreateVirtualOPM(&Main_OPM, &M3_OPM, Window_3D_X, Window_3D_Y,
	 I3DM.Window_3D_width, I3DM.Window_3D_height);

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

	/* Initialize located sound effects */
	Map_square_size = I3DM.Square_size;

	Initialize_located_effects(Map_width * Map_square_size, Map_height *
	 Map_square_size, 12 * Map_square_size);

	/* Initialize party position */
	Initialize_3D_position();

	/* Initialize NPCs */
	Init_3D_NPCs();
	Display_3D_NPCs();
	I3DM.Nr_NPCs = Nr_3D_NPCs;

	/* Initialize automap */
	Init_automap();

	/* Update at initial position */
	Update_automap();

	/* Save position */
	Save_position();

	/* Handle first step */
	After_move();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_ModExit
 * FUNCTION  : Exit 3D map module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.08.94 15:40
 * LAST      : 19.07.95 14:13
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

	/* Exit automap */
	Exit_automap();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_DisInit
 * FUNCTION  : Initialize 3D map display.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 15:29
 * LAST      : 27.10.94 15:29
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
	Init_map_display();

	Add_update_OPM(&M3_OPM);

	/* Add interface object */
	M3_Object = Add_object(Earth_object, &M3_Class, NULL, Window_3D_X,
	 Window_3D_Y, I3DM.Window_3D_width, I3DM.Window_3D_height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_DisExit
 * FUNCTION  : Exit 3D map display.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 15:29
 * LAST      : 27.10.94 15:29
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
	/* Delete interface object */
	Delete_object(M3_Object);

	Remove_update_OPM(&M3_OPM);

	Exit_map_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_MainLoop
 * FUNCTION  : 3D map main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.09.94 14:14
 * LAST      : 05.09.94 14:14
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M3_MainLoop(void)
{
	/* Display map */
	Update_display();

	/* Update time */
	Update_time(Update_duration * 10);

	/* Update NPC positions */
	Handle_3D_NPCs();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_3D_map
 * FUNCTION  : Display the 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.08.94 10:15
 * LAST      : 17.07.95 16:03
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

	/* Put overlays on walls */
	Put_overlays_on_walls();

	/* Render 3D map */
	Render_3D_map();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Render_3D_map
 * FUNCTION  : Render the 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.07.95 16:11
 * LAST      : 17.07.95 16:11
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
	struct Wall_3D *Wall_ptrs[WALLS_MAX];
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

	/* Call 3DM */
	Err = Render_3DM_View();
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
	}
	M3_OPM.status |= OPMSTAT_CHANGED;

	/* Free pointers */
	MEM_Free_pointer(Background_gfx_handle);
	MEM_Free_pointer(Shade_table_handle);
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
 * LAST      : 19.07.95 14:05
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

	I3DM.Shade_factor 			= Lab_ptr->Shade_factor;

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

	/* Set 3DM window size */
	Err = Set_3DM_WindowSize(I3DM.Window_3D_width, I3DM.Window_3D_height);
	if (Err)
	{
		Error_3DM(Err);
		return FALSE;
	}

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
 * LAST      : 27.10.94 11:05
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
	Q = Get_3D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

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
 * LAST      : 27.10.94 11:05
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

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

	/* Is the mouse pointer inside the 3D map window ? */
	if (Q < 9)
	{
		/* Yes -> Enter move mode */
		Push_module(&Move_M3_Mod);
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
 * NAME      : DLeft_3D_object
 * FUNCTION  : DLeft method of 3D map object.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 11:05
 * LAST      : 27.10.94 11:05
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
DLeft_3D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Q, i;

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

	/* Act depending on state */
	switch (Q)
	{
		case 0:
			/* 90 degrees turn to the left */
			for (i=0;i<FULLTURN_STEPS/2-1;i++)
			{
				Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
				Update_display();
				Switch_screens();
			}
			Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
			break;
		case 2:
			/* 90 degrees turn to the right */
	  		for (i=0;i<FULLTURN_STEPS/2-1;i++)
			{
				Rotate_3D(-ANGLE_STEPS / (FULLTURN_STEPS * 2));
				Update_display();
				Switch_screens();
			}
			Rotate_3D(-ANGLE_STEPS / (FULLTURN_STEPS * 2));
			break;
		case 6:
			/* 180 degrees turn to the left */
	  		for (i=0;i<FULLTURN_STEPS-1;i++)
			{
				Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
				Update_display();
				Switch_screens();
			}
			Rotate_3D(ANGLE_STEPS / (FULLTURN_STEPS * 2));
			break;
		case 8:
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
 * LAST      : 20.06.95 12:58
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
		/* Exit program */
		case BLEV_ESC:
		{
			Quit_program = TRUE;
			break;
		}
		/* Move camera up */
		case BLEV_PGUP:
		{
			if (I3DM.Camera_height < I3DM.Wall_height-10)
				I3DM.Camera_height += 5;
			break;
		}
		/* Move camera down */
		case BLEV_PGDN:
		{
			if (I3DM.Camera_height > 10)
				I3DM.Camera_height -= 5;
			break;
		}
		/* Look up */
		case BLEV_UP:
		{
			if (I3DM.Horizon_Y < (I3DM.Window_3D_height / 2 - 8))
				I3DM.Horizon_Y += 8;
			break;
		}
		/* Look down */
		case BLEV_DOWN:
		{
			if (I3DM.Horizon_Y > (8 - (I3DM.Window_3D_height / 2)))
				I3DM.Horizon_Y -= 8;
			break;
		}
		/* Increase shade-factor */
		case BLEV_LEFT:
		{
			if (I3DM.Shade_factor < 4095)
				I3DM.Shade_factor += 1;
			break;
		}
		/* Decrease shade-factor */
		case BLEV_RIGHT:
		{
			if (I3DM.Shade_factor > 0)
				I3DM.Shade_factor -= 1;
			break;
		}
		/* Decrease 3D window size */
		case BLEV_INS:
		{
			if ((I3DM.Window_3D_width > MIN_WINDOW_WIDTH) &&
			 (I3DM.Window_3D_height > MIN_WINDOW_HEIGHT))
			{
				UNSHORT w,h;

				w = I3DM.Window_3D_width - 15;
				h = I3DM.Window_3D_height - 8;

				if ((w >= MIN_WINDOW_WIDTH) && (h >= MIN_WINDOW_HEIGHT))
				{
					Set_3D_window_size(w, h);
				}
			}
			break;
		}
		/* Increase 3D window size */
		case BLEV_DEL:
		{
			if ((I3DM.Window_3D_width < Screen_width) &&
			 (I3DM.Window_3D_height < Panel_Y))
			{
				UNSHORT w,h;

				w = I3DM.Window_3D_width + 15;
				h = I3DM.Window_3D_height + 8;

				if ((w <= Screen_width) && (h <= Panel_Y))
				{
					Set_3D_window_size(w, h);
				}
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
 * NAME      : Initialize_3D_position
 * FUNCTION  : Set the position in the 3D dungeon.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 14:15
 * LAST      : 07.09.94 14:15
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Initialize_3D_position(void)
{
	UNLONG X, Y;

	/* Convert current party coordinates to dungeon coordinates */
	Map_to_dungeon((SISHORT) PARTY_DATA.X, (SISHORT) PARTY_DATA.Y, &X, &Y);

	/* Store in 3D interface */
	*((UNLONG *) &(I3DM.Player_X[1])) = X;
	*((UNLONG *) &(I3DM.Player_Z[1])) = Y;

	/* Clear after the decimal point */
	I3DM.Player_X[0] = 0;
	I3DM.Player_Z[0] = 0;

	/* Set camera angle */
	I3DM.Camera_angle = ((0 - (PARTY_DATA.View_direction * (ANGLE_STEPS / 4)))
	 & (ANGLE_STEPS - 1)) * 65536;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Rotate_3D
 * FUNCTION  : Change the camera angle in the 3D dungeon.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 14:22
 * LAST      : 07.09.94 14:22
 * INPUTS    : SISHORT dAlpha - Change of the angle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Rotate_3D(SISHORT dAlpha)
{
	I3DM.Camera_angle += (dAlpha * 65536);
	I3DM.Camera_angle &= (ANGLE_STEPS * 65536) - 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_3D_vector
 * FUNCTION  : Move along a vector in the 3D dungeon.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 14:23
 * LAST      : 07.09.94 14:23
 * INPUTS    : SILONG dX - X-component of vector.
 *             SILONG dY - Y-component of vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_3D_vector(SILONG dX, SILONG dY)
{
	UNLONG X,Y;
	SILONG fracX, fracY;

	/* Get integer part of coordinates */
	X = * ((UNLONG *) &(I3DM.Player_X[1]));
	Y = * ((UNLONG *) &(I3DM.Player_Z[1]));

	/* Get fractional part of coordinates */
	fracX = (SILONG) I3DM.Player_X[0];
	fracY = (SILONG) I3DM.Player_Z[0];

	/* Add vector to fractional part */
	fracX += dX;
	fracY += dY;

	/* Carry over to integer part */
	while (fracX >= 65536)
	{
		fracX -= 65536;
		X++;
	}
	while (fracY >= 65536)
	{
		fracY -= 65536;
		Y++;
	}

	while (fracX < 0)
	{
		fracX += 65536;
		X--;
	}
	while (fracY < 0)
	{
		fracY += 65536;
		Y--;
	}

	/* Store new coordinates */
	* ((UNLONG *) &(I3DM.Player_X[1])) = X;
	* ((UNLONG *) &(I3DM.Player_Z[1])) = Y;

	I3DM.Player_X[0] = fracX;
	I3DM.Player_Z[0] = fracY;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_3D_window_size
 * FUNCTION  : Set 3D window size.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.09.94 16:46
 * LAST      : 16.09.94 16:46
 * INPUTS    : UNSHORT Width - New window width.
 *             UNSHORT Height - New window height.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_3D_window_size(UNSHORT Width, UNSHORT Height)
{
	UNSHORT Err;

	/* Set dimensions in interface */
	I3DM.Window_3D_width = Width;
	I3DM.Window_3D_height = Height;

	/* Calculate new coordinates */
	Window_3D_X = (Screen_width - Width) / 2;
	Window_3D_Y = (Panel_Y - Height) / 2;

	/* Create new OPM */
	OPM_Del(&M3_OPM);
	OPM_CreateVirtualOPM(&Main_OPM, &M3_OPM, Window_3D_X, Window_3D_Y,
	 Width, Height);

	/* Re-draw slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, min(Panel_Y, 192), 0, 0);

	/* Window less than full screen ? */
	if ((Width < Screen_width) && (Height < Panel_Y))
	{
		/* Yes -> Draw border around 3D window */
		Draw_deep_border(&Main_OPM, Window_3D_X - 1, Window_3D_Y - 1,
		 Width + 2, Height + 2);
	}

	/* Inform 3DM */
	Err = Set_3DM_WindowSize(Width, Height);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_M3_MainLoop
 * FUNCTION  : 3D map movement main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:29
 * LAST      : 27.10.94 10:29
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_M3_MainLoop(void)
{
	/* Update display */
	Update_display();

	/* Update time */
	Update_time(Update_duration * 10);

	/* Update input handling */
	Update_input();

	/* Is the left mouse button pressed ? */
	if (Button_state & 0x0001)
	{
		/* Yes -> Update NPC positions */
		Handle_3D_NPCs();

		/* Move */
		Mouse_move_3D();
	}
	else
	{
		/* No -> Exit move mode */
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mouse_move_3D
 * FUNCTION  : Move in 3D move mode.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:32
 * LAST      : 27.10.94 10:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Mouse_move_3D(void)
{
	SILONG F;
	SISHORT F2;
	SISHORT X, Y;
	UNSHORT W, H;
	UNSHORT W3, H3;
	UNSHORT W6, H6;
	UNSHORT Q;

	/* Get mouse coordinates */
	X = Mouse_X;
	Y = Mouse_Y;

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state(X, Y);

	/* Change mouse-pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

	/* Calculate conversion factor (2-log) */
	{
		struct Lab_data *Lab_ptr;

		Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

		F2 = 1 << (10 - Lab_ptr->Square_size_cm_log);

		MEM_Free_pointer(Lab_data_handle);
	}

	/* Adjust mouse coordinates */
	X -= Window_3D_X;
	Y -= Window_3D_Y;

	/* Get dimensions of display window */
	W = I3DM.Window_3D_width;
	H = I3DM.Window_3D_height;
	W3 = W / 3;
	H3 = H / 3;
	W6 = W3 / 2;
	H6 = H3 / 2;

	/* Move depending on state */
	switch (Q)
	{
		case 0:
			F = (18 * Update_duration * (W3 - X)) / W6;
			Rotate_3D(F);
			if (Y < 2 * (H3 / 3))
			{
				F = (150 * Update_duration * ((H3 - Y) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_party_move(-F * yawSIN, F * yawCOS);
			}
			break;
		case 1:
			F = (150 * Update_duration * (H3 - Y)) / H6;
			F /= F2;
			Make_3D_party_move(-F * yawSIN, F * yawCOS);
			break;
		case 2:
			F = (18 * Update_duration * (X - 2 * W3)) / W6;
			Rotate_3D(-F);
			if (Y < 2 * (H3 / 3))
			{
				F = (150 * Update_duration * ((H3 - Y) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_party_move(-F * yawSIN, F * yawCOS);
			}
			break;
		case 3:
			F = (100 * Update_duration * (W3 - X)) / W6;
			F /= F2;
			Make_3D_party_move(-F * yawCOS, -F * yawSIN);
			break;
		case 4:
			break;
		case 5:
			F = (100 * Update_duration * (X - 2 * W3)) / W6;
			F /= F2;
			Make_3D_party_move(F * yawCOS, F * yawSIN);
			break;
		case 6:
			F = (18 * Update_duration * (W3 - X)) / W6;
			Rotate_3D(F);
			if (Y > 2 * H3 + (H3 / 3))
			{
				F = (100 * Update_duration * ((Y - 2 * H3) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_party_move(F * yawSIN, -F * yawCOS);
			}
			break;
		case 7:
			F = (120 * Update_duration * (Y - 2 * H3)) / H6;
			F /= F2;
			Make_3D_party_move(F * yawSIN, -F * yawCOS);
			break;
		case 8:
			F = (18 * Update_duration * (X - 2 * W3)) / W6;
			Rotate_3D(-F);
			if (Y > 2 * H3 + (H3 / 3))
			{
				F = (100 * Update_duration * ((Y - 2 * H3) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_party_move(F * yawSIN, -F * yawCOS);
			}
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_M3_ModInit
 * FUNCTION  : Initialize 3D map move mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:59
 * LAST      : 27.10.94 10:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_M3_ModInit(void)
{
	struct BBRECT MA;

	/* Create mouse area */
	MA.left = Window_3D_X;
	MA.top = Window_3D_Y;
	MA.width = I3DM.Window_3D_width;
	MA.height = I3DM.Window_3D_height;

	/* Install mouse area */
	Push_MA(&MA);

	/* Indicate move mode is on */
	Move_mode_flag = TRUE;

	/* Guarantee first movement */
	Mouse_move_3D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_M3_ModExit
 * FUNCTION  : Exit 3D map move mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 11:00
 * LAST      : 27.10.94 11:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_M3_ModExit(void)
{
	/* Remove mouse area */
	Pop_MA();

	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M3_MainLoop
 * FUNCTION  : 3D map selection mode main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 14:05
 * LAST      : 20.07.95 20:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M3_MainLoop(void)
{
	/* Fill identify data */
	Identify_data.testColor = 193;
	Identify_data.screenX = Mouse_X - Window_3D_X;
	Identify_data.screenY = Mouse_Y - Window_3D_Y;

	/* Call identification function */
	Identify_3DM(&Identify_data);

	/* Update display (thus rendering the 3D map) */
	Update_display();

	/* Update input handling */
	Update_input();

	/* Is the right mouse button pressed ? */
	if (Button_state & 0x0010)
	{
		/* Yes -> Interpret identification data */
		Interpret_3D_identification_data();

		/* Act depending on selection state */
		switch (Current_map_selection_data.State)
		{
			case NO_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[NO_ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD, NULL);

				break;
			}
			case A_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD, NULL);

				break;
			}
			case OUT_OF_RANGE_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[532]);

				break;
			}
			case OUT_OF_SPEAK_RANGE_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[533]);

				break;
			}
			case OUT_OF_TOUCH_RANGE_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[534]);

				break;
			}
			case BLOCKED_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[535]);

				break;
			}
		}
	}
	else
	{
		/* No -> Exit select mode */
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M3_ModInit
 * FUNCTION  : Initialize 3D map select mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 16:09
 * LAST      : 20.07.95 16:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M3_ModInit(void)
{
	struct BBRECT MA;

	/* Create mouse area */
	MA.left = Window_3D_X;
	MA.top = Window_3D_Y;
	MA.width = I3DM.Window_3D_width;
	MA.height = I3DM.Window_3D_height;

	/* Install mouse area */
	Push_MA(&MA);

	/* Fill identify data */
	Identify_data.testColor = 193;
	Identify_data.screenX = Mouse_X - Window_3D_X;
	Identify_data.screenY = Mouse_Y - Window_3D_Y;

	/* Call identification function */
	Identify_3DM(&Identify_data);

	/* "Render" the 3D map */
	Render_3D_map();

	/* Interpret identification data */
	Interpret_3D_identification_data();

	/* Any action possible? */
	if (Current_map_selection_data.State != NO_MAP_SELECTION)
	{
		/* Yes -> Show */
		Push_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));
	}
	else
	{
		/* No -> Show */
		Push_mouse(&(Mouse_pointers[NO_ACTION_3D_MPTR]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M3_ModExit
 * FUNCTION  : Exit 3D map select mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 16:09
 * LAST      : 20.07.95 16:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M3_ModExit(void)
{
	/* Remove mouse */
	Pop_mouse();

	/* Remove mouse area */
	Pop_MA();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Interpret_3D_identifiation_data
 * FUNCTION  : Interpret 3D identification data.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 20:30
 * LAST      : 22.07.95 15:28
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The result of the interpretation can be found in
 *              Current_map_selection_data.State.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Interpret_3D_identification_data(void)
{
	UNLONG Distance = 0;
	UNSHORT State = NO_MAP_SELECTION;
	UNSHORT NPC_index = 0xFFFF;
	UNSHORT Trigger_modes = 0;

	/* Store found coordinates */
	/* (+ correction for 3DM bug) */
	Current_map_selection_data.Map_X = Identify_data.mapX + 1;
	Current_map_selection_data.Map_Y = Identify_data.mapY + 1;

	/* Act depending on found type */
	switch (Identify_data.type)
	{
		/* Nothing */
		case T_NOTHING:
		{
			break;
		}
		/* Object, wall, floor or ceiling */
		case T_OBJECT:
		case T_WALL:
		case T_SKY:
		{
			/* Get trigger modes for this position */
			Trigger_modes = Get_event_triggers(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF);

			break;
		}
		/* NPC */
		case T_NPC:
		{
			/* Get NPC index */
			NPC_index = Search_3D_NPC(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF);

			if (NPC_index == 0xFFFF)
				break;

			/* Has event ? */
			if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
			{
				/* Yes -> Get triggermodes */
				Trigger_modes = VNPCs[NPC_index].Trigger_modes;
			}
			else
			{
				/* No -> Can this NPC be talked to ? */
				if ((VNPCs[NPC_index].NPC_type != MONSTER_NPC) &&
				 (VNPCs[NPC_index].NPC_type != OBJECT_NPC))
				{
					/* Yes -> Use standard "triggermodes" */
					Trigger_modes = (1 << SPEAK_TRIGGER);
				}
				else
				{
					/* No -> Ignore this NPC */
					NPC_index = 0xFFFF;
				}
			}
			break;
		}
	}

	/* Determine distance between party and selected map square */
	Distance = Get_distance_to_selected_map_square();

	/* Is examining possible / is this too far away for examining ? */
	if (Trigger_modes & (1 << EXAMINE_TRIGGER))
	{
		if (Distance > MAX_EXAMINE_DISTANCE)
		{
			/* Yes -> Set state */
			State = OUT_OF_RANGE_MAP_SELECTION;
		}
		else
		{
			/* No */
			State = A_MAP_SELECTION;
		}
	}

	if (State != OUT_OF_RANGE_MAP_SELECTION)
	{
		/* Is talking possible ? */
		if (Trigger_modes & (1 << SPEAK_TRIGGER))
		{
			/* Yes -> Is this too far away for talking ? */
			if (Distance > MAX_TALK_DISTANCE)
			{
				/* Yes -> Set state */
				State = OUT_OF_SPEAK_RANGE_MAP_SELECTION;
			}
			else
			{
				/* No */
				State = A_MAP_SELECTION;
			}
		}
	}

	if ((State != OUT_OF_RANGE_MAP_SELECTION) &&
	 (State != OUT_OF_SPEAK_RANGE_MAP_SELECTION))
	{
		/* Is manipulating possible ? */
		if (Trigger_modes & ((1 << TOUCH_TRIGGER) | (1 << USE_ITEM_TRIGGER) |
		 (1 << TAKE_TRIGGER)))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if (Distance > MAX_MANIPULATE_DISTANCE)
			{
				/* Yes -> Set state */
				State = OUT_OF_TOUCH_RANGE_MAP_SELECTION;
			}
			else
			{
				/* No -> Check the line between the party and the selected map
					square */
				if (Check_line_to_selected_map_square())
				{
					/* Blocked */
					State = BLOCKED_MAP_SELECTION;
				}
				else
				{
					/* Not blocked */
					State = A_MAP_SELECTION;
				}
			}
		}
	}

	/* Store selection data */
	Current_map_selection_data.State				= State;
	Current_map_selection_data.NPC_index		= NPC_index;
	Current_map_selection_data.Trigger_modes	= Trigger_modes;
	Current_map_selection_data.Distance			= Distance;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_distance_to_selected_map_square
 * FUNCTION  : Calculate_distance between party and selected map square.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.95 14:29
 * LAST      : 21.07.95 14:29
 * INPUTS    : None.
 * RESULT    : UNLONG : Distance to selected map square in 3D units.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_distance_to_selected_map_square(void)
{
	UNLONG Distance;
	SISHORT dX, dY;

	/* Determine distance between party and selected map square */
	dX = Current_map_selection_data.Map_X - PARTY_DATA.X;
	dY = Current_map_selection_data.Map_Y - PARTY_DATA.Y;

	Distance = (dX * dX) + (dY * dY);
	Distance = (UNLONG) sqrt((double) Distance);

	Distance *= Map_square_size;

	return Distance;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_line_to_selected_map_square
 * FUNCTION  : Check the line between party and selected map square.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.95 15:21
 * LAST      : 22.07.95 15:21
 * INPUTS    : None.
 * RESULT    : BOOLEAN : The line is blocked.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_line_to_selected_map_square(void)
{
	BOOLEAN Result = FALSE;
	UNLONG Status;
	SISHORT dX, dY;
	SISHORT Sign_dX, Sign_dY;
	UNSHORT X, Y;
	UNSHORT Var;
	UNSHORT i;

	/* Get selected map square coordinates */
	X = Current_map_selection_data.Map_X;
	Y = Current_map_selection_data.Map_Y;

	/* Calculate vector */
	dX = PARTY_DATA.X - X;
	dY = PARTY_DATA.Y - Y;

	/* Same location as party ? */
	if (!dX && !dY)
	{
		/* Yes -> Not blocked */
		return FALSE;
	}

	/* Calculate vector direction */
	Sign_dX = sgn(dX);
	Sign_dY = sgn(dY);

	/* Calculate absolute vector components */
	dX = abs(dX);
	dY = abs(dY);

	/* Which way does the line go ? */
	if (dX > dY)
	{
		/* Initialize Bresenham variable */
		Var = dX / 2;

		/* Check line */
		for (i=0;i<dX - 1;i++)
		{
			/* Trace line */
			Var += dY;
			if (Var > dX)
			{
				Var -= dX;
				Y += Sign_dY;
			}
			X += Sign_dX;

			/* Blocked ? */
			Status = Get_location_status(X, Y, 0xFFFF);
			if (Status & WALL_VISION_BLOCKED)
			{
				/* Yes -> Break */
				Result = TRUE;
				break;
			}
		}
	}
	else
	{
		/* Initialize Bresenham variable */
		Var = dY / 2;

		/* Check line */
		for (i=0;i<dY - 1;i++)
		{
			/* Trace line */
			Var += dX;
			if (Var > dY)
			{
				Var -= dY;
				X += Sign_dX;
			}
			Y += Sign_dY;

			/* Blocked ? */
			Status = Get_location_status(X, Y, 0xFFFF);
			if (Status & WALL_VISION_BLOCKED)
			{
				/* Yes -> Break */
				Result = TRUE;
				break;
			}
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_3D_mouse_state
 * FUNCTION  : Get state of mouse in 3D map window.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.09.94 11:24
 * LAST      : 21.09.94 11:24
 * INPUTS    : SISHORT X - X-coordinate of mouse
 *             SISHORT Y - Y-coordinate of mouse
 * RESULT    : UNSHORT : Mouse state (0...8 / 9 = out of window)
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_3D_mouse_state(SISHORT X, SISHORT Y)
{
	UNSHORT W, H;
	UNSHORT W3, H3;
	UNSHORT Q;

	/* Translate coordinates */
	X -= Window_3D_X;
	Y -= Window_3D_Y;

	/* Get dimensions of display window */
	W = I3DM.Window_3D_width;
	H = I3DM.Window_3D_height;
	W3 = W / 3;
	H3 = H / 3;

	/* Exit if the mouse pointer is outside of the 3D map window */
	if ((X <0) || (X >= W) || (Y < 0) || (Y >= H))
		return 9;

	/* Determine state depending on position in map window */
	Q = 0;
	if (X >= W3)
	{
		if (X < W - W3)
			Q = 1;
		else
			Q = 2;
	}
	if (Y >= H3)
	{
		if (Y < H - H3)
			Q += 3;
		else
			Q += 6;
	}

	return Q;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_3D_party_move
 * FUNCTION  : The party makes a move in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:23
 * LAST      : 23.07.95 16:31
 * INPUTS    : SILONG dX - X-component of movement vector.
 *             SILONG dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The movement vector is multiplied by 65536.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_3D_party_move(SILONG dX, SILONG dY)
{
	static struct Movement_3D Party_3D_move_data = {
		0, 0,
		0, 0,
		0,
		0xFFFF
	};

	BOOLEAN Result;
	SISHORT Map_X, Map_Y;

	/* Clear flags */
	Moved = FALSE;
	Bumped = FALSE;

	/* Allowed by before-move logic ? */
	if (Before_move())
	{
		/* Yes -> Try to move */
		Party_3D_move_data.X					= * ((UNLONG *) &(I3DM.Player_X[1]));
		Party_3D_move_data.Y					= * ((UNLONG *) &(I3DM.Player_Z[1]));
		/* (I don't use shifts because dX and dY are signed.) */
		Party_3D_move_data.dX				= dX / 65536;
		Party_3D_move_data.dY				= dY / 65536;
		Party_3D_move_data.Travel_mode	= PARTY_DATA.Travel_mode;

		Result = Make_3D_move(&Party_3D_move_data);

		/* Did it work ? */
		if (Result)
		{
			/* Yes -> Set flag */
			Moved = TRUE;

			/* Move */
			if (!Party_3D_move_data.dX)
				dX = 0;
			if (!Party_3D_move_data.dY)
				dY = 0;
			Add_3D_vector(dX, dY);

			/* Calculate new map coordinates */
			Dungeon_to_map(*((UNLONG *) &(I3DM.Player_X[1])),
			 *((UNLONG *) &(I3DM.Player_Z[1])), &Map_X, &Map_Y);

			/* Any change ? */
			if ((PARTY_DATA.X != Map_X) || (PARTY_DATA.Y != Map_Y))
			{
				/* Yes -> Store new coordinates */
				PARTY_DATA.X = Map_X;
				PARTY_DATA.Y = Map_Y;

				/* Update */
				Update_automap();

				After_move();
			}
		}
		else
		{
			/* No */
			Bumped = TRUE;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_3D_move
 * FUNCTION  : Make a move in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:23
 * LAST      : 23.07.95 16:11
 * INPUTS    : struct Movement_2D *Move - Pointer to movement structure.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Make_3D_move(struct Movement_3D *Move)
{
	BOOLEAN Result;
	SILONG dX, dY;

	/* Get movement vector */
	dX = Move->dX;
	dY = Move->dY;

	/* Try to move */
	Result = Try_3D_move(Move->X + dX, Move->Y + dY, Move->NPC_index,
	 Move->Travel_mode);

	/* Did it work ? */
	if (!Result)
	{
		/* No -> Slide (first choice) */
		if (Move->dX > Move->dY)
		{
			dY = 0;

			Result = Try_3D_move(Move->X + dX, Move->Y + dY, Move->NPC_index,
			 Move->Travel_mode);
		}
		else
		{
			dX = 0;

			Result = Try_3D_move(Move->X + dX, Move->Y + dY, Move->NPC_index,
			 Move->Travel_mode);
		}

		/* Did it work ? */
		if (!Result)
		{
			/* No -> Slide (second choice) */
			if (Move->dX > Move->dY)
			{
				dX = 0;
				dY = Move->dY;

				Result = Try_3D_move(Move->X + dX, Move->Y + dY, Move->NPC_index,
				 Move->Travel_mode);
			}
			else
			{
				dX = Move->dX;
				dY = 0;

				Result = Try_3D_move(Move->X + dX, Move->Y + dY, Move->NPC_index,
				 Move->Travel_mode);
			}
		}
	}

	/* Did it work ? */
	if (Result)
	{
		/* Yes -> Update coordinates */
		Move->X += dX;
		Move->Y += dY;

		/* Store actual movement vector */
		Move->dX = dX;
		Move->dY = dY;
	}
	else
	{
		/* No -> Clear movement vector */
		Move->dX = 0;
		Move->dY = 0;
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_3D_move
 * FUNCTION  : Check if a certain move can be made in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:26
 * LAST      : 22.07.95 17:35
 * INPUTS    : SILONG X - New X-coordinate (dungeon coordinates).
 *             SILONG Y - New Y-coordinate (dungeon coordinates).
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 *             UNSHORT Travel_mode - Travel mode.
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_3D_move(SILONG X, SILONG Y, UNSHORT NPC_index, UNSHORT Travel_mode)
{
/*	static struct Collision_3D Collision_table[8] = {
		{ -1, 1, 0x0200 },
		{  0, 1, 0x0007 },
		{  1, 1, 0x0400 },
		{ -1, 0, 0x0049 },
		{  1, 0, 0x0124 },
		{ -1,-1, 0x0800 },
		{  0,-1, 0x01C0 },
		{  1,-1, 0x1000 }
	}; */

/*	static struct Collision_3D Collision_table[8] = {
		{ -1,-1, 0x0001 },
		{  0,-1, 0x0007 },
		{  1,-1, 0x0004 },
		{  1, 0, 0x0124 },
		{  1, 1, 0x0100 },
		{  0, 1, 0x0220 },
		{ -1, 1, 0x0040 },
		{ -1, 0, 0x0049 }
	}; */

	static struct Collision_3D Collision_table[8] = {
		{ -1,-1, 0x0001 },
		{  0,-1, 0x0007 },
		{  1,-1, 0x0004 },
		{ -1, 0, 0x0048 },
		{  1, 0, 0x0124 },
		{ -1, 1, 0x0040 },
		{  0, 1, 0x01C0 },
		{  1, 1, 0x0100 }
	};

	struct Object_group_3D *Object_group_base;
	struct Object_group_3D *Object_group_ptr;
	struct Object_3D *Object_base;
	struct Square_3D *Square;
	struct Lab_data *Lab_ptr;
	BOOLEAN Result;
	SILONG Group_X, Group_Z;
	SISHORT Map_X, Map_Y;
	UNSHORT f;
	UNSHORT Edge;
	UNSHORT c;
	UNSHORT i;
	UNSHORT Q;
	UNSHORT Bitlist;
	UNSHORT Square_X, Square_Z;
	UNBYTE *Ptr;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Calculate map coordinates */
	Dungeon_to_map(X, Y, &Map_X, &Map_Y);

	/* Exit if target coordinates lie outside the map */
	if ((Map_X < 1) || (Map_X > Map_width) || (Map_Y < 1) ||
	 (Map_Y > Map_height))
		return FALSE;

	/* Exit if cheat mode is on / not NPC */
	if ((NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	/* Calculate coordinates inside current map square */
	Square_X = X & (f - 1);
	Square_Z = Y & (f - 1);

	/* Check the destination map square */
	Result = Movement_check_3D(Map_X, Map_Y, NPC_index, Travel_mode);

	/* Collision ? */
	if (Result)
	{
		/* No -> Try edges */
		/* Build edge collision list */
		Bitlist = 0;
		for (i=0;i<8;i++)
		{
			if (!Movement_check_3D(Map_X + Collision_table[i].dX,
			 Map_Y - Collision_table[i].dY, NPC_index, Travel_mode))
			{
				Bitlist |= Collision_table[i].Bit_mask;
			}
		}

		/* Determine edge width */
		Edge = f / 4;
		//Edge = min((f / 4), 50);

		/* Determine current edge area */
		Q = 0;
		if (Square_X >= Edge)
		{
			if (Square_X < f - Edge)
				Q = 1;
			else
				Q = 2;
		}
		if (Square_Z >= Edge)
		{
			if (Square_Z < f - Edge)
				Q += 3;
			else
				Q += 6;
		}

		/* Blocked ? */
		if (Bitlist & (1 << Q))
		{
			/* Yes -> Collision */
			Result = FALSE;
		}
		else
		{
			/* No -> Get pointers to various parts of the lab-data */
			Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

			Ptr = (UNBYTE *) (Lab_ptr + 1);

			Object_group_base = (struct Object_group_3D *) Ptr;

			Ptr += (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;
			Ptr += (Nr_of_floors * sizeof(struct Floor_3D)) + 2;

			Object_base = (struct Object_3D *) Ptr;

			/* Get target square data */
			Square = (struct Square_3D *) (MEM_Claim_pointer(Map_handle)
			 + Map_layers_offset) + ((Map_Y - 1) * Map_width) + Map_X - 1;

			/* Get entry from wall layer */
			c = (UNSHORT) Square->Wall_layer;

			/* Object group ? */
			if (c && (c < FIRST_WALL))
			{
				/* Yes -> Collision ? */
				Object_group_ptr = &(Object_group_base[c-1]);
				if (Check_3D_object_group_collision(Square_X, Square_Z, 0, 0,
				 Travel_mode, Object_group_ptr, Object_base))
				{
					/* Yes -> Collision */
					Result = FALSE;
				}
			}

			MEM_Free_pointer(Map_handle);

			/* Collision ? */
			if (Result)
			{
				/* No -> Is NPC ? */
				if (NPC_index != 0xFFFF)
				{
					/* Yes -> Check for collision with party */
					/* Determine distance from party to NPC */
					Group_X = *((UNLONG *) &(I3DM.Player_X[1])) - X;
					Group_Z = *((UNLONG *) &(I3DM.Player_Z[1])) - Y;

					/* Too large ? */
					if ((Group_X > 0 - f) && (Group_X < f) && (Group_Z > 0 - f)
					 && (Group_Z < f))
					{
						/* No -> Check for collision */
						Object_group_ptr =
						 &(Object_group_base[VNPCs[NPC_index].Graphics_nr - 1]);

						if (Check_3D_object_group_collision(Square_X, Square_Z,
						 (SISHORT) Group_X, (SISHORT) Group_Z, Travel_mode,
						 Object_group_ptr, Object_base))
						{
							/* Collision */
							Result = FALSE;
						}
					}
				}

				/* Collision ? */
				if (Result)
				{
					/* No -> Check for collision with NPCs */
					for (i=0;i<NPCS_PER_MAP;i++)
					{
						/* Anyone there / not checking myself ? */
						if ((NPC_present(i)) && (i != NPC_index))
						{
							/* Yes -> Determine distance to NPC */
							Group_X = VNPCs[i].Source_X - X;
							Group_Z = VNPCs[i].Source_Y - Y;

							/* Too large ? */
							if ((Group_X > 0 - f) && (Group_X < f) &&
							 (Group_Z > 0 - f) && (Group_Z < f))
							{
								/* No -> Check for collision */
								Object_group_ptr =
								 &(Object_group_base[VNPCs[i].Graphics_nr - 1]);

								if (Check_3D_object_group_collision(Square_X,
								 Square_Z, (SISHORT) Group_X, (SISHORT) Group_Z,
								 Travel_mode, Object_group_ptr, Object_base))
								{
									/* Collision */
									Result = FALSE;
									break;
								}
							}
						}
					}
				}
			}

			MEM_Free_pointer(Lab_data_handle);
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Movement_check_3D
 * FUNCTION  : Check if a certain position in a 3D map can be moved to.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 15:32
 * LAST      : 23.07.95 14:13
 * INPUTS    : SISHORT Map_X - Map X-coordinate.
 *             SISHORT Map_Y - Map Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 *             UNSHORT Travel_mode - Travel mode.
 * RESULT    : BOOLEAN : TRUE (not blocked) or FALSE (blocked).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Movement_check_3D(SISHORT Map_X, SISHORT Map_Y, UNSHORT NPC_index,
 UNSHORT Travel_mode)
{
	struct Square_3D *Square;
	struct Lab_data *Lab_ptr;
	struct Floor_3D *Floor_ptr;
	BOOLEAN Result = TRUE;
	UNLONG Status;
	UNSHORT c;
	UNBYTE *Ptr;

	/* Exit if target coordinates lie outside the map */
	if ((Map_X < 1) || (Map_X > Map_width) || (Map_Y < 1) ||
	 (Map_Y > Map_height))
		return FALSE;

	/* Get pointers to various parts of the lab-data */
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);
	Ptr = (UNBYTE *) (Lab_ptr + 1);
	Ptr += (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;
	Floor_ptr = (struct Floor_3D *) Ptr;

	/* Get target square data */
	Square = (struct Square_3D *) (MEM_Claim_pointer(Map_handle)
	 + Map_layers_offset) + ((Map_Y - 1) * Map_width) + Map_X - 1;

	/* Try wall */
	c = (UNSHORT) Square->Wall_layer;

	/* Any wall ? */
	if (c >= FIRST_WALL)
	{
		/* Yes -> Get wall status bits */
		Status = ((struct Wall_3D *) ((UNLONG) Lab_ptr +
		 Wall_offsets[c-FIRST_WALL]))->Flags;

		/* Check bits for current travelmode */
		/* Blocked ? */
		if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
		{
			/* Yes */
			Result = FALSE;
		}
	}

	/* Blocked ? */
	if (Result)
	{
		/* No -> Try floor */
		c = Square->Floor_layer;

		/* Any floor ? */
		if (c)
		{
			/* Yes -> Get floor status bits */
			Status = Floor_ptr[c-1].Flags;

			/* Check bits for current travelmode */
			/* Blocked ? */
			if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
			{
				/* Yes */
				Result = FALSE;
			}
		}
	}

	/* Blocked ? */
	if (Result)
	{
		/* No -> Try ceiling */
		c = Square->Ceiling_layer;

		/* Any ceiling ? */
		if (c)
		{
			/* Yes -> Get ceiling status bits */
			Status = Floor_ptr[c-1].Flags;

			/* Check bits for current travelmode */
			/* Blocked ? */
			if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
			{
				/* Yes */
				Result = FALSE;
			}
		}
	}

	MEM_Free_pointer(Map_handle);
	MEM_Free_pointer(Lab_data_handle);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_3D_object_group_collision
 * FUNCTION  : Check if a certain position in a 3D map causes collision with
 *              an object group.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 10:03
 * LAST      : 07.02.95 10:03
 * INPUTS    : UNSHORT Square_X - X-coordinate inside map square.
 *             UNSHORT Square_Z - Z-coordinate inside map square.
 *             SISHORT Group_X - X-offset of object group.
 *             SISHORT Group_Z - Z-offset of object group.
 *             UNSHORT Travel_mode - Travel mode.
 *             struct Object_group_3D *Object_group - Pointer to object group.
 *             struct Object_3D *Object_base - Pointer to start of object data.
 * RESULT    : BOOLEAN : TRUE (collision) or FALSE (no collision).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_3D_object_group_collision(SISHORT Square_X, SISHORT Square_Z,
 SISHORT Group_X, SISHORT Group_Z, UNSHORT Travel_mode,
 struct Object_group_3D *Object_group, struct Object_3D *Object_base)
{
	struct Object_3D *Object_ptr;
	BOOLEAN Result = FALSE;
	SISHORT Object_X, Object_Z;
	UNSHORT Object_nr, W;
	UNSHORT i;

	/* Check all objects */
	for (i=0;i<OBJECTS_PER_GROUP;i++)
	{
		/* Get object index */
		Object_nr = Object_group->Group[i].Number;

		/* Anything there ? */
		if (Object_nr)
		{
			/* Yes -> Get pointer to data */
			Object_ptr = &(Object_base[Object_nr - 1]);

			/* Blocking ? */
			if (Object_ptr->Flags &
			 (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
			{
				/* Yes -> Get object coordinates */
				Object_X = Object_group->Group[i].X + Group_X;
				Object_Z = Object_group->Group[i].Z + Group_Z;

				/* Get object width */
				W = Object_ptr->Object_width / 2;

				/* Target coordinates inside object ? */
				if ((Square_X >= Object_X - W) && (Square_Z >= Object_Z - W) &&
				 (Square_X < Object_X + W) && (Square_Z < Object_Z + W))
				{
					/* Yes -> Collision */
					Result = TRUE;
					break;
				}
			}
		}
	}

	return (Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Map_to_dungeon
 * FUNCTION  : Convert map to dungeon coordinates.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 11:59
 * LAST      : 11.07.95 15:26
 * INPUTS    : SISHORT Map_X - Map X-coordinate.
 *             SISHORT Map_Y - Map Y-coordinate.
 *             UNLONG *Dungeon_X_ptr - Pointer to dungeon X-coordinate.
 *             UNLONG *Dungeon_Y_ptr - Pointer to dungeon Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Map_to_dungeon(SISHORT Map_X, SISHORT Map_Y, UNLONG *Dungeon_X_ptr,
 UNLONG *Dungeon_Y_ptr)
{
	UNLONG f;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Convert map to dungeon coordinates */
	*Dungeon_X_ptr = ((UNLONG) Map_X - 1) * f + (f / 2);
	*Dungeon_Y_ptr = (UNLONG)(Map_height - Map_Y) * f + (f / 2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dungeon_to_map
 * FUNCTION  : Convert dungeon to map coordinates.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 12:01
 * LAST      : 11.07.95 15:26
 * INPUTS    : UNLONG Dungeon_X - Dungeon X-coordinate.
 *             UNLONG Dungeon_Y - Dungeon Y-coordinate.
 *             SISHORT *Map_X_ptr - Pointer to map X-coordinate.
 *             SISHORT *Map_Y_ptr - Pointer to map Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dungeon_to_map(UNLONG Dungeon_X, UNLONG Dungeon_Y, SISHORT *Map_X_ptr,
 SISHORT *Map_Y_ptr)
{
	UNLONG f;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Convert dungeon to map coordinates */
	*Map_X_ptr = (SISHORT) ((Dungeon_X / f) + 1);
	*Map_Y_ptr = (SISHORT) (Map_height - (Dungeon_Y / f));
}

