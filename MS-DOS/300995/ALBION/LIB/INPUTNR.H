/************
 * NAME     : INPUTNR.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 26-7-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef INPUTNR_H
#define INPUTNR_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* Number bar OID */
struct Number_bar_OID {
	SILONG *Value_ptr;
	SILONG Minimum;
	SILONG Maximum;
	void (*Update)(struct Number_bar_object *Number_bar);
};

/* Number bar object */
struct Number_bar_object {
	struct Object Object;
	UNSHORT Flags;

	SILONG *Value_ptr;
	SILONG Minimum;
	SILONG Maximum;

	void (*Update)(struct Number_bar_object *Number_bar);

	UNSHORT Slider_X;				/* X-coordinate of slider */
	SISHORT Delta_add;
};

/*
 ** External global variables **********************************************
 */

extern struct Object_class Number_bar_Class;

/*
 ** Prototypes *************************************************************
 */

/* Number input functions */
SILONG Input_number(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text);

SILONG Input_number_with_symbol(SILONG Value, SILONG Minimum, SILONG Maximum,
 UNCHAR *Text, UNSHORT Width, UNSHORT Height, MEM_HANDLE Graphics_handle,
 UNLONG Graphics_offset);

#endif

