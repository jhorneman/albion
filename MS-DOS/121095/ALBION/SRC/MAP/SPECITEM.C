/************
 * NAME     : SPECITEM.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBMEM.H>

#include <HDOB.H>
#include <GFXFUNC.H>
#include <SCROLL.H>

#include <SPECITEM.H>
#include <NPCS.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <MAP.H>
#include <3D_MAP.H>
#include <3DCOMMON.H>
#include <GRAPHICS.H>
#include <PRTLOGIC.H>
#include <2D_DISPL.H>

/* defines */

#define COMPASS_X			(7)
#define COMPASS_Y			(5)

#define MONSTER_EYE_X	(5)
#define MONSTER_EYE_Y	(40)

#define CLOCK_X			(5)
#define CLOCK_Y			(74)

/* prototypes */

void Init_compass(void);
void Init_clock(void);

void Exit_compass(void);
void Exit_clock(void);

void Update_compass(void);
void Update_clock(void);

/* global variables */

/* Compass OPM data */
MEM_HANDLE Compass_OPM_handle;
static struct OPM Compass_OPM;

/* Clock OPM data */
MEM_HANDLE Clock_OPM_handle;
static struct OPM Clock_OPM;

/* Special item HDOB handles */
static UNSHORT Compass_HDOB_nr;
static UNSHORT Monster_eye_HDOB_nr;
static UNSHORT Clock_HDOB_nr;

/* Special item HDOBs */
static struct HDOB Compass_HDOB = {
	HDOB_MASK,
	0,
	COMPASS_X, COMPASS_Y,
	30, 29,
	1,
	0,
	NULL,
	0,
	NULL,
	0,
	0,
	NULL
};
static struct HDOB Monster_eye_HDOB = {
	HDOB_MASK,
	0,
	MONSTER_EYE_X, MONSTER_EYE_Y,
	32, 27,
	1,
	0,
	NULL,
	(UNLONG) Eye_symbols,
	NULL,
	0,
	0,
	NULL
};
static struct HDOB Clock_HDOB = {
	HDOB_MASK,
	0,
	CLOCK_X, CLOCK_Y,
	32, 25,
	1,
	0,
	NULL,
	0,
	NULL,
	0,
	0,
	NULL
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_special_items
 * FUNCTION  : Initialize special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 14:51
 * LAST      : 23.08.95 18:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_special_items(void)
{
	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Initialize compass */
		Init_compass();

		/* Insert handle in HDOB data */
		Compass_HDOB.Graphics_handle = Compass_OPM_handle;

		/* Initialize clock */
		Init_clock();

		/* Insert handle in HDOB data */
		Clock_HDOB.Graphics_handle = Clock_OPM_handle;

		/* Mark HDOBs as invisible */
		Compass_HDOB_nr		= 0xFFFF;
		Monster_eye_HDOB_nr	= 0xFFFF;
		Clock_HDOB_nr			= 0xFFFF;
	}
	else
	{
		/* 2D -> Initialize compass */
		Init_compass();

		/* Initialize clock */
		Init_clock();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_compass
 * FUNCTION  : Initialize compass special item.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 18:47
 * LAST      : 23.08.95 18:47
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_compass(void)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Allocate memory for compass graphics */
	Compass_OPM_handle = MEM_Do_allocate(30 * 29, (UNLONG) &Compass_OPM, &OPM_ftype);
	if (!Compass_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return;
	}

	/* Clear memory */
	MEM_Clear_memory(Compass_OPM_handle);

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Compass_OPM_handle);
	Result = OPM_New(30, 29, 1, &Compass_OPM, Ptr);
	MEM_Free_pointer(Compass_OPM_handle);

	/* Success ? */
	if (!Result)
	{
		/* No -> Free OPM memory */
		MEM_Free_memory(Compass_OPM_handle);
		Compass_OPM_handle = NULL;

		/* Report error */
		Error(ERROR_NO_OPM);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_clock
 * FUNCTION  : Initialize clock special item.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 18:48
 * LAST      : 23.08.95 18:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_clock(void)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Allocate memory for clock graphics */
	Clock_OPM_handle = MEM_Do_allocate(32 * 25, (UNLONG) &Clock_OPM, &OPM_ftype);
	if (!Clock_OPM_handle)
	{
		Error(ERROR_OUT_OF_MEMORY);
		return;
	}

	/* Clear memory */
	MEM_Clear_memory(Clock_OPM_handle);

	/* Create a new OPM */
	Ptr = MEM_Claim_pointer(Clock_OPM_handle);
	Result = OPM_New(32, 25, 1, &Clock_OPM, Ptr);
	MEM_Free_pointer(Clock_OPM_handle);

	/* Success ? */
	if (!Result)
	{
		/* No -> Free OPM memory */
		MEM_Free_memory(Clock_OPM_handle);
		Clock_OPM_handle = NULL;

		/* Report error */
		Error(ERROR_NO_OPM);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_special_items
 * FUNCTION  : Exit special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 14:51
 * LAST      : 23.08.95 19:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_special_items(void)
{
	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Hide special items */
		Hide_special_items();

		/* Exit compass */
		Exit_compass();

		/* Exit clock */
		Exit_clock();
	}
	else
	{
		/* 2D -> Exit compass */
		Exit_compass();

		/* Exit clock */
		Exit_clock();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_compass
 * FUNCTION  : Exit compass special item.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 19:01
 * LAST      : 23.08.95 19:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_compass(void)
{
	/* Delete compass OPM and free memory */
	OPM_Del(&Compass_OPM);
	MEM_Free_memory(Compass_OPM_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_clock
 * FUNCTION  : Exit clock special item.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 19:01
 * LAST      : 23.08.95 19:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_clock(void)
{
	/* Delete clock OPM and free memory */
	OPM_Del(&Clock_OPM);
	MEM_Free_memory(Clock_OPM_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Show_special_items
 * FUNCTION  : Show special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 18:36
 * LAST      : 23.08.95 18:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_special_items(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Hide_special_items
 * FUNCTION  : Hide special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 18:35
 * LAST      : 23.08.95 18:35
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function is called by Exit_special items.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Hide_special_items(void)
{
	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Compass visible ? */
		if (Compass_HDOB_nr != 0xFFFF)
		{
			/* Yes -> Remove compass HDOB */
			Delete_HDOB(Compass_HDOB_nr);

			/* Mark HDOB as invisible */
			Compass_HDOB_nr = 0xFFFF;
		}

		/* Monster eye visible ? */
		if (Monster_eye_HDOB_nr != 0xFFFF)
		{
			/* Yes -> Remove monster_eye HDOB */
			Delete_HDOB(Monster_eye_HDOB_nr);

			/* Mark HDOB as invisible */
			Monster_eye_HDOB_nr = 0xFFFF;
		}

		/* Clock visible ? */
		if (Clock_HDOB_nr != 0xFFFF)
		{
			/* Yes -> Remove clock HDOB */
			Delete_HDOB(Clock_HDOB_nr);

			/* Mark HDOB as invisible */
			Clock_HDOB_nr = 0xFFFF;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_special_items_2D
 * FUNCTION  : Diplay special items in a 2D map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 20:12
 * LAST      : 23.08.95 20:12
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_special_items_2D(void)
{
	UNSHORT Special_item_flags;
	UNBYTE *Ptr;

	/* Cheat mode on ? */
	if (Cheat_mode)
	{
		/* Yes -> Party has all special items */
		Special_item_flags = 0xFFFF;
	}
	else
	{
		/* No -> Get special item flags */
		Special_item_flags = PARTY_DATA.Special_item_flags;
	}

	/* Does the party have the compass ? */
	if (Special_item_flags & MONSTER_EYE_ITEM_FLAG)
	{
		/* Yes -> Is a monster watching ? */
		if (Monster_is_watching)
		{
			/* Yes -> Open monster eye */
			Display_object_in_scroll_buffer(&Scroll_2D, Scroll_2D.Playfield_X +
			 MONSTER_EYE_X, Scroll_2D.Playfield_Y + MONSTER_EYE_Y, 32, 27,
			 &Eye_symbols[0] + 32 * 27);
		}
		else
		{
			/* No -> Close monster eye */
			Display_object_in_scroll_buffer(&Scroll_2D, Scroll_2D.Playfield_X +
			 MONSTER_EYE_X, Scroll_2D.Playfield_Y + MONSTER_EYE_Y, 32, 27,
			 &Eye_symbols[0]);
		}
	}

	/* Does the party have the clock ? */
	if (Special_item_flags & CLOCK_ITEM_FLAG)
	{
		/* Yes -> Update clock */
		Update_clock();

		/* Display clock */
		Ptr = MEM_Claim_pointer(Clock_OPM_handle);

		Display_object_in_scroll_buffer(&Scroll_2D, Scroll_2D.Playfield_X +
		 CLOCK_X, Scroll_2D.Playfield_Y + CLOCK_Y, 32, 25, Ptr);

		MEM_Free_pointer(Clock_OPM_handle);
	}
}

void
Prepare_2D_special_items_display(void)
{
	UNSHORT Special_item_flags;

	/* Cheat mode on ? */
	if (Cheat_mode)
	{
		/* Yes -> Party has all special items */
		Special_item_flags = 0xFFFF;
	}
	else
	{
		/* No -> Get special item flags */
		Special_item_flags = PARTY_DATA.Special_item_flags;
	}

	/* Does the party have the compass ? */
	if (Special_item_flags & MONSTER_EYE_ITEM_FLAG)
	{
		/* Yes -> */
		Update_2D_screen_area(MONSTER_EYE_X, MONSTER_EYE_Y, 32, 27);
	}

	/* Does the party have the clock ? */
	if (Special_item_flags & CLOCK_ITEM_FLAG)
	{
		/* Yes -> */
		Update_2D_screen_area(CLOCK_X, CLOCK_Y, 32, 25);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_special_items_3D
 * FUNCTION  : Diplay special items in a 3D map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 15:30
 * LAST      : 23.08.95 18:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_special_items_3D(void)
{
	UNSHORT Special_item_flags;

	/* Cheat mode on ? */
	if (Cheat_mode)
	{
		/* Yes -> Party has all special items */
		Special_item_flags = 0xFFFF;
	}
	else
	{
		/* No -> Get special item flags */
		Special_item_flags = PARTY_DATA.Special_item_flags;
	}

	/* Does the party have the compass ? */
	if (Special_item_flags & COMPASS_ITEM_FLAG)
	{
		/* Yes -> Update compass */
		Update_compass();

		/* Compass visible ? */
		if (Compass_HDOB_nr == 0xFFFF)
		{
			/* No -> Add compass HDOB */
			Compass_HDOB_nr = Add_HDOB(&Compass_HDOB);
		}
	}
	else
	{
		/* No -> Compass visible ? */
		if (Compass_HDOB_nr != 0xFFFF)
		{
			/* Yes -> Remove compass HDOB */
			Delete_HDOB(Compass_HDOB_nr);

			/* Mark HDOB as invisible */
			Compass_HDOB_nr = 0xFFFF;
		}
	}

	/* Does the party have the monster eye ? */
	if (Special_item_flags & MONSTER_EYE_ITEM_FLAG)
	{
		/* Yes -> Monster eye visible ? */
		if (Monster_eye_HDOB_nr == 0xFFFF)
		{
			/* No -> Add monster eye HDOB */
			Monster_eye_HDOB_nr = Add_HDOB(&Monster_eye_HDOB);
		}

		/* Is a monster watching ? */
		if (Monster_is_watching)
		{
			/* Yes -> Open monster eye */
			Monster_eye_HDOB.Graphics_offset = (UNLONG) (&Eye_symbols[0] +
				32 * 27);
		}
		else
		{
			/* No -> Close monster eye */
			Monster_eye_HDOB.Graphics_offset = (UNLONG) (&Eye_symbols[0]);
		}

		/* Change monster eye HDOB */
		Change_HDOB(Monster_eye_HDOB_nr, &Monster_eye_HDOB);
	}
	else
	{
		/* No -> Monster eye visible ? */
		if (Monster_eye_HDOB_nr != 0xFFFF)
		{
			/* Yes -> Remove monster_eye HDOB */
			Delete_HDOB(Monster_eye_HDOB_nr);

			/* Mark HDOB as invisible */
			Monster_eye_HDOB_nr = 0xFFFF;
		}
	}

	/* Does the party have the clock ? */
	if (Special_item_flags & CLOCK_ITEM_FLAG)
	{
		/* Yes -> Update clock */
		Update_clock();

		/* Clock visible ? */
		if (Clock_HDOB_nr == 0xFFFF)
		{
			/* No -> Add clock HDOB */
			Clock_HDOB_nr = Add_HDOB(&Clock_HDOB);
		}
	}
	else
	{
		/* No -> Clock visible ? */
		if (Clock_HDOB_nr != 0xFFFF)
		{
			/* Yes -> Remove clock HDOB */
			Delete_HDOB(Clock_HDOB_nr);

			/* Mark HDOB as invisible */
			Clock_HDOB_nr = 0xFFFF;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_compass
 * FUNCTION  : Update compass special item.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 18:48
 * LAST      : 23.08.95 18:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_compass(void)
{
	static UNSHORT Compass_bit_frame = 0;

	UNSHORT Index;

	/* Re-build compass */
	Put_unmasked_block(&Compass_OPM, 0, 0, 30, 29, &Compass_symbol[0]);

	/* Get angle position index */
	Index = ((((0 - (I3DM.Camera_angle / 65536)) - (ANGLE_STEPS / 120)) &
	 (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 60) + 1) % 60;

	/* Draw compass bit */
	Put_masked_block(&Compass_OPM, Compass_bit_coordinates[Index][0],
	 Compass_bit_coordinates[Index][1], 6, 6, &Compass_bit[0] +
	 Compass_bit_frame * 36);

	/* Update compass bit animation frame */
	Compass_bit_frame = (Compass_bit_frame + 1) & 0x0007;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_clock
 * FUNCTION  : Update clock special item.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 18:48
 * LAST      : 23.08.95 18:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_clock(void)
{
	/* Re-build clock */
	Put_unmasked_block(&Clock_OPM, 0, 0, 32, 25, &Clock_symbol[0]);

	/* Draw first digit of current hour */
	Put_masked_block(&Clock_OPM, 2, 10, 6, 7, &Time_font[0] +
	 (PARTY_DATA.Hour / 10) * 42);

	/* Draw second digit of current hour */
	Put_masked_block(&Clock_OPM, 8, 10, 6, 7, &Time_font[0] +
	 (PARTY_DATA.Hour % 10) * 42);

	/* Draw first digit of current minute */
	Put_masked_block(&Clock_OPM, 16, 10, 6, 7, &Time_font[0] +
	 (PARTY_DATA.Minute / 10) * 42);

	/* Draw second digit of current minute */
	Put_masked_block(&Clock_OPM, 22, 10, 6, 7, &Time_font[0] +
	 (PARTY_DATA.Minute % 10) * 42);
}

