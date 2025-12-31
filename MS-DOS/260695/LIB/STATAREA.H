/************
 * NAME     : STATAREA.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef STATAREA_H
#define STATAREA_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

#define HELP_LINE_INTERVAL		(200)
#define HELP_LINE_TEXT_MAX		(80)

#define PERMANENT_TEXT_INTERVAL		(200)
#define PERMANENT_TEXT_MINIMUM		(200 - 25)

/*
 ** Structure definitions **************************************************
 */

/* Party member OID */
struct Party_member_OID {
	UNSHORT Number;
};

/* Party member object */
struct Party_member_object {
	struct Object Object;
	UNSHORT Number;
	UNSHORT LP_bar_size;
	UNSHORT SP_bar_size;
};

/* Status area object */
struct Status_area_object {
	struct Object Object;
	UNSHORT Active_member;
};

/* Help-line object */
struct Help_line_object {
	struct Object Object;
	UNSHORT Counter;
	UNCHAR Text[HELP_LINE_TEXT_MAX];
};

/* Permanent text object */
struct Permanent_text_object {
	struct Object Object;
	UNSHORT Counter;
	struct Processed_text Processed;
	struct PA Print_area;
};

/*
 ** External global variables **********************************************
 */

extern UNSHORT Party_member_objects[6];

extern struct OPM Status_area_OPM;

extern UNSHORT Help_line_object;
extern UNSHORT Permanent_text_object;

/*
 ** Prototypes *************************************************************
 */

/* Status area initialization function */
void Init_status_area(void);

/* Status area object methods */
UNLONG Init_Status_area_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Status_area_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Status_area_object(struct Object *Object, union Method_parms *P);

UNLONG Update_Status_area(struct Object *Object, union Method_parms *P);

/* Party member object methods */
UNLONG Init_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG DLeft_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Pop_up_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Party_member_object(struct Object *Object, union Method_parms *P);
UNLONG Touch_Party_member_object(struct Object *Object, union Method_parms *P);

/* Party member display support functions */
void Draw_party_members(void);
void Highlight_party_members(UNSHORT Index);
void Feedback_party_members(UNSHORT Index);
void Display_party_status_bars(void);

/* Party member pop-up menu actions */
void Party_member_PUM_evaluator(struct PUM *PUM);
void PUM_Go_inventory(UNLONG Data);
void PUM_Use_magic(UNLONG Data);
void PUM_Make_active_member(UNLONG Data);
void PUM_Talk_to_member(UNLONG Data);

/* On-line help line object methods */
UNLONG Draw_Help_line_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Help_line_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Help_line_object(struct Object *Object, union Method_parms *P);

UNLONG Update_help_line(struct Object *Object, union Method_parms *P);

/* Permanent text area object methods */
UNLONG Init_Permanent_text_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Permanent_text_object(struct Object *Object, union Method_parms *P);
UNLONG Update_Permanent_text_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Permanent_text_object(struct Object *Object, union Method_parms *P);

/* Permanent text area access functions */
void Set_permanent_message_nr(UNSHORT Message_nr);
void Set_permanent_text(UNCHAR *Text);
void Clear_permanent_text(void);

#endif

