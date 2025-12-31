/************
 * NAME     : 2D_MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* pragmas */

#pragma off (unreferenced);

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
#include <BBSYSTEM.H>

#include <ALBION.H>

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
#include <MAGIC.H>
#include <POPUP.H>
#include <XFTYPES.H>
#include <FONT.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <NPCS.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <SOUND.H>
#include <COLOURS.H>
#include <MAP_PUM.H>
#include <MAINMENU.H>

/* defines */

#define MIN_2D_DISPLAY_TICKS	(3)

/* prototypes */

/* 2D map module */
void M2_ModInit(void);
void M2_ModExit(void);
void M2_DisInit(void);
void M2_DisExit(void);

/* 2D map object methods */
UNLONG Touched_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Left_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Right_2D_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_2D_object(struct Object *Object, union Method_parms *P);

/* 2D map mouse move mode */
void Move_mouse_M2_ModInit(void);
void Move_mouse_M2_ModExit(void);
void Move_mouse_M2_MainLoop(void);

void Mouse_move_2D(void);

/* 2D map key move mode */
void Move_key_M2_ModInit(void);
void Move_key_M2_ModExit(void);
void Move_key_M2_MainLoop(void);

BOOLEAN Key_move_2D(void);

/* 2D map selection mode */
void Select_M2_ModInit(void);
void Select_M2_ModExit(void);
void Select_M2_MainLoop(void);

void Display_M2_selection(UNSHORT X, UNSHORT Y);

UNSHORT Get_2D_mouse_state(SISHORT X, SISHORT Y);

/* 2D light state functions */

void Init_2D_light_state(void);
SISHORT Get_target_2D_light_state(void);
void Update_2D_light_state(void);
void Set_2D_light_state(void);

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

/* 2D map mouse move mode module */
static struct Module Move_mouse_M2_Mod = {
	NO_INPUT_HANDLING, MODE_MOD, MAP_2D_SCREEN,
	Move_mouse_M2_MainLoop,
	Move_mouse_M2_ModInit,
	Move_mouse_M2_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 2D map key move mode module */
static struct Module Move_key_M2_Mod = {
	NO_INPUT_HANDLING, MODE_MOD, MAP_2D_SCREEN,
	Move_key_M2_MainLoop,
	Move_key_M2_ModInit,
	Move_key_M2_ModExit,
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
	{ TOUCHED_METHOD,		Touched_2D_object },
	{ LEFT_METHOD,			Left_2D_object },
	{ RIGHT_METHOD,		Right_2D_object },
	{ CUSTOMKEY_METHOD,	Customkeys_2D_object },
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

static UNSHORT Current_2D_light_state;

static UNLONG _2D_time_start = 0;

BOOLEAN Big_guy;

static SISHORT Big_guy_list[] = {
	FALSE, FALSE, TRUE,  FALSE, TRUE,
	TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
	TRUE
};

static UNSHORT Mice_2D[] = {
	UPLEFT_MPTR,	UP2D_MPTR,		UPRIGHT_MPTR,
	LEFT2D_MPTR,	POP_UP_MPTR,	RIGHT2D_MPTR,
	DOWNLEFT_MPTR,	DOWN2D_MPTR,	DOWNRIGHT_MPTR,
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
 * LAST      : 13.10.95 20:10
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
	SOUND_Initialize_located_effects(20 * Map_square_size);

	/* Set 2D map window dimensions */
	Map_2D_width = 360;
	Map_2D_height = 192;

	Mapbuf_width = (Map_2D_width + 31) / 16;
	Mapbuf_height = (Map_2D_height + 31) / 16;

	/* Get icon set index */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	Icon_set_index = Map_ptr->ICON_SET_NR;
	MEM_Free_pointer(Map_handle);

	/* Clear */
	_2D_time_start = 0;

	/* Determine scale */
	Big_guy = Big_guy_list[Icon_set_index - 1];

	/* Set NPC reaction radius */
	if (Big_guy)
		NPC_reaction_radius = 32;
	else
		NPC_reaction_radius = 16;

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
 * LAST      : 13.10.95 20:40
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

		/* Initialize light state */
		Init_2D_light_state();

		/* Add OPMs to update list */
		for (i=0;i<4;i++)
		{
			Add_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
		}

		/* Add interface object */
		M2_Object = Add_object
		(
			Earth_object,
			&M2_Class,
			NULL,
			0,
			0,
			Map_2D_width,
			Map_2D_height
		);

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
 * LAST      : 31.10.95 15:27
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
M2_MainLoop(void)
{
	SILONG dT;

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

	dT = SYSTEM_GetTicks() - _2D_time_start;

	/* Wait if this machine is too fast */
	if (dT < MIN_2D_DISPLAY_TICKS)
	{
		SYSTEM_WaitTicks(MIN_2D_DISPLAY_TICKS - dT);
	};

	_2D_time_start = SYSTEM_GetTicks();
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
	SOUND_Set_listener_position(PARTY_DATA.X * 16, PARTY_DATA.Y * 16, 0);

	/* Reset 2D object list */
	Clear_2D_object_list();

	/* Add party objects */
	Display_2D_party();

	/* Add NPC objects */
	Display_2D_NPCs();

	/* Update 2D light state */
	Update_2D_light_state();

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
		Push_module(&Move_mouse_M2_Mod);
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
 * LAST      : 19.10.95 18:29
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

	/* Act depending on key-code */
	switch (Key_code)
	{
		/* Enter main menu */
		case BLEV_ESC:
		{
			Enter_Main_menu(MAIN_MENU_MAP);
			break;
		}
		/* Move */
		case BLEV_UP:
		case BLEV_DOWN:
		case BLEV_LEFT:
		case BLEV_RIGHT:
		{
			/* Enter move mode */
			Push_module(&Move_key_M2_Mod);
			break;
		}
		/* No reaction */
		default:
		{
			/* Try party shortcuts */
			Result = (BOOLEAN) Customkeys_party(Object, P);
			break;
		}
	}

	return (UNLONG) Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_mouse_M2_ModInit
 * FUNCTION  : Initialize 2D map mouse move mode module.
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
Move_mouse_M2_ModInit(void)
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
 * NAME      : Move_mouse_M2_ModExit
 * FUNCTION  : Exit 2D map mouse move mode module.
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
Move_mouse_M2_ModExit(void)
{
	/* Remove mouse area */
	Pop_MA();

	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_mouse_M2_MainLoop
 * FUNCTION  : 2D map movement main loop.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.94 10:29
 * LAST      : 28.08.95 23:16
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_mouse_M2_MainLoop(void)
{
	UNSHORT Old_map_nr;

	/* Get old map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Update input handling */
	Update_input();

	/* Is the left mouse button pressed ? */
	if (Button_state & 0x0001)
	{
		/* Yes -> Move */
		Mouse_move_2D();

		/* Exit if no longer in the same map */
		if (PARTY_DATA.Map_nr != Old_map_nr)
			return;

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
 * FUNCTION  : Move in 2D mouse move mode.
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
 * NAME      : Move_key_M2_ModInit
 * FUNCTION  : Initialize 2D map key move mode module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 12:31
 * LAST      : 05.10.95 12:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_key_M2_ModInit(void)
{
	/* Indicate move mode is on */
	Move_mode_flag = TRUE;

	/* Guarantee first movement */
	Key_move_2D();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_key_M2_ModExit
 * FUNCTION  : Exit 2D map key move mode module.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 12:31
 * LAST      : 05.10.95 12:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_key_M2_ModExit(void)
{
	/* Indicate move mode is off */
	Move_mode_flag = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_key_M2_MainLoop
 * FUNCTION  : 2D map key movement main loop.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 12:31
 * LAST      : 05.10.95 12:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_key_M2_MainLoop(void)
{
	BOOLEAN Key_pressed;
	UNSHORT Old_map_nr;

	/* Get old map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Update input handling */
	Update_input();

	/* Move */
	Key_pressed = Key_move_2D();

	/* Was a key pressed ? */
	if (Key_pressed)
	{
		/* Yes -> Exit if no longer in the same map */
		if (PARTY_DATA.Map_nr != Old_map_nr)
			return;

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
 * NAME      : Key_move_2D
 * FUNCTION  : Move in 2D key move mode.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 12:31
 * LAST      : 05.10.95 12:52
 * INPUTS    : None.
 * RESULT    : BOOLEAN : A key was pressed.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Key_move_2D(void)
{
	BOOLEAN Key_pressed = FALSE;
	SISHORT dX = 0;
	SISHORT dY = 0;
	UNSHORT Direction;

	/* Left pressed ? */
	if (pressedkeytab[RAW_LEFT])
	{
		/* Yes -> Set direction to left */
		dX = -1;
	}
	else
	{
		/* No -> Right pressed ? */
		if (pressedkeytab[RAW_RIGHT])
		{
			/* Yes -> Set direction to right */
			dX = 1;
		}
	}

	/* Up pressed ? */
	if (pressedkeytab[RAW_UP])
	{
		/* Yes -> Set direction to up */
		dY = -1;
	}
	else
	{
		/* No -> Down pressed ? */
		if (pressedkeytab[RAW_DOWN])
		{
			/* Yes -> Set direction down */
			dY = 1;
		}
	}

	/* Any movement ? */
	if (dX || dY)
	{
		/* Yes -> A key was pressed */
		Key_pressed = TRUE;

		/* Determine direction */
		Direction = Move_directions[sgn(dY) + 1][sgn(dX) + 1];

		/* Move */
		Make_2D_move(Direction);
	}

	return Key_pressed;
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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_light_state
 * FUNCTION  : Initialise 2D light state.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.10.95 19:51
 * LAST      : 13.10.95 19:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_light_state(void)
{
	/* Calculate target light state and set current light state to target */
	Current_2D_light_state = Get_target_2D_light_state();

	/* Set light state */
	Set_2D_light_state();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_target_2D_light_state
 * FUNCTION  : Calculate target light state in a target map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.10.95 19:46
 * LAST      : 13.10.95 21:30
 * INPUTS    : None.
 * RESULT    : SISHORT : Target light state.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

SISHORT
Get_target_2D_light_state(void)
{
	SISHORT Target_light_state = 0;
	SILONG Light_strength;

	/* Act depending on current light state */
	switch (Current_map_light_state)
	{
		/* Always light */
		case ALWAYS_LIGHT:
		{
			Target_light_state = 100;
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

			Target_light_state = Light_strength;
			break;
		}
		/* Changing light */
		case CHANGING_LIGHT:
		{
			Target_light_state = 100;
			break;
		}
	}

	if (Cheat_mode)
		Target_light_state = 100;

	return Target_light_state;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_light_state
 * FUNCTION  : Update the light state in a 2D map.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.10.95 19:44
 * LAST      : 13.10.95 19:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_light_state(void)
{
	SISHORT Delta;
	SISHORT Target_light_state;

	/* Is this a map with changing light ? */
	if (Current_map_light_state != CHANGING_LIGHT)
	{
		/* No -> Get target light state */
		Target_light_state = Get_target_2D_light_state();

		/* Calculate difference */
		Delta = Target_light_state - Current_2D_light_state;

		/* Any difference ? */
		if (Delta)
		{
			/* Yes -> Change current light state */
			Current_2D_light_state += Delta;

			/* Set light state */
			Set_2D_light_state();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_2D_light_state
 * FUNCTION  : Blend the map palette depending on the 2D light state.
 * FILE      : 2D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.10.95 20:07
 * LAST      : 13.10.95 21:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_2D_light_state(void)
{
	struct BBPALETTE Black_palette;
	BOOLEAN Changed_palette;

	/* Changing light in this map ? */
	if (Current_map_light_state != CHANGING_LIGHT)
	{
		/* No -> Clear black palette */
		BASEMEM_FillMemByte((UNBYTE *) &Black_palette,
		 sizeof(struct BBPALETTE), 0);

		/* Initialize black palette structure */
		Black_palette.entries = 256;

		/* Blend */
		Changed_palette = Blend_palette
		(
			1,
			191,
			Current_2D_light_state,
			&Black_palette,
			&Day_map_palette
		);

		/* Has the palette changed ? */
		if (Changed_palette)
		{
			/* Yes -> Update palette */
			Update_palette();
		}
	}
}

