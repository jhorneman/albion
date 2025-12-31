/************
 * NAME     : TEXT.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-7-1994
 * PROJECT  : Text functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : TEXT.H
 ************/

/* includes */

#include <stdio.h>

#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>

#include <TEXT.H>
#include "TEXTVAR.H"

#include <FONT.H>

/* prototypes */

void Text_error(UNSHORT Error_code);
void Text_print_error(UNCHAR *buffer, UNBYTE *data);

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Text_error
 * FUNCTION  : Report a text processor error.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:12
 * LAST      : 11.07.94 16:12
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Text_error(UNSHORT Error_code)
{
 	/* Push error on the error stack */
	ERROR_PushError(Text_print_error, Text_library_name,
	 sizeof(UNSHORT), (UNBYTE *) &Error_code);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Text_print_error
 * FUNCTION  : Print a text processor error.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:13
 * LAST      : 11.07.94 16:13
 * INPUTS    : UNCHAR *buffer - Pointer to output buffer.
 *             UNBYTE *data - Pointer to error data written by MEM_Error.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Output length should not exceed BBERROR_OUTSTRINGSIZE.
 * SEE ALSO  : TEXT.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Text_print_error(UNCHAR *buffer, UNBYTE *data)
{
	UNSHORT i;

	i = *((UNSHORT *) data); 				/* Get error code */

	if (i>TEXTERR_MAX)	  					/* Catch illegal errors */
		i = 0;

	sprintf((char *)buffer,"%s",Text_error_strings[i]);	/* Print error */
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_textstyle_stack
 * FUNCTION  : Reset the text style stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:10
 * LAST      : 23.08.94 17:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_textstyle_stack(void)
{
	/* Reset stack index */
	Textstyle_stack_index = 0;

	/* Put the default text style on the stack */
	Textstyle_stack[0] = &Default_text_style;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_textstyle
 * FUNCTION  : Pushes a new text style on the text style stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:11
 * LAST      : 23.08.94 17:46
 * INPUTS    : struct Textstyle *New - Pointer to text style.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_textstyle(struct Textstyle *New)
{
	/* Is there room on the stack ? */
	if (Textstyle_stack_index < TEXTSTYLES_MAX - 1)
	{
		/* Yes -> Increase stack index */
		Textstyle_stack_index++;

		/* Put new textstyle on stack */
		Textstyle_stack[Textstyle_stack_index] = New;
	}
	else
		/* No -> Error */
		Text_error(TEXTERR_TEXTSTYLE_STACK_OVERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_textstyle
 * FUNCTION  : Pops a text style from the top of the textstyle stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:13
 * LAST      : 23.08.94 17:47
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_textstyle(void)
{
	/* Is the stack empty ? */
	if (Textstyle_stack_index)
	{
		/* No -> Decrease stack index */
		Textstyle_stack_index--;
	}
	else
		/* Yes -> Error */
		Text_error(TEXTERR_TEXTSTYLE_STACK_UNDERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_textstyle
 * FUNCTION  : Change the text style on the top of the text style stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:14
 * LAST      : 23.08.94 17:48
 * INPUTS    : struct Textstyle *New - Pointer to text style.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_textstyle(struct Textstyle *New)
{
	/* Put new text style on top of stack */
	Textstyle_stack[Textstyle_stack_index] = New;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_PA_stack
 * FUNCTION  : Reset the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 16:57
 * LAST      : 23.08.94 17:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_PA_stack(void)
{
	/* Reset stack index */
	PA_stack_index = 0;

	/* Put the default PA on the stack */
	Init_PA(&Default_PA);
	PA_stack[0] = &Default_PA;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_PA
 * FUNCTION  : Pushes a new PA on the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 16:58
 * LAST      : 29.06.95 11:05
 * INPUTS    : struct PA *New - Pointer to PA.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_PA(struct PA *New)
{
	/* Is there room on the stack ? */
	if (PA_stack_index < PA_MAX - 1)
	{
		/* Yes -> Increase stack index */
		PA_stack_index++;

		/* Put new PA on stack */
		PA_stack[PA_stack_index] = New;

		/* Clear additional PA data */
		PA_data_stack[PA_stack_index].Background_handle = NULL;

		/* Initialize new PA */
		Init_PA(New);
	}
	else
	{
		/* No -> Error */
		Text_error(TEXTERR_PA_STACK_OVERFLOW);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_PA
 * FUNCTION  : Pops a PA from the top of the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 17:00
 * LAST      : 29.06.95 11:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_PA(void)
{
	/* Is the stack empty ? */
	if (PA_stack_index)
	{
		/* No -> PA background still allocated ? */
		if (PA_data_stack[PA_stack_index].Background_handle)
		{
			/* Yes -> Clean up */
			Exit_print_area();
		}

		/* Decrease stack index */
		PA_stack_index--;

		/* Initialize "new" PA */
		Init_PA(PA_stack[PA_stack_index]);
	}
	else
	{
		/* Yes -> Error */
		Text_error(TEXTERR_PA_STACK_UNDERFLOW);
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_PA
 * FUNCTION  : Change the PA on the top of the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 17:00
 * LAST      : 23.08.94 17:49
 * INPUTS    : struct PA *New - Pointer to PA.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_PA(struct PA *New)
{
	/* Initialize new PA */
	Init_PA(New);

	/* Put new PA on top of stack */
	PA_stack[PA_stack_index] = New;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_PA
 * FUNCTION  : Initialize a PA.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 17:01
 * LAST      : 29.06.95 11:02
 * INPUTS    : struct PA *PA - Pointer to PA.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_PA(struct PA *PA)
{
	/* Set current Print Area width */
	PA_width = PA->Area.width;
}

