/************
 * NAME     : TEXTPROC.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 12-7-1994
 * PROJECT  : Text functions
 * NOTES    :
 * SEE ALSO : TEXTVAR.C, TEXT.C, TEXT.H
 ************/

/* includes */

#include <string.h>
#include <BBDEF.H>
#include <BBERROR.H>
#include <BBMEM.H>
#include <TEXT.H>
#include "TEXTVAR.H"

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Process_text
 * FUNCTION  : Process a single text.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 11:30
 * LAST      : 08.07.94 11:30
 * INPUTS    : UNCHAR *Text - Pointer to text string.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Process_text(UNCHAR *Text, struct Processed_text *Processed)
{
	UNCHAR *List[2];

	/* Build list with one entry */
	List[0] = Text;
	List[1] = NULL;

	/* Process text list */
	Process_text_list(&List[0], Processed);
};

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Process_text_list
 * FUNCTION  : Process a list of texts.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 11:33
 * LAST      : 08.07.94 11:33
 * INPUTS    : UNCHAR *Text_list[] - Pointer to a NULL-terminated list of
 *              text strings.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Process_text_list(UNCHAR **Text_list, struct Processed_text *Processed)
{
	struct Font *Old_font;
	UNCHAR *Text_ptr, Char, Command[4], *Raw_ptr, *End_of_RTPB;
	UNCHAR *Wrap_ptr, *Start_of_command, *Space_ptr;
	MEM_HANDLE Handle;
	struct Text_command *Command_list;

	/* Save current font */
	Old_font = Font_stack[Font_stack_index];

	/* Reset print parameters */
	Current_justification = PRINT_JUSTIFIED;
	Current_text_style = NORMAL_STYLE;

	/*** MERGE ALL STRINGS INTO ONE ***/

	/* Allocate first raw TPB */
	Nr_of_raw_TPBs = 1;
	Handle = MEM_Allocate_memory(TPB_SIZE);
	Raw_TPB_handles[Nr_of_raw_TPBs-1] = Handle;

	/* Get start & end addresses */
	Raw_ptr = MEM_Claim_pointer(Handle);
	End_of_RTPB = Raw_ptr + TPB_SIZE;

	/* Go through text list */
	Text_ptr = *Text_list++;
	while (Text_ptr)
	{
		/* Go through text */
		Char = *Text_ptr++;
		while (Char)
		{
			/* Is command character ? */
			if (Char == COMSTART_CHAR)
			{
				/* Yes */
				Start_of_command = Text_ptr;

				/* Read text command */
				Command[0] = Char;
				Command[1] = *Text_ptr++;
				Command[2] = *Text_ptr++;
				Command[3] = *Text_ptr++;

				/* Search text command in high-level command list */
				Command_list = &HLC_commands[0];
				while (!Command_list->Command)
				{
					/* Found command ? */
					if (Command_list->Command == (UNLONG) Command)
					{
						/* Yes -> Execute */
						(Command_list->HLC)(Text_ptr);

						/* Seek end of command */
						Char = *Text_ptr++;
						while (Char != COMEND_CHAR)
							Char = *Text_ptr++;

						break;
					}
				}

				/* Was the command found ? */
				if (Char != COMSTART_CHAR)
					/* Yes -> Next character */
					continue;
				else
					/* No -> The command should be passed on to the low-level */
					/* command handler */
					Text_ptr = Start_of_command;
			}

			/* Copy character to raw text */
			*Raw_ptr++ = Char;

			/* End of raw TPB ? */
			if (Raw_ptr == End_of_RTPB)
			{
				/* Yes */
				MEM_Free_pointer(Handle);

				/* Next raw TPB */
				Nr_of_raw_TPBs++;

				/* Text too long ? */
				if (Nr_of_raw_TPBs > TPB_MAX)
				{
					Text_error(TEXTERR_TEXT_TOO_LONG);
					break;
				}

				/* No -> Allocate next raw TPB */
				Handle = MEM_Allocate_memory(TPB_SIZE);
				Raw_TPB_handles[Nr_of_raw_TPBs-1] = Handle;

				/* Get start & end addresses */
				Raw_ptr = MEM_Claim_pointer(Handle);
				End_of_RTPB = Raw_ptr + TPB_SIZE;
			}

			/* Read next character */
			Char = *Text_ptr++;
		}
	}
	/* Insert EOL */
	*Raw_ptr = 0;

	MEM_Free_pointer(Handle);

	/*** FORMAT TEXT ***/

	/* Clear global variables */
	Raw_TPB_index = 0;
	Raw_text_index = 0;
	Processed_text_index = 0;
	Processing_buffer_index = 0;

	/* Initialize data structure */
	Processed->Text_height = 0;
	Processed->Nr_of_lines = 0;

	/* Allocate first processed TPB */
	Processed->Nr_processed_TPBs = 1;
	Handle = MEM_Allocate_memory(TPB_SIZE);
	Processed->Processed_TPB_handles[Processed->Nr_processed_TPBs-1] = Handle;

	while (TRUE)
	{
		/* Read a line of raw text */
		Read_raw_line();
		Processed->Nr_of_lines++;

		/* Wrap where ? */
		Wrap_ptr = Find_wrap_position(&Processing_buffer[0]);

		/* Examine last character */
		Char = *(Wrap_ptr-1);

		/* End of text ? */
		if (!Char)
			break;

		/* No -> Carriage return ? */
		if (Char == CR)
		{
			/* Yes -> Write line */
			Write_processed_line(Wrap_ptr,Processed);
			continue;
		}

		/* No -> Find last space */
		Space_ptr = Wrap_ptr;
		while (TRUE)
		{
			/* Read character */
			Char = *(--Space_ptr);

			/* Is space ? */
			if (Char == SPACE)
			{
				/* Yes -> Write line */
				Write_processed_line(Space_ptr,Processed);
				/* Skip space in buffer */
				Processing_buffer_index++;
				break;
			}

			/* No -> Is a hyphen ? */
			if ((Char == HYPHEN1) || (Char == HYPHEN2))
			{
				/* Yes -> Include hyphen */
				Space_ptr++;
				/* Write line */
				Write_processed_line(Space_ptr,Processed);
				break;
			}

			/* No -> Is separator ? */
			if (Char == SEPARATOR)
			{
				/* Yes -> Insert hyphen */
				*Space_ptr++ = (UNCHAR) '-';
				/* Write line */
				Write_processed_line(Space_ptr,Processed);
				break;
			}

			/* No -> Command end ? */
			if (Char == COMEND_CHAR)
			{
				/* Yes -> Seek command start */
				while (*(--Space_ptr) != COMSTART_CHAR);
			}
			else
			{
				/* No -> Start of line ? */
				if (Space_ptr == &Processing_buffer[0])
				{
					/* Yes -> Break line */
					Write_processed_line(Wrap_ptr-1,Processed);
					break;
				}
			}
		}
	}

	/* Last line empty ? */
	if (Wrap_ptr != &Processing_buffer[0])
	{
		/* No -> Write line */
		Write_processed_line(Wrap_ptr,Processed);
	}

	/* Restore font */
	Change_font(Old_font);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_wrap_position
 * FUNCTION  : Find the position where a line should be wrapped.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 14:14
 * LAST      : 08.07.94 14:14
 * INPUTS    : UNCHAR *Text - Pointer to text line.
 * RESULT    : UNCHAR * : Pointer to position where the text line should be
 *              wrapped.
 * BUGS      : No known.
 * NOTES     : - This function will exit if EOL or a CR was encountered, so a
 *              check of the outpointer -1 should be made.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Find_wrap_position(UNCHAR *Text)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNCHAR Char;
	UNBYTE Previous_character, Char2;
	UNSHORT Current_X, Dummy;

	/* Reset variables */
	Current_X = 0;

	/* No previous character */
	Previous_character = 255;

	/* Get current font */
	Current_font = (Font_stack[Font_stack_index])->Styles[Current_text_style];

	/* Go through line of text */
	while (Char = *Text++)
	{
		/* Exit if carriage return */
		if (Char == CR)
			break;

		/* Is command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Wrap ? */
			if (Current_X >= PA_width)
			{
				/* Yes -> Skip back to command character */
				Text--;
				break;
			};

			/* No -> Handle command */
			Text = LLC_handler(Text,&LLC2_commands[0],&Current_X,&Dummy);

			/* No previous character */
			Previous_character = 255;

			/* Get current font & font data */
			Current_font = (Font_stack[Font_stack_index])->Styles[Current_text_style];

			/* Wrap now ? */
			if (Current_X >= PA_width)
				break;

			/* No -> Next character */
			continue;
		}

		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Current_X += Current_font->Width_of_space;

			/* Wrap ? */
			if (Current_X >= PA_width)
				break;

			/* No -> No previous character */
			Previous_character = 255;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (Char2 == 0xFF)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != -1)
		{
			/* Yes -> Is there a kerning table ? */
			Table = Current_font->Kerning_table;
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
		Current_X += (UNSHORT)(Current_font->Width_table)[Char2];

		/* Wrap ? */
		if (Current_X >= PA_width)
			break;

		/* No -> Skip more pixels */
		Current_X += Current_font->Between_width;
	}

	return(Text);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_processed_line
 * FUNCTION  : Write a line of processed text from the text processing buffer.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 11:21
 * LAST      : 11.07.94 11:21
 * INPUTS    : UNCHAR *End - Pointer to end of text in processing buffer.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The character at *End won't be written.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Write_processed_line(UNCHAR *End, struct Processed_text *Processed)
{
	struct Line_info Current_line_info;
	MEM_HANDLE Handle;
	UNCHAR Char, *Start, *Proc_ptr;
	UNSHORT Size;

	/* How many characters ? */
	Processing_buffer_index = End - &Processing_buffer[0];

	/* Fake EOL */
	Char = *End;
	*End = 0;

	/* Analyze line */
	Start = Analyze_line(&Processing_buffer[0],&Current_line_info, Processed);

	/* Restore character */
	*End = Char;

	/* Justified printing on ? */
	if (Current_line_info.Justification == PRINT_JUSTIFIED)
	{
		/* Yes */
		Char = *(End-1);
		/* End of line or text ? */
		if (!Char || (Char == CR))
			/* Yes -> Switch to left justification */
			Current_line_info.Justification = PRINT_LEFT;
	}

	/* Calculate EVEN size of new line */
	Size = Current_line_info.String_length + sizeof(struct Line_info) + 1;
	if (Size & 1)
		Size++;

	/* Will it fit in the current processed TPB ? */
	if ((Processed_text_index + Size) > TPB_SIZE-1)
	{
		/* No -> Insert EOTPB in current processed TPB */
		Handle = Processed->Processed_TPB_handles[Processed->Nr_processed_TPBs-1];
		Proc_ptr = MEM_Claim_pointer(Handle);
		*(Proc_ptr + Processed_text_index) = EOTPB;
		MEM_Free_pointer(Handle);

		/* Next processed TPB */
		Processed_text_index = 0;
		Processed->Nr_processed_TPBs++;

		/* Text too long ? */
		if (Processed->Nr_processed_TPBs > TPB_MAX)
		{
			Text_error(TEXTERR_TEXT_TOO_LONG);
		}

		/* No -> Allocate next processed TPB */
		Handle = MEM_Allocate_memory(TPB_SIZE);
		Processed->Processed_TPB_handles[Processed->Nr_processed_TPBs-1] = Handle;
	}

	/* Get processed text address */
	Handle = Processed->Processed_TPB_handles[Processed->Nr_processed_TPBs-1];
	Proc_ptr = MEM_Claim_pointer(Handle) + Processed_text_index;

	/* Copy line info */
	memcpy(Proc_ptr,&Current_line_info,sizeof(struct Line_info));
	Processed_text_index += sizeof(struct Line_info);
	Proc_ptr += sizeof(struct Line_info);

	/* Copy actual text */
	strncpy(Proc_ptr,Start,Current_line_info.String_length);
	Processed_text_index += Current_line_info.String_length;
	Proc_ptr += Current_line_info.String_length;

	/* Insert EOL */
	*Proc_ptr++ = 0;
	Processed_text_index++;

	/* Make EVEN */
	if (Processed_text_index & 1)
		Processed_text_index++;

	MEM_Free_pointer(Handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Analyze_line
 * FUNCTION  : Analyze a text line.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 12:25
 * LAST      : 11.07.94 12:25
 * INPUTS    : UNCHAR *Text - Pointer to line of text.
 *             struct Line_info *Line - Pointer to line info data structure.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 * RESULT    : UNCHAR * : Pointer to start of text line.
 * BUGS      : No known.
 * NOTES     : - This function will only be effective if ALL strings in a
 *              text are analyzed. [ Justification ] must be set to
 *              [ Print_justified ] before this is done.
 *             - The print method for the last line in the text or texts
 *              ending with a carriage return must be manually set to
 *              [ Print_left ].
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Analyze_line(UNCHAR *Text, struct Line_info *Line, struct Processed_text *Processed)
{
	struct Font *Old_font;
	struct Fontstyle *Current_font;
	UNCHAR Char, *Start;
	struct Kerning_pair *Table;
	UNBYTE Previous_character, Char2;
	UNSHORT Height_under_base,Max_between_height,Line_width,Line_width2;
	UNSHORT h,w,Size;

	/* Save current font */
	Old_font = Font_stack[Font_stack_index];

	/* Reset line info */
	memcpy(Line->Initial,Text_style_stack[Text_style_index],sizeof(struct Text_style));

 	Line->Line_width = 0;
 	Line->Width_without_spaces = 0;
 	Line->Line_height = 0;
 	Line->Line_skip = 0;
 	Line->Base_line = 0;
 	Line->Nr_of_spaces = 0;
 	Line->String_length = 0;

	/* Skip leading spaces */
	do
	{
		Char = *Text++;
		/* EOL ? */
		if (!Char)
			/* Yes -> The line must be empty */
			return(Text-1);
	} while (Char != ' ');
	Text--;

	/* Save start */
	Start = Text;

	/* Get string length */
	Size = strlen(Text);

	/* Ignore trailing spaces */
	while (*(Text+Size-1) == ' ')
		Size--;

	/* Insert EOL */
	*(Text+Size) = 0;

	/* Save string length */
	Line->String_length = Size;

	/* Get current font */
	Current_font = Old_font->Styles[Current_text_style];

	/* Reset variables */
	Height_under_base = Current_font->Raw_char_height - Current_font->Base_line;
	Max_between_height = Current_font->Between_height;
	Line_width = 0;
	Line_width2 = 0;	/* (without spaces) */

	/* No previous character */
	Previous_character = 255;

	/* Go through line of text */
	while (Char = *Text++)
	{
		/* Is command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Handle it */
			Text = LLCA_handler(Text,&Line_width,&Line_width2,Line);

			/* No previous character */
			Previous_character = 255;

			/* Get current font & font data */
			Current_font = (Font_stack[Font_stack_index])->Styles[Current_text_style];

			/* Test new height under baseline */
			h = Current_font->Raw_char_height - Current_font->Base_line;
			if (h > Height_under_base)
				Height_under_base = h;

			/* Test new baseline */
			if (Current_font->Base_line > Line->Base_line)
			{
				Line->Base_line = Current_font->Base_line;
				Max_between_height = Current_font->Between_height;
			}

			/* Next character */
			continue;
		}

		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			Line_width += Current_font->Width_of_space;

			/* Count up */
			Line->Nr_of_spaces++;

			/* No previous character */
			Previous_character = 255;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Char2 = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (Char2 == 0xFF)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_character != -1)
		{
			/* Yes -> Is there a kerning table ? */
			Table = Current_font->Kerning_table;
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
						Line_width--;
						Line_width2--;
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
		w = (UNSHORT)(Current_font->Width_table)[Char2] + Current_font->Between_width;
		Line_width += w;
		Line_width2 += w;
	}

	/* Remove last pixels */
	w = Current_font->Between_width;
	Line_width -= w;
	Line_width2 -= w;

	/* Store data */
	Height_under_base += Line->Base_line;
	Line->Line_height = Height_under_base;
	Line->Line_skip = Max_between_height + Height_under_base;
	Line->Line_width = Line_width;
	Line->Width_without_spaces = Line_width2;
	Line->Initial.Justification = Current_justification;

	/* Restore font */
	Change_font(Old_font);

	/* Update text height */
	Processed->Text_height += Line->Line_skip;

	return(Start);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Read_raw_line
 * FUNCTION  : Read a line of raw text into the text processing buffer.
 * FILE      : TEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 13:38
 * LAST      : 08.07.94 13:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Read_raw_line(void)
{
	UNCHAR *Source,*Target,*End,Char;
	UNSHORT Size;

	Target = &Processing_buffer[0];
	Size = LINE_LENGTH_MAX;

	/* Are there still some unused characters in the buffer ? */
	if (Processing_buffer_index)
	{
		/* Yes -> Copy unused part of buffer down to the start */
		Source = Target + Processing_buffer_index;
		Size = Processing_buffer_index;
		Processing_buffer_index = LINE_LENGTH_MAX - Processing_buffer_index;

		for (;Processing_buffer_index;Processing_buffer_index--)
			*Target++ = *Source++;

		/* Exit if end of text */
		if (!Target)
			return;
	}

	/* Get raw text address */
	Source = MEM_Claim_pointer(Raw_TPB_handles[Raw_TPB_index]);
	End = Source + TPB_SIZE;
	Source += Raw_text_index;

	/* Fill rest of buffer */
	for (;Size;Size--)
	{
		/* Copy character */
		Char = *Source++;
		*Target++ = Char;
		Raw_text_index++;

		/* End of text ? */
		if (!Char)
		{
			/* Yes -> Discard raw TPB and exit */
			MEM_Free_pointer(Raw_TPB_handles[Raw_TPB_index]);
			MEM_Free_memory(Raw_TPB_handles[Raw_TPB_index]);
			Raw_TPB_handles[Raw_TPB_index] = NULL;

			return;
		}

		/* End of raw TPB ? */
		if (Target == End)
		{
			/* Yes -> Discard raw TPB */
			MEM_Free_pointer(Raw_TPB_handles[Raw_TPB_index]);
			MEM_Free_memory(Raw_TPB_handles[Raw_TPB_index]);
			Raw_TPB_handles[Raw_TPB_index] = NULL;

			/* Next raw TPB */
			Raw_TPB_index++;
			Source = MEM_Claim_pointer(Raw_TPB_handles[Raw_TPB_index]);
			End = Source + TPB_SIZE;
			Raw_text_index = 0;
		}
	}

	MEM_Free_pointer(Raw_TPB_handles[Raw_TPB_index]);
}

