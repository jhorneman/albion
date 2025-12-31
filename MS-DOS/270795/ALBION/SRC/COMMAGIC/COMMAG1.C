/************
 * NAME     : COMMAG1.C
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

