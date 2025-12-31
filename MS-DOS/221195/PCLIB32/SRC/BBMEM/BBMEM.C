/************
 * NAME     : BBMEM.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 30-5-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    : - It is VITAL that [ Claim_pointer ] return zero when handle
 *             NULL is claimed.
 *          : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : BBMEMALO.C, BBMEM.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>

#include <BBERROR.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include "INCLUDE\BBMEMVAR.H"

/* prototypes */

void MEM_Init_workspace(struct Memory_workspace *Workspace);

void MEM_Reset_workspace(struct Memory_workspace *Workspace);

void MEM_Init_area(struct Memory_entry *Area);

void MEM_Print_error(UNCHAR *buffer, UNBYTE *data);

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Init_memory
 * FUNCTION  : Initializes the memory manager and the first workspace.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.10.95 19:45
 * LAST      : 13.11.95 11:47
 * INPUTS    : UNSHORT Nr_requests - Amount of memory requests in list.
 *             struct Memory_request *Request_list - Pointer to a list of
 *              memory requests.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * NOTES     : - This function will check whether the memory manager has
 *              already been initialized.
 *             - MEM_Reset_memory will be called by this function.
 *             - This function will store the unaligned pointers to the
 *              allocated memory block, which will be used by MEM_Exit_memory.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
MEM_Init_memory(UNSHORT Nr_requests, struct Memory_request *Request_list)
{
	struct Memory_entry *Area;
	BOOLEAN Success;
	UNLONG Size;
	UNLONG Start;
	UNLONG End;
	UNSHORT Nr_areas;
	UNSHORT i, j;

	/* Already initialized ? */
	if (MEM_Initialized)
	{
		/* Yes -> Exit */
		return TRUE;
	}

	/* Clear memory entry array */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &MEM_Entries[0],
		MEMORY_ENTRIES_MAX * sizeof(struct Memory_entry),
		0
	);

	/* Clear memory handle array */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &MEM_Handles[0],
		MEMORY_HANDLES_MAX * sizeof(struct Memory_handle),
		0
	);

	/* Clear memory workspace array */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &MEM_Workspaces[0],
		MEMORY_WORKSPACES_MAX * sizeof(struct Memory_workspace),
		0
	);

	/* Clear grabbed memory array */
	BASEMEM_FillMemByte
	(
		(UNBYTE *) &Grabbed_memory[0],
		MEMORY_AREAS_MAX * sizeof(struct Grabbed_memory),
		0
	);

	/* Clear other variables */
	MEM_Critical_error_handler				= NULL;
	MEM_Start_lengthy_operation_handler	= NULL;
	MEM_End_lengthy_operation_handler	= NULL;

	MEM_Quick_free_entry = NULL;

	MEM_Workspace_stack_index = 0;

	MEM_Handles_invalid = FALSE;

	/* Handle each memory request */
	Success = TRUE;
	Nr_areas = 0;
	for (i=0;i<min(Nr_requests, MEMORY_AREAS_MAX);i++)
	{
		/* Get the largest available block */
		Size = BASEMEM_Get_largest_block(Request_list[i].Base_memory_type);

		/* Subtract size that must be left to the system (if possible) */
		if (Size > Request_list[i].Safety)
		{
			Size -= Request_list[i].Safety;
		}
		else
		{
			Size = 0;
		}

		/* Too large ? */
		if (Size > Request_list[i].Maximum)
		{
			/* Yes -> Clip */
			Size = Request_list[i].Maximum;
		}

		/* Too small ? */
		if (Size < max(SMALL_FISH, Request_list[i].Minimum))
		{
			/* Yes -> Free all memory that was grabbed to this point */
			for (j=0;j<i;j++)
			{
				BASEMEM_Free(Grabbed_memory[j].Start);
			}

			/* Report error */
			MEM_Error(MEMERR_REQUEST_DENIED);

			Success = FALSE;
			break;
		}

		/* Allocate it */
		Start = (UNLONG) BASEMEM_Alloc
		(
			Size,
			Request_list[i].Base_memory_type
		);

		/* Success ? */
		if (!Start)
		{
			/* No -> Free all memory that was grabbed to this point */
			for (j=0;j<i;j++)
			{
				BASEMEM_Free(Grabbed_memory[j].Start);
			}

			/* Report error */
			MEM_Error(MEMERR_REQUEST_DENIED);

			Success = FALSE;
			break;
		}

		/* Store original start and size */
		Grabbed_memory[i].Start	= (UNBYTE *) Start;
		Grabbed_memory[i].Size	= Size;

		/* Align start and size */
		End = Start + Size;

		if (End != BASEMEM_Align(End))
		{
			End = BASEMEM_Align(End - BASEMEM_ALIGNMENT);
		}

		Start = BASEMEM_Align(Start);

		/* Initialize area */
		Area = &(MEM_Workspaces[0].Areas[Nr_areas]);

		Area->Start					= (UNBYTE *) Start;
		Area->Size					= (UNLONG)(End - Start);

		Area->Previous				= NULL;
		Area->Next					= NULL;

		Area->WORKSPACE_NUMBER	= 0;
		Area->MEMORY_TYPE			= Request_list[i].Memory_type;

		/* Count up */
		Nr_areas++;
	}

	/* Were we successful ? */
	if (Success)
	{
		/* Yes -> Initialize workspace */
		MEM_Workspaces[0].Nr_of_areas			= Nr_areas;
		MEM_Workspaces[0].Parent_workspace	= 0;

		/* Memory has been initialized */
		MEM_Initialized = TRUE;

		/* Reset workspace stack */
		MEM_Workspace_stack_index	= 0;
		MEM_Workspace_stack[0]		= &(MEM_Workspaces[0]);

		/* Reset memory manager */
		MEM_Reset_memory();
	}

	return Success;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Exit_memory
 * FUNCTION  : Exits the memory manager. All memory is returned to the OS.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 14.10.95 15:26
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will check whether the memory manager has
 *              already been initialized.
 *             - The memory manager data will not be destroyed so the data
 *              can still be used for post-mortem analysis. This means
 *              disasters will happen if BBMEM is used after this function
 *              has been called!!!
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Exit_memory(void)
{
	struct Memory_workspace *Workspace;
	UNSHORT i;

	/* Initialized ? */
	if (MEM_Initialized == TRUE)
	{
		/* Yes -> Destroy the first workspace */
		Workspace = &MEM_Workspaces[0];

		/* Free all areas */
		for(i=0;i<Workspace->Nr_of_areas;i++)
		{
			/* Free memory */
			BASEMEM_Free(Grabbed_memory[i].Start);
		}

		/* Memory is no longer initialized */
		MEM_Initialized = FALSE;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Get_total_memory
 * FUNCTION  : Get the total amount of memory of a certain type.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.11.95 11:38
 * LAST      : 13.11.95 11:38
 * INPUTS    : UNBYTE Type - Desired memory type(s).
 * RESULT    : UNLONG : Size.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
MEM_Get_total_memory(UNBYTE Type)
{
	struct Memory_workspace *Current;
	UNSHORT i;
	UNLONG Total_size = 0;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Check all areas in the current workspace */
	for (i=0;i<(Current->Nr_of_areas);i++)
	{
		/* Right type ? */
		if (Current->Areas[i].MEMORY_TYPE & Type)
		{
			/* Yes -> Add size to total */
			Total_size += Current->Areas[i].Size;
		}
	}

	return Total_size;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Reset_memory
 * FUNCTION  : Resets the memory manager. All memory workspaces apart from the
 *             first one and all handles will be destroyed. All areas will be
 *             initialized.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 06.06.94 12:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Reset_memory(void)
{
	#if FALSE
	UNSHORT i;

	/* Delete all workspaces except the first one */
	for(i=1;i<MEMORY_WORKSPACES_MAX;i++)
	{
		if (MEM_Workspaces[i].Nr_of_areas)
		{
			MEM_Delete_workspace(&MEM_Workspaces[i]);
		}
	}
	#endif

	/* Initialize workspace #0 */
	MEM_Init_workspace(&MEM_Workspaces[0]);

	/* Just to be sure */
	MEM_Clear_all_handles();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Init_workspace
 * FUNCTION  : Initializes a memory workspace. All memory areas belonging to
 *             this workspace are initialized.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 06.06.94 12:02
 * INPUTS    : struct Memory_workspace *Workspace - Pointer to memory
 *              workspace descriptor.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Init_workspace(struct Memory_workspace *Workspace)
{
	/* Clear flags */
	Workspace->Flags = 0;

	/* Set default allocation pass list and other pointers */
	Workspace->Alloc_init		= NULL;
	Workspace->Alloc_exit		= NULL;
	Workspace->Out_of_memory	= NULL;
	Workspace->Pass_list			= MEM_Default_pass_list;

	/* Reset workspace */
	MEM_Reset_workspace(Workspace);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Reset_workspace
 * FUNCTION  : Resets a memory workspace. All memory areas belonging to
 *             this workspace are initialized.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 06.06.94 12:02
 * INPUTS    : struct Memory_workspace *Workspace - Pointer to memory
 *              workspace descriptor.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Reset_workspace(struct Memory_workspace *Workspace)
{
	UNSHORT i;

	/* Initialize all memory areas in this workspace */
	for(i=0;i<Workspace->Nr_of_areas;i++)
	{
		MEM_Init_area(&(Workspace->Areas[i]));
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Init_area
 * FUNCTION  : Initializes a memory area. All memory entries linked to this
 *             one and their handles are deleted. One big, free memory entry
 *             is created.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 31.08.95 11:05
 * INPUTS    : struct Memory_entry *Area - Pointer to memory area descriptor.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Init_area(struct Memory_entry *Area)
{
	struct Memory_entry *Entry;
	struct Memory_entry *Next;

	/* Check all entries in this area */
	Entry = Area->Next;
	while (Entry)
	{
		/* Get next memory entry */
		Next = Entry->Next;

		/* Does this entry have a handle ? */
		if (Entry->BLOCK_HANDLE)
		{
			/* Yes -> Destroy it */
			MEM_Destroy_memory_handle(Entry);
		}

		/* Delete the memory entry */
		MEM_Delete_entry(Entry);

		/* Next entry */
		Entry = Next;
	}

	/* Find a free entry */
	Entry = MEM_Find_free_entry();

	/* Any found ? */
	if (Entry)
	{
		/* Yes -> Initialize the first entry */
		Entry->Start	= Area->Start;
		Entry->Size		= Area->Size;

		/* Add it to the chain */
		MEM_Add_entry(Entry, Area);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Report_error
 * FUNCTION  : Report a memory manager error.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.94 11:49
 * LAST      : 26.10.95 19:25
 * INPUTS    : UNSHORT : Error_code - Error code.
 *             int Line_nr - Line number where error was reported.
 *             char *Filename - Filename where error was reported.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Report_error(UNSHORT Error_code, int Line_nr, char *Filename)
{
	struct Memory_error Error;

	/* Build error report */
	Error.Code			= Error_code;
	Error.Line_nr		= (UNSHORT) Line_nr;
	Error.Filename		= (UNCHAR *) Filename;

	/* Push error on the error stack */
	ERROR_PushError
	(
		MEM_Print_error,
		MEM_Library_name,
		sizeof(struct Memory_error),
		(UNBYTE *) &Error
	);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Report_critical_error
 * FUNCTION  : Report a critical memory manager error.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 11:02
 * LAST      : 26.10.95 19:45
 * INPUTS    : UNSHORT : Error_code - Error code.
 *             int Line_nr - Line number where error was reported.
 *             char *Filename - Filename where error was reported.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Report_critical_error(UNSHORT Error_code, int Line_nr, char *Filename)
{
	/* Report error */
	MEM_Report_error(Error_code, Line_nr, Filename);

	/* Handler installed ? */
	if (MEM_Critical_error_handler)
	{
		/* Yes -> Call handler */
		(MEM_Critical_error_handler)();
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Print_error
 * FUNCTION  : Print a memory manager error.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.94 11:57
 * LAST      : 26.10.95 20:07
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by MEM_Error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : BBMEM.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Print_error(UNCHAR *buffer, UNBYTE *data)
{
	struct Memory_error *Report;
	UNSHORT Code;
	UNCHAR String[BBERROR_OUTSTRINGSIZE];

	/* Get error data */
	Report = (struct Memory_error *) data;
	Code = Report->Code;

	/* Catch illegal errors */
	if (Code > MEMERR_MAX)
		Code = 0;

	/* Line number / filename given ? */
	if (Report->Line_nr && Report->Filename)
	{
		/* Yes -> Build complete message with line/file info */
		_bprintf
		(
			String,
			BBERROR_OUTSTRINGSIZE,
			"%s\n(line %u of file %s)",
			MEM_Error_strings[Code],
			Report->Line_nr,
			Report->Filename
		);

		/* Copy message to output buffer */
		strncpy
		(
			buffer,
			String,
			BBERROR_OUTSTRINGSIZE - 1
		);
	}
	else
	{
		/* No -> Just copy error message to output buffer */
		strncpy
		(
			buffer,
			MEM_Error_strings[Code],
			BBERROR_OUTSTRINGSIZE - 1
		);
	}

	/* Insert EOL */
	*(buffer + BBERROR_OUTSTRINGSIZE - 1) = '\0';

	return;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Find_free_entry
 * FUNCTION  : Find a free memory entry.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 9:47
 * LAST      : 31.05.94 9:47
 * INPUTS    : None.
 * RESULT    : struct Memory_entry * : Pointer to a free memory entry
 *             (NULL = error).
 * BUGS      : No known.
 * NOTES     : - The memory handle and links of the new entry will be cleared.
 *             - A quick scan method is implemented. Whenever an entry
 *              is deleted, it's address is marked.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Memory_entry *
MEM_Find_free_entry(void)
{
	struct Memory_workspace *Current;
	struct Memory_entry *Free_entry = NULL;
	UNSHORT Sea_level = 0;
	UNSHORT i;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Is there a quick scan entry ? */
	if (MEM_Quick_free_entry)
	{
	 	/* Yes -> Get it */
		Free_entry = MEM_Quick_free_entry;

		/* Now there no longer is a quick scan entry */
		MEM_Quick_free_entry = NULL;

		/* Is this entry really free ? */
		if (Free_entry->Size)
			Free_entry = NULL;
	}

	/* Search for a free entry */
	while (!Free_entry)
	{
		/* Search memory entry list for a free entry */
		for (i=0;i<MEMORY_ENTRIES_MAX;i++)
		{
			/* Is this entry free ? */
			if (!MEM_Entries[i].Size)
			{
				/* Yes -> Found it */
				Free_entry = &MEM_Entries[i];
				break;
			}
		}

		/* Found a free entry ? */
		if (!Free_entry)
		{
	  		/* No free entry was found, so we'll have to try and make some. */
	  		/* Some persistent memory blocks must be destroyed */

			/* Drown memory in all areas in the current workspace */
			for (i=0;i<(Current->Nr_of_areas);i++)
				MEM_Drown_memory(&(Current->Areas[i]), Sea_level);

	  		/* Increase the sea level */
	  		Sea_level++;

			/* Has the maximum sea level been reached ? */
	  		if (Sea_level == 256)
			{
				/* Yes -> Error */
  				MEM_Critical_error(MEMERR_OUT_OF_ENTRIES);

				return NULL;
			}
		}
	}

	/* Prepare the new entry */
	Free_entry->Previous			= NULL;
	Free_entry->Next				= NULL;
	Free_entry->BLOCK_HANDLE	= NULL;

	return Free_entry;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Add_entry
 * FUNCTION  : Insert a memory entry in the memory lists.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 9:57
 * LAST      : 31.05.94 9:57
 * INPUTS    : struct Memory_entry *New - Pointer to new memory entry.
 *             struct Memory_entry *Target - Pointer to memory entry
 *              AFTER which the new entry will be inserted.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Target points to the entry AFTER which the new entry will be
 *               inserted, so it is possible to add entries to the start by
 *               having Target point to the area descriptor.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Add_entry(struct Memory_entry *New, struct Memory_entry *Target)
{
	struct Memory_entry *Next_entry;

	/* Get the next entry */
	Next_entry = Target->Next;

	/* Is there a next entry ? */
	if (Next_entry)
	{
		/* Yes -> Link it to the new entry */
		Next_entry->Previous = New;
	}

	/* Link the new entry to the next entry */
	New->Next = Next_entry;

	/* Link the new entry to the target entry */
	New->Previous = Target;

	/* Link the target entry to the new entry */
	Target->Next = New;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Delete_entry
 * FUNCTION  : Delete a memory entry from the memory lists.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 10:05
 * LAST      : 31.05.94 10:05
 * INPUTS    : struct Memory_entry *Target - Pointer to memory entry
 *              that must be deleted.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - A quick scan method is implemented. Whenever an entry is
 *               deleted, it's address is marked.
 *             - This function does not take care of any memory block logic.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Delete_entry(struct Memory_entry *Target)
{
	struct Memory_entry *Previous_entry;
	struct Memory_entry *Next_entry;

	/* Get the previous and next entries */
	Previous_entry	= Target->Previous;
	Next_entry		= Target->Next;

	/* Is there a next entry ? */
	if (Next_entry)
	{
		/* Yes -> Link the previous to the next entry */
		Next_entry->Previous = Previous_entry;
	}

	/* Link the next entry to the previous */
	Previous_entry->Next = Next_entry;

	/* Mark the deleted entry as free */
	Target->Size = 0;

	/* Store this as the quick scan entry */
	MEM_Quick_free_entry = Target;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Clear_all_handles
 * FUNCTION  : Clear all memory handles.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:00
 * LAST      : 31.05.94 16:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function should be called whenever the memory manager
 *              is initialized.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Clear_all_handles(void)
{
	struct Memory_entry *Cleared_entry;
	UNSHORT i;

	/* Check all handles */
	for (i=0;i<MEMORY_HANDLES_MAX;i++)
	{
		/* Get the entry belonging to this handle */
		Cleared_entry = MEM_Handles[i].Entry_ptr;

		/* Any entry ? */
		if (Cleared_entry)
		{
			/* Yes -> Clear link between handle and entry */
			Cleared_entry->BLOCK_HANDLE	= NULL;
			MEM_Handles[i].Entry_ptr		= NULL;
		}
	}
}

#if FALSE
/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Clear_all_claims
 * FUNCTION  : Clear all memory claims in the current workspace.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:02
 * LAST      : 31.05.94 16:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : - This function can severely mess up VMM locking! Not to
 *              mention having unlocked blocks used in interrupts!
 * NOTES     : - Call this function at certain strategic spots in the program
 *              to make sure no memory handles are claimed unnecessarily.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Clear_all_claims(void)
{
	struct Memory_workspace *Current;
	struct Memory_entry *Entry;
	MEM_HANDLE Handle;
	UNSHORT i;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Check all areas in the current workspace */
	for (i=0;i<(Current->Nr_of_areas);i++)
	{
		/* Check all entries in the current area */
		Entry = Current->Areas[i].Next;
		while (Entry)
		{
			/* Get the handle belonging to this entry */
			Handle = Entry->BLOCK_HANDLE;

			/* Any handle ? */
			if (Handle)
			{
				/* Yes -> Clear the claim counter */
				Handle->Claim_counter = 0;
			}

			/* Next entry */
			Entry = Entry->Next;
		}
	}
}
#endif

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Check_if_handle_is_legal
 * FUNCTION  : Check if a memory handle is legal.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 10:41
 * LAST      : 26.10.95 19:48
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 *             int Line_nr - Line number of caller.
 *             char *Filename - Filename of caller.
 * RESULT    : BOOLEAN : Handle is legal.
 *             int Line_nr - Line where function was called.
 *             char *Filename - Name of file where function was called.
 * BUGS      : No known.
 * NOTES     : - If the handle is NULL it will be considered illegal.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
MEM_Check_if_handle_is_legal(MEM_HANDLE Handle, int Line_nr, char *Filename)
{
	/* Check if handle lies within handle array */
	if (((UNBYTE *) Handle) < (UNBYTE *) &MEM_Handles[0])
	{
		MEM_Report_critical_error(MEMERR_ILLEGAL_HANDLE, Line_nr, Filename);
		return FALSE;
	}

	if (((UNBYTE *) Handle) >= (UNBYTE *) &MEM_Handles[MEMORY_HANDLES_MAX])
	{
		MEM_Report_critical_error(MEMERR_ILLEGAL_HANDLE, Line_nr, Filename);
		return FALSE;
	}

	return TRUE;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Get_pointer
 * FUNCTION  : Get an UNCLAIMED pointer to a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:02
 * LAST      : 16.09.95 18:28
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : UNBYTE * : Pointer to memory.
 * BUGS      : No known.
 * NOTES     : - Use this function at your peril : if a garbage collection
 *               occurs, this memory block may be moved without your notice!
 *               This function should only be used when speed is required.
 *               You have to know how the memory manager works!
 *             - This function MUST be able to deal with handle NULL.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
MEM_Get_pointer(MEM_HANDLE Handle)
{
	struct Memory_entry *Entry;

	/* Default is 0 !!! */
	UNBYTE *Ptr = 0;

	/* Is the handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Get the entry belonging to this handle */
			Entry = Handle->Entry_ptr;

			/* Any entry ? */
			if (Entry)
			{
				/* Yes -> Get pointer */
				Ptr = Entry->Start;
			}
			else
			{
				/* No -> Error */
				MEM_Critical_error(MEMERR_CLAIMED_FREE_HANDLE);
			}
		}
	}

	return Ptr;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Claim_pointer
 * FUNCTION  : Claim a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:14
 * LAST      : 16.09.95 18:28
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : UNBYTE * : Pointer to memory.
 * BUGS      : No known.
 * NOTES     : - This function MUST be able to deal with handle NULL.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
MEM_Claim_pointer(MEM_HANDLE Handle)
{
	struct Memory_entry *Entry;
	struct File_type *Ftype;

	/* Default is 0 !!! */
	UNBYTE *Ptr = 0;

	/* Is the handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK ->Increase the claim counter if possible */
			if (Handle->Claim_counter != 255)
				Handle->Claim_counter++;

			/* Get the entry belonging to this handle */
			Entry = Handle->Entry_ptr;

			/* Any entry ? */
			if (Entry)
			{
				/* Yes -> Get pointer */
				Ptr = Entry->Start;

				/* Does this block have a file type ? */
				if (Handle->File_type_ptr)
				{
					/* Yes -> Get file type */
					Ftype = Handle->File_type_ptr;

					/* Should this block be locked ? */
					if (Ftype->Flags & MEM_LOCK)
					{
						/* Yes -> Lock the block */
						BASEMEM_Lock_region(Ptr, Entry->Size);
					}
				}
			}
			else
			{
				/* No -> Error */
				MEM_Critical_error(MEMERR_CLAIMED_FREE_HANDLE);
			}
		}
	}

	return Ptr;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Free_pointer
 * FUNCTION  : Free a pointer to a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:19
 * LAST      : 31.08.95 10:44
 * INPUTS    : MEM_HANDLE Handle - memory handle.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function MUST be able to deal with handle NULL.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Free_pointer(MEM_HANDLE Handle)
{
	struct Memory_entry *Entry;
	struct File_type *Ftype;

	/* Is the handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Decrease the claim counter if possible */
			if (Handle->Claim_counter)
				Handle->Claim_counter--;

			/* Does this block have a file type ? */
			if (Handle->File_type_ptr)
			{
				/* Yes -> Get file type */
				Ftype = Handle->File_type_ptr;

				/* Should this block be unlocked ? */
				if (Ftype->Flags & MEM_LOCK)
				{
					/* Yes -> Get the entry belonging to this handle */
					Entry = Handle->Entry_ptr;

					/* Any entry ? */
					if (Entry)
					{
						/* Yes -> Unlock the block */
						BASEMEM_Unlock_region(Entry->Start, Entry->Size);
					}
				}
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Create_memory_handle
 * FUNCTION  : Create a new memory handle.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:27
 * LAST      : 31.05.94 16:27
 * INPUTS    : struct Memory_entry *New_entry - Pointer to memory entry.
 * RESULT    : MEM_HANDLE : Memory handle (NULL = error).
 * BUGS      : No known.
 * NOTES     : - The memory handle is automatically inserted in the memory
 *              entry.
 *					- The memory handle is cleared completely except for the
 *              pointer to the memory entry.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
MEM_Create_memory_handle(struct Memory_entry *New_entry)
{
	struct Memory_workspace *Current;
	MEM_HANDLE Free_handle = NULL;
	UNSHORT Sea_level = 0;
	UNSHORT i;

	/* Exit if the entry already has a handle */
	if (New_entry->BLOCK_HANDLE)
		return New_entry->BLOCK_HANDLE;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Search for a free handle */
	while (!Free_handle)
	{
		/* Search memory handle list for a free handle */
		for (i=0;i<MEMORY_HANDLES_MAX;i++)
		{
			/* Is this handle free ? */
			if (!MEM_Handles[i].Entry_ptr)
			{
				/* Yes -> Found a free handle */
				Free_handle = &MEM_Handles[i];
				break;
			}
		}

		/* Found a free handle ? */
		if (!Free_handle)
		{
		  	/* No free handle was found, so we'll have to try and make some. */
		  	/* Some persistent memory blocks must be destroyed */

			/* Drown memory in all areas in the current workspace */
			for (i=0;i<(Current->Nr_of_areas);i++)
				MEM_Drown_memory(&(Current->Areas[i]), Sea_level);

	  		/* Increase the sea level */
	  		Sea_level++;

			/* Has the maximum sea level been reached ? */
	  		if (Sea_level == 256)
			{
				/* Yes -> Error */
  				MEM_Error(MEMERR_OUT_OF_HANDLES);

				return NULL;
			}
		}
	}

	/* Clear memory handle */
	BASEMEM_FillMemByte((UNBYTE *) Free_handle,
	 sizeof(struct Memory_handle), 0);

	/* Link memory entry with memory handle */
	Free_handle->Entry_ptr	= New_entry;
	New_entry->BLOCK_HANDLE	= Free_handle;

	return Free_handle;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Destroy_memory_handle
 * FUNCTION  : Destroy a memory handle.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:34
 * LAST      : 12.11.95 23:31
 * INPUTS    : struct Memory_entry *Target - Pointer to memory entry. This
 *              entry's handle will be destroyed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will destroy the handle even if it is claimed.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Destroy_memory_handle(struct Memory_entry *Target)
{
	MEM_HANDLE Handle;

	/* Get the entry's handle */
	Handle = Target->BLOCK_HANDLE;

	/* Any handle ? */
	if (Handle)
	{
		/* Yes -> Remove reference to handle */
		Target->BLOCK_HANDLE = NULL;

		/* Clear memory handle */
		BASEMEM_FillMemByte
		(
			(UNBYTE *) Handle,
			sizeof(struct Memory_handle),
			0
		);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Get_block_size
 * FUNCTION  : Get the size of a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 10:09
 * LAST      : 31.08.95 10:45
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : UNLONG : Size of memory block.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
MEM_Get_block_size(MEM_HANDLE Handle)
{
	struct Memory_entry *Entry;
	UNLONG Size = 0;

	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes -> Get entry */
				Entry = Handle->Entry_ptr;

				/* Get original block size */
				Size = (Entry->Size & 0xFFFFFF00) | (Handle->Size_low_byte);
			}
		}
	}

	return Size;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Get_file_index
 * FUNCTION  : Get the file index of a memory handle.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.07.95 20:04
 * LAST      : 31.08.95 10:46
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : UNLONG : File index of memory handle.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
MEM_Get_file_index(MEM_HANDLE Handle)
{
	UNSHORT File_index = 0;

	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes -> Get file index */
				File_index = Handle->File_index;
			}
		}
	}

	return File_index;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Resize_memory
 * FUNCTION  : Change the size of a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 10:14
 * LAST      : 31.08.95 10:47
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 *             UNLONG : New size of memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Resize_memory(MEM_HANDLE Handle, UNLONG New_size)
{
	struct Memory_workspace *Current;
	struct Memory_entry *New_entry;
	struct Memory_entry *Entry;
	MEM_HANDLE New_handle;
	struct File_type *Ftype;

	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes -> Get current workspace */
				Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

				/* Get entry */
				Entry = Handle->Entry_ptr;

				/* Set new low byte of original size */
				Handle->Size_low_byte = (UNBYTE)(New_size & 0x000000FF);

				/* Align new size */
				New_size = BASEMEM_Align(New_size);

				/* Set minimum size if aligned size is zero */
				if (!New_size)
				{
					New_size = BASEMEM_ALIGNMENT;
				}

				/* Shrinking ? */
				if (Entry->Size > New_size)
				{
					/* Yes -> Split this memory block */
					MEM_Split_memory_block(Entry, New_size);

					/* Merge remaining block with neighbours */
					MEM_Merge_memory_block(Entry->Next);
				}

				/* Expanding ? */
				if (Entry->Size < New_size)
				{
					/* Yes -> Exit if memory movement is not allowed */
					if (Current->Flags & MEM_NO_MEMORY_MOVEMENT)
						return;

					/* Get file type */
					Ftype = Handle->File_type_ptr;

					/* Try to allocate a new memory block */
					New_handle = MEM_Do_allocate(New_size, 0, Ftype);

					/* Success ? */
					if (New_handle)
					{
						/* Yes -> Get original entry */
						Entry = Handle->Entry_ptr;

						/* Get new entry */
						New_entry = New_handle->Entry_ptr;

						/* Relocate */
						(Ftype->Relocator)(Handle, Entry->Start, New_entry->Start,
						 Entry->Size);

						/* Link the new handle to the old entry */
						New_handle->Entry_ptr	= Entry;
						Entry->BLOCK_HANDLE		= New_handle;

						/* Link the old handle to the new entry */
						Handle->Entry_ptr			= New_entry;
						New_entry->BLOCK_HANDLE	= Handle;

						/* Kill the old entry (with the new handle) */
						MEM_Kill_memory(New_handle);
					}
				}
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Free_memory
 * FUNCTION  : Free a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 10:26
 * LAST      : 26.10.95 19:21
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the memory block is no longer claimed, the handle will be
 *              destroyed.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Free_memory(MEM_HANDLE Handle)
{
	struct Memory_workspace *Current;
	UNBYTE Load_counter;

	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Get current workspace */
			Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

			/* Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes -> Is it unclaimed ? */
				if (!Handle->Claim_counter)
				{
					/* Yes -> Count down the load counter */
					Load_counter = Handle->Load_counter;
					if (Load_counter)
					{
						Load_counter--;
						Handle->Load_counter = Load_counter;
					}

					/* Is it still loaded ? */
					if (!Load_counter)
					{
			 			/* No -> Should it be killed ? */
						if ((Current->Flags & MEM_NO_PERSISTENCE) ||
						 (Handle->Flags & MEM_INVALID) ||
						 (Handle->File_type_ptr && (Handle->File_type_ptr->Flags &
						  MEM_KILL_ALWAYS)))
						{
							/* Yes -> Kill */
							MEM_Kill_memory(Handle);
						}
						else
						{
							/* No -> Mark it as un-allocated */
							Handle->Flags &= ~MEM_ALLOCATED;
						}
					}
				}
			}
			else
			{
				/* No -> Report error */
//				MEM_Error(MEMERR_FREED_FREE_HANDLE);
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Kill_memory
 * FUNCTION  : Kill a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 10:53
 * LAST      : 26.10.95 19:50
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - If the memory block is still claimed or loaded more than
 *              once, it will be invalidated.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Kill_memory(MEM_HANDLE Handle)
{
	struct Memory_entry *Entry;

	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes -> Get entry */
				Entry = Handle->Entry_ptr;

				/* Is the handle claimed or loaded more than once ? */
//				if ((Handle->Claim_counter) || (Handle->Load_counter > 1))
				if (Handle->Load_counter > 1)
				{
					/* Yes -> Invalidate this memory block */
					Handle->Flags |= MEM_INVALID;
				}
				else
				{
					/* No -> Destroy the handle and merge the block */
					MEM_Destroy_memory_handle(Entry);
					MEM_Merge_memory_block(Entry);
				}
			}
			else
			{
				/* No -> Report error */
//				MEM_Error(MEMERR_KILLED_FREE_HANDLE);
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Invalidate_memory
 * FUNCTION  : Invalidate a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 11:08
 * LAST      : 31.08.95 10:48
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Invalidate_memory(MEM_HANDLE Handle)
{
	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes */
				Handle->Flags |= MEM_INVALID;
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Clear_memory
 * FUNCTION  : Clear a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 11:10
 * LAST      : 31.08.95 10:48
 * INPUTS    : MEM_HANDLE Handle - Memory handle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Clear_memory(MEM_HANDLE Handle)
{
	struct Memory_entry *Entry;

	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Is the handle allocated ? */
			if (Handle->Flags & MEM_ALLOCATED)
			{
				/* Yes -> Get entry */
				Entry = Handle->Entry_ptr;

				/* Clear memory block */
				BASEMEM_FillMemByte(Entry->Start, Entry->Size, (UNBYTE) 0);
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Relocate
 * FUNCTION  : Copy a memory block from one location to another.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 16:20
 * LAST      : 31.08.95 10:49
 * INPUTS    : MEM_HANDLE Handle - Handle of memory block.
 *             UNBYTE *Source - Pointer to source.
 *             UNBYTE *Target - Pointer to target.
 *             UNLONG Size - Size of memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is the default relocator. This function should be used
 *              instead of a direct call to Copy_memory because some relocators
 *              may need the memory handle.
 *             - NEVER get the source and target from the handle !!!
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Relocate(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target, UNLONG Size)
{
	/* Is handle NULL ? */
	if (Handle)
	{
		/* No -> Check if handle is legal */
		if (MEM_Handle_is_legal(Handle))
		{
			/* OK -> Just copy */
			BASEMEM_CopyMem(Source, Target, Size);
		}
	}
}

#if FALSE
/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Add_workspace
 * FUNCTION  : Initializes a workspace and returns the workspace number.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 14:02
 * LAST      : 06.07.94 14:02
 * INPUTS    : UNLONG Size - Size of workspace.
 *             UNBYTE Memory_type - Memory type in the workspace.
 * RESULT    : struct Memory_workspace * : Pointer to workspace.
 * BUGS      : No known.
 * NOTES     : - The workspace will be initialized, but not pushed on the
 *              stack.
 *             - The memory for this new workspace will be extracted from the
 *              current workspace.
 *             - All workspaces apart from the first one will consist of one
 *              area with one memory type.
 *             - Workspaces are a normal memory block in their parent
 *              workspace, but cannot be relocated because it is impossible
 *              to determine whether any memory entries in the workspace are
 *              unrelocatable. Therefore the parent workspace is cleaned up
 *              before a new workspace is added.
 *             - If there isn't enough memory to make a new workspace, this
 *              function will return workspace number 0. This way, it is
 *              possible to continue working AND to detect an error.
 *             - The workspace structure will be filled with default values.
 *              Custom Out_of_memory handlers etc. can be inserted.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Memory_workspace *
MEM_Add_workspace (UNLONG Size, UNBYTE Memory_type)
{
	struct Memory_workspace *Current;
	struct Memory_workspace *New_workspace = NULL;
	struct Memory_entry *Area;
	struct Memory_entry *Entry;
	struct File_type *Ftype;
	MEM_HANDLE New;
	UNSHORT i;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Find a free workspace */
	for (i=1;i<MEMORY_WORKSPACES_MAX;i++)
	{
		/* Is this workspace free ? */
		if (!MEM_Workspaces[i].Nr_of_areas)
		{
			/* Yes -> Found it */
			New_workspace = &MEM_Workspaces[i];
			break;
		}
	}

	/* Found one ? */
	if (New_workspace)
	{
		/* Yes -> Initialize workspace */
		New_workspace->Parent_workspace = Current;

		/* Initialize workspace filetype */
		Ftype = &(New_workspace->Workspace_ftype);

		Ftype->Relocator		= NULL;
		Ftype->Memory_type	= Memory_type;
		Ftype->Flags			= 0;

		/* Clean up parent workspace */
		MEM_Armageddon();

		/* Is enough memory available ? */
		if (MEM_Inquire_memory(Size ,Ftype->Memory_type))
		{
			/* Yes -> Get memory */
			New = MEM_Do_allocate(Size, (UNLONG) i, Ftype);
			New_workspace->Workspace_handle = New;

			/* Initialize area */
			Area = &(New_workspace->Areas[0]);
			New_workspace->Nr_of_areas = 1;

			Entry = New->Entry_ptr;

			Area->Start					= Entry->Start;
			Area->Size					= Entry->Size;

			Area->Previous				= NULL;
			Area->Next					= NULL;

			Area->WORKSPACE_NUMBER	= i;
			Area->MEMORY_TYPE			= Memory_type;

			/* Initialize workspace */
			MEM_Init_workspace(New_workspace);
		}
		else
		{
			/* No -> Refer to initial workspace */
			New_workspace = &MEM_Workspaces[0];
		}
	}
	else
	{
		/* No -> Error */
		MEM_Error(MEMERR_OUT_OF_WORKSPACES);
		New_workspace = NULL;
	}

	return New_workspace;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Delete_workspace
 * FUNCTION  : Terminates a workspace.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 15:02
 * LAST      : 06.07.94 15:02
 * INPUTS    : struct Memory_workspace *Workspace - Pointer to workspace.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The initial workspace cannot be deleted.
 *             - All child workspaces will be deleted as well.
 *             - All references to this workspace currently on the stack will
 *              be replaced by a reference to the parent workspace.
 *             - The workspace memory will be returned to the parent workspace.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Delete_workspace (struct Memory_workspace *Workspace)
{
	struct Memory_workspace *Parent;
	UNSHORT i;

	/* Is this the initial workspace ? */
	if (Workspace->Parent_workspace)
	{
		/* No -> Get workspace data */
		Parent = Workspace->Parent_workspace;

		/* Is this an occupied workspace ? */
		if (Workspace->Nr_of_areas)
		{
			/* Delete all child workspaces */
			for(i=1;i<MEMORY_WORKSPACES_MAX;i++)
			{
				if ((MEM_Workspaces[i].Nr_of_areas) &&
				 (MEM_Workspaces[i].Parent_workspace == Workspace))
					MEM_Delete_workspace(&MEM_Workspaces[i]);
			}

			/* Replace all references currently on the stack with parent */
			for (i=0;i<MEM_Workspace_stack_index;i++)
			{
				if (MEM_Workspace_stack[i] == Workspace)
					MEM_Workspace_stack[i] = Parent;
			}

			/* Return workspace memory */
			MEM_Push_workspace(Parent);
			MEM_Kill_memory(Workspace->Workspace_handle);
			MEM_Pop_workspace();

			/* Delete workspace */
			Workspace->Nr_of_areas = 0;
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Push_workspace
 * FUNCTION  : Pushes a new workspace on the workspace stack.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 15:24
 * LAST      : 06.07.94 15:24
 * INPUTS    : struct Memory_workspace *New - Pointer to workspace.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Parameter NULL will push the initial workspace.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Push_workspace (struct Memory_workspace *New)
{
	/* Initial workspace ? */
	if (!New)
		New = &MEM_Workspaces[0];

	/* Is there room on the stack ? */
	if (MEM_Workspace_stack_index < WORKSPACE_STACK_MAX - 1)
	{
		/* Yes -> Increase stack index */
		MEM_Workspace_stack_index++;

		/* Put new workspace on stack */
		MEM_Workspace_stack[MEM_Workspace_stack_index] = New;
	}
	else
	{
		/* No -> Error */
		MEM_Error(MEMERR_WORKSPACE_STACK_OVERFLOW);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Pop_workspace
 * FUNCTION  : Pops a workspace from the top of the workspace stack.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 16:35
 * LAST      : 06.07.94 16:35
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Pop_workspace (void)
{
	/* Is the stack empty ? */
	if (MEM_Workspace_stack_index)
	{
		/* No -> Decrease stack index */
		MEM_Workspace_stack_index--;
	}
	else
	{
		/* Yes -> Error */
		MEM_Error(MEMERR_WORKSPACE_STACK_UNDERFLOW);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Change_workspace
 * FUNCTION  : Change the workspace on the top of the workspace stack.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 16:36
 * LAST      : 06.07.94 16:36
 * INPUTS    : struct Memory_workspace *New - Pointer to workspace.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Change_workspace (struct Memory_workspace *New)
{
	MEM_Workspace_stack[MEM_Workspace_stack_index] = New;
}
#endif

#if FALSE
/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Init_memory
 * FUNCTION  : Initializes the memory manager and the first workspace.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 15.08.95 13:11
 * INPUTS    : UNSHORT Nr_requests - Amount of memory requests in list.
 *             struct Memory_request *Request_list - Pointer to a list of
 *              memory requests.
 * RESULT    : TRUE - Success.
 *             FALSE - Failure.
 * BUGS      : No known.
 * NOTES     : - This function will check whether the memory manager has
 *              already been initialized.
 *             - MEM_Reset_memory will be called by this function.
 *             - This function will store the unaligned pointers to the
 *              allocated memory block, which will be used by MEM_Exit_memory.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
MEM_Init_memory(UNSHORT Nr_requests, struct Memory_request *Request_list)
{
	struct Memory_workspace *Workspace;
	struct Memory_entry *Area;
	UNLONG Grabmin;
	UNLONG Grabmax;
	UNLONG Left;
	SILONG New_size;
	UNLONG Size;
	UNLONG Start;
	UNLONG End;
	UNSHORT i;
	UNSHORT Total_area_counter;
	UNSHORT Current_area_counter;

   /* Initialize the first workspace */
	Workspace = &MEM_Workspaces[0];

	/* If not already initialized */
	if (!MEM_Initialized)
	{
		/* Go through the memory requests */
		Total_area_counter = 0;
		for (i=0;i<Nr_requests;i++)
		{
			/* Clear counter */
			Current_area_counter = 0;

			/* Start grabbing */
			Grabmin = 0;
			Grabmax = 0;

			while (Total_area_counter < MEMORY_AREAS_MAX)
			{
				Area = &(Workspace->Areas[Total_area_counter]);

				/* Get the largest block */
				Size = BASEMEM_Get_largest_block(Request_list->Base_memory_type);
//				Size = BASEMEM_Get_largest_real_block(Request_list->Base_memory_type);

				/* Too small ? */
				if (Size < SMALL_FISH)
					break;

				/* Too large ? */
				if (Grabmax + Size > Request_list->Maximum)
				{
					/* Yes -> Clip */
					Size = Request_list->Maximum - Grabmax;

					/* Remainder too small ? */
					if (Size < SMALL_FISH)
						break;
				}

				/* No -> Allocate it */
				Start = (UNLONG) BASEMEM_Alloc(Size, Request_list->Base_memory_type);

				/* Success ? */
				if (!Start)
				{
					/* No -> Break */
					break;
				}

				/* Store original start and size */
				Grabbed_memory[Total_area_counter].Start	= (UNBYTE *) Start;
				Grabbed_memory[Total_area_counter].Size	= Size;

				/* Align start & size */
				End = Start + Size;

				if (End != BASEMEM_Align(End))
					End = BASEMEM_Align(End - BASEMEM_ALIGNMENT);

				Start = BASEMEM_Align(Start);

				/* Initialize area */
				Area->Start					= (UNBYTE *) Start;
				Area->Size					= End - Start;

				Area->Previous				= NULL;
				Area->Next					= NULL;
				Area->WORKSPACE_NUMBER	= 0;
				Area->MEMORY_TYPE			= Request_list->Memory_type;

				/* Add up */
				Grabmin += Area->Size;
				Grabmax += Size;

				/* Next area */
				Current_area_counter++;
				Total_area_counter++;
			}

 			/* How much memory is left ? */
			Left = BASEMEM_Get_free_memory(Request_list->Base_memory_type);

			/* Is enough / any areas to give back ? */
			while ((Left < Request_list->Safety) && (Current_area_counter > 0))
			{
				/* No -> Give some back */
				Area = &(Workspace->Areas[Total_area_counter - 1]);

				/* Calculate new size */
				New_size = Area->Size - (Request_list->Safety - Left);

				/* Give back the entire area ? */
				if ((New_size <= 0) || ((New_size > 0) &&
				 (New_size < SMALL_FISH)))
				{
					/* Yes -> Free memory */
					BASEMEM_Free(Grabbed_memory[Total_area_counter - 1].Start);

					/* Subtract size */
					Grabmin -= Grabbed_memory[Total_area_counter - 1].Size;
					Grabmax -= Area->Size;

					/* Clear area data */
					Area->Size = 0;

					/* Previous area */
					Total_area_counter--;
					Current_area_counter--;

					/* How much memory is left ? */
					Left = BASEMEM_Get_free_memory(Request_list->Base_memory_type);
				}
				else
				{
					/* No -> Shrink area */
					Start = (UNLONG) BASEMEM_Resize(Grabbed_memory[Total_area_counter - 1].Start,
					 New_size);

					/* Success ? */
					if (!Start)
					{
						/* No -> Break */
						break;
					}

					/* Subtract original size */
					Grabmin -= Area->Size;
					Grabmax -= Grabbed_memory[Total_area_counter - 1].Size;

					/* Store new start and size */
					Grabbed_memory[Total_area_counter].Start = (UNBYTE *) Start;
					Grabbed_memory[Total_area_counter].Size = New_size;

					/* Align start & size */
					End = Start + New_size;

					if (End != BASEMEM_Align(End))
						End = BASEMEM_Align(End - BASEMEM_ALIGNMENT);

					Start = BASEMEM_Align(Start);

					/* Re-initialize area */
					Area->Start = (UNBYTE *) Start;
					Area->Size = End - Start;

					/* Add up */
					Grabmin += Area->Size;
					Grabmax += New_size;

					/* How much memory is left ? */
					Left = BASEMEM_Get_free_memory(Request_list->Base_memory_type);
				}
			}

			/* Has the minimum amount been grabbed ? */
			if (Grabmin < Request_list->Minimum)
			{
				/* No -> Request denied */
				/* Did we run out of areas or what ? */
				if (Total_area_counter == MEMORY_AREAS_MAX)
				{
					/* Yes -> Memory was too fragmented */
					MEM_Error(MEMERR_OUT_OF_AREAS);
				}
				else
				{
					/* No -> There just isn't enough */
					MEM_Error(MEMERR_REQUEST_DENIED);
				}

				/* Exit */
				return FALSE;
			}

			/* Next request */
			Request_list++;
		}

		/* Any areas ? */
		if (!Total_area_counter)
		{
			/* No -> Odd request */
			MEM_Error(MEMERR_ODD_REQUEST);
			return FALSE;
		}

		/* Initialize workspace */
		Workspace->Nr_of_areas = Total_area_counter;
		Workspace->Parent_workspace = 0;

		/* Memory has been initialized */
		MEM_Initialized = TRUE;
	}

	/* Reset workspace stack */
	MEM_Workspace_stack_index = 0;
	MEM_Workspace_stack[MEM_Workspace_stack_index] = Workspace;

	/* Reset memory manager */
	MEM_Reset_memory();

	return TRUE;
}
#endif

