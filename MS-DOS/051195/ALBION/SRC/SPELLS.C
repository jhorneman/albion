/************
 * NAME     : SPELLS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-5-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : SPELLS.H
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <GAMETEXT.H>
#include <MAGIC.H>
#include <SOUND.H>
#include <MAP.H>
#include <CAMP.H>
#include <EVELOGIC.H>
#include <COMBAT.H>
#include <COMMAGIC.H>
#include <STATAREA.H>
#include <ITEMLIST.H>
#include <INPUT.H>
#include <TACTICAL.H>
#include <SPELLS.H>
#include <ITMLOGIC.H>
#include <AUTOMAP.H>
#include <TEXTWIN.H>

/* prototypes */

void Do_C0_Spell_9(UNSHORT Member_nr, UNSHORT Strength);

void Do_C0_Spell_17(UNSHORT Member_nr, UNSHORT Strength);

void Do_C0_Spell_18(UNSHORT Member_nr, UNSHORT Strength);

void Do_C0_Spell_19(UNSHORT Member_nr, UNSHORT Strength);

void Do_C1_Spell_1(UNSHORT Member_nr, UNSHORT Strength);

void Do_C1_Spell_5(UNSHORT Member_nr, UNSHORT Strength);

void Do_C1_Spell_11(UNSHORT Member_nr, UNSHORT Strength);

void Do_C2_Spell_7(UNSHORT Member_nr, UNSHORT Strength);

/* global variables */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_9_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.04.95 14:50
 * LAST      : 05.10.95 15:14
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_9_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_9);
}

void
Do_C0_Spell_9(UNSHORT Member_nr, UNSHORT Strength)
{
	SISHORT Max_LP;
	SISHORT Added_LP;

	/* Get max LP */
	Max_LP = Get_max_LP(Party_char_handles[Member_nr - 1]);

	/* Calculate added LP */
	Added_LP = max(1, (Max_LP * 25 * Strength) / 10000);

	/* Heal */
	Do_healing(Member_nr, Added_LP);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_17_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 15:16
 * LAST      : 05.10.95 15:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_17_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_17);
}

void
Do_C0_Spell_17(UNSHORT Member_nr, UNSHORT Strength)
{
	/* Clear condition */
	Clear_condition(Party_char_handles[Member_nr - 1],	STONED);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_18_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 15:16
 * LAST      : 05.10.95 15:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_18_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_18);
}

void
Do_C0_Spell_18(UNSHORT Member_nr, UNSHORT Strength)
{
	/* Clear condition */
	Clear_condition(Party_char_handles[Member_nr - 1],	BLIND);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_19_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 15:16
 * LAST      : 05.10.95 15:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_19_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_19);
}

void
Do_C0_Spell_19(UNSHORT Member_nr, UNSHORT Strength)
{
	/* Clear condition */
	Clear_condition(Party_char_handles[Member_nr - 1],	POISONED);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_21_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 12:44
 * LAST      : 31.08.95 12:44
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_21_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Set light temporary spell */
	Set_light_spell(Strength, Strength);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_1_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 12:48
 * LAST      : 05.10.95 15:17
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_1_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C1_Spell_1);
}

void
Do_C1_Spell_1(UNSHORT Member_nr, UNSHORT Strength)
{
	SISHORT Max_LP;
	SISHORT Added_LP;

	/* Clear all conditions */
	Clear_condition(Party_char_handles[Member_nr - 1], POISONED);
	Clear_condition(Party_char_handles[Member_nr - 1], DISEASED);
	Clear_condition(Party_char_handles[Member_nr - 1], LAMED);
	Clear_condition(Party_char_handles[Member_nr - 1], STONED);
	Clear_condition(Party_char_handles[Member_nr - 1], BLIND);
	Clear_condition(Party_char_handles[Member_nr - 1], PANICKED);
	Clear_condition(Party_char_handles[Member_nr - 1], ASLEEP);
	Clear_condition(Party_char_handles[Member_nr - 1], INSANE);
	Clear_condition(Party_char_handles[Member_nr - 1], IRRITATED);

	/* Get max LP */
	Max_LP = Get_max_LP(Party_char_handles[Member_nr - 1]);

	/* Calculate added LP */
	Added_LP = max(1, (Max_LP * Strength) / 100);

	/* Heal */
	Do_healing(Member_nr, Added_LP);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_2_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 13:00
 * LAST      : 31.08.95 13:00
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_2_handler(UNSHORT Strength)
{
	/* 3D map ? */
	if (_3D_map)
	{
		/* Yes -> Play sound effect */
		SOUND_Play_sound_effect
		(
			38,
			80,
			100,
			50,
			0
		);

		/* Enter automap with extended functions */
		Enter_Automap(SHOW_NPCS_FUNCTION | SHOW_MONSTERS_FUNCTION |
		 SHOW_TRAPS_FUNCTION | SHOW_HIDDEN_FUNCTION);
	}
	else
	{
		/* No -> Insult player */
		Do_text_window(System_text_ptrs[600]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_3_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 15:20
 * LAST      : 05.10.95 15:20
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_3_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C1_Spell_1);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_5_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.10.95 14:22
 * LAST      : 08.10.95 14:22
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_5_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C1_Spell_5);
}

void
Do_C1_Spell_5(UNSHORT Member_nr, UNSHORT Strength)
{
	SISHORT Max_LP;
	SISHORT Added_LP;

	/* Get max LP */
	Max_LP = Get_max_LP(Party_char_handles[Member_nr - 1]);

	/* Calculate added LP */
	Added_LP = max(1, (Max_LP * 60 * Strength) / 10000);

	/* Heal */
	Do_healing(Member_nr, Added_LP);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_11_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 15:25
 * LAST      : 20.10.95 19:19
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_11_handler(UNSHORT Strength)
{
	/* Is the party tired ? */
	if (PARTY_DATA.Camp_counter > TIRED_DURATION)
	{
		/* Yes -> Play sound effect */
		SOUND_Play_sound_effect
		(
			38,
			80,
			100,
			50,
			0
		);

		/* Do spell for each target */
		Do_all_magic_party_targets(Strength, Do_C1_Spell_11);

		/* Clear counter */
		PARTY_DATA.Camp_counter = 0;
	}
	else
	{
		/* No -> Insult the player */
		Do_text_window(System_text_ptrs[413]);
	}
}

void
Do_C1_Spell_11(UNSHORT Member_nr, UNSHORT Strength)
{
	/* De-exhaust */
	De_exhaust_party_member(Member_nr);

	/* Recuperate */
	Recuperate(Member_nr, 100);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 05.10.95 15:30
 * LAST      : 05.10.95 15:30
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_7_handler(UNSHORT Strength)
{
	/* Play sound effect */
	SOUND_Play_sound_effect
	(
		38,
		80,
		100,
		50,
		0
	);

	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C2_Spell_7);
}

void
Do_C2_Spell_7(UNSHORT Member_nr, UNSHORT Strength)
{
	SISHORT Max_LP;
	SISHORT Added_LP;

	/* Get max LP */
	Max_LP = Get_max_LP(Party_char_handles[Member_nr - 1]);

	/* Calculate added LP */
	Added_LP = max(1, (Max_LP * 40 * Strength) / 10000);

	/* Heal */
	Do_healing(Member_nr, Added_LP);
}

