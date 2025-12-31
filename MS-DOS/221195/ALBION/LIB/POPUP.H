/************
 * NAME     : POPUP.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 20-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef POPUP_H
#define POPUP_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* Pop-up menu */
struct PUM {
	UNSHORT Total_entries;
	UNCHAR *Title;
	UNLONG Data;
	void (*Evaluator)(struct PUM *);
	struct PUME *PUME_list;
};

/* Pop-up menu entry */
struct PUME {
	UNSHORT Flags;
	UNSHORT Blocked_message_nr;
	UNSHORT Button_message_nr;
	void (*Function)(UNLONG);
};

/* Bitmasks for PUME.Flags */
#define PUME_ABSENT				(1<<0)
#define PUME_NOT_SELECTABLE	(1<<1)
#define PUME_BLOCKED				(1<<2)
#define PUME_AUTO_CLOSE			(1<<3)

/*
 ** External global variables **********************************************
 */

extern UNSHORT PUM_source_object_handle;

/*
 ** Prototypes *************************************************************
 */

/* Pop-up menu function */
void Do_PUM(SISHORT X, SISHORT Y, struct PUM *Data);
void Do_centered_PUM(SISHORT X, SISHORT Y, UNSHORT Width, UNSHORT Height,
 struct PUM *Data);

#endif

