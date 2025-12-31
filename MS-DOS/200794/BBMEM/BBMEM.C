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

#include <stdio.h>
#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>
#include <TEST.H>
#include "BBMEMVAR.H"

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Init_memory
 * FUNCTION  : Initializes the memory manager and the first workspace.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 06.06.94 12:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will check whether the memory manager has
 *              already been initialized.
 *             - MEM_Reset_memory will be called by this function.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Init_memory(void)
{
	struct Memory_workspace *Workspace;
	struct Memory_entry *Area;
	UNLONG Size, Start, End;
	UNSHORT Area_counter = 0;

	if (MEM_Initialized == FALSE)					/* If not already initialized */
	{
		Workspace = &MEM_Workspaces[0];      	/* Initialize the first workspace */
		while (Area_counter<MEMORY_AREAS_MAX)
		{
			Area = &(Workspace->Areas[Area_counter]);
			BASE_Get_memory(Area);    	  			/* Get memory */
			Size = Area->Size;
			if (!Size)		             			/* Exit if no memory was allocated */
				break;

			Start = (UNLONG) Area->Start;			/* Align start & size */
 			End = Start + Size;
			if (End != MEM_Align(End))
				End = MEM_Align(End - ALIGNMENT);
			Start = MEM_Align(Start);

			Area->Start = (UNBYTE *) Start;		/* Store */
			Area->Size = End - Start;

			Area->Previous = NULL;        	   /* Initialize memory area */
			Area->Next = NULL;
			Area->WORKSPACE_NUMBER = 0;

			Area_counter++;							/* Next area */
		}

		if (!Area_counter)							/* Any luck ? */
			MEM_Error(MEMERR_NO_AREAS);

		Workspace->Nr_of_areas = Area_counter;	/* Set number of areas */

		Workspace->Parent_workspace = 0;

		MEM_Debugging2 = MEM_Debugging;			/* Copy flag */

		MEM_Initialized = TRUE; 					/* Memory has been initialized */
	}

	/* Reset workspace stack */
	MEM_Workspace_stack_index = 0;
	MEM_Workspace_stack[MEM_Workspace_stack_index] = Workspace;

	MEM_Reset_memory();								/* Reset memory manager */
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Exit_memory
 * FUNCTION  : Exits the memory manager. All memory is returned to the OS.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.06.94 12:02
 * LAST      : 06.06.94 12:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will check whether the memory manager has
 *              already been initialized.
 *             - MEM_Reset_memory will be called to make sure that all
 *              handles etc. will become illegal.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Exit_memory(void)
{
 	struct Memory_workspace *Workspace;
	struct Memory_entry *Area;
	UNSHORT i;

	if (MEM_Initialized == TRUE)					/* If not already initialized */
	{
		MEM_Reset_memory();							/* To make old handles invalid */

		Workspace = &MEM_Workspaces[0];      	/* Destroy the first workspace */

		for(i=0;i<Workspace->Nr_of_areas;i++)
		{
			Area = &(Workspace->Areas[i]);
			BASE_Free_memory(Area);    	  	  	/* Free memory */
			Area->Size = 0; 			       	   /* Clear memory area data */
		}
		MEM_Initialized = FALSE;					/* Memory is no longer initialized */
	}
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
	UNSHORT i;

	for(i=1;i<MEMORY_WORKSPACES_MAX;i++) 		/* Delete all workspaces */
	{														/* except the first one */
		if (MEM_Workspaces[i].Nr_of_areas)
			MEM_Delete_workspace(&MEM_Workspaces[i]);
	}

	MEM_Init_workspace(&MEM_Workspaces[0]);	/* Initialize workspace #0 */
	MEM_Clear_all_handles();						/* Just to be sure */
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
	Workspace->Alloc_init = NULL;
	Workspace->Alloc_exit = NULL;
	Workspace->Out_of_memory = MEM_Default_out_of_memory;
	Workspace->Pass_list = MEM_Default_pass_list;

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
		MEM_Init_area(&(Workspace->Areas[i]));
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
 * LAST      : 06.06.94 12:02
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
	struct Memory_entry *Entry, *Next;

	/* Delete chain of memory entries */
	Entry = Area->Next;						/* Get first entry */
	while (Entry)								/* Until the chain ends... */
	{
		Next = Entry->Next;					/* Get next memory entry */
		if (Entry->BLOCK_HANDLE)			/* If this entry has a handle... */
			MEM_Destroy_memory_handle(Entry);	/* Destroy it */
		MEM_Delete_entry(Entry);			/* Delete the memory entry */
		Entry = Next;							/* Next entry */
	}

	/* Create first entry */
	Entry = MEM_Find_free_entry(); 		/* Find free entry */
	Entry->Start = Area->Start;			/* Initialize */
	Entry->Size = Area->Size;
	MEM_Add_entry(Entry,Area);				/* Add to chain */

	/* Fill with debugging fill pattern */
	if (MEM_Debugging2)
		MEM_Fill_memory_long(Entry->Start,Entry->Size,MEM_FILLER_1);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Error
 * FUNCTION  : Report a memory manager error.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.94 11:49
 * LAST      : 03.06.94 11:49
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Error(UNSHORT Error_code)
{
	struct Memory_error Error;
	char X[200];

	Error.Code = Error_code;			/* Initialize memory error structure */

	/* Push error on the error stack */
/*	ERROR_PushError(MEM_Print_error,MEM_Library_name,sizeof(struct Memory_error),(UNBYTE *) &Error);
*/

	MEM_Print_error(&X[0],(UNBYTE *) &Error);
	printf("ERROR : %s\n",X);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Print_error
 * FUNCTION  : Print a memory manager error.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.06.94 11:57
 * LAST      : 06.06.94 15:18
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
	struct Memory_error *Error;
	UNSHORT i;

	Error = (struct Memory_error *) data;
	i = Error->Code;  											/* Get error code */

	if (i>MEMERR_MAX)												/* Catch illegal errors */
		i = 0;

	sprintf((char *)buffer,"%s",MEM_Error_strings[i]);	/* Print error */
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Default_out_of_memory
 * FUNCTION  : Default out of memory handler.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 16:31
 * LAST      : 10.06.94 16:31
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Default_out_of_memory(void)
{
	MEM_Error(MEMERR_OUT_OF_MEMORY);
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
	struct Memory_entry *Free_entry = NULL, *Area;
	UNSHORT Sea_level = 0, i;

	/* Quick scan */
	if (MEM_Quick_free_entry)				 			/* Is there a quick scan entry ? */
	{
		Free_entry = MEM_Quick_free_entry;	 		/* Yes -> Get it */
		MEM_Quick_free_entry = NULL;
		if (Free_entry->Size) Free_entry = NULL; 	/* Is it really free ? */
	}

	/* Search for a free entry */
	while (!Free_entry)
	{
		/* Search memory entry list for a free entry */
		for (i=0;i<MEMORY_ENTRIES_MAX;i++)
		{
			if (!MEM_Entries[i].Size)
			{
				Free_entry = &MEM_Entries[i];
				break;
			}
		}

		if (!Free_entry)
		{
	  		/* No free entry was found, so we'll have to try and make some. */
	  		/* Some persistent memory blocks must be destroyed */
	  		Current = MEM_Workspace_stack[MEM_Workspace_stack_index];
			Area = &(Current->Areas[0]);
			for (i=0;i<(Current->Nr_of_areas);i++)
				MEM_Drown_memory(&Area[i],Sea_level);

	  		/* Increase the sea level and exit if the maximum is reached */
	  		Sea_level++;
	  		if (Sea_level == 256)
			{
  				MEM_Error(MEMERR_OUT_OF_ENTRIES);
				return(NULL);
			}
		}
	}

	/* Prepare block and exit */
	Free_entry->Previous = NULL;
	Free_entry->Next = NULL;
	Free_entry->BLOCK_HANDLE = NULL;

	return(Free_entry);
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

	Next_entry = Target->Next;		  					/* Get next entry */
	if (Next_entry)                              /* If there is a next... */
		Next_entry->Previous = New;					/* Link new to next */
	New->Next = Next_entry;								/* Link next to new */
	New->Previous = Target;								/* Link previous to new */
	Target->Next = New;                         	/* Link new to previous */
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
	struct Memory_entry *Previous_entry,*Next_entry;

	Previous_entry = Target->Previous;		/* Get next & previous */
	Next_entry = Target->Next;

	if (Next_entry)                    		/* Link previous to next */
		Next_entry->Previous = Previous_entry;
	Previous_entry->Next = Next_entry;

	Target->Size = 0;								/* Clear entry */
	MEM_Quick_free_entry = Target;			/* Store */
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

	for (i=0;i<MEMORY_HANDLES_MAX;i++) 				/* For all handles */
	{
		Cleared_entry = MEM_Handles[i].Entry_ptr;	/* Get entry */
		if (Cleared_entry)								/* If any */
		{
			Cleared_entry->BLOCK_HANDLE = NULL;    /* Clear handle pointer */
			MEM_Handles[i].Entry_ptr = NULL;			/* Clear entry pointer */
		}
	}
}

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
 * BUGS      : No known.
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
	struct Memory_entry *Entry, *Area;
	MEM_HANDLE Handle;
	UNSHORT i;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Do all areas in the current workspace */
	Area = &(Current->Areas[0]);
	for (i=0;i<(Current->Nr_of_areas);i++)
	{
		Entry = Area[i].Next;      				/* Start with the first entry */
		while (Entry)                    		/* Do the entire chain */
		{
			Handle = Entry->BLOCK_HANDLE;			/* Get the handle pointer */
			if (Handle)       						/* If this block has a handle */
				Handle->Claim_counter = 0;			/* Clear the claim counter */
			Entry = Entry->Next;      				/* Next entry in the chain */
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Get_pointer
 * FUNCTION  : Get an UNCLAIMED pointer to a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:02
 * LAST      : 31.05.94 16:02
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
	UNBYTE *Ptr = 0;							/* Default is 0 !!! */

	if (Handle)
	{
		Ptr = Handle->Entry_ptr->Start;	/* Get pointer */
		if (MEM_Debugging2)					/* Skip debugging fill pattern */
			Ptr += 16;
	}
	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Claim_pointer
 * FUNCTION  : Claim a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:14
 * LAST      : 31.05.94 16:14
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
	UNBYTE *Ptr = 0;								/* Default is 0 !!! */

	if (Handle)
	{
		if (Handle->Claim_counter != 255)	/* If possible, increase */
			Handle->Claim_counter++;         /* the claim counter */

		Ptr = Handle->Entry_ptr->Start;		/* Get pointer */
		if (MEM_Debugging2)						/* Skip debugging fill pattern */
			Ptr += 16;
	}
	return(Ptr);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Free_pointer
 * FUNCTION  : Free a pointer to a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:19
 * LAST      : 31.05.94 16:19
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
	if (Handle)
		if (Handle->Claim_counter)		/* If possible, decrease */
			Handle->Claim_counter--;	/* the claim counter */
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
	struct Memory_entry *Area;
	MEM_HANDLE Free_handle = NULL;
	UNSHORT i, Sea_level = 0;

	/* Exit if the entry already has a handle */
	if (New_entry->BLOCK_HANDLE)
		return(New_entry->BLOCK_HANDLE);
	else
	{
		/* Search for a free handle */
		while (!Free_handle)
		{
			/* Search memory handle list for a free handle */
			for (i=0;i<MEMORY_HANDLES_MAX;i++)
			{
				if (!MEM_Handles[i].Entry_ptr)
				{
					Free_handle = &MEM_Handles[i];
					break;
				}
			}

			if (!Free_handle)
			{
		  		/* No free handle was found, so we'll have to try and make some. */
		  		/* Some persistent memory blocks must be destroyed */
		  		Current = MEM_Workspace_stack[MEM_Workspace_stack_index];
				Area = &(Current->Areas[0]);
				for (i=0;i<(Current->Nr_of_areas);i++)
					MEM_Drown_memory(&Area[i],Sea_level);

		  		/* Increase the sea level and exit if the maximum is reached */
		  		Sea_level++;
		  		if (Sea_level == 256)
				{
	  				MEM_Error(MEMERR_OUT_OF_HANDLES);
					return(NULL);
				}
			}
		}

		/* Clear memory handle */
		MEM_Fill_memory((UNBYTE *) Free_handle,sizeof(struct Memory_handle),0);

		/* Link memory entry with memory handle */
		Free_handle->Entry_ptr = New_entry;
		New_entry->BLOCK_HANDLE = Free_handle;

		return(Free_handle);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Destroy_memory_handle
 * FUNCTION  : Destroy a memory handle.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.05.94 16:34
 * LAST      : 31.05.94 16:34
 * INPUTS    : struct Memory_entry *Target - Pointer to memory entry. This
 *              entry's handle will be destroyed.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Destroy_memory_handle(struct Memory_entry *Target)
{
	MEM_HANDLE Handle;

	Handle = Target->BLOCK_HANDLE;  					/* Get handle */
	if (Handle)                       				/* If it isn't zero... */
	{
		if (!Handle->Claim_counter)  					/* If not claimed... */
		{
			Target->BLOCK_HANDLE = NULL; 				/* Clear handle */
			Handle->Entry_ptr = NULL;					/* Free memory handle */
		}
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
 * LAST      : 10.06.94 10:09
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
	UNLONG Size;

	/* Get entry */
	Entry = Handle->Entry_ptr;

	/* Get original block size */
	Size = (Entry->Size & 0xFFFFFF00) | (Handle->Size_low_byte);

	/* Compensate debugging buffers */
	if (MEM_Debugging2)
		Size -= 32;

	return(Size);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Resize_memory
 * FUNCTION  : Change the size of a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 10:14
 * LAST      : 10.06.94 10:14
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
	struct Memory_entry *Entry, *New_entry;
	MEM_HANDLE New_handle;
	struct File_type *Ftype;

	/* Get entry */
	Entry = Handle->Entry_ptr;

	/* Set new low byte of original size */
	Handle->Size_low_byte = (UNBYTE)(New_size & 0x000000FF);

	/* Align new size */
	New_size = MEM_Align(New_size);

	/* If shrinking... */
	if (Entry->Size > New_size)
	{
		MEM_Split_memory_block(Entry,New_size);	/* Split this memory block */
		MEM_Merge_memory_block(Entry->Next);		/* Merge remaining block */
	}

	/* If expanding... */
	if (Entry->Size < New_size)
	{
		/* Is memory movement allowed ? */
		Current = MEM_Workspace_stack[MEM_Workspace_stack_index];
		if (Current->Flags & MEM_NO_MEMORY_MOVEMENT)
			return;

		/* Yes */
		Ftype = Handle->File_type_ptr;

		/* Allocate a new memory block */
		New_handle = MEM_Do_allocate(New_size,0,Ftype);

		/* Get original entry */
		Entry = Handle->Entry_ptr;

		/* Get new entry */
		New_entry = New_handle->Entry_ptr;

		/* Copy current contents of block */
		(Ftype->Relocator)(Handle,Entry->Start,New_entry->Start,Entry->Size);

		/* Link the new handle to the old entry */
		New_handle->Entry_ptr = Entry;
		Entry->BLOCK_HANDLE = New_handle;

		/* Link the old handle to the new entry */
		Handle->Entry_ptr = New_entry;
		New_entry->BLOCK_HANDLE = Handle;

		/* Kill the old entry (with the new handle) */
		MEM_Kill_memory(New_handle);
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
 * LAST      : 10.06.94 10:26
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
	struct Memory_entry *Entry;
	UNBYTE C;

	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Get entry */
	Entry = Handle->Entry_ptr;

	/* Is it allocated and unclaimed ? */
	if ((Handle->Flags & MEM_ALLOCATED) || !(Handle->Claim_counter))
	{
		/* Yes -> Count down the load counter */
		C = Handle->Load_counter;
		if (C)
		{
			C--;
			Handle->Load_counter = C;
		}
		/* Is it still loaded ? */
		if (!C)
		{
 			/* No -> Should it be killed ? */
			if ((Current->Flags & MEM_NO_PERSISTENCE) ||
			    (Handle->Flags & MEM_INVALID) ||
				 (Handle->File_type_ptr->Flags & MEM_KILL_ALWAYS))
				/* Yes -> Kill */
				MEM_Kill_memory(Handle);
			else
				/* No -> Mark it as un-allocated */
				Handle->Flags &= ~MEM_ALLOCATED;
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
 * LAST      : 10.06.94 10:53
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

	/* Get entry */
	Entry = Handle->Entry_ptr;

	/* If the handle is claimed or loaded more than once... */
	if ((Handle->Claim_counter) || (Handle->Load_counter))
		/* Invalidate this memory block */
		Handle->Flags |= MEM_INVALID;
	else
	{
		/* Else destroy the handle and merge the block */
		MEM_Destroy_memory_handle(Entry);
		MEM_Merge_memory_block(Entry);
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
 * LAST      : 10.06.94 11:08
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
	Handle->Flags |= MEM_INVALID;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Clear_memory
 * FUNCTION  : Clear a memory block.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 11:10
 * LAST      : 10.06.94 11:10
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

	/* Get entry */
	Entry = Handle->Entry_ptr;

	/* Clear memory block */
	MEM_Fill_memory(Entry->Start,Entry->Size,(UNBYTE) 0);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Relocate
 * FUNCTION  : Copy a memory block from one location to another.
 * FILE      : BBMEM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.06.94 16:20
 * LAST      : 10.06.94 16:20
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
	XXX_Copy_memory(Source,Target,Size);
}

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

struct Memory_workspace*
MEM_Add_workspace (UNLONG Size, UNBYTE Memory_type)
{
	struct Memory_workspace *Current,*New_workspace = NULL;
	struct Memory_entry *Entry,*Area;
	struct File_type *Ftype;
	MEM_HANDLE New;
	UNSHORT i;

	/* Get current workspace */
	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	/* Find a free workspace */
	for (i=1;i<MEMORY_WORKSPACES_MAX;i++)
	{
		if (!MEM_Workspaces[i].Nr_of_areas)
		{
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
		Ftype->Relocator = NULL;
		Ftype->Memory_type = Memory_type;
		Ftype->Flags = 0;

		/* Clean up parent workspace */
		MEM_Armageddon();

		/* Is enough memory available ? */
		if (MEM_Inquire_memory(Size,Ftype->Memory_type))
		{
			/* Yes -> Get memory */
			New = MEM_Do_allocate(Size,(UNLONG) i,Ftype);
			New_workspace->Workspace_handle = New;

			/* Initialize area */
			Area = &(New_workspace->Areas[0]);
			New_workspace->Nr_of_areas = 1;

			Entry = New->Entry_ptr;
			Area->Start = Entry->Start;
			Area->Size = Entry->Size;

			Area->Previous = NULL;
			Area->Next  =NULL;

			Area->WORKSPACE_NUMBER = i;
			Area->MEMORY_TYPE = Memory_type;

			/* Initialize workspace */
			MEM_Init_workspace(New_workspace);
		}
		else
			/* No -> Refer to initial workspace */
			New_workspace = &MEM_Workspaces[0];
	}
	else
	{
		/* No -> Error */
		MEM_Error(MEMERR_OUT_OF_WORKSPACES);
		New_workspace = NULL;
	}

	return(New_workspace);
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
				if ((MEM_Workspaces[i].Nr_of_areas) && (MEM_Workspaces[i].Parent_workspace == Workspace))
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
		/* No -> Error */
		MEM_Error(MEMERR_WORKSPACE_STACK_OVERFLOW);
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
		/* No -> Decrease stack index */
		MEM_Workspace_stack_index--;
	else
		/* Yes -> Error */
		MEM_Error(MEMERR_WORKSPACE_STACK_UNDERFLOW);
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

