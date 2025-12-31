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

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <XLOAD.H>
#include <GFXFUNC.H>
#include <SORT.H>
#include <FINDCOL.H>

#include <CONTROL.H>
#include <MAP.H>
#include <AUTOMAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <GAMETIME.H>
#include <3D_MAP.H>
#include <3DCOMMON.H>
#include <3DM.H>
#include <ANIMATE.H>
#include <XFTYPES.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <POPUP.H>
#include <GAMETEXT.H>

/* structure definitions */
struct Collision_3D {
	SISHORT dX, dY;
	UNSHORT Bit_mask;
};

/* global variables */

static UNLONG Actual_size;
static UNLONG Converted_size;

/* 3D floor texture buffer file type */
static UNCHAR Floor_buffer_fname[] = "Floor buffer";

static struct File_type Floor_buffer_ftype = {
	NULL,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Floor_buffer_fname
};

/* 3D shade table file type */
static UNCHAR Shade_table_fname[] = "Shade table";

static struct File_type Shade_table_ftype = {
	Relocate_Shade_table,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Shade_table_fname
};

/* 3D map module */
struct Module M3_Mod =
{
	0, SCREEN_MOD, MAP_3D_SCREEN,
	M3_Main_loop,
	M3_ModInit,
	M3_ModExit,
	Init_3D_display,
	Exit_3D_display,
	Update_map_display
};

/* 3D map movement module */
static struct Module MM3_Mod =
{
	NO_INPUT_HANDLING, MODE_MOD, MAP_3D_SCREEN,
	MM3_Main_loop,
	Init_MM3,
	Exit_MM3,
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

UNSHORT Nr_of_object_groups, Nr_of_floors, Nr_of_objects, Nr_of_walls;
static UNSHORT Nr_of_floor_buffers, Nr_of_textures, Nr_of_texture_buffers;

MEM_HANDLE Lab_data_handle;
static MEM_HANDLE Shade_table_handle, Background_gfx_handle;
static MEM_HANDLE Floor_buffer_handles[FLOORS_MAX * 2];
static MEM_HANDLE Texture_buffer_handles[TEXTURES_MAX];
static MEM_HANDLE Overlay_handles[WALLS_MAX][8];
static MEM_HANDLE Wall_shade_tables[WALLS_MAX];

struct Texture_3D Floor_textures[FLOORS_MAX][8];
static struct Texture_3D Textures[(OBJECTS_MAX * 8) + (WALLS_MAX * 9)];
static struct Texture_3D *Object_textures[OBJECTS_MAX][8];
static struct Texture_3D *Wall_textures[WALLS_MAX][9];

UNLONG Wall_offsets[WALLS_MAX];
static UNBYTE Wall_states[WALLS_MAX];

struct Interface_3DM I3DM;

static struct OPM M3_OPM;

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
		Push_module(&MM3_Mod);
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
 * LAST      : 04.11.94 16:18
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
	UNSHORT Q;

	/* Get 3D mouse state */
	Q = Get_3D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_3D[Q]]));

	/* Inside pop-up area ? */
	if (Q == 4)
	{
		struct BBRECT MA;
		UNSHORT W, H;
		UNSHORT W3, H3;

		W = I3DM.Window_3D_width;
		H = I3DM.Window_3D_height;
		W3 = W / 3;
		H3 = H / 3;

		MA.left = Window_3D_X + W3;
		MA.top = Window_3D_Y + H3;
		MA.width = W - 2 * W3;
		MA.height = H - 2 * H3;

		Push_MA(&MA);

		Push_module(&Default_module);

		Wait_4_right_unclick();

		Pop_module();

		Pop_MA();

		Map_selected_X = PARTY_DATA.X + Offsets4[PARTY_DATA.View_direction][0];
		Map_selected_Y = PARTY_DATA.Y + Offsets4[PARTY_DATA.View_direction][1];

		Do_PUM(Mouse_X + 16, Mouse_Y - 40, &Map_PUM);
	}

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
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_ModInit
 * FUNCTION  : Initialize 3D map module.
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
M3_ModInit(void)
{
	struct Map_data *Map_ptr;
	struct Lab_data *Lab_ptr;
	struct Floor_3D *Floor_ptr;
	struct Object_3D *Object_ptr;
	struct Wall_3D *Wall_ptr, *Work_ptr;
	struct Overlay_3D *Overlay_ptr;
	UNSHORT i, Err, View_depth;
	UNBYTE *Ptr;

	/* Set dimensions in interface */
	I3DM.Window_3D_width = Screen_width;
	I3DM.Window_3D_height = Panel_Y;

	/* Calculate coordinates */
	Window_3D_X = (Screen_width - I3DM.Window_3D_width) / 2;
	Window_3D_Y = (Panel_Y - I3DM.Window_3D_height) / 2;

	/* Make OPM */
	OPM_CreateVirtualOPM(&Main_OPM, &M3_OPM, Window_3D_X, Window_3D_Y,
	 I3DM.Window_3D_width, I3DM.Window_3D_height);

	/* Load palette (for 3D shading tables) */
	Load_palette(Current_map_palette_nr);

	/* Load LAByrinth data */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	i = Map_ptr->LAB_DATA_NR;

	MEM_Free_pointer(Map_handle);

	Lab_data_handle = Load_subfile(LAB_DATA, i);

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

	/* Build list of offsets to wall data */
	Work_ptr = Wall_ptr;
	for (i=0;i<Nr_of_walls;i++)
	{
		/* Store pointer to wall data */
		Wall_offsets[i] = (UNLONG) Work_ptr - (UNLONG) Lab_ptr;

		/* Skip overlay data */
		Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
		Overlay_ptr += Work_ptr->Nr_overlays;
		Work_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/* Calculate shade table */
	Shade_table_handle = MEM_Do_allocate(65 * 256, (UNLONG) 0, &Shade_table_ftype);

	if ((Current_map_type == CITY_MAP) || (Current_map_type == WILDERNESS_MAP))
	{
		Calculate_shade_table(&Palette, Lab_ptr->Target_R, Lab_ptr->Target_G,
			Lab_ptr->Target_B, 256);
	}
	else
	{
		Calculate_shade_table(&Palette, Lab_ptr->Target_R, Lab_ptr->Target_G,
			Lab_ptr->Target_B, 192);
	}

	/* Load background graphics */
	i = Lab_ptr->Background_graphics_nr;
	if (i)
		Background_gfx_handle = Load_subfile(BACKGROUND_GFX, i);
	else
		Background_gfx_handle = NULL;

	/* Load floor, ceiling, object, overlay and wall graphics */
	Load_3D_floors(Lab_ptr, Floor_ptr);
	Load_3D_textures(Object_ptr, Wall_ptr);
	Load_3D_overlays(Wall_ptr);

	/* Put un-animated overlays on un-animated walls */
	Put_unanimated_overlays_on_unanimated_walls();

	/* Calculate shade tables for transparent walls */
	Calculate_shade_tables_for_transparent_walls();

	/* Initialize 3DM interface */
	I3DM.Flags = 0;
	I3DM.Camera_height = 165;		// Write function !!!
	I3DM.Horizon_Y = Lab_ptr->Horizon_Y;
	I3DM.Horizon_offset = Lab_ptr->Background_offset;
	I3DM.Sky_colour = Lab_ptr->Sky_colour;
	I3DM.Floor_colour	= Lab_ptr->Floor_colour;
	I3DM.Square_size = (1 << Lab_ptr->Square_size_cm_log);
	I3DM.Wall_height = Lab_ptr->Wall_height;
	I3DM.Default_colour = Find_closest_colour(Lab_ptr->Target_R,
	 Lab_ptr->Target_G, Lab_ptr->Target_B);
	I3DM.Shade_table = (UNBYTE *)
	 (((UNLONG) MEM_Get_pointer(Shade_table_handle) + 256) & 0xffffff00);
	I3DM.Shade_factor = Lab_ptr->Shade_factor;
	I3DM.Horizon_angle_factor = Lab_ptr->Background_angle_factor;
	I3DM.Target_OPM = &M3_OPM;

	for (i=0;i<4;i++)
	{
		I3DM.Shade_offsets[i] = (Shade_offsets[i] *
		 Lab_ptr->Shade_deviation) / 100;
	}

	View_depth = Lab_ptr->View_depth;

	/* Calculate shade function */
	Calculate_shade_function(Lab_ptr->Function_A, Lab_ptr->Function_B);

	MEM_Free_pointer(Lab_data_handle);

	/* Initialize party position */
	Initialize_3D_position();

	/* Initialize NPCs */
	Init_3D_NPCs();
	Display_3D_NPCs();

	I3DM.NPCs = NPCs_3D;
	I3DM.Nr_NPCs = Nr_3D_NPCs;

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

	/* Set interface flags (must be cleared !) */
	if ((Current_map_type == CITY_MAP) || (Current_map_type == WILDERNESS_MAP))
		I3DM.Flags |= DUNGEON_OR_CITY;

	/* Initialize 3DM */
	Err = Set_3DM_WindowSize(I3DM.Window_3D_width, I3DM.Window_3D_height);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}
	Err = Set_3DM_AspectRatio(111);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}
	Err = Set_3DM_SquareSize(I3DM.Square_size);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}
	Err = Set_3DM_Map(Map_width, Map_height, I3DM.Wall_height);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}
	Err = Set_3DM_ShadeTable(I3DM.Shade_table);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}
	Err = Init_3DM();
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}
	Err = Set_3DM_ViewDepth(View_depth);
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
		return;
	}

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
 * LAST      : 15.08.94 15:40
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
	UNSHORT i,j;

	/* Destroy OPM */
	OPM_Del(&M3_OPM);

	/* Close 3DM */
	Exit_3DM();

	/* Free floor and ceiling graphics */
	for (i=0;i<Nr_of_floor_buffers;i++)
		MEM_Free_memory(Floor_buffer_handles[i]);

	/* Free object and wall graphics */
	for (i=0;i<Nr_of_texture_buffers;i++)
		MEM_Free_memory(Texture_buffer_handles[i]);

	/* Free overlay graphics */
	for (i=0;i<Nr_of_walls;i++)
	{
		for (j=0;j<OVERLAYS_MAX;j++)
		{
			MEM_Free_memory(Overlay_handles[i][j]);
		}
	}

	/* Free transparent wall shade tables */
	for (i=0;i<Nr_of_walls;i++)
	{
		if (Wall_shade_tables[i])
		{
			MEM_Free_memory(Wall_shade_tables[i]);
			Wall_shade_tables[i] = NULL;
		}
	}

	/* Free background graphics and shade table */
	if (Background_gfx_handle)
		MEM_Free_memory(Background_gfx_handle);
	MEM_Free_memory(Shade_table_handle);

	/* Free LAByrinth data */
	MEM_Free_memory(Lab_data_handle);

	/* Exit automap */
	Exit_automap();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_3D_display
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
Init_3D_display(void)
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
 * NAME      : Exit_3D_display
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
Exit_3D_display(void)
{
	/* Delete interface object */
	Delete_object(M3_Object);

	Remove_update_OPM(&M3_OPM);

	Exit_map_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M3_Main_loop
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
M3_Main_loop(void)
{
	/* Display map */
	Update_display();

	Update_time(1);

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
 * LAST      : 05.09.94 14:11
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
	struct Map_data *Map_ptr;
	struct Lab_data *Lab_ptr;
	struct Wall_3D *Wall_ptrs[WALLS_MAX];
	struct Wall_3D *Work_ptr;
	struct Wall_3D *Wall_ptr;
	struct Overlay_3D *Overlay_ptr;
	UNSHORT i, j;
	UNSHORT Err;
	UNBYTE *Ptr;

	Update_map_stuff();

	/* Set view direction */
	PARTY_DATA.View_direction = ((((((0 - (I3DM.Camera_angle / 65536)) -
	 (ANGLE_STEPS / 8)) & (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 4)) + 1) &
	 0x0003);

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

	Ptr += (Nr_of_objects * sizeof(struct Object_3D)) + 2;

	Wall_ptr = (struct Wall_3D *) Ptr;

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
	I3DM.Shade_table = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Shade_table_handle) + 256) & 0xffffff00);
	I3DM.Background_graphics = MEM_Claim_pointer(Background_gfx_handle)
	 + sizeof(struct Gfx_header);

	/* Put overlays on walls */
	{
		struct Texture_3D *Source_texture, *Target_texture;
		UNBYTE *Source_wall_graphics, *Target_wall_graphics, *Overlay_graphics;
		UNSHORT Q, Frame, k;

		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_of_walls;i++)
		{
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Q = Wall_states[i];

			if (Q >= 4)
			{
				/* Get source and target wall graphics */
				Frame = Get_animation_frame(Work_ptr->Flags, Work_ptr->Nr_frames, i);

				Source_texture = Wall_textures[i][Frame];
				Source_wall_graphics = MEM_Claim_pointer(Source_texture->p.Handle)
					+ Source_texture->X;

				Target_texture = Wall_textures[i][8];
				Target_wall_graphics = MEM_Claim_pointer(Target_texture->p.Handle)
					+ Target_texture->X;

				/* Is this wall animated ? */
				if (Work_ptr->Nr_frames > 1)
				{
					/* Yes -> Copy a new frame to the work frame */
					for (j=0;j<Work_ptr->Graphics_width;j++)
					{
						for (k=0;k<Work_ptr->Graphics_height;k++)
						{
							*Target_wall_graphics++ = *Source_wall_graphics++;
						}
						Source_wall_graphics += 256 - Work_ptr->Graphics_height;
						Target_wall_graphics += 256 - Work_ptr->Graphics_height;
					}
				}
				else
				{
					/* No -> Remove all current overlays */
					for (j=0;j<Work_ptr->Nr_overlays;j++)
					{
						Remove_overlay_from_wall(&(Overlay_ptr[j]),
						 Source_wall_graphics, Target_wall_graphics);
					}
				}

				MEM_Free_pointer(Source_texture->p.Handle);

				/* Put all the overlays on the walls */
				for (j=0;j<Work_ptr->Nr_overlays;j++)
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
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Overlay_ptr += Work_ptr->Nr_overlays;
			Work_ptr = (struct Wall_3D *) Overlay_ptr;
		}
	}

	/* Call 3DM */
	Err = Render_3DM_View();
	if (Err)
	{
		Error_3DM(Err);
		Quit_program = TRUE;
	}
	M3_OPM.status |= OPMSTAT_CHANGED;

	/* Exit */
	MEM_Free_pointer(Background_gfx_handle);
	MEM_Free_pointer(Shade_table_handle);
	MEM_Free_pointer(Map_handle);
	MEM_Free_pointer(Lab_data_handle);

	#ifdef URGLE
	OPM_FillBox(&Status_area_OPM, 180, 10, 150, 10, BLACK);
	xprintf(&Status_area_OPM, 181, 11, "ACTUAL : %lu, CONVERTED : %lu", Actual_size, Converted_size);

	PrintValue(-100, 80, Update_duration, WHITE);
	PrintValue(-100, 40, *((UNLONG *) &(I3DM.Player_X[1])), WHITE);
	PrintValue(-100, 50, *((UNLONG *) &(I3DM.Player_Z[1])), WHITE);
	PrintValue(-100, 60, (I3DM.Camera_angle >> 16), WHITE);
	PrintValue(-50, 80, I3DM.Shade_factor, WHITE);
	#endif
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
Load_3D_floors(struct Lab_data *Lab_ptr, struct Floor_3D *Floor_ptr)
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
	Load_full_batch(FLOOR_3D, Nr_of_floors, &Batch[0], &Handles[0]);

	/* Convert floors and ceilings */
	Index = 0;

	Nr_of_floor_buffers = 1;
	Floor_buffer_handles[0] = Allocate_floor_buffer();
	Target = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Floor_buffer_handles[0]) + 256) & 0xffffff00);

	/* Do all floors */
	for (i=0;i<Nr_of_floors;i++)
	{
		/* Get pointer to floor graphics */
		Source = MEM_Claim_pointer(Handles[i]);

		#ifdef URGLE
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
			Floor_textures[i][j].X = Index * 64;
			Floor_textures[i][j].p.Handle = Floor_buffer_handles[Nr_of_floor_buffers - 1];

			/* Time for a new buffer ? */
			Index++;
			if (Index == 4)
			{
				/* Yes -> Allocate a new one */
				MEM_Free_pointer(Floor_buffer_handles[Nr_of_floor_buffers - 1]);

				Nr_of_floor_buffers++;

				Floor_buffer_handles[Nr_of_floor_buffers - 1] = Allocate_floor_buffer();
				Target = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Floor_buffer_handles[Nr_of_floor_buffers - 1])
				 + 256) & 0xffffff00);

				Index = 0;
			}
		}

		/* Discard floor */
		MEM_Free_pointer(Handles[i]);
		MEM_Free_memory(Handles[i]);
	}

	/* Ready */
	MEM_Free_pointer(Floor_buffer_handles[Nr_of_floor_buffers - 1]);

	#ifdef URGLE
	/* Is this lab-data already initialized ? */
	if (!(Lab_ptr->Flags & LAB_DATA_INIT))
	{
		/* No -> It is now */
		Lab_ptr->Flags |= LAB_DATA_INIT;
	}
	#endif
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Allocate_floor_buffer
 * FUNCTION  : Allocate a floor buffer.
 * FILE      : 3D_MAP.C
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

	return(Handle1);
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

	/* Clear */
	Actual_size = 0;
	Converted_size = 0;

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
			Textures[Nr_of_textures].Width = Object_ptr[i].Graphics_height;
			Textures[Nr_of_textures].Height = Object_ptr[i].Graphics_width;
			Textures[Nr_of_textures].p.Ptr = &Object_textures[i][j];
			Nr_of_textures++;
		}
	}

	/* Then the walls */
	Work_ptr = Wall_ptr;
	for (i=0;i<Nr_of_walls;i++)
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
			Textures[Nr_of_textures].Width = Work_ptr->Graphics_height;
			Textures[Nr_of_textures].Height = Work_ptr->Graphics_width;
			Textures[Nr_of_textures].p.Ptr = &Wall_textures[i][8];
			Nr_of_textures++;
		}

		/* Are the other frames needed ? */
		if (Q != 2)
		{
			/* Yes -> Do each animation frame */
			for (j=0;j<Work_ptr->Nr_frames;j++)
			{
				/* Create texture data */
				Textures[Nr_of_textures].Width = Work_ptr->Graphics_height;
				Textures[Nr_of_textures].Height = Work_ptr->Graphics_width;
				Textures[Nr_of_textures].p.Ptr = &Wall_textures[i][j];
				Nr_of_textures++;
			}
		}

		/* Skip overlay data */
		Overlay_ptr += Work_ptr->Nr_overlays;
		Work_ptr = (struct Wall_3D *) Overlay_ptr;
	}

	/*** Sort textures on height, then width ***/
	Shellsort(Swap_3D_textures, Compare_3D_textures, Nr_of_textures,
	 (UNBYTE *) (UNLONG) 0);

	/*** Get pointers to textures ***/
	{
		struct Texture_3D **P;

		for (i=0;i<Nr_of_textures;i++)
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
		Load_partial_batch(OBJECT_3D, Nr_of_objects, &Batch[0], &Handles[0]);

		/* Convert and copy object graphics */
		for (i=0;i<Nr_of_objects;i++)
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
		MEM_HANDLE Handles[WALLS_MAX];
		UNLONG Size;
		UNSHORT Batch[WALLS_MAX];
		UNBYTE *Ptr;

		/* Build wall batch */
		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_of_walls;i++)
		{
			Batch[i] = Work_ptr->Graphics_nr;

			/* Skip overlay data */
			Overlay_ptr = (struct Overlay_3D *) (Work_ptr + 1);
			Overlay_ptr += Work_ptr->Nr_overlays;
			Work_ptr = (struct Wall_3D *) Overlay_ptr;
		}

		/* Load wall graphics */
		Load_partial_batch(WALL_3D, Nr_of_walls, &Batch[0], &Handles[0]);

		/* Convert and copy wall graphics */
		Work_ptr = Wall_ptr;
		for (i=0;i<Nr_of_walls;i++)
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

	Actual_size += (Texture->Width * Texture->Height);

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
	UNSHORT i,j;
	UNBYTE *Target, *Ptr;

	Actual_size += (Texture->Width * Texture->Height);

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
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 17:17
 * LAST      : 11.08.94 17:17
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
	Shellsort(Swap_3D_textures, Compare_3D_textures2, End - Start + 1,
	 (UNBYTE *) ((UNLONG) Start));

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
		Handle = MEM_Allocate_memory((UNLONG) Height * 256);

		/* Clear texture buffer */
		MEM_Clear_memory(Handle);

		/* Increase */
		Converted_size += Height * 256;

		Texture_buffer_handles[Nr_of_texture_buffers] = Handle;
		Nr_of_texture_buffers++;

		X = 0;

		/* Position as many large textures as possible */
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
 * FILE      : 3D_MAP.C
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
	Load_partial_batch(OVERLAY_3D, Nr_of_walls * OVERLAYS_MAX,
	 &Batch[0][0], &Overlay_handles[0][0]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_shade_tables_for_transparent_walls
 * FUNCTION  : Calculate shade tables for transparent walls.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.02.95 13:35
 * LAST      : 15.02.95 13:35
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_shade_tables_for_transparent_walls(void)
{
	struct Lab_data *Lab_ptr;
	struct Wall_3D *Wall_ptr;
	struct Overlay_3D *Overlay_ptr;
	MEM_HANDLE Handle;
	UNSHORT Target_R, Target_G, Target_B;
	UNSHORT R, G, B;
	UNSHORT i, j;
	UNBYTE *Ptr;

	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Ptr = (UNBYTE *) Lab_ptr + sizeof(struct Lab_data)
	 + Nr_of_object_groups * sizeof(struct Object_group_3D) + 2
	 + Nr_of_floors * sizeof(struct Floor_3D) + 2
	 + Nr_of_objects * sizeof(struct Object_3D) + 2;

	Wall_ptr = (struct Wall_3D *) Ptr;

	for (i=0;i<Nr_of_walls;i++)
	{
		/* Is this a transparent wall ? */
		if (Wall_ptr->Flags & TRANSPARENT_WALL)
		{
			/* Yes -> Allocate space for shade table */
			Handle = MEM_Allocate_memory(512);

			/* Get target colour */
			j = Wall_ptr->Transparent_colour;
			Target_R = Palette.color[j].red;
			Target_G = Palette.color[j].green;
			Target_B= Palette.color[j].blue;

			/* Make shading table */
			Ptr = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Handle) + 256) &
			 0xffffff00);
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

/*				R = ((R - Target_R) * (abs(R - Target_R))) / 256 + Target_R;
				G = ((G - Target_G) * (abs(G - Target_G))) / 256 + Target_G;
				B = ((B - Target_B) * (abs(B - Target_B))) / 256 + Target_B; */

				/* Find the closest matching colour in the palette */
				*Ptr++ = Find_closest_colour(R, G, B);
			}
			MEM_Free_pointer(Handle);

			/* Store handle */
			Wall_shade_tables[i] = Handle;
		}
		else
		{
			/* No -> Clear handle */
			Wall_shade_tables[i] = NULL;
		}

		/* Skip overlay data */
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
		Overlay_ptr += Wall_ptr->Nr_overlays;
		Wall_ptr = (struct Wall_3D *) Overlay_ptr;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_unanimated_overlays_on_unanimated_walls
 * FUNCTION  : Put un-animated overlays on un-animated walls.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.02.95 13:41
 * LAST      : 15.02.95 13:41
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
	struct Lab_data *Lab_ptr;
	struct Texture_3D *Texture;
	struct Wall_3D *Wall_ptr;
	struct Overlay_3D *Overlay_ptr;
	UNSHORT i, j, k, Q;
	UNBYTE *Wall_graphics, *Overlay_graphics;
	UNBYTE *Ptr;

	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Ptr = (UNBYTE *) Lab_ptr + sizeof(struct Lab_data)
	 + Nr_of_object_groups * sizeof(struct Object_group_3D) + 2
	 + Nr_of_floors * sizeof(struct Floor_3D) + 2
	 + Nr_of_objects * sizeof(struct Object_3D) + 2;

	Wall_ptr = (struct Wall_3D *) Ptr;

	for (i=0;i<Nr_of_walls;i++)
	{
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
		Q = Wall_states[i];

		/* Un-animated overlays on un-animated wall ? */
		if (Q == 2)
		{
			/* Yes -> Put all the overlays on the wall */
			Texture = Wall_textures[i][8];
			Wall_graphics = MEM_Claim_pointer(Texture->p.Handle)
				+ Texture->X;

			for (j=0;j<Wall_ptr->Nr_overlays;j++)
			{
				Overlay_graphics = MEM_Claim_pointer(Overlay_handles[i][j]);

				Put_overlay_on_wall(&(Overlay_ptr[j]), Wall_graphics,
					Overlay_graphics);

				MEM_Free_pointer(Overlay_handles[i][j]);
			}

			MEM_Free_pointer(Texture->p.Handle);
		}

		/* Un-animated overlays on animated wall ? */
		if (Q == 3)
		{
			/* Yes -> Put all the overlays on the walls */
			for (k=0;k<Wall_ptr->Nr_frames;k++)
			{
				Texture = Wall_textures[i][k];
				Wall_graphics = MEM_Claim_pointer(Texture->p.Handle)
					+ Texture->X;

				for (j=0;j<Wall_ptr->Nr_overlays;j++)
				{
					Overlay_graphics = MEM_Claim_pointer(Overlay_handles[i][j]);

					Put_overlay_on_wall(&(Overlay_ptr[j]), Wall_graphics,
						Overlay_graphics);

					MEM_Free_pointer(Overlay_handles[i][j]);
				}

				MEM_Free_pointer(Texture->p.Handle);
			}
		}

		/* Skip overlay data */
		Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
		Overlay_ptr += Wall_ptr->Nr_overlays;
		Wall_ptr = (struct Wall_3D *) Overlay_ptr;
	}
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
	UNSHORT Frame;

	if (Object_nr > Nr_of_objects)
		return(0);

	/* Get object data */
	Object = (I3DM.Objects) + Object_nr - 1;

	/* Is an NPC ? */
	if (NPC_nr == 0xFFFF)
	{
		/* No -> Get animation frame */
		Frame = Get_animation_frame(Object->Flags, Object->Nr_frames, Hash_nr);

		/* Get texture data */
		Texture = Object_textures[Object_nr-1][Frame];

		/* Return pointer to texture */
		return(MEM_Get_pointer(Texture->p.Handle) + Texture->X);
	}
	else
	{
		/* Yes -> Get animation frame */
		Frame = Get_animation_frame(Object->Flags, Object->Nr_frames, Hash_nr);

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
 * FILE      : 3D_MAP.C
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

	if (Floor_nr > Nr_of_floors)
		return(0);

	/* Get floor data */
	Floor = (I3DM.Floors) + Floor_nr - 1;

	/* Get animation frame */
	Frame = Get_animation_frame(Floor->Flags, Floor->Nr_frames, Hash_nr);

	/* Get texture data */
	Texture = &Floor_textures[Floor_nr-1][Frame];

	/* Return pointer to texture */
	return((UNBYTE *) ((((UNLONG) MEM_Get_pointer(Texture->p.Handle) + 256) & 0xffffff00) + Texture->X));
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
	UNSHORT Frame, Q;

	if (Wall_nr > Nr_of_walls)
		return(0);

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
	/* Return pointer to shading table */
	return((UNBYTE *) (((UNLONG) MEM_Get_pointer(Wall_shade_tables[Wall_nr-1]) + 256) & 0xffffff00));
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
	UNSHORT i,j;
	UNBYTE *Ptr, c;

	/* Calculate destination address */
	Wall_graphics += Overlay->Y + (Overlay->X * 256);

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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_overlay_from_wall
 * FUNCTION  : Remove an overlay from a wall.
 * FILE      : 3D_MAP.C
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
	UNSHORT i,j;

	/* Calculate destination address */
	i = Overlay->Y + (Overlay->X * 256);
	Source_wall_graphics += i;
	Target_wall_graphics += i;

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
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.94 11:12
 * LAST      : 31.08.94 11:12
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
	UNBYTE *Ptr, Last;
	UNSHORT i,j;

	/* Is this a 3D map ? */
	if (_3D_map)
	{
		/* Yes -> Get pointer to shade table */
		Ptr = (UNBYTE*) (((UNLONG) MEM_Claim_pointer(Shade_table_handle) + 256)
		 & 0xffffff00) + Start;

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

		MEM_Free_pointer(Shade_table_handle);

		/* Set shade table */
		Set_3DM_ShadeTable(I3DM.Shade_table);
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
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 12:33
 * LAST      : 19.06.95 12:38
 * INPUTS    : struct BBPALETTE *Pal - Pointer to colour palette.
 *             UNSHORT Target_R - Target red value.
 *             UNSHORT Target_G - Target green value.
 *             UNSHORT Target_B - Target blue value.
 *             UNSHORT Number - Number of colours that must be shaded.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function assumes that an appropriately sized buffer has
 *              already been allocated.
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

	Ptr = (UNBYTE *) (((UNLONG) MEM_Claim_pointer(Shade_table_handle) + 256) &
	 0xffffff00);

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

	MEM_Free_pointer(Shade_table_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Relocate_Shade_table
 * FUNCTION  : Relocate a shade tale.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.11.94 11:18
 * LAST      : 17.11.94 11:18
 * INPUTS    : MEM_HANDLE Handle - Handle of memory block.
 *             UNBYTE *Source - Pointer to source.
 *             UNBYTE *Target - Pointer to target.
 *             UNLONG Size - Size of memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Relocate_Shade_table(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target,
 UNLONG Size)
{
	UNBYTE *Ptr1, *Ptr2;

	Ptr1 = (UNBYTE *) (((UNLONG) Source + 256) & 0xffffff00);
	Ptr2 = (UNBYTE *) (((UNLONG) Target + 256) & 0xffffff00);

	/* Copy the memory block */
	BASEMEM_CopyMem(Ptr1, Ptr2, 64 * 256);
}

#ifdef URGLE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Texture_vanish_test
 * FUNCTION  : Test if a texture may vanish when shaded away.
 * FILE      : 3D_MAP.C
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
		/* Yes -> Don't bother */
		return(FALSE);

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

	return(Result);
}
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_lab_data
 * FUNCTION  : Convert 3D LAByrinth data (file converter function).
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.94 13:09
 * LAST      : 07.09.94 13:09
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
	UNSHORT i,j;

	Lab_ptr = (struct Lab_data *) Ptr;

	/* Calculate conversion factor */
	f = 9 - Lab_ptr->Square_size_cm_log;

	/* Is conversion necessary ? */
	if (f)
	{
		/* Yes -> Convert all object groups */
		Object_group_ptr = (struct Object_group_3D *) (Lab_ptr + 1);

		if (f > 0)
		{
			f = (1 << f);
			for (i=0;i<Lab_ptr->Nr_object_groups;i++)
			{
				for (j=0;j<OBJECTS_PER_GROUP;j++)
				{
						Object_group_ptr->Group[j].X /= f;
						Object_group_ptr->Group[j].Y /= f;
						Object_group_ptr->Group[j].Z /= f;
				}
				Object_group_ptr++;
			}
		}
		else
		{
			f = (1 << (0-f));
			for (i=0;i<Lab_ptr->Nr_object_groups;i++)
			{
				for (j=0;j<OBJECTS_PER_GROUP;j++)
				{
					Object_group_ptr->Group[j].X *= f;
					Object_group_ptr->Group[j].Y *= f;
					Object_group_ptr->Group[j].Z *= f;
				}
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
 * FILE      : 3D_MAP.C
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
 * NAME      : MM3_Main_loop
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
MM3_Main_loop(void)
{
	/* Update display */
	Update_display();

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
				Make_3D_move(-F * yawSIN, F * yawCOS);
			}
			break;
		case 1:
			F = (150 * Update_duration * (H3 - Y)) / H6;
			F /= F2;
			Make_3D_move(-F * yawSIN, F * yawCOS);
			break;
		case 2:
			F = (18 * Update_duration * (X - 2 * W3)) / W6;
			Rotate_3D(-F);
			if (Y < 2 * (H3 / 3))
			{
				F = (150 * Update_duration * ((H3 - Y) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_move(-F * yawSIN, F * yawCOS);
			}
			break;
		case 3:
			F = (100 * Update_duration * (W3 - X)) / W6;
			F /= F2;
			Make_3D_move(-F * yawCOS, -F * yawSIN);
			break;
		case 4:
			break;
		case 5:
			F = (100 * Update_duration * (X - 2 * W3)) / W6;
			F /= F2;
			Make_3D_move(F * yawCOS, F * yawSIN);
			break;
		case 6:
			F = (18 * Update_duration * (W3 - X)) / W6;
			Rotate_3D(F);
			if (Y > 2 * H3 + (H3 / 3))
			{
				F = (100 * Update_duration * ((Y - 2 * H3) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_move(F * yawSIN, -F * yawCOS);
			}
			break;
		case 7:
			F = (120 * Update_duration * (Y - 2 * H3)) / H6;
			F /= F2;
			Make_3D_move(F * yawSIN, -F * yawCOS);
			break;
		case 8:
			F = (18 * Update_duration * (X - 2 * W3)) / W6;
			Rotate_3D(-F);
			if (Y > 2 * H3 + (H3 / 3))
			{
				F = (100 * Update_duration * ((Y - 2 * H3) - (H3 / 3))) / H6;
				F /= F2;
				Make_3D_move(F * yawSIN, -F * yawCOS);
			}
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_MM3
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
Init_MM3(void)
{
	struct BBRECT MA;

	/* Create mouse area */
	MA.left = Window_3D_X;
	MA.top = Window_3D_Y;
	MA.width = I3DM.Window_3D_width;
	MA.height = I3DM.Window_3D_height;

	Push_MA(&MA);

	/* Indicate move mode is on */
	Move_mode_flag = TRUE;

	/* Guarantee first movement */
	Mouse_move_3D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_MM3
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
Exit_MM3(void)
{
	/* Remove mouse area */
	Pop_MA();

	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
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
		return(9);

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

	return(Q);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_3D_move
 * FUNCTION  : Make a move in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:23
 * LAST      : 15.09.94 14:23
 * INPUTS    : SILONG dX - X-component of movement vector.
 *             SILONG dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_3D_move(SILONG dX, SILONG dY)
{
	BOOLEAN Result;
	UNLONG X0, Y0, X1, Y1;
	SILONG wdX, wdY;
	SISHORT Map_X, Map_Y;
	UNSHORT Backup_X[3], Backup_Z[3];
	UNSHORT i;

	/* Clear flags */
	Moved = FALSE;
	Bumped = FALSE;
	Result = FALSE;

	/* Allowed by before-move logic ? */
	if (Before_move())
	{
		/* Yes -> Save current coordinates */
		for (i=0;i<3;i++)
		{
			Backup_X[i] = I3DM.Player_X[i];
			Backup_Z[i] = I3DM.Player_Z[i];
		}

		/* Get current coordinates */
		X0 = * ((UNLONG *) &(I3DM.Player_X[1]));
		Y0 = * ((UNLONG *) &(I3DM.Player_Z[1]));

		/* Move along vector */
		Add_3D_vector(dX, dY);

		/* Get new coordinates */
		X1 = * ((UNLONG *) &(I3DM.Player_X[1]));
		Y1 = * ((UNLONG *) &(I3DM.Player_Z[1]));

		/* Restore coordinates */
		for (i=0;i<3;i++)
		{
			I3DM.Player_X[i] = Backup_X[i];
			I3DM.Player_Z[i] = Backup_Z[i];
		}

		/* Try to move */
		wdX = dX;
		wdY = dY;
		Result = Try_3D_move(X1, Y1, 0xFFFF, PARTY_DATA.Travel_mode);

		/* Did it work ? */
		if (!Result)
		{
			/* No -> Slide (first choice) */
			if (dX > dY)
			{
				wdY = 0;
				Result = Try_3D_move(X1, Y0, 0xFFFF, PARTY_DATA.Travel_mode);
			}
			else
			{
				wdX = 0;
				Result = Try_3D_move(X0, Y1, 0xFFFF, PARTY_DATA.Travel_mode);
			}
		}

		/* Did it work ? */
		if (!Result)
		{
			/* No -> Slide (second choice) */
			if (dX > dY)
			{
				wdX = 0;
				wdY = dY;
				Result = Try_3D_move(X0, Y1, 0xFFFF, PARTY_DATA.Travel_mode);
			}
			else
			{
				wdX = dX;
				wdY = 0;
				Result = Try_3D_move(X1, Y0, 0xFFFF, PARTY_DATA.Travel_mode);
			}
		}

		/* Did it work ? */
		if (Result)
		{
			/* Yes -> Move */
			Add_3D_vector(wdX, wdY);
			Moved = TRUE;

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

				Update_time(4);

			}
		}
		else
		{
			/* No */
			Update_time(1);

			Bumped = TRUE;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_3D_move
 * FUNCTION  : Check if a certain move can be made in a 3D map.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 14:26
 * LAST      : 15.09.94 14:26
 * INPUTS    : SILONG X - New X-coordinate.
 *             SILONG Y - New Y-coordinate.
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

	/* Exit if cheat mode is on / not NPC */
	if ((NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Calculate map coordinates */
	Dungeon_to_map(X, Y, &Map_X, &Map_Y);

	/* Calculate coordinates inside current map square */
	Square_X = X & (f - 1);
	Square_Z = Y & (f - 1);

	/* Check the destination map square */
	Result = Movement_check_3D(Map_X, Map_Y, Travel_mode);

	/* Collision ? */
	if (Result)
	{
		/* No -> Try edges */
		/* Build edge collision list */
		Bitlist = 0;
		for (i=0;i<8;i++)
		{
			if (!Movement_check_3D(Map_X + Collision_table[i].dX,
			 Map_Y - Collision_table[i].dY, Travel_mode))
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

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Movement_check_3D
 * FUNCTION  : Check if a certain position in a 3D map can be moved to.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 15:32
 * LAST      : 15.09.94 15:32
 * INPUTS    : SISHORT Map_X - Map X-coordinate.
 *             SISHORT Map_Y - Map Y-coordinate.
 *             UNSHORT Travel_mode - Travel mode.
 * RESULT    : BOOLEAN : TRUE (not blocked) or FALSE (blocked).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Movement_check_3D(SISHORT Map_X, SISHORT Map_Y, UNSHORT Travel_mode)
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
		return(FALSE);

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

	return(Result);
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

