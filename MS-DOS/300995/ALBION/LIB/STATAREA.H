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

#define DAMAGE_DISPLAY_INTERVAL	(20)

/*
 ** Structure definitions **************************************************
 */

/* Party member object */
struct Party_member_object {
	struct Object Object;
	UNSHORT Number;

	UNSHORT LP_bar_size;
	UNSHORT SP_bar_size;

	UNSHORT Damage;
	UNSHORT Damage_display_timer;
};

/*
 ** External global variables **********************************************
 */

extern UNSHORT Party_display_object;
extern UNSHORT Party_member_objects[6];

extern struct OPM Status_area_OPM;

extern UNSHORT Help_line_object;
extern UNSHORT Permanent_text_object;

/*
 ** Prototypes *************************************************************
 */

/* Status area management functions */
void Init_status_area(void);

/* Party display management functions */
void Init_party_display(void);
void Reset_party_display(void);

/* Status area object methods */
UNLONG Update_Status_area(struct Object *Object, union Method_parms *P);

/* On-line help line object methods */
UNLONG Update_help_line(struct Object *Object, union Method_parms *P);

/* Permanent text area access functions */
void Set_permanent_message_nr(UNSHORT Message_nr);
void Set_permanent_text(UNCHAR *Text);
void Clear_permanent_text(void);

#endif

