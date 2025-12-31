/************
 * NAME     : HDOB.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 20-7-1994
 * PROJECT  : HDOB system
 * NOTES    : - It is VITAL that [ MEM_Claim_pointer ] return zero when
 *             handle NULL is claimed.
 *          : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : HDOB.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <GFXFUNC.H>
#include <HDOB.H>

/* global variables */
UNSHORT HDOB_Hide_counter;
struct HDOB HDOBs[HDOBS_MAX];

/* external variables */
extern struct OPM Main_OPM;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_HDOBs
 * FUNCTION  : Delete all current HDOBs and switch visibility on.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:20
 * LAST      : 20.07.94 16:20
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_HDOBs(void)
{
	struct HDOB *Current;
	UNSHORT i;

	/* Delete all HDOBs */
	for (i=0;i<HDOBS_MAX;i++)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Delete */
			Current->Flags &= ~HDOB_ON;

			/* Destroy background buffer */
			MEM_Free_memory(Current->Background_handle);
			Current->Background_handle = NULL;
		}
	}

	/* Switch visibility on */
	HDOB_Hide_counter = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_HDOBs
 * FUNCTION  : Show HDOBs.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:21
 * LAST      : 20.07.94 16:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - You must call this function once for each Hide_HDOBs()-call.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_HDOBs(void)
{
	if (HDOB_Hide_counter)
		HDOB_Hide_counter--;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Hide_HDOBs
 * FUNCTION  : Hide HDOBs.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 16:22
 * LAST      : 20.07.94 16:22
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - You must call this function once for each Show_HDOBs()-call.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Hide_HDOBs(void)
{
	if (HDOB_Hide_counter < 0xFFFF)
		HDOB_Hide_counter++;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_HDOB
 * FUNCTION  : Add a HDOB to the current list.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 13:06
 * LAST      : 20.07.94 13:06
 * INPUTS    : struct HDOB *New - Pointer to new HDOB.
 * RESULT    : UNSHORT : HDOB number / 0xFFFF = error.
 * BUGS      : No known.
 * NOTES     : - The HDOB data will be copied to an internal array.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Add_HDOB(struct HDOB *New)
{
	struct HDOB *Free = NULL;
	UNSHORT i, Output = 0xFFFF;

	/* Find a free HDOB */
	for (i=0;i<HDOBS_MAX;i++)
	{
		/* Is this HDOB free ? */
		if (!(HDOBs[i].Flags & HDOB_ON))
		{
			/* Yes */
			Free = &HDOBs[i];

			/* Initialize */
			BASEMEM_CopyMem((UNBYTE *) New, (UNBYTE *) Free, sizeof(struct HDOB));
			Free->Flags = Free->Flags | HDOB_ON;

			/* Allocate background buffer */
			Free->Background_handle = MEM_Allocate_memory(New->Width * New->Height);

			/* Exit */
			Output = i;
			break;
		}
	}
	return (Output);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_HDOB
 * FUNCTION  : Delete a HDOB from the current list.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 13:13
 * LAST      : 20.07.94 13:13
 * INPUTS    : UNSHORT HDOB_nr - Number of HDOB that must be deleted.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_HDOB(UNSHORT HDOB_Nr)
{
	struct HDOB *Delete;

	if (HDOB_Nr != 0xFFFF)
	{
		Delete = &HDOBs[HDOB_Nr];

		/* Switch HDOB off */
		Delete->Flags &= ~HDOB_ON;

		/* Destroy background buffer */
		MEM_Free_memory(Delete->Background_handle);
		Delete->Background_handle = NULL;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_HDOB_position
 * FUNCTION  : Set the position of a HDOB.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.94 15:03
 * LAST      : 22.07.94 15:03
 * INPUTS    : UNSHORT HDOB_Nr - Number of HDOB that must be moved.
 *             SISHORT X - New X-coordinate.
 *             SISHORT Y - New Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_HDOB_position(UNSHORT HDOB_Nr, SISHORT X, SISHORT Y)
{
	struct HDOB *HDOB;

	if (HDOB_Nr != 0xFFFF)
	{
		HDOBs[HDOB_Nr].X = X;
		HDOBs[HDOB_Nr].Y = Y;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_HDOBs
 * FUNCTION  : Display HDOBs on the screen.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 15:01
 * LAST      : 20.07.94 15:01
 * INPUTS    : struct BBPOINT *Mouse - Pointer to point containing the
 *              current mouse coordinates (for attached HDOBs).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_HDOBs(struct BBPOINT *Mouse)
{
	struct HDOB *Current;
	UNBYTE *Graphics_ptr, *Background_ptr;
	UNSHORT i;
	SISHORT X,Y;

	/* Show at all ? */
	if (HDOB_Hide_counter)
		return;

	/* Draw all HDOBs (forward) */
	for (i=0;i<HDOBS_MAX;i++)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Is the current HDOB attached to the mouse ? */
			if (Current->Flags & HDOB_ATTACHED)
			{
				/* Yes -> Add HDOB coordinates to mouse coordinates */
				X = Mouse->x + Current->X;
				Y = Mouse->y + Current->Y;
			}
			else
			{
				/* No -> Use the HDOB coordinates */
				X = Current->X;
				Y = Current->Y;
			}

			/* Save background */
			Background_ptr = MEM_Claim_pointer(Current->Background_handle);
			Get_block(&Main_OPM, X, Y, Current->Width, Current->Height, Background_ptr);
			MEM_Free_pointer(Current->Background_handle);

			/* Get graphics address */
			Graphics_ptr = MEM_Claim_pointer(Current->Graphics_handle) +
			 Current->Graphics_offset;

			/* Draw HDOB */
			switch (Current->Draw_mode)
			{
				case HDOB_MASK:
					Put_masked_block(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr);
					break;
				case HDOB_BLOCK:
					Put_unmasked_block(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr);
					break;
				case HDOB_SILHOUETTE:
					Put_silhouette(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr, (UNBYTE) Current->Data);
					break;
				case HDOB_BOX:
					OPM_FillBox(&Main_OPM, X, Y, Current->Width,
					 Current->Height, (UNBYTE) Current->Data);
					break;
				case HDOB_RECOLOUR:
					Put_recoloured_block(&Main_OPM, X, Y, Current->Width,
					 Current->Height, Graphics_ptr, (UNBYTE *) Current->Data);
					break;
				case HDOB_RECOLBOX:
					Put_recoloured_box(&Main_OPM, X, Y, Current->Width,
					 Current->Height, (UNBYTE *) Current->Data);
					break;
			}

			MEM_Free_pointer(Current->Background_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Erase_HDOBs
 * FUNCTION  : Remove HDOBs from the screen.
 * FILE      : HDOB.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.94 15:03
 * LAST      : 20.07.94 15:03
 * INPUTS    : struct BBPOINT *Mouse - Pointer to point containing the
 *              current mouse coordinates (for attached HDOBs).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Erase_HDOBs(struct BBPOINT *Mouse)
{
	struct HDOB *Current;
	UNBYTE *Background_ptr;
	SISHORT i,X,Y;

	/* Show at all ? */
	if (HDOB_Hide_counter)
		return;

	/* Erase all HDOBs (backward) */
	for (i=HDOBS_MAX-1;i>=0;i--)
	{
		Current = &HDOBs[i];

		/* Is the current HDOB active ? */
		if (Current->Flags & HDOB_ON)
		{
			/* Yes -> Is the current HDOB attached to the mouse ? */
			if (Current->Flags & HDOB_ATTACHED)
			{
				/* Yes -> Add HDOB coordinates to mouse coordinates */
				X = Mouse->x + Current->X;
				Y = Mouse->y + Current->Y;
			}
			else
			{
				/* No -> Use the HDOB coordinates */
				X = Current->X;
				Y = Current->Y;
			}

			/* Restore background */
			Background_ptr = MEM_Claim_pointer(Current->Background_handle);
			Put_unmasked_block(&Main_OPM, X, Y, Current->Width, Current->Height,
			 Background_ptr);
			MEM_Free_pointer(Current->Background_handle);
		}
	}
}

