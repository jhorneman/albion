/************
 * NAME     : BBMEMCHK.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 9-6-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : BBMEM.C, BBMEM.H
 ************/

/* includes */

#include <BBDEF.H>

#include <BBERROR.H>
#include <BBMEM.H>
#include "INCLUDE\BBMEMVAR.H"

/* prototypes */

BOOLEAN MEM_Check_workspace(struct Memory_workspace *Workspace);

BOOLEAN MEM_Check_area(struct Memory_entry *Area);

#if FALSE
struct Memory_workspace *MEM_Add_workspace (UNLONG Size, UNBYTE Memory_type);
void MEM_Delete_workspace (struct Memory_workspace *New);

void MEM_Push_workspace (struct Memory_workspace *New);
void MEM_Pop_workspace (void);
void MEM_Change_workspace (struct Memory_workspace *New);
#endif

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Check_memory
 * FUNCTION  : Check the internal consistency of the memory manager.
 * FILE      : BBMEMCHK.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.06.94 18:43
 * LAST      : 19.08.95 20:21
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Result.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
MEM_Check_memory(void)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Check all workspaces */
	for(i=0;i<MEMORY_WORKSPACES_MAX;i++)
	{
		/* Does this workspace have any areas ? */
		if (MEM_Workspaces[i].Nr_of_areas)
		{
			/* Yes -> Check it */
			Result = MEM_Check_workspace(&MEM_Workspaces[i]);

			if (!Result)
				break;
		}
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Check_workspace
 * FUNCTION  : Check the internal consistency of a memory workspace .
 * FILE      : BBMEMCHK.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.06.94 18:43
 * LAST      : 19.08.95 20:21
 * INPUTS    : struct Memory_workspace *Workspace - Pointer to workspace.
 * RESULT    : BOOLEAN : Result.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
MEM_Check_workspace(struct Memory_workspace *Workspace)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Check all areas in this workspace */
	for(i=0;i<Workspace->Nr_of_areas;i++)
	{
		Result = MEM_Check_area(&(Workspace->Areas[i]));

		if (!Result)
			break;
	}

	return Result;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_Check_area
 * FUNCTION  : Check the internal consistency of a memory area.
 * FILE      : BBMEMCHK.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.06.94 18:43
 * LAST      : 19.08.95 20:19
 * INPUTS    : struct Memory_entry *Area - Pointer to memory area.
 * RESULT    : BOOLEAN : Result.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
MEM_Check_area(struct Memory_entry *Area)
{
	struct Memory_entry *Entry;
	struct Memory_entry *Previous_entry;
	MEM_HANDLE Handle;
	BOOLEAN Result = TRUE;
	SILONG Total_size;
	UNBYTE *Area_end;
	UNBYTE *End_of_last_block;
	UNBYTE *Start;
	UNBYTE *End;

	Total_size = (SILONG) Area->Size;

	Area_end = Area->Start + (UNLONG) Total_size;

	End_of_last_block = Area->Start;

	/* Search all entries in this area */
	Previous_entry = Area;
	Entry = Area->Next;
	while (Entry)
	{
		/* Check if entry lies within array */
		if (((UNBYTE *) Entry) < (UNBYTE *) &MEM_Entries[0])
		{
			MEM_Error(MEMERR_ENTRY_OUTSIDE_ARRAY);

			Result = FALSE;

			break;
		}
		if (((UNBYTE *) Entry) >= (UNBYTE *) &MEM_Entries[MEMORY_ENTRIES_MAX])
		{
			MEM_Error(MEMERR_ENTRY_OUTSIDE_ARRAY);

			Result = FALSE;

			break;
		}

		/* Check if the entry is free */
		if (!Entry->Size)
		{
			MEM_Error(MEMERR_FREE_ENTRY_IN_CHAIN);

			Result = FALSE;

			break;
		}

		/* Check if this is the area descriptor */
		if (Entry == Area)
		{
			MEM_Error(MEMERR_AREA_IN_CHAIN);

			Result = FALSE;

			break;
		}

		/* Check if Previous points at the previous entry */
		if (Entry->Previous != Previous_entry)
		{
			MEM_Error(MEMERR_WRONG_BACKWARD_LINK);

			Result = FALSE;

			break;
		}

		Handle = Entry->BLOCK_HANDLE;

		/* If the entry has a handle, check it */
		if (Handle)
		{
			/* Check if entry pointer in handle matches this entry */
			if (Handle->Entry_ptr != Entry)
			{
				MEM_Error(MEMERR_HANDLE_MISMATCH);

				Result = FALSE;

				break;
			}

			/* Check if file-type pointer is NULL */
			if (Handle->File_type_ptr == NULL)
			{
				MEM_Error(MEMERR_NULL_FILE_TYPE);

				Result = FALSE;

				break;
			}
		}

		Start = Entry->Start;
		End = Start + Entry->Size;

		/* Check if the memory block lies within the area */
		if ((Start < Area->Start) || (Start >= Area_end))
		{
			MEM_Error(MEMERR_BLOCK_OUTSIDE_AREA);

			Result = FALSE;

			break;
		}
		if ((End <= Area->Start) || (End > Area_end))
		{
			MEM_Error(MEMERR_BLOCK_OUTSIDE_AREA);

			Result = FALSE;

			break;
		}

		/* Check if the start of this block is the end of the last block */
		if (Start != End_of_last_block)
		{
			MEM_Error(MEMERR_GAP_BETWEEN_BLOCKS);

			Result = FALSE;

			break;
		}

		/* Count down area size */
		Total_size -= Entry->Size;

		/* Check the next block */
		End_of_last_block = End;
		Previous_entry = Entry;
		Entry = Entry->Next;
	}

	/* Check if the sum of all memory blocks equals the area size */
	if (Total_size || (End_of_last_block != Area_end))
	{
		MEM_Error(MEMERR_WRONG_AREA_SIZE);

		Result = FALSE;
	}

	return Result;
}

#if FALSE
/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : MEM_List
 * FUNCTION  : List the current status of the memory manager.
 * FILE      : BBMEMCHK.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.06.94 18:43
 * LAST      : 09.06.94 18:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_List(void)
{
	struct Memory_workspace *Current,*Workspace;
	struct Memory_entry *Area, *Entry;
	struct File_type *Ftype;
	MEM_HANDLE Handle;
	UNSHORT i,j;

	Current = MEM_Workspace_stack[MEM_Workspace_stack_index];

	for (j=0;j<MEMORY_WORKSPACES_MAX;j++)
	{
		Workspace = &MEM_Workspaces[j];
		if (Workspace->Nr_of_areas)
		{
			printf("*** WORKSPACE %u (%u area",j,Workspace->Nr_of_areas);
			if (Workspace->Nr_of_areas > 1)
				printf("s");
			printf(")");
			if (Workspace == Current)
				printf(" <- CURRENT");
			printf(" ***\n");

			for(i=0;i<Workspace->Nr_of_areas;i++)
			{
				Area = &(Workspace->Areas[i]);
				printf("--- AREA %u (%lu bytes) ---\n",i,Area->Size);
				Entry = Area->Next;
				while (Entry)
				{
					if (Entry->BLOCK_HANDLE)
					{
						Handle = Entry->BLOCK_HANDLE;

						if (Handle->Flags & MEM_ALLOCATED)
						{
							printf("Allocated : %lu bytes. Handle %lu,  C:%u,  L:%u  P:%u",
							 Entry->Size, (UNLONG) (Handle - &MEM_Handles[0] + 1),
 							 Handle->Claim_counter,Handle->Load_counter,
							 Handle->Priority);

							if (Handle->Flags & MEM_INVALID)
								printf(" (invalid)");
						}
						else
						{
							printf("Persistent : %lu bytes. Handle %lu,  C:%u,  L:%u  P:%u",
							 Entry->Size, (UNLONG) (Handle - &MEM_Handles[0] + 1),
 							 Handle->Claim_counter,Handle->Load_counter,
							 Handle->Priority);
						}
						printf(".\n");

						/* Print file type data */
						Ftype = Handle->File_type_ptr;
						if (Ftype->Name)
						{
							printf("  %s (%lx)\n",(char *) Ftype->Name,Handle->File_index);
						}
					}
					else
						printf("Free : %lu bytes.\n",Entry->Size);

					Entry = Entry->Next;
				}
			}
			printf("\n");
		}
	}
}
#endif

