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
#include <stdlib.h>

#include <BBDEF.H>
#include <BBMEM.H>

#include <TEXT.H>
#include "TEXTVAR.H"

#include <FONT.H>

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LLC_handler
 * FUNCTION  : Low level text command handler.
 * FILE      : TEXTCOMM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 15:48
 * LAST      : 11.07.95 15:22
 * INPUTS    : UNCHAR *Text - Pointer to string (after command character).
 *             struct LLC_Text_command *Command_list - Pointer to LLC table.
 * RESULT    : UNCHAR * : Pointer to string after command.
 * BUGS      : No known.
 * NOTES     : - All print-function-internal variables that can be changed
 *              by text commands should be implemented as global variables.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
LLC_handler(UNCHAR *Text, struct LLC_Text_command *Command_list)
{
	UNSHORT i;
	UNCHAR Command[4], Char;
	BOOLEAN Flag;

	/* Read command */
	Command[0] = *Text++;
	Command[1] = *Text++;
	Command[2] = *Text++;
	Command[3] = *Text++;

	/* Exception for BLOK command (yuk) */
	/* Is BLOK command ? */
	if ((Command[0] == 'B') && (Command[1] == 'L') &&
	 (Command[2] == 'O') && (Command[3] == 'K'))
	{
		/* Yes -> Search end of text */
		while (*Text)
			Text++;

		/* Exit */
		return Text;
	}

	/* Search text command in command list */
	while (Command_list->Command_handler)
	{
		/* Compare commands */
		Flag = TRUE;
		for (i=0;i<4;i++)
		{
			if (Command_list->Command[i] != Command[i])
			{
				Flag = FALSE;
				break;
			}
		}

		/* Found ? */
		if (Flag)
		{
			/* Yes -> Execute */
			(Command_list->Command_handler)(Text);
			break;
		}

		/* No -> Next command */
		Command_list++;
	}

	/* Seek end of command */
	Char = *Text++;
	while (Char != COMEND_CHAR)
		Char = *Text++;

	return Text;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_LLC_nr
 * FUNCTION  : Get 3-digit number from text command.
 * FILE      : TEXTCOMM.C
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
	UNCHAR Number[] = "000";

	strncpy(Number,Text,3);
	return((UNSHORT) strtoul(Number, NULL, 10));
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : LLC_ ...
 * FUNCTION  : Low level text commands.
 * FILE      : TEXTCOMM.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.07.94 14:04
 * LAST      : 23.08.94 12:17
 * INPUTS    : UNCHAR *Text - Pointer to string (after command).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
LLC_Set_ink(UNCHAR *Text)
{
	Current_text_style.Ink = Get_LLC_nr(Text);
}

void
LLC_Set_shadow(UNCHAR *Text)
{
	Current_text_style.Shadow = Get_LLC_nr(Text);
}

void
LLC_No_shadow(UNCHAR *Text)
{
	Current_text_style.Shadow = 0xFFFF;
}

void
LLC_Set_normal_style(UNCHAR *Text)
{
	Current_text_style.Style = NORMAL_STYLE;
}

void
LLC_Set_fat_style(UNCHAR *Text)
{
	Current_text_style.Style = FAT_STYLE;
}

void
LLC_Set_high_style(UNCHAR *Text)
{
	Current_text_style.Style = HIGH_STYLE;
}

void
LLC_Set_fat_high_style(UNCHAR *Text)
{
	Current_text_style.Style = FAT_HIGH_STYLE;
}

void
LLC_Set_big_style(UNCHAR *Text)
{
	Current_text_style.Style = BIG_STYLE;
}

void
LLC_Set_normal_font(UNCHAR *Text)
{
	Current_text_style.Font = &Normal_font;
}

void
LLC_Set_techno_font(UNCHAR *Text)
{
	Current_text_style.Font = &Techno_font;
}

void
LLC_Set_capital(UNCHAR *Text)
{
	Current_text_style.Style = FAT_HIGH_STYLE;
	Current_text_style.Ink = CAPITAL_COLOUR;
	Current_X += 16;
	Width_without_spaces += 16;
}

void
LLC_Unset_capital(UNCHAR *Text)
{
	Current_text_style.Style = NORMAL_STYLE;
	Current_text_style.Ink = Textstyle_stack[Textstyle_stack_index]->Ink;
}

void
LLC_Default(UNCHAR *Text)
{
	Current_text_style.Font = &Normal_font;
	Current_text_style.Style = NORMAL_STYLE;
	Current_text_style.Ink = Textstyle_stack[Textstyle_stack_index]->Ink;
	Current_text_style.Shadow = Textstyle_stack[Textstyle_stack_index]->Shadow;
	Current_text_style.Justification = PRINT_JUSTIFIED;
}

void
LLC_HJump(UNCHAR *Text)
{
	UNSHORT dX;

	dX = Get_LLC_nr(Text);

	Current_X += dX;
	Width_without_spaces += dX;
}

void
LLC_VJump(UNCHAR *Text)
{
	UNSHORT dY;

	dY = Get_LLC_nr(Text);

	if (dY > Current_line_info.Skip)
		Current_line_info.Skip = dY;
}

void
LLC_Set_centered(UNCHAR *Text)
{
	Current_text_style.Justification = PRINT_CENTERED;
}

void
LLC_Set_left(UNCHAR *Text)
{
	Current_text_style.Justification = PRINT_LEFT;
}

void
LLC_Set_right(UNCHAR *Text)
{
	Current_text_style.Justification = PRINT_RIGHT;
}

void
LLC_Set_justified(UNCHAR *Text)
{
	Current_text_style.Justification = PRINT_JUSTIFIED;
}

