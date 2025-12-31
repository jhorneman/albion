/************
 * NAME     : SCROLBAR.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 21-10-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <BBDEF.H>
#include <BBOPM.H>
#include <TEXT.H>
#include <CONTROL.H>
#include <USERFACE.H>

/*
 ** Defines ****************************************************************
 */

#define SCROLL_BAR_WIDTH	(6)

/*
 ** Structure definitions **************************************************
 */

/* Scroll-bar OID */
struct Scroll_bar_OID {
	UNSHORT Total_units;
	UNSHORT Units_width;
	UNSHORT Units_height;
	void (*Update)(struct Scroll_bar_object *Scroll_bar);
};

/* Scroll bar object */
struct Scroll_bar_object {
	struct Object Object;
	UNSHORT Flags;
	UNSHORT Total_units;
	UNSHORT Units_width;
	UNSHORT Units_height;
	UNSHORT Slider_height;		/* Height of slider in pixels */
	UNSHORT Slider_Y;				/* Y-coordinate of slider */
	UNSHORT Row_height;			/* Height of row in pixels */
	UNSHORT Current_row;			/* Current row */
	void (*Update)(struct Scroll_bar_object *Scroll_bar);
	UNSHORT Max_Y;					/* Maximum Y-coordinate of slider */
	UNSHORT Result;            /* Current unit selected by scroll bar (0...) */
};

/* Bitmasks for Scroll_bar.Flags */
#define SLIDER_SELECTED			(1<<0)

/*
 ** External global variables **********************************************
 */

extern struct Object_class Scroll_bar_Class;

extern UNSHORT Slider_colours[2][9];

/*
 ** Prototypes *************************************************************
 */

/* Scroll bar methods */
UNLONG Init_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Draw_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Set_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Get_Scroll_bar_object(struct Object *Object, union Method_parms *P);
UNLONG Left_Scroll_bar_object(struct Object *Object, union Method_parms *P);

/* Scroll bar support functions */
void Scroll_bar_page_up(struct Scroll_bar_object *Scroll_bar);
void Scroll_bar_page_down(struct Scroll_bar_object *Scroll_bar);
void Light_slider(struct Scroll_bar_object *Scroll_bar);
void Update_slider(struct Scroll_bar_object *Scroll_bar, UNSHORT Y);

/* Scroll bar display functions */
void Do_draw_scroll_bar(struct Scroll_bar_object *Scroll_bar);

#endif

