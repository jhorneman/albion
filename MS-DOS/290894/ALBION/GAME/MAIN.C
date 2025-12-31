/************
 * NAME     : MAIN.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26.07.94 15:54
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#include <stdio.h>

/* includes */

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBDOS.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBEVENT.H>
#include <BBOPM.H>
#include <BBSYSTEM.H>
#include <BBMEM.H>

/* prototypes */

void main(int argc, char** argv);

UNLONG Main_init (void);
void Main_exit (UNLONG Init_state);

void Main_error_output_function(UNCHAR * buffer);

/* defines */
#define MAIN_BASEMEM_INITED	(1L << 1)
#define MAIN_DOS_INITED			(1L << 2)
#define MAIN_DSA_INITED			(1L << 3)
#define MAIN_ERROR_INITED		(1L << 4)
#define MAIN_EVENT_INITED		(1L << 5)
#define MAIN_MEM_INITED			(1L << 6)
#define MAIN_SYSTEM_INITED		(1L << 7)
#define MAIN_READY_INITED		(1L << 31)

/* global variables */
UNSHORT Nr_memory_requests = 1;
struct Memory_request Request[1] = {
	0xFF,
	BASEMEM_Status_Flat,
	1500000,
	1600000
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : main
 * FUNCTION  : Main function of Albion
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 15:59
 * LAST      : 26.07.94 15:59
 * INPUTS    : int argc - Number of arguments.
 *             char **argv - Pointer to list of arguments.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will initialize all libraries and call BBMAIN.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
main(int argc, char **argv)
{
	UNLONG Init_state;

	/* Initialize everything */
	Init_state = Main_init();

	/* Everything OK ? */
	if (Init_state & MAIN_READY_INITED)
	{
		/* Yes -> Start BBMAIN */
		BBMAIN(argc, argv);
	}

	/* Exit system */
	Main_exit(Init_state);

	/* Show errors */
	ERROR_PrintAllErrors(BBERROR_PAE_NORMAL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Main_init
 * FUNCTION  : Initialize all libraries
 * FILE      : MAIN.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.94 16:05
 * LAST      : 26.07.94 16:05
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
	UNLONG Init_state = 0;

	/* Initialize ERROR */
	ERROR_Init(&Main_error_output_function);

	/* Initialize BASEMEM */
	if (BASEMEM_Init())
		Init_state |= MAIN_BASEMEM_INITED;
	else
		return(Init_state);

	/* Initialize DOS */
	if (DOS_Init())
		Init_state |= MAIN_DOS_INITED;
	else
		return(Init_state);

	/* Initialize DSA */
	if (DSA_Init())
		Init_state |= MAIN_DSA_INITED;
	else
		return(Init_state);

	/* Initialize EVENT */
	if (BLEV_Init())
		Init_state |= MAIN_EVENT_INITED;
	else
		return(Init_state);

	/* Initialize SYSTEM */
	if (SYSTEM_Init())
		Init_state |= MAIN_SYSTEM_INITED;
	else
		return(Init_state);

	/* Initialize MEM */
	if (MEM_Init_memory(Nr_memory_requests,&Request[0]))
		Init_state |= MAIN_MEM_INITED;
	else
		return(Init_state);

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
	printf("%s",(char *) buffer);
}

