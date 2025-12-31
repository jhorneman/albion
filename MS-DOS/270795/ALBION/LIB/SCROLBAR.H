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
#include <CONTROL.H>

/*
 ** Defines ****************************************************************
 */

#define BETWEEN				(5)

#define SCROLL_BAR_WIDTH	(6)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

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

/* Scroll-bar OID */
struct Scroll_bar_OID {
	UNSHORT Total_units;
	UNSHORT Units_width;
	UNSHORT Units_height;
	void (*Update)(struct Scroll_bar_object *Scroll_bar);
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Scroll_bar_Class;

extern UNSHORT Slider_colours[2][9];

/*
 ** Prototypes *************************************************************
 */

#endif

