/************
 * NAME     : DIALOGUE.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

/* Dialogue types */
#define NORMAL_DIALOGUE			(0)
#define PLACE_DIALOGUE			(1)
#define RIDDLEMOUTH_DIALOGUE	(2)

/*
 ** Structure definitions **************************************************
 */

/* Dialogue node */
struct Dialogue_node {
	MEM_HANDLE Text_handle;
	UNSHORT Text_block_nr;
	UNSHORT Flags;
};

/* Bitmasks for Dialogue_node.Flags */
#define DIALOGUE_NO_DEFAULT		(1 << 0)

/*
 ** External global variables **********************************************
 */

extern BOOLEAN In_Dialogue;
extern BOOLEAN End_dialogue_flag;

extern struct OPM Dialogue_OPM;

extern MEM_HANDLE Dialogue_char_handle;
extern MEM_HANDLE Dialogue_small_portrait_handle;

extern MEM_HANDLE Dialogue_event_set_handles[2];
extern MEM_HANDLE Dialogue_event_text_handles[2];

extern UNSHORT Dialogue_type;
extern UNSHORT Dialogue_char_type;
extern UNSHORT Dialogue_char_index;

extern UNBYTE New_words_bit_array[];

/*
 ** Prototypes *************************************************************
 */

void Do_Dialogue(UNSHORT Type, UNSHORT Char_type, UNSHORT Char_index);

void Set_current_dialogue_node(struct Dialogue_node *Node);

/* Dictionary management functions */
void Init_dictionary(void);
void Exit_dictionary(void);

UNCHAR *Find_word_in_dictionary(UNCHAR *Dictionary, UNSHORT Word_index);
UNSHORT Search_word_in_dictionary(UNCHAR *Dictionary, UNCHAR *Input_word);

#endif

