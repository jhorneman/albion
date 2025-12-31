/************
 * NAME     : STACKS.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 6-7-1994
 * PROJECT  : Stack management functions
 * NOTES    : - These functions assume that NULL is 0.
 * SEE ALSO : STACKS.H
 ************/

/* includes */

#include <BBDEF.H>
#include <BBERROR.H>
#include <stdio.h>
#include <STACKS.H>

/* global variable declarations */

UNCHAR MEM_Library_name[] = "Stack management";

UNCHAR *STACK_Error_strings[] = {
	"Stack overflow.",
	"Stack underflow."
};

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_stack
 * FUNCTION  : Resets a stack.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 11:40
 * LAST      : 06.07.94 11:40
 * INPUTS    : struct Stack *Stack - Pointer to stack structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : STACKS.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_stack(struct Stack *Stack)
{
	/* Set the stack index to zero */
	Stack->Stack_index = 0;

	/* Put the default entry on the stack */
	(Stack->Stack_base)[0] = Stack->Default_entry;

	/* Is there an Init_stack_entry function ? */
	if (Stack->Init_stack_entry)
		/* Yes -> Initialize the new entry */
		(Stack->Init_stack_entry)(Stack->Default_entry);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_stack_top
 * FUNCTION  : Get the entry at the top of the stack.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 12:59
 * LAST      : 06.07.94 12:59
 * INPUTS    : struct Stack *Stack - Pointer to stack structure.
 * RESULT    : UNLONG : New entry.
 * BUGS      : No known.
 * SEE ALSO  : STACKS.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_stack_top(struct Stack *Stack)
{
	return((Stack->Stack_base)[Stack->Stack_index]);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_stack
 * FUNCTION  : Pushes an entry on a stack.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 11:43
 * LAST      : 06.07.94 11:43
 * INPUTS    : struct Stack *Stack - Pointer to stack structure.
 *             UNLONG New - New entry.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : STACKS.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_stack(struct Stack *Stack, UNLONG New)
{
	UNSHORT Index;

	/* Get the next stack index */
	Index = (Stack->Stack_index)++;

	/* Is the stack full ? */
	if (Index >= Stack->Stack_size)
		/* Yes -> Error */
		STACK_Error(STACKERR_OVERFLOW);
	else
	{
		/* No -> Put new entry on the stack */
		(Stack->Stack_base)[Index] = New;

		/* Store new stack index */
		Stack->Stack_index = Index;

		/* Is there an Init_stack_entry function ? */
		if (Stack->Init_stack_entry)
			/* Yes -> Initialize the new entry */
			(Stack->Init_stack_entry)(New);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_stack
 * FUNCTION  : Pop an entry from a stack.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 11:53
 * LAST      : 06.07.94 11:53
 * INPUTS    : struct Stack *Stack - Pointer to stack structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : STACKS.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_stack(struct Stack *Stack)
{
	UNSHORT Index;

	/* Get the current stack index */
	Index = (Stack->Stack_index);

	/* Is the stack empty ? */
	if (!Index)
		/* Yes -> Error */
		STACK_Error(STACKERR_OVERFLOW);
	else
	{
		/* No -> Decrease and store stack index */
		Stack->Stack_index = --Index;

		/* Is there an Init_stack_entry function ? */
		if (Stack->Init_stack_entry)
			/* Yes -> Initialize the old entry */
			(Stack->Init_stack_entry)((Stack->Stack_base)[Index]);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_stack_top
 * FUNCTION  : Change the entry at the top of the stack.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 11:59
 * LAST      : 06.07.94 11:59
 * INPUTS    : struct Stack *Stack - Pointer to stack structure.
 *             UNLONG New - New entry.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : STACKS.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_stack_top(struct Stack *Stack, UNLONG New)
{
	UNSHORT Index;

	/* Get the current stack index */
	Index = (Stack->Stack_index);

	/* Put new entry on the stack */
	(Stack->Stack_base)[Index] = New;

	/* Is there an Init_stack_entry function ? */
	if (Stack->Init_stack_entry)
		/* Yes -> Initialize the new entry */
		(Stack->Init_stack_entry)(New);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : STACK_Error
 * FUNCTION  : Report a stack management error.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 11:57
 * LAST      : 06.07.94 11:57
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : STACKS.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
MEM_Error(UNSHORT Error_code)
{
	char X[200];

	/* Push error on the error stack */
/*	ERROR_PushError(STACK_Print_error,STACK_Library_name,sizeof(UNSHORT),(UNBYTE *) &Error_code);
*/

	STACK_Print_error(&X[0],(UNBYTE *) &Error_code);
	printf("ERROR : %s\n",X);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : STACK_Print_error
 * FUNCTION  : Print a stack management error.
 * FILE      : STACKS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.07.94 11:57
 * LAST      : 06.07.94 11:57
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by STACK_Error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : STACKS.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
STACK_Print_error(UNCHAR *buffer, UNBYTE *data)
{
	UNSHORT i;

	i = *((UNSHORT *) data);	 /* Get error code */

	sprintf((char *)buffer,"%s",STACK_Error_strings[i]);	/* Print error */
}

