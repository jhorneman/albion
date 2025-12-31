/************
 * NAME     : MAINMENU.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 22-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MAINMENU.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>

#include <BBDEF.H>
#include <BBMEM.H>
#include <BBDOS.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <MAIN.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <MAINMENU.H>
#include <PRTLOGIC.H>
#include <GAMETEXT.H>
#include <BOOLREQ.H>
#include <COLOURS.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <TEXTWIN.H>
#include <INPUTNR.H>
#include <INPUT.H>
#include <BUTTONS.H>
#include <SOUND.H>
#include <SAVELOAD.H>
#include <SELWIN.H>
#include <POPUP.H>
#include <COMBAT.H>
#include <VERSION.H>
#include <DIAGNOST.H>

/* defines */

#define MAX_SAVED_GAMES					(100)
#define MAX_SAVED_GAME_NAME_LENGTH	(40)

#define OPTIONS_WIDTH		(170)
#define OPTIONS_HEIGHT		(174)

/* structure definitions */

/* Options window object */
struct Options_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;
};

/* prototypes */

/* Main menu module functions */
void Main_menu_ModInit(void);
void Main_menu_ModExit(void);
void Main_menu_DisInit(void);
void Main_menu_DisExit(void);
void Main_menu_MainLoop(void);

/* Main menu pop-up menu functions */
void Main_menu_PUM_evaluator(struct PUM *PUM);

void Main_menu_Start_new_game(UNLONG Data);
void Main_menu_Continue_game(UNLONG Data);
void Main_menu_Load_game(UNLONG Data);
void Main_menu_Save_game(UNLONG Data);
void Main_menu_Options(UNLONG Data);
void Main_menu_View_intro(UNLONG Data);
void Main_menu_View_credits(UNLONG Data);
void Main_menu_End_game(UNLONG Data);

void Build_list_of_saved_games(void);

UNSHORT Load_saved_game_evaluator(UNSHORT Index, UNBYTE *Data);

void Draw_saved_game(struct OPM *OPM, struct Object *Object, UNSHORT Index,
 UNBYTE *Data, BOOLEAN Blocked);

/* Options window object methods */
UNLONG Init_Options_object(struct Object *Object, union Method_parms *P);
UNLONG Exit_Options_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Options_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_Options_object(struct Object *Object, union Method_parms *P);

void Options_Exit(struct Button_object *Button);

/* global variables */

static UNSHORT Main_menu_mode;
static UNSHORT Main_menu_picture_nr;

static SILONG Main_menu_MIDI_volume;
static SILONG Main_menu_digital_volume;
static SILONG Main_menu_3D_window_size;
static SILONG Main_menu_combat_detail_level;
static SILONG Main_menu_combat_message_speed;

static MEM_HANDLE Saved_game_list_handle;
static UNSHORT Nr_saved_games;

static BOOLEAN Game_saved;

static UNSHORT Secret_word_index = 0;
static UNCHAR Secret_word[] = "SCHNOSM";

/* Module */
static struct Module Main_menu_Mod = {
	0, SCREEN_MOD, MAIN_MENU_SCREEN,
	Main_menu_MainLoop,
	Main_menu_ModInit,
	Main_menu_ModExit,
	Main_menu_DisInit,
	Main_menu_DisExit,
	NULL
};

/* Main menu pop-up menu */
static struct PUME Main_menu_PUMEs[] = {
	{PUME_AUTO_CLOSE,			0, 703,	Main_menu_Continue_game},
	{PUME_NOT_SELECTABLE,	0, 0,		NULL},
	{PUME_AUTO_CLOSE,			0, 702,	Main_menu_Start_new_game},
	{PUME_AUTO_CLOSE,			0, 704,	Main_menu_Load_game},
	{PUME_AUTO_CLOSE,			0, 705,	Main_menu_Save_game},
	{PUME_NOT_SELECTABLE,	0, 0,		NULL},
	{PUME_AUTO_CLOSE,			0, 706,	Main_menu_Options},
	{PUME_AUTO_CLOSE,			0, 707,	Main_menu_View_intro},
	{PUME_AUTO_CLOSE,			0, 708,	Main_menu_View_credits},
	{PUME_NOT_SELECTABLE,	0, 0,		NULL},
	{PUME_AUTO_CLOSE,			0, 709,	Main_menu_End_game}
};

static struct PUM Main_menu_PUM = {
	11,
	NULL,
	0,
	Main_menu_PUM_evaluator,
	Main_menu_PUMEs
};

/* Options window method list */
static struct Method Options_methods[] = {
	{ INIT_METHOD,			Init_Options_object },
	{ EXIT_METHOD,			Exit_Options_object },
	{ DRAW_METHOD,			Draw_Options_object },
	{ UPDATE_METHOD,		Update_help_line },
	{ CLOSE_METHOD,		Close_Window_object },
	{ RIGHT_METHOD,		Close_Window_object },
	{ TOUCHED_METHOD,		Dehighlight },
	{ RESTORE_METHOD,		Restore_window },
	{ CUSTOMKEY_METHOD,	Customkeys_Options_object },
	{ 0, NULL}
};

/* Options window class description */
static struct Object_class Options_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Options_window_object),
	&Options_methods[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_Main_menu
 * FUNCTION  : Enter Main menu screen.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:50
 * LAST      : 02.10.95 17:46
 * INPUTS    : UNSHORT Mode - Main menu mode.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Enter_Main_menu(UNSHORT Mode)
{
	/* Store mode */
	Main_menu_mode = Mode;

	/* First time ? */
	if (Main_menu_mode == MAIN_MENU_PROGRAM_START)
	{
		/* Yes -> Make the main menu a local module */
		Main_menu_Mod.Flags |= LOCAL_MOD;
	}
	else
	{
		/* No -> Make the main menu a global module */
		Main_menu_Mod.Flags &= ~LOCAL_MOD;
	}

	/* Enter Main menu screen */
	Exit_display();
	Push_module(&Main_menu_Mod);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_ModInit
 * FUNCTION  : Initialize Main menu module.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:51
 * LAST      : 29.10.95 18:39
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_ModInit(void)
{
	Push_root(&Main_OPM);

	/* No game was saved until now */
	Game_saved = FALSE;

	/* Reset palette fading */
	Reset_palette_fading();

	/* Switch sound off */
	SOUND_Sound_off();

	/* Choose a picture */
	Main_menu_picture_nr = 20 + rand() % 8;

	/* Load picture */
	Load_small_picture(Main_menu_picture_nr);

	/* Initialize display */
	Main_menu_DisInit();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_ModExit
 * FUNCTION  : Exit Main menu module.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:52
 * LAST      : 17.10.95 22:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_ModExit(void)
{
	Destroy_small_picture();

	Pop_root();

	/* Switch sound back on */
	SOUND_Sound_on();

	/* Restart or quit ? */
	if (Restart_game_flag || Quit_program_flag)
	{
		/* Yes -> Pop all modules */
		Reset_module_stack();

		/* Stop music */
		SOUND_Play_song(0);
		SOUND_Play_ambient_song(0);

		/* Switch sound effects off */
		SOUND_Sound_effects_off();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_DisInit
 * FUNCTION  : Initialise Main menu display.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:52
 * LAST      : 30.09.95 14:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_DisInit(void)
{
	UNCHAR Version_string[VERSION_STRING_LENGTH];

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

	/* Draw picture */
	Display_small_picture_centered
	(
		&Main_OPM,
		0,
		0,
		Screen_width,
		Panel_Y
	);


	/* Get Albion version */
	Get_version_string(Version_string);

	/* Print Albion version */
	Set_ink(WHITE_TEXT);
	xprintf
	(
		&Main_OPM,
		5,
		Panel_Y - 25,
		"Albion %s",
		Version_string
	);
	Print_string
	(
		&Main_OPM,
		5,
		Panel_Y - 15,
		"Copyright 1995 Blue Byte Software GmbH"
	);

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_DisExit
 * FUNCTION  : Exit Main menu display.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.09.95 15:53
 * LAST      : 17.10.95 23:14
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_DisExit(void)
{
	/* Display slab */
	OPM_CopyOPMOPM
	(
		&Slab_OPM,
		&Main_OPM,
		0,
		0,
		360,
		Panel_Y,
		0,
		0
	);

	/* Restart or quit ? */
	if (!Restart_game_flag && !Quit_program_flag)
	{
		/* No -> Does the party display object exist ? */
		if (Is_object_present(Party_display_object))
		{
			/* Yes -> Redraw it */
			Execute_method(Party_display_object, DRAW_METHOD, NULL);
		}
	}

	Switch_screens();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_MainLoop
 * FUNCTION  : Main loop of Main menu.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 17:52
 * LAST      : 27.09.95 17:52
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_MainLoop(void)
{
	/* Do main menu pop-up menu */
	Main_menu_PUM.Title = System_text_ptrs[700];

	PUM_source_object_handle = 0;

	Do_centered_PUM
	(
		0,
		0,
		Screen_width,
		Panel_Y,
		&Main_menu_PUM
	);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_PUM_evaluator
 * FUNCTION  : Evaluate main menu pop-up menu.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 17:40
 * LAST      : 26.10.95 13:49
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_PUM_evaluator(struct PUM *PUM)
{
	struct PUME *PUMES;

	PUMES = PUM->PUME_list;

	/* Game over ? */
	if (Main_menu_mode == MAIN_MENU_GAME_OVER)
	{
		/* Yes -> Cannot continue game */
		PUMES[0].Flags |= PUME_ABSENT;
	}

	/* In map screen ? */
	if (Main_menu_mode != MAIN_MENU_MAP)
	{
		/* No -> Cannot save game */
		PUMES[4].Flags |= PUME_ABSENT;
	}

	/* In combat screen ? */
	if (Main_menu_mode == MAIN_MENU_COMBAT)
	{
		/* Yes -> Cannot view intro or credits */
		PUMES[7].Flags |= PUME_ABSENT;
		PUMES[8].Flags |= PUME_ABSENT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_Start_new_game
 * FUNCTION  : Start a new game (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 15:51
 * LAST      : 27.09.95 15:51
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_Start_new_game(UNLONG Data)
{
	/* Are you sure ? */
	if (Boolean_requester(System_text_ptrs[715]))
	{
		/* Yes -> Set restart data */
		Restart_game_flag = TRUE;
		Load_game_nr = 0;

		/* Exit main menu */
		Exit_display();
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_Continue_game
 * FUNCTION  : Continue game (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 23:51
 * LAST      : 23.10.95 18:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_Continue_game(UNLONG Data)
{
	BOOLEAN Result;
	UNCHAR Number[10];
	UNSHORT Value;

	/* First time ? */
	if (Main_menu_mode == MAIN_MENU_PROGRAM_START)
	{
		/* Yes -> Read most recently saved saved game */
		Result = Read_INI_variable
		(
			"ALBION",
			"SAVED_GAME_NR",
			Number,
			10
		);

		/* Found ? */
		if (Result)
		{
			/* Yes -> Determine saved game number */
			Value = (UNSHORT) atoi(Number);

			/* Is a legal number ?
			  (note MAX_SAVED_GAMES is legal here) */
			if ((Value > 0) && (Value <= MAX_SAVED_GAMES))
			{
				/* Yes -> Select first data path */
				Select_data_path(0);

				/* Select saved game */
				Select_saved_game(Value);

				/* Does this saved game file exist ? */
				if (!File_exists(Save_game_fname))
				{
					/* No -> Restart the game */
					Value = 0;
				}
			}

			/* Load this saved game */
			Load_game_nr = Value;
		}
		else
		{
			/* No -> Restart the game */
			Load_game_nr = 0;
		}

		/* Set restart data */
		Restart_game_flag = TRUE;

		/* Exit main menu */
		Exit_display();
		Pop_module();
	}
	else
	{
		/* No -> Exit main menu */
		Exit_display();
		Pop_module();
		Init_display();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_Load_game
 * FUNCTION  : Load a saved game (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 13:41
 * LAST      : 23.10.95 18:41
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_Load_game(UNLONG Data)
{
	UNSHORT Selected_save_game_nr;

	/* Build list of existing saved games */
	Build_list_of_saved_games();

	/* Any ? */
	if (Nr_saved_games)
	{
		/* Yes -> Select a saved game */
		Selected_save_game_nr = Do_select_window
		(
			System_text_ptrs[711],
			Load_saved_game_evaluator,
			Draw_saved_game,
			MAX_SAVED_GAMES - 1,
			320,
			12,
			NULL
		);

		/* Any selected ? */
		if (Selected_save_game_nr != 0xFFFF)
		{
			/* Yes -> Set restart data */
			Restart_game_flag = TRUE;
			Load_game_nr = Selected_save_game_nr + 1;

			/* Exit main menu */
			Exit_display();
			Pop_module();
		}
	}
	else
	{
		/* No -> Tell the player */
		Do_text_window(System_text_ptrs[717]);
	}

	/* Destroy saved game name list */
	MEM_Free_memory(Saved_game_list_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_Save_game
 * FUNCTION  : Save game (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 15:37
 * LAST      : 23.10.95 18:40
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_Save_game(UNLONG Data)
{
	UNSHORT Selected_save_game_nr;
	UNSHORT Length;
	UNCHAR Buffer[100];
	UNCHAR *Ptr;

	/* Build list of existing saved games */
	Build_list_of_saved_games();

	/* Select a saved game */
	Selected_save_game_nr = Do_select_window
	(
		System_text_ptrs[712],
		NULL,
		Draw_saved_game,
		MAX_SAVED_GAMES - 1,
		320,
		12,
		NULL
	);

	/* Any selected ? */
	if (Selected_save_game_nr != 0xFFFF)
	{
		/* Yes -> Calculate pointer to saved game name */
		Ptr = MEM_Claim_pointer(Saved_game_list_handle) +
		 (Selected_save_game_nr * (MAX_SAVED_GAME_NAME_LENGTH + 1));

		/* Get saved game name length */
		Length = strlen(Ptr);

		MEM_Free_pointer(Saved_game_list_handle);

		/* Does this saved game already exist ? */
		if (Length)
		{
			/* Yes -> Are you sure ? */
			if (Boolean_requester(System_text_ptrs[713]))
			{
				/* Yes -> Copy saved game name to input buffer
				  (note I'm using Ptr although the handle was freed!) */
				strncpy
				(
					Buffer,
					Ptr,
					MAX_SAVED_GAME_NAME_LENGTH
				);
				Buffer[MAX_SAVED_GAME_NAME_LENGTH] = '\0';

				/* Input saved game name */
				Input_string_in_window
				(
					MAX_SAVED_GAME_NAME_LENGTH,
					System_text_ptrs[714],
					Buffer
				);

				/* Anything entered ? */
				if (strlen(Buffer))
				{
					/* Yes -> Save game state */
					Save_game_state(Selected_save_game_nr + 1, Buffer);

					/* The current position was saved */
					Game_saved = TRUE;
				}
			}
		}
		else
		{
			/* No -> Input saved game name */
			Buffer[0] = '\0';
			Input_string_in_window
			(
				MAX_SAVED_GAME_NAME_LENGTH,
				System_text_ptrs[714],
				Buffer
			);

			/* Anything entered ? */
			if (strlen(Buffer))
			{
				/* Yes -> Save game state */
				Save_game_state(Selected_save_game_nr + 1, Buffer);

				/* The current position was saved */
				Game_saved = TRUE;
			}
		}
	}

	/* Destroy saved game name list */
	MEM_Free_memory(Saved_game_list_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_Options
 * FUNCTION  : Options (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 15:01
 * LAST      : 10.10.95 15:01
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_Options(UNLONG Data)
{
	UNSHORT Obj;

	/* Do it */
	Push_module(&Window_Mod);

	Obj = Add_object
	(
		0,
		&Options_Class,
		NULL,
		(Screen_width - OPTIONS_WIDTH) / 2,
		(Panel_Y	- OPTIONS_HEIGHT) / 2,
		OPTIONS_WIDTH,
		OPTIONS_HEIGHT
	);

	Execute_method(Obj, DRAW_METHOD, NULL);

	Wait_4_object(Obj);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_View_intro
 * FUNCTION  : View intro (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 23:00
 * LAST      : 23.10.95 18:39
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_View_intro(UNLONG Data)
{
	/* In map screen ? */
	if (Main_menu_mode == MAIN_MENU_MAP)
	{
		/* Yes -> Save current game state */
		Save_game_state(MAX_SAVED_GAMES, "Temporary saved game");

		/* The current position was saved */
		Game_saved = TRUE;
	}

	/* Give show intro command */
	Main_program_command = SHOW_INTRO_COMMAND;

	/* "Quit" */
	Quit_program_flag = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_View_credits
 * FUNCTION  : View credits (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 16:18
 * LAST      : 23.10.95 18:39
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_View_credits(UNLONG Data)
{
	/* In map screen ? */
	if (Main_menu_mode == MAIN_MENU_MAP)
	{
		/* Yes -> Save current game state */
		Save_game_state(MAX_SAVED_GAMES, "Temporary saved game");

		/* The current position was saved */
		Game_saved = TRUE;
	}

	/* Give show credits command */
	Main_program_command = SHOW_CREDITS_COMMAND;

	/* Quit */
	Quit_program_flag = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_menu_End_game
 * FUNCTION  : End game (main menu pop-up menu).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 22:54
 * LAST      : 31.10.95 13:00
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_menu_End_game(UNLONG Data)
{
	/* Are you sure ? */
	if (Boolean_requester(System_text_ptrs[710]))
	{
		/* Yes -> In map screen / has no game been saved yet ? */
		if ((Main_menu_mode == MAIN_MENU_MAP) && !Game_saved)
		{
			/* Yes -> Save current game state */
			Save_game_state(MAX_SAVED_GAMES, "Temporary saved game");

			/* The current position was saved */
			Game_saved = TRUE;
		}

		/* Give exit program command */
		Main_program_command = EXIT_PROGRAM_COMMAND;

		/* Quit */
		Quit_program_flag = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_list_of_saved_games
 * FUNCTION  : Build a list of saved games.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.09.95 23:10
 * LAST      : 23.10.95 18:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will store the number of existing saved games
 *              in Nr_saved_games.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Build_list_of_saved_games(void)
{
	struct find_t File_info;
	unsigned Return_code;

	UNLONG Length;
	UNLONG Read_bytes;
	SISHORT File_handle = -1;
	UNSHORT Saved_game_nr;
	UNCHAR *Nr = "000";
	UNCHAR *File_name;
	UNCHAR *Ptr;

	/* Clear */
	Nr_saved_games = 0;

	/* Allocate memory for saved game list */
	Saved_game_list_handle = MEM_Allocate_memory(MAX_SAVED_GAMES *
	 (MAX_SAVED_GAME_NAME_LENGTH + 1));

	/* Success ? */
	if (!Saved_game_list_handle)
	{
		/* No -> Error */
		Error(ERROR_OUT_OF_MEMORY);

		return;
	}

	/* Clear list */
	MEM_Clear_memory(Saved_game_list_handle);

	/* Select first data path */
	Select_data_path(0);

	/* Find first file */
	Return_code = _dos_findfirst
	(
		"SAVES\\SAVE.???",
		_A_NORMAL,
		&File_info
	);

	/* Found any ? */
	while (!Return_code)
	{
		/* Yes -> Get file name */
		File_name = (UNCHAR *) File_info.name;

		/* Extract saved game number */
		Nr[0] = File_name[5];
		Nr[1] = File_name[6];
		Nr[2] = File_name[7];
		Nr[3] = '\0';

		Saved_game_nr = atoi(Nr);

		/* Is a legal number ?
		  (note that MAX_SAVED_GAMES is not allowed here) */
		if ((Saved_game_nr > 0) && (Saved_game_nr < MAX_SAVED_GAMES))
		{
			/* Yes -> Select saved game */
			Select_saved_game(Saved_game_nr);

			/* Open saved game file */
			File_handle = DOS_Open
			(
				Save_game_fname,
				BBDOSFILESTAT_READ
			);

			/* Success ? */
			if (File_handle != -1)
			{
				/* Yes -> Read length of saved game name */
				Read_bytes = DOS_Read
				(
					File_handle,
					(UNBYTE *) &Length,
					sizeof(UNLONG)
				);

				/* Read everything ? */
				if (Read_bytes == sizeof(UNLONG))
				{
					/* Yes -> Truncate length if too long */
					Length = min(Length, MAX_SAVED_GAME_NAME_LENGTH);

					/* Calculate pointer to saved game name */
					Ptr = MEM_Claim_pointer(Saved_game_list_handle) +
					 ((Saved_game_nr - 1) * (MAX_SAVED_GAME_NAME_LENGTH + 1));

					/* Read saved game name */
					Read_bytes = DOS_Read
					(
						File_handle,
						(UNBYTE *) Ptr,
						Length
					);

					MEM_Free_pointer(Saved_game_list_handle);

					/* Read everything ? */
					if (Read_bytes == Length)
					{
						/* Yes -> Count up */
						Nr_saved_games++;
					}
					else
					{
						/* No -> Error */
						Error(ERROR_FILE_LOAD);
					}
				}
				else
				{
					/* No -> Error */
					Error(ERROR_FILE_LOAD);
				}

				/* Close file */
				DOS_Close(File_handle);
			}
			else
			{
				/* No -> Error */
				Error(ERROR_FILE_LOAD);
			}
		}

		/* Find next file */
		Return_code = _dos_findnext(&File_info);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_saved_game_evaluator
 * FUNCTION  : Evaluator for loading saved games (select window).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 13:43
 * LAST      : 27.09.95 15:19
 * INPUTS    : UNSHORT Index - Selectable index (0...).
 *             UNBYTE *Data - Pointer to data passed to select window.
 * RESULT    : UNSHORT : Blocked message number / 0 = entry is OK /
 *              0xFFFF = entry is absent.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Load_saved_game_evaluator(UNSHORT Index, UNBYTE *Data)
{
	UNSHORT Message_nr;
	UNCHAR *Ptr;

	/* Calculate pointer to saved game name */
	Ptr = MEM_Claim_pointer(Saved_game_list_handle) +
	 (Index * (MAX_SAVED_GAME_NAME_LENGTH + 1));

	/* Does this saved game exist ? */
	if (strlen(Ptr))
	{
		/* Yes -> Entry is OK */
		Message_nr = 0;
	}
	else
	{
		/* No -> Entry is absent */
		Message_nr = 0xFFFF;
	}

	MEM_Free_pointer(Saved_game_list_handle);

	return Message_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_saved_game
 * FUNCTION  : Saved game draw function (select window).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 13:45
 * LAST      : 06.10.95 18:21
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             struct Object *Object - Pointer to object data.
 *             UNSHORT Index - Selectable index (0...).
 *             UNBYTE *Data - Pointer to data passed to select window.
 *             BOOLEAN Blocked - Set if the entry is blocked.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_saved_game(struct OPM *OPM, struct Object *Object, UNSHORT Index,
 UNBYTE *Data, BOOLEAN Blocked)
{
	UNCHAR *Ptr;

	/* Calculate pointer to saved game name */
	Ptr = MEM_Claim_pointer(Saved_game_list_handle) +
	 (Index * (MAX_SAVED_GAME_NAME_LENGTH + 1));

	/* Does this saved game exist ? */
	if (strlen(Ptr))
	{
		/* Yes */
		Set_ink(SILVER_TEXT);

		/* Print saved game number */
		xprintf
		(
			OPM,
			Object->X + 2,
			Object->Y + 1,
			"%2u",
			Index + 1
		);

		/* Print saved game name */
		Print_string
		(
			OPM,
			Object->X + 22,
			Object->Y + 1,
			Ptr
		);
	}
	else
	{
		/* No */
		Set_ink(GREY_TEXT);

		/* Print saved game number */
		xprintf
		(
			OPM,
			Object->X + 2,
			Object->Y + 1,
			"%2u",
			Index + 1
		);

		/* Print "empty" string */
		Print_string
		(
			OPM,
			Object->X + 22,
			Object->Y + 1,
			System_text_ptrs[716]
		);
	}

	MEM_Free_pointer(Saved_game_list_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Options_object
 * FUNCTION  : Initialize method of Options window object.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 15:04
 * LAST      : 22.10.95 23:14
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Options_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data Options_exit_button_data;
	static struct Button_OID Options_exit_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		695,
		&Options_exit_button_data,
		Options_Exit
	};

	struct Number_bar_OID Number_bar_OID;

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	/* Get MIDI volume */
	Main_menu_MIDI_volume = (SILONG) SOUND_Get_MIDI_volume();

	/* Build MIDI volume bar OID */
	Number_bar_OID.Value_ptr	= &Main_menu_MIDI_volume;
	Number_bar_OID.Minimum		= 0;
	Number_bar_OID.Maximum		= 127;
	Number_bar_OID.Update		= NULL;
	Number_bar_OID.Print			= NULL;

	/* Add MIDI volume bar object */
	Add_object
	(
		Object->Self,
		&Number_bar_Class,
		(UNBYTE *) &Number_bar_OID,
		15,
		22,
		140,
		12
	);

	/* Get digital volume */
	Main_menu_digital_volume = (SILONG) SOUND_Get_digital_volume();

	/* Build digital volume bar OID */
	Number_bar_OID.Value_ptr	= &Main_menu_digital_volume;
	Number_bar_OID.Minimum		= 0;
	Number_bar_OID.Maximum		= 127;
	Number_bar_OID.Update		= NULL;
	Number_bar_OID.Print			= NULL;

	/* Add digital volume bar object */
	Add_object
	(
		Object->Self,
		&Number_bar_Class,
		(UNBYTE *) &Number_bar_OID,
		15,
		49,
		140,
		12
	);

	/* Get 3D window size */
	Main_menu_3D_window_size = (SILONG) Window_3D_size;

	/* Build 3D window size bar OID */
	Number_bar_OID.Value_ptr	= &Main_menu_3D_window_size;
	Number_bar_OID.Minimum		= 0;
	Number_bar_OID.Maximum		= 100;
	Number_bar_OID.Update		= NULL;
	Number_bar_OID.Print			= NULL;

	/* Add 3D window size bar object */
	Add_object
	(
		Object->Self,
		&Number_bar_Class,
		(UNBYTE *) &Number_bar_OID,
		15,
		76,
		140,
		12
	);

	/* Get combat detail level */
	Main_menu_combat_detail_level = (SILONG) Combat_display_detail_level;

	/* Build combat detail level bar OID */
	Number_bar_OID.Value_ptr	= &Main_menu_combat_detail_level;
	Number_bar_OID.Minimum		= MIN_COMBAT_DISPLAY_DETAIL_LEVEL;
	Number_bar_OID.Maximum		= MAX_COMBAT_DISPLAY_DETAIL_LEVEL;
	Number_bar_OID.Update		= NULL;
	Number_bar_OID.Print			= NULL;

	/* Add combat detail level bar object */
	Add_object
	(
		Object->Self,
		&Number_bar_Class,
		(UNBYTE *) &Number_bar_OID,
		15,
		103,
		140,
		12
	);

	/* Get combat message speed */
	Main_menu_combat_message_speed = (SILONG) Combat_message_speed;

	/* Build combat message speed bar OID */
	Number_bar_OID.Value_ptr	= &Main_menu_combat_message_speed;
	Number_bar_OID.Minimum		= MIN_COMBAT_MESSAGE_SPEED;
	Number_bar_OID.Maximum		= MAX_COMBAT_MESSAGE_SPEED;
	Number_bar_OID.Update		= NULL;
	Number_bar_OID.Print			= NULL;

	/* Add combat message speed bar object */
	Add_object
	(
		Object->Self,
		&Number_bar_Class,
		(UNBYTE *) &Number_bar_OID,
		15,
		130,
		140,
		12
	);

	/* Initialize exit button data */
	Options_exit_button_data.Text_button_data.Text = System_text_ptrs[64];

	/* Add exit button to window */
 	Add_object
	(
		Object->Self,
		&Button_Class,
		(UNBYTE *) &Options_exit_button_OID,
		(Object->Rect.width - 50) / 2,
		OPTIONS_HEIGHT - 22,
		50,
		11
	);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_Options_object
 * FUNCTION  : Exit method of Options window object.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 17:09
 * LAST      : 22.10.95 23:16
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Exit_Options_object(struct Object *Object, union Method_parms *P)
{
	UNCHAR Number[10];

	/* Store MIDI volume in INI file */
	_bprintf(Number, 10, "%u", Main_menu_MIDI_volume);
	Write_INI_variable
	(
		"ALBION",
		"MIDI_VOLUME",
		Number
	);

	/* Set MIDI volume */
	SOUND_Set_MIDI_volume((UNSHORT) Main_menu_MIDI_volume);

	/* Store digital volume in INI file */
	_bprintf(Number, 10, "%u", Main_menu_digital_volume);
	Write_INI_variable
	(
		"ALBION",
		"DIGI_VOLUME",
		Number
	);

	/* Set digital volume */
	SOUND_Set_digital_volume((UNSHORT) Main_menu_digital_volume);

	/* Store 3D window size in INI file */
	_bprintf(Number, 10, "%u", Main_menu_3D_window_size);
	Write_INI_variable
	(
		"ALBION",
		"3D_WINDOW_SIZE",
		Number
	);

	/* Store 3D window size */
	Window_3D_size = (UNSHORT) Main_menu_3D_window_size;

	/* Store combat detail level in INI file */
	_bprintf(Number, 10, "%u", Main_menu_combat_detail_level);
	Write_INI_variable
	(
		"ALBION",
		"COMBAT_DETAIL_LEVEL",
		Number
	);

	/* Store combat detail level */
	Combat_display_detail_level = (UNSHORT) Main_menu_combat_detail_level;

	/* Store combat message speed in INI file */
	_bprintf(Number, 10, "%u", Main_menu_combat_message_speed);
	Write_INI_variable
	(
		"ALBION",
		"COMBAT_MESSAGE_SPEED",
		Number
	);

	/* Store combat message speed */
	Combat_message_speed = Main_menu_combat_message_speed;

	/* Exit Window */
	Exit_Window_object(Object, P);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Options_object
 * FUNCTION  : Draw method of Options window object.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 17:02
 * LAST      : 23.10.95 12:53
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Options_object(struct Object *Object, union Method_parms *P)
{
	struct Options_window_object *Options_window;
	struct OPM *OPM;

	Options_window = (struct Options_window_object *) Object;
	OPM = &(Options_window->Window_OPM);

	/* Draw window */
	Draw_Window_object(Object, P);

	/* Print number bar descriptions */
	Print_centered_string
	(
		OPM,
		15,
		12,
		140,
		System_text_ptrs[741]
	);

	Print_centered_string
	(
		OPM,
		15,
		39,
		140,
		System_text_ptrs[742]
	);

	Print_centered_string
	(
		OPM,
		15,
		66,
		140,
		System_text_ptrs[743]
	);

	Print_centered_string
	(
		OPM,
		15,
		93,
		140,
		System_text_ptrs[415]
	);

	Print_centered_string
	(
		OPM,
		15,
		120,
		140,
		System_text_ptrs[416]
	);

	/* Draw children */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_Options_object
 * FUNCTION  : Customkey method of Options window object.
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.95 21:38
 * LAST      : 27.10.95 22:13
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : Reacted to key.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_Options_object(struct Object *Object, union Method_parms *P)
{
	UNSHORT Key_code;

	/* Exit if cheat keys are already activated */
	if (Cheat_keys_activated)
		return 0;

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	/* Is this key the current key in the secret word ? */
	if (Key_code == (UNSHORT) tolower(Secret_word[Secret_word_index]))
	{
		/* Yes -> Increase secret word index */
		Secret_word_index++;

		/* Has the entire secret word been entered ? */
		if (Secret_word_index == strlen(Secret_word))
		{
			/* Yay! */
			Cheat_keys_activated = TRUE;

			Fade_to_white();
			Fade_from_white();
		}
	}
	else
	{
		/* No -> Reset secret word index */
		Secret_word_index = 0;
	}

	return 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Options_Exit
 * FUNCTION  : Exit Options window (button).
 * FILE      : MAINMENU.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.10.95 15:09
 * LAST      : 10.10.95 15:09
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Options_Exit(struct Button_object *Button)
{
	Delete_object(Button->Object.Parent);
}

