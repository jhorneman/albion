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

/*
 ** Structure definitions **************************************************
 */

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

/* Status area management functions */
void Init_status_area(void);
void Reset_status_area(void);

/* Status area object methods */
UNLONG Update_Status_area(struct Object *Object, union Method_parms *P);

/* On-line help line object methods */
UNLONG Update_help_line(struct Object *Object, union Method_parms *P);

/* Permanent text area access functions */
void Set_permanent_message_nr(UNSHORT Message_nr);
void Set_permanent_text(UNCHAR *Text);
void Clear_permanent_text(void);

#endif

