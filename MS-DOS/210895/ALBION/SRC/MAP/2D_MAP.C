/************
 * NAME     : 2D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-7-1994
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
#include <2D_PATH.H>
#include <2D_COLL.H>
#include <PRTLOGIC.H>
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
#include <COLOURS.H>

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

static UNSHORT M2_Object;

MEM_HANDLE Icondata_handle;
MEM_HANDLE Icongfx_handle;
MEM_HANDLE Blocklist_handle;

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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : M2_ModInit
 * FUNCTION  : Initialize 2D map module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:14
 * LAST      : 13.08.95 18:20
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
	MEM_HANDLE Handle;
	BOOLEAN Result;
	UNSHORT Icon_set_index;

	/* Initialize located sound effects */
	Map_square_size = 16;
	Initialize_located_effects(Map_width * Map_square_size, Map_height *
	 Map_square_size, 20 * Map_square_size);

	/* Set 2D map window dimensions */
	Map_2D_width = 360;
	Map_2D_height = 192;

	Mapbuf_width = (Map_2D_width + 31) / 16;
	Mapbuf_height = (Map_2D_height + 31) / 16;

	/* Get icon set index */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	Icon_set_index = Map_ptr->ICON_SET_NR;
	MEM_Free_pointer(Map_handle);

	/* Determine scale */
	Big_guy = Big_guy_list[Icon_set_index - 1];

	/* Load icon data */
	Handle = Load_subfile(ICON_DATA, Icon_set_index);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}
	Icondata_handle = Handle;

	/* Load icon graphics */
	Handle = Load_subfile(ICON_GFX, Icon_set_index);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}
	Icongfx_handle = Handle;

	/* Load block list */
	Handle = Load_subfile(BLOCK_LIST, Icon_set_index);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return;
	}
	Blocklist_handle = Handle;

	/* Set scrolling data */
	Scroll_2D.Update_unit		= Update_2D_unit;
	Scroll_2D.Unit_width			= 16;
	Scroll_2D.Unit_height		= 16;
	Scroll_2D.Viewport_width	= Map_2D_width;
	Scroll_2D.Viewport_height	= Map_2D_height;
	Scroll_2D.Playfield_width	= Map_width * 16 ;
	Scroll_2D.Playfield_height	= Map_height * 16;

	/* Show party */
	Hide_party = FALSE;

	/* Initialize party and NPCs */
	Result = Init_2D_party_path();
	if (!Result)
		return;

	Init_2D_NPCs();

	/* Reset 2D object list */
	Clear_2D_object_list();

	/* Add party objects */
	Display_2D_party();

	/* Add NPC objects */
	Display_2D_NPCs();

	/* Prepare 2D display */
	Prepare_2D_display();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	M2_ModExit
 * FUNCTION  : Exit 2D map module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:00
 * LAST      : 08.08.95 15:13
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
	/* Destroy scroll data */
	Exit_scroll(&Scroll_2D);

	Exit_2D_NPCs();

	Exit_2D_party_path();

	/* Free memory */
	MEM_Free_memory(Icondata_handle);
	MEM_Free_memory(Blocklist_handle);
	MEM_Free_memory(Icongfx_handle);
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

	if (!Map_display_initialized)
	{
		/* Initialize map display */
		Init_map_display();

		/* Add OPMs to update list */
		for (i=0;i<4;i++)
		{
			Add_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
		}

		/* Add interface object */
		M2_Object = Add_object(Earth_object, &M2_Class, NULL, 0, 0, Map_2D_width,
		 Map_2D_height);

		Map_display_initialized = TRUE;
	}
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

	if (Map_display_initialized)
	{
		/* Delete interface object */
		Delete_object(M2_Object);

		/* Remove OPMs from update list */
		for (i=0;i<4;i++)
		{
			Remove_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
		}

		/* Exit map display */
		Exit_map_display();

		Map_display_initialized = FALSE;
	}
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
 * NOTES     : - This function is called from other main loop functions!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
M2_MainLoop(void)
{
	/* Display map */
	Update_display();

	/* Update party positions */
	Update_2D_party_position();

	/* Update NPC positions */
	Handle_2D_NPCs();

	/* Reduce steps left */
	_2D_steps_left = max(0, _2D_steps_left - _2D_speed);

	/* Update time */
	Update_time((_2D_speed * 100) / 2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_map
 * FUNCTION  : Display the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.94 12:18
 * LAST      : 10.08.95 12:04
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

	/* Set 3D position of listener */
	Set_listener_position(PARTY_DATA.X * 16, PARTY_DATA.Y * 16, 0);

	/* Reset 2D object list */
	Clear_2D_object_list();

	/* Add party objects */
	Display_2D_party();

	/* Add NPC objects */
	Display_2D_NPCs();

	/* Update display */
	Draw_2D_map();

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
 * LAST      : 10.08.95 12:06
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
		/* Yes -> Move */
		Mouse_move_2D();

		/* Display map */
		M2_MainLoop();
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
		{
			Make_2D_move(NORTH_WEST);
			break;
		}
		/* Try to move up */
		case 1:
		{
			Make_2D_move(NORTH8);
			break;
		}
		/* Try to move up-right */
		case 2:
		{
			Make_2D_move(NORTH_EAST);
			break;
		}
		/* Try to move left */
		case 3:
		{
			Make_2D_move(WEST8);
			break;
		}
		/* No action */
		case 4:
		{
			break;
		}
		/* Try to move right */
		case 5:
		{
			Make_2D_move(EAST8);
			break;
		}
		/* Try to move down-left */
		case 6:
		{
			Make_2D_move(SOUTH_WEST);
			break;
		}
		/* Try to move down */
		case 7:
		{
			Make_2D_move(SOUTH8);
			break;
		}
		/* Try to move down-right */
		case 8:
		{
			Make_2D_move(SOUTH_EAST);
			break;
		}
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
 * LAST      : 09.08.95 11:16
 * INPUTS    : SISHORT X - X-coordinate of mouse.
 *             SISHORT Y - Y-coordinate of mouse.
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
	pX = Party_object_X - Scroll_2D.Playfield_X;
	pY = Party_object_Y - Scroll_2D.Playfield_Y - pH;

	/* Determine state depending on position relative to party leader */
	Q = 0;
	if (X >= max(pX, 4))
	{
		if (X < min(pX + pW, Map_2D_width - 3))
			Q = 1;
		else
			Q = 2;
	}
	if (Y >= max(pY, 4))
	{
		if (Y < min(pY + pH, Map_2D_height - 3))
			Q += 3;
		else
			Q += 6;
	}

	return Q;
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
 *             SISHORT *Screen_X - Pointer to screen X-coordinate.
 *             SISHORT *Screen_Y - Pointer to screen Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Convert_2D_map_to_screen_coordinates(UNSHORT Map_X, UNSHORT Map_Y,
 SISHORT *Screen_X, SISHORT *Screen_Y)
{
	SILONG pfX, pfY;

	/* Calculate coordinates of map square within the 2D map playfield */
	pfX = (Map_X - 1) * 16;
	pfY = (Map_Y - 1) * 16;

	/* Make coordinates relative to current 2D map scroll position */
	pfX -= Scroll_2D.Playfield_X;
	pfY -= Scroll_2D.Playfield_Y;

	/* Output coordinates */
	*Screen_X = (UNSHORT) pfX;
	*Screen_Y = (UNSHORT) pfY;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Map_to_2D
 * FUNCTION  : Convert map to 2D coordinates.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.08.95 11:31
 * LAST      : 14.08.95 07:34
 * INPUTS    : SISHORT Map_X - Map X-coordinate.
 *             SISHORT Map_Y - Map Y-coordinate.
 *             UNLONG *X_2D_ptr - Pointer to 2D X-coordinate.
 *             UNLONG *Y_2D_ptr - Pointer to 2D Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Map_to_2D(SISHORT Map_X, SISHORT Map_Y, UNLONG *X_2D_ptr,
 UNLONG *Y_2D_ptr)
{
	/* Are the coordinates inside the map ? */
	if ((Map_X > 0) && (Map_X <= Map_width) && (Map_Y > 0) &&
	 (Map_Y <= Map_height))
	{
		/* Yes -> Convert map to 2D coordinates */
		*X_2D_ptr = (Map_X - 1) * 16;
		*Y_2D_ptr = (Map_Y - 1) * 16 + 15;
	}
	else
	{
		/* No -> Set dummy coordinates */
		*X_2D_ptr = 0;
		*Y_2D_ptr = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : _2D_to_map
 * FUNCTION  : Convert 2D to map coordinates.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.08.95 11:31
 * LAST      : 14.08.95 07:36
 * INPUTS    : UNLONG X_2D - 2D X-coordinate.
 *             UNLONG Y_2D - 2D Y-coordinate.
 *             SISHORT *Map_X_ptr - Pointer to map X-coordinate.
 *             SISHORT *Map_Y_ptr - Pointer to map Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
_2D_to_map(UNLONG X_2D, UNLONG Y_2D, SISHORT *Map_X_ptr,
 SISHORT *Map_Y_ptr)
{
	SISHORT Map_X, Map_Y;

	/* Convert 2D to map coordinates */
	Map_X = (SISHORT) (X_2D / 16) + 1;
	Map_Y = (SISHORT) ((Y_2D - 15) / 16) + 1;

	/* Are the coordinates inside the map ? */
	if ((Map_X > 0) && (Map_X <= Map_width) && (Map_Y > 0) &&
	 (Map_Y <= Map_height))
	{
		/* Yes -> Store the map coordinates */
		*Map_X_ptr = Map_X;
		*Map_Y_ptr = Map_Y;
	}
	else
	{
		/* No -> Set dummy coordinates */
		*Map_X_ptr = 0;
		*Map_Y_ptr = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Initialize_2D_position
 * FUNCTION  : Set the position in the 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 01:00
 * LAST      : 13.08.95 01:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Initialize_2D_position(void)
{
	UNLONG Camera_X, Camera_Y;

	/* Reset path */
	Reset_2D_path();

	/* Calculate target camera coordinates */
	Calculate_2D_camera_target_position();

	/* Calculate target camera coordinates */
	Get_2D_camera_target_position(&Camera_X, &Camera_Y);

	/* Set camera position */
	Set_2D_camera_position(Camera_X, Camera_Y);
}

