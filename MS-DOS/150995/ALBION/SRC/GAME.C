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
#include <ANIMATE.H>
#include <STATAREA.H>
#include <MUSIC.H>
#include <USERFACE.H>
#include <GAMETIME.H>
#include <GAMETEXT.H>
#include <COMBAT.H>
#include <DIALOGUE.H>
#include <GRAPHICS.H>
#include <COLOURS.H>

/* defines */

#define NR_GAME_COMMANDS	(12)

/* prototypes */

void Start_lengthy_memory_operation(void);
void End_lengthy_memory_operation(void);

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
BOOLEAN Quit_program_flag	= FALSE;
BOOLEAN Loading_game			= FALSE;
BOOLEAN Restart_game_flag	= FALSE;
BOOLEAN Cheat_mode			= FALSE;
BOOLEAN Diagnostic_mode		= FALSE;
BOOLEAN Super_VGA				= FALSE;

static BOOLEAN Enter_combat_directly_flag = FALSE;

//UNSHORT Nr_auto_notes;

UNSHORT Language = GERMAN;

//UNSHORT Language = ENGLISH;

UNSHORT Load_game_nr = 0;
static BOOLEAN No_load_flag = FALSE;

UNSHORT Screen_width, Screen_height, Panel_Y;
UNSHORT Forbidden_X, Forbidden_Y;

UNSHORT Earth_object;

const double PI = 3.1415927;

/* Error handling data */
static UNCHAR _Game_library_name[] = "Albion";

static struct Error_message _Game_errors[] = {
	{ERROR_OUT_OF_MEMORY,					"Out of memory."},
	{ERROR_NO_OPM,								"OPM could not be created."},
	{ERROR_NO_SCREEN,							"Screen could not be opened."},
	{ERROR_FILE_LOAD,							"File could not be loaded."},
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
 * LAST      : 14.08.95 10:49
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
	struct Event_action Game_begins_action =
	{
		0,	NO_ACTOR_TYPE, 0,
		GAME_BEGINS_ACTION, 0, 0,
		NULL, NULL, NULL
	};
	BOOLEAN Result;

	/* Initialize data paths */
	Init_data_paths();
	Search_second_data_path();

	/* Set initial position */
	PARTY_DATA.Map_nr				= 300;
	PARTY_DATA.X					= 31;
	PARTY_DATA.Y					= 76;
	PARTY_DATA.View_direction	= 1;
	PARTY_DATA.Travel_mode		= ON_FOOT;

	/* Set initial party member */
	PARTY_DATA.Member_nrs[0]		= 1;
	PARTY_DATA.Battle_order[0]		= 1;
	PARTY_DATA.Walking_order[0]	= 1;
	PARTY_DATA.Active_member		= 1;

	/* Set initial game time */
	PARTY_DATA.Hour = 8;

	/* Evaluate arguments */
	Evaluate_game_arguments(argc, (UNCHAR **) argv);

	/* Reset system */
	Result = Init_screen();
	if (!Result)
		goto EXIT_GAME;

	/* Install BBMEM operation handlers */
	MEM_Critical_error_handler				= Critical_error_handler;
	MEM_Start_lengthy_operation_handler	= Start_lengthy_memory_operation;
	MEM_End_lengthy_operation_handler	= End_lengthy_memory_operation;

	/* Initialize sound system */
	Result = Init_sound_system();
	if (!Result)
		goto EXIT_GAME;

	Sound_on();
	Located_sound_effects_on();

  	Init_font();

	/*** START THE GAME ***/
	do
	{
		/* Reset */
		Reset_root_stack();
		Reset_module_stack();
		Reset_MA_stack();

		/* Initialize game data */
		Result = Init_game_data();
		if (!Result)
			goto EXIT_GAME;

		Result = Init_party_data(Load_game_nr, No_load_flag);
		if (!Result)
			goto EXIT_GAME;

		Result = Calculate_game_tables();
		if (!Result)
			goto EXIT_GAME;

		Earth_object = Add_object(0, &Earth_Class, NULL, 0, 0, Screen_width,
		 Screen_height);

		Init_status_area();

		Init_animation();

		Init_time();

		if (!Load_game_nr)
		{
			/* Game begins event logic */
			Perform_action(&Game_begins_action);
		}

		if (Enter_combat_directly_flag)
		{
			Enter_Combat(2, 2);
		}
		else
		{
			/* Initialize map */
			Result = Init_map();
			if (!Result)
				goto EXIT_GAME;
		}

		/* Clear flags */
		Loading_game		= FALSE;
		Quit_program_flag	= FALSE;
		Restart_game_flag	= FALSE;

		/* This is the global main loop. Each element is followed by a check
		 to see if the program should be ended. */
		while (!Quit_program_flag)
		{
			Handle_input();
			if (Quit_program_flag)
				break;

			Main_loop();
			if (Quit_program_flag)
				break;

			Switch_screens();
		}

		/* Exit data */
		if (!Enter_combat_directly_flag)
		{
 			Exit_map();
		}
		Exit_party_data();
		Exit_game_data();
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
 * NAME      : Evaluate_game_arguments
 * FUNCTION  : Evaluate game arguments.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 10:35
 * LAST      : 15.08.95 22:47
 * INPUTS    : UNSHORT argc - Number of arguments.
 *             UNCHAR **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Evaluate_game_arguments(UNSHORT argc, UNCHAR **argv)
{
	static UNCHAR *Commands[NR_GAME_COMMANDS] = {
		"map",
		"svga",
		"x",
		"y",
		"diag",
		"language",
		"noload",
		"vd",
		"combat",
		"demo",
		"cheat",
		"load",
	};

	UNSHORT Argument_index;
	UNSHORT Command_index;
	UNSHORT Value;

	/* Check all arguments */
	for (Argument_index=1;Argument_index<argc;Argument_index++)
	{
		/* Parse current argument */
		Command_index = Parse_argument(argv[Argument_index], NR_GAME_COMMANDS,
		 Commands);

		/* Act depending on command index */
		switch (Command_index)
		{
			case 0:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
				{
					PARTY_DATA.Map_nr = Value;
					PARTY_DATA.X = 10;
					PARTY_DATA.Y = 10;
				}
				break;
			}
			case 1:
			{
				Super_VGA = TRUE;
				break;
			}
			case 2:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.X = Value;
				break;
			}
			case 3:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.Y = Value;
				break;
			}
			case 4:
			{
				Diagnostic_mode = TRUE;
				break;
			}
			case 5:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);

				if (Value < LANGUAGES_MAX)
					Language = Value;

				break;
			}
			case 6:
			{
				No_load_flag = TRUE;
				break;
			}
			case 7:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				PARTY_DATA.View_direction = Value;
				break;
			}
			case 8:
			{
				Enter_combat_directly_flag = TRUE;
				break;
			}
			case 9:
			{
				PARTY_DATA.Map_nr = 199;
				PARTY_DATA.X = 10;
				PARTY_DATA.Y = 10;
				PARTY_DATA.View_direction = 1;

				PARTY_DATA.Member_nrs[1] = 2;
				PARTY_DATA.Member_nrs[2] = 5;
				PARTY_DATA.Member_nrs[3] = 4;
				PARTY_DATA.Member_nrs[4] = 6;
				PARTY_DATA.Member_nrs[5] = 9;

				PARTY_DATA.Battle_order[0] = 1;
				PARTY_DATA.Battle_order[1] = 2;
				PARTY_DATA.Battle_order[2] = 3;
				PARTY_DATA.Battle_order[3] = 4;
				PARTY_DATA.Battle_order[4] = 5;

				break;
			}
			case 10:
			{
				Cheat_mode = TRUE;
				break;
			}
			case 11:
			{
				Argument_index++;
				Load_game_nr = Parse_value(argv[Argument_index]);
				break;
			}
		}
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
 * LAST      : 19.08.95 16:22
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

	/* Allocate memory for main OPM */
	Main_OPM_handle = MEM_Do_allocate(Screen_width * Screen_height,
	 (UNLONG) &Main_OPM, &OPM_ftype);
	if (!Main_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return FALSE;
	}

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Main_OPM_handle);
	Result = OPM_New(Screen_width, Screen_height, 1, &Main_OPM, Ptr);
	MEM_Free_pointer(Main_OPM_handle);

	if (!Result)
	{
		Error(ERROR_NO_OPM);
		return FALSE;
	}

	/* Initialize palette structure */
	Palette.entries = 256;
	Palette.version = 0;

	/* Open screen */
	if (Super_VGA)
	{
		Result = DSA_OpenScreen(&Screen, &Main_OPM, &Palette,
		 NULL, 0, 0, SCREENTYPE_DOUBLEBUFFER);

		if (!Result)
		{
			Error(ERROR_NO_SCREEN);
			return FALSE;
		}
	}
	else
	{
		Result = DSA_OpenScreen(&Screen, &Main_OPM, &Palette,
		 NULL, 0, 0, SCREENTYPE_FORCE_CHAIN4 | SCREENTYPE_DOUBLEBUFFER);

		if (!Result)
		{
			Error(ERROR_NO_SCREEN);
			return FALSE;
		}
	}

	/* Correction for "improvement" in PCLIB32 */
	global_screenport.screenpalptr = &Palette;

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
	Slab_OPM_handle = MEM_Do_allocate(360 * 240, (UNLONG) &Slab_OPM,
	 &OPM_ftype);
	if (!Slab_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return FALSE;
	}

	/* Copy slab graphics to OPM */
	Ptr = MEM_Claim_pointer(Handle);
	Ptr2 = MEM_Claim_pointer(Slab_OPM_handle);

	OPM_New(360, 240, 1, &Slab_OPM, Ptr2);
	memcpy(Ptr2, Ptr, 360 * 240);

	MEM_Free_pointer(Slab_OPM_handle);

	/* Destroy graphics */
	MEM_Free_pointer(Handle);
	MEM_Kill_memory(Handle);

	/* Print Albion version */
/*	Get_version_string(Version_string);
	Set_ink(WHITE_TEXT);
	xprintf(&Slab_OPM, 1,  1, "Albion %s", Version_string);
	xprintf(&Slab_OPM, 1, 11, "Copyright 1995 Blue Byte Software GmbH"); */

	/* Generate slab backup OPM */
	Slab_backup_OPM_handle = MEM_Do_allocate(T1_WIDTH * T1_HEIGHT,
	 (UNLONG) &Slab_backup_OPM, &OPM_ftype);
	if (!Slab_backup_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return FALSE;
	}

	Ptr = MEM_Claim_pointer(Slab_backup_OPM_handle);
	OPM_New(T1_WIDTH, T1_HEIGHT, 1, &Slab_backup_OPM, Ptr);
	MEM_Free_pointer(Slab_backup_OPM_handle);

	/* Make a backup of part of the slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Slab_backup_OPM, 360 - T1_WIDTH,
	 240 - T1_HEIGHT, T1_WIDTH, T1_HEIGHT, 0, 0);

	/* Protect colours */
	Clear_protected_colours();
	Add_protected_colours(BLINK, 1);
/*	Add_protected_colours(0, 1);
	Add_protected_colours(255, 1); */

	/* Initialize palette */
	Init_palette();

	/* Clear screen */
	OPM_FillBox(&Main_OPM, 0, 0, Screen_width, Screen_height, 0);

	/* Display slab */
	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, 240, 0, 0);
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
 * LAST      : 14.11.94 10:07
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
	DSA_CloseScreen(&Screen);

	/* Delete slab OPM and free memory */
	OPM_Del(&Slab_OPM);
	MEM_Free_memory(Slab_OPM_handle);

	/* Delete screen OPM and free memory */
	OPM_Del(&Main_OPM);
	MEM_Free_memory(Main_OPM_handle);
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

