/************
 * NAME     : SCRIPT.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 31-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBSYSTEM.H>
#include <BBERROR.H>

#include <ALBION.H>

#include <XLOAD.H>
#include <TEXT.H>
#include <GFXFUNC.H>
#include <FLC.H>
#include <HDOB.H>

#include <MAIN.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <GAMETIME.H>
#include <PRTLOGIC.H>
#include <FONT.H>
#include <XFTYPES.H>
#include <SCRIPT.H>
#include <GAMETEXT.H>
#include <STATAREA.H>
#include <CONTROL.H>
#include <2D_MAP.H>
#include <2D_DISPL.H>
#include <2D_COLL.H>
#include <2D_PATH.H>
#include <3DCOMMON.H>
#include <EVELOGIC.H>
#include <GRAPHICS.H>
#include <COLOURS.H>
#include <MUSIC.H>
#include <ICCHANGE.H>
#include <TEXTWIN.H>

/* defines */

#define MAX_SCRIPT_COMMANDS 	(43)
#define MAX_SCRIPT_LINES		(2000)

/* Wipe types */
#define PLAIN_WIPE_TYPE					(0)
#define WIPE_WIPE_TYPE					(1)
#define FADE_TO_BLACK_WIPE_TYPE		(2)
#define FADE_TO_WHITE_WIPE_TYPE		(3)
#define FADE_FROM_BLACK_WIPE_TYPE	(4)
#define FADE_FROM_WHITE_WIPE_TYPE	(5)

/* Error codes */
#define SCRIPTERR_UNPARSABLE_LINE	(1)
#define SCRIPTERR_RECURSION			(2)
#define SCRIPTERR_TOO_MANY_LINES		(3)

#define SCRIPTERR_MAX					(3)

/* enumerations */

enum eScript_line_type {
	EMPTY_SCRIPT_LINE,
	COMMAND_SCRIPT_LINE,
	COMMENT_SCRIPT_LINE,
	UNPARSABLE_SCRIPT_LINE
};

enum eScript_bg_type {
	NO_SCRIPT_BG,
	MAP_2D_SCRIPT_BG,
	PICTURE_SCRIPT_BG,
	FLIC_SCRIPT_BG
};

/* type definitions */

typedef enum eScript_line_type SCRIPT_LINE_TYPE_T;
typedef enum eScript_bg_type SCRIPT_BG_TYPE_T;

typedef BOOLEAN (*Script_command_handler)(UNCHAR *Script_ptr);

/* structure definitions */

struct Script_line_data {
	SCRIPT_LINE_TYPE_T Line_type;
	UNLONG Offset;
};

/* Script error data */
struct Script_error_data {
	UNSHORT Code;
	UNSHORT Line_nr;
	UNSHORT Script_nr;
};

/* prototypes */

/* Script module */
void Script_ModInit(void);
void Script_MainLoop(void);
void Script_DisUpd(void);

/* Script support functions */
UNSHORT Analyse_script(MEM_HANDLE Script_handle,
 MEM_HANDLE Script_line_data_handle);
UNCHAR *Read_numeric_argument(UNCHAR *Script_ptr, SILONG *Value_ptr);

/* Script command handlers */
BOOLEAN Party_jump_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Party_move_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN NPC_jump_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN NPC_move_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Show_map_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Do_event_chain_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Show_pic_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Sound_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Song_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Ambient_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Set_quest_bit_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Clear_quest_bit_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN NPC_on_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN NPC_off_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Lock_NPC_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Unlock_NPC_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Change_icon_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Wipe_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Play_anim_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Text_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Party_member_text_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN NPC_text_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Active_member_text_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Pause_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Fill_screen_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Restore_pal_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Start_anim_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Play_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Stop_anim_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Load_pal_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Turn_party_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Turn_NPC_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Lock_camera_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Unlock_camera_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Camera_jump_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Camera_move_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Update_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Fade_to_black_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Fade_to_white_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Fade_from_black_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Fade_from_white_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Party_on_script_command_handler(UNCHAR *Script_ptr);
BOOLEAN Party_off_script_command_handler(UNCHAR *Script_ptr);

/* Script error functions */
void Script_error(UNSHORT Error_code);
void Script_print_error(UNCHAR *buffer, UNBYTE *data);

/* Animation playback functions */
UNSHORT Start_animation(UNSHORT Flic_nr, SISHORT Playback_X,
 SISHORT Playback_Y, UNSHORT Play_mode);
void Play_animation_frames(UNSHORT Nr_frames);
void Stop_animation(void);

/* global variables */

/* Script variables */
BOOLEAN In_Script;

static UNSHORT Current_script_nr;
static UNSHORT Current_script_line_nr;
static SCRIPT_BG_TYPE_T Script_background_type;

static MEM_HANDLE SCRIPT_Script_handle;
static MEM_HANDLE SCRIPT_Script_line_data_handle;
static BOOLEAN Abort_script_flag;
static UNSHORT Nr_lines_in_script;

/* Animation variables */
static BOOLEAN ANIM_Playing;

static MEM_HANDLE ANIM_Flic_handle;

static struct BBRECT ANIM_New_clip;

static struct Flic_playback ANIM_FLC;

static UNLONG ANIM_dT;

static UNSHORT ANIM_Play_mode;

/* Script command handler table */
static Script_command_handler Script_command_handlers[MAX_SCRIPT_COMMANDS] = {
	Party_jump_script_command_handler,
	Party_move_script_command_handler,
	NPC_jump_script_command_handler,
	NPC_move_script_command_handler,
	Show_map_script_command_handler,
	Do_event_chain_script_command_handler,
	Show_pic_script_command_handler,
	Sound_script_command_handler,
	Song_script_command_handler,
	Ambient_script_command_handler,
	Set_quest_bit_script_command_handler,
	Clear_quest_bit_script_command_handler,
	NPC_on_script_command_handler,
	NPC_off_script_command_handler,
	Lock_NPC_script_command_handler,
	Unlock_NPC_script_command_handler,
	Change_icon_script_command_handler,
	Wipe_script_command_handler,
	Play_anim_script_command_handler,
	Text_script_command_handler,
	Party_member_text_script_command_handler,
	NPC_text_script_command_handler,
	Active_member_text_script_command_handler,
	Pause_script_command_handler,
	Fill_screen_script_command_handler,
	Restore_pal_script_command_handler,
	Start_anim_script_command_handler,
	Play_script_command_handler,
	Stop_anim_script_command_handler,
	Load_pal_script_command_handler,
	Turn_party_script_command_handler,
	Turn_NPC_script_command_handler,
	Lock_camera_script_command_handler,
	Unlock_camera_script_command_handler,
	Camera_jump_script_command_handler,
	Camera_move_script_command_handler,
	Update_script_command_handler,
	Fade_to_black_script_command_handler,
	Fade_to_white_script_command_handler,
	Fade_from_black_script_command_handler,
	Fade_from_white_script_command_handler,
	Party_on_script_command_handler,
	Party_off_script_command_handler
};

/* Script command table */
static UNCHAR *Script_commands[MAX_SCRIPT_COMMANDS] = {
	"party_jump",
	"party_move",
	"npc_jump",
	"npc_move",
	"show_map",
	"do_event_chain",
	"show_pic",
	"sound",
	"song",
	"ambient",
	"set_quest_bit",
	"clear_quest_bit",
	"npc_on",
	"npc_off",
	"npc_lock",
	"npc_unlock",
	"change_icon",
	"wipe",
	"play_anim",
	"text",
	"party_member_text",
	"npc_text",
	"active_member_text",
	"pause",
	"fill_screen",
	"restore_pal",
	"start_anim",
	"play",
	"stop_anim",
	"load_pal",
	"party_turn",
	"npc_turn",
	"camera_lock",
	"camera_unlock",
	"camera_jump",
	"camera_move",
	"update",
	"fade_to_black",
	"fade_to_white",
	"fade_from_black",
	"fade_from_white",
	"party_on",
	"party_off"
};

/* Script module */
static struct Module Script_Mod = {
	LOCAL_MOD, SCREEN_MOD, NO_SCREEN,
	Script_MainLoop,
	Script_ModInit,
	NULL,
	NULL,
	NULL,
	Script_DisUpd
};

/* Error handling data */
static UNCHAR Script_library_name[] = "Script";

static UNCHAR *Script_error_strings[SCRIPTERR_MAX + 1] = {
	"Illegal error code.",
	"Line %u in script %u could not be parsed.",
	"Tried to start script while executing script.",
	"Too many lines in script."
};

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_script
 * FUNCTION  : Execute a forced control script.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.07.95 16:29
 * LAST      : 13.08.95 19:45
 * INPUTS    : UNSHORT Script_index - Script file index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_script(UNSHORT Script_index)
{
	/* Already in script ? */
	if (In_Script)
	{
		/* Yes -> Error */
		Script_error(SCRIPTERR_RECURSION);
	}
	else
	{
		/* No -> Try to load script file */
		SCRIPT_Script_handle = Load_subfile(SCRIPT, Script_index);
		if (!SCRIPT_Script_handle)
		{
			Error(ERROR_FILE_LOAD);

			Exit_program();

			return;
		}

		/* Store script number */
		Current_script_nr = Script_index;

		/* Do script */
		Push_module(&Script_Mod);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Script_ModInit
 * FUNCTION  : Initialize Script module.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 18:08
 * LAST      : 15.08.95 16:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Script_ModInit(void)
{
	struct Script_line_data *Script_line_data_ptr;
	UNSHORT Command_index;
	UNSHORT i;
	UNCHAR *Script_ptr;
	UNCHAR *Script_start_ptr;
	UNCHAR Char;

	/* In script */
	In_Script = TRUE;

	/* Install stuff */
	Push_root(&Main_OPM);
	Push_mouse(&(Mouse_pointers[SCRIPT_MPTR]));

	/* Add update OPM */
	Add_update_OPM(&Status_area_OPM);

	/* 2D map ? */
	if (!_3D_map)
	{
		/* Yes -> Add OPMs to update list */
		for (i=0;i<4;i++)
		{
			Add_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
		}
	}

	/* Allocate and clear script line data buffer */
	SCRIPT_Script_line_data_handle = MEM_Allocate_memory(MAX_SCRIPT_LINES *
	 sizeof(struct Script_line_data));
	MEM_Clear_memory(SCRIPT_Script_line_data_handle);

	/* Analyse script */
	Nr_lines_in_script = Analyse_script(SCRIPT_Script_handle,
	 SCRIPT_Script_line_data_handle);

	/* Reset line number */
	Current_script_line_nr = 0;

	/* Reset script background type */
	Script_background_type = NO_SCRIPT_BG;
	Set_screen_type(NO_SCREEN);

	/* Execute script */
	Script_line_data_ptr = (struct Script_line_data *)
	 MEM_Claim_pointer(SCRIPT_Script_line_data_handle);

	Script_start_ptr = (UNCHAR *) MEM_Claim_pointer(SCRIPT_Script_handle);

	for (i=0;i<Nr_lines_in_script;i++)
	{
		/* Set line number */
		Current_script_line_nr = i + 1;

		/* Could the current line contain a command ? */
		if (Script_line_data_ptr[i].Line_type == COMMAND_SCRIPT_LINE)
		{
			/* Yes -> Get pointer to current line */
			Script_ptr = Script_start_ptr + Script_line_data_ptr[i].Offset;

			/* Skip leading spaces */
			Char = *Script_ptr;
			while (Char && (Char == ' '))
			{
				Script_ptr++;
				Char = *Script_ptr;
			}

			/* Parse command */
			Command_index = Parse_argument(Script_ptr, MAX_SCRIPT_COMMANDS,
			 Script_commands);

			/* Found something ? */
			if (Command_index != 0xFFFF)
			{
				/* Yes -> Skip command */
				Char = *Script_ptr;
				while (Char && (Char != ' ') && (Char != ';'))
				{
					Script_ptr++;
					Char = *Script_ptr;
				}

				/* Execute command */
				Abort_script_flag =
				 (Script_command_handlers[Command_index])(Script_ptr);

				/* Abort script / exit program ? */
				if (Abort_script_flag || Quit_program_flag)
				{
					/* Yes */
					break;
				}
			}
			else
			{
				/* No -> Indicate this line could not be parsed */
				Script_line_data_ptr[i].Line_type = UNPARSABLE_SCRIPT_LINE;

				Script_error(SCRIPTERR_UNPARSABLE_LINE);
			}
		}
	}

	MEM_Free_pointer(SCRIPT_Script_line_data_handle);
	MEM_Free_pointer(SCRIPT_Script_handle);

	/* Exit script module */
	Pop_module();

	/* Free script line data buffer */
	MEM_Free_memory(SCRIPT_Script_line_data_handle);

	/* Destroy script */
	MEM_Kill_memory(SCRIPT_Script_handle);

	/* Stop animation (if any) */
	Stop_animation();

	/* Unlock camera and NPCs */
	Unlock_2D_camera();
	Unlock_all_NPCs();

	/* 2D map ? */
	if (!_3D_map)
	{
		/* Yes -> Remove OPMs from update list */
		for (i=0;i<4;i++)
		{
			Remove_update_OPM(&(Scroll_2D.Scroll_OPMs[i]));
		}
	}

	/* Remove update OPM */
	Remove_update_OPM(&Status_area_OPM);

	/* Remove stuff */
	Pop_mouse();
	Pop_root();

	/* No longer in script */
	In_Script = FALSE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Script_MainLoop
 * FUNCTION  : Script main loop.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 18:14
 * LAST      : 13.08.95 15:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Script_MainLoop(void)
{
	/* Is the current script background the 2D map ? */
	if (Script_background_type == MAP_2D_SCRIPT_BG)
	{
		/* Yes -> Display the 2D map */
		M2_MainLoop();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Script_DisUpd
 * FUNCTION  : Update Script display.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 18:19
 * LAST      : 11.08.95 18:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Script_DisUpd(void)
{
	/* Is the current script background the 2D map ? */
	if (Script_background_type == MAP_2D_SCRIPT_BG)
	{
		/* Yes -> Display the 2D map */
		Display_2D_map();
	}
	else
	{
		/* No -> Do colour cycling */
//		Cycle_colours(Current_palette_nr);
	}
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Analyse_script
 * FUNCTION  : Analyse the lines of a forced control script.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 10:53
 * LAST      : 14.08.95 02:13
 * INPUTS    : MEM_HANDLE Script_handle - Handle of script file.
 *             MEM_HANDLE Script_line_data_handle - Handle of script line
 *              data buffer.
 * RESULT    : UNSHORT : Number of lines in script.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Analyse_script(MEM_HANDLE Script_handle, MEM_HANDLE Script_line_data_handle)
{
	struct Script_line_data *Script_line_data_ptr;
	BOOLEAN Exit_flag;
	UNLONG Script_length;
	UNSHORT Nr_lines;
	UNCHAR *Script_ptr;
	UNCHAR *Script_start_ptr;
	UNCHAR *Script_end_ptr;
	UNCHAR Char;

	/* Get script length */
	Script_length = MEM_Get_block_size(Script_handle);

	/* Make script one character longer */
	Script_length += 1;

	MEM_Resize_memory(Script_handle, Script_length + 1);

	/* Insert 0 at end of script */
	Script_ptr = (UNCHAR *) MEM_Claim_pointer(Script_handle);

	*(Script_ptr + Script_length - 1) = '\0';

	/* Analyse script lines */
	Nr_lines = 0;

	Script_line_data_ptr = (struct Script_line_data *)
	 MEM_Claim_pointer(Script_line_data_handle);

	Script_start_ptr = Script_ptr;
	Script_end_ptr = Script_ptr + Script_length;

	while (Script_ptr < Script_end_ptr)
	{
		/* Store offset to current line */
		Script_line_data_ptr[Nr_lines].Offset = Script_ptr - Script_start_ptr;

		/* Set default line type */
		Script_line_data_ptr[Nr_lines].Line_type = EMPTY_SCRIPT_LINE;

		/* Analyse current script line */
		Exit_flag = FALSE;
		while (!Exit_flag)
		{
			/* Get current character */
			Char = *Script_ptr++;

			/* Act depending on character type */
			switch (Char)
			{
				/* 0 (end of script) */
				case '\0':
				{
					/* Exit */
					Exit_flag = TRUE;
					break;
				}
				/* Space (must be leading space) */
				case ' ':
				{
					/* Skip */
					break;
				}
				/* Carriage Return */
				case 0x0D:
				{
					/* Replace the character with a 0 */
					*(Script_ptr - 1) = '\0';

					/* Replace the next character (presumably 0x0A) with a 0 */
					if (*Script_ptr == 0x0A)
					{
						*Script_ptr = '\0';
					}

 					/* Exit */
					Exit_flag = TRUE;
					break;
				}
				/* Semicolon */
				case ';':
				{
					/* Indicate this line is a comment */
					Script_line_data_ptr[Nr_lines].Line_type =
					 COMMENT_SCRIPT_LINE;

					/* Exit */
					Exit_flag = TRUE;
					break;
				}
				/* A character - probably the start of a command */
				default:
				{
					/* Indicate this line is a command */
					Script_line_data_ptr[Nr_lines].Line_type =
					 COMMAND_SCRIPT_LINE;

					/* Exit */
					Exit_flag = TRUE;
					break;
				}
			}
		}

		/* Skip to end of line */
		do
		{
			Char = *Script_ptr++;
		}
		while (Char && (Char != 0x0D));

		/* Was 0x0D found ? */
		if (Char == 0x0D)
		{
			/* Yes -> Replace the character with a 0 */
			*(Script_ptr - 1) = '\0';

			/* Replace the next character (presumably 0x0A) with a 0 */
			if (*Script_ptr == 0x0A)
			{
				*Script_ptr++ = '\0';
			}
		}

		/* Count up */
		Nr_lines++;

		/* Maximum reached ? */
		if (Nr_lines == MAX_SCRIPT_LINES)
		{
			/* Yes -> Report error */
			Script_error(SCRIPTERR_TOO_MANY_LINES);

			/* Clear number of lines */
			Nr_lines = 0;

			break;
		}
	}

	MEM_Free_pointer(Script_line_data_handle);
	MEM_Free_pointer(Script_handle);

	if (Nr_lines)
	{
		/* Shrink script line data buffer */
		MEM_Resize_memory(Script_line_data_handle, Nr_lines *
		 sizeof(struct Script_line_data));
	}

	return Nr_lines;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Read_numeric_argument
 * FUNCTION  : Read a numeric argument from a script.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 11:38
 * LAST      : 11.08.95 13:19
 * INPUTS    : UNCHAR *Script_ptr - Pointer to script.
 *             SILONG *Value_ptr - Pointer to argument value.
 * RESULT    : UNCHAR * : Pointer to script after argument.
 * BUGS      : No known.
 * NOTES     : - This function will return 0 if no argument was found.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Read_numeric_argument(UNCHAR *Script_ptr, SILONG *Value_ptr)
{
	BOOLEAN Minus_sign_found = FALSE;
	SILONG Value = 0;
	UNSHORT Number_length;
	UNCHAR Number[21];
	UNCHAR Char;

	/* Skip leading spaces */
	Char = *Script_ptr;
	while (Char == ' ')
	{
		Script_ptr++;
		Char = *Script_ptr;
	}

	/* Is this character a minus sign ? */
	if (Char == '-')
	{
		/* Yes -> Indicate this */
		Minus_sign_found = TRUE;

		/* Read next character */
		Script_ptr++;
		Char = *Script_ptr;
	}

	/* Read number as string */
	Number_length = 0;
	while ((Char >= '0') && (Char <= '9') && (Number_length < 20))
	{
		/* Copy character */
		Number[Number_length] = Char;

		/* Count up */
		Number_length++;

		/* Read next character */
		Script_ptr++;
		Char = *Script_ptr;
	}

	/* Was any number found ? */
	if (Number_length)
	{
		/* Yes -> Insert EOL in number string */
		Number[Number_length] = 0;

		/* Convert number */
		Value = atoi(Number);
	}

	/* Was a minus sign found ? */
	if (Minus_sign_found)
	{
		/* Yes -> Negate */
		Value = 0 - Value;
	}

	/* Store value */
	*Value_ptr = Value;

	return Script_ptr;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_jump_script_command_handler
 * FUNCTION  : Party jump script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:57
 * LAST      : 12.08.95 10:57
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_jump_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG New_X;
	SILONG New_Y;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &New_X);
	Script_ptr = Read_numeric_argument(Script_ptr, &New_Y);

	/* Change position */
	Change_position((UNSHORT) New_X, (UNSHORT) New_Y,
	 PARTY_DATA.View_direction);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_move_script_command_handler
 * FUNCTION  : Party move script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:55
 * LAST      : 12.08.95 20:05
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_move_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG dX;
	SILONG dY;
	UNSHORT VD;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &dX);
	Script_ptr = Read_numeric_argument(Script_ptr, &dY);

	/* Set view direction */
	VD = View_directions[sgn(dY) + 1][sgn(dX) + 1];
	if (VD != 0xFFFF)
	{
		PARTY_DATA.View_direction = VD;
	}

	/* Move the party in this direction */
	Store_2D_movement((SISHORT) dX, (SISHORT) dY);

	/* Update map coordinates */
	PARTY_DATA.X += dX;
	PARTY_DATA.Y += dY;

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : NPC_jump_script_command_handler
 * FUNCTION  : NPC jump script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:59
 * LAST      : 12.08.95 10:59
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
NPC_jump_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;
	SILONG New_X;
	SILONG New_Y;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);
	Script_ptr = Read_numeric_argument(Script_ptr, &New_X);
	Script_ptr = Read_numeric_argument(Script_ptr, &New_Y);

	/* Set NPC position */
	Set_NPC_position(NPC_index, New_X, New_Y);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : NPC_move_script_command_handler
 * FUNCTION  : NPC move script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:59
 * LAST      : 12.08.95 23:17
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
NPC_move_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;
	SILONG dX;
	SILONG dY;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);
	Script_ptr = Read_numeric_argument(Script_ptr, &dX);
	Script_ptr = Read_numeric_argument(Script_ptr, &dY);

	/* Move the NPC */
	Move_locked_NPC((UNSHORT) NPC_index, (SISHORT) dX, (SISHORT) dY);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_map_script_command_handler
 * FUNCTION  : Show map script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 18:21
 * LAST      : 11.08.95 18:21
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Show_map_script_command_handler(UNCHAR *Script_ptr)
{
	/* Switch to map background */
	Script_background_type = MAP_2D_SCRIPT_BG;
	Set_screen_type(MAP_2D_SCREEN);

	/* Redraw */
	Update_display();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_event_chain_script_command_handler
 * FUNCTION  : Do event chain script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 13:53
 * LAST      : 12.08.95 13:53
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Do_event_chain_script_command_handler(UNCHAR *Script_ptr)
{
	struct Event_context *Context;
	SILONG Chain_nr;
	UNSHORT First_block_nr;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Chain_nr);

	/* Get first block number */
	First_block_nr = Get_event_first_block_nr((UNSHORT) Chain_nr);

	/* Make a new context */
	Context = Push_event_context();

	/* Initialize context */
	Context->Source 			= MAP_EVENT_SOURCE;
	Context->Set_handle 		= Map_handle;
	Context->Text_handle		= Map_text_handle;
	Context->Chain_nr 		= (UNSHORT) Chain_nr;
	Context->Chain_block_nr = First_block_nr;
	Context->Block_nr 		= First_block_nr;
	Context->Max_blocks 		= Nr_event_blocks;
	Context->Base 				= Event_data_offset;
	Context->Validator 		= Map_context_validator;

	Context->Source_data.Map_source.Map_nr 	= PARTY_DATA.Map_nr;
	Context->Source_data.Map_source.Trigger	= SCRIPT_TRIGGER;
	Context->Source_data.Map_source.Value 		= 0;
	Context->Source_data.Map_source.X 			= 0;
	Context->Source_data.Map_source.Y 			= 0;

	/* Execute it */
	Execute_event_chain();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_pic_script_command_handler
 * FUNCTION  : Show picture script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 18:21
 * LAST      : 12.08.95 15:12
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Show_pic_script_command_handler(UNCHAR *Script_ptr)
{
	MEM_HANDLE Handle;
	SILONG Picture_nr;
	SILONG X, Y;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &Picture_nr);
	Script_ptr = Read_numeric_argument(Script_ptr, &X);
	Script_ptr = Read_numeric_argument(Script_ptr, &Y);

	/* Load picture */
	Handle = Load_subfile(PICTURE, (UNSHORT) Picture_nr);
	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		return TRUE;
	}

	/* Display picture */
	Display_LBM(&Main_OPM, (UNSHORT) X, (UNSHORT) Y, Handle);

	/* Destroy picture */
	MEM_Free_memory(Handle);

	/* Switch to picture background */
	Script_background_type = PICTURE_SCRIPT_BG;
	Set_screen_type(NO_SCREEN);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sound_script_command_handler
 * FUNCTION  : Sound script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 12:53
 * LAST      : 11.08.95 12:53
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Sound_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Effect_nr;
	SILONG Priority;
	SILONG Volume;
	SILONG Variability;
	SILONG Frequency;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &Effect_nr);
	Script_ptr = Read_numeric_argument(Script_ptr, &Priority);
	Script_ptr = Read_numeric_argument(Script_ptr, &Volume);
	Script_ptr = Read_numeric_argument(Script_ptr, &Variability);
	Script_ptr = Read_numeric_argument(Script_ptr, &Frequency);

	/* Play sound effect */
	Play_sound_effect((UNSHORT) Effect_nr, (UNSHORT) Priority,
	 (UNSHORT) Volume, (UNSHORT) Variability, (UNSHORT) Frequency);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Song_script_command_handler
 * FUNCTION  : Song script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:22
 * LAST      : 11.08.95 13:22
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Song_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Song_nr;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Song_nr);

	/* Play song */
	Play_song((UNSHORT) Song_nr);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Ambient_script_command_handler
 * FUNCTION  : Ambient song script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 17:25
 * LAST      : 13.08.95 17:25
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Ambient_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Song_nr;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Song_nr);

	/* Play ambient song */
	Play_ambient_song((UNSHORT) Song_nr);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_quest_bit_script_command_handler
 * FUNCTION  : Set quest bit script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:23
 * LAST      : 11.08.95 13:23
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Set_quest_bit_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Bit_index;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Bit_index);

	/* Set quest bit */
	Write_bit_array(QUEST_BIT_ARRAY, (UNLONG) Bit_index, SET_BIT_ARRAY);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clear_quest_bit_script_command_handler
 * FUNCTION  : Clear quest bit script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:24
 * LAST      : 11.08.95 13:24
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Clear_quest_bit_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Bit_index;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Bit_index);

	/* Clear quest bit */
	Write_bit_array(QUEST_BIT_ARRAY, (UNLONG) Bit_index, CLEAR_BIT_ARRAY);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : NPC_on_script_command_handler
 * FUNCTION  : NPC on script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:24
 * LAST      : 11.08.95 13:24
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
NPC_on_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;
	UNLONG Bit_index;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);

	/* Calculate bit index */
	Bit_index = ((PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP) + NPC_index;

	/* Clear CD array bit */
	Write_bit_array(CD_BIT_ARRAY, Bit_index, CLEAR_BIT_ARRAY);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : NPC_off_script_command_handler
 * FUNCTION  : NPC off script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:26
 * LAST      : 11.08.95 13:26
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
NPC_off_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;
	UNLONG Bit_index;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);

	/* Calculate bit index */
	Bit_index = ((PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP) + NPC_index;

	/* Set CD array bit */
	Write_bit_array(CD_BIT_ARRAY, Bit_index, SET_BIT_ARRAY);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Lock_NPC_script_command_handler
 * FUNCTION  : Lock NPC script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:27
 * LAST      : 11.08.95 13:27
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Lock_NPC_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);

	/* Lock NPC */
	Lock_NPC((UNSHORT) NPC_index);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unlock_NPC_script_command_handler
 * FUNCTION  : Unlock NPC script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:27
 * LAST      : 11.08.95 13:27
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Unlock_NPC_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);

	/* Unlock NPC */
	Unlock_NPC((UNSHORT) NPC_index);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_icon_script_command_handler
 * FUNCTION  : Change icon script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 13:30
 * LAST      : 11.08.95 13:30
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Change_icon_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG X, Y;
	SILONG Type;
	SILONG Sub_type;
	SILONG Value;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &X);
	Script_ptr = Read_numeric_argument(Script_ptr, &Y);
	Script_ptr = Read_numeric_argument(Script_ptr, &Type);
	Script_ptr = Read_numeric_argument(Script_ptr, &Sub_type);
	Script_ptr = Read_numeric_argument(Script_ptr, &Value);

	/* Change icon */
	Do_change_icon((SISHORT) X, (SISHORT) Y, (UNSHORT) Type,
	 (UNSHORT) Sub_type, (UNSHORT) Value);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wipe_script_command_handler
 * FUNCTION  : Wipe script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 19:53
 * LAST      : 12.08.95 17:18
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Wipe_script_command_handler(UNCHAR *Script_ptr)
{
	/* Wipe */
	Wipe(WIPE_WIPE_TYPE);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_anim_script_command_handler
 * FUNCTION  : Play animation script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 16:48
 * LAST      : 13.08.95 18:43
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Play_anim_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Flic_nr;
	SILONG Playback_X;
	SILONG Playback_Y;
	SILONG Nr_repeats;
	SILONG Play_mode;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &Flic_nr);
	Script_ptr = Read_numeric_argument(Script_ptr, &Playback_X);
	Script_ptr = Read_numeric_argument(Script_ptr, &Playback_Y);
	Script_ptr = Read_numeric_argument(Script_ptr, &Nr_repeats);
	Script_ptr = Read_numeric_argument(Script_ptr, &Play_mode);

	/* Switch to flic background */
	Script_background_type = FLIC_SCRIPT_BG;

	/* Play animation */
	Play_animation((UNSHORT) Flic_nr, (SISHORT) Playback_X,
	 (SISHORT) Playback_Y, (UNSHORT) Nr_repeats, (UNSHORT) Play_mode);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Text_script_command_handler
 * FUNCTION  : Text script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 17:24
 * LAST      : 11.08.95 17:24
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Text_script_command_handler(UNCHAR *Script_ptr)
{
	struct Event_context *Context;
	MEM_HANDLE Text_handle;
	SILONG Block_nr;

	/* Set ink colour */
	Set_ink(SILVER_TEXT);

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get memory handle of text file */
	Text_handle = Context->Text_handle;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Block_nr);

	/* Do normal text window */
	Do_text_file_window(Text_handle, (UNSHORT) Block_nr);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_member_text_script_command_handler
 * FUNCTION  : Party member text script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 17:24
 * LAST      : 11.08.95 17:24
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_member_text_script_command_handler(UNCHAR *Script_ptr)
{
	struct Event_context *Context;
	MEM_HANDLE Text_handle;
	SILONG Char_nr;
	SILONG Block_nr;
	UNSHORT Member_index;

	/* Set ink colour */
	Set_ink(GOLD_TEXT);

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get memory handle of text file */
	Text_handle = Context->Text_handle;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &Char_nr);
	Script_ptr = Read_numeric_argument(Script_ptr, &Block_nr);

	/* Find character in party */
	Member_index = Find_character_in_party((UNSHORT) Char_nr);

	/* Found ? */
	if (Member_index != 0xFFFF)
	{
		/* Yes -> Is this character capable ? */
		if (Character_capable(Party_char_handles[Member_index - 1]))
		{
			/* Yes -> Do text window */
			Do_text_file_window_with_symbol
			(
				Text_handle,
				(UNSHORT) Block_nr,
				PORTRAIT_WIDTH,
				PORTRAIT_HEIGHT,
				Small_portrait_handles[Member_index - 1],
				0
			);
		}
	}

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : NPC_text_script_command_handler
 * FUNCTION  : NPC text  script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 17:26
 * LAST      : 11.08.95 17:26
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
NPC_text_script_command_handler(UNCHAR *Script_ptr)
{
	struct Event_context *Context;
	struct Character_data *Char;
	MEM_HANDLE Text_handle;
	MEM_HANDLE NPC_char_handle;
	MEM_HANDLE NPC_portrait_handle;
	SILONG Char_nr;
	SILONG Block_nr;

	/* Set ink colour */
	Set_ink(GOLD_TEXT);

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get memory handle of text file */
	Text_handle = Context->Text_handle;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &Char_nr);
	Script_ptr = Read_numeric_argument(Script_ptr, &Block_nr);

	/* Load NPC character data */
	NPC_char_handle = Load_subfile(NPC_CHAR, Char_nr);
	if (!NPC_char_handle)
	{
		Error(ERROR_FILE_LOAD);
		return TRUE;
	}

	/* Get pointer to character data */
	Char = (struct Character_data *) MEM_Claim_pointer(NPC_char_handle);

	/* Load portrait */
	NPC_portrait_handle = Load_subfile(SMALL_PORTRAIT,
	 (UNSHORT) Char->Portrait_nr);

	MEM_Free_pointer(NPC_char_handle);
	MEM_Free_memory(NPC_char_handle);

	if (!NPC_portrait_handle)
	{
		Error(ERROR_FILE_LOAD);
		return TRUE;
	}

	/* Do text window */
	Do_text_file_window_with_symbol
	(
		Text_handle,
		(UNSHORT) Block_nr,
		PORTRAIT_WIDTH,
		PORTRAIT_HEIGHT,
		NPC_portrait_handle,
		0
	);

	/* Destroy portrait */
	MEM_Free_memory(NPC_portrait_handle);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Active_member_text_script_command_handler
 * FUNCTION  : Active member text script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 17:27
 * LAST      : 11.08.95 17:27
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Active_member_text_script_command_handler(UNCHAR *Script_ptr)
{
	struct Event_context *Context;
	MEM_HANDLE Text_handle;
	SILONG Block_nr;

	/* Set ink colour */
	Set_ink(GOLD_TEXT);

	/* Get current event context */
	Context = &(Event_context_stack[Event_context_stack_index]);

	/* Get memory handle of text file */
	Text_handle = Context->Text_handle;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Block_nr);

	/* Is this character capable ? */
	if (Character_capable(Active_char_handle))
	{
		/* Yes -> Do text window */
		Do_text_file_window_with_symbol
		(
			Text_handle,
			(UNSHORT) Block_nr,
			PORTRAIT_WIDTH,
			PORTRAIT_HEIGHT,
			Small_portrait_handles[PARTY_DATA.Active_member - 1],
			0
		);
	}

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pause_script_command_handler
 * FUNCTION  : Pause script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 17:35
 * LAST      : 11.08.95 21:38
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Pause_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Duration;
	UNLONG T;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Duration);

	/* Wait for user ? */
	if (!Duration)
	{
		/* Yes */
		Wait_4_user();
	}
	else
	{
		/* No -> Get time */
		T = SYSTEM_GetTicks();

		/* Wait */
		while (SYSTEM_GetTicks() < T + Duration)
		{
			Update_display();
		}
	}

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fill_screen_script_command_handler
 * FUNCTION  : Fill screen script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 20:37
 * LAST      : 11.08.95 21:59
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Fill_screen_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Colour;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Colour);

	/* Clear the screen */
	OPM_FillBox(&Main_OPM, 0, 0, Screen_width, Panel_Y, (UNBYTE) Colour);

	/* Switch to no background */
	Script_background_type = NO_SCRIPT_BG;
	Set_screen_type(NO_SCREEN);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_pal_screen_script_command_handler
 * FUNCTION  : Restore pal screen script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 22:01
 * LAST      : 11.08.95 22:01
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Restore_pal_script_command_handler(UNCHAR *Script_ptr)
{
	/* Restore the palette */
	Restore_palette();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_anim_script_command_handler
 * FUNCTION  : Start animation script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:03
 * LAST      : 13.08.95 18:42
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Start_anim_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Flic_nr;
	SILONG Playback_X;
	SILONG Playback_Y;
	SILONG Play_mode;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &Flic_nr);
	Script_ptr = Read_numeric_argument(Script_ptr, &Playback_X);
	Script_ptr = Read_numeric_argument(Script_ptr, &Playback_Y);
	Script_ptr = Read_numeric_argument(Script_ptr, &Play_mode);

	/* Switch to flic background */
	Script_background_type = FLIC_SCRIPT_BG;

	/* Start animation */
	Start_animation((UNSHORT) Flic_nr, (SISHORT) Playback_X,
	 (SISHORT) Playback_Y, (UNSHORT) Play_mode);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_script_command_handler
 * FUNCTION  : Play frames of animation script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:03
 * LAST      : 12.08.95 10:03
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Play_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Nr_frames;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Nr_frames);

	/* Play animation frames */
	Play_animation_frames((UNSHORT) Nr_frames);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stop_anim_script_command_handler
 * FUNCTION  : Stop animation script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:04
 * LAST      : 12.08.95 10:04
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Stop_anim_script_command_handler(UNCHAR *Script_ptr)
{
	/* Stop animation (if any) */
	Stop_animation();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Load_pal_script_command_handler
 * FUNCTION  : Load palette script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:36
 * LAST      : 12.08.95 10:36
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Load_pal_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Palette_nr;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Palette_nr);

	/* Load palette */
	Load_palette((UNSHORT) Palette_nr);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Turn_party_script_command_handler
 * FUNCTION  : Turn party script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 17:40
 * LAST      : 12.08.95 17:40
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Turn_party_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG View_direction;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &View_direction);

	/* Set new view direction */
	PARTY_DATA.View_direction = (UNSHORT) View_direction;

	/* Store current position in path */
	Store_2D_current_position();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Turn_NPC_script_command_handler
 * FUNCTION  : Turn NPC script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 17:33
 * LAST      : 12.08.95 17:33
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Turn_NPC_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG NPC_index;
	SILONG View_direction;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &NPC_index);
	Script_ptr = Read_numeric_argument(Script_ptr, &View_direction);

	/* Set new view direction */
	VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction =
	 (UNSHORT) View_direction;

	/* Store current position in path */
	Store_2D_current_position();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Lock_camera_script_command_handler
 * FUNCTION  : Lock camera script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 12:33
 * LAST      : 12.08.95 12:33
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Lock_camera_script_command_handler(UNCHAR *Script_ptr)
{
	/* Lock the camera */
	Lock_2D_camera();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unlock_camera_script_command_handler
 * FUNCTION  : Unlock camera script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 12:34
 * LAST      : 12.08.95 12:34
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Unlock_camera_script_command_handler(UNCHAR *Script_ptr)
{
	/* Unlock the camera */
	Unlock_2D_camera();

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Camera_jump_script_command_handler
 * FUNCTION  : Camera jump script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 12:35
 * LAST      : 12.08.95 12:35
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Camera_jump_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG New_X;
	SILONG New_Y;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &New_X);
	Script_ptr = Read_numeric_argument(Script_ptr, &New_Y);

	/* Set camera position */
	Set_2D_camera_position((UNLONG) New_X, (UNLONG) New_Y);
	Set_2D_camera_target_position((UNLONG) New_X, (UNLONG) New_Y);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Camera_move_script_command_handler
 * FUNCTION  : Camera move script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 12:37
 * LAST      : 13.08.95 11:41
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Camera_move_script_command_handler(UNCHAR *Script_ptr)
{
	UNLONG Camera_X;
	UNLONG Camera_Y;
	SILONG dX;
	SILONG dY;

	/* Read arguments */
	Script_ptr = Read_numeric_argument(Script_ptr, &dX);
	Script_ptr = Read_numeric_argument(Script_ptr, &dY);

	/* Get current camera coordinates */
	Get_2D_camera_position(&Camera_X, &Camera_Y);

	/* Calculate new target coordinates */
	Camera_X += dX;
	Camera_Y += dY;

	/* Set camera position */
	Set_2D_camera_position(Camera_X, Camera_Y);
	Set_2D_camera_target_position(Camera_X, Camera_Y);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_script_command_handler
 * FUNCTION  : Update script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 13:39
 * LAST      : 12.08.95 13:39
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Update_script_command_handler(UNCHAR *Script_ptr)
{
	SILONG Counter;
	UNSHORT i;

	/* Read argument */
	Script_ptr = Read_numeric_argument(Script_ptr, &Counter);

	/* Update */
	for (i=0;i<Counter;i++)
	{
		Update_input();
		Main_loop();
		Switch_screens();
	}

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_to_black_script_command_handler
 * FUNCTION  : Fade to black script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 17:18
 * LAST      : 12.08.95 17:18
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Fade_to_black_script_command_handler(UNCHAR *Script_ptr)
{
	/* Wipe */
	Wipe(FADE_TO_BLACK_WIPE_TYPE);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_to_white_script_command_handler
 * FUNCTION  : Fade to white script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 17:18
 * LAST      : 12.08.95 17:18
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Fade_to_white_script_command_handler(UNCHAR *Script_ptr)
{
	/* Wipe */
	Wipe(FADE_TO_WHITE_WIPE_TYPE);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_from_black_script_command_handler
 * FUNCTION  : Fade from black script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 17:18
 * LAST      : 12.08.95 17:18
 * INPUTS    : UNCHAR *Script_ptr - Pointer from current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Fade_from_black_script_command_handler(UNCHAR *Script_ptr)
{
	/* Wipe */
	Wipe(FADE_FROM_BLACK_WIPE_TYPE);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Fade_from_white_script_command_handler
 * FUNCTION  : Fade from white script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 17:18
 * LAST      : 12.08.95 17:18
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Fade_from_white_script_command_handler(UNCHAR *Script_ptr)
{
	/* Wipe */
	Wipe(FADE_FROM_WHITE_WIPE_TYPE);

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_on_script_command_handler
 * FUNCTION  : Party on script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 14:22
 * LAST      : 13.08.95 14:22
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_on_script_command_handler(UNCHAR *Script_ptr)
{
	/* Show party */
	Hide_party = FALSE;

	return FALSE;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Party_off_script_command_handler
 * FUNCTION  : Party off script command handler.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.08.95 14:22
 * LAST      : 13.08.95 14:22
 * INPUTS    : UNCHAR *Script_ptr - Pointer to current line in script, after
 *              command.
 * RESULT    : BOOLEAN : End script flag.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Party_off_script_command_handler(UNCHAR *Script_ptr)
{
	/* Hide party */
	Hide_party = TRUE;

	return FALSE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Script_error
 * FUNCTION  : Report a script error.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 12:56
 * LAST      : 13.08.95 18:52
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Script_error(UNSHORT Error_code)
{
	struct Script_error_data Error_data;

	/* Initialize error structure */
	Error_data.Code			= Error_code;
	Error_data.Line_nr		= Current_script_line_nr;
	Error_data.Script_nr	= Current_script_nr;

	/* Push error on the error stack */
	ERROR_PushError(Script_print_error, Script_library_name,
	 sizeof(struct Script_error_data), (UNBYTE *) &Error_data);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Script_print_error
 * FUNCTION  : Print a script error.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 12:56
 * LAST      : 13.08.95 18:52
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by Script_error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Script_print_error(UNCHAR *buffer, UNBYTE *data)
{
	struct Script_error_data *Error_data;
	UNSHORT Error_code;

	Error_data = (struct Script_error_data *) data;

	/* Get error code */
	Error_code = Error_data->Code;

	/* Catch illegal errors */
	if (Error_code > SCRIPTERR_MAX)
		Error_code = 0;

 	/* Print error */
	sprintf((char *)buffer, Script_error_strings[Error_code],
	 Error_data->Line_nr, Error_data->Script_nr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_animation
 * FUNCTION  : Play an animation.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 14:08
 * LAST      : 11.08.95 16:31
 * INPUTS    : UNSHORT Flic_nr - Flic number (1...).
 *             SISHORT Playback_X - Screen X-coordinate.
 *             SISHORT Playback_Y - Screen Y-coordinate.
 *             UNSHORT Nr_repetitions - Number of repetitions.
 *             UNSHORT Play_mode - Animation play mode.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The last frame will be left on the screen to ensure smooth
 *              transitions.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_animation(UNSHORT Flic_nr, SISHORT Playback_X, SISHORT Playback_Y,
 UNSHORT Nr_repetitions, UNSHORT Play_mode)
{
	UNSHORT Nr_frames;

	/* Any repetitions ? */
	if (Nr_repetitions)
	{
		/* Yes -> Start animation */
		Nr_frames = Start_animation(Flic_nr, Playback_X, Playback_Y,
		 Play_mode);

		/* Did an error occur ? */
		if (Nr_frames != 0xFFFF)
		{
			/* No -> Play all frames repeatedly */
			Play_animation_frames(Nr_frames * Nr_repetitions);

			/* Stop animation */
			Stop_animation();
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_animation
 * FUNCTION  : Load and intialize an animation.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:05
 * LAST      : 15.09.95 13:53
 * INPUTS    : UNSHORT Flic_nr - Flic number (1...).
 *             SISHORT Playback_X - Screen X-coordinate.
 *             SISHORT Playback_Y - Screen Y-coordinate.
 *             UNSHORT Play_mode - Animation play mode.
 * RESULT    : UNSHORT : Number of frames / 0xFFFF (error).
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Start_animation(UNSHORT Flic_nr, SISHORT Playback_X, SISHORT Playback_Y,
 UNSHORT Play_mode)
{
	BOOLEAN Result;
	UNLONG File_base_offset;
	UNLONG Flic_length;
	SISHORT File_handle;
	UNBYTE *Ptr;

	/* Already playing an animation ? */
	if (ANIM_Playing)
	{
		/* Yes -> Stop it */
		Stop_animation();
	}

	/* Clear FLC playback data */
	BASEMEM_FillMemByte((UNBYTE *) &ANIM_FLC, sizeof(struct Flic_playback), 0);

	/* Get length of flic */
	Flic_length = Get_subfile_length
	(
		FLIC,
		Flic_nr
	);

	if (Flic_length == 0xFFFFFFFF)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return 0xFFFF;
	}

	/* Should it be streamed ? */
	if (Flic_length > 100000)
	{
		/* Yes -> Open flic */
		Result = Open_subfile
		(
			FLIC,
			Flic_nr,
			&File_handle,
			&File_base_offset
		);

		if (!Result)
		{
			Error(ERROR_FILE_LOAD);
			Exit_program();
			return 0xFFFF;
		}

		/* Allocate load buffer */
		ANIM_Flic_handle = MEM_Allocate_memory
		(
			((Screen_width * Screen_height) * 5) / 4
		);

		/* Set playback data */
		ANIM_FLC.Input_buffer_length	= ((Screen_width * Screen_height) * 5) / 4;
		ANIM_FLC.Input_file_handle		= File_handle;
		ANIM_FLC.File_base_offset		= File_base_offset;

		ANIM_FLC.Flags |= FLC_STREAM;
	}
	else
	{
		/* No -> Load flic */
		ANIM_Flic_handle = Load_subfile
		(
			FLIC,
			Flic_nr
		);

		if (!ANIM_Flic_handle)
		{
			Error(ERROR_FILE_LOAD);
			Exit_program();
			return 0xFFFF;
		}
	}

	/* Store playback parameters */
	ANIM_Play_mode = Play_mode & ~NO_PAL_PLAY_MODE_FLAG;

	/* Playing back */
	ANIM_Playing = TRUE;

	Ptr = MEM_Claim_pointer(ANIM_Flic_handle);

	/* Prepare playback */
	ANIM_FLC.Output_frame			= &Main_OPM;
	ANIM_FLC.Playback_X				= Playback_X;
	ANIM_FLC.Playback_Y				= Playback_Y;
	ANIM_FLC.Input_buffer			= Ptr;
	ANIM_FLC.Black_colour_index	= BLACK;
	ANIM_FLC.Trans_colour_index	= 0;

	/* May the FLC player change the palette ? */
	if (Play_mode & NO_PAL_PLAY_MODE_FLAG)
	{
		/* No */
		ANIM_FLC.Palette				= NULL;
		ANIM_FLC.First_colour		= 0;
		ANIM_FLC.Max_colours			= 0;
	}
	else
	{
		/* Yes */
		ANIM_FLC.Palette				= &Palette;
		ANIM_FLC.First_colour		= 0;
		ANIM_FLC.Max_colours			= 192;
	}

	/* Start playback */
	FLC_Start_flic_playback(&ANIM_FLC);

	/* Calculate delta time */
	ANIM_dT = max(1, (ANIM_FLC.Flic.speed * 6) / 100);

	/* Play outside of the map ? */
	if (ANIM_Play_mode == OUTSIDE_MAP_PLAY_MODE)
	{
		/* Yes -> Build playback clipping rectangle */
		ANIM_New_clip.left	= max(Playback_X, 0);
		ANIM_New_clip.top		= max(Playback_Y, 0);
		ANIM_New_clip.width	= min(ANIM_FLC.Width, Screen_width - Playback_X);
		ANIM_New_clip.height	= min(ANIM_FLC.Height, Screen_height - Playback_Y);
	}
	else
	{
		/* No -> 2D or 3D map ? */
		if (_3D_map)
		{
			/* 3D map -> Relative to 3D map window */
			Playback_X += Window_3D_X;
			Playback_Y += Window_3D_Y;

			/* Build playback clipping rectangle */
			ANIM_New_clip.left	= max(Playback_X, Window_3D_X);
			ANIM_New_clip.top		= max(Playback_Y, Window_3D_Y);
			ANIM_New_clip.width	= min(ANIM_FLC.Width, Window_3D_X +
										 I3DM.Window_3D_width - Playback_X);
			ANIM_New_clip.height	= min(ANIM_FLC.Height, Window_3D_Y +
										 I3DM.Window_3D_height - Playback_Y);
		}
		else
		{
			/* 2D map -> Relative to 3D map window */
			Playback_X += MAP_2D_X;
			Playback_Y += MAP_2D_Y;

			/* Build playback clipping rectangle */
			ANIM_New_clip.left	= max(Playback_X, MAP_2D_X);
			ANIM_New_clip.top		= max(Playback_Y, MAP_2D_Y);
			ANIM_New_clip.width	= min(ANIM_FLC.Width, MAP_2D_X + Map_2D_width -
										 Playback_X);
			ANIM_New_clip.height	= min(ANIM_FLC.Height, MAP_2D_Y + Map_2D_height -
										 Playback_Y);
		}
	}

	/* Current screen is 2D map ? */
	if (Current_screen_type(0) == MAP_2D_SCREEN)
	{
		/* Yes -> Draw without 2D objects ? */
		if (ANIM_Play_mode == NO_2D_OBJECTS_PLAY_MODE)
		{
			/* Yes -> Reset 2D object list */
			Clear_2D_object_list();

			/* Draw 2D map */
			Draw_2D_map();
		}

		/* Prepare screen */
		Current_2D_OPM = &Main_OPM;
		Draw_2D_scroll_buffer();
		Current_2D_OPM = NULL;
	}

	/* Hide HDOBs */
	Hide_HDOBs();

	return ANIM_FLC.Nr_frames;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Play_animation_frames
 * FUNCTION  : Play some frames of an animation.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:05
 * LAST      : 12.08.95 16:46
 * INPUTS    : UNSHORT Nr_frames - Number of frames to be played.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The last frame will be left on the screen to ensure smooth
 *              transitions.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Play_animation_frames(UNSHORT Nr_frames)
{
	struct BBRECT ANIM_Old_clip;
	UNLONG T;
	UNSHORT i;

	/* Exit if not playing an animation */
	if (!ANIM_Playing)
		return;

	/* Install clip area */
	memcpy(&ANIM_Old_clip, &(Main_OPM.clip), sizeof(struct BBRECT));
	memcpy(&(Main_OPM.clip), &ANIM_New_clip, sizeof(struct BBRECT));

	/* Play each frame */
	for (i=0;i<Nr_frames;i++)
	{
		/* Get time */
		T = SYSTEM_GetTicks();

		/* Decode frame */
		FLC_Playback_flic_frame();

		/* Current screen is 2D map / play under 2D objects ? */
		if ((Current_screen_type(0) == MAP_2D_SCREEN) &&
		 (ANIM_Play_mode == UNDER_2D_OBJECTS_PLAY_MODE))
		{
			/* Yes -> Redraw objects */
			Redraw_2D_objects();
		}

		/* Has the palette been changed ? */
		if (ANIM_FLC.Palette_was_changed)
		{
			/* Yes -> Update palette */
			Update_palette(0, 192);
		}

		/* Show frame */
		Switch_screens();

		/* Wait */
		while (SYSTEM_GetTicks() < (UNLONG)(T + ANIM_dT))
		{
		};
	}

	/* Restore clip area */
	memcpy(&(Main_OPM.clip), &ANIM_Old_clip, sizeof(struct BBRECT));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Stop_animation
 * FUNCTION  : Stop playing an animation.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 10:06
 * LAST      : 13.09.95 16:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Stop_animation(void)
{
	/* Playing an animation ? */
	if (ANIM_Playing)
	{
		/* Yes -> Stop playback */
		FLC_Stop_flic_playback();

		/* Show HDOBs */
		Show_HDOBs();

		/* Were we streaming ? */
		if (ANIM_FLC.Flags & FLC_STREAM)
		{
			/* Yes -> Close flic */
			Close_subfile(ANIM_FLC.Input_file_handle);
		}

		/* Remove flic (or input buffer when streaming) */
		MEM_Free_pointer(ANIM_Flic_handle);
		MEM_Free_memory(ANIM_Flic_handle);

		/* Clear flag */
		ANIM_Playing = FALSE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wipe
 * FUNCTION  : Wipe the screen.
 * FILE      : SCRIPT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 19:54
 * LAST      : 11.08.95 19:54
 * INPUTS    : UNSHORT Wipe_type - Wipe type index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Only the top of the screen will be wiped.
 * SEE ALSO  : FLC.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wipe(UNSHORT Wipe_type)
{
	/* Act depending on wipe type */
	switch (Wipe_type)
	{
		/* Plain switch */
		case PLAIN_WIPE_TYPE:
		{
			/* Update the screen */
		   Update_screen();
			break;
		}
		/* Wipe */
		case WIPE_WIPE_TYPE:
		{
			Wipe_out(0, 0, Screen_width, Screen_height);
			break;
		}
		/* Fade to black */
		case FADE_TO_BLACK_WIPE_TYPE:
		{
			Fade_to_black();
			break;
		}
		/* Fade to white */
		case FADE_TO_WHITE_WIPE_TYPE:
		{
			Fade_to_white();
			break;
		}
		/* Fade from black */
		case FADE_FROM_BLACK_WIPE_TYPE:
		{
			Fade_from_black();
			break;
		}
		/* Fade from white */
		case FADE_FROM_WHITE_WIPE_TYPE:
		{
			Fade_from_white();
			break;
		}
	}
}

