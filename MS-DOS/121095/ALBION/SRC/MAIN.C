/************
 * NAME     : MAIN.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26.07.94 15:54
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <sys\timeb.h>

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

/* defines */

#define NR_MEMORY_REQUESTS 	(2)

#define NR_MAIN_COMMANDS		(5)

/* global variables */
static HTIMER BB32_Timer_handle;

static jmp_buf Jump_buffer;

BOOLEAN No_AIL = FALSE;

static struct Memory_request Request[NR_MEMORY_REQUESTS] = {
	{
		0xFF,
		BASEMEM_Status_Flat, // | BASEMEM_Status_Lock,
		5000000,
	   6000000,
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
	"notimer",
	"nomouse",
	"maxmem",
	"noload"
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of Albion
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 15:59
 * LAST      : 30.09.95 14:43
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will initialize the PC library and call BBMAIN.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, const char **argv)
{
	static struct timeb Time_buffer;
	int Value;

	UNLONG Init_state;
	UNCHAR Version_string[VERSION_STRING_LENGTH];

	/* Get the time */
	ftime(&Time_buffer);

	/* Print Albion version */
	Get_version_string(Version_string);
	printf("\nAlbion %s\nCopyright 1995 Blue Byte Software GmbH\n\n",
	 Version_string);

	/* Does the user want help ? */
	if (!strnicmp(argv[1], "?", 1))
	{
		/* Yes -> Print help text */
		printf("The following parameters are available:\n");
		printf("\n");
		printf("QUIET    \n");
		printf("The program does not play any music or sound effects.\n");
		printf("NOTIMER  \n");
		printf("The program does not install a timer interrupt.\n");
		printf("NOMOUSE  \n");
		printf("The program does not install a mouse interrupt.\n");
		printf("MAXMEM   maxmem\n");
		printf("Sets the maximum amount of XMS memory in kilobytes that the program tries\n");
		printf("to allocate.\n");
		printf("NOLOAD   \n");
		printf("The program does not extract files from a saved game.\n");
		printf("\n");
		printf("\n");
		printf("The parameters are case-insensitive and need not be preceded by a backslash.\n");
		printf("\n");
	}
	else
	{
		/* No -> Evaluate arguments */
		Evaluate_program_arguments(argc, (UNCHAR **) argv);

		/* Initialize everything */
		Init_state = Main_init();

//		BASEMEM_Report();

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

		/* Have any errors been reported ? */
		if (!ERROR_StackEmpty())
		{
			/* Yes -> Redirect all (!) output to error log */
			freopen("ERROR.LOG", "at", stdout);
			freopen("ERROR.LOG", "at", stderr);
			freopen("ERROR.LOG", "at", stdaux);
			freopen("ERROR.LOG", "at", stdprn);

			/* Print error log header */
			printf("\n\n**** Error log ****\n\n");

			/* Print Albion version string */
			printf("Albion %s\n", Version_string);

			/* Print the time */
			printf("Executed on %s\n", ctime(&Time_buffer.time));

			/* Print contents of error stack */
			ERROR_PrintAllErrors(BBERROR_PAE_NORMAL);
		}
		else
		{
//			BASEMEM_Report();

			/* No -> Bye, bye */
			printf("\nThank you for playing Albion.\n\n");
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Critical_error_handler
 * FUNCTION  : Handle a critical error.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 11:25
 * LAST      : 31.08.95 11:26
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
	/* Leave the program immediately */
	longjmp(Jump_buffer, 1);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Evaluate_program_arguments
 * FUNCTION  : Evaluate program arguments.
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 10:39
 * LAST      : 28.09.95 14:27
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
				break;
			}
			/* No timer interrupt will be installed */
			case 1:
			{
				SYSTEM_Init_Flags |= SYSTEM_FLAG_NOTIMERINT;
				SYSTEM_Init_Flags |= SYSTEM_FLAG_TIMERSYSTEMTASK;
				break;
			}
			/* No mouse interrupt will be installed */
			case 2:
			{
				SYSTEM_Init_Flags |= SYSTEM_FLAG_NOMOUSEINT;
				SYSTEM_Init_Flags |= SYSTEM_FLAG_MOUSESYSTEMTASK;
				break;
			}
			/* Set maximum amount of grabbed XMS memory */
			case 3:
			{
				Request[0].Maximum = Parse_value(argv[Argument_index + 1]) * 1024;
				break;
			}
			/* Don't load file parts from saved game */
			case 4:
			{
				No_load_flag = TRUE;
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
 * LAST      : 27.08.95 20:11
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
	UNLONG Init_state = 0;

	/* Initialize ERROR */
	ERROR_Init(Main_error_output_function);

	/* Initialize BASEMEM */
	if (BASEMEM_Init())
		Init_state |= MAIN_BASEMEM_INITED;
	else
		return Init_state;

	/* Initialize DOS */
	if (DOS_Init())
		Init_state |= MAIN_DOS_INITED;
	else
		return Init_state;

	/* Initialize DSA */
	if (DSA_Init())
		Init_state |= MAIN_DSA_INITED;
	else
		return Init_state;

	/* Initialize EVENT */
	if (BLEV_Init())
		Init_state |= MAIN_EVENT_INITED;
	else
		return Init_state;

	/* Initialize SYSTEM */
	if (SYSTEM_Init())
		Init_state |= MAIN_SYSTEM_INITED;
	else
		return Init_state;

	/* Initialize MEM */
	if (MEM_Init_memory(NR_MEMORY_REQUESTS, &Request[0]))
	{
		/* Get workspace */
		Workspace = MEM_Workspace_stack[0];

		/* Set memory allocation pass list */
		Workspace->Pass_list = Memory_pass_list;

		Init_state |= MAIN_MEM_INITED;
	}
	else
		return Init_state;

	/* Everything is OK */
	Init_state |= MAIN_READY_INITED;

	return (Init_state);
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
 * LAST      : 26.07.94 16:12
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
	printf("%s", (char *) buffer);
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

