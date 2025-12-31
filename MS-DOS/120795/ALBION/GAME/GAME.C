/************
 * NAME     : GAME.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26.07.94 16:46
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

/* global variables */

struct OPM Main_OPM, Slab_OPM;
struct SCREENPORT Screen;

struct BBPALETTE Palette;
static struct BBPALETTE Backup_palette;

static MEM_HANDLE Main_OPM_handle;
static MEM_HANDLE Slab_OPM_handle;

static UNSHORT Load_game_nr = 0;
static UNSHORT Load_song_nr = 0;
static UNSHORT Load_ambient_nr = 0;

MEM_HANDLE Party_char_handles[6];
MEM_HANDLE Small_portrait_handles[6];

MEM_HANDLE Active_char_handle;

MEM_HANDLE Party_event_set_handles[6][2];
MEM_HANDLE Party_event_text_handles[6][2];

MEM_HANDLE Default_event_set_handle;
MEM_HANDLE Default_event_text_handle;

BOOLEAN Quit_program = FALSE;
BOOLEAN Terminal_condition = FALSE;

BOOLEAN Cheat_mode = FALSE;
BOOLEAN Diagnostic_mode = FALSE;
BOOLEAN Super_VGA = FALSE;

static BOOLEAN Enter_combat_directly_flag = FALSE;

BOOLEAN Bumped, Moved;

UNLONG Music_flags;

UNSHORT Nr_auto_notes;

UNSHORT Language = GERMAN;

struct Party_data PARTY_DATA;

UNSHORT Screen_width, Screen_height, Panel_Y;
UNSHORT Forbidden_X, Forbidden_Y;

UNBYTE Recolour_tables[10][256];
UNBYTE Red_table[256];

UNSHORT Earth_object;

UNLONG Body_items_offset;
UNLONG Backpack_items_offset;

static UNSHORT Level_factors[LEVEL_MAX];
static UNLONG Level_start_EP[CLASSES_MAX] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

const double PI = 3.1415927;

UNSHORT Compass_bit_coordinates[60][2];

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
 * LAST      : 26.07.94 16:48
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

	PARTY_DATA.Map_nr = 199;
	PARTY_DATA.X = 10;
	PARTY_DATA.Y = 10;
	PARTY_DATA.View_direction = 1;

	PARTY_DATA.Travel_mode = 0;

	PARTY_DATA.Member_nrs[0] = 1;
	PARTY_DATA.Member_nrs[1] = 2;
	PARTY_DATA.Member_nrs[2] = 3;
	PARTY_DATA.Member_nrs[3] = 4;
	PARTY_DATA.Member_nrs[4] = 9;

	PARTY_DATA.Active_member = 1;

	PARTY_DATA.Battle_order[0] = 1;
	PARTY_DATA.Battle_order[1] = 2;
	PARTY_DATA.Battle_order[2] = 3;
	PARTY_DATA.Battle_order[3] = 4;
	PARTY_DATA.Battle_order[4] = 8;

	PARTY_DATA.Hour = 11;

	/* Evaluate arguments */
	Evaluate_game_arguments(argc, (UNCHAR **) argv);

	/* Calculate offsets */
	{
		struct Character_data *Ptr;

		Body_items_offset = (UNLONG) ((UNBYTE *) &(Ptr->Body_items[0]) -
		 (UNBYTE *) Ptr);
		Backpack_items_offset = (UNLONG) ((UNBYTE *) &(Ptr->Backpack_items[0]) -
		 (UNBYTE *) Ptr);
	}

	/* Reset system */
	Result = Init_screen();
	if (!Result)
		return;

	Reset_root_stack();
	Reset_module_stack();
	Reset_MA_stack();

	Init_font();
	Init_music();

	Earth_object = Add_object(0, &Earth_Class, NULL, 0, 0, Screen_width,
	 Screen_height);

	if (Load_game_nr)
	{
		Load_game_state(Load_game_nr);
	}
	else
	{
		Load_initial_game_state();
	}

	Result = Init_game_data();
	if (!Result)
		return;

	Result = Init_party_data();
	if (!Result)
		return;

	Result = Calculate_game_tables();
	if (!Result)
		return;

	Active_char_handle = Party_char_handles[PARTY_DATA.Active_member-1];

	Nr_auto_notes = 0;

	Init_status_area();
	Init_animation();
	Init_time();
	Calculate_level_factors();

	/* Game begins event logic */
	Perform_action(&Game_begins_action);

	if (Enter_combat_directly_flag)
	{
		Enter_Combat(1, 2);
	}
	else
	{
		/* Initialize map */
		Result = Init_map();
		if (!Result)
			return;

		if (Load_song_nr)
			Play_song(Load_song_nr);

		if (Load_ambient_nr)
			Play_ambient_song(Load_ambient_nr);
	}

	/* This is the global main loop. Each element is followed by a check
	 to see if the program should be ended. */
	while (!Quit_program)
	{
		Handle_input();
		if (Quit_program)
			break;

		Main_loop();
		if (Quit_program)
			break;

		Switch_screens();
	}

	Exit_party_data();
	Exit_game_data();
	Exit_music();
	Exit_screen();
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
	Quit_program = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Evaluate_game_arguments
 * FUNCTION  : Evaluate game arguments.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 10:35
 * LAST      : 11.05.95 10:35
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
	static UNCHAR *Commands[13] = {
		"map",
		"svga",
		"cheat",
		"x",
		"y",
		"diag",
		"language",
		"music",
		"saved",
		"song",
		"ambient",
		"vd",
		"combat"
	};

	UNSHORT Argument_index;
	UNSHORT Command_index;
	UNSHORT Value;

	for (Argument_index=1;Argument_index<argc;Argument_index++)
	{
		Command_index = Parse_argument(argv[Argument_index], 13,
		 Commands);

		switch (Command_index)
		{
			case 0:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.Map_nr = Value;
				break;
			}
			case 1:
			{
				Super_VGA = TRUE;
				break;
			}
			case 2:
			{
				Cheat_mode = TRUE;
				break;
			}
			case 3:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.X = Value;
				break;
			}
			case 4:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value)
					PARTY_DATA.Y = Value;
				break;
			}
			case 5:
			{
				Diagnostic_mode = TRUE;
				break;
			}
			case 6:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				if (Value < LANGUAGES_MAX)
					Language = Value;
				break;
			}
			case 7:
			{
				Music_flags |= SOUNDSYSTEM_ON;
				Music_flags |= MUSIC_ON;
				Music_flags |= AMBIENT_ON;
				Music_flags |= SOUND_FX_ON;
				break;
			}
			case 8:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);

				Load_game_nr = Value;
				break;
			}
			case 9:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);

				Load_song_nr = Value;
				break;
			}
			case 10:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);

				Load_ambient_nr = Value;
				break;
			}
			case 11:
			{
				Argument_index++;
				Value = Parse_value(argv[Argument_index]);
				PARTY_DATA.View_direction = Value;
				break;
			}
			case 12:
			{
				Enter_combat_directly_flag = TRUE;
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
 * LAST      : 14.11.94 10:05
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
	UNBYTE *Ptr, *Ptr2;

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
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_game_data
 * FUNCTION  : Initialize game data.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:42
 * LAST      : 29.06.95 18:01
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_game_data(void)
{
	/* Initialize game texts */
	Init_game_texts();

	/* Load default event-set and -texts */
	Default_event_set_handle = Load_subfile(EVENT_SET, DEFAULT_EVENT_SET);
	if (!Default_event_set_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	Default_event_text_handle = Load_subfile(EVENT_TEXT, DEFAULT_EVENT_SET);
	if (!Default_event_text_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load all item data */
	if (!Init_item_data())
		return FALSE;

	/* Load all magic data */
	if (!Init_magic_data())
		return FALSE;

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_game_data
 * FUNCTION  : Delete game data.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:42
 * LAST      : 29.06.95 18:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_game_data(void)
{
	/* Exit game texts */
	Exit_game_texts();

	/* Destroy default event-set and -texts */
	MEM_Free_memory(Default_event_set_handle);
	MEM_Free_memory(Default_event_text_handle);

	/* Destroy item data */
	Exit_item_data();

	/* Destroy magic data */
	Exit_magic_data();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_game_tables
 * FUNCTION  : Calculate tables needed by the game.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.05.95 12:20
 * LAST      : 04.05.95 12:20
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Calculate_game_tables(void)
{
	UNSHORT Index;
	UNSHORT i;

	/* Calculate positions of compass bit */
	Index = 0;
	for (i=0;i<360;i+=6)
	{
		Compass_bit_coordinates[Index][0] = 12 + 11 *
		 sin(((double) i * 2 * PI) / 360);
		Compass_bit_coordinates[Index][1] = 12 - 11 *
		 cos(((double) i * 2 * PI) / 360);
		Index++;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_party_data
 * FUNCTION  : Initialize the party members' data.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:22
 * LAST      : 14.11.94 10:22
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_party_data(void)
{
	struct Character_data *Char;
	BOOLEAN Result;
	UNSHORT Batch[12];
	UNSHORT i;

	/* Select and load saved game data */


	/* Build party character data batch */
	for (i=0;i<6;i++)
	{
		Batch[i] = PARTY_DATA.Member_nrs[i];
	}

	/* Load party character data */
	Result = Load_partial_batch(PARTY_CHAR, 6, &Batch[0],
	 &Party_char_handles[0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Build small portraits batch */
	for (i=0;i<6;i++)
	{
		/* Is there a party member here ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Put portrait number into batch */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Party_char_handles[i]);

			Batch[i] = (UNSHORT) Char->Portrait_nr;

			MEM_Free_pointer(Party_char_handles[i]);
		}
		else
			/* No */
			Batch[i] = 0;
	}

	/* Load small portraits */
	Result = Load_partial_batch(SMALL_PORTRAIT, 6, &Batch[0],
	 &Small_portrait_handles[0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Build party event-set and -text batch */
	for (i=0;i<6;i++)
	{
		/* Is there a party member here ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Put event set numbers into batch */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Party_char_handles[i]);

			Batch[2 * i] = (UNSHORT) Char->Event_set_1_nr;
			Batch[2 * i + 1] = (UNSHORT) Char->Event_set_2_nr;

			MEM_Free_pointer(Party_char_handles[i]);
		}
		else
		{
			/* No */
			Batch[2 * i] = 0;
			Batch[2 * i + 1] = 0;
		}
	}

	/* Load event sets */
	Result = Load_partial_batch(EVENT_SET, 12, &Batch[0],
	 &Party_event_set_handles[0][0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load event texts */
	Result = Load_partial_batch(EVENT_TEXT, 12, &Batch[0],
	 &Party_event_text_handles[0][0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_party_data
 * FUNCTION  : Delete all party members' data.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 10:22
 * LAST      : 14.11.94 10:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_party_data(void)
{
	UNSHORT i;

	/* Destroy party members' data */
	for (i=0;i<6;i++)
	{
		MEM_Free_memory(Party_char_handles[i]);
		MEM_Free_memory(Small_portrait_handles[i]);
		MEM_Free_memory(Party_event_set_handles[i][0]);
		MEM_Free_memory(Party_event_set_handles[i][1]);
		MEM_Free_memory(Party_event_text_handles[i][0]);
		MEM_Free_memory(Party_event_text_handles[i][1]);
	}
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
	UNCHAR String2[200];

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

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Initialize_palette
 * FUNCTION  : Initialize the palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 11:09
 * LAST      : 19.10.94 11:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will load and initialize the top 64 colours.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_palette(void)
{
	MEM_HANDLE Handle;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear bottom part of palette */
	for (i=0;i<192;i++)
	{
		Palette.color[i].red = 0;
		Palette.color[i].green = 0;
 		Palette.color[i].blue = 0;
	}

	/* Load top part of palette */
	Handle = Load_file(BASEPAL);

	/* Copy colours to palette */
	Ptr = MEM_Claim_pointer(Handle);
	for (i=192;i<256;i++)
	{
		Palette.color[i].red = *Ptr++;
		Palette.color[i].green = *Ptr++;
 		Palette.color[i].blue = *Ptr++;
	}

	/* Destroy palette file */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Initialize backup palette structure */
	Backup_palette.entries = 256;
	Backup_palette.version = 0;

	/* Save palette */
	Save_palette();

	/* Update palette */
	Update_palette();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_palette
 * FUNCTION  : Load a palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 16:31
 * LAST      : 20.10.94 16:31
 * INPUTS    : UNSHORT Palette_nr - Palette subfile number.
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * NOTES     : - This function will load and initialize the bottom 192 colours.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_palette(UNSHORT Palette_nr)
{
	MEM_HANDLE Handle;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Load palette file */
	Handle = Load_subfile(PALETTE, Palette_nr);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Copy colours to palette */
	Ptr = MEM_Claim_pointer(Handle);
	for (i=0;i<192;i++)
	{
		Palette.color[i].red = *Ptr++;
		Palette.color[i].green = *Ptr++;
 		Palette.color[i].blue = *Ptr++;
	}

	/* Destroy palette file */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);

	/* Save palette */
	Save_palette();

	/* Update palette */
	Update_palette();

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_palette
 * FUNCTION  : Save the palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 14:07
 * LAST      : 17.06.95 14:07
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_palette(void)
{
	UNSHORT i;

	/* Save the palette */
	for (i=0;i<256;i++)
	{
		Backup_palette.color[i].red = Palette.color[i].red;
		Backup_palette.color[i].green = Palette.color[i].green;
		Backup_palette.color[i].blue = Palette.color[i].blue;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_palette
 * FUNCTION  : Restore the palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 14:08
 * LAST      : 17.06.95 14:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will call Restore_palette. Thus the restored
 *              palette will be activated and all recolouring tables are
 *              updated.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Restore_palette(void)
{
	UNSHORT i;

	/* Restore the palette */
	for (i=0;i<256;i++)
	{
		Palette.color[i].red = Backup_palette.color[i].red;
		Palette.color[i].green = Backup_palette.color[i].green;
		Palette.color[i].blue = Backup_palette.color[i].blue;
	}

	/* Update palette */
	Update_palette();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_palette
 * FUNCTION  : Update the palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.10.94 16:34
 * LAST      : 24.06.95 18:04
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will make sure the palette is activated and
 *              that all recolouring tables are updated.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_palette(void)
{
	/* Prepare colour finding */
	Prepare_colour_find(&Palette, 0, 256);

	/* Calculate recolouring tables */
	Calculate_recolouring_table(&(Recolour_tables[0][0]), 0, 256,
	 0, 0, 0, 50);
	Calculate_recolouring_table(&(Recolour_tables[1][0]), 0, 256,
	 0, 0, 0, 40);
	Calculate_recolouring_table(&(Recolour_tables[2][0]), 0, 256,
	 0, 0, 0, 30);
	Calculate_recolouring_table(&(Recolour_tables[3][0]), 0, 256,
	 0, 0, 0, 20);
	Calculate_recolouring_table(&(Recolour_tables[4][0]), 0, 256,
	 0, 0, 0, 10);

	Calculate_recolouring_table(&(Recolour_tables[5][0]), 0, 256,
	 0xff, 0xff, 0xff, 10);
	Calculate_recolouring_table(&(Recolour_tables[6][0]), 0, 256,
	 0xff, 0xff, 0xff, 20);
	Calculate_recolouring_table(&(Recolour_tables[7][0]), 0, 256,
	 0xff, 0xff, 0xff, 30);
	Calculate_recolouring_table(&(Recolour_tables[8][0]), 0, 256,
	 0xff, 0xff, 0xff, 40);
	Calculate_recolouring_table(&(Recolour_tables[9][0]), 0, 256,
	 0xff, 0xff, 0xff, 50);

	Calculate_recolouring_table(&(Red_table[0]), 0, 256,
	 0xff, 0, 0, 50);

	/* Tell CONTROL to activate palette */
	Palette_has_changed = TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_recolouring_table
 * FUNCTION  : Calculate a recolouring table for the current palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.10.94 11:18
 * LAST      : 19.10.94 11:18
 * INPUTS    : UNBYTE *Table - Pointer to start (!) of recolouring table.
 *             UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 *             UNSHORT Target_R - Target red value.
 *             UNSHORT Target_G - Target green value.
 *             UNSHORT Target_B - Target blue value.
 *             UNSHORT Percentage - 0% : original colour,
 *              100% : target colour.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_recolouring_table(UNBYTE *Table, UNSHORT Start, UNSHORT Size,
 UNSHORT Target_R, UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage)
{
	UNSHORT R, G, B;
	UNSHORT i;

	for (i=Start;i<Start + Size;i++)
	{
		/* Get the current colour */
		R = Palette.color[i].red;
		G = Palette.color[i].green;
		B = Palette.color[i].blue;

		/* Interpolate towards the target colour */
		R = ((R - Target_R) * (100 - Percentage)) / 100 + Target_R;
		G = ((G - Target_G) * (100 - Percentage)) / 100 + Target_G;
		B = ((B - Target_B) * (100 - Percentage)) / 100 + Target_B;

		/* Find the closest matching colour in the palette */
		Table[i] = Find_closest_colour(R, G, B);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Recolour_palette
 * FUNCTION  : Recolour the current palette.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 14:14
 * LAST      : 17.06.95 14:14
 * INPUTS    : UNSHORT Start - Number of first colour.
 *             UNSHORT Size - Number of colours to recolour.
 *             UNSHORT Target_R - Target red value.
 *             UNSHORT Target_G - Target green value.
 *             UNSHORT Target_B - Target blue value.
 *             UNSHORT Percentage - 0% : original colour,
 *              100% : target colour.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function takes colours from the backup palette,
 *              changes them, and puts them in the real palette. It does
 *              not call Update_palette, but activates the new palette
 *              directly. The recolouring tables aren't adapted.
 *              This function should only be used for effects, and afterwards
 *              Restore_palette should be called.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Recolour_palette(UNSHORT Start, UNSHORT Size, UNSHORT Target_R,
 UNSHORT Target_G, UNSHORT Target_B, UNSHORT Percentage)
{
	UNSHORT R, G, B;
	UNSHORT i;

	for (i=Start;i<Start + Size;i++)
	{
		/* Get the current colour from the backup palette */
		R = Backup_palette.color[i].red;
		G = Backup_palette.color[i].green;
		B = Backup_palette.color[i].blue;

		/* Interpolate towards the target colour */
		R = ((R - Target_R) * (100 - Percentage)) / 100 + Target_R;
		G = ((G - Target_G) * (100 - Percentage)) / 100 + Target_G;
		B = ((B - Target_B) * (100 - Percentage)) / 100 + Target_B;

		/* Store the new colour in the current palette */
		Palette.color[i].red = R;
		Palette.color[i].green = G;
		Palette.color[i].blue = B;
	}

	/* Tell CONTROL to activate palette */
	Palette_has_changed = TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_bit_array
 * FUNCTION  : Clear a bit array.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 16:25
 * LAST      : 05.07.95 10:19
 * INPUTS    : UNSHORT Array_type - Array type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_bit_array(UNSHORT Array_type)
{
	UNLONG Max = 0;
	UNSHORT i;
	UNBYTE *Array = NULL;

	/* Which bit array ? */
	switch (Array_type)
	{
		/* Quest bits */
		case QUEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Quest;
			Max = QUEST_MAX;
			break;
		}
		/* Chest bits */
		case CHEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Chest_open;
			Max = CHESTS_MAX;
			break;
		}
		/* Door bits */
		case DOOR_BIT_ARRAY:
		{
			Array = PARTY_DATA.Door_open;
			Max = DOORS_MAX;
			break;
		}
		/* Character Deleted bits */
		case CD_BIT_ARRAY:
		{
			Array = PARTY_DATA.CD;
			Max = CD_MAX;
			break;
		}
		/* Event save bits */
		case EVENT_SAVE_BIT_ARRAY:
		{
			Array = PARTY_DATA.Event;
			Max = EVENT_MAX;
			break;
		}
		/* Known word bits */
		case KNOWN_WORDS_BIT_ARRAY:
		{
			Array = Known_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* New word bits */
		case NEW_WORDS_BIT_ARRAY:
		{
			Array = New_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* Goto-point bits */
		case GOTO_POINTS_BIT_ARRAY:
		{
			Array = PARTY_DATA.Goto_points;
			Max = GOTO_POINTS_MAX;
			break;
		}
		/* Error */
		default:
		{
			/* Report error */
			Error(ERROR_BIT_ARRAY_TYPE);
			return;
			break;
		}
	}

	/* Legal array ? */
	if (Array)
	{
		/* Yes -> Clear bit array */
		for (i=0;i<((Max + 7) / 8);i++)
			Array[i] = 0;
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_BIT_ARRAY_ILLEGAL_ARRAY);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Read_bit_array
 * FUNCTION  : Read a bit from a bit array.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:53
 * LAST      : 05.07.95 10:19
 * INPUTS    : UNSHORT Array_type - Array type.
 *             UNLONG Bit_number - Bit number.
 * RESULT    : BOOLEAN : TRUE (bit was set) or FALSE (bit was cleared).
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Read_bit_array(UNSHORT Array_type, UNLONG Bit_number)
{
	UNLONG Max = 0;
	UNBYTE *Array = NULL;

	/* Which bit array ? */
	switch (Array_type)
	{
		/* Quest bits */
		case QUEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Quest;
			Max = QUEST_MAX;
			break;
		}
		/* Chest bits */
		case CHEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Chest_open;
			Max = CHESTS_MAX;
			break;
		}
		/* Door bits */
		case DOOR_BIT_ARRAY:
		{
			Array = PARTY_DATA.Door_open;
			Max = DOORS_MAX;
			break;
		}
		/* Character Deleted bits */
		case CD_BIT_ARRAY:
		{
			Array = PARTY_DATA.CD;
			Max = CD_MAX;
			break;
		}
		/* Event save bits */
		case EVENT_SAVE_BIT_ARRAY:
		{
			Array = PARTY_DATA.Event;
			Max = EVENT_MAX;
			break;
		}
		/* Known word bits */
		case KNOWN_WORDS_BIT_ARRAY:
		{
			Array = Known_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* New word bits */
		case NEW_WORDS_BIT_ARRAY:
		{
			Array = New_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* Goto-point bits */
		case GOTO_POINTS_BIT_ARRAY:
		{
			Array = PARTY_DATA.Goto_points;
			Max = GOTO_POINTS_MAX;
			break;
		}
		/* Error */
		default:
		{
			/* Report error */
			Error(ERROR_BIT_ARRAY_TYPE);
			return FALSE;
			break;
		}
	}

	/* Legal array ? */
	if (Array)
	{
		/* Yes -> Bit number in range ? */
		if (Bit_number < Max)
		{
			/* Yes -> Read bit */
			return((BOOLEAN) ((Array[Bit_number >> 3]) &
			 (1 << (Bit_number & 0x0007))));
		}
		else
		{
			/* No -> Report error */
			Error(ERROR_BIT_ARRAY_OUT_OF_RANGE);

			/* False */
			return FALSE;
		}
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_BIT_ARRAY_ILLEGAL_ARRAY);

		/* False */
		return FALSE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_bit_array
 * FUNCTION  : Write a bit from a bit array.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 17:53
 * LAST      : 05.07.95 10:19
 * INPUTS    : UNSHORT Array_type - Array type.
 *             UNLONG Bit_number - Bit number.
 *             UNSHORT Write_mode - Write mode.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMEVAR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Write_bit_array(UNSHORT Array_type, UNLONG Bit_number, UNSHORT Write_mode)
{
	UNLONG Max = 0;
	UNBYTE *Array = NULL;

	/* Which bit array ? */
	switch (Array_type)
	{
		/* Quest bits */
		case QUEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Quest;
			Max = QUEST_MAX;
			break;
		}
		/* Chest bits */
		case CHEST_BIT_ARRAY:
		{
			Array = PARTY_DATA.Chest_open;
			Max = CHESTS_MAX;
			break;
		}
		/* Door bits */
		case DOOR_BIT_ARRAY:
		{
			Array = PARTY_DATA.Door_open;
			Max = DOORS_MAX;
			break;
		}
		/* Character Deleted bits */
		case CD_BIT_ARRAY:
		{
			Array = PARTY_DATA.CD;
			Max = CD_MAX;
			break;
		}
		/* Event save bits */
		case EVENT_SAVE_BIT_ARRAY:
		{
			Array = PARTY_DATA.Event;
			Max = EVENT_MAX;
			break;
		}
		/* Known word bits */
		case KNOWN_WORDS_BIT_ARRAY:
		{
			Array = Known_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* New word bits */
		case NEW_WORDS_BIT_ARRAY:
		{
			Array = New_words_bit_array_ptr;
			Max = WORDS_MAX;
			break;
		}
		/* Goto-point bits */
		case GOTO_POINTS_BIT_ARRAY:
		{
			Array = PARTY_DATA.Goto_points;
			Max = GOTO_POINTS_MAX;
			break;
		}
		/* Error */
		default:
		{
			/* Report error */
			Error(ERROR_BIT_ARRAY_TYPE);
			return;
			break;
		}
	}

	/* Legal array ? */
	if (Array)
	{
		/* Yes -> Bit number in range ? */
		if (Bit_number < Max)
		{
			/* Yes -> Which write mode ? */
			switch (Write_mode)
			{
				/* Set bit */
				case SET_BIT_ARRAY:
				{
					Array[Bit_number >> 3] |= (1 << (Bit_number & 0x0007));
					break;
				}
				/* Clear bit */
				case CLEAR_BIT_ARRAY:
				{
					Array[Bit_number >> 3] &= ~(1 << (Bit_number & 0x0007));
					break;
				}
				/* Toggle bit */
				case TOGGLE_BIT_ARRAY:
				{
					Array[Bit_number >> 3] ^= (1 << (Bit_number & 0x0007));
					break;
				}
				/* Error */
				default:
				{
					/* Report error and exit */
					Error(ERROR_BIT_ARRAY_ACCESS_TYPE);
					return;
					break;
				}
			}

			/* Demon logic */
			/* Which bit array ? */
			switch (Array_type)
			{
				/* Character Deleted bits */
				case CD_BIT_ARRAY:
				{
					/* Update present status of all NPCs */
					Update_NPC_present_status();
					break;
				}
			}
		}
		else
		{
			/* No -> Report error */
			Error(ERROR_BIT_ARRAY_OUT_OF_RANGE);
		}
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_BIT_ARRAY_ILLEGAL_ARRAY);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Activate_party_member
 * FUNCTION  : Activate a party member.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.12.94 10:52
 * LAST      : 30.12.94 10:52
 * INPUTS    : UNSHORT Member_nr - Member index (1...6).
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Activate_party_member(UNSHORT Member_nr)
{
	static struct Event_action Activate_party_member_action =
	{
		0,	PARTY_ACTOR_TYPE, 0,
		MAKE_ACTIVE_ACTION, 0, 0,
		Do_activate_party_member, NULL, NULL
	};
	BOOLEAN Result;

	/* Anyone there ? */
	if (!PARTY_DATA.Member_nrs[Member_nr-1])
		return FALSE;

	/* Is this member already active ? */
	if (PARTY_DATA.Active_member == Member_nr)
		return TRUE;

	/* Complex dialogue check */

	/* Check body conditions */

	Activate_party_member_action.Actor_index = Member_nr;
	Result = Perform_action(&Activate_party_member_action);

	return(Result);
}

BOOLEAN
Do_activate_party_member(struct Event_action *Action)
{
	UNSHORT Member_nr;

	Member_nr = Action->Actor_index;

	/* Make this member active */
	Active_char_handle = Party_char_handles[Member_nr-1];
	PARTY_DATA.Active_member = Member_nr;

	/* Do stuff when in combat */

	return TRUE;
}

/*
	moveq.l	#Dialogue_ID,d0		; Yes -> In dialogue ?
	jsr	Find_module
	beq.s	.No1
	tst.b	NPC_or_member		; With party member ?
	bne.s	.Yes
	tst.b	Joined			; Joined ?
	beq.s	.Skip
.Yes:	cmp.w	Dialogue_NPC_index,d7	; With this member ?
	beq.s	.Exit
	bra.s	.No1
.Skip:	Get	Dialogue_handle,a0		; Get NPC's languages
	move.b	Learned_languages(a0),d0
	Free	Dialogue_handle
	and.b	Learned_languages(a1),d0	; Compare with new active's
	tst.b	d0			; Que ?
	bne.s	.No1
	move.w	#106,d0			; No comprendo, signor !
	jsr	Do_prompt_window
	bra	.Exit
.No1:

	move.w	Body_conditions(a1),d0	; Get conditions
	move.w	d0,d1
	and.w	#Active_mask,d1		; Possible active ?
	bne	.Exit

	move.b	d5,Active_handle		; Store
	move.w	d7,Active_member
	jsr	Check_active_member		; Update status
	tst.b	Battling			; In combat ?
	beq.s	.No2
	lea.l	Combat_party,a0		; Set active participant
	subq.w	#1,d7
	mulu.w	#Participant_data_size,d7
	add.l	d7,a0
	move.l	a0,Active_participant
.No2:	moveq.l	#0,d6			; Success !
.Exit:	Free	d5
.Exit2:	tst.w	d6			; Any luck ?
	movem.l	(sp)+,d0/d1/d5-d7/a0/a1
	rts
*/


/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_in_party
 * FUNCTION  : Check if a character is in the party.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.06.95 17:12
 * LAST      : 22.06.95 17:12
 * INPUTS    : UNSHORT Char_index - Character index.
 * RESULT    : BOOLEAN : TRUE (in party) or FALSE (not in party).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_in_party(UNSHORT Char_index)
{
	BOOLEAN Result = FALSE;
	UNSHORT i;

	/* Search party */
	for (i=0;i<6;i++)
	{
		/* Is this the right character ? */
		if (PARTY_DATA.Member_nrs[i] == Char_index)
		{
			/* Yes */
			Result = TRUE;
			break;
		}
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_capable
 * FUNCTION  : Check if a character is capable.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.01.95 10:23
 * LAST      : 26.01.95 10:23
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE (capable) or FALSE (incapable).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_capable(MEM_HANDLE Char_handle)
{
	if (Get_conditions(Char_handle) & INCAPABLE_MASK)
		return FALSE;
	else
		return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_can_see
 * FUNCTION  : Check if the party can see.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 16:39
 * LAST      : 07.02.95 16:39
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (can see) or FALSE (cannot see).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_can_see(void)
{
	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_is_full
 * FUNCTION  : Check if the party is full.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.07.95 17:35
 * LAST      : 03.07.95 17:35
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (is full) or FALSE (is not full).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_is_full(void)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Check party */
	for (i=0;i<6;i++)
	{
		/* Free slot ? */
		if (!PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Party is not full */
			Result = FALSE;

			/* Exit */
			break;
		}
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_has_magical_abilities
 * FUNCTION  : Check if a character has magical abilities.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 14:45
 * LAST      : 04.04.95 14:45
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_has_magical_abilities(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes */
		Result = TRUE;
	}
	else
	{
		/* No -> Check */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Any spell classes known ? */
		Result = (BOOLEAN)(Char->xKnown_spell_classes);

		MEM_Free_pointer(Char_handle);
	}

	return(Result);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_knows_spells
 * FUNCTION  : Check if a character knows any spells.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.04.95 14:12
 * LAST      : 05.04.95 14:12
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_knows_spells(MEM_HANDLE Char_handle)
{
	BOOLEAN Result = FALSE;
	UNSHORT i, j;

	/* Does the character have any magical abilities ? */
	if (Character_has_magical_abilities(Char_handle))
	{
		/* Yes -> Check all spells */
		for (i=0;i<SPELL_CLASSES_MAX;i++)
		{
			/* Check all spells in this class */
			for (j=0;j<SPELLS_PER_CLASS;j++)
			{
				/* Is this spell known ? */
				if (Spell_known(Char_handle, i, j))
				{
					/* Yes! */
					Result = TRUE;
					break;
				}
			}

			/* Know any spells yet ? */
			if (Result)
				break;
		}
	}
	return(Result);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Character_is_overweight
 * FUNCTION  : Check if a character is overweight.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 11:07
 * LAST      : 20.06.95 11:07
 * INPUTS    : MEM_HANDLE Char_handle - Memory handle of character data.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Character_is_overweight(MEM_HANDLE Char_handle)
{
	/* Carrying too much ? */
	if (Get_carried_weight(Char_handle) >= ((UNLONG) Get_attribute(Char_handle,
	 STRENGTH) * 1000))
		return TRUE;
	else
		return FALSE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Probe
 * FUNCTION  : Probe a value. A random number between 0 and the probe range
 *              is determined. If this number is smaller or equal to the
 *              probe value, the probe was successful.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 12:49
 * LAST      : 13.03.95 12:49
 * INPUTS    : SISHORT Value - Probe value.
 *             UNSHORT Range - Probe range.
 * RESULT    : BOOLEAN : Probe success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Probe(SISHORT Value, UNSHORT Range)
{
	/* Zero or lower ? */
	if (Value < 0)
		return FALSE;

	/* Equal to or above probe range ? */
	if (Value >= Range)
		return TRUE;

	/* Probe */
	return((rand() % Range) <= Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Probe_attribute
 * FUNCTION  : Probe a character's attribute.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 12:52
 * LAST      : 13.03.95 12:52
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Attribute_nr - Attribute index.
 * RESULT    : BOOLEAN : Probe success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Probe_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr)
{
	return(Probe(Get_attribute(Char_handle, Attribute_nr), 100));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Probe_skill
 * FUNCTION  : Probe a character's skill.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 12:53
 * LAST      : 13.03.95 12:53
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Skill_nr - Skill index.
 * RESULT    : BOOLEAN : Probe success.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Probe_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr)
{
	return(Probe(Get_skill(Char_handle, Skill_nr), 100));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_rnd_50_100
 * FUNCTION  : Get random value between 50% of 100% of input value.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 13:52
 * LAST      : 31.03.95 13:52
 * INPUTS    : UNSHORT Value - Input value.
 * RESULT    : UNSHORT : Return value.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_rnd_50_100(UNSHORT Value)
{
	return((((rand() % 51) + 50) * Value) / 100);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_attribute
 * FUNCTION  : Get a character's attribute.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:44
 * LAST      : 07.03.95 16:44
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Attribute_nr - Attribute index.
 * RESULT    : UNSHORT : Attribute value.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr)
{
	struct Character_data *Char;
	UNSHORT Value = 0;

	/* Legal attribute number ? */
	if ((Attribute_nr > 0) && (Attribute_nr <= ATTRS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Get attribute value */
		Value = Char->Attributes[Attribute_nr-1].Normal +
		 Char->Attributes[Attribute_nr-1].Magic;

		MEM_Free_pointer(Char_handle);
	}

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_attribute
 * FUNCTION  : Set a character's attribute.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:21
 * LAST      : 31.03.95 14:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Attribute_nr - Attribute index.
 *             SISHORT Value - New value.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_attribute(MEM_HANDLE Char_handle, UNSHORT Attribute_nr, SISHORT Value)
{
	struct Character_data *Char;

	/* Legal attribute number ? */
	if ((Attribute_nr > 0) && (Attribute_nr <= ATTRS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Set attribute value */
		Char->Attributes[Attribute_nr-1].Normal = Value;

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_skill
 * FUNCTION  : Get a character's skill.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 16:45
 * LAST      : 07.03.95 16:45
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Skill_nr - Skill index.
 * RESULT    : UNSHORT : Skill value.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr)
{
	struct Character_data *Char;
	UNSHORT Value = 0;

	/* Legal skill number ? */
	if ((Skill_nr > 0) && (Skill_nr <= SKILLS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Get skill value */
		Value = Char->Skills[Skill_nr-1].Normal +
		 Char->Skills[Skill_nr-1].Magic;

		MEM_Free_pointer(Char_handle);
	}

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_skill
 * FUNCTION  : Set a character's skill.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:22
 * LAST      : 31.03.95 14:22
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Skill_nr - Skill index.
 *             SISHORT Value - New value.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_skill(MEM_HANDLE Char_handle, UNSHORT Skill_nr, SISHORT Value)
{
	struct Character_data *Char;

	/* Legal skill number ? */
	if ((Skill_nr > 0) && (Skill_nr <= SKILLS_MAX))
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Set skill value */
		Char->Skills[Skill_nr-1].Normal = Value;

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_condition
 * FUNCTION  : Set a character's body condition.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:23
 * LAST      : 08.03.95 16:23
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Condition_nr - Body condition index.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr)
{
	struct Character_data *Char;

	/* Legal condition ? */
	if (Condition_nr  < MAX_CONDITIONS)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Set condition */
		Char->xBody_conditions |= (1 << Condition_nr);

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_condition
 * FUNCTION  : Clear a character's body condition.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:24
 * LAST      : 08.03.95 16:24
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Condition_nr - Body condition index.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clear_condition(MEM_HANDLE Char_handle, UNSHORT Condition_nr)
{
	struct Character_data *Char;

	/* Legal condition ? */
	if (Condition_nr  < MAX_CONDITIONS)
	{
		/* Yes -> Get character data */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Clear condition */
		Char->xBody_conditions &= ~(1 << Condition_nr);

		MEM_Free_pointer(Char_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_conditions
 * FUNCTION  : Get a character's body conditions.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 18:25
 * LAST      : 07.03.95 18:25
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Body conditions bit-list.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_conditions(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get conditions */
	Value = Char->xBody_conditions;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_char_name
 * FUNCTION  : Get a character's name.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 16:51
 * LAST      : 13.03.95 16:51
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNCHAR *Name - Pointer to name buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The buffer should be at least CHAR_NAME_LENGTH + 1 bytes long.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_char_name(MEM_HANDLE Char_handle, UNCHAR *Name)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Copy character name */
	strcpy(Name, Char->xName[Language]);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_class
 * FUNCTION  : Get a character's class.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:09
 * LAST      : 04.04.95 13:09
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Class.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_class(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get class */
	Value = (UNSHORT) Char->xClass;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_level
 * FUNCTION  : Get a character's level.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:12
 * LAST      : 04.04.95 13:12
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Level.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_level(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get level */
	Value = (UNSHORT) Char->xLevel;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_sex
 * FUNCTION  : Get a character's sex.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:21
 * LAST      : 04.04.95 13:21
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Sex.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_sex(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get sex */
	Value = (UNSHORT) Char->xSex;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_race
 * FUNCTION  : Get a character's race.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:22
 * LAST      : 04.04.95 13:22
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Race.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_race(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get race */
	Value = (UNSHORT) Char->xRace;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_LP
 * FUNCTION  : Get a character's life points.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:27
 * LAST      : 31.03.95 14:27
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Life points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_LP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get life points */
	Value = Char->xLife_points;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_SP
 * FUNCTION  : Get a character's spell points.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:28
 * LAST      : 31.03.95 14:28
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Spell points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_SP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get spell points */
	Value = Char->xSpell_points;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_LP
 * FUNCTION  : Set a character's life points.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.95 14:04
 * LAST      : 03.06.95 14:04
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT New_LP - New amount of life points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_LP(MEM_HANDLE Char_handle, UNSHORT New_LP)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set life points */
	Char->xLife_points = min(New_LP, Char->xLife_points_maximum +
	 Char->xLife_points_magic);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_SP
 * FUNCTION  : Set a character's spell points.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.95 14:05
 * LAST      : 03.06.95 14:05
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT New_SP - New amount of spell points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_SP(MEM_HANDLE Char_handle, UNSHORT New_SP)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set spell points */
	Char->xSpell_points = min(New_SP, Char->xSpell_points_maximum +
	 Char->xSpell_points_magic);

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_EP
 * FUNCTION  : Get a character's experience points.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 15:19
 * LAST      : 31.03.95 15:19
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNLONG : Experience points.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_EP(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNLONG Value;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Get experience points */
	Value = Char->xExperience_points;

	MEM_Free_pointer(Char_handle);

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_EP
 * FUNCTION  : Set a character's experience points.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 15:20
 * LAST      : 31.03.95 15:20
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNLONG Value - New experience points.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_EP(MEM_HANDLE Char_handle, UNLONG Value)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Set experience points */
	Char->xExperience_points = Value;

	MEM_Free_pointer(Char_handle);

	/* Check member levels */
	Check_member_levels();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_carried_weight
 * FUNCTION  : Get the weight carried by a character.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 12:25
 * LAST      : 03.04.95 12:25
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNLONG : Weight carried by character (in grammes).
 * BUGS      : No known.
 * NOTES     : - Although this function should be used to access the carried
 *              weight, it will still set the Carried_weight variable, just
 *              in case.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_carried_weight(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNLONG Weight;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Calculate weight */
	Weight = 0;

	/* Add weight of body items */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything in body slot ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes -> Add weight */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			Weight += (UNLONG) Item_data->Weight;

			Free_item_data();
		}
	}

	/* Add weight of backpack items */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Anything in backpack slot ? */
		if (!Packet_empty(&(Char->Backpack_items[i])))
		{
			/* Yes -> Add weight */
			Item_data = Get_item_data(&(Char->Backpack_items[i]));

			Weight += (UNLONG) Item_data->Weight *
			 (UNSHORT) Char->Backpack_items[i].Quantity;

			Free_item_data();
		}
	}

	/* Add weight of gold */
	Weight += (UNLONG) Char->Char_gold * GOLD_WEIGHT;

	/* Add weight of food */
	Weight += (UNLONG) Char->Char_food * FOOD_WEIGHT;

	/* Write carried weight */
	Char->xCarried_weight = Weight;

	MEM_Free_pointer(Char_handle);

	return(Weight);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_nr_occupied_hands
 * FUNCTION  : Get the number of occupied hands of a character.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:48
 * LAST      : 03.04.95 14:48
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Number of occupied hands.
 * BUGS      : No known.
 * NOTES     : - Although this function should be used to access the number
 *              of occupied hands, it will still set the Hands_occupied
 *              variable, just in case.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_nr_occupied_hands(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT Nr_hands;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Clear counter */
	Nr_hands = 0;

	/* Anything in the right hand? */
	if (!Packet_empty(&(Char->Body_items[RIGHT_HAND - 1])))
	{
		/* Yes -> Add number of hands to counter */
		Item_data = Get_item_data(&(Char->Body_items[RIGHT_HAND - 1]));
		Nr_hands += Item_data->Hand_use;
		Free_item_data();
	}

	/* Anything in the right hand? */
	if (!Packet_empty(&(Char->Body_items[LEFT_HAND - 1])))
	{
		/* Yes -> Add number of hands to counter */
		Item_data = Get_item_data(&(Char->Body_items[LEFT_HAND - 1]));
		Nr_hands += Item_data->Hand_use;
		Free_item_data();
	}

	/* Write number of occupied hands */
	Char->xHands_occupied = Nr_hands;

	MEM_Free_pointer(Char_handle);

	return(Nr_hands);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Spell_class_known
 * FUNCTION  : Check if a character knows a certain spell class.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 13:05
 * LAST      : 04.04.95 13:05
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 * RESULT    : BOOLEAN : Known.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Spell_class_known(MEM_HANDLE Char_handle, UNSHORT Class_nr)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes */
		Result = TRUE;
	}
	else
	{
		/* No -> Check */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Spell class known ? */
		Result = (BOOLEAN)(Char->xKnown_spell_classes & (1 << Class_nr));

		MEM_Free_pointer(Char_handle);
	}

	return(Result);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Spell_known
 * FUNCTION  : Check if a character knows a certain spell.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 12:37
 * LAST      : 04.04.95 12:37
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : BOOLEAN : Known.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Spell_known(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes */
		Result = TRUE;
	}
	else
	{
		/* No -> Check */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Spell known ? */
		Result = (BOOLEAN)(Char->xKnown_spells[Class_nr] & (1 << Spell_nr));

		MEM_Free_pointer(Char_handle);
	}

	return(Result);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_strength
 * FUNCTION  : Get the strength of a certain spell of a character.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.04.95 12:46
 * LAST      : 04.04.95 12:46
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNLONG : Spell strength (0...10000).
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_spell_strength(MEM_HANDLE Char_handle, UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Character_data *Char;
	UNLONG Value;

	/* Cheat mode ? */
	if (Cheat_mode)
	{
		/* Yes */
		Value = 100 * 100;
	}
	else
	{
		/* No -> Check */
		Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

		/* Get spell strength */
		Value = (UNLONG) Char->xSpell_capabilities[Class_nr][Spell_nr];

		MEM_Free_pointer(Char_handle);
	}

	return(Value);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_damage
 * FUNCTION  : Get the damage a character can do.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:03
 * LAST      : 24.05.95 11:47
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Damage.
 * BUGS      : No known.
 * NOTES     : - This function should be used to access the amount of
 *              damage. The character data variable should NOT be accessed!
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_damage(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i;
	SISHORT Damage;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Calculate initial damage */
	Damage = Char->xDamage + Char->xDamage_magic;

	/* Add damage of body items */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything in body slot ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			/* Cursed ? */
			if (Char->Body_items[i].Flags & CURSED)
			{
				/* Yes -> Subtract damage value of item */
				Damage -= (UNSHORT) Item_data->Damage_pts;
			}
			else
			{
				/* No -> Add damage value of item */
				Damage += (UNSHORT) Item_data->Damage_pts;
			}

			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);

	return((UNSHORT) min(Damage, 0));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_protection
 * FUNCTION  : Get the protection a character has.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 11:06
 * LAST      : 24.05.95 11:47
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Protection.
 * BUGS      : No known.
 * NOTES     : - This function should be used to access the amount of
 *              protection. The character data variable should NOT be
 *              accessed!
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_protection(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i;
	SISHORT Protection;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Calculate initial protection */
	Protection = Char->xProtection + Char->xProtection_magic;

	/* Add protection of body items */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Anything in body slot ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* Yes */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			/* Cursed ? */
			if (Char->Body_items[i].Flags & CURSED)
			{
				/* Yes -> Subtract protection value of item */
				Protection -= (UNSHORT) Item_data->Protection_pts;
			}
			else
			{
				/* No -> Add protection value of item */
				Protection += (UNSHORT) Item_data->Protection_pts;
			}

			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);

	return((UNSHORT) min(Protection, 0));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_damage
 * FUNCTION  : Do damage to a party member.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 10:45
 * LAST      : 21.04.95 10:45
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 *             UNSHORT Damage - Amount of damage.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_damage(UNSHORT Member_nr, UNSHORT Damage)
{
	SISHORT LP;

	/* Cheat mode ? */
	if (!Cheat_mode)
	{
		/* No -> Anyone there ? */
		if (PARTY_DATA.Member_nrs[Member_nr - 1])
		{
			/* Yes -> Decrease life points */
			LP = Get_LP(Party_char_handles[Member_nr - 1]);
			LP = max(LP - Damage, 0);
			Set_LP(Party_char_handles[Member_nr - 1], LP);

			/* "Dead" ? */
			if (!LP)
			{
				/* Yes -> Kill party member */
				Kill_party_member(Member_nr);
			}
			else
			{
				/* No -> Show damage */

			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Kill_party_member
 * FUNCTION  : "Kill" a party member.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 10:53
 * LAST      : 21.04.95 10:53
 * INPUTS    : UNSHORT Member_nr - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Kill_party_member(UNSHORT Member_nr)
{
	/* Anyone there ? */
	if (PARTY_DATA.Member_nrs[Member_nr - 1])
	{
		/* Yes -> "Kill" */
		Set_condition(Party_char_handles[Member_nr - 1], UNCONSCIOUS);

		/* Clear life-points */
		Set_LP(Party_char_handles[Member_nr - 1], 0);

		/* Show "death" */

		/* Was this the active party member / not in combat ? */
		if ((Member_nr == PARTY_DATA.Active_member) && (!In_Combat))
		{
			/* Yes -> Select a new leader */
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_level_factors
 * FUNCTION  : Calculate level factors.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 13:11
 * LAST      : 31.03.95 13:11
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Calculate_level_factors(void)
{
	UNSHORT Old;
	UNSHORT New;
	UNSHORT i;

	Old = 0;
	for (i=1;i<=LEVEL_MAX;i++)
	{
		New = i + Old;
		Level_factors[i - 1] = New;
		Old = New;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_member_levels
 * FUNCTION  : Check if any party member's should advance to the next level.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 13:17
 * LAST      : 31.03.95 13:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function should be called whenever a party member's
 *              experience points are changed.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Check_member_levels(void)
{
	UNLONG Current_EP;
	UNLONG Start_EP;
	UNLONG Next_EP;
	UNSHORT Levels[6];
	UNSHORT Current_level;
	UNSHORT i, j;

	/* Check entire party */
	for (i=0;i<6;i++)
	{
		/* Clear level */
		Levels[i] = 1;

		/* Anyone there ? */
		if (PARTY_DATA.Member_nrs[i])
		{
			/* Yes -> Is this character alive ? */
			if (!(Get_conditions(Party_char_handles[i]) & DEAD_MASK))
			{
				/* Yes -> Get current experience points */
				Current_EP = Get_EP(Party_char_handles[i]);

				/* Get start EP */
				Start_EP = Level_start_EP[Get_class(Party_char_handles[i])];

				/* While the maximum level hasn't been reached yet */
				while (Levels[i] < LEVEL_MAX)
				{
					/* Calculate the EP needed for the next level */
					/* (the levels start at 1 !) */
					Next_EP = Start_EP * Level_factors[Levels[i]];

					/* Enough for the next level ? */
					if (Current_EP >= Next_EP)
					{
						/* Yes -> Increase level */
						Levels[i]++;
					}
					else
					{
						/* No -> Break off */
						break;
					}
				}

				/* Any changes ? */
				Current_level = Get_level(Party_char_handles[i]);
				if (Levels[i] != Current_level)
				{
					/* Yes -> Increase or decrease ? */
					if (Levels[i] > Current_level)
					{
						/* Increase */
						for (j=0;j<Levels[i] - Current_level;j++)
						{
							Increase_character_level(Party_char_handles[i]);
						}
					}
				}
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Increase_character_level
 * FUNCTION  : Increase the level of a character.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.03.95 14:03
 * LAST      : 31.03.95 14:03
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only increases the level and character values
 *              and does not display anything.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Increase_character_level(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	UNSHORT New_APR = 0;
	UNSHORT Extra_LP;
	UNSHORT Extra_SP;
	UNSHORT Extra_TP;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Increase character level */
	Char->xLevel += 1;

	/* Is the APR factor zero ? */
	if (Char->Special[0])
	{
		/* No -> Calculate new attacks per round */
		New_APR = (UNSHORT) Get_level(Char_handle) / Char->Special[0];
	}

	/* Is the new APR zero ? */
	if (!New_APR)
	{
		/* Yes -> Make it at least one */
		New_APR = 1;
	}

	/* Store new APR */
	Char->Attacks_per_round = New_APR;

	/* Calculate extra LP & LP max */
	Extra_LP = Get_rnd_50_100(Char->Special[1]);

	/* Add */
	Char->xLife_points_maximum += Extra_LP;
	Set_LP(Char_handle, Get_LP(Char_handle) + Extra_LP);

	/* Does this character have magical abilities ? */
	if (Character_has_magical_abilities(Char_handle))
	{
		/* Yes -> Calculate extra SP & SP max */
		Extra_SP = Get_rnd_50_100(Char->Special[2]);

		/* Add intelligence bonus */
		Extra_SP += Get_attribute(Char_handle, INTELLIGENCE) / 20;

		/* Add */
		Char->xSpell_points_maximum += Extra_SP;
		Set_SP(Char_handle, Get_SP(Char_handle) + Extra_SP);
	}

	/* Calculate extra training points */
	Extra_TP = Get_rnd_50_100(Char->Special[4]);

	/* Add */
	Char->Training_points += Extra_TP;

	MEM_Free_pointer(Char_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Char_inventory_empty
 * FUNCTION  : Check if a character's inventory is empty.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 14:35
 * LAST      : 27.04.95 14:35
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : BOOLEAN : Character's inventory is empty.
 * BUGS      : No known.
 * NOTES     : - Both body and backpack items are checked.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Char_inventory_empty(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check if character's inventory is empty */
	Result = (Inventory_empty(&(Char->Body_items[0]), ITEMS_ON_BODY)) |
	 (Inventory_empty(&(Char->Backpack_items[0]), ITEMS_PER_CHAR));

	MEM_Free_pointer(Char_handle);

	return(Result);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Char_inventory_full
 * FUNCTION  : Check if a character's inventory is full.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 14:38
 * LAST      : 27.04.95 14:38
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : BOOLEAN : Character's inventory is full.
 * BUGS      : No known.
 * NOTES     : - Only backpack items are checked.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Char_inventory_full(MEM_HANDLE Char_handle)
{
	struct Character_data *Char;
	BOOLEAN Result;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check if character's inventory is empty */
	Result = Inventory_full(&(Char->Backpack_items[0]), ITEMS_PER_CHAR);

	MEM_Free_pointer(Char_handle);

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_items_on_body
 * FUNCTION  : Count all the items of a certain type on a character's
 *              body.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 10:43
 * LAST      : 28.04.95 10:43
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Item_index - Index of item that must be counted.
 * RESULT    : UNSHORT : Counter.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_items_on_body(MEM_HANDLE Char_handle, UNSHORT Item_index)
{
 	struct Character_data *Char;
	UNSHORT Counter = 0;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check all item slots on body */
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is this the right item ? */
		if (Char->Body_items[i].Index == Item_index)
		{
			/* Yes -> Count up */
			Counter++;
		}
	}

	MEM_Free_pointer(Char_handle);

	return(Counter);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_items_in_backpack
 * FUNCTION  : Count all the items of a certain type in a character's
 *              backpack.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 23:11
 * LAST      : 27.04.95 23:11
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Item_index - Index of item that must be counted.
 * RESULT    : UNSHORT : Counter.
 * BUGS      : No known.
 * NOTES     :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_items_in_backpack(MEM_HANDLE Char_handle, UNSHORT Item_index)
{
 	struct Character_data *Char;
	UNSHORT Counter = 0;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check all item slots in backpack */
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Is this the right item ? */
		if (Char->Backpack_items[i].Index == Item_index)
		{
			/* Yes -> Count up */
			Counter += (UNSHORT) Char->Backpack_items[i].Quantity;
		}
	}

	MEM_Free_pointer(Char_handle);

	return(Counter);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Magical_item_evaluator
 * FUNCTION  : Check if item is magical (item select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.04.95 15:31
 * LAST      : 27.04.95 15:31
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Magical_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Magical item ? */
	if (Item_data->Spell_nr)
	{
		/* Yes -> Any charges left ? */
		if (Packet->Charges)
		{
			/* Yes -> Item is OK */
			Message_nr = 0;
		}
		else
		{
			/* No -> "No more charges" */
			Message_nr = 94;
		}
	}
	else
	{
		/* No -> "Item is not magical" */
		Message_nr = 93;
	}

	Free_item_data();

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Vital_item_evaluator
 * FUNCTION  : Check if item is vital for solving the game (item select
 *              window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.07.95 17:20
 * LAST      : 03.07.95 17:20
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Vital_item_evaluator(struct Character_data *Char, struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr = 0;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Is the item undroppable ? */
	if (Item_data->Flags & UNDROPPABLE)
	{
		/* Yes -> "Item is vital" */
		Message_nr = 511;
	}

	Free_item_data();

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Alive_member_evaluator
 * FUNCTION  : Check if a party member is alive (member select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.04.95 15:07
 * LAST      : 28.04.95 15:07
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 * RESULT    : UNSHORT : Message nr / 0 = member is OK
 *              / 0xFFFF = member is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Alive_member_evaluator(MEM_HANDLE Char_handle)
{
	/* Is this party member alive ? */
	if (Get_conditions(Char_handle) & DEAD_MASK)
	{
		/* No -> "Member's dead!" */
		return 127;
	}
	else
	{
		/* Yes -> Member is OK */
		return 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_party_member
 * FUNCTION  : Remove a party member from the party.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 15:25
 * LAST      : 30.06.95 15:25
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_party_member(UNSHORT Member_index)
{
	struct Character_data *Char;

	/* Get character data */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Party_char_handles[Member_index - 1]);

	/* Does this person have a CD bit number ? */
	if (Char->Destination_CD_bit != 0xFFFF)
	{
		/* Yes -> Un-delete NPC */
		Write_bit_array(CD_BIT_ARRAY, (UNLONG) Char->Destination_CD_bit,
		 CLEAR_BIT_ARRAY);
	}

	MEM_Free_pointer(Party_char_handles[Member_index - 1]);

	/* Save character data */
	Save_subfile(Party_char_handles[Member_index - 1], PARTY_CHAR,
	 PARTY_DATA.Member_nrs[Member_index - 1]);

	/* Free memory */
	MEM_Kill_memory(Party_char_handles[Member_index - 1]);
	MEM_Free_memory(Small_portrait_handles[Member_index - 1]);
	MEM_Free_memory(Party_event_set_handles[Member_index - 1][0]);
	MEM_Free_memory(Party_event_set_handles[Member_index - 1][1]);
	MEM_Free_memory(Party_event_text_handles[Member_index - 1][0]);
	MEM_Free_memory(Party_event_text_handles[Member_index - 1][1]);

	/* Remove party member */
	PARTY_DATA.Member_nrs[Member_index - 1] = 0;

	/* Reset the status area */
	Reset_status_area();
}

/*
	cmp.w	Active_member,d7		; Was this the leader ?
	bne.s	.Exit
	jsr	Select_new_active_member	; Yes -> New leader
.Exit:	movem.l	(sp)+,d0-d2/d7/a0/a1
	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_party_member
 * FUNCTION  : Add a party member to the party.
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 15:26
 * LAST      : 30.06.95 15:26
 * INPUTS    : UNSHORT Char_index - Party character index.
 *             UNSHORT NPC_index - NPC index (0...) / 0xFFFF = no NPC.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Add_party_member(UNSHORT Char_index, UNSHORT NPC_index)
{
	struct Character_data *Char;
	MEM_HANDLE Char_handle;
	BOOLEAN Result;
	UNLONG CD_bit_nr;
	SISHORT i;
	UNSHORT New_member_index = 0;
	UNSHORT Occupied_battle_order_slots;
	UNSHORT Batch[2];

	/* Find a free slot in the party */
	for (i=0;i<6;i++)
	{
		if (!PARTY_DATA.Member_nrs[i])
		{
			New_member_index = i + 1;
			break;
		}
	}

	/* Found one ? */
	if (!New_member_index)
	{
		/* No -> Error */
		Error(ERROR_NO_PLACE_IN_PARTY);
		return FALSE;
	}

	/* Load character data */
	Char_handle = Load_subfile(PARTY_CHAR, Char_index);
	if (!Char_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Store character data handle */
	Party_char_handles[New_member_index - 1] = Char_handle;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Load small portrait */
	Small_portrait_handles[New_member_index - 1] =
	 Load_subfile(SMALL_PORTRAIT, (UNSHORT) Char->Portrait_nr);
	if (!Small_portrait_handles[New_member_index - 1])
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Build event set batch */
	Batch[0] = (UNSHORT) Char->Event_set_1_nr;
	Batch[1] = (UNSHORT) Char->Event_set_2_nr;

	/* Load event sets */
	Result = Load_partial_batch(EVENT_SET, 2, Batch,
	 &Party_event_set_handles[New_member_index - 1][0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load event texts */
	Result = Load_partial_batch(EVENT_TEXT, 2, Batch,
	 &Party_event_text_handles[New_member_index - 1][0]);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Was this an NPC ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Does this person already have a CD bit number ? */
		if (Char->Destination_CD_bit == 0xFFFF)
		{
			/* No ->Calculate CD bit number */
			CD_bit_nr = ((PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP) + NPC_index;

			/* Store */
			Char->Destination_CD_bit = CD_bit_nr;
		}

		/* Delete NPC */
		Write_bit_array(CD_BIT_ARRAY, CD_bit_nr, SET_BIT_ARRAY);
	}

	MEM_Free_pointer(Char_handle);

	/* Build list of occupied battle order slots */
	Occupied_battle_order_slots = 0;
	for (i=0;i<6;i++)
	{
		if (PARTY_DATA.Member_nrs[i])
		{
			Occupied_battle_order_slots |= (1 << PARTY_DATA.Battle_order[i]);
		}
	}

	/* Search for a free battle order slot */
	for (i=11;i>=0;i--)
	{
		if (!(Occupied_battle_order_slots & (1 << i)))
		{
			PARTY_DATA.Battle_order[New_member_index - 1] = i;
		}
	}

	/* Insert character in party */
	PARTY_DATA.Member_nrs[New_member_index - 1] = Char_index;

	/* Reset the status area */
	Reset_status_area();

	return TRUE;
}

