/************
 * NAME     : DIALOGUE.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : DIALOGUE.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <DIALOGUE.H>
#include <EVELOGIC.H>

/* global variables */

struct Module Dialogue_Mod = {
	0, SCREEN_MOD, DIALOGUE_SCREEN,
	Update_display,
	Dialogue_ModInit,
	Dialogue_ModExit,
	NULL,NULL,
	NULL
};

BOOLEAN In_Dialogue = FALSE;

struct OPM Dialogue_OPM;

MEM_HANDLE Dialogue_char_handle;
MEM_HANDLE Dialogue_small_portrait_handle;

MEM_HANDLE Dialogue_event_set_handles[2];
MEM_HANDLE Dialogue_event_text_handles[2];

UNSHORT Dialogue_char_type;
UNSHORT Dialogue_char_index;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_Dialogue
 * FUNCTION  : Start a dialogue.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.04.95 10:49
 * LAST      : 24.04.95 10:49
 * INPUTS    : UNSHORT Dialogue_type - Dialogue type.
 *             UNSHORT Char_type - Character type.
 *             UNSHORT Char_index - Character index.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_Dialogue(UNSHORT Dialogue_type, UNSHORT Char_type, UNSHORT Char_index)
{

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_ModInit
 * FUNCTION  : Initialize Dialogue module.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_ModInit(void)
{

	OPM_CreateVirtualOPM(&Main_OPM, &Dialogue_OPM, 0, 0, Screen_width, Panel_Y);

	Push_root(&Dialogue_OPM);

	Add_update_OPM(&Dialogue_OPM);


}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dialogue_ModExit
 * FUNCTION  : Exit Dialogue module.
 * FILE      : DIALOGUE.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dialogue_ModExit(void)
{
}


