/************
 * NAME     : GAME.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1994
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
#include <stdio.h>
#include <stdlib.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDOS.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBSYSTEM.H>
#include <BBEVENT.H>
#include <BBOPM.H>
#include <BBMEM.H>

#include <HDOB.H>
#include <XLOAD.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>
#include <TEXT.H>

#include <MAIN.H>
#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <EVELOGIC.H>
#include <PRTLOGIC.H>
#include <ITMLOGIC.H>
#include <2D_MAP.H>
#include <XFTYPES.H>
#include <FONT.H>
#include <STATAREA.H>
#include <SOUND.H>
#include <USERFACE.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <COMBAT.H>
#include <DIALOGUE.H>
#include <GRAPHICS.H>
#include <COLOURS.H>
#include <MAINMENU.H>

/*
 ** Defines ****************************************************************
 */

#define NR_GAME_COMMANDS	(12)

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_sound(void);

void Start_lengthy_memory_operation(void);
void End_lengthy_memory_operation(void);

void Get_INI_variables(void);

/*
 ** Global variables *******************************************************
 */

/* OPMs */
struct OPM Main_OPM;
struct OPM Slab_OPM;
struct OPM Slab_backup_OPM;

/* Screen */
struct SCREENPORT Screen;

/* OPM memory handles */
static MEM_HANDLE Main_OPM_handle;
static MEM_HANDLE Slab_OPM_handle;
static MEM_HANDLE Slab_backup_OPM_handle;

/* Global flags */
BOOLEAN Quit_program_flag;
BOOLEAN Loading_game;
BOOLEAN Restart_game_flag;
BOOLEAN Game_over_flag;
BOOLEAN Cheat_mode;
BOOLEAN Diagnostic_mode;
BOOLEAN Super_VGA;
BOOLEAN No_load_flag;

UNSHORT Language = GERMAN;

UNSHORT Load_game_nr = 0;

UNSHORT Screen_width;
UNSHORT Screen_height;
UNSHORT Panel_Y;
UNSHORT Forbidden_X, Forbidden_Y;

UNSHORT Earth_object;

const double PI = 3.1415927;

/* Background buffer file type */
static UNCHAR Background_buffer_fname[] = "Background buffer";

struct File_type Background_buffer_ftype = {
	MEM_Relocate,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Background_buffer_fname
};

/* Error handling data */
static UNCHAR _Game_library_name[] = "Albion";

static struct Error_message _Game_errors[] = {
	{ERROR_OUT_OF_MEMORY,					"Out of memory."},
	{ERROR_NO_OPM,								"OPM could not be created."},
	{ERROR_NO_SCREEN,							"Screen could not be opened."},
	{ERROR_FILE_LOAD,							"File access error."},
	{ERROR_ITEM_NO_SWAP_BACK,				"An item could not be swapped back."},
	{ERROR_AUTOMAP_FILE,						"Automap file has the wrong length."},
	{ERROR_CLONE_SAME_MAPS,					"Cloned maps have the same number."},
	{ERROR_CLONE_MAPS_NOT_SAME_SIZE, 	"Cloned maps are not the same size."},
	{ERROR_CLONE_MAPS_NOT_SAME_TYPE, 	"Cloned maps are not of the same type."},
	{ERROR_SCROLL_BAR_WRONG_PARAMETERS, "Wrong parameters passed to scrollbar."},
	{ERROR_ILLEGAL_PROJECTILE_TYPE, 		"Illegal projectile type."},
	{ERROR_OUT_OF_COMOBS,					"Out of combat objects."},
	{ERROR_ILLEGAL_COMOB,					"Illegal COMOB used."},
	{ERROR_OUT_OF_COMOB_BEHAVIOURS,		"Out of combat object behaviours."},
	{ERROR_3D_COLOUR_CYCLING_IN_2D_MAP, "3D colour cycling in a 2D map."},
	{ERROR_BIT_ARRAY_ACCESS_TYPE, 		"Illegal bit array access type."},
	{ERROR_BIT_ARRAY_TYPE, 					"Illegal bit array type."},
	{ERROR_BIT_ARRAY_OUT_OF_RANGE, 		"Bit number out of range."},
	{ERROR_ILLEGAL_DIALOGUE_RECURSION, 	"Illegal dialogue recursion."},
	{ERROR_ILLEGAL_COMBAT_RECURSION, 	"Illegal combat recursion."},
	{ERROR_NO_PLACE_IN_PARTY,				"No free place in party."},
	{ERROR_BIT_ARRAY_ILLEGAL_ARRAY,		"Illegal bit array."},
	{ERROR_EMPTY_MONSTER_GROUP,			"Empty monster group."},
	{ERROR_STRANGE_LBM,						"Strange LBM data."},
	{ERROR_ILLEGAL_COMOB_FRAME,			"Illegal COMOB frame."},
	{ERROR_ILLEGAL_SAVE_GAME,				"Illegal save game file."},
	{ERROR_ILLEGAL_SAVE_GAME_VERSION,	"Illegal save game version."},
	{ERROR_TOO_MANY_ITEMS_IN_PACKET,		"Too many items in packet."},
	{ERROR_SETUP_INI_ACCESS_ERROR,		"Could not access SETUP.INI."},
	{ERROR_SAVED_GAME_ACCESS_ERROR,		"Could not access saved game."},
	{ERROR_TOO_MANY_3D_WALLS,				"Too many 3D walls."},
	{ERROR_TOO_MANY_3D_FLOORS,				"Too many 3D floors."},
	{ERROR_TOO_MANY_3D_OBJECTS,			"Too many 3D objects."},
	{ERROR_TOO_MANY_3D_TEXTURES,			"Too many 3D textures."},
	{ERROR_ILLEGAL_X_COORDINATE,			"Illegal X-coordinate."},
	{ERROR_ILLEGAL_Y_COORDINATE,			"Illegal Y-coordinate."},
	{0, NULL}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BBMAIN
 * FUNCTION  : Main function of Albion
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:48
 * LAST      : 03.11.95 20:16
 * INPUTS    : UNSHORT argc - Number of arguments.
 *             UNCHAR **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
BBMAIN(UNSHORT argc, const UNCHAR **argv)
{
	static struct Event_action Game_begins_action =
	{
		0,	NO_ACTOR_TYPE, 0,
		GAME_BEGINS_ACTION, 0, 0,
		NULL, NULL, NULL
	};

	BOOLEAN First_time_flag;
	BOOLEAN Result;

	/* Initialize object pool
	  (must be here to avoid Switch_screens updating objects) */
	Clear_object_pool();

	/* Initialize screen */
	Result = Init_screen();

	/* Success ? */
	if (!Result)
	{
		/* No -> Exit program */
		Main_program_command = EXIT_PROGRAM_COMMAND;
		return;
	}

	/* Initialize sound */
	Result = Init_sound();

	/* Success ? */
	if (!Result)
	{
		/* No -> Exit screen */
		Exit_screen();

		/* Exit program */
		Main_program_command = EXIT_PROGRAM_COMMAND;
		return;
	}

	/* Install BBMEM operation handlers */
	MEM_Critical_error_handler				= Critical_error_handler;
	MEM_Start_lengthy_operation_handler	= Start_lengthy_memory_operation;
	MEM_End_lengthy_operation_handler	= End_lengthy_memory_operation;

	/* Calculate various game tables */
	Result = Calculate_game_tables();

	/* Success ? */
	if (!Result)
	{
		/* No -> Exit game */
		Main_program_command = EXIT_PROGRAM_COMMAND;
		goto EXIT_GAME;
	}

	/*** START THE GAME ***/
	First_time_flag	= TRUE;
	Restart_game_flag	= FALSE;
	Quit_program_flag	= FALSE;
	Loading_game		= FALSE;

	do
	{
		/* Reset palette fading */
		Reset_palette_fading();

		/* Initialize object pool */
		Clear_object_pool();

		/* Reset game stacks */
		Reset_root_stack();
		Reset_module_stack();
		Reset_MA_stack();
		Reset_HDOBs();

		/* Add earth object */
		Earth_object = Add_object
		(
			0,
			&Earth_Class,
			NULL,
			0,
			0,
			Screen_width,
			Screen_height
		);

		/* Initialize fonts */
		Init_font();

		/* Initialize game texts */
		Init_game_texts();

		/* Initialize status area */
		Init_status_area();

		/* First time ? */
		if (First_time_flag)
		{
			/* Yes -> No longer */
			First_time_flag = FALSE;

			/* Get variables from INI file */
			Get_INI_variables();

			/* Enter the main menu */
			Enter_Main_menu(MAIN_MENU_PROGRAM_START);

			/* Quit ? */
			if (Quit_program_flag)
				break;
		}

		/* Clear flag */
		Game_over_flag    = FALSE;

		/* Initialize game data */
		Result = Init_game_data();
		if (!Result)
			break;

		/* Initialize party data (& load saved game) */
		Result = Init_party_data(Load_game_nr, No_load_flag);
		if (!Result)
			break;

		/* Restarting the game ? */
		if (!Load_game_nr)
		{
			/* Yes -> Game begins event logic */
			Perform_action(&Game_begins_action);
		}

		/* Clear flag
		  (this flag must be cleared before the call to Init_map because it
		   influences the event logic) */
		Restart_game_flag	= FALSE;

		/* Initialize map */
		Result = Init_map();
		if (!Result)
			break;

		/* Clear flag
		  (this flag is cleared here because from this point onward there is no
		   need to check whether the program is loading or operating normally) */
		Loading_game		= FALSE;

		/* This is the global main loop. Each element is followed by a check
		  to see if the program should be ended. */
		for(;;)
		{
			/* Handle input */
			Handle_input();
			if (Quit_program_flag || Restart_game_flag)
				break;

			/* Execute main loop of current module */
			Main_loop();
			if (Quit_program_flag || Restart_game_flag)
				break;

			/* Switch screens */
			Switch_screens();
			if (Quit_program_flag || Restart_game_flag)
				break;
		}

		/* Exit data */
	 	Exit_map();
		Exit_party_data();
		Exit_game_data();

		/* Exit game texts */
		Exit_game_texts();

		/* Exit fonts */
		Exit_font();

		/* Kill! Kill! Kill! */
		MEM_Armageddon();
	}
	/* Restart with another saved game ? */
	while (Restart_game_flag);

EXIT_GAME:

	/* Exit sound and vision */
	SOUND_Exit_sound_system();
	Exit_screen();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_lengthy_memory_operation
 * FUNCTION  : Indicate the start of a lengthy memory operation.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.95 15:59
 * LAST      : 22.07.95 15:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume it will only be called once per
 *              operation.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Start_lengthy_memory_operation(void)
{
	Push_mouse(&(Mouse_pointers[RAM_MPTR]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : End_lengthy_memory_operation
 * FUNCTION  : Indicate the end of a lengthy memory operation.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.95 16:00
 * LAST      : 22.07.95 16:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may assume it will only be called once per
 *              operation.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
End_lengthy_memory_operation(void)
{
	Pop_mouse();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_program
 * FUNCTION  : Leave the program.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:48
 * LAST      : 23.10.95 16:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_program(void)
{
	/* Give exit program command */
	Main_program_command = EXIT_PROGRAM_COMMAND;

	/* Quit */
	Quit_program_flag = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_INI_variables
 * FUNCTION  : Get variables from INI file.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 18:48
 * LAST      : 22.10.95 23:23
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_INI_variables(void)
{
	BOOLEAN Result;
	UNCHAR Number[10];

	/* Read 3D window size variable */
	Result = Read_INI_variable
	(
		"ALBION",
		"3D_WINDOW_SIZE",
		Number,
		10
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Store value */
		Window_3D_size = (UNSHORT) atoi(Number);
	}
	else
	{
		/* No -> Set default value */
		Window_3D_size = 100;
	}

	/* Read combat detail level variable */
	Result = Read_INI_variable
	(
		"ALBION",
		"COMBAT_DETAIL_LEVEL",
		Number,
		10
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Store value */
		Combat_display_detail_level = (UNSHORT) atoi(Number);
	}
	else
	{
		/* No -> Set default value */
		Combat_display_detail_level = DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL;
	}

	/* Read combat message speed variable */
	Result = Read_INI_variable
	(
		"ALBION",
		"COMBAT_MESSAGE_SPEED",
		Number,
		10
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Store value */
		Combat_message_speed = (UNSHORT) atoi(Number);
	}
	else
	{
		/* No -> Set default value */
		Combat_message_speed = DEFAULT_COMBAT_MESSAGE_SPEED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_screen
 * FUNCTION  : Initialize the screen.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:05
 * LAST      : 03.11.95 16:31
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_screen(void)
{
	MEM_HANDLE Handle;
	BOOLEAN Result;
	UNBYTE *Ptr;
	UNBYTE *Ptr2;

	/* Determine screen size */
	Determine_screen_size();

 	/* Allocate memory for main OPM */
	Main_OPM_handle = MEM_Do_allocate
	(
		Screen_width * Screen_height,
		(UNLONG) &Main_OPM,
		&OPM_ftype
	);
	if (!Main_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return FALSE;
	}

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Main_OPM_handle);
	Result = OPM_New
	(
		Screen_width,
		Screen_height,
		1,
		&Main_OPM,
		Ptr
	);
	MEM_Free_pointer(Main_OPM_handle);

	if (!Result)
	{
		Error(ERROR_NO_OPM);
		return FALSE;
	}

	/* Initialize palette structure */
	Palette.entries = 256;
	Palette.version = 0;

	/* Open the screen */
	Result = Open_screen();
	if (!Result)
	{
		return FALSE;
	}

	/* Init mouse-pointer */
	Init_mouse_system();

	/* Load slab graphics */
	Handle = Load_file(SLAB);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Generate slab OPM */
	Slab_OPM_handle = MEM_Do_allocate
	(
		360 * 240,
		(UNLONG) &Slab_OPM,
		&ROPM_ftype
	);
	if (!Slab_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return FALSE;
	}

	/* Copy slab graphics to OPM */
	Ptr = MEM_Claim_pointer(Handle);
	Ptr2 = MEM_Claim_pointer(Slab_OPM_handle);

	OPM_New
	(
		360,
		240,
		1,
		&Slab_OPM,
		Ptr2
	);
	memcpy
	(
		Ptr2,
		Ptr,
		360 * 240
	);

	MEM_Free_pointer(Slab_OPM_handle);

	/* Destroy graphics */
	MEM_Free_pointer(Handle);
	MEM_Kill_memory(Handle);

	/* Generate slab backup OPM */
	Slab_backup_OPM_handle = MEM_Do_allocate
	(
		CHEAT_WIDTH * CHEAT_HEIGHT,
		(UNLONG) &Slab_backup_OPM,
		&ROPM_ftype
	);
	if (!Slab_backup_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return FALSE;
	}

	Ptr = MEM_Claim_pointer(Slab_backup_OPM_handle);
	OPM_New
	(
		CHEAT_WIDTH,
		CHEAT_HEIGHT,
		1,
		&Slab_backup_OPM,
		Ptr
	);
	MEM_Free_pointer(Slab_backup_OPM_handle);

	/* Make a backup of part of the slab */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Slab_backup_OPM,
		Slab_OPM.width - CHEAT_WIDTH,
		Slab_OPM.height - CHEAT_HEIGHT,
		CHEAT_WIDTH,
		CHEAT_HEIGHT,
		0,
		0
	);

	/* Clear screen */
	OPM_FillBox
	(
		&Main_OPM,
		0,
		0,
		Screen_width,
		Screen_height,
		0
	);

	/* Display slab */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		0,
		0,
		360,
		240,
		0,
		0
	);

	Switch_screens();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_screen
 * FUNCTION  : Close the screen.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:07
 * LAST      : 03.11.95 16:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_screen(void)
{
	/* Exit mouse-pointer */
	Exit_mouse_system();

	/* Close the screen */
	Close_screen();

	/* Delete slab OPM and free memory */
	OPM_Del(&Slab_OPM);
	MEM_Free_memory(Slab_OPM_handle);

	/* Delete screen OPM and free memory */
	OPM_Del(&Main_OPM);
	MEM_Free_memory(Main_OPM_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Determine_screen_size
 * FUNCTION  : Determine the size-based screen parameters.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.09.95 18:02
 * LAST      : 17.09.95 18:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will calculate Screen_width, Screen_height,
 *              Panel_Y, Forbidden_X and Forbidden_Y.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Determine_screen_size(void)
{
	/* Determine screen dimensions */
	if (Super_VGA)
	{
		Screen_width = 640;
		Screen_height = 480;
		Panel_Y = 384;

		Forbidden_X = 350;
		Forbidden_Y = 384;
	}
	else
	{
		Screen_width = 360;
		Screen_height = 240;
		Panel_Y = 192;

/*		Screen_width = 320;
		Screen_height = 200;
		Panel_Y = 152; */

		Forbidden_X = 181;
		Forbidden_Y = Panel_Y + 3;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_screen
 * FUNCTION  : Open the screen.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.09.95 17:59
 * LAST      : 30.09.95 16:56
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Open_screen(void)
{
	BOOLEAN Result;

	/* Open screen */
	if (Super_VGA)
	{
		Result = DSA_OpenScreen
		(
			&Screen,
			&Main_OPM,
			&Palette,
			NULL,
			0,
			0,
			SCREENTYPE_DOUBLEBUFFER
		);
		if (!Result)
		{
			Error(ERROR_NO_SCREEN);
			return FALSE;
		}
	}
	else
	{
		Result = DSA_OpenScreen
		(
			&Screen,
			&Main_OPM,
			&Palette,
			NULL,
			0,
			0,
			SCREENTYPE_FORCE_CHAIN4 | SCREENTYPE_DOUBLEBUFFER
		);
		if (!Result)
		{
			Error(ERROR_NO_SCREEN);
			return FALSE;
		}
	}

	/* Correction for "improvement" in PCLIB32 */
	global_screenport.screenpalptr = &Palette;

	/* Initialize palette */
	Init_palette();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Close_screen
 * FUNCTION  : Close the screen.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.09.95 17:59
 * LAST      : 17.09.95 17:59
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Close_screen(void)
{
	/* Close the screen */
	DSA_CloseScreen(&Screen);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_sound
 * FUNCTION  : Initialize sound.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.09.95 12:45
 * LAST      : 28.09.95 12:45
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_sound(void)
{
	BOOLEAN Result;
	UNCHAR Number[10];

	/* Initialize sound system */
	Result = SOUND_Init_sound_system();
	if (!Result)
		return FALSE;

	/* Read MIDI volume variable */
	Result = Read_INI_variable
	(
		"ALBION",
		"MIDI_VOLUME",
		Number,
		10
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Set MIDI volume */
		SOUND_Set_MIDI_volume((UNSHORT) atoi(Number));
	}
	else
	{
		/* No -> Reset volume to default */
		SOUND_Set_MIDI_volume(127);
	}

	/* Read digital volume variable */
	Result = Read_INI_variable
	(
		"ALBION",
		"DIGI_VOLUME",
		Number,
		10
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Set digital volume */
		SOUND_Set_digital_volume((UNSHORT) atoi(Number));
	}
	else
	{
		/* No -> Reset volume to default */
		SOUND_Set_digital_volume(127);
	}

	/* Switch sound and located sound effects on */
	SOUND_Sound_on();
	SOUND_Located_sound_effects_on();

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Report_error
 * FUNCTION  : Report an error.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.11.94 11:18
 * LAST      : 29.10.95 19:25
 * INPUTS    : UNSHORT Error_code - Error code.
 *             int Line_nr - Line number where error was reported.
 *             char *Filename - Filename where error was reported.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Report_error(UNSHORT Error_code, int Line_nr, char *Filename)
{
	struct Error_report Report;

	/* Clear error report */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &Report,
		sizeof(struct Error_report),
		0
	);

	/* Build error report */
	Report.Code			= Error_code;
	Report.Messages	= &(_Game_errors[0]);
	Report.Line_nr		= (UNSHORT) Line_nr;
	Report.Filename	= (UNCHAR *) Filename;

	/* Push error on the error stack */
	ERROR_PushError
	(
		Print_error,
		_Game_library_name,
		sizeof(struct Error_report),
		(UNBYTE *) &Report
	);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_error
 * FUNCTION  : Print an error.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 12:58
 * LAST      : 14.10.95 18:36
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error report.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : GAME.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_error(UNCHAR *buffer, UNBYTE *data)
{
	struct Error_report *Report;
	struct Error_message *Messages;
	UNSHORT Code;
	UNCHAR String1[BBERROR_OUTSTRINGSIZE];
	UNCHAR String2[BBERROR_OUTSTRINGSIZE];

	/* Get error data */
	Report = (struct Error_report *) data;
	Code = Report->Code;
	Messages = Report->Messages;

	/* Find error code */
	while (Messages->Code)
	{
		/* Is this the one ? */
		if (Messages->Code == Code)
		{
			/* Yes -> Build message */
			_bprintf
			(
				String1,
				BBERROR_OUTSTRINGSIZE,
				Messages->Message,
				Report->Value1,
				Report->Value2
			);

			/* Line number / filename given ? */
			if (Report->Line_nr && Report->Filename)
			{
				/* Yes -> Build complete message with line/file info */
				_bprintf
				(
					String2,
					BBERROR_OUTSTRINGSIZE,
					"%s\n(line %u of file %s)",
					String1,
					Report->Line_nr,
					Report->Filename
				);

				/* Copy to output buffer */
				strncpy
				(
					buffer,
					String2,
					BBERROR_OUTSTRINGSIZE - 1
				);
			}
			else
			{
				/* No -> Copy message to output buffer without line/file info */
				strncpy
				(
					buffer,
					String1,
					BBERROR_OUTSTRINGSIZE - 1
				);
			}

			/* Insert EOL */
			*(buffer + BBERROR_OUTSTRINGSIZE - 1) = '\0';

			return;
		}
		/* No -> Next message */
		Messages++;
	}

	/* Not found -> Illegal error code */
	_bprintf
	(
		buffer,
		BBERROR_OUTSTRINGSIZE,
		"Illegal error code."
	);
}

