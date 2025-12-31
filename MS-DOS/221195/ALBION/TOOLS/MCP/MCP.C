/************
 * NAME     : MCP.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 13-11-1995
 * PROJECT  : Show memory
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <process.h>
#include <dos.h>
#include <sys\types.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <env.h>
#include <malloc.h>
#include <ctype.h>

#include <MCP.H>

/*
 ** Defines ****************************************************************
 */

/* Amount of disk space needed for using VMM */
#define MINIMUM_VMM_MEMORY		(16L * 1024L * 1024L)

/* Number of commands recognised by program */
#define NR_MCP_COMMANDS			(3)

#define MAX_DATA_PATHS			(2)
#define MAX_DATA_PATH_LENGTH	(100)

/* Program message indices */
#define WRONG_DIR_MESSAGE						(0)
#define NO_CD_MESSAGE							(1)
#define GOODBYE_MESSAGE							(2)
#define NO_ENV_MEM_MESSAGE						(3)
#define NOT_ENOUGH_DISK_SPACE_MESSAGE		(4)

#define MAX_MESSAGES								(5)

/* Supported languages */
#define GERMAN					(0)
#define ENGLISH				(1)
#define FRENCH					(2)
#define MAX_LANGUAGES		(3)

/* BBDEF replacements */
#undef NULL
#define NULL	(0L)

#undef TRUE
#define TRUE	(1)

#undef FALSE
#define FALSE	(0)

/*
 ** Type definitions *******************************************************
 */

/* BBDEF replacements */
/* 8 bit integers */
typedef unsigned char		UNCHAR;
typedef signed char			SIBYTE;
typedef unsigned char		UNBYTE;

/* 16 bit integers */
typedef signed short	int	SISHORT;
typedef unsigned short int	UNSHORT;

/* 32 bit integers */
typedef signed long int		SILONG;
typedef unsigned long int 	UNLONG;

/* Boolean (TRUE or FALSE) */
typedef SISHORT BOOLEAN;

/*
 ** Structure definitions **************************************************
 */

/*
 ** Prototypes *************************************************************
 */

void main(int argc, char** argv);

UNSHORT Start_game(void);
void Show_video(UNCHAR *Video_filename);

int __far OS_critical_error_handler(unsigned deverr, unsigned errcode,
 unsigned __far *devhdr);

/* Command-line parameter functions */
void Evaluate_program_arguments(UNSHORT argc, UNCHAR **argv);

UNSHORT Parse_argument(UNCHAR *Argument, UNSHORT Nr_commands, UNCHAR **Command_list);
UNSHORT Parse_value(UNCHAR *Argument);

/* Datapath management functions */
BOOLEAN Init_data_paths(void);
void Add_data_path(UNCHAR *Data_path_ptr);
void Select_data_path(UNSHORT Data_path_index);
BOOLEAN Search_second_data_path(void);
BOOLEAN Check_CD(void);

/* INI file access functions */
BOOLEAN Read_INI_variable(UNCHAR *Section_name, UNCHAR *Variable_name,
 UNCHAR *Value_buffer, UNSHORT Max_variable_length);

BOOLEAN Write_INI_variable(UNCHAR *Section_name, UNCHAR *Variable_name,
 UNCHAR *Value_buffer);

/* I/O support functions */
BOOLEAN File_exists(UNCHAR *Filename);
BOOLEAN Directory_exists(UNCHAR *Directory_name);
void Change_directory(UNCHAR *Pathname);

/*
 ** Global variables *******************************************************
 */

static UNSHORT Language = GERMAN;

/* Program control flags */
BOOLEAN No_sound_flag = FALSE;
BOOLEAN No_video_flag = FALSE;
BOOLEAN Use_VMM_flag = FALSE;

/* No sound in videos flag */
static BOOLEAN Quiet_video_flag = FALSE;

/* Data path data */
static UNCHAR Data_paths[MAX_DATA_PATHS][MAX_DATA_PATH_LENGTH];
static UNSHORT Nr_data_paths = 0;

/* INI file data */
static UNCHAR INI_filename[] = "SETUP.INI";

static UNCHAR Evil_characters[] = " \r\n\t";

/* MCP commands */
static UNCHAR *MCP_commands[NR_MCP_COMMANDS] = {
	"nosound",
	"novideo",
	"vmm",
};

/* Program messages */
static UNCHAR *Program_messages[MAX_LANGUAGES][MAX_MESSAGES] = {
	/* German */
	{
		/* Albion wasn't started from the Albion directory */
		"Bitte starten Sie ALBION aus dem ALBION-Verzeichnis heraus.",

		/* The Albion CD could not be found in the CD-ROM drive */
		"Bitte legen Sie die ALBION-CD in Ihr CD-Laufwerk.",

		/* Good-bye message */
		"Danke, daá Sie ALBION gespielt haben.",

		/* Not enough memory for VMM environment string */
		"Nicht genug Speicher fr Umgebungsvariabeln.\n"
		"Virtueller Speicher konnte nicht aktiviert werden.",

		/* Not enough disk space to use VMM */
		"Nicht genug freier Platz auf Laufwerk %c um virtuellen Speicher\n"
		"einzurichten. %lu Bytes werden noch ben”tigt."
	},
	/* English */
	{
		/* Albion wasn't started from the Albion directory */
		"Please start ALBION from the ALBION directory.",

		/* The Albion CD could not be found in the CD-ROM drive */
		"Please insert the Albion CD in your CD-ROM drive.",

		/* Good-bye message */
		"Thank you for playing Albion.",

		/* Not enough memory for VMM environment string */
		"Not enough memory for environment string.\n"
		"Virtual memory could not be activated.",

		/* Not enough disk space to use VMM */
		"There is not enough room on drive %c to use virtual memory.\n"
		"%lu more bytes are needed."
	},
	/* French */
	{
		/* Albion wasn't started from the Albion directory */
		"Please start ALBION from the ALBION directory.",

		/* The Albion CD could not be found in the CD-ROM drive */
		"Please insert the Albion CD in your CD-ROM drive.",

		/* Good-bye message */
		"Thank you for playing Albion.",

		/* Not enough memory for VMM environment string */
		"Not enough memory for environment string.\n"
		"Virtual memory could not be activated.",

		/* Not enough disk space to use VMM */
		"There is not enough room on drive %c to use virtual memory.\n"
		"%lu more bytes are needed."
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of MCP
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.11.95 13:27
 * LAST      : 14.11.95 16:16
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, char** argv)
{
	struct _diskfree_t Disk_space_info;
	BOOLEAN Result;
	UNLONG Free_space;
	UNSHORT Command;
	unsigned Current_drive_nr;
	int Return_code;

	/* Print fake DOS/4GW banner */
	printf("DOS/4GW Protected Mode Run-time  Version 1.97\n");
	printf("Copyright (c) Rational Systems, Inc. 1990-1994\n");

	/* Evaluate arguments */
	Evaluate_program_arguments(argc, (UNCHAR **) argv);

	/* Make the MCP heap as small as possible */
	_heapshrink();

	/* Install OS critical error handler */
 	_harderr(OS_critical_error_handler);

	/* Initialize data paths */
	Result = Init_data_paths();

	/* Successful ? */
	if (!Result)
	{
		/* No -> Insult user */
		printf(Program_messages[Language][WRONG_DIR_MESSAGE]);

		/* Exit */
		goto EXIT_PROGRAM;
	}

	/* Search for a second data path */
	Result = Search_second_data_path();

	/* Successful ? */
	if (!Result)
	{
		/* No -> Insult user */
		printf(Program_messages[Language][NO_CD_MESSAGE]);

		/* Exit */
		goto EXIT_PROGRAM;
	}

	/* Check if the second data path is the Albion CD */
	Result = Check_CD();

	/* Successful ? */
	if (!Result)
	{
		/* No -> Insult user */
		printf(Program_messages[Language][NO_CD_MESSAGE]);

		/* Exit */
		goto EXIT_PROGRAM;
	}

	/* Use VMM ? */
	if (Use_VMM_flag)
	{
		/* Yes -> Select first data path */
		Select_data_path(0);

		/* Get current drive */
		_dos_getdrive(&Current_drive_nr);

		/* Get disk space info */
		Return_code = _dos_getdiskfree(Current_drive_nr, &Disk_space_info);

		/* Success ? */
		if (!Return_code)
		{
			/* Yes -> Calculate amount of free space */
			Free_space = (UNLONG) Disk_space_info.avail_clusters *
			 (UNLONG) Disk_space_info.sectors_per_cluster *
			 (UNLONG) Disk_space_info.bytes_per_sector;

			/* Is enough for VMM ? */
			if (Free_space < MINIMUM_VMM_MEMORY)
			{
				/* No -> Insult user */
				printf
				(
					Program_messages[Language][NOT_ENOUGH_DISK_SPACE_MESSAGE],
					((int) Current_drive_nr + 'A' - 1),
					(MINIMUM_VMM_MEMORY - Free_space)
				);

				/* Exit */
				goto EXIT_PROGRAM;
			}
		}
	}

	/* Has the user selected no digital sound / no sound at all ? */
	Select_data_path(0);
	if (!File_exists("DRIVERS\\DIG.INI") || No_sound_flag)
	{
		/* Yes -> Make all videos quiet */
		Quiet_video_flag = TRUE;
	}

	/* Give initial command */
	Command = SHOW_INTRO_MCP;
	do
	{
		/* Act depending on command */
		switch (Command)
		{
			/* Start the program */
			case START_PROGRAM_MCP:
			{
				/* Start the game */
				Command = Start_game();
				break;
			}
			/* Show the intro sequence */
			case SHOW_INTRO_MCP:
			{
				/* Show */
				Show_video("INTRO.SMK");

				/* Give start program command */
				Command = START_PROGRAM_MCP;
				break;
			}
			/* Show the credits sequence */
			case SHOW_CREDITS_MCP:
			{
				/* Show */
				Show_video("CREDITS.SMK");

				/* Give start program command */
				Command = START_PROGRAM_MCP;
				break;
			}
			/* All others */
			default:
			{
				/* Quit */
				Command = EXIT_PROGRAM_MCP;
				break;
			}
		}
	}
	/* Until exit program command is given */
	while (Command != EXIT_PROGRAM_MCP);

	/* Bye, bye */
	printf(Program_messages[Language][GOODBYE_MESSAGE]);

EXIT_PROGRAM:

	/* Select first data path */
	Select_data_path(0);

	printf("\n");
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Start_game
 * FUNCTION  : Start_the_game
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 18:33
 * LAST      : 14.11.95 16:16
 * INPUTS    : None.
 * RESULT    : UNSHORT : Return value (= new command).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Start_game(void)
{
	static char *No_sound_arg = "NOSOUND";
	static char *No_video_arg = "NOVIDEO";
	static char *VMM_env = "DOS4GVM=@ALBION.VMC";

	static char *Argument_list[] = {
		"MAIN.EXE",
		NULL,
		NULL,
		NULL
	};
	static char *Environment_list[] = {
		"DOS4G=QUIET",
		NULL,
		NULL
	};

	UNSHORT Command;
	UNSHORT Arg_index;
	UNSHORT Env_index;
	int Set_env_return_code;
	int Program_return_code;

	/* Default command is exit program
	  (in case an error occurs) */
	Command = EXIT_PROGRAM_MCP;

	/* Build argument list */
	Arg_index = 1;

	/* Sound off ? */
	if (No_sound_flag)
	{
		/* Yes -> Add no sound argument */
		Argument_list[Arg_index] = No_sound_arg;

		/* Count up */
		Arg_index++;
	}

	/* Video off ? */
	if (No_video_flag)
	{
		/* Yes -> Add no video argument */
		Argument_list[Arg_index] = No_video_arg;

		/* Count up */
		Arg_index++;
	}

	/* End argument list */
	Argument_list[Arg_index] = NULL;

	/* Tell DOS/4GW to be quiet
	  (don't worry if this fails) */
	setenv
	(
		"DOS4G",
		"QUIET",
		1
	);

	/* Use VMM ? */
	if (Use_VMM_flag)
	{
		/* Yes -> Tell DOS/4GW to use VMM */
		Set_env_return_code = setenv
		(
			"DOS4GVM",
			"@ALBION.VMC",
			1
		);

		/* Successful ? */
		if (Set_env_return_code)
		{
			/* No -> What went wrong ?
			  Act depending on error code */
			switch (errno)
			{
				/* Not enough memory */
				case ENOMEM:
				{
					/* Insult the user */
					printf(Program_messages[Language][NO_ENV_MEM_MESSAGE]);
					break;
				}
				/* Something else (unlikely) */
				default:
				{
					break;
				}
			}

			/* Exit */
			return EXIT_PROGRAM_MCP;
		}
	}

	/* Select first data path */
	Select_data_path(0);

	/* Execute program */
	Program_return_code = spawnv
	(
		P_WAIT,
		"MAIN.EXE",
		Argument_list
	);

	/* Used VMM ? */
	if (Use_VMM_flag)
	{
		/* Yes -> Tell DOS/4GW to stop using VMM */
		Set_env_return_code = setenv
		(
			"DOS4GVM",
			NULL,
			1
		);
	}

	/* Tell DOS/4GW to be quiet */
	Set_env_return_code = setenv
	(
		"DOS4G",
		NULL,
		1
	);

	/* Success ? */
	if (Program_return_code != -1)
	{
		/* Yes -> Use return code as command */
		Command = (UNSHORT) Program_return_code;
	}

	return Command;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_video
 * FUNCTION  : Show a video.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 18:33
 * LAST      : 08.11.95 12:20
 * INPUTS    : UNCHAR *Video_filename - Filename of video.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_video(UNCHAR *Video_filename)
{
	/* Exit if no videos should be shown */
	if (No_video_flag)
		return;

	/* Select second data path */
	Select_data_path(1);

	/* Switch to video directory */
	Change_directory("VIDEO");

	/* Do the video player and the video exist ? */
	if (File_exists("SMACKPLY.EXE") &&
	 File_exists(Video_filename))
	{
		/* Yes -> Should the videos be quiet ? */
		if (Quiet_video_flag)
		{
			/* Yes -> Spawn video player */
			spawnl
			(
				P_WAIT,
				"SMACKPLY.EXE",
				"SMACKPLY",
				Video_filename,
				"/U3",
				"/T0",
				NULL
			);
		}
		else
		{
			/* No -> Spawn video player */
			spawnl
			(
				P_WAIT,
				"SMACKPLY.EXE",
				"SMACKPLY",
				Video_filename,
				"/U3",
				NULL
			);
		}
	}

	/* Switch back to normal directory */
	Change_directory("..");
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : OS_critical_error_handler
 * FUNCTION  : Handle a critical OS error.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.11.95 13:16
 * LAST      : 03.11.95 13:57
 * INPUTS    : unsigned deverr.
 *             unsigned errcode.
 *             unsigned __far *devhdr.
 * RESULT    : int : Reaction to error.
 * BUGS      : No known.
 * NOTES     : - See page 288 of the Watcom C library reference.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

int __far
OS_critical_error_handler(unsigned deverr, unsigned errcode,
 unsigned __far *devhdr)
{
//	cprintf("Critical error: deverr = %4.4x errcode = %d\r\n", deverr, errcode);

	/* Ignore the error and let the caller handle it */
	return _HARDERR_IGNORE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Evaluate_program_arguments
 * FUNCTION  : Evaluate program arguments.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 10:39
 * LAST      : 08.11.95 12:17
 * INPUTS    : UNSHORT argc - Number of arguments.
 *             UNCHAR **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Evaluate_program_arguments(UNSHORT argc, UNCHAR **argv)
{
	UNSHORT Argument_index;
	UNSHORT Command_index;

	/* Evaluate all arguments */
	for (Argument_index=1;Argument_index<argc;Argument_index++)
	{
		/* Parse current argument */
		Command_index = Parse_argument
		(
			argv[Argument_index],
			NR_MCP_COMMANDS,
			MCP_commands
		);

		/* Act depending on command type */
		switch (Command_index)
		{
			/* Sound system will not be installed */
			case 0:
			{
				No_sound_flag = TRUE;
				break;
			}
			/* No videos will be shown */
			case 1:
			{
				No_video_flag = TRUE;
				break;
			}
			/* Main program will use VMM */
			case 2:
			{
				Use_VMM_flag = TRUE;
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Parse_argument
 * FUNCTION  : Parse an argument.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 20:11
 * LAST      : 11.08.95 21:21
 * INPUTS    : UNCHAR *Argument - Pointer to argument.
 *             UNSHORT Nr_commands - Number of commands in list.
 *             UNCHAR **Command_list - Pointer to list of commands.
 * RESULT    : UNSHORT : Command index (0...Nr_commands - 1) /
 *              0xFFFF (not parsed).
 * BUGS      : No known.
 * NOTES     : - The comparison is case-insensitive.
 *             - The argument need not be null-terminated.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Parse_argument(UNCHAR *Argument, UNSHORT Nr_commands, UNCHAR **Command_list)
{
	UNSHORT Command_index = 0xFFFF;
	UNSHORT i;

	/* Find argument in argument list */
	for (i=0;i<Nr_commands;i++)
	{
		/* Is this the one ? */
		if (!strnicmp(Argument, Command_list[i], strlen(Command_list[i])))
		{
			/* Yes -> Found it */
			Command_index = i;

			break;
		}
	}

	return Command_index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Parse_value
 * FUNCTION  : Parse a value.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.05.95 20:15
 * LAST      : 10.05.95 20:15
 * INPUTS    : UNCHAR *Argument - Pointer to argument.
 * RESULT    : UNSHORT : Value / 0 (not parsed).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Parse_value(UNCHAR *Argument)
{
	return atoi(Argument);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_data_paths
 * FUNCTION  : Initialize data paths.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 09:50
 * LAST      : 25.10.95 22:53
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function will install the current directory as the
 *              first data path.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_data_paths(void)
{
	static UNCHAR Program_name[] = "ALBION.EXE";
	char *cwd;

	/* No data paths */
	Nr_data_paths = 0;

	/* Get current working directory */
	cwd = getcwd(&(Data_paths[0][0]), MAX_DATA_PATH_LENGTH);

	/* Success ? */
	if (cwd != NULL)
	{
		/* Yes -> Count up */
		Nr_data_paths++;
	}

	/* Select first data path */
	Select_data_path(0);

	/* Can the program be found here ? */
	if (File_exists(Program_name))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_data_path
 * FUNCTION  : Add a new data path.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 09:53
 * LAST      : 28.07.95 09:53
 * INPUTS    : UNCHAR *Data_path_ptr - Pointer to data path.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_data_path(UNCHAR *Data_path_ptr)
{
	/* Any data paths free ? */
	if (Nr_data_paths < MAX_DATA_PATHS)
	{
		/* Yes -> Copy data path */
		strcpy(&(Data_paths[Nr_data_paths][0]), Data_path_ptr);

		/* Count up */
		Nr_data_paths++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_data_path
 * FUNCTION  : Select a data path.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 10:08
 * LAST      : 28.07.95 10:08
 * INPUTS    : UNSHORT Data_path_index - Data path index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_data_path(UNSHORT Data_path_index)
{
	/* Is this a legal data path index ? */
	if (Data_path_index < Nr_data_paths)
	{
		/* Yes -> Select it */
		Change_directory(&(Data_paths[Data_path_index][0]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_second_data_path
 * FUNCTION  : Look for a second data path.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.08.95 10:45
 * LAST      : 30.10.95 02:39
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Second data path was found.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Search_second_data_path(void)
{
	static UNCHAR Data_dir[] = "XLDLIBS";

	BOOLEAN Data_path_was_found = FALSE;
	BOOLEAN Result;
	int File_handle;
	UNSHORT Length;
	UNCHAR New_data_path[MAX_DATA_PATH_LENGTH];

	/* Read source path variable */
	Result = Read_INI_variable
	(
		"SYSTEM",
		"SOURCE_PATH",
		New_data_path,
		MAX_DATA_PATH_LENGTH
	);

	/* Found ? */
	if (Result)
	{
		/* Yes -> Remove backslash at the end (if any) */
		Length = strlen(New_data_path);

		if (New_data_path[Length - 1] == '\\')
		{
			New_data_path[Length - 1] = '\0';
		}

		/* Does a data directory exist ? */
		if (Directory_exists(Data_dir))
		{
			#if FALSE
			/* Yes -> Can an arbitrary file be opened for writing ? */
			File_handle = open
			(
				"SETUP.EXE",
				O_WRONLY | O_BINARY
			);
			if (File_handle != -1)
			{
				/* Yes -> Close the file */
				close(File_handle);
			}
			else
			#endif
			{
				/* No -> Access denied ? */
//				if (errno == EACCES)
				{
					/* Yes -> Add as second data path */
					Add_data_path(New_data_path);

					/* Data path was found */
					Data_path_was_found = TRUE;
				}
			}
		}
	}

	return Data_path_was_found;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_CD
 * FUNCTION  : Check if the second data path is the Albion CD.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.10.95 15:48
 * LAST      : 28.10.95 15:48
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Second data path is the Albion CD.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_CD(void)
{
	static const UNCHAR Volume_name[] = "ALBION";

	struct find_t File_info;
	unsigned Return_code;

	/* Select second data path */
	Select_data_path(1);

	/* Find volume */
	Return_code = _dos_findfirst
	(
		Volume_name,
		_A_VOLID,
		&File_info
	);

	/* Found it ? */
	if (Return_code)
		return FALSE;
	else
		return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Read_INI_variable
 * FUNCTION  : Read a variable from the INI file.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 20:23
 * LAST      : 27.09.95 21:37
 * INPUTS    : UNCHAR *Section_name - Pointer to section name.
 *             UNCHAR *Variable_name - Pointer to variable name.
 *             UNCHAR *Value_buffer - Pointer to output buffer of variable.
 *             UNSHORT Max_variable_length - Length of output buffer.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function can be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Read_INI_variable(UNCHAR *Section_name, UNCHAR *Variable_name,
 UNCHAR *Value_buffer, UNSHORT Max_variable_length)
{
	FILE *INI_file_handle;

	BOOLEAN Section_found;
	BOOLEAN Variable_found;
	UNSHORT Length;
	UNCHAR Line_buffer[200];
	UNCHAR *Ptr;

	/* Clear value buffer */
	memset(Value_buffer, '\0', Max_variable_length);

	/* Look in first data path */
	Select_data_path(0);

	/* Try to open INI file */
	INI_file_handle = fopen
	(
		INI_filename,
		"rt"
	);

	/* Success ? */
	if (!INI_file_handle)
	{
		/* No -> Error */
//		Error(ERROR_SETUP_INI_ACCESS_ERROR);
		return FALSE;
	}

	/* Scan INI file */
	Section_found = FALSE;
	Variable_found = FALSE;
	for (;;)
	{
		/* Read next line */
		Ptr = fgets
		(
			Line_buffer,
			199,
			INI_file_handle
		);

		/* Exit if no line could be read */
		if (!Ptr)
			break;

		/* Strip offending characters from the start of the string */
		Length = strspn(Line_buffer, Evil_characters);
		if (Length)
		{
			memmove
			(
				Line_buffer,
				Line_buffer + Length,
				strlen(Line_buffer - Length + 1)
			);
		}

		/* Strip offending characters from the end of the string */
		Length = strcspn(Line_buffer,	Evil_characters);
		Line_buffer[Length] = '\0';

		/* Continue if the string is empty */
		if (!strlen(Line_buffer))
			continue;

		/* Is this a section header ? */
		if ((Line_buffer[0] == '[') &&
		 (Line_buffer[strlen(Line_buffer) - 1] == ']'))
		{
			/* Yes -> Was the section found already ? */
			if (Section_found)
			{
				/* Yes -> The variable isn't in this section */
				break;
			}
			else
			{
				/* No -> Is it the section we're looking for ? */
				if (!strnicmp
				(
					Line_buffer + 1,
					Section_name,
					strlen(Line_buffer) - 2
				))
				{
					/* Yes -> Yay! */
					Section_found = TRUE;
				}
			}
		}
		else
		{
			/* No -> Is this variable we're looking for ? */
			if (!strnicmp
			(
				Line_buffer,
				Variable_name,
				min(strlen(Line_buffer), strlen(Variable_name))
			))
			{
				/* Yes -> Look for = */
				Ptr = strchr(Line_buffer,(int) '=');

				/* Found ? */
				if (Ptr)
				{
					/* Yes -> Copy variable to output buffer */
					strncpy
					(
						Value_buffer,
						Ptr + 1,
						Max_variable_length - 1
					);

					/* Insert EOL */
					Value_buffer[Max_variable_length - 1] = '\0';
				}
				else
				{
					/* No -> Clear output buffer */
					Value_buffer[0] = '\0';
				}

				/* Yay! */
				Variable_found = TRUE;
				break;
			}
		}
	}

	/* Close file */
	fclose(INI_file_handle);

	return Variable_found;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_INI_variable
 * FUNCTION  : Write a variable in the INI file.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.09.95 20:24
 * LAST      : 10.10.95 19:09
 * INPUTS    : UNCHAR *Section_name - Pointer to section name.
 *             UNCHAR *Variable_name - Pointer to variable name.
 *             UNCHAR *Value - Pointer to new variable value.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function can be called before PCLIB32 is initialised!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Write_INI_variable(UNCHAR *Section_name, UNCHAR *Variable_name,
 UNCHAR *Value_buffer)
{
	FILE *INI_file_handle;
	FILE *Temp_file_handle;
	int Return;

	BOOLEAN Section_found;
	BOOLEAN Variable_found;
	UNSHORT Length;
	UNCHAR Line_buffer[200];
	UNCHAR Line_buffer2[200];
	UNCHAR *Ptr;

	UNCHAR Temp_filename[_MAX_PATH];
	UNCHAR Drive[_MAX_DRIVE];
	UNCHAR Dir[_MAX_DIR];
	UNCHAR Fname[_MAX_FNAME];
	UNCHAR Ext[_MAX_EXT];

	/* Look in first data path */
	Select_data_path(0);

	/* Deconstruct INI filename */
	_splitpath
	(
		INI_filename,
		Drive,
		Dir,
		Fname,
		Ext
	);

	/* Build temporary filename */
	_makepath
	(
		Temp_filename,
		Drive,
		Dir,
		Fname,
		"TMP"
	);

	/* Try to open INI file */
	INI_file_handle = fopen
	(
		INI_filename,
		"rt"
	);

	/* Success ? */
	if (!INI_file_handle)
	{
		/* No -> Error */
//		Error(ERROR_SETUP_INI_ACCESS_ERROR);
		return FALSE;
	}

	/* Try to open temporary file */
	Temp_file_handle = fopen
	(
		Temp_filename,
		"wt"
	);

	/* Success ? */
	if (!Temp_file_handle)
	{
		/* No -> Close INI file */
		fclose(INI_file_handle);

		/* Error */
//		Error(ERROR_SETUP_INI_ACCESS_ERROR);
		return FALSE;
	}

	/* Scan INI file */
	Section_found = FALSE;
	Variable_found = FALSE;
	for (;;)
	{
		/* Read next line */
		Ptr = fgets
		(
			Line_buffer,
			199,
			INI_file_handle
		);

		/* Exit if no line could be read */
		if (!Ptr)
			break;

		/* Strip offending characters from the start of the string */
		Length = strspn(Line_buffer, Evil_characters);
		if (Length)
		{
			memmove
			(
				Line_buffer,
				Line_buffer + Length,
				strlen(Line_buffer - Length + 1)
			);
		}

		/* Strip offending characters from the end of the string */
		Length = strcspn(Line_buffer,	Evil_characters);
		Line_buffer[Length] = '\0';

		/* Is the string empty ? */
		if (!strlen(Line_buffer))
		{
			/* Yes -> Write empty string to output file */
			Return = fputs
			(
				"\n",
				Temp_file_handle
			);

			/* Success ? */
			if (Return <= 0)
			{
				/* No -> Error */
//				Error(ERROR_SETUP_INI_ACCESS_ERROR);
				break;
			}

			/* Continue with next line */
			continue;
		}

		/* Already found the variable ? */
		if (!Variable_found)
		{
			/* No -> Is this a section header ? */
			if ((Line_buffer[0] == '[') &&
			 (Line_buffer[strlen(Line_buffer) - 1] == ']'))
			{
				/* Yes -> Was the section found already ? */
				if (Section_found)
				{
					/* Yes -> The variable isn't in this section, so we'll just
					  have to insert it here */
					_bprintf
					(
						Line_buffer2,
						200,
						"%s=%s\n",
						Variable_name,
						Value_buffer
					);

					/* Write line to output file */
					Return = fputs
					(
						Line_buffer2,
						Temp_file_handle
					);

					/* Success ? */
					if (Return <= 0)
					{
						/* No -> Error */
//						Error(ERROR_SETUP_INI_ACCESS_ERROR);
						break;
					}

					/* Yay! */
					Variable_found = TRUE;
				}
				else
				{
					/* No -> Is it the section we're looking for ? */
					if (!strnicmp
					(
						Line_buffer + 1,
						Section_name,
						strlen(Line_buffer) - 2
					))
					{
						/* Yes -> Yay! */
						Section_found = TRUE;
					}
				}
			}
			else
			{
				/* No -> Is this variable we're looking for ? */
				if (!strnicmp
				(
					Line_buffer,
					Variable_name,
					min(strlen(Line_buffer), strlen(Variable_name))
				))
				{
					/* Yes -> Re-write it
					 (this line will be written to the output file later on) */
					_bprintf
					(
						Line_buffer,
						200,
						"%s=%s",
						Variable_name,
						Value_buffer
					);

					/* Yay! */
					Variable_found = TRUE;
				}
			}
		}

		/* Write line to output file */
		Return = fputs
		(
			Line_buffer,
			Temp_file_handle
		);

		/* Success ? */
		if (Return <= 0)
		{
			/* No -> Error */
//			Error(ERROR_SETUP_INI_ACCESS_ERROR);
			break;
		}

		/* Write end-of-line to output file */
		Return = fputs
		(
			"\n",
			Temp_file_handle
		);

		/* Success ? */
		if (Return <= 0)
		{
			/* No -> Error */
//			Error(ERROR_SETUP_INI_ACCESS_ERROR);
			break;
		}
	}

	/* Found the section ? */
	if (!Section_found)
	{
		/* No -> Then we must create it */
		_bprintf
		(
			Line_buffer,
			200,
			"[%s]\n",
			Section_name
		);

		/* Write line to output file */
		Return = fputs
		(
			Line_buffer,
			Temp_file_handle
		);

		/* Success ? */
		if (Return <= 0)
		{
			/* No -> Error */
//			Error(ERROR_SETUP_INI_ACCESS_ERROR);
		}
		else
		{
			/* Yes -> Then we must create the variable */
			_bprintf
			(
				Line_buffer,
				200,
				"%s=%s\n",
				Variable_name,
				Value_buffer
			);

			/* Write line to output file */
			Return = fputs
			(
				Line_buffer,
				Temp_file_handle
			);

			/* Success ? */
			if (Return <= 0)
			{
				/* No -> Error */
//				Error(ERROR_SETUP_INI_ACCESS_ERROR);
			}
			else
			{
				/* Yes -> Yay! */
				Variable_found = TRUE;
			}
		}
	}
	else
	{
		/* Yes -> Found the variable ? */
		if (!Variable_found)
		{
			/* Yes -> We'll just have to insert it here */
			_bprintf
			(
				Line_buffer,
				200,
				"%s=%s\n",
				Variable_name,
				Value_buffer
			);

			/* Write line to output file */
			Return = fputs
			(
				Line_buffer,
				Temp_file_handle
			);

			/* Success ? */
			if (Return <= 0)
			{
				/* No -> Error */
//				Error(ERROR_SETUP_INI_ACCESS_ERROR);
			}
			else
			{
				/* Yes -> Yay! */
				Variable_found = TRUE;
			}
		}
	}

	/* Close files */
	fclose(INI_file_handle);
	fclose(Temp_file_handle);

	/* Were we successful ? */
	if (Variable_found)
	{
		/* Yes -> Delete the original INI file */
		remove(INI_filename);

		/* Rename the temporary file, making it the INI file */
		rename(Temp_filename, INI_filename);
	}
	else
	{
		/* No -> Delete the temporary file */
		remove(Temp_filename);
	}

	return Variable_found;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : File_exists
 * FUNCTION  : Check if a file exists.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.07.95 15:05
 * LAST      : 12.09.95 10:55
 * INPUTS    : UNCHAR *Filename - Pointer to filename.
 * RESULT    : BOOLEAN : TRUE (exists) or FALSE (does not exist).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
File_exists(UNCHAR *Filename)
{
	BOOLEAN Result = TRUE;
	short ID;

	/* Try to open the file */
	ID = open(Filename, O_RDONLY | O_BINARY);

	/* Failure ? */
	if (ID == -1)
	{
		/* Yes -> File not found ? */
		if (errno == ENOENT)
		{
			/* Yes -> File does not exist */
			Result = FALSE;
		}
	}
	else
	{
		/* No -> Close file */
		close(ID);
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Directory_exists
 * FUNCTION  : Check if a directory exists.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.95 12:50
 * LAST      : 27.10.95 12:50
 * INPUTS    : UNCHAR *Directory_name - Pointer to directory name.
 * RESULT    : BOOLEAN : TRUE (exists) or FALSE (does not exist).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Directory_exists(UNCHAR *Directory_name)
{
	BOOLEAN Result = FALSE;
	DIR *dirp;

	/* Try to open the directory */
	dirp = opendir(Directory_name);

	/* Success ? */
	if (dirp != NULL)
	{
		/* Yes -> Close directory */
		closedir(dirp);

		/* Directory exists */
		Result = TRUE;
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_directory
 * FUNCTION  : Change the current directory.
 * FILE      : MCP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.07.95 15:05
 * LAST      : 12.09.95 10:54
 * INPUTS    : UNCHAR *Pathname - Pointer to pathname.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_directory(UNCHAR *Pathname)
{
	unsigned int Dummy;
	unsigned int Drive_nr;

	/* Does the pathname contain a drive letter ? */
	if (strlen(Pathname) > 2)
	{
		if (*(Pathname + 1) == ':')
		{
			/* Yes -> Change to this drive */
			Drive_nr = (unsigned int) toupper(Pathname[0]) -
			 (unsigned int) 'A' + 1;

			_dos_setdrive(Drive_nr, &Dummy);
		}
	}

	/* Change directory */
	chdir(Pathname);
}

