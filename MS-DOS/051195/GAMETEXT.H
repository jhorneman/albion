/************
 * NAME     : GAMETEXT.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef GAMETEXT_H
#define GAMETEXT_H

#include <BBDEF.H>
#include <BBMEM.H>
#include <TEXT.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_SYSTEM_TEXTS		(800)

/* Maximum number of times one word can appear in the merged dictionary */
#define MAX_WORD_INSTANCES		(8)

#define WORD_LENGTH				(21)		/* Including EOL */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern UNCHAR *System_text_ptrs[];

extern struct HLC_Text_command HLC_commands[];

extern MEM_HANDLE PUM_char_handle;
extern MEM_HANDLE Subject_char_handle;

/*
 ** Prototypes *************************************************************
 */

/* Game text management functions */
BOOLEAN Init_game_texts(void);
void Exit_game_texts(void);

/* Dictionary management functions */
void Init_dictionary(void);
void Exit_dictionary(void);

//void Relocate_system_texts(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target,
// UNLONG Size);

UNCHAR *Find_text_block(UNBYTE *Ptr, UNSHORT Number);

void Build_subblock_catalogue(UNCHAR *Start, UNLONG *Sub_block_offsets,
 UNSHORT Max_sub_blocks);

void Find_word_in_dictionary(UNSHORT Word_index, UNCHAR *Word_ptr);

UNSHORT Get_merged_word_index(UNSHORT Word_index);

UNSHORT Search_word_in_dictionary(UNCHAR *Input_word);

UNSHORT Search_references_to_merged_word(UNSHORT Merged_word_index,
 UNSHORT *Word_index_ptr);

UNSHORT Select_word(void);

#endif

