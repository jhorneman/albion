/************
 * NAME     : COMMAGIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 24-4-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMMAGIC.H
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

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
#include <STATAREA.H>
#include <ITEMLIST.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

/* global variables */

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_1_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 29.05.95 15:14
 * LAST      : 02.06.95 16:07
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_1);
}

void
Do_C0_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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

		/* Lame victim */
		Set_condition(Victim_part->Char_handle, LAMED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_3_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 29.05.95 15:22
 * LAST      : 02.06.95 16:08
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_3);
}

void
Do_C0_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
		Add_temporary_effect(Victim_part, POISON_TEMP_EFFECT, Strength, 10);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_4_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:48
 * LAST      : 03.06.95 13:48
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_4_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_4);
}

void
Do_C0_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
		Add_temporary_effect(Victim_part, HURRY_TEMP_EFFECT, Strength, 10);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_5_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:50
 * LAST      : 03.06.95 13:50
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_5_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_5);
}

void
Do_C0_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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

		/* Show LP of victim */
		Victim_part->Flags |= PART_SHOW_LP;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_6_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:52
 * LAST      : 03.06.95 13:52
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_6_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_6);
}

void
Do_C0_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
		Add_temporary_effect(Victim_part, FROZEN_TEMP_EFFECT, Strength, 10);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_7_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_6);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_8_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_6);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_10_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:53
 * LAST      : 03.06.95 13:53
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

void
Do_C0_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
		Add_temporary_effect(Victim_part, BLINDED_TEMP_EFFECT, Strength, 10);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_11_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:54
 * LAST      : 03.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_11_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_12_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:54
 * LAST      : 03.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_12_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_10);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_13_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:54
 * LAST      : 03.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_13_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_13);
}

void
Do_C0_Spell_13(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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

		/* Let victim sleep */
		Set_condition(Victim_part->Char_handle, ASLEEP);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_14_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 15:17
 * LAST      : 02.06.95 15:17
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_14_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_14);
}

void
Do_C0_Spell_14(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Is there already a trap on this square ? */
	if (!(Combat_matrix[Tactical_Y][Tactical_X].Trap_handler))
	{
		/* No -> Do effect */

		/* Set trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_handler =
		 Handle_C0_Spell_14_trap;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_strength = Strength;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_C0_Spell_14_trap
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 15:17
 * LAST      : 02.06.95 15:17
 * INPUTS    : struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_C0_Spell_14_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength)
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
 * NAME      : C0_Spell_15_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:46
 * LAST      : 03.06.95 13:46
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_15_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_15);
}

void
Do_C0_Spell_15(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Start of effect */

	/* Is there a trap on this square ? */
	if (!(Combat_matrix[Tactical_Y][Tactical_X].Trap_handler))
	{
		/* Yes -> Rest of effect */

		/* Remove trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_handler = NULL;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_strength = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_20_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 03.06.95 13:55
 * LAST      : 03.06.95 13:55
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_20_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C0_Spell_20);
}

void
Do_C0_Spell_20(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	SISHORT LP;
	UNSHORT Damage;

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
		/* No -> Calculate damage */
		Damage = max(((Strength * 50) / 100), 1);

		/* Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Get LP of victim */
			LP = Get_LP(Victim_part->Char_handle);

			/* Can the victim take this damage ? */
			if (LP > Damage)
			{
				/* Yes -> Do damage */
				Do_combat_damage(Victim_part, Damage);

				/* Do survival effect */

			}
			else
			{
				/* No -> Destroy monster */
				Destroy_participant(Victim_part);

				/* Do death effect */
			}
		}
		else
		{
			/* No -> Do damage to victim */
			Do_combat_damage(Victim_part, Damage);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_4_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 11:15
 * LAST      : 06.06.95 11:15
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_4_handler(UNSHORT Strength)
{
	struct Combat_participant *Victim_part;
	UNSHORT Destination_X, Destination_Y;

	/* Get victim participant data */
	Victim_part = Get_magic_combat_part_target();
	if (!Victim_part)
		return;

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */

		/* Handle magical defense */
		Strength = Handle_magical_defense(Victim_part, Strength);
	}

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Can the victim be blinked ? */
		if (Get_conditions(Victim_part->Char_handle) & BLINK_MASK)
		{
			/* No -> Tell the player */

		}
		else
		{
			/* Yes -> Get the destination coordinates */
			Destination_X = Current_use_magic_data.Extra_target_data %
			 NR_TACTICAL_COLUMNS;
			Destination_Y = Current_use_magic_data.Extra_target_data /
			 NR_TACTICAL_COLUMNS;

			/* Is the destination occupied ? */
			if (Combat_matrix[Destination_Y][Destination_X].Part)
			{
				/* Yes -> Tell the player */
			}
			else
			{
				/* No -> Victim is monster ? */
				if (Victim_part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Rest of effect */
				}

				/* Copy to destination */
				Combat_matrix[Destination_Y][Destination_X].Part = Victim_part;

				/* Clear source */
				Combat_matrix[Victim_part->Tactical_Y][Victim_part->Tactical_X].Part
				 = NULL;

				/* Set new tactical coordinates */
				Victim_part->Tactical_X = Destination_X;
				Victim_part->Tactical_Y = Destination_Y;

				/* Reset action */
				Victim_part->Current_action = NO_COMACT;

				/* Victim is monster ? */
				if (Victim_part->Type == MONSTER_PART_TYPE)
				{
					/* Yes -> Calculate new 3D coordinates for main COMOB */
					Convert_tactical_to_3D_coordinates(Destination_X,
					 Destination_Y, &(Victim_part->Main_COMOB->X_3D),
					 &(Victim_part->Main_COMOB->Z_3D));
				}

				/* Re-draw combat screen */
				Update_screen();
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_6_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_6_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_8_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:49
 * LAST      : 07.06.95 13:49
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C1_Spell_8);
}

void
Do_C1_Spell_8(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Effect */

	/* Show monster data */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_9_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     :
 * LAST      :
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_9_handler(UNSHORT Strength)
{
	/* Select random monsters ! No endbosses ! */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_10_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 13:50
 * LAST      : 07.06.95 13:50
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C1_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C1_Spell_10);
}

void
Do_C1_Spell_10(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Start of effect */

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Rest of effect */

		/* Irritate victim */
		Set_condition(Victim_part->Char_handle, IRRITATED);
	}
}

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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_1_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:34
 * LAST      : 14.06.95 10:26
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_1);
}

void
Do_C3_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	MEM_HANDLE Firering_handle = NULL;
	MEM_HANDLE Fireball_handle = NULL;
	struct COMOB *Fireball_COMOB;
	struct COMOB *Firering_COMOB;
	struct COMOB *COMOB;
	union COMOB_behaviour_data *Behaviour_data;
	SILONG X_3D, Y_3D, Z_3D;
	SILONG dX_3D, dY_3D, dZ_3D;
	UNSHORT Nr_steps;
	UNSHORT i, j;

	/* Load graphics */
	Firering_handle = Load_subfile(COMBAT_GFX, 20);
	if (!Firering_handle)
		return;

	Fireball_handle = Load_subfile(COMBAT_GFX, 21);
	if (!Fireball_handle)
	{
		MEM_Free_memory(Firering_handle);
		return;
	}

	/* Make COMOBs */
	Fireball_COMOB = Add_COMOB(100);
	Firering_COMOB = Add_COMOB(100);

	/* Success ? */
	if (Fireball_COMOB && Firering_COMOB)
	{
		/* Yes -> Prepare fireball movement */
		Nr_steps = Prepare_COMOB_movement(Current_use_magic_data.Casting_participant,
		 Fireball_COMOB, Victim_part->Tactical_X, Victim_part->Tactical_Y, 15);

		/* Save movement vector */
		dX_3D = Fireball_COMOB->dX_3D;
		dY_3D = Fireball_COMOB->dY_3D;
		dZ_3D = Fireball_COMOB->dZ_3D;

		/* Clear movement vector */
		Fireball_COMOB->dX_3D = 0;
		Fireball_COMOB->dY_3D = 0;
		Fireball_COMOB->dZ_3D = 0;

		/* Set fireball display parameters */
		Fireball_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		Fireball_COMOB->Special_handle = Transluminance_table_handle;

		Fireball_COMOB->Hotspot_X_offset = 50;
		Fireball_COMOB->Hotspot_Y_offset = 50;

		Fireball_COMOB->Display_width = 100 - (15 * 4);
		Fireball_COMOB->Display_height = 100 - (15 * 4);

		Fireball_COMOB->Graphics_handle = Fireball_handle;

		Fireball_COMOB->Nr_frames =
		 Get_nr_frames(Fireball_COMOB->Graphics_handle);

		/* Copy fireball coordinates to firering COMOB */
		Firering_COMOB->X_3D = Fireball_COMOB->X_3D;
		Firering_COMOB->Y_3D = Fireball_COMOB->Y_3D;
		Firering_COMOB->Z_3D = Fireball_COMOB->Z_3D;

		/* Set firering display parameters */
		Firering_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		Firering_COMOB->Special_handle = Transluminance_table_handle;

		Firering_COMOB->Hotspot_X_offset = 50;
		Firering_COMOB->Hotspot_Y_offset = 50;

		Firering_COMOB->Display_width = 100 + (15 * 20);
		Firering_COMOB->Display_height = 100 + (15 * 20);

		Firering_COMOB->Graphics_handle = Firering_handle;

		Firering_COMOB->Nr_frames =
		 Get_nr_frames(Firering_COMOB->Graphics_handle);

		/* Build up the fireball and the firering */
		for (i=0;i<15;i++)
		{
			/* Draw combat screen */
			Update_screen();

			/* Make fireball bigger */
			Fireball_COMOB->Display_width += 4;
			Fireball_COMOB->Display_height += 4;

			/* Make firering smaller */
			Firering_COMOB->Display_width -= 20;
			Firering_COMOB->Display_height -= 20;
		}

		/* Hide firering */
		Hide_COMOB(Firering_COMOB);

		/* Set fireball's movement vector */
		Fireball_COMOB->dX_3D = dX_3D;
		Fireball_COMOB->dY_3D = dY_3D;
		Fireball_COMOB->dZ_3D = dZ_3D;

		/* Let fireball move towards target */
		for (i=0;i<Nr_steps;i++)
		{
			Update_screen();
		}
	}
	else
	{
		/* No -> Clear error stack (the player need not know) */
		ERROR_ClearStack();

		/* Delete fireball if necessary */
		if (Fireball_COMOB)
			Delete_COMOB(Fireball_COMOB);

		/* Delete firering if necessary */
		if (Firering_COMOB)
			Delete_COMOB(Firering_COMOB);
	}

	/* Handle magical defense */
	Strength = Handle_magical_defense(Victim_part, Strength);

	/* Spell deflected ? */
	if (Strength)
	{
		/* No -> Continue with effect ? */
		if (Fireball_COMOB && Firering_COMOB)
		{
			/* Yes -> Rest of effect */
			/* Show firering */
			Show_COMOB(Firering_COMOB);

			/* Get fireball coordinates */
			X_3D = Fireball_COMOB->X_3D;
			Y_3D = Fireball_COMOB->Y_3D;
			Z_3D = Fireball_COMOB->Z_3D;

			/* Delete fireball and firering */
			Delete_COMOB(Fireball_COMOB);
			Delete_COMOB(Firering_COMOB);

			/* Generate sparks */
			for (i=0;i<40;i++)
			{
				/* Add spark COMOB */
				COMOB = Add_COMOB(25 + (rand() % 50));

				/* Success ? */
				if (!COMOB)
				{
					/* No -> Clear error stack */
					ERROR_ClearStack();

					/* Stop generating more sparks */
					break;
				}

				/* Calculate random movement vector */
				COMOB->dX_3D = (rand() % (2 * COMOB_DEC_FACTOR)) -
				 COMOB_DEC_FACTOR;
				COMOB->dY_3D = (rand() % (3 * COMOB_DEC_FACTOR)) -
				 COMOB_DEC_FACTOR;
				COMOB->dZ_3D = (rand() % (2 * COMOB_DEC_FACTOR)) -
				 COMOB_DEC_FACTOR;

				/* Copy coordinates from firering and add vector */
				COMOB->X_3D = X_3D + COMOB->dX_3D;
				COMOB->Y_3D = Y_3D + COMOB->dY_3D;
				COMOB->Z_3D = Z_3D + COMOB->dZ_3D;

				/* Set random lifespan */
				COMOB->Lifespan = 70 + (rand() % 20);

				/* Set display parameters */
				COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
				COMOB->Special_handle = Transluminance_table_handle;

				COMOB->Hotspot_X_offset = 50;
				COMOB->Hotspot_Y_offset = 50;

				/* Set random size */
				COMOB->Display_width = 80 + (rand() % 20);
				COMOB->Display_height = COMOB->Display_width;

				/* Select random spark type */
				COMOB->Graphics_handle = Spark_gfx_handles[6 + rand() % 2];

				/* Set number of animation frames */
				COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

				/* Select random animation frame */
				COMOB->Frame = rand() % COMOB->Nr_frames;

				/* Add bounce behaviour */
				Behaviour_data = Add_COMOB_behaviour(COMOB, NULL, Bounce_handler);

				/* Set random bounce behaviour data */
				Behaviour_data->Bounce_data.Gravity = 10 + rand() % 5;
				Behaviour_data->Bounce_data.Bounce = 60;
				Behaviour_data->Bounce_data.Air_friction = 0;

				/* Add trail */
//				Add_trail_to_COMOB(COMOB, 5);
			}
		}

 		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));
	}

	/* Free memory */
	if (Firering_handle)
		MEM_Free_memory(Firering_handle);
	if (Fireball_handle)
		MEM_Free_memory(Fireball_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_2_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:34
 * LAST      : 06.06.95 13:34
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_2);
}

void
Do_C3_Spell_2(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C3_Spell_3_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_3);
}

void
Do_C3_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C3_Spell_4_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_4_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_4);
}

void
Do_C3_Spell_4(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C3_Spell_5_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_5_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_5);
}

void
Do_C3_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C3_Spell_6_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:35
 * LAST      : 06.06.95 13:35
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_6_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_6);
}

void
Do_C3_Spell_6(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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
 * NAME      : C3_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:14
 * LAST      : 02.06.95 16:14
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_7_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_7);
}

void
Do_C3_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Is there already a trap on this square ? */
	if (!(Combat_matrix[Tactical_Y][Tactical_X].Trap_handler))
	{
		/* No -> Start effect */

		/* Set trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_handler =
		 Handle_C3_Spell_7_trap;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_strength = Strength;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_C3_Spell_7_trap
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:14
 * LAST      : 02.06.95 16:14
 * INPUTS    : struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_C3_Spell_7_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength)
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
 * NAME      : C3_Spell_8_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:15
 * LAST      : 02.06.95 16:15
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_8_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_7);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_9_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:16
 * LAST      : 02.06.95 16:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_9_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_9);
}

void
Do_C3_Spell_9(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Is there already a trap on this square ? */
	if (!(Combat_matrix[Tactical_Y][Tactical_X].Trap_handler))
	{
		/* No -> Start effect */

		/* Set trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_handler =
		 Handle_C3_Spell_9_trap;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_strength = Strength;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_C3_Spell_9_trap
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:16
 * LAST      : 03.06.95 17:27
 * INPUTS    : struct Combat_participant *Victim_part - Pointer to victim
 *              participant data.
 *             UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_C3_Spell_9_trap(struct Combat_participant *Victim_part,
 UNSHORT Strength)
{
	UNSHORT Tactical_X, Tactical_Y;

	/* Get victim's tactical coordinates */
	Tactical_X = Victim_part->Tactical_X;
	Tactical_Y = Victim_part->Tactical_Y;

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

	/* Remove trap */
	Combat_matrix[Tactical_Y][Tactical_X].Trap_handler = NULL;
	Combat_matrix[Tactical_Y][Tactical_X].Trap_strength = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_10_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 02.06.95 16:16
 * LAST      : 02.06.95 16:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_10_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_9);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_11_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:47
 * LAST      : 06.06.95 13:47
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_11_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_11);
}

void
Do_C3_Spell_11(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	UNSHORT Damage;
	UNSHORT LP;

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
		/* No -> Calculate damage */
		Damage = max(((Strength * 50) / 100), 1);

		/* Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

 		/* Do damage to victim */
		Do_combat_damage(Victim_part, max(((Strength * 50) / 100), 1));

		/* Give LP to caster */
		LP = Get_LP(Current_use_magic_data.Casting_participant->Char_handle);
		Set_LP(Current_use_magic_data.Casting_participant->Char_handle,
		 LP + Damage);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_12_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:49
 * LAST      : 06.06.95 13:49
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_12_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_12);
}

void
Do_C3_Spell_12(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	UNSHORT Damage;
	UNSHORT SP;

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

		/* Get victim's SP */
		SP = Get_SP(Victim_part->Char_handle);

		/* Does the victim have this many SP ? */
		Damage = min(Damage, SP);

		/* Victim is monster ? */
		if (Victim_part->Type == MONSTER_PART_TYPE)
		{
			/* Yes -> Rest of effect */
		}

 		/* Remove SP from victim */
		Set_SP(Victim_part->Char_handle, SP - Damage);

		/* Give SP to caster */
		SP = Get_SP(Current_use_magic_data.Casting_participant->Char_handle);
		Set_SP(Current_use_magic_data.Casting_participant->Char_handle,
		 SP + Damage);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_13_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 06.06.95 13:54
 * LAST      : 06.06.95 13:54
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function uses Current_use_magic_data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_13_handler(UNSHORT Strength)
{
	struct Character_data *Char;
	UNSHORT Added;

	/* Increase magical defense */
	Char = (struct Character_data *)
	 MEM_Claim_pointer(Current_use_magic_data.Casting_participant->Char_handle);

	Added = max(((Strength * 50) / 100), 1);
	Char->xProtection_magic = min((Char->xProtection_magic + Added), 100);

	MEM_Free_pointer(Current_use_magic_data.Casting_participant->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_14_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 14:06
 * LAST      : 07.06.95 14:06
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_14_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_14);
}

void
Do_C3_Spell_14(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
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

 		/* Dissolve victim */
		Destroy_participant(Victim_part);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C3_Spell_15_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 15:46
 * LAST      : 07.06.95 15:46
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C3_Spell_15_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C3_Spell_15);
}

void
Do_C3_Spell_15(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	/* Start of effect */

	/* Is there a trap on this square ? */
	if (!(Combat_matrix[Tactical_Y][Tactical_X].Trap_handler))
	{
		/* Yes -> Rest of effect */

		/* Remove trap */
		Combat_matrix[Tactical_Y][Tactical_X].Trap_handler = NULL;
		Combat_matrix[Tactical_Y][Tactical_X].Trap_strength = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_1_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 14:13
 * LAST      : 07.06.95 14:13
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C5_Spell_1_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C5_Spell_1);
}

void
Do_C5_Spell_1(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	UNSHORT Damage;

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
		Damage =
		 Get_damage(Current_use_magic_data.Casting_participant->Char_handle);
		Do_combat_damage(Victim_part, max(((Damage * Strength * 2) / 100), 1));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_3_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 15:48
 * LAST      : 07.06.95 15:48
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C5_Spell_3_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C5_Spell_3);
}

void
Do_C5_Spell_3(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i;

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

		/* Curse the victim's metal items */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Victim_part->Char_handle);

		/* Check body items */
		for (i=0;i<ITEMS_ON_BODY;i++)
		{
			/* Anything in this packet ? */
			if (!Packet_empty(&(Char->Body_items[i])))
			{
				/* Yes -> Get item data */
				Item_data = Get_item_data(&(Char->Body_items[i]));

				/* Is this a metallic item ? */
				if (Item_data->Flags & METALLIC)
				{
					/* Yes -> Curse it */
					Curse_item(Victim_part->Char_handle, i + 1);
				}
				Free_item_data();
			}
		}

		/* Check backpack items */
		for (i=0;i<ITEMS_PER_CHAR;i++)
		{
			/* Anything in this packet ? */
			if (!Packet_empty(&(Char->Backpack_items[i])))
			{
				/* Yes -> Get item data */
				Item_data = Get_item_data(&(Char->Backpack_items[i]));

				/* Is this a metallic item ? */
				if (Item_data->Flags & METALLIC)
				{
					/* Yes -> Curse it */
					Curse_item(Victim_part->Char_handle, i + ITEMS_ON_BODY + 1);
				}
				Free_item_data();
			}
		}
		MEM_Free_pointer(Victim_part->Char_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_5_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 15:56
 * LAST      : 07.06.95 15:56
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C5_Spell_5_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C5_Spell_5);
}

void
Do_C5_Spell_5(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT i;

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

		/* Let the victim's metal items rust */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Victim_part->Char_handle);

		/* Check body items */
		for (i=0;i<ITEMS_ON_BODY;i++)
		{
			/* Anything in this packet ? */
			if (!Packet_empty(&(Char->Body_items[i])))
			{
				/* Yes -> Get item data */
				Item_data = Get_item_data(&(Char->Body_items[i]));

				/* Is this a metallic item ? */
				if (Item_data->Flags & METALLIC)
				{
					/* Yes -> Break it */
					Char->Body_items[i].Flags |= BROKEN_ITEM;

					/* Move to apres pool */
					Put_item_in_apres_pool(&(Char->Body_items[i]));
					Remove_item(Victim_part->Char_handle, i + 1, 1);
				}

				Free_item_data();
			}
		}

		/* Check backpack items */
		for (i=0;i<ITEMS_PER_CHAR;i++)
		{
			/* Anything in this packet ? */
			if (!Packet_empty(&(Char->Backpack_items[i])))
			{
				/* Yes -> Get item data */
				Item_data = Get_item_data(&(Char->Backpack_items[i]));

				/* Is this a metallic item ? */
				if (Item_data->Flags & METALLIC)
				{
					/* Yes -> Break it */
					Char->Backpack_items[i].Flags |= BROKEN_ITEM;
				}
				Free_item_data();
			}
		}
		MEM_Free_pointer(Victim_part->Char_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : COMMAGIC.C
 * AUTHOR    :
 * FIRST     : 07.06.95 16:16
 * LAST      : 07.06.95 16:16
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C5_Spell_7_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_combat_targets(Strength, Do_C5_Spell_7);
}

void
Do_C5_Spell_7(struct Combat_participant *Victim_part, UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Strength)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	UNSHORT Damage;
	UNSHORT i;

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
		/* No -> Check the victim's metal body items */
		Char = (struct Character_data *)
		 MEM_Claim_pointer(Victim_part->Char_handle);

		/* Check body items */
		Damage = 0;
		for (i=0;i<ITEMS_ON_BODY;i++)
		{
			/* Anything in this packet ? */
			if (!Packet_empty(&(Char->Body_items[i])))
			{
				/* Yes -> Get item data */
				Item_data = Get_item_data(&(Char->Body_items[i]));

				/* Is this a metallic item ? */
				if (Item_data->Flags & METALLIC)
				{
					/* Yes -> Increase damage */
					Damage += max(((Strength * 50) / 100), 1);
				}

				Free_item_data();
			}
		}
		MEM_Free_pointer(Victim_part->Char_handle);

		/* Any damage ? */
		if (Damage)
		{
			/* Yes -> Victim is monster ? */
			if (Victim_part->Type == MONSTER_PART_TYPE)
			{
				/* Yes -> Rest of effect */
			}

			/* Do damage to victim */
			Do_combat_damage(Victim_part, Damage);
		}
	}
}

