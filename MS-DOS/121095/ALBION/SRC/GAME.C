/************
 * NAME     : GAME.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

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
#include <MUSIC.H>
#include <USERFACE.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <COMBAT.H>
#include <DIALOGUE.H>
#include <GRAPHICS.H>
#include <COLOURS.H>
#include <MAINMENU.H>

/* defines */

#define NR_GAME_COMMANDS	(12)

/* prototypes */

BOOLEAN Init_sound(void);

void Start_lengthy_memory_operation(void);
void End_lengthy_memory_operation(void);

void Get_INI_variables(void);

/* global variables */

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

UNSHORT Screen_width, Screen_height, Panel_Y;
UNSHORT Forbidden_X, Forbidden_Y;

UNSHORT Earth_object;

const double PI = 3.1415927;

/* Background buffer file type */
static UNCHAR Background_buffer_fname[] = "Background buffer";

struct File_type Background_buffer_ftype = {
	NULL,
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
	{ERROR_NULL_COMOB_PASSED,				"NULL passed as COMOB pointer."},
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
 * LAST      : 28.09.95 14:32
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

	/* Initialize data paths */
	Init_data_paths();
	Search_second_data_path();

	/* Initialize screen */
	Result = Init_screen();
	if (!Result)
		goto EXIT_GAME;

	/* Install BBMEM operation handlers */
	MEM_Critical_error_handler				= Critical_error_handler;
	MEM_Start_lengthy_operation_handler	= Start_lengthy_memory_operation;
	MEM_End_lengthy_operation_handler	= End_lengthy_memory_operation;

	/* Initialize sound */
	Result = Init_sound();
	if (!Result)
		goto EXIT_GAME;

	/* Calculate various game tables */
	Result = Calculate_game_tables();
	if (!Result)
		goto EXIT_GAME;

	/*** START THE GAME ***/
	First_time_flag	= TRUE;
	Restart_game_flag	= FALSE;
	Quit_program_flag	= FALSE;
	Loading_game		= FALSE;

	do
	{
		/* Reset game stacks */
		Reset_root_stack();
		Reset_module_stack();
		Reset_MA_stack();

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

		/* Initialize map */
		Result = Init_map();
		if (!Result)
			break;

		/* Clear flags */
		Loading_game		= FALSE;
		Restart_game_flag	= FALSE;

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

		/* Kill! Kill! Kill! */
		MEM_Armageddon();
	}
	/* Restart with another saved game ? */
	while (Restart_game_flag);

EXIT_GAME:

	/* Exit sound and vision */
	Exit_sound_system();
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
 * FUNCTION  : Tell main loop to leave the program.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:48
 * LAST      : 26.07.94 16:48
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
 * LAST      : 10.10.95 18:48
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_screen
 * FUNCTION  : Initialize the screen.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:05
 * LAST      : 30.09.95 16:56
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

	/* Initialize mouse pointer (palette is changed !) */
	SYSTEM_ShowMousePtr(SYSTEM_MousePointerNormal);

	/* Reset mouse-pointer stack */
	Reset_mouse_stack();

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
		&OPM_ftype
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
		&OPM_ftype
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
 * LAST      : 17.09.95 18:06
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
	Result = Init_sound_system();
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
		Set_MIDI_volume((UNSHORT) atoi(Number));
	}
	else
	{
		/* No -> Reset volume to default */
		Set_MIDI_volume(127);
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
		Set_digital_volume((UNSHORT) atoi(Number));
	}
	else
	{
		/* No -> Reset volume to default */
		Set_digital_volume(127);
	}

	/* Switch sound and located sound effects on */
	Sound_on();
	Located_sound_effects_on();

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
 * LAST      : 22.05.95 13:56
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

	/* Build error report */
	Report.Code = Error_code;
	Report.Messages = &(_Game_errors[0]);
/*	Report.Line_nr = (UNSHORT) Line_nr;
	Report.Filename = (UNCHAR *) Filename; */

	/* Push error on the error stack */
	ERROR_PushError(Print_error, _Game_library_name,
	 sizeof(struct Error_report), (UNBYTE *) &Report);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_error
 * FUNCTION  : Print an error.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.09.94 12:58
 * LAST      : 22.05.95 13:57
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
	UNCHAR String1[150];
//	UNCHAR String2[200];

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
			sprintf(String1, Messages->Message, Report->Value1, Report->Value2);

			#if FALSE
			/* Line number / filename given ? */
			if (Report->Line_nr && Report->Filename)
			{
				/* Yes -> Build complete message with line/file info */
				sprintf(String2, "%s\n(line %u of file %s)", String1,
				 Report->Line_nr, Report->Filename);

				/* Copy to output buffer */
				strncpy((char *) buffer, String2, 149);
			}
			else
			#endif
			{
				/* Yes -> Copy message to output buffer without line/file info */
				strncpy((char *) buffer, String1, 149);
			}

			return;
		}
		/* No -> Next message */
		Messages++;
	}

	/* Not found -> Illegal error code */
	sprintf((char *) buffer, "Illegal error code.");
}

