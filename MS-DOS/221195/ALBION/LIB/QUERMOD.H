/************
 * NAME     : QUERMOD.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 28-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef QUERMOD_H
#define QUERMOD_H

#include <BBDEF.H>
#include <EVELOGIC.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Used_item_char_handle;
extern UNSHORT Used_item_slot_index;

/*
 ** Prototypes *************************************************************
 */

void Change_Used_Item_event(void);

void Datachange_event(void);

void Query_event(void);

void Modify_event(void);

#endif

