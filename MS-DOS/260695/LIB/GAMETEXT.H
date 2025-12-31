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

/* Error codes */
#define ERROR_SYSTEXT_LOAD					(1)
#define ERROR_ILLEGAL_TEXT_BLOCK_NR		(2)
#define ERROR_SYSTEXT_BLOCK_CONFLICT	(3)
#define ERROR_ILLEGAL_SYSTEXT_NR			(4)
#define ERROR_SUBBLOCK_CONFLICT			(5)
#define ERROR_ILLEGAL_SUBBLOCK_NR		(6)

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

BOOLEAN Init_game_texts(void);
void Exit_game_texts(void);

void Convert_text_file(UNCHAR *Start, UNLONG Size, UNCHAR **Text_block_ptrs,
 UNSHORT Max_text_blocks);
void Relocate_system_texts(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target,
 UNLONG Size);
UNCHAR *Find_text_block(UNBYTE *Ptr, UNSHORT Number);
void Build_subblock_catalogue(UNCHAR *Start, UNLONG *Sub_block_offsets,
 UNSHORT Max_sub_blocks);

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

void Gametext_error(UNSHORT Error_code);

#endif

