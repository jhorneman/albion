/************
 * NAME     : TEXTCOMM.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 8-7-1994
 * PROJECT  : Text functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : TEXT.H
 ************/

/* includes */

#include <string.h>
#include <BBDEF.H>
#include <BBMEM.H>
#include <TEXT.H>
#include "TEXTVAR.H"

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LLC_handler
 * FUNCTION  : Low level text command handler.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 15:48
 * LAST      : 11.07.94 15:48
 * INPUTS    : UNCHAR *Text - Pointer to string (after command character).
 *             struct Text_command *Command_list - Pointer to LLC table.
 *             UNSHORT *Screen_X - Pointer to screen X-coordinate.
 *             UNSHORT *Buffer_X - Pointer to buffer X-coordinate.
 * RESULT    : UNCHAR * : Pointer to string after command.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
LLC_handler(UNCHAR *Text, struct Text_command *Command_list,
 UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	UNCHAR Command[4], Char;

	/* Read command */
	Command[0] = *Text++;
	Command[1] = *Text++;
	Command[2] = *Text++;
	Command[3] = *Text++;

	/* Search text command in command list */
	while (!Command_list->Command)
	{
		/* Found command ? */
		if (Command_list->Command == (UNLONG) Command)
		{
			/* Yes -> Execute */
			(Command_list->LLC)(Text,Screen_X,Buffer_X);
			break;
		}
	}

	/* Seek end of command */
	Char = *Text++;
	while (Char != COMEND_CHAR)
		Char = *Text++;

	return(Text);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :
 * FUNCTION  : Low level text commands.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 14:04
 * LAST      : 12.07.94 14:04
 * INPUTS    : UNCHAR *Text - Pointer to string (after command).
 *             UNSHORT *Screen_X - Pointer to screen X-coordinate.
 *             UNSHORT *Buffer_X - Pointer to buffer X-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
LLC_(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{

}

void
LLC_Set_ink(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Print_buffer();
	Ink_colour = Get_LLC_nr(Text);
}

void
LLC_Set_shadow(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Print_buffer();
	Shadow_colour = Get_LLC_nr(Text);
}

void
LLC_No_shadow(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Print_buffer();
	Shadow_colour = 0xFFFF;
}

void
LLC_Set_normal_style(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Current_text_style = Normal_style;
}

void
LLC_Set_fat_style(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Current_text_style = Fat_style;
}

void
LLC_Set_high_style(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Current_text_style = High_style;
}

void
LLC_Set_fat_high_style(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Current_text_style = Fat_high_style;
}

void
LLC_Set_normal_font(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Change_font(&Default_font);
}

void
LLC_Set_techno_font(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Change_font(&Techno_font);
}

void
LLC_Set_capital(UNCHAR *Text, UNSHORT *Screen_X, UNSHORT *Buffer_X)
{
	Print_buffer();

	Current_text_style = Fat_high_style;
	Ink_colour = CAPITAL_COLOUR;
}


/*
LLC_Set_capital:
	movem.l	d0/a0,-(sp)
	jsr	Print_buffer
	move.l	#Fat_high_style,Current_text_style
	move.w	#Capital_colour,Ink_colour
	move.l	PA_Sp,a0			; Get left border
	move.l	(a0),a0
	move.w	X1(a0),d4
	add.w	#10,d4			; Jump
	move.w	d4,d0			; Print X = Screen X
	and.w	#$fff0,d0
	move.w	d0,Print_X
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
	movem.l	(sp)+,d0/a0
	rts

LLC_Set_capital2:
	move.l	#Fat_high_style,Current_text_style
	moveq.l	#10,d4			; Jump
	rts

LLC_Unset_capital:
	move.l	a0,-(sp)
	jsr	Print_buffer
	move.l	#Normal_style,Current_text_style
	move.l	PA_Sp,a0
	move.l	(a0),a0
	move.w	PA_Ink(a0),Ink_colour
	move.l	(sp)+,a0
	rts

LLC_Unset_capital2:
	move.l	#Normal_style,Current_text_style
	rts

LLC_HJump:
	movem.l	d0/a1,-(sp)
	jsr	Get_LLC_nr3		; Get jump
	cmp.w	PA_width,d0		; Too far ?
	bpl.s	.Exit
	jsr	Print_buffer
	move.l	PA_Sp,a1			; Get left border
	move.l	(a1),a1
	move.w	X1(a1),d4
	add.w	d0,d4			; Jump
	move.w	d4,d0			; Print X = Screen X
	and.w	#$fff0,d0
	move.w	d0,Print_X
	move.w	d4,d5			; Buffer X = Screen X AND $000f
	and.w	#$000f,d5
.Exit:	movem.l	(sp)+,d0/a1
	rts

LLC_HJump2:
	movem.l	d0/a1,-(sp)
	jsr	Get_LLC_nr3		; Get jump
	cmp.w	PA_width,d0		; Too far ?
	bpl.s	.Exit
	move.w	d0,d4			; Jump
.Exit:	movem.l	(sp)+,d0/a1
	rts

LLC_Default:
	move.l	a0,-(sp)
	jsr	Print_buffer
	lea.l	Default_Font,a0		; Default font
	jsr	Change_Font
	move.l	#Normal_style,Current_text_style
	move.l	PA_Sp,a0			; Get current PA
	move.l	(a0),a0
	move.w	PA_Ink(a0),Ink_colour	; Default colours
	move.w	PA_Shadow(a0),Shadow_colour
	move.l	(sp)+,a0
	rts

LLC_Default2:
	move.l	a0,-(sp)
	lea.l	Default_Font,a0		; Default font
	jsr	Change_Font
	move.l	#Normal_style,Current_text_style
	move.l	(sp)+,a0
	rts

*/




/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LLCA_handler
 * FUNCTION  : Low level text command handler - for line analysis only.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 17:24
 * LAST      : 11.07.94 17:24
 * INPUTS    : UNCHAR *Text - Pointer to string (after command character).
 *             struct Text_command *Command_list - Pointer to LLC table.
 *             UNSHORT *Line_width - Pointer to line width.
 *             UNSHORT *Line_width2 - Pointer to line width without spaces.
 *             struct Line_info *Current - Pointer to line info data structure.
 * RESULT    : UNCHAR * : Pointer to string after command.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
LLCA_handler(UNCHAR *Text, UNSHORT *Line_width, UNSHORT *Line_width2,
 struct Line_info *Current)
{
	UNCHAR Command[4], Char;
	struct Text_command *Command_list = &LLCA_commands[0];

	/* Read command */
	Command[0] = *Text++;
	Command[1] = *Text++;
	Command[2] = *Text++;
	Command[3] = *Text++;

	/* Search text command in command list */
	while (!Command_list->Command)
	{
		/* Found command ? */
		if (Command_list->Command == (UNLONG) Command)
		{
			/* Yes -> Execute */
			(Command_list->LLCA)(Text,Line_width,Line_width2,Current);
			break;
		}
	}

	/* Seek end of command */
	Char = *Text++;
	while (Char != COMEND_CHAR)
		Char = *Text++;

	return(Text);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_LLC_nr
 * FUNCTION  : Get 3-digit number from text command.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 14:08
 * LAST      : 12.07.94 14:08
 * INPUTS    : UNCHAR *Text - Pointer to string (after command).
 * RESULT    : UNSHORT : Number.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_LLC_nr(UNCHAR *Text)
{
	UNCHAR *Number = "000";

	strncpy(Number,Text,3);
	return((UNSHORT) strtol(Number));
}

