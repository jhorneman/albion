/************
 * NAME     : SELWIN.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef SELWIN_H
#define SELWIN_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Type definitions *******************************************************
 */

typedef UNSHORT (*Selectable_evaluator)(UNSHORT Index, UNBYTE *Data);

typedef void (*Draw_selectable)(struct OPM *OPM, struct Object *Object,
 UNSHORT Index, UNBYTE *Data, BOOLEAN Blocked);

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

UNSHORT Do_select_window(UNCHAR *Text, Selectable_evaluator Evaluator,
 Draw_selectable Draw_function, UNSHORT Nr_entries, UNSHORT Entry_width,
 UNSHORT Entry_height, UNBYTE *Data);

#endif

