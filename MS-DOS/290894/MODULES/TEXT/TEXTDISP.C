/************
 * NAME     : TEXTDISP.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-8-1994
 * PROJECT  : Text functions
 * NOTES    : - These functions assume that NULL and FALSE are 0.
 * SEE ALSO : TEXT.H
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>
#include <BBOPM.H>

#include <XLOAD.H>
#include <TEXT.H>
#include "TEXTVAR.H"

#include <FONT.H>

/* global variables */
UNSHORT Current_base_line;

UNSHORT Space_table[LINE_LENGTH_MAX];

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_string
 * FUNCTION  : Print a normal string.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 11:40
 * LAST      : 23.08.94 11:40
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             UNCHAR *String - Pointer to string.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Do not use this function to display processed strings.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *String)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNBYTE Previous_character, Char2;
	UNCHAR Char;

	/* Set X-coordinate */
	Current_X = X;
	Start_X = PA_stack[PA_stack_index]->Area.left;

	/* No previous character */
	Previous_character = 0xFF;

	/* Get current text style */
	memcpy(&Current_text_style, Textstyle_stack[Textstyle_stack_index],
	 sizeof(struct Textstyle));

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	/* Set current base line */
	Current_base_line = Current_font->Base_line;

	/* While the line still fits in the OPM... */
	while (Current_X < OPM->width)
	{
		/* Read a character from the string */
		Char = *String++;

		/* Exit if EOL */
		if (!Char)
			break;

		/* Is this a command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Handle it */
			String = LLC_handler(String, &LLC_commands[0]);

			/* No previous character */
			Previous_character = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Next character */
			continue;
		}

		/* Is this a space ? */
		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Current_X += Current_font->Width_of_space;

			/* No previous character */
			Previous_character = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Char2)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != 0xFF)
		{
			/* Yes -> Is there a kerning table ? */
			Table = (struct Kerning_pair *) Current_font->Kerning_table;
			if (Table)
			{
				/* Yes -> Look for kerning pair */
				/* (The first kerning pair is a flag and must be skipped !) */
				Table++;
				while (*(UNSHORT *) Table)
				{
					/* Found ? */
					if ((Table->First == Previous_character) && (Table->Second == Char2))
					{
						/* Yes -> Remove pixel */
						Current_X--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}
		/* Save character */
		Previous_character = Char2;

		/* Print character */
		(Current_font->Print_char)(OPM, Current_X, Y, Current_font, Char2, Current_text_style.Ink,
		 Current_text_style.Shadow);

		/* Skip character width in pixels */
		Current_X += (UNSHORT)(Current_font->Width_table)[Char2]
		 + Current_font->Between_width;
	}
}

/*
;***************************************************************************
; [ Put a processed text line on the screen, centered horizontally ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Width of area in which to center (.w)
;        a0 - Pointer to text line (.l)
; All registers are restored
;***************************************************************************
Put_centered_text_line:
	movem.l	d0/d2,-(sp)
	move.w	d0,d3			; Save
	jsr	Get_line_length		; Get width
	exg.l	d0,d3
	sub.w	d3,d2
	bmi.s	.Skip			; Possible ?
	addq.w	#1,d2			; Yes
	lsr.w	#1,d2
	add.w	d2,d0
.Skip:	jsr	Put_text_line
	movem.l	(sp)+,d0/d2
	rts

;***************************************************************************
; [ Put a processed text line on the screen, centered in a box ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of box (.w)
;        d3 - Height of box (.w)
;        a0 - Pointer to text line (.l)
; All registers are restored
;***************************************************************************
Put_centered_box_text_line:
	movem.l	d0-d5,-(sp)
	move.w	d0,d4			; Save
	move.w	d1,d5
	jsr	Get_line_size		; Get size
	exg.l	d0,d4
	exg.l	d1,d5
	sub.w	d4,d2			; Centre horizontally
	bmi.s	.Skip1
	addq.w	#1,d2
	lsr.w	#1,d2
	add.w	d2,d0
.Skip1:	sub.w	d5,d3			; Centre vertically
	bmi.s	.Skip2
	addq.w	#1,d3
	lsr.w	#1,d3
	add.w	d3,d1
.Skip2:	jsr	Put_text_line
	movem.l	(sp)+,d0-d5
	rts

 */

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_processed_string
 * FUNCTION  : Print a processed: string.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 13:19
 * LAST      : 23.08.94 13:19
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             UNCHAR *String - Pointer to string (preceded by line info).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_processed_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *String)
{
	struct Line_info *Line;

	/* Get line info */
	Line = (struct Line_info *) String;
	String += sizeof(struct Line_info);

	/* Set vertical data */
	Current_base_line = (UNSHORT) Line->Base_line;

	/* Set current text style */
	memcpy(&Current_text_style, &(Line->Style), sizeof(struct Textstyle));

	switch(Line->Style.Justification)
	{
		case PRINT_LEFT:
		{
			/* Just print */
			Print_normal_string(OPM, X, Y, String);
			break;
		}
		case PRINT_CENTERED:
		{
			SISHORT dX;

			/* Calculate displacement */
			dX = PA_width - Line->Width;

			/* Is centering possible ? */
			if (dX > 0)
				/* Yes */
				X += dX / 2;
			else
				/* No -> Set for next time */
				Line->Style.Justification = PRINT_LEFT;

			/* Print */
			Print_normal_string(OPM, X, Y, String);

			break;
		}
		case PRINT_RIGHT:
		{
			/* Move string to the right */
			X += (PA_width - Line->Width);

			/* Print */
			Print_normal_string(OPM, X, Y, String);

			break;
		}
		case PRINT_JUSTIFIED:
		{
			/* Is justification possible ? */
			if (Prepare_justification(Line))
				/* Yes -> Print justified string */
				Print_justified_string(OPM, X, Y, String);
			else
			{
				/* No -> Set for next time */
				Line->Style.Justification = PRINT_LEFT;

				/* Print */
				Print_normal_string(OPM, X, Y, String);
			}
			break;
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_normal_string
 * FUNCTION  : Print a normal string.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 13:23
 * LAST      : 23.08.94 13:23
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             UNCHAR *String - Pointer to string.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Current_text_style has already been set by
 *              Print_processed_string().
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_normal_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *String)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNBYTE Previous_character, Char2;
	UNCHAR Char;

	/* Set X-coordinate */
	Current_X = X;
	Start_X = PA_stack[PA_stack_index]->Area.left;

	/* No previous character */
	Previous_character = 0xFF;

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	/* While the line still fits in the OPM... */
	while (Current_X < OPM->width)
	{
		/* Read a character from the string */
		Char = *String++;

		/* Exit if EOL */
		if (!Char)
			break;

		/* Is this a command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Handle it */
			String = LLC_handler(String, &LLC_commands[0]);

			/* No previous character */
			Previous_character = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Next character */
			continue;
		}

		/* Is this a space ? */
		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Current_X += Current_font->Width_of_space;

			/* No previous character */
			Previous_character = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Char2)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != 0xFF)
		{
			/* Yes -> Is there a kerning table ? */
			Table = (struct Kerning_pair *) Current_font->Kerning_table;
			if (Table)
			{
				/* Yes -> Look for kerning pair */
				/* (The first kerning pair is a flag and must be skipped !) */
				Table++;
				while (*(UNSHORT *) Table)
				{
					/* Found ? */
					if ((Table->First == Previous_character) && (Table->Second == Char2))
					{
						/* Yes -> Remove pixel */
						Current_X--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}
		/* Save character */
		Previous_character = Char2;

		/* Print character */
		(Current_font->Print_char)(OPM, Current_X, Y, Current_font, Char2, Current_text_style.Ink,
		 Current_text_style.Shadow);

		/* Skip character width in pixels */
		Current_X += (UNSHORT)(Current_font->Width_table)[Char2]
		 + Current_font->Between_width;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_justified_string
 * FUNCTION  : Print a justified string.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 13:15
 * LAST      : 23.08.94 13:15
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             UNCHAR *String - Pointer to string.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Current_text_style has already been set by
 *              Print_processed_string().
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_justified_string(struct OPM *OPM, SISHORT X, SISHORT Y, UNCHAR *String)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNSHORT Space_index;
	UNBYTE Previous_character, Char2;
	UNCHAR Char;

	/* Set X-coordinate */
	Current_X = X;
	Start_X = PA_stack[PA_stack_index]->Area.left;

	/* No previous character */
	Previous_character = 0xFF;

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	/* Start with the first space */
	Space_index = 0;

	/* While the line still fits in the OPM... */
	while (Current_X < OPM->width)
	{
		/* Read a character from the string */
		Char = *String++;

		/* Exit if EOL */
		if (!Char)
			break;

		/* Is this a command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Handle it */
			String = LLC_handler(String, &LLC_commands[0]);

			/* No previous character */
			Previous_character = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Next character */
			continue;
		}

		/* Is this a space ? */
		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Current_X += Space_table[Space_index];
			Space_index++;

			/* No previous character */
			Previous_character = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Char2)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != 0xFF)
		{
			/* Yes -> Is there a kerning table ? */
			Table = (struct Kerning_pair *) Current_font->Kerning_table;
			if (Table)
			{
				/* Yes -> Look for kerning pair */
				/* (The first kerning pair is a flag and must be skipped !) */
				Table++;
				while (*(UNSHORT *) Table)
				{
					/* Found ? */
					if ((Table->First == Previous_character) && (Table->Second == Char2))
					{
						/* Yes -> Remove pixel */
						Current_X--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}
		/* Save character */
		Previous_character = Char2;

		/* Print character */
		(Current_font->Print_char)(OPM, Current_X, Y, Current_font, Char2, Current_text_style.Ink,
		 Current_text_style.Shadow);

		/* Skip character width in pixels */
		Current_X += (UNSHORT)(Current_font->Width_table)[Char2]
		 + Current_font->Between_width;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_justification
 * FUNCTION  : Prepare a text line for justification.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 12:58
 * LAST      : 24.08.94 12:58
 * INPUTS    : struct Line_info *Line - Pointer to current line info.
 * RESULT    : BOOLEAN : Possible or impossible.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Prepare_justification(struct Line_info *Line)
{
	SISHORT Free_pixels;
	UNSHORT Space_width, Remaining_pixels;
	UNSHORT i,j, Seed;

	/* Exit if there are no spaces in this line */
	if (!Line->Nr_of_spaces)
		return(FALSE);

	/* Calculate amount of free pixels */
	Free_pixels = PA_width - Line->Width_without_spaces - 1;

	/* Exit if zero or less */
	if (Free_pixels <= 0)
		return(FALSE);

	/* Calculate space width and remaining pixels */
	Space_width = Free_pixels / Line->Nr_of_spaces;
	Remaining_pixels = Free_pixels % Line->Nr_of_spaces;

	/* Exit if space width is too small */
	if (Space_width < MINIMUM_SPACE)
		return(FALSE);

	/* Divide pixels over spaces */
	for (i=0;i<Line->Nr_of_spaces;i++)
		Space_table[i] = Space_width;

	/* Divide remaining pixels */
	Seed = 0;
	for (i=0;i<Remaining_pixels;i++)
	{
		while (TRUE)
		{
			/* Select a space */
			Seed = Seed * 17 + 87;
			j = Seed % Line->Nr_of_spaces;

			/* Has this space already been widened ? */
			if (Space_table[j] == Space_width)
			{
				/* No -> Widen it */
				Space_table[j]++;
				/* Next space */
				break;
			}
		}
	}

	/* It can be done */
	return(TRUE);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_character
 * FUNCTION  : Print a normal character.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 11:30
 * LAST      : 23.08.94 11:30
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             struct Fontstyle *Fontstyle - Pointer to font style.
 *             UNSHORT Character - Character index (1...)
 *             UNSHORT Ink - Ink colour.
 *             UNSHORT Shadow - Shadow colour (0xFFFF = no shadow).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow)
{
	UNBYTE *Ptr;

	/* Get address of character graphics */
	Ptr = MEM_Claim_pointer(Fontstyle->Graphics_handle)
	 + Fontstyle->Graphics_offset + ((Character - 1) * Fontstyle->Raw_size);

	/* Calculate vertical offset */
	Y += (Current_base_line - Fontstyle->Base_line);

	/* Is there a shadow ? */
	//if (Shadow != 0xFFFF)
		/* Yes -> Display it */
		//Put_silhouette(OPM, X+1, Y+1, Fontstyle->Raw_width,
		// Fontstyle->Raw_height, Ptr, Shadow);

	/* Display the character */
/*	Put_unmasked_block(OPM, X, Y, Fontstyle->Raw_width, Fontstyle->Raw_height,
	 Ptr, 1); */
	Put_character(OPM, X, Y, Fontstyle->Raw_width, Fontstyle->Raw_height,
	 Ptr, 1);

	MEM_Free_pointer(Fontstyle->Graphics_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_fat_character
 * FUNCTION  : Print a fat character.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.94 11:30
 * LAST      : 23.08.94 11:30
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             struct Fontstyle *Fontstyle - Pointer to font style.
 *             UNSHORT Character - Character index (1...)
 *             UNSHORT Ink - Ink colour.
 *             UNSHORT Shadow - Shadow colour (0xFFFF = no shadow).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_fat_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow)
{
	UNBYTE *Ptr;

	/* Get address of character graphics */
	Ptr = MEM_Claim_pointer(Fontstyle->Graphics_handle)
	 + Fontstyle->Graphics_offset + ((Character - 1) * Fontstyle->Raw_size);

	/* Calculate vertical offset */
	Y += (Current_base_line - Fontstyle->Base_line);

	/* Is there a shadow ? */
/*	if (Shadow != 0xFFFF)
  	{ */
		/* Yes -> Display it */
/*		Put_silhouette(OPM, X+1, Y+1, Fontstyle->Raw_width,
		 Fontstyle->Raw_height, Ptr, Shadow);
		Put_silhouette(OPM, X+2, Y+1, Fontstyle->Raw_width,
		 Fontstyle->Raw_height, Ptr, Shadow);
	} */

	/* Display the character */
	Put_fat_character(OPM, X, Y, Fontstyle->Raw_width, Fontstyle->Raw_height,
	 Ptr, 1);

	MEM_Free_pointer(Fontstyle->Graphics_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Print_high_character
 * FUNCTION  : Print a high character.
 * FILE      : TEXTDISP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.08.94 15:21
 * LAST      : 27.08.94 15:21
 * INPUTS    : struct OPM *OPM - Pointer to target OPM.
 *             SISHORT X - Left X-coordinate in OPM.
 *             SISHORT Y - Top Y-coordinate in OPM.
 *             struct Fontstyle *Fontstyle - Pointer to font style.
 *             UNSHORT Character - Character index (1...)
 *             UNSHORT Ink - Ink colour.
 *             UNSHORT Shadow - Shadow colour (0xFFFF = no shadow).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Print_high_character(struct OPM *OPM, SISHORT X, SISHORT Y,
 struct Fontstyle *Fontstyle, UNSHORT Character, UNSHORT Ink, UNSHORT Shadow)
{
	UNBYTE *Ptr;

	/* Get address of character graphics */
	Ptr = MEM_Claim_pointer(Fontstyle->Graphics_handle)
	 + Fontstyle->Graphics_offset + ((Character - 1) * Fontstyle->Raw_size);

	/* Calculate vertical offset */
	Y += (Current_base_line - Fontstyle->Base_line);

	/* Is there a shadow ? */
//	if (Shadow != 0xFFFF)
		/* Yes -> Display it */
//		Put_high_character(OPM, X+1, Y+1, Fontstyle->Raw_width,
//		 Fontstyle->Raw_height, Ptr, Shadow);

	/* Display the character */
	Put_high_character(OPM, X, Y, Fontstyle->Raw_width, Fontstyle->Raw_height,
	 Ptr, 0);

	MEM_Free_pointer(Fontstyle->Graphics_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_line_width
 * FUNCTION  : Get the width in pixels of a text line.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 12:38
 * LAST      : 24.08.94 12:38
 * INPUTS    : UNCHAR *Text - Pointer to text.
 * RESULT    : UNSHORT : Width in pixels.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_line_width(UNCHAR *Text)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNBYTE Previous_character, Char2;
	UNCHAR Char;

	/* Set X-coordinate */
	Current_X = 0;
	Start_X = 0;

	/* No previous character */
	Previous_character = 0xFF;

	/* Get current text style */
	memcpy(&Current_text_style, Textstyle_stack[Textstyle_stack_index],
	 sizeof(struct Textstyle));

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	while (TRUE)
	{
		/* Read a character from the string */
		Char = *Text++;

		/* Exit if EOL */
		if (!Char)
			break;

		/* Is this a command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Handle it */
			Text = LLC_handler(Text, &LLCW_commands[0]);

			/* No previous character */
			Previous_character = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Next character */
			continue;
		}

		/* Is this a space ? */
		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Current_X += Current_font->Width_of_space;

			/* No previous character */
			Previous_character = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Char2)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != 0xFF)
		{
			/* Yes -> Is there a kerning table ? */
			Table = (struct Kerning_pair *) Current_font->Kerning_table;
			if (Table)
			{
				/* Yes -> Look for kerning pair */
				/* (The first kerning pair is a flag and must be skipped !) */
				Table++;
				while (*(UNSHORT *) Table)
				{
					/* Found ? */
					if ((Table->First == Previous_character) && (Table->Second == Char2))
					{
						/* Yes -> Remove pixel */
						Current_X--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}
		/* Save character */
		Previous_character = Char2;

		/* Skip character width in pixels */
		Current_X += (UNSHORT)(Current_font->Width_table)[Char2]
		 + Current_font->Between_width;
	}

	/* Remove last pixels */
	Current_X -= Current_font->Between_width;

	return(Current_X);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_line_size
 * FUNCTION  : Get the width and height in pixels of a text line.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.08.94 12:52
 * LAST      : 24.08.94 12:52
 * INPUTS    : UNCHAR *Text - Pointer to text.
 *             UNSHORT *Width - Pointer to width variable.
 *             UNSHORT *Height - Pointer to height variable.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_line_size(UNCHAR *Text, UNSHORT *Width, UNSHORT *Height)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNSHORT Current_height;
	UNBYTE Previous_character, Char2;
	UNCHAR Char;

	/* Set X-coordinate */
	Current_X = 0;
	Start_X = 0;

	/* No previous character */
	Previous_character = 0xFF;

	/* Get current text style */
	memcpy(&Current_text_style, Textstyle_stack[Textstyle_stack_index],
	 sizeof(struct Textstyle));

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	/* Get current font height */
	Current_height = Current_font->Raw_height;

	while (TRUE)
	{
		/* Read a character from the string */
		Char = *Text++;

		/* Exit if EOL */
		if (!Char)
			break;

		/* Is this a command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Handle it */
			Text = LLC_handler(Text, &LLCW_commands[0]);

			/* No previous character */
			Previous_character = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Test new font height */
			if (Current_font->Raw_height > Current_height)
				Current_height = Current_font->Raw_height;

			/* Next character */
			continue;
		}

		/* Is this a space ? */
		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Current_X += Current_font->Width_of_space;

			/* No previous character */
			Previous_character = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Char2)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != 0xFF)
		{
			/* Yes -> Is there a kerning table ? */
			Table = (struct Kerning_pair *) Current_font->Kerning_table;
			if (Table)
			{
				/* Yes -> Look for kerning pair */
				/* (The first kerning pair is a flag and must be skipped !) */
				Table++;
				while (*(UNSHORT *) Table)
				{
					/* Found ? */
					if ((Table->First == Previous_character) && (Table->Second == Char2))
					{
						/* Yes -> Remove pixel */
						Current_X--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}
		/* Save character */
		Previous_character = Char2;

		/* Skip character width in pixels */
		Current_X += (UNSHORT)(Current_font->Width_table)[Char2]
		 + Current_font->Between_width;
	}

	/* Remove last pixels */
	Current_X -= Current_font->Between_width;

	/* Store results */
	*Width = Current_X;
	*Height = Current_height;
}

