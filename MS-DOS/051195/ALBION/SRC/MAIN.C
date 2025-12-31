/************
 * NAME     : MAIN.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-94
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

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <sys\timeb.h>
#include <string.h>
#include <process.h>
#include <dos.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDOS.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBEVENT.H>
#include <BBOPM.H>
#include <BBSYSTEM.H>
#include <BBMEM.H>

#include <DLL.H>
#include <AIL.H>

#include <VERSION.H>
#include <MAIN.H>
#include <GAMEVAR.H>
#include <DIAGNOST.H>
#include <XFTYPES.H>

/*
 ** Defines ****************************************************************
 */

/* Number of memory requests */
#define NR_MEMORY_REQUESTS 	(2)

/* Number of commands recognised by program */
#define NR_MAIN_COMMANDS		(3)

/* Initialisation status flag */
#define MAIN_BASEMEM_INITED	(1L << 1)
#define MAIN_DOS_INITED			(1L << 2)
#define MAIN_DSA_INITED			(1L << 3)
#define MAIN_EVENT_INITED		(1L << 5)
#define MAIN_MEM_INITED			(1L << 6)
#define MAIN_SYSTEM_INITED		(1L << 7)
#define MAIN_READY_INITED		(1L << 15)

/* Program message indices */
#define WRONG_DIR_MESSAGE		(0)
#define NO_CD_MESSAGE			(1)
#define ERROR_MESSAGE			(2)
#define GOODBYE_MESSAGE			(3)
#define ERROR_LOG_END_MESSAGE	(4)

#define MAX_MESSAGES				(5)

/*
 ** Prototypes *************************************************************
 */

void main(int argc, const char** argv);

void Show_video(UNCHAR *Video_filename);

void Open_error_log(void);
void Close_error_log(void);

void Out_of_memory_handler(UNLONG Size, UNLONG Findex,
 struct File_type *Ftype);

int __far OS_critical_error_handler(unsigned deverr, unsigned errcode,
 unsigned __far *devhdr);

void Evaluate_program_arguments(UNSHORT argc, UNCHAR **argv);

UNLONG Main_init(void);
void Main_exit(UNLONG Init_state);

void Main_error_output_function(UNCHAR * buffer);

void *AIL_Alloc(size_t Size);
void AIL_Free(void *Ptr);

BOOLEAN Init_AIL_system(void);
void Exit_AIL_system(void);

void cdecl My_system_timer(void);

void Relocate_OPM(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target, UNLONG Size);

/*
 ** Global variables *******************************************************
 */

/* AIL timer handle */
static HTIMER BB32_Timer_handle;

/* Critical error setjmp / longjmp buffer */
static jmp_buf Jump_buffer;

/* Needed by DOS4GW for spawning */
unsigned __near __minreal = 100 * 1024;

/* Quiet mode flag */
BOOLEAN No_AIL = FALSE;

/* No sound in videos flag */
static BOOLEAN Quiet_video_flag = FALSE;

/* Skip intro flag */
static BOOLEAN Skip_intro_flag = FALSE;

/* Error log status flag */
static BOOLEAN Error_log_opened = FALSE;

/* Error log file handle */
FILE *Error_log_file = NULL;

/* Main program loop command */
UNSHORT Main_program_command;

/* Program start time buffer */
static struct timeb Time_buffer;

/* Version string */
static UNCHAR Version_string[VERSION_STRING_LENGTH];

/* Memory request array for BBMEM */
static struct Memory_request Request[NR_MEMORY_REQUESTS] = {
	{
		0xFF,
		BASEMEM_Status_Flat,
		5000000,
	  10000000,
		 128000,
	},
	{
		0xFF,
		BASEMEM_Status_Dos,
		      0,
	   1000000,
		 100000,
	}
};

/*
static struct Memory_request Request[NR_MEMORY_REQUESTS] = {
	{
		0xFF,
		BASEMEM_Status_Flat,
		5602304,
		5602304,
		 128000,
	},
	{
		0xFF,
		BASEMEM_Status_Dos,
		 430320,
		 430320,
		 100000,
	}
}; */

/* Memory allocation pass list */
MEM_Alloc_pass Memory_pass_list[] = {
	MEM_Alloc_pass_standard,
	MEM_Alloc_pass_garbage,
	MEM_Alloc_pass_clean,
	NULL									/* End of list */
};

/* Main commands */
static UNCHAR *Main_commands[NR_MAIN_COMMANDS] = {
	"quiet",
	"maxmem",
	"skipintro"
};

/* Program messages */
static UNCHAR *Program_messages[MAX_MESSAGES] = {
	"Please start ALBION from the ALBION directory.",
	"Please insert the Albion CD in your CD-ROM drive.",
	"An error has occurred.\nPlease examine the file ERROR.LOG.",
	"Thank you for playing Albion.",
	"Please read the README.TXT document on how to contact\nthe Blue Byte hotline."
};

/* Normal OPM file type */
UNCHAR OPM_fname[] = "OPM";

struct File_type OPM_ftype = {
	NULL,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	&OPM_fname[0]
};

/* Relocatable OPM file type */
UNCHAR ROPM_fname[] = "Relocatable OPM";

struct File_type ROPM_ftype = {
	Relocate_OPM,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	&ROPM_fname[0]
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of Albion
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 15:59
 * LAST      : 28.10.95 15:53
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, const char **argv)
{
	int Value;
	BOOLEAN Result;
	UNLONG Init_state;

	/* Get the time */
	ftime(&Time_buffer);

	/* Print Albion version */
	Get_version_string(Version_string);
	printf("\nAlbion %s\nCopyright 1995 Blue Byte Software GmbH\n\n",
	 Version_string);

	/* Evaluate arguments */
	Evaluate_program_arguments(argc, (UNCHAR **) argv);

	/* Initialize ERROR */
	ERROR_Init(Main_error_output_function);

	/* Install OS critical error handler */
 	_harderr(OS_critical_error_handler);

	/* Initialize data paths */
	Result = Init_data_paths();

	/* Successful ? */
	if (!Result)
	{
		/* No -> Insult player */
		printf(Program_messages[WRONG_DIR_MESSAGE]);

		/* Exit */
		goto EXIT_PROGRAM;
	}

	/* Search for a second data path */
	Result = Search_second_data_path();

	/* Successful ? */
	if (!Result)
	{
		/* No -> Insult player */
		printf(Program_messages[NO_CD_MESSAGE]);

		/* Exit */
		goto EXIT_PROGRAM;
	}

	/* Check if the second data path is the Albion CD */
	Result = Check_CD();

	/* Successful ? */
	if (!Result)
	{
		/* No -> Insult player */
		printf(Program_messages[NO_CD_MESSAGE]);

		/* Exit */
		goto EXIT_PROGRAM;
	}

	/* Has the user selected no digital sound ? */
	Select_data_path(0);
	if (!File_exists("DRIVERS\\DIG.INI"))
	{
		/* Yes -> Make all videos quiet */
		Quiet_video_flag = TRUE;
	}

	/* Skip the intro ? */
	if (Skip_intro_flag)
	{
		/* Yes -> Give start program command */
		Main_program_command = START_PROGRAM_COMMAND;
	}
	else
	{
		/* No -> Give initial command */
		Main_program_command = SHOW_INTRO_COMMAND;
	}

	do
	{
		/* Act depending on command */
		switch (Main_program_command)
		{
			/* Start the program */
			case START_PROGRAM_COMMAND:
			{
				/* Initialize everything */
				Init_state = Main_init();

				/* Everything OK ? */
				if (Init_state & MAIN_READY_INITED)
				{
					/* Yes -> Set jump buffer */
					Value = setjmp(Jump_buffer);

					/* First time ? */
					if (!Value)
					{
						/* Yes -> Start BBMAIN */
						BBMAIN(argc, argv);
					}
				}

				/* Exit system */
				Main_exit(Init_state);

				/* Close all files */
				fcloseall();
				break;
			}
			/* Show the intro sequence */
			case SHOW_INTRO_COMMAND:
			{
				/* Show */
				Show_video("INTRO.SMK");

				/* Give start program command */
				Main_program_command = START_PROGRAM_COMMAND;
				break;
			}
			/* Show the credits sequence */
			case SHOW_CREDITS_COMMAND:
			{
				/* Show */
				Show_video("CREDITS.SMK");

				/* Give start program command */
				Main_program_command = START_PROGRAM_COMMAND;
				break;
			}
		}
	}
	/* Until exit program command is given */
	while (Main_program_command != EXIT_PROGRAM_COMMAND);

	/* Have any errors been reported / has the error log been opened ? */
	if (!ERROR_StackEmpty() || Error_log_opened)
	{
		/* Yes -> Open error log */
		Open_error_log();

		/* Print contents of error stack */
		ERROR_PrintAllErrors(BBERROR_PAE_NORMAL);

		/* Close error log */
		Close_error_log();

		/* Inform the player */
		printf(Program_messages[ERROR_MESSAGE]);
	}
	else
	{
		/* No -> Bye, bye */
		printf(Program_messages[GOODBYE_MESSAGE]);
	}

EXIT_PROGRAM:

	/* Select first data path */
	Select_data_path(0);

	printf("\n\n");
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_video
 * FUNCTION  : Show a video.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.10.95 18:33
 * LAST      : 31.10.95 13:26
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
			/* Yes -> Spawn video player
			  (play second sound track, which does not exist) */
			spawnl
			(
				P_WAIT,
				"SMACKPLY.EXE",
				"SMACKPLY",
				Video_filename,
				"/F10",
				"/U3",
				"/T2",
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
				"/F10",
				"/U3",
				NULL
			);
		}
	}
	else
	{
		/* No -> Report error */
		Error(ERROR_FILE_LOAD);
	}

	/* Switch back to normal directory */
	Change_directory("..");
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Open_error_log
 * FUNCTION  : Open the error log and print the error log header.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.10.95 18:30
 * LAST      : 27.10.95 12:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Open_error_log(void)
{
	/* Select first data path */
	Select_data_path(0);

	/* Has the error log already been opened ? */
	if (!Error_log_opened)
	{
		/* No -> Open error log */
		Error_log_file = fopen("ERROR.LOG", "at");

		/* Print error log header */
		fprintf(Error_log_file, "\n\n**** Error log ****\n\n");

		/* Print Albion version string */
		fprintf(Error_log_file, "Albion %s\n", Version_string);

		/* Print the time */
		fprintf(Error_log_file, "Executed on %s\n", ctime(&Time_buffer.time));

		/* Indicate the error log has been opened */
		Error_log_opened = TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Close_error_log
 * FUNCTION  : Close the error log.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.95 12:21
 * LAST      : 27.10.95 12:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Close_error_log(void)
{
	/* Has the error log been opened ? */
	if (Error_log_opened)
	{
		/* Yes -> Print error log end */
		fprintf(Error_log_file, "\n\n");
		fprintf(Error_log_file, Program_messages[ERROR_LOG_END_MESSAGE]);
		fprintf(Error_log_file, "\n\n");

		/* Close error log */
		fclose(Error_log_file);

		/* Indicate the error log is no longer open */
		Error_log_opened = FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Out_of_memory_handler
 * FUNCTION  : Handle BBMEM out of memory error.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.10.95 18:38
 * LAST      : 14.10.95 18:38
 * INPUTS    : UNLONG Size - Desired size.
 *             UNLONG Findex - File index.
 *             struct File_type *Ftype - File type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Out_of_memory_handler(UNLONG Size, UNLONG Findex, struct File_type *Ftype)
{
	/* Open error log */
	Open_error_log();

	/* Print error description */
	fprintf(Error_log_file, "\nBBMEM could not allocate memory.\n");

	if (Ftype->Name)
	{
		fprintf
		(
			Error_log_file,
			"Type : %s %lu\n",
			Ftype->Name,
			Findex
		);
	}

	fprintf(Error_log_file, "Desired size: %lu\n", Size);

	/* Print memory manager report */
	MEM_Report(Error_log_file);

	fprintf(Error_log_file, "\n");
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Critical_error_handler
 * FUNCTION  : Handle a critical error.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 11:25
 * LAST      : 23.10.95 16:12
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Critical_error_handler(void)
{
	/* Give exit program command */
	Main_program_command = EXIT_PROGRAM_COMMAND;

	/* Leave the program immediately */
	longjmp(Jump_buffer, 1);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : OS_critical_error_handler
 * FUNCTION  : Handle a critical OS error.
 * FILE      : MAIN.C
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
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 10:39
 * LAST      : 21.10.95 22:10
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
		Command_index = Parse_argument(argv[Argument_index], NR_MAIN_COMMANDS,
		 Main_commands);

		/* Act depending on command type */
		switch (Command_index)
		{
			/* AIL will not be installed */
			case 0:
			{
				No_AIL = TRUE;
				Quiet_video_flag = TRUE;
				break;
			}
			/* Set maximum amount of grabbed XMS memory */
			case 1:
			{
				Request[0].Maximum = Parse_value(argv[Argument_index + 1]) * 1024;
				break;
			}
			/* Skip the intro sequence */
			case 2:
			{
				Skip_intro_flag = TRUE;
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_init
 * FUNCTION  : Initialize all libraries
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:05
 * LAST      : 26.10.95 16:17
 * INPUTS    : None.
 * RESULT    : UNLONG : Bitlist containing initialization state.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Main_init(void)
{
	struct Memory_workspace *Workspace;
	BOOLEAN Result;
	UNLONG Init_state = 0;

	/* Initialize BASEMEM */
	Result = BASEMEM_Init();

	/* Success ? */
	if (Result)
	{
		/* Yes -> Indicate BASEMEM was initialized */
		Init_state |= MAIN_BASEMEM_INITED;
	}
	else
	{
		/* No -> Exit */
		return Init_state;
	}

	/* Initialize DOS */
	Result = DOS_Init();

	/* Success ? */
	if (Result)
	{
		/* Yes -> Indicate DOS was initialized */
		Init_state |= MAIN_DOS_INITED;
	}
	else
	{
		/* No -> Exit */
		return Init_state;
	}

	/* Initialize DSA */
	Result = DSA_Init();

	/* Success ? */
	if (Result)
	{
		/* Yes -> Indicate DSA was initialized */
		Init_state |= MAIN_DSA_INITED;
	}
	else
	{
		/* No -> Exit */
		return Init_state;
	}

	/* Initialize EVENT */
	Result = BLEV_Init();

	/* Success ? */
	if (Result)
	{
		/* Yes -> Indicate EVENT was initialized */
		Init_state |= MAIN_EVENT_INITED;
	}
	else
	{
		/* No -> Exit */
		return Init_state;
	}

	/* Initialize SYSTEM */
	Result = SYSTEM_Init();

	/* Success ? */
	if (Result)
	{
		/* Yes -> Indicate SYSTEM was initialized */
		Init_state |= MAIN_SYSTEM_INITED;
	}
	else
	{
		/* No -> Exit */
		return Init_state;
	}

	/* Initialize BBMEM */
	Result = MEM_Init_memory(NR_MEMORY_REQUESTS, &Request[0]);

	/* Success ? */
	if (Result)
	{
		/* Yes -> Get workspace */
		Workspace = MEM_Workspace_stack[0];

		/* Set out of memory handler */
		Workspace->Out_of_memory = Out_of_memory_handler;

		/* Set allocation pass list */
		Workspace->Pass_list = Memory_pass_list;

		/* Indicate BBMEM was initialized */
		Init_state |= MAIN_MEM_INITED;
	}
	else
	{
		/* No -> Open error log */
		Open_error_log();

		/* Print error description */
		fprintf(Error_log_file, "\nBBMEM could not be initialised.\n");

		/* Print BASEMEM report */
		BASEMEM_Report(Error_log_file);

		fprintf(Error_log_file, "\n");

		/* Exit */
		return Init_state;
	}

	/* Everything is OK */
	Init_state |= MAIN_READY_INITED;

	return Init_state;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_exit
 * FUNCTION  : Terminate all libraries
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:11
 * LAST      : 26.07.94 16:11
 * INPUTS    : UNLONG Init_state - Bitlist containing initialization state.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_exit(UNLONG Init_state)
{
	if (Init_state & MAIN_MEM_INITED)
		MEM_Exit_memory();

	if (Init_state & MAIN_SYSTEM_INITED)
		SYSTEM_Exit();

	if (Init_state & MAIN_EVENT_INITED)
		BLEV_Exit();

	if (Init_state & MAIN_DSA_INITED)
		DSA_Exit();

	if (Init_state & MAIN_DOS_INITED)
		DOS_Exit();

	if (Init_state & MAIN_BASEMEM_INITED)
		BASEMEM_Exit();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_error_output_function
 * FUNCTION  : Output function for BBERROR.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:12
 * LAST      : 26.10.95 16:16
 * INPUTS    : UNCHAR *buffer - Pointer to buffer that must be put out.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Main_error_output_function(UNCHAR * buffer)
{
	/* Has the error log been opened ? */
	if (Error_log_opened)
	{
		/* Yes -> Send the error to the error log */
		fprintf(Error_log_file, "%s", (char *) buffer);
	}
	else
	{
		/* No -> Send the error to the screen */
		printf("%s", (char *) buffer);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : AIL_Alloc
 * FUNCTION  : Memory allocation function for AIL.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 15:46
 * LAST      : 10.02.95 15:46
 * INPUTS    : size_t Size - Size of memory block.
 * RESULT    : void * : Pointer to memory (NULL = failure).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void *
AIL_Alloc(size_t Size)
{
	return((void *) BASEMEM_Alloc((UNLONG) Size, BASEMEM_Status_Flat));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : AIL_Free
 * FUNCTION  : Memory free function for AIL.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.02.95 15:46
 * LAST      : 10.02.95 15:46
 * INPUTS    : void *Ptr - Pointer to memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
AIL_Free(void *Ptr)
{
	BASEMEM_Free((UNBYTE *) Ptr);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_AIL_system
 * FUNCTION  : Init AIL system.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.02.95 17:29
 * LAST      : 11.02.95 17:29
 * INPUTS    : None.
 * RESULT    : BOOLEAN : AIL installed successfully.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_AIL_system(void)
{
	/* Install AIL ? */
	if (No_AIL)
	{
		/* No -> Exit */
		return FALSE;
	}
	else
	{
		/* Yes -> Make sure AIL uses BASEMEM to allocate memory */
		MEM_use_malloc(AIL_Alloc);
		MEM_use_free(AIL_Free);

		/* Initialize AIL */
		AIL_startup();

		/* Register the BB32 timer with AIL */
		BB32_Timer_handle = AIL_register_timer(My_system_timer);

		/* Set timer frequency */
		AIL_set_timer_frequency(BB32_Timer_handle, (ULONG) TICKS_PER_SECOND);

		/* Start timer */
		AIL_start_timer(BB32_Timer_handle);

		return TRUE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_AIL_system
 * FUNCTION  : Exit AIL system.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.02.95 17:29
 * LAST      : 15.03.95 12:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_AIL_system(void)
{
	/* Was AIL installed ? */
	if (!No_AIL)
	{
		/* Yes -> Stop the timer */
		AIL_stop_timer(BB32_Timer_handle);

		/* Un-register the timer */
		AIL_release_timer_handle(BB32_Timer_handle);

		/* Shutdown AIL */
		AIL_shutdown();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : My_system_timer
 * FUNCTION  : Timer-interrupt function under AIL 3.0
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.11.94 10:41
 * LAST      : 15.02.95 14:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : AIL API reference.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

#pragma off (check_stack)

void cdecl
My_system_timer(void)
{
	SYSTEM_Timer_function();
}

#pragma on (check_stack)

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Parse_argument
 * FUNCTION  : Parse an argument.
 * FILE      : MAIN.C
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
 * FILE      : MAIN.C
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
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Relocate_OPM
 * FUNCTION  : Relocate a relocatable OPM.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.94 12:03
 * LAST      : 17.10.95 18:56
 * INPUTS    : MEM_HANDLE Handle - Handle of memory block.
 *             UNBYTE *Source - Pointer to source.
 *             UNBYTE *Target - Pointer to target.
 *             UNLONG Size - Size of memory block.
 * RESULT    : None.
 * BUGS      : - This function doesn't work properly when the OPM has virtual
 *              OPMs. This is because the virtual OPMs are not connected to
 *              their parent OPMs.
 * NOTES     : - Naturally, this is a bit of a cheat. None of the OPM
 *              functions will ever claim a pointer. But it works anyway.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Relocate_OPM(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target, UNLONG Size)
{
	/* Copy the memory block */
	BASEMEM_CopyMem(Source, Target, Size);

	/* Change pointer to data in OPM structure */
	((struct OPM *) Handle->File_index)->data = Target;
}

