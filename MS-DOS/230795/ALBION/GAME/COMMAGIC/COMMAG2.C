/************
 * NAME     : COMMAG2.C
 * AUTOR    : Rainer Reber, BlueByte
 * START    : 24-4-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMMAGIC.H
 ************/

/* includes */

#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>
#include <BBERROR.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <GAMETEXT.H>
#include <MAGIC.H>
#include <MAP.H>
#include <EVELOGIC.H>
#include <COMBAT.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <STATAREA.H>
#include <ITEMLIST.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_1_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:52
 * LAST      : 07.06.95 13:52
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_1);
}

void
Do_C2_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

		/* Add temporary effect */
		Add_temporary_effect(Victim_part, BERSERKER_TEMP_EFFECT, Strength, 10);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_2_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:54
 * LAST      : 07.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_2);
}

void
Do_C2_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct Character_data *Char;
	BOOLEAN Is_ghost = FALSE;

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
 		/* Yes -> Start of effect */
	}

	/* Is the victim a ghost ? */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Victim_part->Char_handle);

	if (Char->Flags & GHOST)
		Is_ghost = TRUE;

	MEM_Free_pointer(Victim_part->Char_handle);

	if (Is_ghost)
	{
		/* Yes -> Handle magical defense */
		Strength = Handle_magical_defense(Victim_part, Strength);

		/* Spell deflected ? */
		if (Strength)
		{
			/* No -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */
			}

			/* Destroy victim */
			Destroy_participant(Victim_part);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_3_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:55
 * LAST      : 07.06.95 13:55
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_4_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:55
 * LAST      : 07.06.95 13:55
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_4_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_2);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_5_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:37
 * LAST      : 06.06.95 13:37
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_5_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_5);
}

void
Do_C2_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

 		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_6_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:59
 * LAST      : 07.06.95 13:59
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_6_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_6);
}

void
Do_C2_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct Character_data *Char;
	UNSHORT Added;

	/* Increase magical defense */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Victim_part->Char_handle);

	Added = max(((Strength * 50) / 100), 1);
	Char->xProtection_magic = min((Char->xProtection_magic + Added), 100);

	MEM_Free_pointer(Victim_part->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_8_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:38
 * LAST      : 06.06.95 13:38
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_8);
}

void
Do_C2_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

		/* Scare victim */
		Set_condition(Victim_part->Char_handle, PANICKED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_9_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:38
 * LAST      : 06.06.95 13:38
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_9_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_8);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_10_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:38
 * LAST      : 06.06.95 13:38
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_8);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_11_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 14:03
 * LAST      : 07.06.95 14:03
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C2_Spell_11_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C2_Spell_11);
}

void
Do_C2_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	UNSHORT Damage;
	UNSHORT Attribute;

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */
	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Calculate "damage" */
		Damage = max(((Strength * 50) / 100), 1);

		/* Get victim's strength attribute */
		Attribute = Get_attribute(Victim_part->Char_handle, STRENGTH);

		/* Is the victim's strength attribute high enough ? */
		Damage = min(Damage, Attribute);

		/* Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

 		/* Remove strength from victim */
		Set_attribute(Victim_part->Char_handle, STRENGTH, Attribute - Damage);

		/* Give strength to caster */
		Attribute =
		 Get_attribute(Current_use_magic_data.Casting_participant->Char_handle,
		 STRENGTH);
		Set_attribute(Current_use_magic_data.Casting_participant->Char_handle,
		 STRENGTH, Attribute + Damage);
	}
}

