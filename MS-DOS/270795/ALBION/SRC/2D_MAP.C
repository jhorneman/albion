/************
 * NAME     : 2D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28.07.94 13:10
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
#include <BBMEM.H>

#include <XLOAD.H>
#include <SCROLL.H>
#include <GFXFUNC.H>
#include <TEXT.H>

#include <DIAGNOST.H>
#include <CONTROL.H>
#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <2D_DISPL.H>
#include <EVELOGIC.H>
#include <POPUP.H>
#include <XFTYPES.H>
#include <FONT.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <NPCS.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <MUSIC.H>

/* defines */

#define PARTY_PATH_LENGTH		(256)

/* structure definitions */

struct Member_data_2D {
	MEM_HANDLE Graphics_handle;
	UNSHORT Path_distance, Counter;
	UNSHORT Path_index;
	UNSHORT Frame; //, State;
};

struct Path_entry_2D {
	UNSHORT X, Y;
	UNSHORT View_direction, State;
};

/* prototypes */

/* 2D map module */
void M2_ModInit(void);
void M2_ModExit(void);
void M2_DisInit(void);
void M2_DisExit(void);
void M2_MainLoop(void);

/* 2D map object methods */
UNLONG Touched_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Left_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Right_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_2D_object(struct Object *Object, union Method_parms *P);

/* 2D map move mode */
void Move_M2_MainLoop(void);
void Mouse_move_2D(void);
void Move_M2_ModInit(void);
void Move_M2_ModExit(void);

/* 2D map selection mode */
void Select_M2_MainLoop(void);
void Display_M2_selection(UNSHORT X, UNSHORT Y);
void Select_M2_ModInit(void);
void Select_M2_ModExit(void);

UNSHORT Get_2D_mouse_state(SISHORT X, SISHORT Y);

/* 2D movement and collision detection */

BOOLEAN Special_2D_high_check(UNSHORT X, UNSHORT Y);
BOOLEAN Special_2D_low_check(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY);

BOOLEAN Double_movement_check_2D(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY,
 UNSHORT Travel_mode, UNSHORT NPC_index);
BOOLEAN Extra_vertical_movement_check_2D(UNSHORT X, UNSHORT Y,
 UNSHORT NPC_index);

BOOLEAN Do_check_2D(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY,
 UNSHORT Travel_mode, UNSHORT NPC_index);
BOOLEAN Check_2D_NPC_collision(UNSHORT X, UNSHORT Y, UNSHORT NPC_index);

BOOLEAN Sucking_chairs_2D(struct Movement_2D *Move);

void Update_2D_party_position(BOOLEAN Movement);
void Store_2D_movement(SISHORT dX, SISHORT dY);
void Make_2D_path_entry(UNSHORT X, UNSHORT Y);

void Display_2D_party(void);

/* global variables */

/* 2D map module */
struct Module M2_Mod = {
	0, SCREEN_MOD, MAP_2D_SCREEN,
	M2_MainLoop,
	M2_ModInit,
	M2_ModExit,
	M2_DisInit,
	M2_DisExit,
	Update_map_display
};

/* 2D map move mode module */
static struct Module Move_M2_Mod = {
	NO_INPUT_HANDLING, MODE_MOD, MAP_2D_SCREEN,
	Move_M2_MainLoop,
	Move_M2_ModInit,
	Move_M2_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 2D map select mode module */
static struct Module Select_M2_Mod = {
	LOCAL_MOD | NO_INPUT_HANDLING, MODE_MOD, NO_SCREEN,
	Select_M2_MainLoop,
	Select_M2_ModInit,
	Select_M2_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 2D map object method list */
static struct Method M2_methods[] = {
	{ TOUCHED_METHOD, Touched_2D_object },
	{ LEFT_METHOD, Left_2D_object },
	{ RIGHT_METHOD, Right_2D_object },
	{ CUSTOMKEY_METHOD, Customkeys_2D_object },
	{ 0, NULL}
};

/* 2D map object class description */
static struct Object_class M2_Class = {
	0, sizeof(struct Object),
	&M2_methods[0]
};

UNLONG Party_object_X, Party_object_Y;

static UNSHORT M2_Object;

MEM_HANDLE Icondata_handle;
static MEM_HANDLE Icongfx_handle;
MEM_HANDLE Blocklist_handle;

static struct Object_2D Party_objects[6];
static struct Member_data_2D M2_Party[6];
static UNSHORT Active_member_state;

static struct Path_entry_2D Party_path[PARTY_PATH_LENGTH];
UNSHORT Party_animation[12] = {1, 1, 1, 2, 2, 2, 1, 1, 1, 0, 0, 0};

static UNSHORT Party_path_index;
static SISHORT _2D_steps_left;
UNSHORT _2D_speed;
static UNSHORT _2D_scale;
UNSHORT Preview_direction;

static UNSHORT Direction_pairs[8][2] = {
	{NORTH, NORTH}, {NORTH, EAST}, {EAST, EAST}, {SOUTH, EAST},
	{SOUTH, SOUTH}, {SOUTH, WEST}, {WEST, WEST}, {NORTH, WEST}
};

BOOLEAN Big_guy;
static SIBYTE Big_guy_list[] = {
	FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE
};

static UNSHORT Mice_2D[] = {
	UPLEFT_MPTR, UP2D_MPTR, UPRIGHT_MPTR,
	LEFT2D_MPTR, POP_UP_MPTR, RIGHT2D_MPTR,
	DOWNLEFT_MPTR, DOWN2D_MPTR, DOWNRIGHT_MPTR,
	DEFAULT_MPTR			/* Outside of the map window */
};

UNBYTE *Icon_graphics_ptr;
struct Icon_2D_data *Icon_data_ptr;

struct Scroll_data Scroll_2D;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M2_ModInit
 * FUNCTION  : Initialize 2D map module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:14
 * LAST      : 18.07.95 12:06
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M2_ModInit(void)
{
	struct Map_data *Map_ptr;
	struct Character_data *Char;
	MEM_HANDLE Handle;
	UNSHORT Nr;
	UNSHORT i, j;

	/* Initialize located sound effects */
	Map_square_size = 16;
	Initialize_located_effects(Map_width * Map_square_size, Map_height *
	 Map_square_size, 20 * Map_square_size);

	/* Set 2D map window dimensions */
	Map_2D_width = 360;
	Map_2D_height = 192;

	Mapbuf_width = (Map_2D_width + 31) / 16;
	Mapbuf_height = (Map_2D_height + 31) / 16;

	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	/* Determine scale */
	Big_guy = Big_guy_list[Map_ptr->ICON_SET_NR - 1];

	/* Load party graphics */
	if (Big_guy)
		j = PARTYGR_GFX;
	else
		j = PARTYKL_GFX;

	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Get 2D graphics number */
			Char = (struct Character_data *) MEM_Claim_pointer(Party_char_handles[i]);
			Nr = (UNSHORT) Char->Graphics_2D_nr;
			MEM_Free_pointer(Party_char_handles[i]);

			/* Load graphics */
			Handle = Load_subfile(j, Nr);
			if (!Handle)
			{
				Error(ERROR_FILE_LOAD);
				Exit_program();
				return;
			}
			M2_Party[i].Graphics_handle = Handle;
		}
	}

	/* Load icon data */
	Handle = Load_subfile(ICON_DATA, (UNSHORT) Map_ptr->ICON_SET_NR);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}
	Icondata_handle = Handle;

	/* Load icon graphics */
	Handle = Load_subfile(ICON_GFX, (UNSHORT) Map_ptr->ICON_SET_NR);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}
	Icongfx_handle = Handle;

	/* Load block list */
	Handle = Load_subfile(BLOCK_LIST, (UNSHORT) Map_ptr->ICON_SET_NR);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}
	Blocklist_handle = Handle;

	MEM_Free_pointer(Map_handle);

	Preview_direction = 0xFFFF;

	/* No objects */
	Clear_2D_object_list();

	/* Initialize party objects */
	{
		struct Gfx_header *Gfx;

		for (i=0;i<6;i++)
		{
			/* Anyone there ? */
			if (PARTY_DATA.Member_nrs[i])
			{
				/* Yes -> Get graphics header */
				Gfx = (struct Gfx_header *)
				 MEM_Claim_pointer(M2_Party[i].Graphics_handle);

				/* Set graphics data */
				Party_objects[i].Width = Gfx->Width;
				Party_objects[i].Height = Gfx->Height;
				Party_objects[i].Level = 0;
				Party_objects[i].Graphics_handle = M2_Party[i].Graphics_handle;

				MEM_Free_pointer(M2_Party[i].Graphics_handle);

				/* Add party object to list */
				Add_2D_object(&Party_objects[i]);

				/* Set random start frame */
				M2_Party[i].Frame = rand() % 12;
			}
		}
	}

	Party_path_index = 0;

	if (Big_guy)
	{
		_2D_scale = 4;
	}
	else
	{
		_2D_scale = 2;
	}

	_2D_speed = _2D_scale;

	/* Initialize party path */
	{
		UNSHORT X,Y;

		X = (PARTY_DATA.X - 1) * 16;
	 	Y = (PARTY_DATA.Y - 1) * 16 + 15;

		for (i=0;i<PARTY_PATH_LENGTH;i++)
		{
			Party_path[i].X = X;
			Party_path[i].Y = Y;
			Party_path[i].View_direction = PARTY_DATA.View_direction;
			Party_path[i].State = STANDING_STATE;
		}

		Party_object_X = X;
		Party_object_Y = Y;

		for (i=0;i<6;i++)
		{
			M2_Party[i].Path_index = 0;
			M2_Party[i].Path_distance = 0;

			Party_objects[i].X = X;
			Party_objects[i].Y = Y;

			Party_objects[i].Graphics_offset = ((PARTY_DATA.View_direction * 3
			 + Party_animation[M2_Party[i].Frame])
			 * Party_objects[i].Width * Party_objects[i].Height)
			 + sizeof(struct Gfx_header);
		}
	}

	/* Set scrolling data */
	Scroll_2D.Update_unit = Update_2D_unit;
	Scroll_2D.Unit_width = 16;
	Scroll_2D.Unit_height = 16;
	Scroll_2D.Viewport_width = Map_2D_width;
	Scroll_2D.Viewport_height = Map_2D_height;
	Scroll_2D.Playfield_width = Map_width * 16 ;
	Scroll_2D.Playfield_height = Map_height * 16;

	/* Initialize */
	Init_2D_NPCs();
	Display_2D_NPCs();

	Prepare_2D_display();
	Initialize_2D_camera();

	/* Initialize scrolling */
	Icon_data_ptr = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);
	Icon_graphics_ptr = MEM_Claim_pointer(Icongfx_handle);

	Init_scroll(&Scroll_2D, Party_object_X - (Map_2D_width / 2) + 16,
	 Party_object_Y - (Map_2D_height / 2) - 8);

	MEM_Free_pointer(Icongfx_handle);
	MEM_Free_pointer(Icondata_handle);

	/* Save position */
	Save_position();

	/* Handle first step */
	After_move();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	M2_ModExit
 * FUNCTION  : Exit 2D map module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:00
 * LAST      : 28.07.94 12:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M2_ModExit(void)
{
	UNSHORT i;

	/* Destroy scroll data */
	Exit_scroll(&Scroll_2D);

	Exit_2D_NPCs();

	/* Free memory */
	MEM_Free_memory(Icondata_handle);
	MEM_Free_memory(Blocklist_handle);
	MEM_Free_memory(Icongfx_handle);

	for (i=0;i<6;i++)
		MEM_Free_memory(M2_Party[i].Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M2_DisInit
 * FUNCTION  : Initialize 2D map display.
 * FILE      : 2D_MAP.C
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
M2_DisInit(void)
{
	UNSHORT i;

	Init_map_display();

	/* Add OPMs to update list */
	for (i=0;i<4;i++)
	{
		Add_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
	}

	/* Add interface object */
	M2_Object = Add_object(Earth_object, &M2_Class, NULL, 0, 0, Map_2D_width,
	 Map_2D_height);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M2_DisExit
 * FUNCTION  : Exit 2D map display.
 * FILE      : 2D_MAP.C
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
M2_DisExit(void)
{
	UNSHORT i;

	/* Delete interface object */
	Delete_object(M2_Object);

	/* Remove OPMs from update list */
	for (i=0;i<4;i++)
	{
		Remove_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
	}

	Exit_map_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M2_MainLoop
 * FUNCTION  : 2D map main loop.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 11:19
 * LAST      : 18.07.95 14:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M2_MainLoop(void)
{
	/* Has the party moved ? */
	if (Moved)
	{
		/* Yes -> This has already been displayed */
		Moved = FALSE;
	}
	else
	{
		/* No -> Update party member positions */
		Move_2D(0, 0, FALSE);
	}

	Set_listener_position(PARTY_DATA.X * 16, PARTY_DATA.Y * 16, 0);

	/* Update NPC positions */
	Handle_2D_NPCs();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_map
 * FUNCTION  : Display the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:18
 * LAST      : 28.07.94 12:18
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_map(void)
{
	/* Update map stuff */
	Update_map_stuff();

	/* Reset 2D object list */
	Clear_2D_object_list();

	/* Add party objects */
	Display_2D_party();

	/* Add NPC objects */
	Display_2D_NPCs();

	/* Update display */
	Icon_data_ptr = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);
	Icon_graphics_ptr = MEM_Claim_pointer(Icongfx_handle);

	Draw_2D_map(MAP_2D_X, MAP_2D_Y);

	MEM_Free_pointer(Icongfx_handle);
	MEM_Free_pointer(Icondata_handle);

	if (Diagnostic_mode)
	{
		struct Square_2D *Map_ptr;
		UNSHORT B1, B2, B3;
		UNSHORT Underlay_nr;
		UNSHORT Overlay_nr;
		UNBYTE *Map_data_ptr;

		Map_data_ptr = MEM_Claim_pointer(Map_handle);
		Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
		Map_ptr += PARTY_DATA.X - 1 + ((PARTY_DATA.Y - 1) * Map_width);

		/* Read bytes from map */
		B1 = (UNSHORT) Map_ptr->m[0];
		B2 = (UNSHORT) Map_ptr->m[1];
		B3 = (UNSHORT) Map_ptr->m[2];

		MEM_Free_pointer(Map_handle);

		/* Build overlay and underlay number */
		Underlay_nr = ((B2 & 0x0F) << 8) | B3;
		Overlay_nr = (B1 << 4) | (B2 >> 4);

		OPM_FillBox(&Status_area_OPM, 181, 3, 80, 40, BLACK);

		Push_textstyle(&Diagnostic_text_style);
		xprintf(&Status_area_OPM, 182, 4, "Underlay : %lu", Underlay_nr);
		xprintf(&Status_area_OPM, 182, 14, "Overlay : %lu", Overlay_nr);
		Pop_textstyle();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Touched_2D_object
 * FUNCTION  : Touched method of 2D map object.
 * FILE      : 2D_MAP.C
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
Touched_2D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Q;

	/* Update highlighting */
	Normal_touched(Object, P);

	/* Get 2D mouse state */
	Q = Get_2D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_2D[Q]]));

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_2D_object
 * FUNCTION  : Left method of 2D map object.
 * FILE      : 2D_MAP.C
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
Left_2D_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Q;

	/* Get 2D mouse state */
	Q = Get_2D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_2D[Q]]));

	/* Is the mouse pointer inside the 2D map window ? */
	if (Q < 9)
	{
		/* Yes -> Enter move mode */
		Push_module(&Move_M2_Mod);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Right_2D_object
 * FUNCTION  : Right method of 2D map object.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.11.94 16:53
 * LAST      : 15.06.95 16:40
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Right_2D_object(struct Object *Object, union Method_parms *P)
{
	/* Select a map square */
	Push_module(&Select_M2_Mod);

	/* Do map pop-up menu */
	Do_PUM(Mouse_X + 16, Mouse_Y - 40, &Map_PUM);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_2D_object
 * FUNCTION  : Custom keys method of 2D map object.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 13:03
 * LAST      : 20.06.95 12:56
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Reacted to key.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_2D_object(struct Object *Object, union Method_parms *P)
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
		/* Move up */
		case BLEV_UP:
		{
			Make_2D_move(NORTH8);
			break;
		}
		/* Move left */
		case BLEV_LEFT:
		{
			Make_2D_move(WEST8);
			break;
		}
		/* Move right */
		case BLEV_RIGHT:
		{
			Make_2D_move(EAST8);
			break;
		}
		/* Move down */
		case BLEV_DOWN:
		{
			Make_2D_move(SOUTH8);
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
 * NAME      : Move_M2_MainLoop
 * FUNCTION  : 2D map movement main loop.
 * FILE      : 2D_MAP.C
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
Move_M2_MainLoop(void)
{
	/* Update input handling */
	Update_input();

	/* Is the left mouse button pressed ? */
	if (Button_state & 0x0001)
	{
		/* Yes -> Update NPC positions */
		Handle_2D_NPCs();

		/* Move */
		Mouse_move_2D();

		Set_listener_position(PARTY_DATA.X * 16, PARTY_DATA.Y * 16, 0);
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
 * NAME      : Mouse_move_2D
 * FUNCTION  : Move in 2D move mode.
 * FILE      : 2D_MAP.C
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
Mouse_move_2D(void)
{
	UNSHORT Q;

	/* Get 2D mouse state */
	Q = Get_2D_mouse_state(Mouse_X, Mouse_Y);

	/* Change mouse-pointer accordingly */
	Change_mouse(&(Mouse_pointers[Mice_2D[Q]]));

	/* Move depending on state */
	switch (Q)
	{
		/* Try to move up-left */
		case 0:
			Make_2D_move(NORTH_WEST);
			break;
		/* Try to move up */
		case 1:
			Make_2D_move(NORTH8);
			break;
		/* Try to move up-right */
		case 2:
			Make_2D_move(NORTH_EAST);
			break;
		/* Try to move left */
		case 3:
			Make_2D_move(WEST8);
			break;
		/* No action */
		case 4:
			Move_2D(0, 0, FALSE);
			break;
		/* Try to move right */
		case 5:
			Make_2D_move(EAST8);
			break;
		/* Try to move down-left */
		case 6:
			Make_2D_move(SOUTH_WEST);
			break;
		/* Try to move down */
		case 7:
			Make_2D_move(SOUTH8);
			break;
		/* Try to move down-right */
		case 8:
			Make_2D_move(SOUTH_EAST);
			break;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_M2_ModInit
 * FUNCTION  : Initialize 2D map move mode module.
 * FILE      : 2D_MAP.C
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
Move_M2_ModInit(void)
{
	struct BBRECT MA;

	/* Create mouse area */
	MA.left = MAP_2D_X;
	MA.top = MAP_2D_Y;
	MA.width = Map_2D_width;
	MA.height = Map_2D_height;

	Push_MA(&MA);

	/* Indicate move mode is on */
	Move_mode_flag = TRUE;

	/* Guarantee first movement */
	Mouse_move_2D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_M2_ModExit
 * FUNCTION  : Exit 2D map move mode module.
 * FILE      : 2D_MAP.C
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
Move_M2_ModExit(void)
{
	/* Remove mouse area */
	Pop_MA();

	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M2_MainLoop
 * FUNCTION  : 2D map selection mode main loop.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.11.94 14:01
 * LAST      : 02.11.94 14:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M2_MainLoop(void)
{
	/* Update display */
	Update_display();

	/* Update input handling */
	Update_input();

	/* Is the right mouse button pressed ? */
	if (Button_state & 0x0010)
	{
		/* Yes -> Highlight */
		Display_M2_selection(Mouse_X, Mouse_Y);
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
 * NAME      : Display_M2_selection
 * FUNCTION  : Display 2D map selection.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.11.94 15:01
 * LAST      : 21.07.95 16:26
 * INPUTS    : UNSHORT X - Mouse X-coordinate.
 *             UNSHORT Y - Mouse Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_M2_selection(UNSHORT X, UNSHORT Y)
{
	/* Adjust incoming coordinates */
	X = ((X + (Scroll_2D.Playfield_X & 0x000F)) & 0xFFF0) -
	 (Scroll_2D.Playfield_X & 0x000F);
	Y = ((Y + (Scroll_2D.Playfield_Y & 0x000F)) & 0xFFF0) -
	 (Scroll_2D.Playfield_Y & 0x000F);

	/* Determine map coordinates */
	Current_map_selection_data.Map_X = (X + (Scroll_2D.Playfield_X & 0x000F)) /
	 16 + Mapbuf_X + 1;
	Current_map_selection_data.Map_Y = (Y + (Scroll_2D.Playfield_Y & 0x000F)) /
	 16 + Mapbuf_Y + 1;

	/* Display selection */
	Put_recoloured_box(&Main_OPM, X, Y, 16, 16, &(Recolour_tables[7][0]));
	Put_masked_block(&Main_OPM, X - 1, Y - 1, 18, 18, Select_2D_cursor);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M2_ModInit
 * FUNCTION  : Initialize 2D map selection mode module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.11.94 14:01
 * LAST      : 19.07.95 18:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M2_ModInit(void)
{
	struct BBRECT MA;
	SISHORT X, Y;
	UNSHORT W, H;

	/* Output 2D map in main OPM */
	Current_2D_OPM = &Main_OPM;

	/* Cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes -> Can select all over the screen */
		MA.left = MAP_2D_X;
		MA.top = MAP_2D_Y;
		MA.width = Map_2D_width;
		MA.height = Map_2D_height;
	}
	else
	{
		/* No -> Determine dimensions of party leader */
		if (Big_guy)
		{
			W = 32;
			H = 48;
		}
		else
		{
			W = 16;
			H = 32;
		}

		/* Determine top-left coordinates of mouse area */
		X = (Party_object_X & 0xFFF0) - (Scroll_2D.Playfield_X & 0xFFF0)
		 - (Scroll_2D.Playfield_X & 0x000F) - 32;
		Y = ((Party_object_Y - H + 1) & 0xFFF0) - (Scroll_2D.Playfield_Y & 0xFFF0)
		 - (Scroll_2D.Playfield_Y & 0x000F) - 16;

		/* Add dimensions of mouse area */
		W += 64 - 1;
		H += 32 - 1;

		/* Clip area */
		if (X < MAP_2D_X)
		{
			W = X + W - 1;
			X = MAP_2D_X;
		}

		if (Y < MAP_2D_Y)
		{
			H = Y + H - 1;
			Y = MAP_2D_Y;
		}

		if (X + W >= MAP_2D_X + Map_2D_width)
		{
			W = MAP_2D_X + Map_2D_width - X;
		}

		if (Y + H >= MAP_2D_Y + Map_2D_height)
		{
			H = MAP_2D_Y + Map_2D_height - Y;
		}

		/* Build mouse area */
		MA.left = X;
		MA.top = Y;
		MA.width = W;
		MA.height = H;
	}

	/* Install mouse area */
	Push_MA(&MA);

	/* Disable mouse pointer */
	Mouse_off();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M2_ModExit
 * FUNCTION  : Exit 2D map selection mode module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.11.94 16:50
 * LAST      : 02.11.94 16:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M2_ModExit(void)
{
	/* Remove mouse area */
	Pop_MA();

	/* Enable mouse pointer */
	Mouse_on();

	/* Output 2D map directly */
	Current_2D_OPM = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_2D_mouse_state
 * FUNCTION  : Get state of mouse in 2D map window.
 * FILE      : 2D_MAP.C
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
Get_2D_mouse_state(SISHORT X, SISHORT Y)
{
	SISHORT pX, pY;
	UNSHORT pW, pH;
	UNSHORT Q;

	/* Translate coordinates */
	X -= MAP_2D_X;
	Y -= MAP_2D_Y;

	/* Exit if the mouse pointer is outside of the 2D map window */
	if ((X >= Map_2D_width) || (Y >= Map_2D_height))
		return 9;

	/* Determine dimensions of party leader */
	if (Big_guy)
	{
		pW = 32;
		pH = 48;
	}
	else
	{
		pW = 16;
		pH = 32;
	}

	/* Determine position of party leader */
	pX = Party_object_X - Scroll_2D.Playfield_X + (pW / 2) - 32;
	pY = Party_object_Y - Scroll_2D.Playfield_Y - (pH / 2) - 32;

	/* Determine state depending on position relative to party leader */
	Q = 0;
	if (X >= pX)
	{
		if (X < pX + 64)
			Q = 1;
		else
			Q = 2;
	}
	if (Y >= pY)
	{
		if (Y < pY + 64)
			Q += 3;
		else
			Q += 6;
	}

	return Q;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_2D_move
 * FUNCTION  : Make a move in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 23.07.95 16:25
 * INPUTS    : UNSHORT Direction - Direction (0...7).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_2D_move(UNSHORT Direction)
{
	static struct Movement_2D Party_2D_move_data = {
		0,
		0, 0,
		0,
		0, 0,
		0,
		0xFFFF,
		0xFFFF,
		0,
		0xFFFF,
		0
	};
	BOOLEAN Result;

	/* Clear flags */
	Moved = FALSE;
	Bumped = FALSE;

	/* Allowed by before-move logic ? */
	if (Before_move())
	{
		/* Yes -> Try to move */
		Party_2D_move_data.X						= PARTY_DATA.X;
		Party_2D_move_data.Y						= PARTY_DATA.Y;
		Party_2D_move_data.Direction			= Direction;
		Party_2D_move_data.View_direction	= PARTY_DATA.View_direction;
		Party_2D_move_data.Travel_mode		= PARTY_DATA.Travel_mode;
		Party_2D_move_data.State				= Active_member_state;

		Result = Try_2D_move(&Party_2D_move_data);

		/* Enter move */
		PARTY_DATA.X					= Party_2D_move_data.X;
		PARTY_DATA.Y					= Party_2D_move_data.Y;
		PARTY_DATA.View_direction	= Party_2D_move_data.View_direction;
		Active_member_state			= Party_2D_move_data.State;

		Move_2D(Party_2D_move_data.dX, Party_2D_move_data.dY,
		 (BOOLEAN) Party_2D_move_data.Flags & UPDATE_AFTER_MOVE);

		/* Success ? */
		if (Result)
		{
			/* Yes -> Set flag */
			Moved = TRUE;

			/* Do after-move logic */
			After_move();
		}
		else
		{
			/* No */
			Bumped = TRUE;

			/* Time passes */
//			Update_time(100);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_2D_move
 * FUNCTION  : Check if a certain move can be made in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 08.09.94 16:42
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_2D_move(struct Movement_2D *Move)
{
	static UNSHORT Target_VDs[8] = {
		NORTH, NORTH, EAST, SOUTH, SOUTH, SOUTH, WEST, NORTH
	};
	static SISHORT Big_diagonal_check_offsets[4][2] = {
		{0,1}, {1,1}, {2,1}, {2,0}
	};
	static SISHORT Small_diagonal_check_offsets[3][2] = {
		{0,1}, {1,1}, {1,0}
	};

	BOOLEAN Result = TRUE;
	BOOLEAN Slide_flag = FALSE;
	SISHORT dX, dY;
	SISHORT ddDir;
	SISHORT corrX = 0;
	UNSHORT X, Y;
	UNSHORT Direction;
	UNSHORT cDir;
	UNSHORT dDir;
	UNSHORT cVD;
	UNSHORT tVD = 0xFFFF;
	UNSHORT ntVD;
	UNSHORT Q;
	UNSHORT i;

	/* Get important data */
	X = Move->X;
	Y = Move->Y;
	Direction = Move->Direction;

	/* Get movement vector */
	dX = Offsets8[Direction][0];
	dY = Offsets8[Direction][1];

	/* Get current direction */
	cVD = Move->View_direction;
	cDir = cVD * 2;

	/* Get reverse slide direction (if any) */
	if (Move->Slide_direction != 0xFFFF)
	{
		Move->Reverse_slide = (Move->Slide_direction + 2) & 0x0003;
	}

	/* Calculate direction difference */
	if (Direction >= cDir)
	{
		dDir = Direction - cDir;
		ddDir = 1;
	}
	else
	{
		dDir = cDir - Direction;
		ddDir = -1;
	}

	if (dDir > 4)
	{
		dDir = 8 - dDir;
		ddDir = 0 - ddDir;
	}

	/* Does the player want to move orthogonally and is the difference in
	 directions equal to 90 degrees ? */
	if ((dDir == 2) && (!(Direction & 1)))
	{
		/* Yes -> The player must be rotated */
		tVD = Target_VDs[Direction];
	}

	/* Is the difference in directions greater than 90 degrees ? */
	if (dDir > 2)
	{
		/* Yes -> The player must be rotated. An extra rotation step is needed
		 between the current and the target direction. This extra view (!)
		 direction must be determined. */
		tVD = Target_VDs[(cDir + (ddDir * ((dDir + 1) / 2))) & 0x0007];

	}

	/* Is rotation necessary ? */
	if (tVD != 0xFFFF)
	{
		/* Yes -> Rotate */
		Move->Slide_direction = 0xFFFF;
		Move->Reverse_slide = 0xFFFF;

		/* Are we standing ? */
		if (Move->State == STANDING_STATE)
		{
			/* Yes -> Rotate on the spot */
			Move->View_direction = tVD;
			Move->dX = 0;
			Move->dY = 0;

			/* Checking the party ? */
			if (Move->NPC_index == 0xFFFF)
			{
				/* Preview to the desired (!) direction */
				Preview_direction = Direction;
			}

			return TRUE;
		}
		else
		{
			/* No -> Switch directly to the desired view direction */
			Move->View_direction = Target_VDs[Direction];
		}
	}

	/* Big or small guy ? */
	if (Big_guy)
	{
		/* Big guy -> Does the player want to move diagonally ? */
		if (Direction & 1)
		{
			/* Yes -> Determine horizontal correction */
			if (dX < 0)
				corrX = 1;

			/* First we must determine the block state for the
			 current position */
			Q = 0;

			for (i=0;i<4;i++)
			{
				if (!Do_check_2D(X + corrX, Y, (Big_diagonal_check_offsets[i][0]
				 * dX), (Big_diagonal_check_offsets[i][1] * dY),
				 Move->Travel_mode, Move->NPC_index))
				{
					Q |= (1<<i);
				}
				if ((i == 1) || (i == 2))
				{
					if (!Check_2D_NPC_collision(X +
					 (Big_diagonal_check_offsets[i][0] * dX) + corrX, Y +
					 (Big_diagonal_check_offsets[i][1] * dY), Move->NPC_index))
					{
						Q |= (1<<i);
					}
				}
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/*** Movement may be possible ***/
				case 0:
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Move */
						Move->dX = dX;
						Move->dY = dY;
					}
					else
					{
						/* No -> Movement is blocked */
						Result = FALSE;
					}
					break;
				/*** Movement is blocked ***/
				case 10:
				case 11:
				case 13:
				case 14:
				case 15:
					Result = FALSE;
					break;
				/*** Try a diagonal move ***/
				case 9:
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Moving up ? */
						if (dY < 0)
						{
							/* Yes -> Can we slip through ? */
							if ((Special_2D_high_check(X + 2 * dX + corrX, Y)) &&
							 (Special_2D_low_check(X + corrX, Y - 1, dX, dY)))
							{
								/* Yes */
								Move->dX = dX;
								Move->dY = dY;
								break;
							}
						}
						else
						{
							/* No -> Can we slip through ? */
							if ((Special_2D_high_check(X + corrX, Y + 1)) &&
							 (Special_2D_low_check(X + 2 * dX + corrX, Y, dX, dY)))
							{
								/* Yes */
								Move->dX = dX;
								Move->dY = dY;
								break;
							}
						}
					}

					Result = FALSE;
					break;
				/*** Try a diagonal move ***/
				case 8:
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Moving up ? */
						if (dY < 0)
						{
							/* Yes -> Can we pass behind ? */
							if (Special_2D_high_check(X + 2 * dX + corrX, Y))
							{
								/* Yes */
								Move->dX = dX;
								Move->dY = dY;
								break;
							}
						}
						else
						{
							/* No -> Can we pass over ? */
							if (Special_2D_low_check(X + 2 * dX + corrX, Y, dX, dY))
							{
								/* Yes */
								Move->dX = dX;
								Move->dY = dY;
								break;
							}
						}
					}

					/* Try a vertical slide */
					/* Determine new view- and movement-direction */
					if (dY < 0)
						ntVD = NORTH;
					else
						ntVD = SOUTH;

					/* Is this movement possible ? */
					if ((Double_movement_check_2D(X, Y, 0, dY, Move->Travel_mode, Move->NPC_index))
					 && (ntVD != Move->Reverse_slide))
					{
						/* Yes -> Did we slide in this direction last time ? */
						if (ntVD == Move->Slide_direction)
						{
							/* Yes -> Turn around */
							Move->View_direction = ntVD;
						}

						/* Move vertically */
						Move->dX = 0;
						Move->dY = dY;

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = ntVD;
					}
					else
						Result = FALSE;
					break;
				/*** Try a diagonal move ***/
				case 1:
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Moving up ? */
						if (dY < 0)
						{
							/* Yes -> Can we pass behind ? */
							if (Special_2D_low_check(X + corrX, Y - 1, dX, dY))
							{
								/* Yes */
								Move->dX = dX;
								Move->dY = dY;
								break;
							}
						}
						else
						{
							/* No -> Can we pass over ? */
							if (Special_2D_high_check(X + corrX, Y + 1))
							{
								/* Yes */
								Move->dX = dX;
								Move->dY = dY;
								break;
							}
						}
					}

					/* Determine new view- and movement-direction */
					if (dX < 0)
						ntVD = WEST;
					else
						ntVD = EAST;

					/* Try a horizontal slide */
					/* Is this movement possible ? */
					if ((Double_movement_check_2D(X, Y, dX, 0, Move->Travel_mode, Move->NPC_index))
					 && (ntVD != Move->Reverse_slide))
					{
						/* Yes -> Did we slide in this direction last time ? */
						if (ntVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = ntVD;

						/* Move horizontally */
						Move->dX = dX;
						Move->dY = 0;

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = ntVD;
					}
					else
						Result = FALSE;
					break;
				/*** Try a horizontal slide ***/
				case 2:
				case 3:
				case 5:
				case 6:
				case 7:
					/* Determine new view- and movement-direction */
					if (dX < 0)
						ntVD = WEST;
					else
						ntVD = EAST;

					/* Is this movement possible ? */
					if ((Double_movement_check_2D(X, Y, dX, 0, Move->Travel_mode, Move->NPC_index))
					 && (ntVD != Move->Reverse_slide))
					{
						/* Yes -> Did we slide in this direction last time ? */
						if (ntVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = ntVD;

						/* Move horizontally */
						Move->dX = dX;
						Move->dY = 0;

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = ntVD;
					}
					else
						Result = FALSE;
					break;
				/*** Try a vertical slide ***/
				case 12:
					/* Determine new view- and movement-direction */
					if (dY < 0)
						ntVD = NORTH;
					else
						ntVD = SOUTH;

					/* Is this movement possible ? */
					if ((Double_movement_check_2D(X, Y, 0, dY, Move->Travel_mode, Move->NPC_index))
					 && (ntVD != Move->Reverse_slide))
					{
						/* Yes -> Did we slide in this direction last time ? */
						if (ntVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = ntVD;

						/* Move vertically */
						Move->dX = 0;
						Move->dY = dY;

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = ntVD;
					}
					else
						Result = FALSE;
					break;
				/*** Try to slide perpendicular to the current view direction ***/
				case 4:
					/* Determine perpendicular direction */
					cDir = (Direction - (cDir - Direction)) & 0x0007;
					cVD = cDir / 2;

					/* Are we sliding back ? */
					if (cVD != Move->Reverse_slide)
					{
						/* No -> Did we slide in this direction last time ? */
						if (cVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = cVD;

						/* Move */
						Move->dX = Offsets8[cDir][0];
						Move->dY = Offsets8[cDir][1];

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = cVD;
					}
					else
						Result = FALSE;
					break;
			}
		}
		else
		{
			/* No -> Is the current view direction horizontal or vertical ? */
			if (cVD & 1)
			{
				/* Horizontal movement */
				/* Determine horizontal correction */
				if (dX > 0)
					corrX = 1;

				/* Determine the block state for the current position */
				Q = 0;

				for (i=0;i<3;i++)
				{
					if (!Do_check_2D(X + corrX, Y, dX, i - 1, Move->Travel_mode,
					 Move->NPC_index))
					{
						Q |= (1<<i);
					}
					if (i == 1)
					{
						if (!Check_2D_NPC_collision(X + corrX + dX, Y + i - 1,
						 Move->NPC_index))
						{
							Q |= (1<<i);
						}
					}
				}

				/* Then we act according to the block state */
				switch (Q)
				{
					/*** Movement is possible ***/
					case 0:
					case 1:
					case 4:
					case 5:
						Move->dX = dX;
						Move->dY = dY;
						break;
					/*** Movement is blocked ***/
					case 7:
						Result = FALSE;
						break;
					/*** Try a vertical slide up ***/
					case 2:
					case 6:
						if ((Double_movement_check_2D(X, Y, 0, -1, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != NORTH))
						{
							/* Move north */
							Move->dX = 0;
							Move->dY = -1;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = NORTH;
						}
						else
						{
							/* If unsuccessful, try a vertical slide down */
							if ((Double_movement_check_2D(X, Y, 0, 1, Move->Travel_mode, Move->NPC_index))
							 && (Move->Reverse_slide != SOUTH))
							{
								/* Move south */
								Move->dX = 0;
								Move->dY = 1;

								/* Sliding occurred */
								Slide_flag = TRUE;
								Move->Slide_direction = SOUTH;
							}
							else
								Result = FALSE;
						}
						break;
					/*** Try a vertical slide down ***/
					case 3:
						if ((Double_movement_check_2D(X, Y, 0, 1, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != SOUTH))
						{
							/* Move south */
							Move->dX = 0;
							Move->dY = 1;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = SOUTH;
						}
						else
						{
							/* If unsuccessful, try a vertical slide up */
							if ((Double_movement_check_2D(X, Y, 0, -1, Move->Travel_mode, Move->NPC_index))
							 && (Move->Reverse_slide != NORTH))
							{
								/* Move north */
								Move->dX = 0;
								Move->dY = -1;

								/* Sliding occurred */
								Slide_flag = TRUE;
								Move->Slide_direction = NORTH;
							}
							else
								Result = FALSE;
						}
						break;
				}
			}
			else
			{
				/* Vertical movement */
				/* Determine the block state for the current position */
				Q = 0;

				for (i=0;i<4;i++)
				{
					if (!Do_check_2D(X, Y, i - 1, dY, Move->Travel_mode,
					 Move->NPC_index))
					{
						Q |= (1<<i);
					}
					if (i == 1)
					{
						if (!Check_2D_NPC_collision(X + i - 1, Y + dY,
						 Move->NPC_index))
						{
							Q |= (1<<i);
						}
					}
				}

				/* Then we act according to the block state */
				switch (Q)
				{
					/*** Movement may be possible ***/
					case 0:
					case 1:
					case 8:
					case 9:
						/* Is vertical movement possible here ? */
						if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
						 Move->NPC_index))
						{
							/* Yes -> Move */
							Move->dX = dX;
							Move->dY = dY;
						}
						else
						{
							/* No -> Movement is blocked */
							Result = FALSE;
						}
						break;
					/*** Movement is blocked ***/
					case 5:
					case 6:
					case 10:
					case 11:
					case 13:
					case 15:
						Result = FALSE;
						break;
					/*** Try a horizontal slide to the right ***/
					case 2:
					case 3:
					case 7:
						if ((Double_movement_check_2D(X, Y, 1, 0, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != EAST))
						{
							/* Move east */
							Move->dX = 1;
							Move->dY = 0;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = EAST;
						}
						else
							Result = FALSE;
						break;
					/*** Try a horizontal slide to the left ***/
					case 4:
					case 12:
					case 14:
						if ((Double_movement_check_2D(X, Y, -1, 0, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != WEST))
						{
							/* Move west */
							Move->dX = -1;
							Move->dY = 0;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = WEST;
						}
						else
							Result = FALSE;
						break;
				}
			}
		}
	}
	else
	{
		/* Small guy -> Does the player want to move diagonally ? */
		if (Direction & 1)
		{
			/* Yes -> First we must determine the block state for the
			 current position */
			Q = 0;

			for (i=0;i<3;i++)
			{
				if (!Do_check_2D(X, Y, (Small_diagonal_check_offsets[i][0] * dX),
				 (Small_diagonal_check_offsets[i][1] * dY), Move->Travel_mode,
				 Move->NPC_index))
				{
					Q |= (1<<i);
				}
				if (i == 1)
				{
					if (!Check_2D_NPC_collision(X +
					 (Small_diagonal_check_offsets[i][0] * dX),
					 Y + (Small_diagonal_check_offsets[i][1] * dY),
					 Move->NPC_index))
					{
						Q |= (1<<i);
					}
				}
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/*** Movement is possible ***/
				case 0:
					Move->dX = dX;
					Move->dY = dY;
					break;
				/*** Movement is blocked ***/
				case 5:
				case 7:
					Result = FALSE;
					break;
				/*** Try a horizontal slide ***/
				case 1:
				case 3:
					/* Determine new view- and movement-direction */
					if (dX < 0)
						ntVD = WEST;
					else
						ntVD = EAST;

					/* Sliding back ? */
					if (Move->Reverse_slide != ntVD)
					{
						/* No -> Did we slide in this direction last time ? */
						if (ntVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = ntVD;

						/* Move horizontally */
						Move->dX = dX;
						Move->dY = 0;

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = ntVD;
					}
					else
						Result = FALSE;

					break;
				/*** Try a vertical slide ***/
				case 4:
				case 6:
					/* Determine new view- and movement-direction */
					if (dY < 0)
						ntVD = NORTH;
					else
						ntVD = SOUTH;

					/* Sliding back ? */
					if (Move->Reverse_slide != ntVD)
					{
						/* Did we slide in this direction last time ? */
						if (ntVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = ntVD;

						/* Move vertically */
						Move->dX = 0;
						Move->dY = dY;

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = ntVD;
					}
					else
						Result = FALSE;

					break;
				/*** Try to slide perpendicular to the current view direction ***/
				case 2:
					/* Determine perpendicular direction */
					cDir = (Direction - (cDir - Direction)) & 0x0007;
					cVD = cDir / 2;

					/* Sliding back ? */
					if (Move->Reverse_slide != cVD)
					{
						/* Did we slide in this direction last time ? */
						if (cVD == Move->Slide_direction)
							/* Yes -> Turn around */
							Move->View_direction = cVD;

						/* Move */
						Move->dX = Offsets8[cDir][0];
						Move->dY = Offsets8[cDir][1];

						/* Sliding occurred */
						Slide_flag = TRUE;
						Move->Slide_direction = cVD;
					}
					else
						Result = FALSE;

					break;
			}
		}
		else
		{
			/* No -> Is the current view direction horizontal or vertical ? */
			if (cVD & 1)
			{
				/* Horizontal movement */
				/* Determine the block state for the current position */
				Q = 0;

				for (i=0;i<3;i++)
				{
					if (!Do_check_2D(X, Y, dX, i - 1, Move->Travel_mode,
					 Move->NPC_index))
					{
						Q |= (1<<i);
					}
					if (i == 1)
					{
						if (!Check_2D_NPC_collision(X + dX, Y + i - 1,
						 Move->NPC_index))
						{
							Q |= (1<<i);
						}
					}
				}

				/* Then we act according to the block state */
				switch (Q)
				{
					/*** Movement is possible ***/
					case 0:
					case 1:
					case 4:
					case 5:
						Move->dX = dX;
						Move->dY = dY;
						break;
					/*** Movement is blocked ***/
					case 7:
						Result = FALSE;
						break;
					/*** Try a vertical slide up ***/
					case 2:
					case 6:
						if ((Double_movement_check_2D(X, Y, 0, -1, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != NORTH))
						{
							/* Move north */
							Move->dX = 0;
							Move->dY = -1;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = NORTH;
						}
						else
						{
							/* If unsuccessful, try a vertical slide down */
							if ((Double_movement_check_2D(X, Y, 0, 1, Move->Travel_mode, Move->NPC_index))
							 && (Move->Reverse_slide != SOUTH))
						 	{
								/* Move south */
								Move->dX = 0;
								Move->dY = 1;

								/* Sliding occurred */
								Slide_flag = TRUE;
								Move->Slide_direction = SOUTH;
							}
							else
								Result = FALSE;
						}
						break;
					/*** Try a vertical slide down ***/
					case 3:
						if ((Double_movement_check_2D(X, Y, 0, 1, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != SOUTH))
						{
							/* Move south */
							Move->dX = 0;
							Move->dY = 1;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = SOUTH;
						}
						else
						{
							/* If unsuccessful, try a vertical slide up */
							if ((Double_movement_check_2D(X, Y, 0, -1, Move->Travel_mode, Move->NPC_index))
							 && (Move->Reverse_slide != NORTH))
							{
								/* Move north */
								Move->dX = 0;
								Move->dY = -1;

								/* Sliding occurred */
								Slide_flag = TRUE;
								Move->Slide_direction = NORTH;
							}
							else
								Result = FALSE;
						}
						break;
				}
			}
			else
			{
				/* Vertical movement */
				/* Determine the block state for the current position */
				Q = 0;

				for (i=0;i<3;i++)
				{
					if (!Do_check_2D(X, Y, i - 1, dY, Move->Travel_mode,
					 Move->NPC_index))
					{
						Q |= (1<<i);
					}
					if (i == 1)
					{
						if (!Check_2D_NPC_collision(X + i - 1, Y + dY,
						 Move->NPC_index))
						{
							Q |= (1<<i);
						}
					}
				}

				/* Then we act according to the block state */
				switch (Q)
				{
					/*** Movement is possible ***/
					case 0:
					case 1:
					case 4:
					case 5:
						Move->dX = dX;
						Move->dY = dY;
						break;
					/*** Movement is blocked ***/
					case 7:
						Result = FALSE;
						break;
					/*** Try a horizontal slide to the right ***/
					case 3:
						if ((Double_movement_check_2D(X, Y, 1, 0, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != EAST))
						{
							/* Move east */
							Move->dX = 1;
							Move->dY = 0;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = EAST;
						}
						else
						{
							/* If unsuccessful, try a horizontal slide to the left */
							if ((Double_movement_check_2D(X, Y, -1, 0, Move->Travel_mode, Move->NPC_index))
							 && (Move->Reverse_slide != WEST))
							{
								/* Move west */
								Move->dX = -1;
								Move->dY = 0;

								/* Sliding occurred */
								Slide_flag = TRUE;
								Move->Slide_direction = WEST;
							}
							else
								Result = FALSE;
						}
						break;
					/*** Try a horizontal slide to the left ***/
					case 2:
					case 6:
						if ((Double_movement_check_2D(X, Y, -1, 0, Move->Travel_mode, Move->NPC_index))
						 && (Move->Reverse_slide != WEST))
						{
							/* Move west */
							Move->dX = -1;
							Move->dY = 0;

							/* Sliding occurred */
							Slide_flag = TRUE;
							Move->Slide_direction = WEST;
						}
						else
						{
							/* If unsuccessful, try a horizontal slide to the right */
							if ((Double_movement_check_2D(X, Y, 1, 0, Move->Travel_mode, Move->NPC_index))
							 && (Move->Reverse_slide != EAST))
							{
								/* Move east */
								Move->dX = 1;
								Move->dY = 0;

								/* Sliding occurred */
								Slide_flag = TRUE;
								Move->Slide_direction = WEST;
							}
							else
								Result = FALSE;
						}
						break;
				}
			}
		}
	}

	/* Movement blocked ? */
	if (!Result)
	{
		/* Yes -> Clear vector */
		Move->dX = 0;
		Move->dY = 0;
		Move->Flags &= ~UPDATE_AFTER_MOVE;
	}

	/* Any movement at all ? */
	if (Move->dX || Move->dY)
	{
		/* Yes -> Suck ? */
		if (!(Move->Flags & DONT_SUCK))
		{
			/* Yes -> Suck */
			Result = Sucking_chairs_2D(Move);
		}

		/* Not an NPC ? */
		if (Move->NPC_index == 0xFFFF)
		{
			/* Preview direction depends on movement direction */
			Preview_direction = Direction;

			#ifdef URGLE
			/* Yes -> Set preview direction depending on state */
			switch (Move->State)
			{
				/* "Special" states */
				case SITTING_NL_STATE:
				case SITTING_NR_STATE:
					Preview_direction = NORTH8;
					break;
				case SITTING_E_STATE:
					Preview_direction = EAST8;
					break;
				case SITTING_SL_STATE:
				case SITTING_SR_STATE:
					Preview_direction = SOUTH8;
					break;
				case SITTING_W_STATE:
					Preview_direction = WEST8;
					break;
				/* Normal state : walking */
				default:
					/* Preview direction depends on movement direction */
					Preview_direction = Direction;
			}
			#endif
		}
	}

	/* Any sliding this time ? */
	if (!Slide_flag)
	{
		/* No -> Clear */
		Move->Slide_direction = 0xFFFF;

		/* Any movement at all ? */
		if (Move->dX || Move->dY)
		{
			/* Yes -> Unblock sliding */
			Move->Reverse_slide = 0xFFFF;
		}
 	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Special_2D_high_check
 * FUNCTION  : Check if an icon in a 2D map can be passed behind diagonally.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.94 13:04
 * LAST      : 27.07.95 10:35
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Special_2D_high_check(UNSHORT X, UNSHORT Y)
{
	struct Icon_2D_data *Icon_data;
	struct Square_2D *Map_ptr;
	UNLONG Status;
	UNSHORT B1, B2, B3;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;
	UNSHORT Under_h = 0, Over_h = 0;

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return FALSE;

	/* Read bytes from map */
	Map_ptr = (struct Square_2D *) (MEM_Claim_pointer(Map_handle)
	 + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);

	B1 = (UNSHORT) Map_ptr->m[0];
	B2 = (UNSHORT) Map_ptr->m[1];
	B3 = (UNSHORT) Map_ptr->m[2];

	MEM_Free_pointer(Map_handle);

	/* Build overlay and underlay number */
	Underlay_nr = ((B2 & 0x0F) << 8) | B3;
	Overlay_nr = (B1 << 4) | (B2 >> 4);

	Icon_data = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);

	/* Any underlay ? */
	if (Underlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Underlay_nr-2].Flags;
		Under_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
	}

	/* Any overlay ? */
	if (Overlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Overlay_nr-2].Flags;
		Over_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
	}

	MEM_Free_pointer(Icondata_handle);

	/* Does the under- or the overlay have a height greater than 0 ? */
	if (Under_h || Over_h)
	{
		/* Yes -> You can pass behind */
		return TRUE;
	}
	else
	{
		/* No -> Don't even try */
		return FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Special_2D_low_check
 * FUNCTION  : Check if an icon in a 2D map can be passed over diagonally.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.94 13:04
 * LAST      : 27.07.95 10:35
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             SISHORT dX - Horizontal component of movement vector.
 *             SISHORT dY - Horizontal component of movement vector.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Special_2D_low_check(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY)
{
	struct Icon_2D_data *Icon_data;
	struct Square_2D *Map_ptr;
	UNLONG Status;
	UNSHORT B1, B2, B3;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;
	UNSHORT Under_h = 0;
	UNSHORT Over_h = 0;
	UNSHORT Frame = 0;
	UNSHORT Pixel;
	UNSHORT Side;

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return FALSE;

	/* Read bytes from map */
	Map_ptr = (struct Square_2D *) (MEM_Claim_pointer(Map_handle)
	 + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);

	B1 = (UNSHORT) Map_ptr->m[0];
	B2 = (UNSHORT) Map_ptr->m[1];
	B3 = (UNSHORT) Map_ptr->m[2];

	MEM_Free_pointer(Map_handle);

	/* Build overlay and underlay number */
	Underlay_nr = ((B2 & 0x0F) << 8) | B3;
	Overlay_nr = (B1 << 4) | (B2 >> 4);

	Icon_data = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);

	/* Any underlay ? */
	if (Underlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Underlay_nr-2].Flags;
		Under_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
	}

	/* Any overlay ? */
	if (Overlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Overlay_nr-2].Flags;
		Over_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;

		/* And first frame number */
		Frame = Icon_data[Overlay_nr-2].First_frame;
	}

	MEM_Free_pointer(Icondata_handle);

	/* Exit if there is no overlay */
	if (Overlay_nr < 2)
		return FALSE;

	/* Does the under- or the overlay have a height greater than 1 ? */
	if ((Under_h > 1) || (Over_h > 1))
	{
		/* Yes -> Forget it */
		return FALSE;
	}
	else
	{
		/* No -> Test the bottom pixel of the overlay */
		Icon_graphics_ptr = MEM_Claim_pointer(Icongfx_handle);

		/* Which side ? */
		if (dY < 0)
		{
			if (dX < 0)
				Side = 0;
			else
				Side = 15;
		}
		else
		{
			if (dX < 0)
				Side = 15;
			else
				Side = 0;
		}

		/* Get the correct pixel */
		Pixel = *(Icon_graphics_ptr + (256 * Frame) + (15 * 16) + Side);

		MEM_Free_pointer(Icongfx_handle);

		/* Is it transparent ? */
		if (Pixel)
		{
			/* No -> Forget it */
			return FALSE;
		}
		else
		{
			/* Yes -> Pass over it */
			return TRUE;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Double_movement_check_2D
 * FUNCTION  : Check if a certain position in a 2D map can be moved to.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 26.11.94 16:38
 * INPUTS    : UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             SISHORT dX - delta X.
 *             SISHORT dY - delta Y.
 *             UNSHORT Travel_mode - Travel mode.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - This function will also check the square next to the given
 *              location in 2D maps where big characters are shown.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Double_movement_check_2D(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY,
 UNSHORT Travel_mode, UNSHORT NPC_index)
{
	/* Any movement ? */
	if (dX || dY)
	{
		/* Yes -> Check for collision with NPC's */
		if (!(Check_2D_NPC_collision(X + dX, Y + dY, NPC_index)))
			return FALSE;
	}

	/* Big guy ? */
	if (Big_guy)
	{
		/* Yes -> Check two squares */
		if (!(Do_check_2D(X, Y, dX, dY, Travel_mode, NPC_index)
		 & Do_check_2D(X + 1, Y, dX, dY, Travel_mode, NPC_index)))
		{
			return FALSE;
		}

		/* Moving vertically ? */
		if (dY)
		{
			/* Yes -> Extra check */
			if (!Extra_vertical_movement_check_2D(X + dX, Y + dY, NPC_index))
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	else
	{
		/* No -> Check one square */
		return(Do_check_2D(X, Y, dX, dY, Travel_mode, NPC_index));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Extra_vertical_movement_check_2D
 * FUNCTION  : Check if a vertical movement in a large-scale 2D map is possible.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.11.94 16:38
 * LAST      : 26.11.94 16:38
 * INPUTS    : UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - This function checks if there's a "wall" between the two
 *              target squares.
 *             - Only call this when moving vertically.
 *             - The function can handle small-scale maps by itself.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Extra_vertical_movement_check_2D(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	UNLONG StatusL, StatusR;

	/* Large-scale ? */
	if (Big_guy)
	{
		/* Yes -> Do extra check for both target squares */
		StatusL = Get_location_status(X, Y, NPC_index);
		StatusR = Get_location_status(X + 1, Y, NPC_index);

		if ((StatusL & (1 << (BLOCKED_DIRECTIONS_B + EAST)))
		 || (StatusR & (1 << (BLOCKED_DIRECTIONS_B + WEST))))
			return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_check_2D
 * FUNCTION  : Check if a certain position in a 2D map can be moved to.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 06.09.94 17:54
 * INPUTS    : UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             SISHORT dX - X-component of movement vector.
 *             SISHORT dY - Y-component of movement vector.
 *             UNSHORT Travel_mode - Travel mode.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Do_check_2D(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY,
 UNSHORT Travel_mode, UNSHORT NPC_index)
{
	static UNSHORT Directions[3][3] = {
		{7, 0, 1},
		{6, 8, 2},
		{5, 4, 3}
	};
	UNLONG Status;
	UNSHORT State;
	UNSHORT Dir1, Dir2;
	UNSHORT X2, Y2;

	/* Exit if no movement */
	if (!dX && !dY)
		return TRUE;

	/* Calculate target coordinates */
	X2 = X + dX;
	Y2 = Y + dY;

	/* Exit if target coordinates lie outside the map */
	if ((X2 < 1) || (X2 > Map_width) || (Y2 < 1) || (Y2 > Map_height))
		return FALSE;

	/* Exit if cheat mode is on / not NPC */
	if ((NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	/* Get direction */
	Dir1 = Directions[sgn(dY) + 1][sgn(dX) + 1];

	/* Get the original location's status */
	Status = Get_location_status(X, Y, NPC_index);

	/* Check if the original location can be left in this direction */
	if ((Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir1][0])))
	 && (Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir1][1]))))
		return FALSE;

	/* Get the new location's status */
	Status = Get_location_status(X2, Y2, NPC_index);

	/* Checking the party ? */
	if (NPC_index == 0xFFFF)
	{
		/* Yes -> Get state */
		State = (Status & STATE) >> STATE_B;

		/*  Sitting or sleeping state ? */
		if ((State == SITTING_NL_STATE) || (State == SITTING_NR_STATE) ||
		 (State == SITTING_E_STATE) || (State == SITTING_SL_STATE) ||
		 (State == SITTING_SR_STATE) || (State == SITTING_W_STATE) ||
		 (State == SLEEPING1_STATE) || (State == SLEEPING2_STATE))
		{
			/* Yes -> Exit */
			return FALSE;
		}
	}

	/* Get reverse direction */
	Dir2 = (Dir1 + 4) & 0x0007;

	/* Check if the new location can be entered from this direction */
	if ((Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir2][0])))
	 && (Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir2][1]))))
		return FALSE;

	/* Check if the new location is accessible for the current travelmode */
	if (Status & (1 << (BLOCKED_TRAVELMODES_B + Travel_mode)))
		return FALSE;
	else
		return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_2D_NPC_collision
 * FUNCTION  : Check if a certain position in the 2D map is blocked by an NPC.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.11.94 12:32
 * LAST      : 08.05.95 14:39
 * INPUTS    : UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : TRUE (no collision) or FALSE (blocked).
 * BUGS      : No known.
 * NOTES     : - This function will also check if an NPC collides with the
 *              party.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_2D_NPC_collision(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	UNSHORT i;

	/* Exit if cheat mode is on / not NPC */
	if ((NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	/* Is NPC ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Collision with party ? */
		if ((PARTY_DATA.X == X) && (PARTY_DATA.Y == Y))
		{
			/* Yes -> Exit */
			return FALSE;
		}
	}

	/* Check if there are any NPCs on this location */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself ? */
		if ((NPC_present(i)) && (i != NPC_index))
		{
			/* Yes -> Collision ? */
			if ((VNPCs[i].Map_X == X) && (VNPCs[i].Map_Y == Y))
			{
				/* Yes -> Exit */
				return FALSE;
			}

			if ((VNPCs[i].Next_X == X) && (VNPCs[i].Next_Y == Y))
			{
				/* Yes -> Exit */
				return FALSE;
			}
		}
	}

	/* Everything's OK */
	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sucking_chairs_2D
 * FUNCTION  : Handle sucking chairs in 2D maps.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.12.94 12:23
 * LAST      : 02.12.94 12:23
 * INPUTS    : struct Movement_2D *Move - Pointer to movement data.
 * RESULT    : BOOLEAN : TRUE (no collision) or FALSE (blocked).
 * BUGS      : No known.
 * NOTES     : - This function may change coordinates, movement vector and
 *              state.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Sucking_chairs_2D(struct Movement_2D *Move)
{
	struct Movement_2D Extra;
	BOOLEAN Update = FALSE, Result = TRUE;
	UNSHORT X, Y;
	UNSHORT Current_state, Left_state = 0, Right_state = 0;
	SISHORT dX, dY;

	/* Get movement vector */
	dX = Move->dX;
	dY = Move->dY;

	/* Exit if no movement */
	if (!dX && !dY)
		return TRUE;

	/* Get destination map coordinates */
	X = Move->X + dX;
	Y = Move->Y + dY;

	/* Copy movement data */
	memcpy(&Extra, Move, sizeof(struct Movement_2D));

	/* Set new coordinates */
	Extra.X = X;
	Extra.Y = Y;

	/* Don't suck ! */
	Extra.Flags |= DONT_SUCK;

	/* Big guy ? */
	if (Big_guy)
	{
		/* Yes -> Get current state */
		Current_state = Move->State;

		/* Get state at destination location and the location to the right */
		Left_state = ((Get_location_status(X, Y, Move->NPC_index) & STATE) >>
		 STATE_B);
		Right_state = ((Get_location_status(X + 1, Y, Move->NPC_index) &
		 STATE) >> STATE_B);

		/* Currently sitting on an east- or west-chair, but the new left foot
		 state is standing ? */
		if (((Current_state == SITTING_W_STATE) || (Current_state == SITTING_E_STATE))
		 && (Left_state == STANDING_STATE))
		{
			/* Yes -> Try to jump off */
			Extra.dX = dX;
			Extra.dY = 0;

			/* Determine jump direction */
			if (dX < 0)
				Extra.Direction = WEST8;
			else
				Extra.Direction = EAST8;

			/* Possible ? */
			if (Try_2D_move(&Extra))
			{
	  			/* Yes -> Set new coordinates */
				Move->X = X + Extra.dX;
				Move->Y = Y + Extra.dY;
				Update = TRUE;

				/* Set states */
				Left_state = Right_state = STANDING_STATE;
			}
			else
			{
				/* No */
				Result = FALSE;
			}

			/* Clear movement vector */
			dX = 0;
			dY = 0;
		}

		/* Left foot is on a sitting-east or -west state ? */
		if ((Left_state == SITTING_E_STATE) || (Left_state == SITTING_W_STATE))
		{
			/* Yes -> Just jump */
			Move->X = X;
			Move->Y = Y;
			dX = 0;
			dY = 0;
			Update = TRUE;
		}

		/* Right foot is on a sitting-east or -west state ? */
		if ((Right_state == SITTING_E_STATE) || (Right_state == SITTING_W_STATE))
		{
			/* Yes -> Jump right */
			Move->X = X + 1;
			Move->Y = Y;
			dX = 0;
			dY = 0;
			Update = TRUE;

			/* Set state */
			Left_state = Right_state;
		}

		/* Is a foot on the correct north- or south-state ? */
		if ((Left_state == SITTING_NL_STATE) || (Right_state == SITTING_NR_STATE)
		 || (Left_state == SITTING_SL_STATE) || (Right_state == SITTING_SR_STATE))
		{
			/* Yes -> Just jump */
			Move->X = X;
			Move->Y = Y;
			dX = 0;
			dY = 0;
			Update = TRUE;
		}
		else
		{
			/* No -> Currently sitting on a north- or south-chair ? */
			if ((Current_state == SITTING_NL_STATE) || (Current_state == SITTING_NR_STATE)
			 || (Current_state == SITTING_SL_STATE) || (Current_state == SITTING_SR_STATE))
			{
				/* Yes -> New left or right foot state is standing ? */
				if ((Left_state == STANDING_STATE) || (Right_state == STANDING_STATE))
				{
					/* Yes -> Try to jump off */
					Extra.dX = dX;
					Extra.dY = 0;

					/* Determine jump direction */
					if (dX < 0)
						Extra.Direction = WEST8;
					else
						Extra.Direction = EAST8;

					/* Possible ? */
					if (Try_2D_move(&Extra))
					{
						/* Yes -> Set new coordinates */
						Move->X = X + Extra.dX;
						Move->Y = Y + Extra.dY;
						Update = TRUE;

						/* Set states */
						Left_state = Right_state = STANDING_STATE;
					}
					else
					{
						/* No */
						Result = FALSE;
					}

					/* Clear movement vector */
					dX = 0;
					dY = 0;
				}
			}

			/* Moving right or vertically and the right foot is on a
			 sitting-left state ? */
			if (((dX > 0) || (dY)) && ((Right_state == SITTING_NL_STATE)
			 || (Right_state == SITTING_SL_STATE)))
			{
				/* Yes -> Jump right */
				Move->X = X + 1;
				Move->Y = Y;
				dX = 0;
				dY = 0;
				Update = TRUE;

				/* Set states */
				Left_state = Right_state;
				Right_state++;
			}

			/* Moving left or vertically and the left foot is on a
			 sitting-right state ? */
			if (((dX < 0) || (dY)) && ((Left_state == SITTING_NR_STATE)
			 || (Left_state == SITTING_SR_STATE)))
			{
				/* Yes -> Jump left */
				Move->X = X - 1;
				Move->Y = Y;
				dX = 0;
				dY = 0;
				Update = TRUE;

				/* Set states */
				Right_state = Left_state;
				Left_state--;
			}
		}
	}

	/* Store new state */
	Move->State = Left_state;

	/* Set flag if update is needed */
	if (Update)
		Move->Flags |= UPDATE_AFTER_MOVE;

	/* Store new movement vector */
	Move->dX = dX;
	Move->dY = dY;

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_2D
 * FUNCTION  : Move the party along a vector in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 27.07.95 10:36
 * INPUTS    : SISHORT dX - X-component of vector.
 *             SISHORT dY - Y-component of vector.
 *             BOOLEAN Update - If true, the party path is updated even if
 *              there is no movement.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_2D(SISHORT dX, SISHORT dY, BOOLEAN Update)
{
	/* Any movement ? */
	if (dX || dY || Update || (Party_path[Party_path_index].View_direction !=
	 PARTY_DATA.View_direction))
	{
		/* Yes -> Store movement in path */
		Store_2D_movement(dX, dY);

		/* Another 16 steps must be taken */
		_2D_steps_left += 16;

		for (;;)
		{
			/* Update party positions */
			Update_2D_party_position(TRUE);
			Update_time((_2D_speed * 100) / 2);
			Handle_2D_NPCs();

			/* Reduce steps left */
			_2D_steps_left -= _2D_speed;

			/* Display 2D map */
			Display_2D_map();

			/* Last step ? */
			if (_2D_steps_left >= _2D_speed)
			{
				/* No -> Show movement */
				Switch_screens();
			}
			else
			{
				/* Yes -> break */
				break;
			}
		}

		/* Just to be sure */
		if (_2D_steps_left < 0)
			_2D_steps_left = 0;

		/* Update map coordinates */
		PARTY_DATA.X += dX;
		PARTY_DATA.Y += dY;
	}
	else
	{
		/* No -> Update anyway ? */
		if (Update)
		{
			/* Yes -> Store movement in path */
			Store_2D_movement(dX, dY);

			/* Another 16 steps must be taken */
			_2D_steps_left += 16;

			/* Make a path entry */
			Make_2D_path_entry((PARTY_DATA.X - 1) * 16,
			 (PARTY_DATA.Y - 1) * 16 + 15);

			/* One more step */
//			_2D_steps_left++;
		}
		else
		{
			/* No -> But did the view direction change ? */
			if ((Party_path[Party_path_index].View_direction !=
			 PARTY_DATA.View_direction) && (Active_member_state ==
			 STANDING_STATE))
			{
				/* Yes -> Make a path entry */
				Make_2D_path_entry(Party_path[Party_path_index].X,
				 Party_path[Party_path_index].Y);

				/* One more step */
				_2D_steps_left++;
				// Update_time(100);
			}
		}

		/* No -> Any steps left ? */
		if (_2D_steps_left > 0)
		{
			/* Yes -> Take final steps */
			Update_2D_party_position(TRUE);
			Update_time((_2D_steps_left * 100) / 2);
			Handle_2D_NPCs();
			_2D_steps_left = 0;
		}
		else
		{
			/* No -> Update other party members */
			Update_2D_party_position(FALSE);
			Update_time((_2D_speed * 100) / 2);
			Handle_2D_NPCs();
		}

		/* Show movement */
		Display_2D_map();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_party_position
 * FUNCTION  : Update the positions of the party objects in the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.11.94 12:54
 * LAST      : 21.11.94 12:55
 * INPUTS    : BOOLEAN Movement - TRUE or FALSE.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_party_position(BOOLEAN Movement)
{
	UNSHORT Active_member, i;
	UNSHORT Active_idx, Previous_idx, Current_idx;
	SISHORT Current_distance, Max_distance, Diff;

	Active_member = PARTY_DATA.Active_member - 1;

	/* Get active member's path index */
	Active_idx = M2_Party[Active_member].Path_index;

	/* Calculate distance between active member's and party path index */
	if (Active_idx <= Party_path_index)
		Current_distance = Party_path_index - Active_idx;
	else
		Current_distance = (Party_path_index + PARTY_PATH_LENGTH) - Active_idx;

	/* Increase active member's path index */
	if (Current_distance >= _2D_speed)
		Active_idx += _2D_speed;
	else
		Active_idx += Current_distance;

	if (Active_idx >= PARTY_PATH_LENGTH)
		Active_idx -= PARTY_PATH_LENGTH;

	/* Store active member's path index */
	M2_Party[Active_member].Path_index = Active_idx;

	/* Set current party object coordinates */
	Party_object_X = Party_path[Active_idx].X;
	Party_object_Y = Party_path[Active_idx].Y;

	/* Follow the leader */
	Previous_idx = Active_idx;

	/* Handle other party members */
	for (i=0;i<6;i++)
	{
		/* Anyone here / not the active member ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != Active_member))
		{
			/* Yes -> Moving */
			if (Movement)
			{
				/* Yes -> Time to change the path distance ? */
				if (!M2_Party[i].Counter)
				{
					/* Yes -> Set new path distance */
					M2_Party[i].Path_distance = (4 * _2D_scale) + (rand() % 3) - 1;

					/* Reset counter */
					M2_Party[i].Counter = 4 + (rand() & 0x001F);
				}
				else
				{
					/* No -> Count down */
					M2_Party[i].Counter--;
				}
			}

			/* Get path index and distance to predecessor */
			Current_idx = M2_Party[i].Path_index;
			if (Current_idx <= Previous_idx)
				Current_distance = Previous_idx - Current_idx;
			else
				Current_distance = (Previous_idx + PARTY_PATH_LENGTH) - Current_idx;

			/* Get maximum distance */
			Max_distance = M2_Party[i].Path_distance;
			if (Movement)
				Max_distance = (3 * Max_distance) / 2;

			/* Time to catch up ? */
			if (Current_distance > Max_distance)
			{
				/* Yes -> Calculate difference */
				Diff = Current_distance - Max_distance;

				/* Greater than movement speed ? */
				if (Diff > _2D_speed)
				{
					/* Yes -> Hurry up */
					Current_idx += _2D_speed;
				}
				else
				{
					/* No -> Just do it */
					Current_idx += Diff;
				}

				/* Check for overflow */
				if (Current_idx >= PARTY_PATH_LENGTH)
					Current_idx -= PARTY_PATH_LENGTH;

				/* Store new path index */
				M2_Party[i].Path_index = Current_idx;
			}

			/* Follow me */
			Previous_idx = Current_idx;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Store_2D_movement
 * FUNCTION  : Store a movement of the party in the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.11.94 21:42
 * LAST      : 20.11.94 21:42
 * INPUTS    : SISHORT dX - X-component of movement vector.
 *             SISHORT dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Exactly 16 steps will be taken.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Store_2D_movement(SISHORT dX, SISHORT dY)
{
	UNSHORT i;

	/* Take 16 steps */
	for (i=1;i<=16;i++)
	{
		/* Make a path entry */
		Make_2D_path_entry((PARTY_DATA.X - 1) * 16 +	(dX * i),
		 (PARTY_DATA.Y - 1) * 16 + 15 + (dY * i));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_2D_path_entry
 * FUNCTION  : Make a single entry in the party's path in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.07.95 16:44
 * LAST      : 24.07.95 16:44
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_2D_path_entry(UNSHORT X, UNSHORT Y)
{
	UNSHORT Index;

	/* Get party path index */
	Index = Party_path_index;

	/* Increase path index */
	Index++;
	if (Index >= PARTY_PATH_LENGTH)
		Index -= PARTY_PATH_LENGTH;

	/* Store coordinates and view direction */
	Party_path[Index].X					= X;
	Party_path[Index].Y					= Y;
	Party_path[Index].View_direction	= PARTY_DATA.View_direction;
	Party_path[Index].State				= Active_member_state;

	/* Store new party path index */
	Party_path_index = Index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_party
 * FUNCTION  : Display party members in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.11.94 22:00
 * LAST      : 07.12.94 11:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_party(void)
{
	UNSHORT i, idx;
	UNSHORT Frame;
	SISHORT dX, dY;

	/* Handle party member objects */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Get path index */
			idx = M2_Party[i].Path_index;

			/* Has this object moved ? */
			dX = Party_objects[i].X - Party_path[idx].X;
			dY = Party_objects[i].Y - Party_path[idx].Y;

			/* Set object coordinates */
			Party_objects[i].X = Party_path[idx].X;
			Party_objects[i].Y = Party_path[idx].Y;

			/* Determine animation frame depending on state */
			switch (Party_path[idx].State)
			{
				/* "Special" states */
				case SITTING_NL_STATE:
				{
					Frame = 12;
					Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_NR_STATE:
				{
					Frame = 12;
					Party_objects[i].X -= 16;
					Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_E_STATE:
				{
					Frame = 13;
					Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_SL_STATE:
				{
					Frame = 14;
					Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_SR_STATE:
				{
					Frame = 14;
					Party_objects[i].X -= 16;
					Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_W_STATE:
				{
					Frame = 15;
					Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				/* Normal state : walking */
				default:
				{
					/* Frame depends on view direction and animation phase */
					Frame = Party_path[idx].View_direction * 3
					 + Party_animation[M2_Party[i].Frame];

					Party_objects[i].Flags &= ~ADD_ICONS_OVER_OBJECT;

					/* Is this the active member ? */
					if (i != PARTY_DATA.Active_member - 1)
					{
						/* No -> Slight correction */
						Party_objects[i].Y--;
						dY++;
					}

					break;
				}
			}

			/* Set graphics offset */
			Party_objects[i].Graphics_offset = Frame * (Party_objects[i].Width
			 * Party_objects[i].Height) + sizeof(struct Gfx_header);

			/* Add party object to list */
			Add_2D_object(&Party_objects[i]);

			/* Should we animate ? */
			if (dX || dY)
			{
				/* Yes -> Next animation frame */
				M2_Party[i].Frame = (M2_Party[i].Frame + 1) % 12;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_2D_party_member
 * FUNCTION  :	Search for a party member in the current map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 13:42
 * LAST      : 22.12.94 13:42
 * INPUTS    : UNSHORT X - Map X-coordinate.
 *             UNSHORT Y - Map Y-coordinate.
 * RESULT    : UNSHORT : Index of found party member / 0xFFFF means nothing
 *              was found.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_2D_party_member(UNSHORT X, UNSHORT Y)
{
	UNSHORT Found_member = 0xFFFF;
	UNSHORT i;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0xFFFF;

	/* Convert coordinates */
	X = (X - 1) * 16 + 7;
	Y = (Y - 1) * 16 + 7;

	/* Search party members */
	for(i=0;i<6;i++)
	{
		/* Anyone there / not checking active member ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> In the right place ? */
			if ((X >= Party_objects[i].X) && (Y <= Party_objects[i].Y) &&
			 (X < Party_objects[i].X + Party_objects[i].Width) &&
			 (Y > Party_objects[i].Y - Party_objects[i].Height))
			{
				/* Yes -> Did we already find a party member ? */
				if (Found_member != 0xFFFF)
				{
					/* Yes -> Is this one better ? */
					if (Party_objects[i].Y > Party_objects[Found_member].Y)
					{
						/* Yes -> Mark */
						Found_member = i;
					}
					else
					{
						/* Well, is it ? */
						if ((Party_objects[i].Y == Party_objects[Found_member].Y)
						 && (Party_objects[i].X > Party_objects[Found_member].X))
						{
							/* Yes -> Mark */
							Found_member = i;
						}
					}
				}
				else
				{
					/* No -> Mark */
					Found_member = i;
				}
			}
		}
	}

	/* Find anything ? */
	return Found_member;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_2D_map_to_screen_coordinates
 * FUNCTION  : Convert 2D map to screen coordinates.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 19:46
 * LAST      : 20.06.95 19:46
 * INPUTS    : UNSHORT Map_X - Map X-coordinate (1...).
 *             UNSHORT Map_Y - Map Y-coordinate (1...).
 *             UNSHORT *Screen_X - Pointer to screen X-coordinate.
 *             UNSHORT *Screen_Y - Pointer to screen Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the screen coordinates lie outside of the 2D map window,
 *              the returned coordinates will be 0xFFFF, 0xFFFF.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Convert_2D_map_to_screen_coordinates(UNSHORT Map_X, UNSHORT Map_Y,
 UNSHORT *Screen_X, UNSHORT *Screen_Y)
{
	SILONG pfX, pfY;

	/* Calculate coordinates of map square within the 2D map playfield */
	pfX = (Map_X - 1) * 16;
	pfY = (Map_Y - 1) * 16;

	/* Make coordinates relative to current 2D map scroll position */
	pfX -= Scroll_2D.Playfield_X;
	pfY -= Scroll_2D.Playfield_Y;

	/* Are these coordinates inside the 2D map window ? */
	if ((pfX < MAP_2D_X) || (pfX >= MAP_2D_X + Map_2D_width) ||
	 (pfY < MAP_2D_Y) || (pfY >= MAP_2D_Y + Map_2D_height))
	{
		/* No -> Indicate this */
		pfX = 0xFFFF;
		pfY = 0xFFFF;
	}

	/* Output coordinates */
	*Screen_X = (UNSHORT) pfX;
	*Screen_Y = (UNSHORT) pfY;
}

