/************
 * NAME     : STACKS.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 6-7-1994
 * PROJECT  : Stack management functions
 * NOTES    :
 * SEE ALSO : STACKS.C
 ************/

#ifndef STACKS_H
#define STACKS_H

/*
 ** Defines ****************************************************************
 */

/* Error codes */

#define STACKERR_OVERFLOW 		(1)
#define STACKERR_UNDERFLOW		(2)

/*
 ** Structure definitions **************************************************
 */

struct Stack {
	UNSHORT Stack_index;			/* The current stack (array) index */
	UNSHORT Stack_size;			/* The maximum number of stack entries */
	UNLONG *Stack_base;			/* A pointer to the stack array */
	UNLONG Default_entry;		/* The default stack entry */
	void (*Init_stack_entry)(UNLONG);	/* A pointer to a stack entry initialization function (or NULL) */
};

/*
 ** Prototypes *************************************************************
 */

void Reset_stack(struct Stack *Stack);
UNLONG Get_stack_top(struct Stack *Stack);
void Push_stack(struct Stack *Stack, UNLONG New);
void Pop_stack(struct Stack *Stack);
void Change_stack_top(struct Stack *Stack, UNLONG New);

void STACK_Error(UNSHORT Error_code);
void STACK_Print_error(UNCHAR *buffer, UNBYTE *data);

#endif

