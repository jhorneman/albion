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

#define PUMES_MAX				(20)

/*
 ** Structure definitions **************************************************
 */

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

/* Pop-up menu */
struct PUM {
	UNSHORT Total_entries;
	UNCHAR *Title;
	UNLONG Data;
	void (*Evaluator)(struct PUM *);
	struct PUME *PUME_list;
};

/* Pop-up menu OID */
struct PUM_OID {
	struct PUM *Data;
	UNSHORT Nr_entries;
	UNSHORT Entry_list[PUMES_MAX];
};

/* Pop-up menu object */
struct PUM_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	struct PUM *Data;
	UNSHORT Nr_entries;
	UNSHORT Entry_list[PUMES_MAX];
};

/* Pop-up menu entry OID */
struct PUME_OID {
	UNSHORT Number;
};

/* Pop-up menu entry object */
struct PUME_object {
	struct Object Object;
	UNSHORT Number;
};

/*
 ** External global variables **********************************************
 */

extern UNSHORT PUM_source_object_handle;

/*
 ** Prototypes *************************************************************
 */

/* Pop-up menu function */
void Do_PUM(SISHORT X, SISHORT Y, struct PUM *Data);

/* Pop-up menu methods */
UNLONG Init_PUM_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_PUM_object(struct Object *Object, union Method_parms *P);

/* Pop-up menu entry methods */
UNLONG Init_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_PUME_object(struct Object *Object, union Method_parms *P);
UNLONG Left_PUME_object(struct Object *Object, union Method_parms *P);

#endif

