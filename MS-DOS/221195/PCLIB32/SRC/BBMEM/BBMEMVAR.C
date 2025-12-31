/************
 * NAME     : BBMEMVAR.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 1-6-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    :
 * SEE ALSO : BBMEM.C, BBMEM.H
 ************/

/* includes */

#include <BBDEF.H>

#include <BBERROR.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include "INCLUDE\BBMEMVAR.H"

/* external global variables */

UNBYTE MEM_Debugging = FALSE;

struct File_type MEM_Default_file_type = {
	MEM_Relocate,
	0xff,
	0,
	MEM_KILL_ALWAYS,					/* MUST BE SET FOR NORMAL MEMORY */
	NULL
};

MEM_Alloc_pass MEM_Default_pass_list[] = {
	MEM_Alloc_pass_standard,
	MEM_Alloc_pass_garbage,
	MEM_Alloc_pass_drown,
	NULL									/* End of list */
};

/* global variable declarations */

void (*MEM_Critical_error_handler)(void) = NULL;
void (*MEM_Start_lengthy_operation_handler)(void) = NULL;
void (*MEM_End_lengthy_operation_handler)(void) = NULL;

struct Memory_entry MEM_Entries[MEMORY_ENTRIES_MAX];
struct Memory_handle MEM_Handles[MEMORY_HANDLES_MAX];
struct Memory_workspace MEM_Workspaces[MEMORY_WORKSPACES_MAX];

struct Memory_entry *MEM_Quick_free_entry = NULL;

struct Grabbed_memory Grabbed_memory[MEMORY_AREAS_MAX];

struct Memory_workspace *MEM_Workspace_stack[WORKSPACE_STACK_MAX];
UNSHORT MEM_Workspace_stack_index = 0;

BOOLEAN MEM_Initialized = FALSE;

BOOLEAN MEM_Handles_invalid = FALSE;

MEM_Alloc_pass *MEM_Pass_list_ptr;

UNCHAR MEM_Library_name[] = "Memory manager";

UNCHAR *MEM_Error_strings[MEMERR_MAX+1] = {
	"Illegal error code.",		  	/* Error code was 0 or > MEMERR_MAX. */
	"Out of memory entries.",    	/* MEM_Find_new_entry() failed. */
	"Out of memory handles.",	  	/* MEM_Create_memory_handle() failed. */
	"Memory handle mismatch.",	  	/* BLOCK_HANDLE didn't match Entry_ptr. */
	"Memory request denied.",	  	/* MEM_Init_memory() failed. */
	"Entry lies outside array.",
	"File type is NULL.",
	"Free entry in chain.",
	"Area descriptor in chain.",
	"Wrong backward link.",
	"Memory block lies outside area.",
	"Gap between memory blocks.",	/* End of one block is not the start of the next. */
	"Sum of blocks doesn't match area size.",
	"Out of memory.",
	"Tried to claim free handle.",
	"Illegal handle.",
	"Tried to free free handle.",
	"Tried to kill free handle."
};

