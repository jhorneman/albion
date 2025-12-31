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
	/* Push error on the error stack */
	ERROR_PushError(Text_print_error,Text_library_name,sizeof(UNWORD),(UNBYTE *) &Error_code);
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
 * NAME      : Init_text
 * FUNCTION  : Initialize text system.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 13:37
 * LAST      : 22.08.94 13:37
 * INPUTS    : UNSHORT Screen_width - Width of the screen in pixels.
 *             UNSHORT Screen_height - Height of the screen in pixels.
 *             struct Font *Font - Pointer to the default font.
 *             struct Textstyle *Textstyle - Pointer to the default text style.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_text(UNSHORT Screen_width, UNSHORT Screen_height,
 struct Textstyle *Textstyle)
{
	UNBYTE *Ptr;

	/* Make default PA */
	Default_PA.Area.left = 0;
	Default_PA.Area.top = 0;
	Default_PA.Area.width = Screen_width;
	Default_PA.Area.height = Screen_height;

	memcpy(&Default_PA.PA_Textstyle, Textstyle, sizeof(struct Textstyle));

	Default_PA.Background_handle = NULL;

	/* Set default font and textstyle */
	Default_font = Textstyle->Font;
	Default_textstyle = Textstyle;

	/* Make output OPM */
	Text_OPM_handle = MEM_Do_allocate(Screen_width * FONT_HEIGHT_MAX,
	 (UNLONG) &Text_OPM, &OPM_ftype);

	Ptr = MEM_Claim_pointer(Text_OPM_handle);
	OPM_New(Screen_width, FONT_HEIGHT_MAX, 1, &Text_OPM, Ptr);
	MEM_Free_pointer(Text_OPM_handle);

	/* Initialize stacks */
	Reset_font_stack();
	Reset_PA_stack();
	Reset_textstyle_stack();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_text
 * FUNCTION  : Terminate the text system.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 13:37
 * LAST      : 22.08.94 13:37
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_text(void)
{
	/* Reset stacks */
	Reset_font_stack();
	Reset_PA_stack();
	Reset_textstyle_stack();

	/* Destroy output OPM */
	OPM_Del(&Text_OPM);
	MEM_Free_memory(Text_OPM_handle);
	Text_OPM_handle = NULL;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_font_stack
 * FUNCTION  : Reset the font stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 09:56
 * LAST      : 22.08.94 16:51
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
	Init_font(Default_font);
	Font_stack[Font_stack_index] = Default_font;
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
Push_font(struct Font *New)
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
Pop_font(void)
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
Change_font(struct Font *New)
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
Init_font(struct Font *Font)
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
 * LAST      : 22.08.94 16:52
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
	Init_textstyle(Default_textstyle);
	Textstyle_stack[Textstyle_stack_index] = Default_textstyle;
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
Pop_textstyle(void)
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
Change_textstyle(struct Textstyle *New)
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
 * LAST      : 22.08.94 16:55
 * INPUTS    : struct Textstyle *Textstyle - Pointer to textstyle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_textstyle(struct Textstyle *Textstyle)
{
	Push_font(Textstyle->Font);
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
 * INPUTS    : struct Textstyle *Textstyle - Pointer to textstyle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_textstyle(struct Textstyle *Textstyle)
{
	Pop_font();
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_PA_stack
 * FUNCTION  : Reset the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 16:57
 * LAST      : 22.08.94 16:57
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
	PA_stack_index = 0;
	Init_PA(&Default_PA);
	PA_stack[PA_stack_index] = &Default_PA;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Push_PA
 * FUNCTION  : Pushes a new PA on the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 16:58
 * LAST      : 22.08.94 16:58
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
	if (PA_stack_index < FONTS_MAX - 1)
	{
		/* Yes -> Increase stack index */
		PA_stack_index++;
		/* Initialize new PA */
		Init_PA(New);
		/* Put new PA on stack */
		PA_stack[PA_stack_index] = New;
	}
	else
		/* No -> Error */
		Text_error(TEXTERR_PA_STACK_OVERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Pop_PA
 * FUNCTION  : Pops a PA from the top of the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 17:00
 * LAST      : 22.08.94 17:00
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
		/* No -> Exit old PA */
		Exit_PA(PA_stack[PA_stack_index]);
		/* Decrease stack index */
		PA_stack_index--;
	else
		/* Yes -> Error */
		Text_error(TEXTERR_PA_STACK_UNDERFLOW);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_PA
 * FUNCTION  : Change the PA on the top of the PA stack.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 17:00
 * LAST      : 22.08.94 17:00
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
	/* Exit old PA */
	Exit_PA(PA_stack[PA_stack_index]);
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
 * LAST      : 22.08.94 17:01
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
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_PA
 * FUNCTION  : Exits a PA.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.08.94 17:01
 * LAST      : 22.08.94 17:01
 * INPUTS    : struct PA *PA - Pointer to PA.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_PA(struct PA *PA)
{
}


/*
;***************************************************************************
; [ Display & reset print buffer ]
;   IN : d1 - Screen Y-coordinate (.w)
;        d4 - Screen X-coordinate (.w)
;        d5 - Buffer X-coordinate (.w)
;        d6 - Number of characters in buffer (.w)
;  OUT : d5 - New buffer X-coordinate (.w)
;        d6 - 0 (.w)
; Changed registers : d5,d6
;***************************************************************************
Print_buffer:
	tst.w	d6			; Any characters in buffer ?
	beq.s	.Exit
	movem.l	d0/d4/d7/a0,-(sp)		; Yes -> Print buffer
	move.w	Print_X,d0		; Get X-coordinate
	move.w	d5,d6			;     width in truncs
	add.w	#15,d6
	and.w	#$fff0,d6
	lsr.w	#4,d6
	move.w	Current_line_height,d7	;     height in pixels
	jsr	Put_line_buffer_shadow
	jsr	Put_line_buffer
	jsr	Clear_print_buffer		; Clear print buffer
	move.w	d4,d0			; Print X = Screen X
	and.w	#$fff0,d0
	move.w	d0,Print_X
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
	moveq.l	#0,d6			; Counter = 0
	movem.l	(sp)+,d0/d4/d7/a0
.Exit:	rts

;***************************************************************************
; [ Clear print buffer ]
; All registers are restored
;***************************************************************************
Clear_print_buffer:
	movem.l	d0/d1/a0,-(sp)
	move.l	#Line_buffer_size,d0	; Clear print buffer
	moveq.l	#0,d1
	lea.l	Line_buffer,a0
	jsr	Fill_memory
	movem.l	(sp)+,d0/d1/a0
	rts
*/

