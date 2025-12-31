/************
 * NAME     : BUTTONS.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-1-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : BUTTONS.C
 ************/

#ifndef BUTTONS_H
#define BUTTONS_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

/* Button types */
#define TEXT_BUTTON_TYPE				(0)
#define SYMBOL_BUTTON_TYPE				(1)
#define DOUBLE_SYMBOL_BUTTON_TYPE	(2)

/* Button type flags */
#define BUTTON_BUTTON		(0)
#define SWITCH_BUTTON		(1<<8)
#define RADIO_BUTTON			(2<<8)
#define CONTINUOUS_BUTTON	(3<<8)

/*
 ** Structure definitions **************************************************
 */

/* Button data */
union Button_data {
	struct {
		UNCHAR *Text;
	} Text_button_data;
	struct {
		UNSHORT Width, Height;
		MEM_HANDLE Graphics_handle;
		UNLONG Graphics_offset;
	} Symbol_button_data;
	struct {
		UNSHORT Width, Height;
		MEM_HANDLE Graphics_handle;
		UNLONG Graphics_offset;
	} Double_symbol_button_data;
};

/* Button OID */
struct Button_OID {
	UNSHORT Type;
	UNSHORT Number;					/* Needed for radio buttons */
	UNSHORT Help_message_nr;		/* 0xFFFF = no help message */
	union Button_data *Data;
	void (*Function)(struct Button_object *);
};

/* Button object */
struct Button_object {
	struct Object Object;
	UNSHORT Type;
	UNSHORT State;
	UNSHORT Number;
	UNSHORT Help_message_nr;
	union Button_data *Data;
	void (*Function)(struct Button_object *);
};
/* State indicates on/off for switches and radio buttons, and first time
 for continuous buttons */

/* Radio button group object */
struct Radio_group_object {
	struct Object Object;
	UNSHORT State;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Button_Class;
extern struct Object_class Radio_group_Class;

/*
 ** Prototypes *************************************************************
 */

UNLONG Init_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Feedback_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Highlight_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Help_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Button_object(struct Object *Object, union Method_parms *P);
UNLONG Get_Button_object(struct Object *Object, union Method_parms *P);

UNLONG Draw_Radio_group_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Radio_group_object(struct Object *Object, union Method_parms *P);
UNLONG Get_Radio_group_object(struct Object *Object, union Method_parms *P);

UNSHORT Find_radio_button(struct Radio_group_object *Radio_group, UNSHORT Number);

#endif

