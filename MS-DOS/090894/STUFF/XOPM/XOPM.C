/************
 * NAME     : XOPM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 5-8-1994
 * PROJECT  : eXtended OPM functions
 * NOTES    :
 * SEE ALSO : XOPM.C
 ************/

/* includes */

#include <BBDEF.H>
#include <BBOPM.H>
#include <BBBASMEM.H>

#include <GFXFUNC.H>
#include <XOPM.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XOPM_Create_base
 * FUNCTION  : Create a new base XOPM.
 * FILE      : XOPM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 16:22
 * LAST      : 08.08.94 16:22
 * INPUTS    : SISHORT X - X-coordinate on screen.
 *             SISHORT Y - Y-coordinate on screen.
 *             UNSHORT Width - Width of base XOPM.
 *             UNSHORT Height - Height of base XOPM.
 *             struct XOPM *XOPM - Pointer to new XOPM structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XOPM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XOPM_Create_base(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height,
 struct XOPM *XOPM)
{
	UNBYTE *Ptr;

	/* Allocate memory for OPM */
	XOPM->Handle = MEM_Do_allocate(Width * Height, (UNLONG) &(XOPM->OPM),
	 &OPM_ftype);

	/* Create an OPM */
	Ptr = MEM_Claim_pointer(XOPM->Handle);
	OPM_New(Width, Height, 1, &(XOPM->OPM), Ptr);
	MEM_Free_pointer(XOPM->Handle);

	/* Initialize XOPM */
	XOPM->Flags = 0;
	XOPM->X = X;
	XOPM->Y = Y;
	XOPM->Parent = NULL;
	XOPM->Child = NULL;
	XOPM->Next = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XOPM_Delete_base
 * FUNCTION  : Delete a base XOPM.
 * FILE      : XOPM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 16:30
 * LAST      : 08.08.94 16:30
 * INPUTS    : struct XOPM *XOPM - Pointer to XOPM structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XOPM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XOPM_Delete_base(struct XOPM *XOPM)
{
	/* Remove children */
	while (XOPM->Child)
		XOPM_Remove_child(XOPM->Child);

	/* Delete OPM */
	OPM_Del(&(XOPM->OPM));

	/* Free OPM memory */
	MEM_Free_memory(XOPM->Handle);
	XOPM->Handle = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XOPM_Add_child
 * FUNCTION  : Add a child XOPM to a base XOPM.
 * FILE      : XOPM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 16:32
 * LAST      : 08.08.94 16:32
 * INPUTS    : struct XOPM *Base - Pointer to base XOPM structure.
 *             SISHORT X - X-coordinate in base OPM.
 *             SISHORT Y - Y-coordinate in base OPM.
 *             UNSHORT Width - Width of child XOPM.
 *             UNSHORT Height - Height of child XOPM.
 *             struct XOPM *Child - Pointer to child XOPM structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XOPM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XOPM_Add_child(struct XOPM *Base, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height, struct XOPM *Child)
{
	struct XOPM *T;

	/* Create a virtual OPM */
	OPM_CreateVirtualOPM(&(Base->OPM), &(Child->OPM), X, Y, Width, Height);

	/* Initialize child XOPM */
	Child->Flags = 0;
	Child->X = X;
	Child->Y = Y;
	Child->Child = NULL;

	/* Link child to base XOPM */
	Child->Parent = Base;
	T = Base->Child;
	Base->Child = Child;
	Child->Next = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XOPM_Remove_child
 * FUNCTION  : Remove a child XOPM.
 * FILE      : XOPM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.94 16:45
 * LAST      : 08.08.94 16:45
 * INPUTS    : struct XOPM *Child - Pointer to child XOPM.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XOPM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XOPM_Remove_child(struct XOPM *Child)
{
	struct XOPM *Base, *T;

	/* Delete virtual OPM */
	OPM_Del(&(Child->OPM));

	/* Unlink child XOPM */
	Base = Child->Parent;
	T = Base->Child;

	if (Child == T)
		Base->Child = Child->Next;
	else
	{
		while (T->Next != Child)
			T = Base->Child;
		T->Next = Child->Next;
	}

	Child->Parent = NULL;
	Child->Next = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : XOPM_Check_update
 * FUNCTION  : Set update flags for all child XOPMs given that a certain area
 *              of a base XOPM will be changed.
 * FILE      : XOPM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.08.94 16:48
 * LAST      : 05.08.94 16:48
 * INPUTS    : struct XOPM *XOPM - Pointer to XOPM.
 *             SISHORT X - Left X-coordinate.
 *             SISHORT Y - Top Y-coordinate.
 *             UNSHORT Width - Width of modified area.
 *             UNSHORT Height - Height of modified area.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : XOPM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
XOPM_Check_update(struct XOPM *XOPM, SISHORT X, SISHORT Y, UNSHORT Width,
 UNSHORT Height)
{
	struct XOPM *Next;
	struct BBRECT *C;
	UNSHORT X2, Y2;

	/* Calculate bottom/right coordinates */
	X2 = X + Width - 1;
	Y2 = Y + Height - 1;

	/* Is this a child XOPM ? */
	if (XOPM->Parent)
		/* Yes -> Base OPM is parent */
		XOPM = XOPM->Parent;

	/* Check all children of the base XOPM */
	Next = XOPM->Child;
	while (Next)
	{
		/* Get clipping rectangle */
		C = &(Next->OPM.clip);

		/* Is the area within this XOPM's clip rectangle ? */
		if (!((X >= C->left + C->width) || (Y >= C->top + C->height )
		 || (X2 < C->left) || (Y2 < C->top)))
		{
			/* Yes -> Update */
			Next->Flags |= XOPM_UPDATE;
			Next->OPM.status |= OPMSTAT_CHANGED;


		}

		/* Next XOPM */
		Next = Next->Next;
	}
}

			/* Is the area partially outside this XOPM's clip rectangle ? */
			if (!((X >= C->left) && (Y >= C->top)
			 && (X2 < C->left + C->width) && (Y2 < C->top + C->height)))
			{
				/* Yes -> The base XOPM must be updated as well*/
				XOPM->Flags |= XOPM_UPDATE;
				XOPM->OPM.status |= OPMSTAT_CHANGED;
			}

