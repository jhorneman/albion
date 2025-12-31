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
#include <BBMEM.H>

#include <TEXT.H>
#include "TEXTVAR.H"

#include <FONT.H>
#include <GAMETEXT.H>

/* prototypes */

UNCHAR *Analyze_line(UNCHAR *Text, struct Processed_text *Processed);

void Read_raw_line(void);

UNCHAR *Find_wrap_position(UNCHAR *Text);

void Write_processed_line(UNCHAR *End, struct Processed_text *Processed);

/* global variables */

static UNSHORT Processed_text_index; 	/* Index in current processed TPB */
static UNSHORT Raw_text_index;				/* Index in current raw TPB */
static UNSHORT Processing_buffer_index;	/* Index in processing buffer */

static UNBYTE Processing_buffer[MAX_LINE_LENGTH];

static MEM_HANDLE First_raw_TPB_handle;
static MEM_HANDLE Current_raw_TPB_handle;
static MEM_HANDLE Current_processed_TPB_handle;

struct Textstyle This_line_style;

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Process_text
 * FUNCTION  : Process a single text.
 * FILE      : TEXTPROC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 11:30
 * LAST      : 22.06.95 15:14
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
	Process_text_list(List, Processed);
};

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Process_text_list
 * FUNCTION  : Process a list of texts.
 * FILE      : TEXTPROC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 11:33
 * LAST      : 11.10.95 18:05
 * INPUTS    : UNCHAR **Text_list - Pointer to a NULL-terminated list of
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
	struct TPB *Current_raw_TPB;
	struct TPB *Current_processed_TPB;
	MEM_HANDLE Handle;
	UNCHAR Char;
	UNCHAR *Text_ptr;
	UNCHAR *Raw_ptr;
	UNCHAR *End_of_RTPB;
	UNCHAR *Wrap_ptr;
	UNCHAR *Space_ptr;

	/* Get initial text style */
	memcpy(&Current_text_style, Textstyle_stack[Textstyle_stack_index],
	 sizeof(struct Textstyle));

	/* Initialize start text style for this line */
	memcpy(&This_line_style, &Current_text_style, sizeof(struct Textstyle));

	/* Clear processed text data */
	Processed->Nr_of_lines	= 0;
	Processed->Text_height	= 0;
	Processed->First_handle	= NULL;

	/*** MERGE ALL STRINGS INTO ONE ***/

	/* Allocate first raw TPB */
	First_raw_TPB_handle = MEM_Do_allocate
	(
		TPB_SIZE,
		0,
		&TPB_ftype
	);
	if (!First_raw_TPB_handle)
	{
		Text_error(TEXTERR_OUT_OF_MEMORY);
		return;
	}

	Current_raw_TPB_handle = First_raw_TPB_handle;

	/* Get start & end addresses */
	Raw_ptr = MEM_Claim_pointer(Current_raw_TPB_handle);
	End_of_RTPB = Raw_ptr + TPB_SIZE;

	/* Initialize TPB */
	Current_raw_TPB = (struct TPB *) Raw_ptr;
	Raw_ptr += sizeof(struct TPB);
	Current_raw_TPB->Next_handle = NULL;

	/* Go through text list */
	Text_ptr = *Text_list++;
	while (Text_ptr)
	{
		for (;;)
		{
			/* Read a character from the string */
			Char = *Text_ptr++;

			/* Exit if EOL */
			if (!Char)
				break;

			/* Is this a command character ? */
			if (Char == COMSTART_CHAR)
			{
				struct HLC_Text_command *Command_list;
				BOOLEAN Flag;
				UNSHORT i, j;
				UNCHAR Command[4];
				UNCHAR *Start_of_command;
				UNCHAR *Add_ptr;

				/* Yes */
				Start_of_command = Text_ptr;

				/* Read text command */
				Command[0] = *Text_ptr++;
				Command[1] = *Text_ptr++;
				Command[2] = *Text_ptr++;
				Command[3] = *Text_ptr++;

				/* Exception for BLOK command (yuk) */
				/* Is BLOK command ? */
				if ((Command[0] == 'B') && (Command[1] == 'L') &&
				 (Command[2] == 'O') && (Command[3] == 'K'))
				{
					/* Yes -> Break */
					break;
				}
				else
				{
					/* No -> Search text command in high-level command list */
					Command_list = &HLC_commands[0];
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
							/* Yes -> Execute command */
							Add_ptr = (Command_list->Command_handler)(Text_ptr);

							/* Any string to add ? */
							if (Add_ptr)
							{
								/* Yes -> Add new string to raw text */
								j = strlen(Add_ptr);
								for (i=0;i<j;i++)
								{
									/* Copy character from new string to raw text */
									*Raw_ptr++ = *Add_ptr++;

									/* End of raw TPB ? */
									if (Raw_ptr == End_of_RTPB)
									{
										/* Yes -> Allocate next raw TPB */
										Handle = MEM_Do_allocate
										(
											TPB_SIZE,
											0,
											&TPB_ftype
										);

										/* Success ? */
										if (!Handle)
										{
											/* No -> Report error */
											Text_error(TEXTERR_OUT_OF_MEMORY);

											/* Force end */
											*Text_ptr = '\0';
											break;
										}

										/* Link to previous raw TPB */
										Current_raw_TPB->Next_handle = Handle;
										MEM_Free_pointer(Current_raw_TPB_handle);
										Current_raw_TPB_handle = Handle;

										/* Get start & end addresses */
										Raw_ptr = MEM_Claim_pointer(Current_raw_TPB_handle);
										End_of_RTPB = Raw_ptr + TPB_SIZE;

										/* Initialize TPB */
										Current_raw_TPB = (struct TPB *) Raw_ptr;
										Raw_ptr += sizeof(struct TPB);
										Current_raw_TPB->Next_handle = NULL;
									}
								}
							}

							/* Seek end of command */
							Char = *Text_ptr++;
							while (Char != COMEND_CHAR)
								Char = *Text_ptr++;

							break;
						}

						/* No -> Next command */
						Command_list++;
					}

					/* Was the command found ? */
					if (Char != COMSTART_CHAR)
					{
						/* Yes -> Next character */
						continue;
					}
					else
					{
						/* No -> The command should be passed on to the low-level */
						/* command handler */
						Text_ptr = Start_of_command;
					}
				}
			}

			/* Copy character to raw text */
			*Raw_ptr++ = Char;

			/* End of raw TPB ? */
			if (Raw_ptr == End_of_RTPB)
			{
				/* Yes -> Allocate next raw TPB */
				Handle = MEM_Do_allocate
				(
					TPB_SIZE,
					0,
					&TPB_ftype
				);
				if (!Handle)
				{
					Text_error(TEXTERR_OUT_OF_MEMORY);
					break;
				}

				/* Link to previous raw TPB */
				Current_raw_TPB->Next_handle = Handle;
				MEM_Free_pointer(Current_raw_TPB_handle);
				Current_raw_TPB_handle = Handle;

				/* Get start & end addresses */
				Raw_ptr = MEM_Claim_pointer(Current_raw_TPB_handle);
				End_of_RTPB = Raw_ptr + TPB_SIZE;

				/* Initialize TPB */
				Current_raw_TPB = (struct TPB *) Raw_ptr;
				Raw_ptr += sizeof(struct TPB);
				Current_raw_TPB->Next_handle = NULL;
			}
		}
		/* Next text in list */
		Text_ptr = *Text_list++;
	}

	/* Insert EOL */
	*Raw_ptr = 0;

	MEM_Free_pointer(Current_raw_TPB_handle);

	/*** FORMAT TEXT ***/

	/* Initialize variables */
	Raw_text_index = sizeof(struct TPB);
	Processed_text_index = sizeof(struct TPB);
	Processing_buffer_index = 0;
	Current_raw_TPB_handle = First_raw_TPB_handle;

	/* Initialize data structure */
	Processed->Text_height = 0;
	Processed->Nr_of_lines = 0;

	/* Dry run ? */
	if (!Process_text_dry_run_flag)
	{
		/* No -> Allocate first processed TPB */
		Current_processed_TPB_handle = MEM_Do_allocate
		(
			TPB_SIZE,
			0,
			&TPB_ftype
		);
		if (!Current_processed_TPB_handle)
		{
			Text_error(TEXTERR_OUT_OF_MEMORY);
			return;
		}

		Processed->First_handle = Current_processed_TPB_handle;

		/* Prepare first processed TPB */
		Current_processed_TPB = (struct TPB *) MEM_Claim_pointer(Current_processed_TPB_handle);

		Current_processed_TPB->Next_handle	= NULL;
		Current_processed_TPB->Height			= 0;
		Current_processed_TPB->Nr_of_lines	= 0;

		MEM_Free_pointer(Current_processed_TPB_handle);
	}
	else
	{
		/* Yes -> Insert NULL handle */
		Processed->First_handle = NULL;
	}

	for (;;)
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
			Write_processed_line(Wrap_ptr, Processed);
			continue;
		}

		/* No -> Find last space */
		Space_ptr = Wrap_ptr;
		while (TRUE)
		{
			/* Read character */
			Space_ptr--;
			Char = *Space_ptr;

			/* Is space ? */
			if (Char == SPACE)
			{
				/* Yes -> Search the first of this group of spaces */
				while ((*(Space_ptr - 1) == SPACE) &&
				 (Space_ptr > &Processing_buffer[1]))
				{
					Space_ptr--;
				}

				/* Write line */
				Write_processed_line(Space_ptr, Processed);

				/* Skip spaces in buffer */
				while ((Processing_buffer[Processing_buffer_index] == SPACE) &&
				 ((Processing_buffer + Processing_buffer_index) < Wrap_ptr))
				{
					Processing_buffer_index++;
				}
				break;
			}

			/* No -> Is a hyphen ? */
			if ((Char == HYPHEN1) || (Char == HYPHEN2))
			{
				/* Yes -> Include hyphen */
				Space_ptr++;

				/* Write line */
				Write_processed_line(Space_ptr, Processed);
				break;
			}

			/* No -> Is separator ? */
			if (Char == SEPARATOR)
			{
				/* Yes -> Insert hyphen */
				*Space_ptr++ = HYPHEN1;

				/* Write line */
				Write_processed_line(Space_ptr, Processed);
				break;
			}

			/* No -> Command end ? */
			if (Char == COMEND_CHAR)
			{
				/* Yes -> Seek command start */
				Space_ptr--;
				while (*Space_ptr != COMSTART_CHAR)
				{
					Space_ptr--;
				}
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
		UNBYTE *Ptr;

		/* No -> Write line */
		Write_processed_line(Wrap_ptr, Processed);

		/* Dry run ? */
		if (!Process_text_dry_run_flag)
		{
			/* No -> Insert EOTPB in current processed TPB */
			Ptr = MEM_Claim_pointer(Current_processed_TPB_handle);

			*(Ptr + Processed_text_index) = EOTPB;

			MEM_Free_pointer(Current_processed_TPB_handle);
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_wrap_position
 * FUNCTION  : Find the position where a line should be wrapped.
 * FILE      : TEXTPROC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 14:14
 * LAST      : 10.09.95 17:32
 * INPUTS    : UNCHAR *Text - Pointer to text line.
 * RESULT    : UNCHAR * : Pointer to position where the text line should be
 *              wrapped.
 * BUGS      : No known.
 * NOTES     : - This function will exit if EOL or a CR was encountered, so a
 *              check of the output pointer - 1 should be made.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Find_wrap_position(UNCHAR *Text)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNBYTE Translated_char;
	UNBYTE Previous_translated_char;
	UNCHAR Char;
	UNCHAR Previous_char;

	/* Set X-coordinate */
	TEXT_Current_X = 0;

	/* No previous character */
	Previous_char = 0xFF;
	Previous_translated_char = 0xFF;

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	while (TRUE)
	{
		/* Read a character from the string */
		Char = *Text++;

		/* Exit if EOL or carriage return */
		if ((!Char) || (Char == CR))
			break;

		/* Is this a command character ? */
		if (Char == COMSTART_CHAR)
		{
			/* Yes -> Wrap ? */
			if (TEXT_Current_X >= PA_width)
			{
				/* Yes -> Skip back to command character */
				Text--;
				break;
			};

			/* No -> Handle it */
			Text = LLC_handler(Text, &LLCW_commands[0]);

			/* No previous character */
			Previous_char = 0xFF;
			Previous_translated_char = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Wrap now ? */
			if (TEXT_Current_X >= PA_width)
				break;

			/* No -> Next character */
			continue;
		}

		/* Is this a space ? */
		if ((Char == SPACE) || (Char == SOLID_SPACE))
		{
			/* Yes -> Skip pixels */
			TEXT_Current_X += Current_font->Width_of_space;

			#if FALSE
			/* Was the previous character a period ? */
			if (Previous_char == PERIOD)
			{
				/* Yes -> Skip even more pixels */
				TEXT_Current_X += 3 * Current_font->Width_of_space;
			}
			#endif

			/* Wrap ? */
			if (TEXT_Current_X >= PA_width)
				break;

			/* No -> No previous character */
			Previous_char = 0xFF;
			Previous_translated_char = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Translated_char = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Translated_char)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_translated_char != 0xFF)
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
					if ((Table->First == Previous_translated_char) &&
					 (Table->Second == Translated_char))
					{
						/* Yes -> Remove pixel */
						TEXT_Current_X--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}

		/* Save character */
		Previous_char = Char;
		Previous_translated_char = Translated_char;

		/* Skip character width in pixels */
		TEXT_Current_X += (UNSHORT)(Current_font->Width_table)[Translated_char];

		/* Wrap ? */
		if (TEXT_Current_X >= PA_width)
			break;

		/* No -> Skip more pixels */
		TEXT_Current_X += Current_font->Between_width;
	}

	return Text;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Write_processed_line
 * FUNCTION  : Write a line of processed text from the text processing buffer.
 * FILE      : TEXTPROC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 11:21
 * LAST      : 22.06.95 15:17
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
	struct TPB *Current_processed_TPB;
	struct Line_info Info;
	MEM_HANDLE Handle;
	UNCHAR Char;
	UNCHAR *Start;
	UNCHAR *Ptr;
	UNSHORT Size;

	/* How many characters ? */
	Processing_buffer_index = End - &Processing_buffer[0];

	/* Fake EOL */
	Char = *End;
	*End = '\0';

	/* Analyze line */
	Start = Analyze_line(&Processing_buffer[0], Processed);

	/* Restore character */
	*End = Char;

	/* Justified printing on ? */
	if (Current_line_info.Style.Justification == PRINT_JUSTIFIED)
	{
		/* Yes -> Get last character */
		Char = *(End-1);

		/* End of line or text ? */
		if (!Char || (Char == CR))
		{
			/* Yes -> Switch to left justification */
			Current_line_info.Style.Justification = PRINT_LEFT;
			This_line_style.Justification = PRINT_LEFT;
		}
	}

	/* Calculate EVEN size of new line */
	Size = Current_line_info.String_length + sizeof(struct Line_info) + 1;
	if (Size & 1)
		Size++;

	/* Will it fit in the current processed TPB ? */
	if ((Processed_text_index + Size) >= TPB_SIZE-1)
	{
		/* No -> Dry run ? */
		if (!Process_text_dry_run_flag)
		{
			/* No -> Allocate next processed TPB */
			Handle = MEM_Do_allocate
			(
				TPB_SIZE,
				0,
				&TPB_ftype
			);

			/* Success ? */
			if (Handle)
			{
				/* Yes -> Link to previous processed TPB */
				Ptr = MEM_Claim_pointer(Current_processed_TPB_handle);
				Current_processed_TPB = (struct TPB *) Ptr;
				Current_processed_TPB->Next_handle = Handle;

				/* Insert EOTPB in current processed TPB */
				*(Ptr + Processed_text_index) = EOTPB;
				MEM_Free_pointer(Current_processed_TPB_handle);

				/* Prepare next processed TPB */
				Current_processed_TPB_handle = Handle;

				Current_processed_TPB = (struct TPB *)
				 MEM_Claim_pointer(Current_processed_TPB_handle);

				Current_processed_TPB->Next_handle	= NULL;
				Current_processed_TPB->Height			= 0;
				Current_processed_TPB->Nr_of_lines	= 0;

				MEM_Free_pointer(Current_processed_TPB_handle);
			}
			else
			{
				/* No -> Report error */
				Text_error(TEXTERR_OUT_OF_MEMORY);
			}
		}

		/* Reset processed text index */
		Processed_text_index = sizeof(struct TPB);
	}

	/* Dry run ? */
	if (!Process_text_dry_run_flag)
	{
		/* No -> Get processed text address */
		Ptr = MEM_Claim_pointer(Current_processed_TPB_handle);
		Current_processed_TPB = (struct TPB *) Ptr;
		Ptr += Processed_text_index;

		/* Create line info */
		memcpy
		(
			&Info,
			&Current_line_info,
			sizeof(struct Line_info)
		);
		memcpy
		(
			&(Info.Style),
			&This_line_style,
			sizeof(struct Textstyle)
		);

		/* Copy line info */
		memcpy
		(
			Ptr,
			&Info,
			sizeof(struct Line_info)
		);
		Processed_text_index += sizeof(struct Line_info);
		Ptr += sizeof(struct Line_info);

		/* Update start text style for next line */
		memcpy
		(
			&This_line_style,
			&Current_text_style,
			sizeof(struct Textstyle)
		);

		/* Copy actual text */
		strncpy
		(
			Ptr,
			Start,
			Current_line_info.String_length
		);
		Processed_text_index += Current_line_info.String_length;
		Ptr += Current_line_info.String_length;

		/* Insert EOL */
		*Ptr++ = 0;
		Processed_text_index++;

		/* Make EVEN */
		if (Processed_text_index & 1)
			Processed_text_index++;

		MEM_Free_pointer(Current_processed_TPB_handle);

		/* Update total TPB height */
		Current_processed_TPB->Height += Current_line_info.Skip;

		/* Increase number of lines in TPB */
		Current_processed_TPB->Nr_of_lines++;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Analyze_line
 * FUNCTION  : Analyze a text line.
 * FILE      : TEXTPROC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.07.94 12:25
 * LAST      : 10.09.95 17:32
 * INPUTS    : UNCHAR *Text - Pointer to line of text.
 *             struct Processed_text *Processed - Pointer to processed text
 *              data structure.
 * RESULT    : UNCHAR * : Pointer to actual start of text line.
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
Analyze_line(UNCHAR *Text, struct Processed_text *Processed)
{
	struct Fontstyle *Current_font;
	struct Kerning_pair *Table;
	UNSHORT Height_under_base;
	UNSHORT Max_between_height;
	UNSHORT w;
	UNSHORT Size;
	UNBYTE Translated_char;
	UNBYTE Previous_translated_char;
	UNCHAR Char;
	UNCHAR Previous_char;
	UNCHAR *Start;

	/* Reset line info */
	memcpy(&(Current_line_info.Style), &Current_text_style, sizeof(struct Textstyle));

 	Current_line_info.Width						= 0;
 	Current_line_info.Width_without_spaces	= 0;
 	Current_line_info.Height					= 0;
 	Current_line_info.Skip						= 0;
 	Current_line_info.Base_line				= 0;
 	Current_line_info.Nr_of_spaces			= 0;
 	Current_line_info.String_length			= 0;

	/* Skip leading spaces */
	do
	{
		Char = *Text++;

		/* EOL ? */
		if (!Char)
		{
			/* Yes -> The line is empty */
			return Text-1;
		}
	} while (Char == SPACE);
	Text--;

	/* Save start */
	Start = Text;

	/* Get string length */
	Size = strlen(Text);

	/* Ignore trailing spaces */
	while (*(Text+Size-1) == SPACE)
		Size--;

	/* Insert EOL */
	/* This is harmless because Write_processed_line() has already saved the
	 last possibly interesting character, which is now being replaced by an
	 EOL. */
	*(Text+Size) = 0;

	/* Save string length */
	Current_line_info.String_length = Size;

	/* Get current font */
	Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

	/* Reset variables */
	Height_under_base = Current_font->Raw_height - Current_font->Base_line;
	Max_between_height = Current_font->Between_height;
	Current_line_info.Base_line = Current_font->Base_line;

	Width_without_spaces = 0;
	TEXT_Current_X = 0;

	/* No previous character */
	Previous_char = 0xFF;
	Previous_translated_char = 0xFF;

	/* Go through line of text */
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
			UNSHORT h;

			/* Yes -> Handle it */
			Text = LLC_handler(Text, &LLCA_commands[0]);

			/* No previous character */
			Previous_char = 0xFF;
			Previous_translated_char = 0xFF;

			/* Get current font (which may have changed) */
			Current_font = Current_text_style.Font->Styles[Current_text_style.Style];

			/* Test new height under baseline */
			h = Current_font->Raw_height - Current_font->Base_line;
			if (h > Height_under_base)
				Height_under_base = h;

			/* Test new baseline */
			if (Current_font->Base_line > Current_line_info.Base_line)
			{
				Current_line_info.Base_line = Current_font->Base_line;
				Max_between_height = Current_font->Between_height;
			}

			/* Next character */
			continue;
		}

		/* Is this a space ? */
		if (Char == SPACE)
		{
			/* Yes -> Skip pixels */
			TEXT_Current_X += Current_font->Width_of_space;

			#if FALSE
			/* Was the previous character a period ? */
			if (Previous_char == PERIOD)
			{
				/* Yes -> Skip even more pixels */
				TEXT_Current_X += 3 * Current_font->Width_of_space;
				Width_without_spaces += 3 * Current_font->Width_of_space;
			}
			#endif

			/* Count up */
			Current_line_info.Nr_of_spaces++;

			/* No previous character */
			Previous_char = 0xFF;
			Previous_translated_char = 0xFF;

			/* Next character */
			continue;
		}

		/* Is this a solid space ? */
		if (Char == SOLID_SPACE)
		{
			/* Yes -> Skip pixels */
			TEXT_Current_X += Current_font->Width_of_space;
			Width_without_spaces += Current_font->Width_of_space;

			/* No previous character */
			Previous_char = 0xFF;
			Previous_translated_char = 0xFF;

			/* Next character */
			continue;
		}

		/* Legal character ? */
		if ((UNBYTE) Char < 32)
			continue;

		/* Yes -> Translate character */
		Translated_char = (Current_font->Translation)[((UNSHORT) Char) - 32];

		/* Is the character in the font ? */
		if (!Translated_char)
			continue;

		/* Yes -> Is there a previous character ? */
		if (Previous_translated_char != 0xFF)
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
					if ((Table->First == Previous_translated_char) &&
					 (Table->Second == Translated_char))
					{
						/* Yes -> Remove pixel */
						TEXT_Current_X--;
						Width_without_spaces--;
						break;
					}
					/* No -> Next entry */
					Table++;
				}
			}
		}
		/* Save character */
		Previous_char = Char;
		Previous_translated_char = Translated_char;

		/* Skip character width in pixels */
		w = (UNSHORT)(Current_font->Width_table)[Translated_char] +
		 Current_font->Between_width;
		TEXT_Current_X += w;
		Width_without_spaces += w;
	}

	/* Remove last pixels */
	w = Current_font->Between_width;
	TEXT_Current_X -= w;
	Width_without_spaces -= w;

	/* Store data */
	Height_under_base += Current_line_info.Base_line;

	Current_line_info.Height					= Height_under_base;
	Current_line_info.Skip						= Max_between_height + Height_under_base;
	Current_line_info.Width						= TEXT_Current_X;
	Current_line_info.Width_without_spaces	= Width_without_spaces;
	Current_line_info.Style.Justification	= Current_text_style.Justification;

	This_line_style.Justification = Current_text_style.Justification;

	/* Update text height */
	Processed->Text_height += Current_line_info.Skip;

	return Start;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Read_raw_line
 * FUNCTION  : Read a line of raw text into the text processing buffer.
 * FILE      : TEXTPROC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.07.94 13:38
 * LAST      : 23.08.94 18:44
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
	struct TPB *Current_raw_TPB;
	UNCHAR *Source,*Target,Char;
	UNSHORT Size;

	Target = &Processing_buffer[0];
	Size = MAX_LINE_LENGTH;

	/* Are there still some unused characters in the buffer ? */
	if (Processing_buffer_index)
	{
		/* Yes -> Copy unused part of buffer down to the start */
		memmove
		(
			Target,
			Target + Processing_buffer_index,
			MAX_LINE_LENGTH - Processing_buffer_index
		);

		/* Less characters need be read */
		Size = Processing_buffer_index;

		/* The target is further back */
		Target += MAX_LINE_LENGTH - Processing_buffer_index;

		/* Go back to the start of the buffer */
		Processing_buffer_index = 0;

		/* Exit if end of text */
		if (!Current_raw_TPB_handle)
			return;
	}

	/* Get raw text address */
	Source = MEM_Claim_pointer(Current_raw_TPB_handle);
	Current_raw_TPB = (struct TPB *) Source;
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
			/* Yes -> Discard current raw TPB */
			MEM_Free_pointer(Current_raw_TPB_handle);
			MEM_Free_memory(Current_raw_TPB_handle);
			Current_raw_TPB_handle = NULL;

			/* Exit */
			return;
		}

		/* End of raw TPB ? */
		if (Raw_text_index == TPB_SIZE)
		{
			MEM_HANDLE Handle;

			/* Yes -> Discard current raw TPB */
			Handle = Current_raw_TPB->Next_handle;
			MEM_Free_pointer(Current_raw_TPB_handle);
			MEM_Free_memory(Current_raw_TPB_handle);

			/* Next raw TPB */
			Current_raw_TPB_handle = Handle;
			Source = MEM_Claim_pointer(Current_raw_TPB_handle);
			Raw_text_index = sizeof(struct TPB);
			Current_raw_TPB = (struct TPB *) Source;
			Source += Raw_text_index;
		}
	}

	MEM_Free_pointer(Current_raw_TPB_handle);
}

