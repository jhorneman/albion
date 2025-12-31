/************
 * NAME     : GAMETEXT.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 22-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBERROR.H>

#include <XLOAD.H>
#include <TEXT.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <XFTYPES.H>
#include <GAMETEXT.H>
#include <INVENTO.H>
#include <ITMLOGIC.H>
#include <STATAREA.H>
#include <COMBAT.H>
#include <BUTTONS.H>
#include <CONTROL.H>
#include <INPUT.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <FONT.H>
#include <DIALOGUE.H>

/* defines */

#define NR_DICTIONARIES			(3)
#define WORDS_PER_DICTIONARY  (500)

#define MAX_WORDS_IN_WINDOW	(12)

#define WORD_OBJ_WIDTH			(100)
#define WORD_OBJ_HEIGHT			(10)

/* Error codes */
#define ERROR_SYSTEXT_LOAD					(1)
#define ERROR_ILLEGAL_TEXT_BLOCK_NR		(2)
#define ERROR_SYSTEXT_BLOCK_CONFLICT	(3)
#define ERROR_ILLEGAL_SYSTEXT_NR			(4)
#define ERROR_SUBBLOCK_CONFLICT			(5)
#define ERROR_ILLEGAL_SUBBLOCK_NR		(6)
#define ERROR_TOO_MANY_WORDS				(7)

/* structure definitions */

/* Word list entry */
struct Word_list_entry {
	UNSHORT Merged_word_index;
	UNSHORT Flags;
};

/* Bitmasks for Word_list_entry.Flags */
#define WORD_NEW		(1 << 0)

/* Word selection window OID */
struct Word_window_OID {
	UNSHORT *Result_ptr;
};

/* Word selection window object */
struct Word_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	UNSHORT *Result_ptr;

	UNSHORT Nr_words;

	UNSHORT Scroll_bar_object;
};

/* Word OID */
struct Word_OID {
	UNSHORT Number;
};

/* Word object */
struct Word_object {
	struct Object Object;
	UNSHORT Number;
};

/* prototypes */

void Convert_text_file(UNCHAR *Start, UNLONG Size, UNCHAR **Text_block_ptrs,
 UNSHORT Max_text_blocks);

/* Dictionary support functions */
void Merge_dictionary(MEM_HANDLE Dictionary_handle, UNSHORT First_word,
 UNSHORT Max_words);
void Add_word_to_merged_dictionary(UNCHAR *New_word_ptr, UNSHORT Word_index);

/* Word selection window object methods */
UNLONG Init_Word_window_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Word_window_object(struct Object *Object, union Method_parms *P);
UNLONG Customkeys_Word_window_object(struct Object *Object,
 union Method_parms *P);

UNSHORT Build_word_list(void);
void Swap_words(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_words(UNLONG A, UNLONG B, UNBYTE *Data);

void Update_word_list(struct Scroll_bar_object *Scroll_bar);
void Word_input(struct Button_object *Button);

/* Word object methods */
UNLONG Init_Word_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Word_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Word_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Word_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Word_object(struct Object *Object, union Method_parms *P);

void Draw_word(struct Word_object *Word);

/* High-level text commands */
UNCHAR *HLC_New_word(UNCHAR *Text);
UNCHAR *HLC_Unknown_word(UNCHAR *Text);
UNCHAR *HLC_Price(UNCHAR *Text);
UNCHAR *HLC_Damage(UNCHAR *Text);
UNCHAR *HLC_Weapon(UNCHAR *Text);

UNCHAR *HLC_Select_leader(UNCHAR *Text);
UNCHAR *HLC_Select_self(UNCHAR *Text);
UNCHAR *HLC_Select_inventory(UNCHAR *Text);
UNCHAR *HLC_Select_dialogue(UNCHAR *Text);
UNCHAR *HLC_Select_combat(UNCHAR *Text);
UNCHAR *HLC_Select_victim(UNCHAR *Text);
UNCHAR *HLC_Select_PUM_char(UNCHAR *Text);
UNCHAR *HLC_Select_subject_char(UNCHAR *Text);

UNCHAR *HLC_Name(UNCHAR *Text);
UNCHAR *HLC_He(UNCHAR *Text);
UNCHAR *HLC_Him(UNCHAR *Text);
UNCHAR *HLC_His(UNCHAR *Text);
UNCHAR *HLC_Man(UNCHAR *Text);
UNCHAR *HLC_Race(UNCHAR *Text);
UNCHAR *HLC_Class(UNCHAR *Text);
UNCHAR *HLC_Race_insult(UNCHAR *Text);
UNCHAR *HLC_Class_insult(UNCHAR *Text);

UNCHAR *HLC_Add_new_word(UNCHAR *Text);

/* Error handling */
void Gametext_error(UNSHORT Error_code);

/* global variables */
UNCHAR *System_text_ptrs[MAX_SYSTEM_TEXTS];

static MEM_HANDLE System_text_handle;

static UNCHAR Default_system_text[] = "NOT DEFINED";

MEM_HANDLE PUM_char_handle;
MEM_HANDLE Subject_char_handle;

static MEM_HANDLE HLC_Subject_handle;
static UNCHAR HLC_String[50];

/* Dictionary variables */
static UNSHORT Merged_dictionary_index_list[WORDS_MAX];
static UNSHORT Nr_merged_words;
static MEM_HANDLE Merged_dictionary_handle;
static MEM_HANDLE Word_list_handle;

/* High-level text commands */
struct HLC_Text_command HLC_commands[] = {
	{{'U','N','K','N'}, HLC_Unknown_word},
	{{'P','R','I','C'}, HLC_Price},
	{{'D','A','M','G'}, HLC_Damage},
	{{'W','E','A','P'}, HLC_Weapon},

	{{'L','E','A','D'}, HLC_Select_leader},
	{{'S','E','L','F'}, HLC_Select_self},
	{{'I','N','V','E'}, HLC_Select_inventory},
	{{'D','I','A','L'}, HLC_Select_dialogue},
	{{'C','O','M','B'}, HLC_Select_combat},
	{{'V','I','C','T'}, HLC_Select_victim},
	{{'P','U','M',' '}, HLC_Select_PUM_char},
	{{'S','U','B','J'}, HLC_Select_subject_char},

	{{'N','A','M','E'}, HLC_Name},
	{{'H','E',' ',' '}, HLC_He},
	{{'H','I','M',' '}, HLC_Him},
	{{'H','I','S',' '}, HLC_His},
	{{'M','A','N',' '}, HLC_Man},
	{{'R','A','C','E'}, HLC_Race},
	{{'C','L','A','S'}, HLC_Class},
	{{'R','A','C','I'}, HLC_Race_insult},
	{{'C','L','S','I'}, HLC_Class_insult},

	{{'W','O','R','D'}, HLC_Add_new_word},

	{{0,0,0,0}, NULL}
};

/* Word window method list */
static struct Method Word_window_methods[] = {
	{ INIT_METHOD, Init_Word_window_object },
	{ EXIT_METHOD, Exit_Window_object },
	{ DRAW_METHOD, Draw_Word_window_object },
	{ UPDATE_METHOD, Update_help_line },
	{ RIGHT_METHOD, Close_Window_object },
	{ CUSTOMKEY_METHOD, Customkeys_Word_window_object },
	{ CLOSE_METHOD, Close_Window_object },
	{ TOUCHED_METHOD, Dehighlight },
	{ RESTORE_METHOD, Restore_window },
	{ 0, NULL}
};

/* Word window class description */
static struct Object_class Word_window_Class = {
	OBJECT_DISPLAY_BASE, sizeof(struct Word_window_object),
	&Word_window_methods[0]
};

/* Word method list */
static struct Method Word_methods[] = {
	{ INIT_METHOD, Init_Word_object },
	{ DRAW_METHOD, Draw_Word_object },
	{ FEEDBACK_METHOD, Feedback_Word_object },
	{ HIGHLIGHT_METHOD, Highlight_Word_object },
	{ LEFT_METHOD, Left_Word_object },
	{ TOUCHED_METHOD, Normal_touched },
	{ 0, NULL}
};

/* Word class description */
static struct Object_class Word_Class = {
	0, sizeof(struct Word_object),
	&Word_methods[0]
};

/* Error messages */
static UNCHAR _Gametext_library_name[] = "Gametext";

static struct Error_message _Gametext_errors[] = {
	{ERROR_SYSTEXT_LOAD, 				"System texts could not be loaded."},
	{ERROR_ILLEGAL_TEXT_BLOCK_NR,		"Illegal text block number."},
	{ERROR_SYSTEXT_BLOCK_CONFLICT,	"System text block conflict."},
	{ERROR_ILLEGAL_SYSTEXT_NR, 		"Illegal system text number."},
	{ERROR_SUBBLOCK_CONFLICT, 			"Sub-block conflict."},
	{ERROR_ILLEGAL_SUBBLOCK_NR, 		"Illegal sub-block number."},
	{ERROR_TOO_MANY_WORDS,				"Too many words in the merged dictionary."},
	{0, NULL}
};

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_game_texts
 * FUNCTION  : Initialize game texts.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 16:13
 * LAST      : 29.06.95 18:09
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - Success, FALSE - Failure.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_game_texts(void)
{
	UNBYTE *Ptr;

	/* Load system texts file */
	System_text_handle = Load_file(SYSTEM_TEXTS + Language);
	if (!System_text_handle)
	{
		Gametext_error(ERROR_SYSTEXT_LOAD);
		return FALSE;
	}

	/* Convert system texts file */
	Ptr = MEM_Claim_pointer(System_text_handle);

	Convert_text_file(Ptr, MEM_Get_block_size(System_text_handle),
	 System_text_ptrs, MAX_SYSTEM_TEXTS);

	MEM_Free_pointer(System_text_handle);

	/* Initialize dictionary */
	Init_dictionary();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_game_texts
 * FUNCTION  : Exit game texts.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 16:13
 * LAST      : 29.06.95 18:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_game_texts(void)
{
	UNSHORT i;

	/* Remove references to system texts */
	for (i=0;i<MAX_SYSTEM_TEXTS;i++)
	{
		System_text_ptrs[i] = NULL;
	}

	/* Destroy system texts */
	MEM_Free_memory(System_text_handle);

	/* Destroy dictionary */
	Exit_dictionary();
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_text_block
 * FUNCTION  : Find a (compiled) text block in a text file.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 16:45
 * LAST      : 23.12.94 10:12
 * INPUTS    : UNBYTE *Ptr - Pointer to start of text file.
 *             UNSHORT Number - Text number (0...).
 * RESULT    : UNCHAR * - Pointer to text block.
 * BUGS      : No known.
 * NOTES     : - This function will return a pointer to an error text if
 *              an error occurs.
 *             - This function takes a pointer instead of a memory handle
 *              because the calling program should take care of claiming
 *              AND FREEING of the handle.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
Find_text_block(UNBYTE *Ptr, UNSHORT Number)
{
	UNSHORT Nr_text_blocks, i;
	UNSHORT *Block_lengths;
	static UNCHAR Error_text[] = "Wrong text number.";

	/* Read number of text blocks */
	Nr_text_blocks = *((UNSHORT *) Ptr);

	/* Legal text block number ? */
	if (Number >= Nr_text_blocks)
	{
		/* No -> Error */
		Gametext_error(ERROR_ILLEGAL_TEXT_BLOCK_NR);

		/* Return error text */
		return Error_text;
	}

	/* Find text block */
	Ptr += 2;
	Block_lengths = (UNSHORT *) Ptr;
	Ptr += Nr_text_blocks * 2;

	for (i=0;i<Number;i++)
		Ptr += Block_lengths[i];

	return (UNCHAR *) Ptr;
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_text_file
 * FUNCTION  : Scan a text file and divide into text blocks.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.10.94 17:34
 * LAST      : 25.10.94 17:34
 * INPUTS    : UNCHAR *Start - Pointer to start of text file.
 *             UNLONG Size - Size of text file.
 *             UNCHAR **Text_block_ptrs - Pointer to an array of pointers
 *              to text blocks.
 *             UNSHORT Max_text_blocks - Number of entries in array.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The format for text blocks is :
 *              [0000:...], where 0000 is the text block number and ... is
 *              the actual string.
 *             - The text block numbers start at 0.
 *             - It is assumed the text printing routine can handle (ignore)
 *              the carriage returns in the text. These aren't filtered.
 *             - The pointers will point to the start of the block. The text
 *              blocks will be terminated by a 0.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Convert_text_file(UNCHAR *Start, UNLONG Size, UNCHAR **Text_block_ptrs,
 UNSHORT Max_text_blocks)
{
	UNSHORT Text_block_nr, i;
	UNCHAR Number[] = "0000", *Ptr;

	/* Insert EOL at end of text block */
	*(Start + Size - 1) = 0;

	/* Clear pointers to text blocks */
	for (i=0;i<Max_text_blocks;i++)
	{
		Text_block_ptrs[i] = NULL;
	}

	/* Search file for text blocks */
	Ptr = Start;
	for (;;)
	{
		/* Find start of text block */
		Ptr = strchr(Ptr, (int) '[');

		/* Exit if no start was found */
		if (!Ptr)
			break;

		/* Skip '[' */
		Ptr++;

		/* Extract number of text block */
		strncpy(Number, Ptr ,4);
		Text_block_nr = (UNSHORT) strtoul(Number, NULL, 10);

		/* Skip number and colon */
		Ptr += 5;

		/* Legal text block number ? */
		if (Text_block_nr < Max_text_blocks)
		{
			/* Yes -> Is this text block already taken ? */
			if (Text_block_ptrs[Text_block_nr])
			{
				/* Yes -> Error */
				Gametext_error(ERROR_SYSTEXT_BLOCK_CONFLICT);
			}
			else
			{
				/* No -> Write pointer to text block in text list */
				Text_block_ptrs[Text_block_nr] = Ptr;
			}
		}
		else
		{
			/* No -> Error */
			Gametext_error(ERROR_ILLEGAL_SYSTEXT_NR);
		}

		/* Search end of text block */
		Ptr = strchr(Ptr, (int) ']');

		/* Exit if no end was found */
		if (!Ptr)
			break;

		/* Set EOL at the end of the text block */
		*Ptr++ = 0;
	}

	/* Insert pointers in all unused text blocks */
	for (i=0;i<Max_text_blocks;i++)
	{
		if (!Text_block_ptrs[i])
			Text_block_ptrs[i] = Default_system_text;
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Relocate_system_texts
 * FUNCTION  : Relocate the system texts.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.11.94 17:56
 * LAST      : 14.11.94 17:56
 * INPUTS    : MEM_HANDLE Handle - Handle of memory block.
 *             UNBYTE *Source - Pointer to source.
 *             UNBYTE *Target - Pointer to target.
 *             UNLONG Size - Size of memory block.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This will only work for one system texts file.
 * SEE ALSO  : BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Relocate_system_texts(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target,
 UNLONG Size)
{
	SILONG Diff;
	UNSHORT i;

	/* Copy the memory block */
	BASEMEM_CopyMem(Source,Target,Size);

	/* Adjust pointers to system texts */
	Diff = (UNLONG) Target - (UNLONG) Source;

	for (i=0;i<MAX_SYSTEM_TEXTS;i++)
	{
		if (System_text_ptrs[i])
			System_text_ptrs[i] += Diff;
	}
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_subblock_catalogue
 * FUNCTION  : Scan a text block and build a catalogue of sub-blocks.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 19:15
 * LAST      : 27.06.95 19:36
 * INPUTS    : UNCHAR *Start - Pointer to start of text block.
 *             UNLONG *Sub_block_offsets - Pointer to an array of offsets
 *              to text sub-blocks.
 *             UNSHORT Max_sub_blocks - Number of entries in array.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The start of a sub-block is marked by the {BLOKxxx}-command,
 *              where xxx is the sub-block number.
 *             - The end of a sub-block is marked by the next BLOK-command,
 *              or the end of the text.
 *             - All text before the first BLOK-command is ignored. Therefore
 *              offsets cannot be zero, as all blocks start witht a command.
 *             - The text block numbers start at 0.
 *             - This function assumes that the text block ends with an EOL.
 *             - The pointers will point to the start of the block.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Build_subblock_catalogue(UNCHAR *Start, UNLONG *Sub_block_offsets,
 UNSHORT Max_sub_blocks)
{
	UNSHORT Sub_block_nr, i;
	UNCHAR Command[4];
	UNCHAR Number[] = "000";
	UNCHAR *Ptr;

	/* Clear offsets to sub-blocks */
	for (i=0;i<Max_sub_blocks;i++)
	{
		Sub_block_offsets[i] = 0;
	}

	/* Search file for sub-blocks */
	Ptr = Start;
	for (;;)
	{
		/* Find start of text command */
		Ptr = strchr(Ptr, (int) COMSTART_CHAR);

		/* Exit if no start was found */
		if (!Ptr)
			break;

		/* Skip start of command */
		Ptr++;

		/* Read text command */
		strncpy(Command, Ptr, 4);
		Ptr += 4;

		/* Is BLOK ? */
		if (!(strncmp(Command, "BLOK", 4)))
		{
			/* Yes -> Extract number of sub-block */
			strncpy(Number, Ptr ,3);
			Sub_block_nr = (UNSHORT) strtoul(Number, NULL, 10);

			/* Skip number and end of command */
			Ptr += 4;

			/* Legal sub-block number ? */
			if (Sub_block_nr < Max_sub_blocks)
			{
				/* Yes -> Is this text block already taken ? */
				if (Sub_block_offsets[Sub_block_nr])
				{
					/* Yes -> Error */
					Gametext_error(ERROR_SUBBLOCK_CONFLICT);
				}
				else
				{
					/* No -> Store offset to sub-block in catalogue */
					Sub_block_offsets[Sub_block_nr] = (UNLONG)(Ptr - Start);
				}
			}
			else
			{
				/* No -> Error */
				Gametext_error(ERROR_ILLEGAL_SUBBLOCK_NR);
			}
		}
	}
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_dictionary
 * FUNCTION  : Initialize the dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 19:47
 * LAST      : 01.07.95 16:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_dictionary(void)
{
	MEM_HANDLE Dictionary_handles[NR_DICTIONARIES];
	BOOLEAN Result;
	UNSHORT Batch[NR_DICTIONARIES];
	UNSHORT i;

	/* Build dictionary batch */
	for (i=0;i<NR_DICTIONARIES;i++)
		Batch[i] = i + 1;

	/* Load dictionaries */
	Result = Load_full_batch(DICTIONARY, NR_DICTIONARIES, Batch,
	 Dictionary_handles);
	if (!Result)
	{
		Error(ERROR_FILE_LOAD);
		return;
	}

	/* Allocate memory for merged dictionary */
	Merged_dictionary_handle = MEM_Allocate_memory(WORDS_MAX * WORD_LENGTH);

	/* Clear merged dictionary word counter */
	Nr_merged_words = 0;

	/* Reset index list */
	for (i=0;i<WORDS_MAX;i++)
	{
		Merged_dictionary_index_list[i] = 0xFFFF;
	}

	/* Merge all dictionaries */
	for (i=0;i<NR_DICTIONARIES;i++)
	{
		/* Merge dictionary */
		Merge_dictionary(Dictionary_handles[i], i * WORDS_PER_DICTIONARY,
		 WORDS_PER_DICTIONARY);

		/* Kill dictionary memory */
		MEM_Kill_memory(Dictionary_handles[i]);
	}

	/* Shrink merged dictionary */
	MEM_Resize_memory(Merged_dictionary_handle, Nr_merged_words * WORD_LENGTH);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_dictionary
 * FUNCTION  : Exit the dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.06.95 19:47
 * LAST      : 26.06.95 16:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_dictionary(void)
{
	/* Free merged dictionary memory */
	MEM_Free_memory(Merged_dictionary_handle);

	/* Clear merged dictionary word counter */
	Nr_merged_words = 0;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_word_in_dictionary
 * FUNCTION  : Find a word in a dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.06.95 09:58
 * LAST      : 27.06.95 11:22
 * INPUTS    : UNSHORT Word_index - Word index (0...).
 *             UNCHAR *Word_ptr - Pointer to word buffer.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The word buffer should be at least WORD_LENGTH long.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Find_word_in_dictionary(UNSHORT Word_index, UNCHAR *Word_ptr)
{
	static UNCHAR *Illegal_word = "ILLEGAL";
	UNSHORT Merged_word_index;
	UNCHAR *Dictionary_ptr;

	/* Legal word index ? */
	if (Word_index >= WORDS_MAX)
	{
		/* No -> Copy ILLEGAL word */
		strcpy(Word_ptr, Illegal_word);

		return;
	}

	/* Get pointer to merged dictionary */
	Dictionary_ptr = (UNCHAR *) MEM_Claim_pointer(Merged_dictionary_handle);

	/* Get index of requested word */
	Merged_word_index = Merged_dictionary_index_list[Word_index];

	/* Copy word */
	strcpy(Word_ptr, Dictionary_ptr + (Merged_word_index * WORD_LENGTH));

	MEM_Free_pointer(Merged_dictionary_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_merged_word_index
 * FUNCTION  : Get the index of a word in the merged dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 12:05
 * LAST      : 30.06.95 12:05
 * INPUTS    : UNSHORT Word_index - Word index (0...).
 * RESULT    : UNSHORT : Merged word index / 0xFFFF = illegal word.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_merged_word_index(UNSHORT Word_index)
{
	UNSHORT Merged_word_index = 0xFFFF;

	/* Legal word index ? */
	if (Word_index < WORDS_MAX)
	{
		/* Yes -> Get index of requested word */
		Merged_word_index = Merged_dictionary_index_list[Word_index];
	}

	return Merged_word_index;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_word_in_dictionary
 * FUNCTION  : Search a word in a dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.06.95 10:05
 * LAST      : 29.06.95 16:29
 * INPUTS    : UNCHAR *Input_word - Pointer to word.
 * RESULT    : UNSHORT : Index of word in merged dictionary.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_word_in_dictionary(UNCHAR *Input_word)
{
	BOOLEAN Equal;
	UNSHORT Merged_word_index = 0xFFFF;
	UNSHORT Input_word_length;
	UNSHORT i, j;
	UNCHAR *Word_ptr;

	Word_ptr = (UNCHAR *) MEM_Claim_pointer(Merged_dictionary_handle);

	/* Determine length of input word */
	Input_word_length = strlen(Input_word);

	/* Search all words in the merged dictionary */
	for (i=0;i<Nr_merged_words;i++)
	{
		/* Does the current word have the same length as the input word */
		if (strlen(Word_ptr) == Input_word_length)
		{
			/* Yes -> Compare the current word with the input word */
			Equal = TRUE;
			for (j=0;j<Input_word_length;j++)
			{
				if (toupper(Word_ptr[j]) != toupper(Input_word[j]))
				{
					Equal = FALSE;
					break;
				}
			}

			/* Is this the input word ? */
			if (Equal)
			{
				/* Yes -> Found it */
				Merged_word_index = i;
				break;
			}
		}

		/* Next word */
		Word_ptr += WORD_LENGTH;
	}

	MEM_Free_pointer(Merged_dictionary_handle);

	return Merged_word_index;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_references_to_merged_word
 * FUNCTION  : Search all the references to a word in the merged dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:02
 * LAST      : 27.06.95 12:02
 * INPUTS    : UNSHORT Merged_word_index - Index of word in merged
 *              dictionary.
 *             UNSHORT *Word_index_ptr - Pointer to word index array.
 * RESULT    : UNSHORT : Number of found instances of this word.
 * BUGS      : No known.
 * NOTES     : - The word index array should have at least MAX_WORD_INSTANCES
 *              entries.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_references_to_merged_word(UNSHORT Merged_word_index,
 UNSHORT *Word_index_ptr)
{
	UNSHORT Nr_instances = 0;
	UNSHORT i;

	for (i=0;i<WORDS_MAX;i++)
	{
		/* Is this a reference to the merged word ? */
		if (Merged_dictionary_index_list[i] == Merged_word_index)
		{
			/* Yes -> Store word index in array */
			*Word_index_ptr++ = i;

			/* Count up */
			Nr_instances++;

			/* Too much ? */
			if (Nr_instances == MAX_WORD_INSTANCES)
			{
				/* Yes -> Break */
				break;
			}
		}
	}

	return Nr_instances;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Merge_dictionary
 * FUNCTION  : Add a dictionary to the merged dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.06.95 16:05
 * LAST      : 27.06.95 11:23
 * INPUTS    : MEM_HANDLE Dictionary_handle - Handle of dictionary.
 *             UNSHORT First_word - Index of first word to be processed
 *              (0...).
 *             UNSHORT Max_words - Maximum number of words to be processed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This is the ONLY function that accesses the dictionary
 *              files.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Merge_dictionary(MEM_HANDLE Dictionary_handle, UNSHORT First_word,
 UNSHORT Max_words)
{
	UNSHORT Nr_words;
	UNSHORT i;
	UNCHAR *Word_ptr;

	Word_ptr = (UNCHAR *) MEM_Claim_pointer(Dictionary_handle);

	/* Skip words */
	for (i=0;i<First_word;i++)
	{
		/* Skip word */
		Word_ptr += WORD_LENGTH;
	}

	/* Determine how many words should be processed */
	Nr_words = min(WORDS_MAX - First_word, Max_words);

	/* Check all words */
	for (i=First_word;i<First_word + Nr_words;i++)
	{
		/* Is empty ? */
		if (*Word_ptr)
		{
			/* No -> Add word to merged dictionary */
			Add_word_to_merged_dictionary(Word_ptr, i);
		}

		/* Skip word */
		Word_ptr += WORD_LENGTH;
	}

	MEM_Free_pointer(Dictionary_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_word_to_merged_dictionary
 * FUNCTION  : Add a word to the merged dictionary.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.06.95 15:55
 * LAST      : 26.06.95 17:39
 * INPUTS    : UNCHAR *New_word_ptr - Pointer to new word.
 *             UNSHORT Word_index - Word index (0...).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_word_to_merged_dictionary(UNCHAR *New_word_ptr, UNSHORT Word_index)
{
	UNSHORT i;
	UNCHAR *Merged_ptr;
	UNCHAR *Word_ptr;

	/* Is the merged dictionary full ? */
	if (Nr_merged_words == WORDS_MAX)
	{
		/* Yes -> Error */
		Dialogue_error(ERROR_TOO_MANY_WORDS);
		return;
	}

	Merged_ptr = (UNCHAR *) MEM_Claim_pointer(Merged_dictionary_handle);

	/* Check all words in the merged dictionary */
	Word_ptr = Merged_ptr;
	for (i=0;i<Nr_merged_words;i++)
	{
		/* Are these words the same ? */
		if (!strcmp(Word_ptr, New_word_ptr))
		{
			/* Yes -> Insert a reference in the index list */
			Merged_dictionary_index_list[Word_index] = i;

			/* Exit */
			MEM_Free_pointer(Merged_dictionary_handle);

			return;
		}

		/* Next word in the merged dictionary */
		Word_ptr += WORD_LENGTH;
	}

	/* The word wasn't found -> Add a new word */
	strcpy(Word_ptr, New_word_ptr);

	/* Insert a reference in the index list */
	Merged_dictionary_index_list[Word_index] = Nr_merged_words;

	/* Count up */
	Nr_merged_words++;

	MEM_Free_pointer(Merged_dictionary_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_word
 * FUNCTION  : Select a word from all known dictionary words.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 11:49
 * LAST      : 27.06.95 13:23
 * INPUTS    : None.
 * RESULT    : UNSHORT : Merged word index (0...) / 0xFFFF = aborted /
 *              0xFFFE = unknown word.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_word(void)
{
	struct Word_window_OID OID;
	UNSHORT Selected_word = 0xFFFF;
	UNSHORT Word_window_object;

	/* Do word window */
	OID.Result_ptr = &Selected_word;

	Push_module(&Window_Mod);

	Word_window_object = Add_object(0, &Word_window_Class,
	 (UNBYTE *) &OID, 50, 50, 200, 119);

	Execute_method(Word_window_object, DRAW_METHOD, NULL);

	Wait_4_object(Word_window_object);

	return Selected_word;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Word_window_object
 * FUNCTION  : Init method of Word Window object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:15
 * LAST      : 28.06.95 18:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Word_window_object(struct Object *Object, union Method_parms *P)
{
	static union Button_data Word_input_button_data;
	static struct Button_OID Word_input_button_OID = {
		TEXT_BUTTON_TYPE,
		0,
		0xFFFF,
		&Word_input_button_data,
		Word_input
	};

	struct Word_window_object *Word_window;
	struct Word_window_OID *OID;
	struct Word_OID Word_OID;
	struct Scroll_bar_OID Scroll_bar_OID;
	SISHORT Y;
	UNSHORT i;

	Word_window = (struct Word_window_object *) Object;
	OID = (struct Word_window_OID *) P;

	/* Copy data from OID */
	Word_window->Result_ptr = OID->Result_ptr;

	/* Build word list */
	Word_window->Nr_words = Build_word_list();

	/* Sort word list */
	Shellsort(Swap_words, Compare_words, Word_window->Nr_words, NULL);

	/* Scroll bar needed ? */
	if (Word_window->Nr_words > MAX_WORDS_IN_WINDOW)
	{
		/* Yes -> Calculate width and height WITH scroll bar */
		Change_object_size(Object->Self, WORD_OBJ_WIDTH + 27 + BETWEEN +
		 SCROLL_BAR_WIDTH, (MAX_WORDS_IN_WINDOW * (WORD_OBJ_HEIGHT + 1)) + 42);

		/* Centre window */
		Change_object_position(Object->Self, (Screen_width -
		 Object->Rect.width) / 2, (Panel_Y - Object->Rect.height) / 2);

		/* Make scroll bar */
		Scroll_bar_OID.Total_units = Word_window->Nr_words;
		Scroll_bar_OID.Units_width = 1;
		Scroll_bar_OID.Units_height = MAX_WORDS_IN_WINDOW;
		Scroll_bar_OID.Update = Update_word_list;

		/* Add object */
		Word_window->Scroll_bar_object = Add_object(Object->Self,
		 &Scroll_bar_Class, (UNBYTE *) &Scroll_bar_OID, WORD_OBJ_WIDTH + 13 +
		 BETWEEN, 12, SCROLL_BAR_WIDTH, (MAX_WORDS_IN_WINDOW *
		 (WORD_OBJ_HEIGHT + 1)) - 1);

		/* Make word objects */
		Y = 12;
	   Word_OID.Number = 0;
		for (i=0;i<MAX_WORDS_IN_WINDOW;i++)
		{
			/* Add object */
			Add_object(Object->Self, &Word_Class, (UNBYTE *) &Word_OID, 13, Y,
			 WORD_OBJ_WIDTH, WORD_OBJ_HEIGHT);

			/* Increase Y-coordinate */
			Y += WORD_OBJ_HEIGHT + 1;

			/* Count up */
			Word_OID.Number++;
		}
	}
	else
	{
		/* No -> Calculate width and height WITHOUT scroll bar */
		Change_object_size(Object->Self, WORD_OBJ_WIDTH + 27,
		 (Word_window->Nr_words * (WORD_OBJ_HEIGHT + 1)) + 42);

		/* Centre window */
		Change_object_position(Object->Self, (Screen_width -
		 Object->Rect.width) / 2, (Panel_Y - Object->Rect.height) / 2);

		/* Make word objects */
		Y = 12;
	   Word_OID.Number = 0;
		for (i=0;i<Word_window->Nr_words;i++)
		{
			/* Add object */
			Add_object(Object->Self, &Word_Class, (UNBYTE *) &Word_OID, 13, Y,
			 WORD_OBJ_WIDTH, WORD_OBJ_HEIGHT);

			/* Increase Y-coordinate */
			Y += WORD_OBJ_HEIGHT + 1;

			/* Count up */
			Word_OID.Number++;
		}

		/* Indicate there is no scroll bar */
		Word_window->Scroll_bar_object = 0;
	}

	/* Initialize button data */
	Word_input_button_data.Text_button_data.Text = System_text_ptrs[507];

	/* Add button to window */
 	Add_object(Object->Self, &Button_Class, (UNBYTE *) &Word_input_button_OID,
	 13 + (WORD_OBJ_WIDTH - 70) / 2, Y + 5, 70, 13);

	/* Prepare window */
	Prepare_window((struct Window_object *) Object);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Word_window_object
 * FUNCTION  : Draw method of Word Window object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:26
 * LAST      : 28.06.95 18:38
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Word_window_object(struct Object *Object, union Method_parms *P)
{
	struct Word_window_object *Word_window;
	struct OPM *OPM;
	UNSHORT W, H;

	Word_window = (struct Word_window_object *) Object;
	OPM = &(Word_window->Window_OPM);

	/* Get window dimensions */
	W = Object->Rect.width;
	H = Object->Rect.height;

	/* Draw window's shadow */
	Put_recoloured_box(OPM, 10, 10, W - 10, H - 10, &(Recolour_tables[0][0]));

	/* Draw window */
	Draw_window_inside(OPM, 7, 7, W - 14, H - 14);
	Draw_window_border(OPM, 0, 0, W, H);

	/* Any words known ? */
	if (Word_window->Nr_words)
	{
		/* Yes -> Draw border around words */
		Draw_deep_border(OPM, 12, 11, WORD_OBJ_WIDTH + 2, H - 41);
	}

	/* Draw words and buttons */
	Execute_child_methods(Object->Self, DRAW_METHOD, NULL);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Customkeys_Word_window_object
 * FUNCTION  : Custom keys method of Word_window object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.07.95 13:08
 * LAST      : 04.07.95 13:08
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Customkeys_Word_window_object(struct Object *Object, union Method_parms *P)
{
	union Method_parms P2;
	struct Word_window_object *Word_window;
	struct Word_list_entry *Word_list_ptr;
	UNSHORT Key_code;
	UNSHORT i;
	UNCHAR Char;
	UNCHAR *Dictionary_ptr;
	UNCHAR *Word_ptr;

	Word_window = (struct Word_window_object *) Object;

	/* Does the word window have a scroll-bar ? */
	if (!(Word_window->Scroll_bar_object))
	{
		/* No -> Exit */
		return 0;
	}

	/* Get key-code */
	Key_code = (UNSHORT) (P->Event->sl_key_code);

	/* Is legal ? */
	if ((Key_code < 32) || (Key_code > 255))
	{
		/* No -> Exit */
		return 0;
	}

	/* Get character */
	Char = (UNCHAR) (Key_code & 0x00FF);

	/* Make upper case */
	Char = toupper(Char);

	/* Is this a letter ? */
	if ((Char >= 'A') && (Char <= 'Z'))
	{
		/* Yes */
		Word_list_ptr = (struct Word_list_entry *)
		 MEM_Claim_pointer(Word_list_handle);
		Dictionary_ptr = (UNCHAR *)
		 MEM_Claim_pointer(Merged_dictionary_handle);

		/* Check all words */
		for (i=0;i<Word_window->Nr_words;i++)
		{
			/* Get current word */
			Word_ptr = Dictionary_ptr + (Word_list_ptr[i].Merged_word_index *
			 WORD_LENGTH);

			/* Does it start with right letter ? */
			if (toupper(Word_ptr[0]) == Char)
			{
				/* Yes -> Jump to this word */
				P2.Value = (UNLONG) i;
				Execute_method(Word_window->Scroll_bar_object, SET_METHOD, &P2);

				/* Break */
				break;
			}
		}
	}

	return 0;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Build_word_list
 * FUNCTION  : Build a list containing all known dictionary words.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 11:41
 * LAST      : 27.06.95 11:41
 * INPUTS    : None.
 * RESULT    : UNSHORT : Number of words in the word list.
 * BUGS      : No known.
 * NOTES     : - The handle of the word list will be stored in
 *              Word_list_handle.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Build_word_list(void)
{
	struct Word_list_entry *Word_list_ptr;
	UNSHORT Nr_words = 0;
	UNSHORT i, j;

	/* Allocate memory for word list */
	Word_list_handle = MEM_Allocate_memory(WORDS_MAX *
	 sizeof(struct Word_list_entry));

	Word_list_ptr = (struct Word_list_entry *)
	 MEM_Claim_pointer(Word_list_handle);

	/* Check all words in the merged dictionary */
	for (i=0;i<Nr_merged_words;i++)
	{
		/* Check all words in the dictionary */
		for (j=0;j<WORDS_MAX;j++)
		{
			/* Is this a reference to the word in the merged dictionary ? */
			if (Merged_dictionary_index_list[j] == i)
			{
				/* Yes -> Is this word known / cheat mode ? */
				if (Cheat_mode || Read_bit_array(KNOWN_WORDS_BIT_ARRAY, j))
				{
					/* Yes -> Create an entry for this word */
					Word_list_ptr->Merged_word_index = i;

					/* Is it a new word ? */
					if (Read_bit_array(NEW_WORDS_BIT_ARRAY, j))
					{
						/* Yes -> Mark */
						Word_list_ptr->Flags = WORD_NEW;
					}
					else
					{
						/* No */
						Word_list_ptr->Flags = 0;
					}

					/* Next entry */
					Word_list_ptr++;

					/* Count up */
					Nr_words++;
				}

				break;
			}
		}
	}
	MEM_Free_pointer(Word_list_handle);

	return Nr_words;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_words
 * FUNCTION  : Swap two words (for sorting).
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:11
 * LAST      : 28.06.95 14:11
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_words(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Word_list_entry *Word_list_ptr;
	struct Word_list_entry T;

	Word_list_ptr = (struct Word_list_entry *)
	 MEM_Claim_pointer(Word_list_handle);

	memcpy((UNBYTE *) &T, (UNBYTE *) (Word_list_ptr + A),
	 sizeof(struct Word_list_entry));
	memcpy((UNBYTE *) (Word_list_ptr + A), (UNBYTE *) (Word_list_ptr + B),
	 sizeof(struct Word_list_entry));
	memcpy((UNBYTE *) (Word_list_ptr + B), (UNBYTE *) &T,
	 sizeof(struct Word_list_entry));

	MEM_Free_pointer(Word_list_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_words
 * FUNCTION  : Compare two words (for sorting).
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.06.95 14:11
 * LAST      : 07.07.95 11:17
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : BOOLEAN : element A should come after element B.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_words(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct Word_list_entry *Word_list_ptr;
	BOOLEAN Result = FALSE;
	BOOLEAN Words_are_identical = TRUE;
	UNSHORT Length;
	UNSHORT i;
	UNCHAR *Dictionary_ptr;
	UNCHAR *Word_A, *Word_B;
	UNBYTE cA, cB;

	Word_list_ptr = (struct Word_list_entry *)
	 MEM_Claim_pointer(Word_list_handle);
	Dictionary_ptr = (UNCHAR *) MEM_Claim_pointer(Merged_dictionary_handle);

	/* Are both words new or old ? */
	if (((Word_list_ptr + A)->Flags & WORD_NEW) ==
	 ((Word_list_ptr + B)->Flags & WORD_NEW))
	{
		/* Yes -> Get pointers to words */
		Word_A = Dictionary_ptr + ((Word_list_ptr + A)->Merged_word_index *
		 WORD_LENGTH);
		Word_B = Dictionary_ptr + ((Word_list_ptr + B)->Merged_word_index *
		 WORD_LENGTH);

		/* Get length of shortest word */
		Length = min(strlen(Word_A), strlen(Word_B));

		/* Compare words */
		for (i=0;i<Length;i++)
		{
			/* Get characters and convert to upper case */
			cA = toupper(Word_A[i]);
			cB = toupper(Word_B[i]);

			/* Continue if both characters are the same */
			if (cA == cB)
				continue;

			/* Else compare these characters */
			if (cA > cB)
			{
				Result = TRUE;
			}

			/* These words are not identical */
			Words_are_identical = FALSE;

			/* Break */
			break;
		}

		/* Are these words identical ? */
		if (Words_are_identical)
		{
			/* Yes -> Compare lengths */
			if (strlen(Word_A) > strlen(Word_B))
				Result = TRUE;
		}
	}
	else
	{
		/* No -> New word gets priority */
		Result = (BOOLEAN) !((Word_list_ptr + A)->Flags & WORD_NEW);
	}

	MEM_Free_pointer(Merged_dictionary_handle);
	MEM_Free_pointer(Word_list_handle);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_word_list
 * FUNCTION  : Update the word list (scroll bar function).
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:15
 * LAST      : 27.06.95 12:15
 * INPUTS    : struct Scroll_bar_object *Scroll_bar - Pointer to scroll bar
 *              object.
 * RESULT    : None.
 * BUGS      : No known.
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_word_list(struct Scroll_bar_object *Scroll_bar)
{
	struct Object *Parent;
	UNSHORT Child;

	/* Get parent object data */
	Parent = Get_object_data(Scroll_bar->Object.Parent);

	/* Draw all child objects except scroll bar */
	Child = Parent->Child;
	while (Child)
	{
		/* Is scroll bar ? */
		if (Child != Scroll_bar->Object.Self)
		{
			/* No -> Draw */
			Execute_method(Child, DRAW_METHOD, NULL);
		}

		/* Next brother */
		Child = (Get_object_data(Child))->Next;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Word_input.
 * FUNCTION  : Enter a word (button).
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 13:30
 * LAST      : 29.06.95 15:03
 * INPUTS    : struct Button_object *Button - Pointer to button object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Word_input(struct Button_object *Button)
{
	struct Word_window_object *Word_window;
	UNSHORT Merged_word_index;
	UNCHAR Word[WORD_LENGTH];

	/* Get word selection window data */
	Word_window = (struct Word_window_object *)
	 Get_object_data(Button->Object.Parent);

	/* Enter a word */
	Word[0] = 0;
	Input_string_in_window(WORD_LENGTH - 1, NULL, Word);

	/* Any word entered ? */
	if (Word[0])
	{
		/* Yes -> Search word in merged dictionary */
		Merged_word_index = Search_word_in_dictionary(Word);

		/* Found it ? */
		if (Merged_word_index == 0xFFFF)
		{
			/* No -> Use UNKNOWN WORD code */
			Merged_word_index = 0xFFFE;
		}

		/* Select this word */
		*(Word_window->Result_ptr) = Merged_word_index;

		/* Close word selection window */
		Execute_method(Word_window->Object.Self, CLOSE_METHOD, NULL);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_Word_object
 * FUNCTION  : Init method of Word object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:29
 * LAST      : 27.06.95 12:29
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Init_Word_object(struct Object *Object, union Method_parms *P)
{
	struct Word_object *Word;
	struct Word_OID *OID;

	Word = (struct Word_object *) Object;
	OID = (struct Word_OID *) P;

	/* Copy data from OID */
	Word->Number = OID->Number;

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_Word_object
 * FUNCTION  : Draw method of Word object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:29
 * LAST      : 27.06.95 12:29
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Draw_Word_object(struct Object *Object, union Method_parms *P)
{
	struct Word_object *Word;
	struct Word_window_object *Word_window;
	struct OPM *OPM;

	Word = (struct Word_object *) Object;
	Word_window = (struct Word_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(Word_window->Window_OPM);

	/* Clear word area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw word */
	Draw_word(Word);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Feedback_Word_object
 * FUNCTION  : Feedback method of Word object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:29
 * LAST      : 27.06.95 12:29
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Feedback_Word_object(struct Object *Object, union Method_parms *P)
{
	struct Word_object *Word;
	struct Word_window_object *Word_window;
	struct OPM *OPM;

	Word = (struct Word_object *) Object;
	Word_window = (struct Word_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(Word_window->Window_OPM);

	/* Clear word area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Draw_deep_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);

	/* Draw word */
	Draw_word(Word);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Highlight_Word_object
 * FUNCTION  : Highlight method of Word object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:29
 * LAST      : 27.06.95 12:29
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Highlight_Word_object(struct Object *Object, union Method_parms *P)
{
	struct Word_object *Word;
	struct Word_window_object *Word_window;
	struct OPM *OPM;

	Word = (struct Word_object *) Object;
	Word_window = (struct Word_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(Word_window->Window_OPM);

	/* Clear word area */
	Draw_window_inside(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height);
	Put_recoloured_box(OPM, Object->X, Object->Y, Object->Rect.width,
	 Object->Rect.height, &(Recolour_tables[6][0]));

	/* Draw word */
	Draw_word(Word);

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Left_Word_object
 * FUNCTION  : Left method of Word object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:30
 * LAST      : 28.06.95 15:35
 * INPUTS    : struct Object *Object - Pointer to object.
 *             union Method_parms *P - Method parameters.
 * RESULT    : UNLONG : 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Left_Word_object(struct Object *Object, union Method_parms *P)
{
	struct Word_object *Word;
	struct Word_window_object *Word_window;
	struct Word_list_entry *Word_list_ptr;
	UNSHORT Word_list_index;

	Word = (struct Word_object *) Object;
	Word_window = (struct Word_window_object *)
	 Get_object_data(Object->Parent);

	/* Clicked ? */
	if (Normal_clicked(Object))
	{
		/* Yes -> Get word list index */
		Word_list_index = Word->Number;
		if (Word_window->Scroll_bar_object)
		{
			Word_list_index +=
			 (UNSHORT) Execute_method(Word_window->Scroll_bar_object,
			 GET_METHOD, NULL);
		}

		/* Get word data */
		Word_list_ptr = (struct Word_list_entry *)
		 MEM_Claim_pointer(Word_list_handle);

		Word_list_ptr += Word_list_index;

		/* Select this word */
		*(Word_window->Result_ptr) = Word_list_ptr->Merged_word_index;

		MEM_Free_pointer(Word_list_handle);

		/* Close word selection window */
		Execute_method(Word_window->Object.Self, CLOSE_METHOD, NULL);
	}

	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_word
 * FUNCTION  : Draw Word object.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.06.95 12:31
 * LAST      : 27.06.95 13:03
 * INPUTS    : struct Word_object *Word - Pointer to word object.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_word(struct Word_object *Word)
{
	struct Word_list_entry *Word_list_ptr;
	struct Object *Object;
	struct Word_window_object *Word_window;
	struct OPM *OPM;
	UNSHORT Word_list_index;
	UNCHAR *Dictionary_ptr;

	Object = &(Word->Object);
	Word_window = (struct Word_window_object *)
	 Get_object_data(Object->Parent);
	OPM = &(Word_window->Window_OPM);

	/* Get word list index */
	Word_list_index = Word->Number;
	if (Word_window->Scroll_bar_object)
	{
		Word_list_index +=
		 (UNSHORT) Execute_method(Word_window->Scroll_bar_object, GET_METHOD,
		 NULL);
	}

	/* Get word data */
	Word_list_ptr = (struct Word_list_entry *)
	 MEM_Claim_pointer(Word_list_handle);

	Word_list_ptr += Word_list_index;

	/* Select ink colour depending on word state */
	if (Word_list_ptr->Flags & WORD_NEW)
		Set_ink(GOLD_TEXT);
	else
		Set_ink(SILVER_TEXT);

	/* Print word */
	Dictionary_ptr = (UNCHAR *) MEM_Claim_pointer(Merged_dictionary_handle);

	Print_string(OPM, Object->X + 2, Object->Y + 1, Dictionary_ptr +
	 (Word_list_ptr->Merged_word_index * WORD_LENGTH));

	MEM_Free_pointer(Merged_dictionary_handle);

	MEM_Free_pointer(Word_list_handle);
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : HLC_ ...
 * FUNCTION  : High level text commands.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 14:45
 * LAST      : 01.07.95 17:12
 * INPUTS    : UNCHAR *Text - Pointer to string (after command).
 * RESULT    : UNCHAR * : Pointer to string that must be inserted / NULL.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
HLC_Unknown_word(UNCHAR *Text)
{
	return NULL;
}

UNCHAR *
HLC_Price(UNCHAR *Text)
{
	return NULL;
}

UNCHAR *
HLC_Damage(UNCHAR *Text)
{
	/* Convert current combat damage */
	sprintf(HLC_String, "%u", Combat_text_damage);

	return HLC_String;
}

UNCHAR *
HLC_Weapon(UNCHAR *Text)
{
	struct Item_packet Packet;

	/* Build fake packet */
	BASEMEM_FillMemByte((UNBYTE *) &Packet, sizeof(struct Item_packet), 0);
	Packet.Index = Combat_text_weapon_item_index;
	Packet.Quantity = 0;

	/* Get item name */
	Get_item_name(&Packet, HLC_String);

	return HLC_String;
}

UNCHAR *
HLC_Select_leader(UNCHAR *Text)
{
	/* Select active character */
	HLC_Subject_handle = Active_char_handle;

	return NULL;
}

UNCHAR *
HLC_Select_self(UNCHAR *Text)
{
	/* Select first party member */
	HLC_Subject_handle = Party_char_handles[0];

	return NULL;
}

UNCHAR *
HLC_Select_inventory(UNCHAR *Text)
{
	/* Select inventory character */
	HLC_Subject_handle = Inventory_char_handle;

	return NULL;
}

UNCHAR *
HLC_Select_dialogue(UNCHAR *Text)
{
	/* Select dialogue partner */
	HLC_Subject_handle = Dialogue_char_handle;

	return NULL;
}

UNCHAR *
HLC_Select_combat(UNCHAR *Text)
{
	/* Select currently acting combat participant */
	if (Current_acting_part)
		HLC_Subject_handle = Current_acting_part->Char_handle;
	else
		HLC_Subject_handle = NULL;

	return NULL;
}

UNCHAR *
HLC_Select_victim(UNCHAR *Text)
{
	/* Select victim of currently acting combat participant */
	if (Current_victim_part)
		HLC_Subject_handle = Current_victim_part->Char_handle;
	else
		HLC_Subject_handle = NULL;

	return NULL;
}

UNCHAR *
HLC_Select_PUM_char(UNCHAR *Text)
{
	/* Select character relative to current PUM */
	HLC_Subject_handle = PUM_char_handle;

	return NULL;
}

UNCHAR *
HLC_Select_subject_char(UNCHAR *Text)
{
	/* Select current subject character */
	HLC_Subject_handle = Subject_char_handle;

	return NULL;
}

UNCHAR *
HLC_Name(UNCHAR *Text)
{
	/* Get subject name */
	Get_char_name(HLC_Subject_handle, HLC_String);

	return HLC_String;
}

UNCHAR *
HLC_He(UNCHAR *Text)
{
	UNSHORT Nr;

	/* Select string depending on subject sex */
	switch (Get_sex(HLC_Subject_handle))
	{
		case MALE:
		{
			Nr = 75;
			break;
		}
		case FEMALE:
		{
			Nr = 76;
			break;
		}
		default:
		{
			Nr = 77;
			break;
		}
	}

	return System_text_ptrs[Nr];
}

UNCHAR *
HLC_Him(UNCHAR *Text)
{
	UNSHORT Nr;

	/* Select string depending on subject sex */
	switch (Get_sex(HLC_Subject_handle))
	{
		case MALE:
		{
			Nr = 78;
			break;
		}
		case FEMALE:
		{
			Nr = 79;
			break;
		}
		default:
		{
			Nr = 80;
			break;
		}
	}

	return System_text_ptrs[Nr];
}

UNCHAR *
HLC_His(UNCHAR *Text)
{
	UNSHORT Nr;

	/* Select string depending on subject sex */
	switch (Get_sex(HLC_Subject_handle))
	{
		case MALE:
		{
			Nr = 81;
			break;
		}
		case FEMALE:
		{
			Nr = 82;
			break;
		}
		default:
		{
			Nr = 83;
			break;
		}
	}

	return System_text_ptrs[Nr];
}

UNCHAR *
HLC_Man(UNCHAR *Text)
{
	UNSHORT Nr;

	/* Select string depending on subject sex */
	switch (Get_sex(HLC_Subject_handle))
	{
		case MALE:
		{
			Nr = 84;
			break;
		}
		case FEMALE:
		{
			Nr = 85;
			break;
		}
		default:
		{
			Nr = 86;
			break;
		}
	}

	return System_text_ptrs[Nr];
}

UNCHAR *
HLC_Race(UNCHAR *Text)
{
	return System_text_ptrs[Get_race(HLC_Subject_handle) + 456];
}

UNCHAR *
HLC_Class(UNCHAR *Text)
{
	return System_text_ptrs[Get_class(HLC_Subject_handle) + 95];
}

UNCHAR *
HLC_Race_insult(UNCHAR *Text)
{
	return System_text_ptrs[Get_race(HLC_Subject_handle) + 472];
}

UNCHAR *
HLC_Class_insult(UNCHAR *Text)
{
	return System_text_ptrs[Get_class(HLC_Subject_handle) + 135];
}

UNCHAR *
HLC_Add_new_word(UNCHAR *Text)
{
	UNSHORT Word_instances[MAX_WORD_INSTANCES];
	UNSHORT Merged_word_index;
	UNSHORT Nr_instances;
	UNSHORT i;
	UNCHAR *End_ptr;

	/* Find end of command */
	End_ptr = strchr(Text, (int) COMEND_CHAR);

	/* Exit if no end was found */
	if (!End_ptr)
		return NULL;

	/* Insert an EOL */
	*End_ptr = 0;

	/* Search word in merged dictionary */
	Merged_word_index = Search_word_in_dictionary(Text);

	/* Found it ? */
	if (Merged_word_index != 0xFFFF)
	{
		/* Yes -> Get references to selected word */
		Nr_instances = Search_references_to_merged_word(Merged_word_index,
		 Word_instances);

		/* Make all references known */
		for (i=0;i<Nr_instances;i++)
		{
			/* Modify known words bit array */
			Write_bit_array(KNOWN_WORDS_BIT_ARRAY, Word_instances[i],
			 SET_BIT_ARRAY);

			/* In Dialogue ? */
			if (In_Dialogue)
			{
				/* Yes -> Modify new words bit array as well */
				Write_bit_array(NEW_WORDS_BIT_ARRAY, Word_instances[i],
				 SET_BIT_ARRAY);
			}
		}
	}

	/* Restore end of command character */
	*End_ptr = COMEND_CHAR;

	return NULL;
}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Gametext_error
 * FUNCTION  : Report a gametext error.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 15:48
 * LAST      : 31.01.95 15:49
 * INPUTS    : UNSHORT : Error_code - Error code.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  : GAMETEXT.H, BBERROR.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Gametext_error(UNSHORT Error_code)
{
	struct Error_report Report;

	/* Build error report */
	Report.Code = Error_code;
	Report.Messages = &(_Gametext_errors[0]);

	/* Push error on the error stack */
	ERROR_PushError(Print_error, _Gametext_library_name,
	 sizeof(struct Error_report), (UNBYTE *) &Report);
}

