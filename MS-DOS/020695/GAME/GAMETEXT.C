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
#include <DIALOGUE.H>
#include <COMBAT.H>

/* global variables */
UNCHAR *System_text_ptrs[MAX_SYSTEM_TEXTS];

static MEM_HANDLE System_text_handle;

MEM_HANDLE PUM_char_handle;
MEM_HANDLE Subject_char_handle;

static MEM_HANDLE HLC_Subject_handle;
static UNCHAR HLC_String[50];

static UNCHAR Default_system_text[] = "NOT DEFINED";

/* High-level text commands */
struct HLC_Text_command HLC_commands[] = {
	{{'W','O','R','D'}, HLC_New_word},
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

	{{0,0,0,0}, NULL}
};

static UNCHAR _Gametext_library_name[] = "Gametext";

static struct Error_message _Gametext_errors[] = {
	{ERROR_SYSTEXT_LOAD, 				"System texts could not be loaded."},
	{ERROR_ILLEGAL_TEXT_BLOCK_NR,		"Illegal text block number."},
	{ERROR_SYSTEXT_BLOCK_CONFLICT,	"System text block conflict."},
	{ERROR_ILLEGAL_SYSTEXT_NR, 		"Illegal system text number."},
	{ERROR_SUBBLOCK_CONFLICT, 			"Sub-block conflict."},
	{ERROR_ILLEGAL_SUBBLOCK_NR, 		"Illegal sub-block number."},
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
 * LAST      : 22.12.94 16:13
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
		return(FALSE);
	}

	/* Convert system texts file */
	Ptr = MEM_Claim_pointer(System_text_handle);

	Convert_text_file(Ptr, MEM_Get_block_size(System_text_handle),
	 System_text_ptrs, MAX_SYSTEM_TEXTS);

	MEM_Free_pointer(System_text_handle);

	return(TRUE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_game_texts
 * FUNCTION  : Exit game texts.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 16:13
 * LAST      : 22.12.94 16:13
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
		return(Error_text);
	}

	/* Find text block */
	Ptr += 2;
	Block_lengths = (UNSHORT *) Ptr;
	Ptr += Nr_text_blocks * 2;

	for (i=0;i<Number;i++)
		Ptr += Block_lengths[i];

	return((UNCHAR *) Ptr);
}

/*
 ****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Convert_text_file
 * FUNCTION  : Scan a text file and divide into text block.
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
 * NAME      : Build_sub_block_catalogue
 * FUNCTION  : Scan a text block and build a catalogue of sub-blocks.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 19:15
 * LAST      : 31.01.95 19:15
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
		Ptr += 3;

		/* Is BLOK ? */
		if (strncmp(Command, "BLOK", 4))
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
 * NAME      : HLC_ ...
 * FUNCTION  : High level text commands.
 * FILE      : GAMETEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 14:45
 * LAST      : 18.04.95 16:37
 * INPUTS    : UNCHAR *Text - Pointer to string (after command).
 * RESULT    : UNCHAR * : Pointer to string that must be inserted / NULL.
 * BUGS      : No known.
 * SEE ALSO  : TEXT.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNCHAR *
HLC_New_word(UNCHAR *Text)
{
	UNSHORT Number;

	/* Get word number */
	Number = Get_LLC_nr(Text);

	/* Mark word */
	Write_bit_array(KNOWN_WORDS_BIT_ARRAY, (UNLONG) Number, SET_BIT_ARRAY);
//	Write_bit_array(NEW_WORDS_BIT_ARRAY, (UNLONG) Number, SET_BIT_ARRAY);

	return(NULL);
}


UNCHAR *
HLC_Unknown_word(UNCHAR *Text)
{
	return(NULL);
}

UNCHAR *
HLC_Price(UNCHAR *Text)
{
	return(NULL);
}

UNCHAR *
HLC_Damage(UNCHAR *Text)
{
	/* Convert current combat damage */
	sprintf(HLC_String, "%u", Combat_text_damage);

	return(HLC_String);
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

	return(HLC_String);
}

UNCHAR *
HLC_Select_leader(UNCHAR *Text)
{
	/* Select active character */
	HLC_Subject_handle = Active_char_handle;

	return(NULL);
}

UNCHAR *
HLC_Select_self(UNCHAR *Text)
{
	/* Select first party member */
	HLC_Subject_handle = Party_char_handles[0];

	return(NULL);
}

UNCHAR *
HLC_Select_inventory(UNCHAR *Text)
{
	/* Select inventory character */
	HLC_Subject_handle = Inventory_char_handle;

	return(NULL);
}

UNCHAR *
HLC_Select_dialogue(UNCHAR *Text)
{
	/* Select dialogue partner */
	HLC_Subject_handle = Dialogue_char_handle;

	return(NULL);
}

UNCHAR *
HLC_Select_combat(UNCHAR *Text)
{
	/* Select currently acting combat participant */
	if (Current_acting_part)
		HLC_Subject_handle = Current_acting_part->Char_handle;
	else
		HLC_Subject_handle = NULL;

	return(NULL);
}

UNCHAR *
HLC_Select_victim(UNCHAR *Text)
{
	/* Select victim of currently acting combat participant */
	if (Current_victim_part)
		HLC_Subject_handle = Current_victim_part->Char_handle;
	else
		HLC_Subject_handle = NULL;

	return(NULL);
}

UNCHAR *
HLC_Select_PUM_char(UNCHAR *Text)
{
	/* Select character relative to current PUM */
	HLC_Subject_handle = PUM_char_handle;

	return(NULL);
}

UNCHAR *
HLC_Select_subject_char(UNCHAR *Text)
{
	/* Select current subject character */
	HLC_Subject_handle = Subject_char_handle;

	return(NULL);
}

UNCHAR *
HLC_Name(UNCHAR *Text)
{
	/* Get subject name */
	Get_char_name(HLC_Subject_handle, HLC_String);

	return(HLC_String);
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

	return(System_text_ptrs[Nr]);
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

	return(System_text_ptrs[Nr]);
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

	return(System_text_ptrs[Nr]);
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

	return(System_text_ptrs[Nr]);
}

UNCHAR *
HLC_Race(UNCHAR *Text)
{
	return(System_text_ptrs[Get_race(HLC_Subject_handle) + 456]);
}

UNCHAR *
HLC_Class(UNCHAR *Text)
{
	return(System_text_ptrs[Get_class(HLC_Subject_handle) + 95]);
}

UNCHAR *
HLC_Race_insult(UNCHAR *Text)
{
	return(System_text_ptrs[Get_race(HLC_Subject_handle) + 472]);
}

UNCHAR *
HLC_Class_insult(UNCHAR *Text)
{
	return(System_text_ptrs[Get_class(HLC_Subject_handle) + 135]);
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

