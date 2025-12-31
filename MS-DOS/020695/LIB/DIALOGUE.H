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

/*
 ** External global variables **********************************************
 */

extern BOOLEAN In_Dialogue;

extern struct OPM Dialogue_OPM;

extern MEM_HANDLE Dialogue_char_handle;
extern MEM_HANDLE Dialogue_small_portrait_handle;

extern MEM_HANDLE Dialogue_event_set_handles[2];
extern MEM_HANDLE Dialogue_event_text_handles[2];

extern UNSHORT Dialogue_char_type;
extern UNSHORT Dialogue_char_index;

/*
 ** Prototypes *************************************************************
 */

void Do_Dialogue(UNSHORT Dialogue_type, UNSHORT Char_type, UNSHORT Char_index);

void Dialogue_ModInit(void);
void Dialogue_ModExit(void);

#endif

