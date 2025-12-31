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
#include <string.h>
#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>
#include <TEXT.H>
#include "TEXTVAR.H"

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
	UNCHAR X[200];

	/* Push error on the error stack */
/*	ERROR_PushError(Text_print_error,Text_library_name,sizeof(UNWORD),(UNBYTE *) &Error_code);
*/

	Text_print_error(&X[0],(UNBYTE *) &Error_code);
	printf("ERROR : %s\n",X);
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
 * NAME      : Reset_font_stack
 * FUNCTION  : Reset the font stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 09:56
 * LAST      : 12.07.94 09:56
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_font_stack(void)
{
	Font_stack_index = 0;
	/* Push_font(&Default_font); */
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_font
 * FUNCTION  : Pushes a new font on the font stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:17
 * LAST      : 11.07.94 16:17
 * INPUTS    : struct Font *New - Pointer to font.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_font (struct Font *New)
{
	/* Is there room on the stack ? */
	if (Font_stack_index < FONTS_MAX - 1)
	{
		/* Yes -> Increase stack index */
		Font_stack_index++;
		/* Initialize new font */
		Init_font(New);
		/* Put new font on stack */
		Font_stack[Font_stack_index] = New;
	}
	else
		/* No -> Error */
		Text_error(TEXTERR_FONT_STACK_OVERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_font
 * FUNCTION  : Pops a font from the top of the font stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:21
 * LAST      : 11.07.94 16:21
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_font (void)
{
	/* Is the stack empty ? */
	if (Font_stack_index)
		/* No -> Decrease stack index */
		Font_stack_index--;
	else
		/* Yes -> Error */
		Text_error(TEXTERR_FONT_STACK_UNDERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_font
 * FUNCTION  : Change the font on the top of the font stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:24
 * LAST      : 11.07.94 16:24
 * INPUTS    : struct Font *New - Pointer to font.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_font (struct Font *New)
{
	/* Initialize new font */
	Init_font(New);
	/* Put new font on top of stack */
	Font_stack[Font_stack_index] = New;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_font
 * FUNCTION  : Initialize a font.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 16:25
 * LAST      : 11.07.94 16:25
 * INPUTS    : struct Font *Font - Pointer to font.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_font (struct Font *Font)
{
	UNSHORT i;
	struct Fontstyle *Style;
	struct Kerning_pair *Table;
	UNBYTE *Translation;

	/* Check all font styles */
	for (i=0;i<STYLES_MAX;i++)
	{
		/* Get font style */
		Style = Font->Styles[i];
		/* Kerning table available ? */
		if (Style->Kerning_table)
		{
			/* Yes -> Already converted ? */
			Table = Style->Kerning_table;
			if (*((UNSHORT *) Table) != 0xFFFF)
			{
				/* No -> Convert */
				*((UNSHORT *) Table) = 0xFFFF;
				Table++;

				Translation = Style->Translation;

				/* Until the table has ended */
				while (*(UNSHORT *) Table)
				{
					/* Convert kerning pairs */
					Table->First = Translation[((UNSHORT) Table->First) - 32];
					Table->Second = Translation[((UNSHORT) Table->Second) -32];
					/* Next entry */
					Table++;
				}
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_textstyle_stack
 * FUNCTION  : Reset the text style stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:10
 * LAST      : 12.07.94 10:10
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
	Textstyle_stack_index = 0;
	/* Push_textstyle(&Default_textstyle); */
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_textstyle
 * FUNCTION  : Pushes a new text style on the text style stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:11
 * LAST      : 12.07.94 10:11
 * INPUTS    : struct Textstyle *New - Pointer to text styke.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Push_textstyle (struct Textstyle *New)
{
	/* Is there room on the stack ? */
	if (Textstyle_stack_index < FONTS_MAX - 1)
	{
		/* Yes -> Increase stack index */
		Textstyle_stack_index++;
		/* Initialize new text style */
		Init_textstyle(New);
		/* Put new textstyle on stack */
		Textstyle_stack[Textstyle_stack_index] = New;
	}
	else
		/* No -> Error */
		Text_error(TEXTERR_STYLE_STACK_OVERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_textstyle
 * FUNCTION  : Pops a text style from the top of the textstyle stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:13
 * LAST      : 12.07.94 10:13
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Pop_textstyle (void)
{
	/* Is the stack empty ? */
	if (Textstyle_stack_index)
		/* No -> Exit old text style */
		Exit_textstyle(Textstyle_stack[Textstyle_stack_index]);
		/* Decrease stack index */
		Textstyle_stack_index--;
	else
		/* Yes -> Error */
		Text_error(TEXTERR_STYLE_STACK_UNDERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_textstyle
 * FUNCTION  : Change the text style on the top of the text style stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:14
 * LAST      : 12.07.94 10:14
 * INPUTS    : struct Textstyle *New - Pointer to text style.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_textstyle (struct Textstyle *New)
{
	/* Exit old text style */
	Exit_textstyle(Textstyle_stack[Textstyle_stack_index]);
	/* Initialize new text style */
	Init_textstyle(New);
	/* Put new text style on top of stack */
	Textstyle_stack[Textstyle_stack_index] = New;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_textstyle
 * FUNCTION  : Initialize a text style.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 10:15
 * LAST      : 12.07.94 10:15
 * INPUTS    : struct Textstyle *Textstyle - Pointer to textstyle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_textstyle (struct Textstyle *Textstyle)
{

}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_textstyle
 * FUNCTION  : Exits a text style.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 13:39
 * LAST      : 12.07.94 13:39
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_textstyle (void)
{
	Pop_font();
}

