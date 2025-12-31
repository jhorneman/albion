/************
 * NAME     : ICCHANGE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>

#include <BBDEF.H>
#include <BBMEM.H>

#include <MAP.H>
#include <NPCS.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <PRTLOGIC.H>
#include <EVELOGIC.H>
#include <ICCHANGE.H>

/* defines */

#define MODIFICATIONS_PER_LIST	(100)

/* structure definitions */

/* Modification list entry */
struct Modification {
	UNBYTE X, Y;
	UNBYTE Type, Sub_type;
	UNSHORT Value;
	UNSHORT Map_nr;
};

/* Modification list */
struct Modification_list {
	MEM_HANDLE Next_list;
	struct Modification Modifications[MODIFICATIONS_PER_LIST];
};

/* prototypes */

void Destroy_modification_list(MEM_HANDLE Handle);

void Remove_modification(MEM_HANDLE Handle, UNSHORT Index,
 UNSHORT Nr_modifications);

void Replace_2D_cutmap_block(SISHORT X, SISHORT Y, UNSHORT Copy_flags,
 UNSHORT Block_index);

void Mix_2D_cutmap_block(SISHORT X, SISHORT Y, UNSHORT Copy_flags,
 UNSHORT Block_index);

/* global variables */

MEM_HANDLE Permanent_modification_list = NULL;
UNSHORT Nr_permanent_modifications;

MEM_HANDLE Temporary_modification_list = NULL;
UNSHORT Nr_temporary_modifications;

/* Modification list file type */
static UNCHAR Modification_list_fname[] = "Modification list";

static struct File_type Modification_list_ftype = {
	NULL,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Modification_list_fname
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_modifications
 * FUNCTION  : Initialize modification list.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 15:54
 * LAST      : 03.09.95 18:55
 * INPUTS    : UNBYTE *Data - Pointer to start of modification data.
 *             MEM_HANDLE *First_list_handle_ptr - Pointer to handle of first list.
 * RESULT    : UNSHORT : Number of modifications in list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Init_modifications(UNBYTE *Data, MEM_HANDLE *First_list_handle_ptr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle;
	MEM_HANDLE Handle2;
	UNSHORT Nr_modifications;
	UNSHORT i;

	/* Read number of modifications */
	Nr_modifications = *((UNSHORT *) Data);
	Data += 2;

	/* Allocate first modification list */
	Handle = MEM_Do_allocate(sizeof(struct Modification_list), 0,
	 &Modification_list_ftype);

	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Store handle */
	*First_list_handle_ptr = Handle;

	/* Clear next list entry */
	List->Next_list = NULL;

	/* Copy complete lists */
	for (i=0;i<Nr_modifications / MODIFICATIONS_PER_LIST;i++)
	{
		/* Copy all modifications in this list */
		memcpy((UNBYTE *) &(List->Modifications[0]), Data,
		 MODIFICATIONS_PER_LIST * sizeof(struct Modification));

		Data += MODIFICATIONS_PER_LIST * sizeof(struct Modification);

		/* Allocate next modification list */
		Handle2 = MEM_Do_allocate(sizeof(struct Modification_list),
		 (UNLONG) i + 1, &Modification_list_ftype);

		/* Insert handle in current list */
		List->Next_list = Handle2;

		/* Switch to next list */
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Modification_list *) MEM_Claim_pointer(Handle);

		/* Clear next list entry */
		List->Next_list = NULL;
	}

	/* Any modifications left ? */
	if (Nr_modifications % MODIFICATIONS_PER_LIST)
	{
		/* Yes -> Copy these as well */
		memcpy(&(List->Modifications[0]), Data, (Nr_modifications %
		 MODIFICATIONS_PER_LIST) * sizeof(struct Modification));

//		Data += (Nr_modifications % MODIFICATIONS_PER_LIST) *
//		 sizeof(struct Modification);
	}

	MEM_Free_pointer(Handle);

	return Nr_modifications;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_modifications
 * FUNCTION  : Exit modification list.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:48
 * LAST      : 03.09.95 18:57
 * INPUTS    : MEM_HANDLE First_list_handle - Handle of first list.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_modifications(MEM_HANDLE First_list_handle)
{
	Destroy_modification_list(First_list_handle);
}

void
Destroy_modification_list(MEM_HANDLE Handle)
{
	struct Modification_list *List;

	/* Get pointer to list */
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Is there a next list ? */
	if (List->Next_list)
	{
		/* Yes -> Destroy the next list first */
		Destroy_modification_list(List->Next_list);
	}

	/* Destroy this list */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_modifications_for_saving
 * FUNCTION  : Prepare a modification list for saving.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 17:57
 * LAST      : 03.09.95 19:00
 * INPUTS    : MEM_HANDLE First_list_handle - Memory handle of prepared
 *              modification list.
 *             UNSHORT Nr_modifications - Number of modifications in list.
 * RESULT    : MEM_HANDLE : Handle of converted modification list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Prepare_modifications_for_saving(MEM_HANDLE First_list_handle,
 UNSHORT Nr_modifications)
{
	struct Modification_list *List;
	MEM_HANDLE Output_handle;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Allocate memory */
	Output_handle = MEM_Allocate_memory(2 + Nr_modifications *
	 sizeof(struct Modification));

	Ptr = MEM_Claim_pointer(Output_handle);

	/* Write number of modifications */
	*((UNSHORT *) Ptr) = Nr_modifications;
	Ptr += 2;

	/* Get first modification list */
	Handle = First_list_handle;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Copy complete lists */
	for (i=0;i<Nr_modifications / MODIFICATIONS_PER_LIST;i++)
	{
		/* Copy all modifications in this list */
		memcpy(Ptr, &(List->Modifications[0]), MODIFICATIONS_PER_LIST *
		 sizeof(struct Modification));
		Ptr += MODIFICATIONS_PER_LIST * sizeof(struct Modification);

		/* Switch to next list */
		Handle2 = List->Next_list;
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Modification_list *) MEM_Claim_pointer(Handle);
	}

	/* Any modifications left ? */
	if (Nr_modifications % MODIFICATIONS_PER_LIST)
	{
		/* Yes -> Copy these as well */
		memcpy(Ptr, &(List->Modifications[0]), (Nr_modifications %
		 MODIFICATIONS_PER_LIST) * sizeof(struct Modification));
	}

	MEM_Free_pointer(Handle);
	MEM_Free_pointer(Output_handle);

	return Output_handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_modifications
 * FUNCTION  : Make modifications to a map.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 16:24
 * LAST      : 03.09.95 19:00
 * INPUTS    : UNSHORT Map_nr - Map number.
 *             MEM_HANDLE First_list_handle - Memory handle of prepared
 *              modification list.
 *             UNSHORT Nr_modifications - Number of modifications in list.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_modifications(UNSHORT Map_nr, MEM_HANDLE First_list_handle,
 UNSHORT Nr_modifications)
{
	struct Modification_list *List;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;

	/* Get first modification list */
	Handle = First_list_handle;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all modifications */
	for (i=0;i<Nr_modifications;i++)
	{
		/* In the right map ? */
		if (List->Modifications[i].Map_nr == Map_nr)
		{
			/* Yes -> Make a modification */
			Do_change_icon(List->Modifications[i].X,
			 List->Modifications[i].Y,
			 List->Modifications[i].Type,
			 List->Modifications[i].Sub_type,
			 List->Modifications[i].Value);
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clone_modifications
 * FUNCTION  : Clone modifications of one map to another map.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 13:25
 * LAST      : 30.06.95 10:35
 * INPUTS    : UNSHORT Source_map_nr - Source map number.
 *             UNSHORT Target_map_nr - Target map number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This routine MUST deal with the fact that the modification
 *              list grows during the search.
 *             - Unlike other modification list functions, this function
 *              will check only the permanent modification list.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clone_modifications(UNSHORT Source_map_nr, UNSHORT Target_map_nr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;

	/* Get first modification list */
	Handle = Permanent_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all the modifications */
	for (i=0;i<Nr_permanent_modifications;i++)
	{
		/* In the source map ? */
		if (List->Modifications[i].Map_nr == Source_map_nr)
		{
			/* Yes -> Make same modification in target map */
			Add_permanent_modification(
			 List->Modifications[i].X,
			 List->Modifications[i].Y,
			 List->Modifications[i].Type,
			 List->Modifications[i].Sub_type,
			 List->Modifications[i].Value,
			 Target_map_nr);
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_permanent_modification
 * FUNCTION  : Add a new modification to the permanent list.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 15:59
 * LAST      : 08.09.95 22:52
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT Type - Modification type.
 *             UNSHORT Sub_type - Modification sub-type.
 *             UNSHORT Value - Modification value.
 *             UNSHORT Map_nr - Map number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The solution to the horrible logic problem was found by
 *              Marcus Pukropski.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_permanent_modification(UNSHORT X, UNSHORT Y, UNSHORT Type,
 UNSHORT Sub_type, UNSHORT Value, UNSHORT Map_nr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle;
	MEM_HANDLE Handle2;
	BOOLEAN Found;
	UNSHORT i;

	/* Get first modification list */
	Handle = Permanent_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all current entries */
	Found = FALSE;
	for (i=0;i<Nr_permanent_modifications;i++)
	{
		/* Have we already found a similar entry ? */
		if (!Found)
		{
			/* No -> Is this the same entry as the one we're adding ? */
			if ((List->Modifications[i].X			== X) &&
			 (List->Modifications[i].Y				== Y) &&
			 (List->Modifications[i].Type			== Type) &&
			 (List->Modifications[i].Sub_type	== Sub_type) &&
			 (List->Modifications[i].Map_nr		== Map_nr))
			{
				/* Yes -> Is this the very last entry ? */
				if (i < (Nr_permanent_modifications - 1))
				{
					/* No -> Remove this entry */
					Remove_modification
					(
						Handle,
						i % MODIFICATIONS_PER_LIST,
						Nr_permanent_modifications
					);
				}

				/* Count down */
				Nr_permanent_modifications--;

				/* Found! */
				Found = TRUE;
			}
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	/* Insert new modification */
	i = Nr_permanent_modifications % MODIFICATIONS_PER_LIST;

	List->Modifications[i].X			= X;
	List->Modifications[i].Y			= Y;
	List->Modifications[i].Type		= Type;
	List->Modifications[i].Sub_type	= Sub_type;
	List->Modifications[i].Value		= Value;
	List->Modifications[i].Map_nr		= Map_nr;

	/* Count up */
	Nr_permanent_modifications++;

	/* Time for a new list ? */
	if (!(Nr_permanent_modifications % MODIFICATIONS_PER_LIST))
	{
		/* Yes -> Allocate next modification list */
		Handle2 = MEM_Do_allocate
		(
			sizeof(struct Modification_list),
			(UNLONG) (Nr_permanent_modifications / MODIFICATIONS_PER_LIST),
			&Modification_list_ftype
		);

		/* Insert handle in current list */
		List->Next_list = Handle2;

		/* Switch to next list */
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Modification_list *) MEM_Claim_pointer(Handle);

		/* Clear next list entry */
		List->Next_list = NULL;
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_temporary_modification
 * FUNCTION  : Add a new modification to the temporary list.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 15:59
 * LAST      : 08.09.95 22:53
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT Type - Modification type.
 *             UNSHORT Sub_type - Modification sub-type.
 *             UNSHORT Value - Modification value.
 *             UNSHORT Map_nr - Map number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The solution to the horrible logic problem was found by
 *              Marcus Pukropski.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_temporary_modification(UNSHORT X, UNSHORT Y, UNSHORT Type,
 UNSHORT Sub_type, UNSHORT Value, UNSHORT Map_nr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle;
	MEM_HANDLE Handle2;
	BOOLEAN Found;
	UNSHORT i;

	/* Get first modification list */
	Handle = Temporary_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all current entries */
	Found = FALSE;
	for (i=0;i<Nr_temporary_modifications;i++)
	{
		/* Have we already found a similar entry ? */
		if (!Found)
		{
			/* No -> Is this the same entry as the one we're adding ? */
			if ((List->Modifications[i].X			== X) &&
			 (List->Modifications[i].Y				== Y) &&
			 (List->Modifications[i].Type			== Type) &&
			 (List->Modifications[i].Sub_type	== Sub_type) &&
			 (List->Modifications[i].Map_nr		== Map_nr))
			{
				/* Yes -> Is this the very last entry ? */
				if (i < (Nr_temporary_modifications - 1))
				{
					/* No -> Remove this entry */
					Remove_modification
					(
						Handle,
						i % MODIFICATIONS_PER_LIST,
						Nr_temporary_modifications
					);
				}

				/* Count down */
				Nr_temporary_modifications--;

				/* Found! */
				Found = TRUE;
			}
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	/* Insert new modification */
	i = Nr_temporary_modifications % MODIFICATIONS_PER_LIST;

	List->Modifications[i].X			= X;
	List->Modifications[i].Y			= Y;
	List->Modifications[i].Type		= Type;
	List->Modifications[i].Sub_type	= Sub_type;
	List->Modifications[i].Value		= Value;
	List->Modifications[i].Map_nr		= Map_nr;

	/* Count up */
	Nr_temporary_modifications++;

	/* Time for a new list ? */
	if (!(Nr_temporary_modifications % MODIFICATIONS_PER_LIST))
	{
		/* Yes -> Allocate next modification list */
		Handle2 = MEM_Do_allocate
		(
			sizeof(struct Modification_list),
			(UNLONG) (Nr_temporary_modifications / MODIFICATIONS_PER_LIST),
			&Modification_list_ftype
		);

		/* Insert handle in current list */
		List->Next_list = Handle2;

		/* Switch to next list */
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Modification_list *) MEM_Claim_pointer(Handle);

		/* Clear next list entry */
		List->Next_list = NULL;
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_modification
 * FUNCTION  : Remove a modification from a modification list.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.09.95 16:42
 * LAST      : 08.09.95 21:44
 * INPUTS    : MEM_HANDLE Handle - Handle of modification sub-list.
 *             UNSHORT Index - Entry index within current sub-list (0...).
 *             UNSHORT Nr_modifications - Number of modifications.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The number of modifications won't be counted down by this
 *              function.
 *             - The entire last sub-list will be copied down, not just the
 *              elements which are actually in use.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_modification(MEM_HANDLE Handle, UNSHORT Index,
 UNSHORT Nr_modifications)
{
	struct Modification_list *List;
	struct Modification_list *Next_list;
	MEM_HANDLE List_handle;
	MEM_HANDLE Previous_list_handle;
	MEM_HANDLE Next_list_handle;

	/* Exit if the modification list is already empty */
	if (!Nr_modifications)
		return;

	List_handle				= Handle;
	Previous_list_handle = NULL;

	List = (struct Modification_list *) MEM_Claim_pointer(List_handle);

	/* Are we removing the last entry in this sub-list ? */
	if (Index < (MODIFICATIONS_PER_LIST - 1))
	{
		/* No -> Copy the rest of the current sub-list one entry down */
		memmove
		(
			(UNBYTE *) &(List->Modifications[Index]),
			(UNBYTE *) &(List->Modifications[Index + 1]),
			(MODIFICATIONS_PER_LIST - Index - 1) * sizeof(struct Modification)
		);
	}

	/* Is there a next list ? */
	if (List->Next_list)
	{
		/* Yes -> Get pointer to next list */
		Next_list_handle	= List->Next_list;
		Next_list			= (struct Modification_list *) MEM_Claim_pointer(Next_list_handle);

		/* Copy the first entry of the next list into the last entry of this
		  list */
		memmove
		(
			(UNBYTE *) &(List->Modifications[MODIFICATIONS_PER_LIST - 1]),
			(UNBYTE *) &(Next_list->Modifications[0]),
			sizeof(struct Modification)
		);

		MEM_Free_pointer(List_handle);

		/* Handle full sub-lists following the current sub-list */
		for (;;)
		{
			/* Go to the next list */
			List = Next_list;

			Previous_list_handle	= List_handle;
			List_handle				= Next_list_handle;

			/* Copy the list one entry down */
			memmove
			(
				(UNBYTE *) &(List->Modifications[0]),
			 	(UNBYTE *) &(List->Modifications[1]),
				(MODIFICATIONS_PER_LIST - 1) * sizeof(struct Modification)
			);

			/* Exit if there is no next list */
			if (!List->Next_list)
			{
				MEM_Free_pointer(List_handle);
				break;
			}

			/* Get pointer to next list */
			Next_list_handle	= List->Next_list;
			Next_list			= (struct Modification_list *) MEM_Claim_pointer(Next_list_handle);

			/* Copy the first entry of the next list into the last entry
			  of this list */
			memmove
			(
				(UNBYTE *) &(List->Modifications[MODIFICATIONS_PER_LIST - 1]),
				(UNBYTE *) &(Next_list->Modifications[0]),
				sizeof(struct Modification)
			);

			/* Free handle of current list */
			MEM_Free_pointer(List_handle);
		}
	}
	else
	{
		MEM_Free_pointer(List_handle);
	}

	/* Is the last sub-list now empty ? */
	if (!(Nr_modifications % MODIFICATIONS_PER_LIST))
	{
		/* Yes -> Is there a previous list ? */
		if (Previous_list_handle)
		{
			/* Yes -> Delete the last sub-list */
			MEM_Free_memory(List_handle);

			/* Remove the link in the next-to-last sub-list */
			List = (struct Modification_list *) MEM_Claim_pointer(Previous_list_handle);
			List->Next_list = NULL;
			MEM_Free_pointer(Previous_list_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_change_icon
 * FUNCTION  : Change an icon.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 16:41
 * LAST      : 08.09.95 22:05
 * INPUTS    : SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Type - Modification type.
 *             UNSHORT Sub_type - Modification sub-type.
 *             UNSHORT Value - Modification value.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The icon is changed in the current map.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_change_icon(SISHORT X, SISHORT Y, UNSHORT Type, UNSHORT Sub_type,
 UNSHORT Value)
{
	UNBYTE *Map_data_ptr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return;

	/* Get map data */
	Map_data_ptr = MEM_Claim_pointer(Map_handle);

	/* Which change type ? */
	switch (Type)
	{
		/* 2D underlay */
		case UNDERLAY_2D_CHANGE:
		{
			struct Square_2D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 2D map ? */
			if (!_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->m[1] &= 0xF0;
				Map_ptr->m[1] |= (UNBYTE) ((Value & 0x0F00) >> 8);
				Map_ptr->m[2] = (UNBYTE) (Value & 0x00FF);

				/* Update map area */
				Update_2D_map_area(X, Y, 1, 1);
			}
			break;
		}
		/* 2D overlay */
		case OVERLAY_2D_CHANGE:
		{
			struct Square_2D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 2D map ? */
			if (!_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->m[0] = (UNBYTE) ((Value & 0x0FF0) >> 4);
				Map_ptr->m[1] &= 0x0F;
				Map_ptr->m[1] |= (UNBYTE) ((Value & 0x000F) << 4);

				/* Update map area */
				Update_2D_map_area(X, Y, 1, 1);
			}
			break;
		}
		/* 3D wall or object group */
		case NORMAL_3D_CHANGE:
		{
			struct Square_3D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->Wall_layer = (UNBYTE) Value;
			}
			break;
		}
		/* 3D floor */
		case FLOOR_3D_CHANGE:
		{
			struct Square_3D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->Floor_layer = (UNBYTE) Value;
			}
			break;
		}
		/* 3D ceiling */
		case CEILING_3D_CHANGE:
		{
			struct Square_3D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->Ceiling_layer = (UNBYTE) Value;
			}
			break;
		}
		/* NPC movement mode */
		case NPC_MOVE_CHANGE:
		{
			UNSHORT NPC_index;

			NPC_index = (UNSHORT) X;

			/* Exit if the NPC index is illegal */
			if (NPC_index >= NPCS_PER_MAP)
				break;

			/* Exit if the new movement mode is absolute or relative path */
			if ((Value == ABS_PATH_MOVEMENT) || (Value == REL_PATH_MOVEMENT))
				break;

			/* Change NPC movement mode */
			VNPCs[NPC_index].Movement_type = Value;

			break;
		}
		/* NPC graphics */
		case NPC_GFX_CHANGE:
		{
			UNSHORT NPC_index;

			NPC_index = (UNSHORT) X;

			/* Exit if the NPC index is illegal */
			if (NPC_index >= NPCS_PER_MAP)
				break;

			/* 2D or 3D map ? */
			if (_3D_map)
			{
				/* 3D map -> Change NPC graphics */
				VNPCs[NPC_index].Graphics_nr = Value;
			}
			else
			{
				/* 2D map -> Exit current graphics */
				Exit_2D_NPC_graphics(NPC_index);

				/* Change NPC graphics */
				VNPCs[NPC_index].Graphics_nr = Value;

				/* Init new graphics */
				Init_2D_NPC_graphics(NPC_index);
			}

			break;
		}
		/* Event entry */
		case EVENT_ENTRY_CHANGE:
		{
			struct Map_event_entry *Entry_data;
			UNSHORT Nr_entries;
			UNSHORT i;
			UNBYTE *Ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* Get pointer to event entry data */
			Ptr = Map_data_ptr + Event_entry_offset;

			/* Find the event entry-list for the desired Y-coordinate */
			for (i=0;i<Y;i++)
			{
				Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
			}
			Nr_entries = *((UNSHORT *) Ptr);
			Ptr += 2;
			Entry_data = (struct Map_event_entry *) Ptr;

			/* Search the event entries for the desired X-coordinate */
			for (i=0;i<Nr_entries;i++)
			{
				/* Right X-coordinate ? */
				if (Entry_data[i].X == X)
				{
					/* Yes -> Change event entry */
					Entry_data[i].First_block_nr = Value;

					/* Indicate that the map events have changed */
					Map_events_changed = TRUE;

					break;
				}
			}
			break;
		}
		/* Replace 2D cut-map block */
		case BLOCK_2D_REPLACE:
		{
			Replace_2D_cutmap_block(X, Y, Sub_type, Value);
			break;
		}
		/* Mix 2D cut-map block */
		case BLOCK_2D_MIX:
		{
			Mix_2D_cutmap_block(X, Y, Sub_type, Value);
			break;
		}
		/* Trigger mode */
		case TRIGGER_MODE_CHANGE:
		{
			struct Map_event_entry *Entry_data;
			UNSHORT Nr_entries;
			UNSHORT i;
			UNBYTE *Ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* Get pointer to event entry data */
			Ptr = Map_data_ptr + Event_entry_offset;

			/* Find the event entry-list for the desired Y-coordinate */
			for (i=0;i<Y;i++)
			{
				Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
			}
			Nr_entries = *((UNSHORT *) Ptr);
			Ptr += 2;
			Entry_data = (struct Map_event_entry *) Ptr;

			/* Search the event entries for the desired X-coordinate */
			for (i=0;i<Nr_entries;i++)
			{
				/* Right X-coordinate ? */
				if (Entry_data[i].X == X)
				{
					/* Yes -> Change trigger modes */
					Entry_data[i].Trigger_modes = Value;

					/* Indicate that the map events have changed */
					Map_events_changed = TRUE;

					break;
				}
			}
			break;
		}
	}
	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Replace_2D_cutmap_block
 * FUNCTION  : Replace a 2D map part with a cut-map block.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.09.95 22:00
 * LAST      : 08.09.95 22:00
 * INPUTS    : SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Copy_flags - Copy flags :
 *             	0 - copy underlays.
 *             	1 - copy overlays.
 *             UNSHORT Block_index - Cut-map block index.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The block is changed in the current map.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Replace_2D_cutmap_block(SISHORT X, SISHORT Y, UNSHORT Copy_flags,
 UNSHORT Block_index)
{
	struct Square_2D *Map_ptr;
	struct Square_2D *Block;
	struct Cut_block_2D *Block_ptr;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;
	UNSHORT B1, B2, B3;
	UNSHORT i, j;
	UNBYTE *Map_data_ptr;
 	UNBYTE *Ptr;

	/* Exit if the map isn't initialized / 3D map */
	if (!Map_initialized || _3D_map)
		return;

	/* Get map data */
	Map_data_ptr = MEM_Claim_pointer(Map_handle);

	/* Find cut-map block */
	Ptr = MEM_Claim_pointer(Blocklist_handle);

	for(i=0;i<Block_index;i++)
	{
		Block_ptr = (struct Cut_block_2D *) Ptr;

		Ptr += sizeof(struct Cut_block_2D) + (Block_ptr->Width *
			Block_ptr->Height * sizeof(struct Square_2D));
	}
	Block_ptr = (struct Cut_block_2D *) Ptr;

	/* Update map area */
	Update_2D_map_area(X, Y, Block_ptr->Width, Block_ptr->Height);

	/* Change icons */
	Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);
	Block = (struct Square_2D *) (Block_ptr + 1);

	for (i=0;i<Block_ptr->Height;i++)
	{
		for (j=0;j<Block_ptr->Width;j++)
		{
			/* Are the coordinates inside of the map ? */
			if ((X > 0) && (X <= Map_width) && (Y > 0) && (Y <= Map_height))
			{
				/* Yes -> Read bytes from block */
				B1 = (UNSHORT) Block->m[0];
				B2 = (UNSHORT) Block->m[1];
				B3 = (UNSHORT) Block->m[2];

				/* Build overlay and underlay number */
				Underlay_nr = ((B2 & 0x0F) << 8) | B3;
				Overlay_nr = (B1 << 4) | (B2 >> 4);

				/* Copy underlays ? */
				if (Copy_flags & 0x01)
				{
					/* Yes -> Put in map */
					Map_ptr->m[1] &= 0xF0;
					Map_ptr->m[1] |= (UNBYTE) ((Underlay_nr & 0x0F00) >> 8);
					Map_ptr->m[2] = (UNBYTE) (Underlay_nr & 0x00FF);
				}

				/* Copy overlays ? */
				if (Copy_flags & 0x02)
				{
					/* Yes -> Put in map */
					Map_ptr->m[0] = (UNBYTE) ((Overlay_nr & 0x0FF0) >> 4);
					Map_ptr->m[1] &= 0x0F;
					Map_ptr->m[1] |= (UNBYTE) ((Overlay_nr & 0x000F) << 4);
				}
			}

			/* Next X-coordinate */
			X++;
			Map_ptr++;
			Block++;
		}
		/* Next Y-coordinate */
		X -= Block_ptr->Width;
		Y++;
		Map_ptr += Map_width - Block_ptr->Width;
	}

	MEM_Free_pointer(Blocklist_handle);
	MEM_Free_pointer(Map_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mix_2D_cutmap_block
 * FUNCTION  : Mix a 2D map part with a cut-map block.
 * FILE      : ICCHANGE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.09.95 22:03
 * LAST      : 08.09.95 22:03
 * INPUTS    : SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Copy_flags - Copy flags :
 *             	0 - copy underlays.
 *             	1 - copy overlays.
 *             UNSHORT Block_index - Cut-map block index.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The block is changed in the current map.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Mix_2D_cutmap_block(SISHORT X, SISHORT Y, UNSHORT Copy_flags,
 UNSHORT Block_index)
{
	struct Square_2D *Map_ptr;
	struct Square_2D *Block;
	struct Cut_block_2D *Block_ptr;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;
	UNSHORT B1, B2, B3;
	UNSHORT i, j;
	UNBYTE *Map_data_ptr;
 	UNBYTE *Ptr;

	/* Exit if the map isn't initialized / 3D map */
	if (!Map_initialized || _3D_map)
		return;

	/* Get map data */
	Map_data_ptr = MEM_Claim_pointer(Map_handle);

	/* Find cut-map block */
	Ptr = MEM_Claim_pointer(Blocklist_handle);

	for(i=0;i<Block_index;i++)
	{
		Block_ptr = (struct Cut_block_2D *) Ptr;

		Ptr += sizeof(struct Cut_block_2D) + (Block_ptr->Width *
			Block_ptr->Height * sizeof(struct Square_2D));
	}
	Block_ptr = (struct Cut_block_2D *) Ptr;

	/* Update map area */
	Update_2D_map_area(X, Y, Block_ptr->Width, Block_ptr->Height);

	/* Change icons */
	Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);
	Block = (struct Square_2D *) (Block_ptr + 1);

	for (i=0;i<Block_ptr->Height;i++)
	{
		for (j=0;j<Block_ptr->Width;j++)
		{
			/* Are the coordinates inside of the map ? */
			if ((X > 0) && (X <= Map_width) && (Y > 0) && (Y <= Map_height))
			{
				/* Yes -> Read bytes from block */
				B1 = (UNSHORT) Block->m[0];
				B2 = (UNSHORT) Block->m[1];
				B3 = (UNSHORT) Block->m[2];

				/* Build overlay and underlay number */
				Underlay_nr = ((B2 & 0x0F) << 8) | B3;
				Overlay_nr = (B1 << 4) | (B2 >> 4);

				/* Any underlay / copy underlays ? */
				if ((Underlay_nr > 1) && (Copy_flags & 0x01))
				{
					/* Yes -> Put in map */
					Map_ptr->m[1] &= 0xF0;
					Map_ptr->m[1] |= (UNBYTE) ((Underlay_nr & 0x0F00) >> 8);
					Map_ptr->m[2] = (UNBYTE) (Underlay_nr & 0x00FF);
				}

				/* Any overlay / copy overlays ? */
				if ((Overlay_nr > 1) && (Copy_flags & 0x02))
				{
					/* Yes -> Put in map */
					Map_ptr->m[0] = (UNBYTE) ((Overlay_nr & 0x0FF0) >> 4);
					Map_ptr->m[1] &= 0x0F;
					Map_ptr->m[1] |= (UNBYTE) ((Overlay_nr & 0x000F) << 4);
				}
			}

			/* Next X-coordinate */
			X++;
			Map_ptr++;
			Block++;
		}
		/* Next Y-coordinate */
		X -= Block_ptr->Width;
		Y++;
		Map_ptr += Map_width - Block_ptr->Width;
	}

	MEM_Free_pointer(Blocklist_handle);
	MEM_Free_pointer(Map_handle);
}


