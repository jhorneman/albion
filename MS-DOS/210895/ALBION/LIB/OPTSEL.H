/************
 * NAME     : OPTSEL.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 30-12-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef OPTSEL_H
#define OPTSEL_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_SPELLS_IN_WINDOW	(10)

#define SPELL_WIDTH		(192)
#define SPELL_HEIGHT		(12)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* Spell selection window OID */
struct Spell_window_OID {
	MEM_HANDLE Char_handle;
	struct Spell_object_data *Spell_info;
	UNSHORT *Selected_class_ptr;
	UNSHORT *Selected_spell_ptr;
	UNSHORT Nr_spells;
};

/* Spell selection window object */
struct Spell_window_object {
	/* This part MUST be the same as the Window struct */
	struct Object Object;
	MEM_HANDLE Background_handle;
	struct OPM Background_OPM;
	struct OPM Window_OPM;

	MEM_HANDLE Char_handle;
	struct Spell_object_data *Spell_info;
	UNSHORT *Selected_class_ptr;
	UNSHORT *Selected_spell_ptr;
	UNSHORT Nr_spells;

	UNSHORT Scroll_bar_object;
};

/* Spell object data */
struct Spell_object_data {
	UNSHORT Status;
	UNSHORT Class_nr;
	UNSHORT Spell_nr;
	UNSHORT Strength;
	UNSHORT Quantity;
};

/* Values for Spell_object_data.Status */
#define SPELL_OK						(0)
#define SPELL_WRONG_AREA			(1)
#define SPELL_TOO_MUCH_SP			(2)
#define SPELL_TOO_MUCH_LP			(3)

/* Spell OID */
struct Spell_OID {
	UNSHORT Number;
};

/* Spell object */
struct Spell_object {
	struct Object Object;
	UNSHORT Number;
};

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

/* Spell selection function */
BOOLEAN Select_spell(MEM_HANDLE Char_handle);

#endif
