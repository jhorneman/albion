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
#include <BBMEM.H>

#include <GAME.H>
#include <GAMEVAR.H>
#include <CONTROL.H>
#include <MAGIC.H>
#include <COMBVAR.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMMAGIC.H>
#include <COMMSUB.H>
#include <ITMLOGIC.H>
#include <TACTICAL.H>
#include <XFTYPES.H>

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_4_handler
 *						 Blink
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
	struct COMOB_behaviour *behave;
	struct COMOB *COMOB;
	struct Combat_participant *Victim_part;
	SILONG time;
	UNSHORT Destination_X, Destination_Y;
	SILONG left,right,top,bottom;
	SILONG x,y,z;
	SILONG stars,nextstar;

	/* Get victim participant data */
	Victim_part = Get_magic_combat_part_target();
	if (!Victim_part)
		return;

	/* Gr”áe Monster */
	Get_3D_part_coordinates(Victim_part,&x,&y,&z);
	Get_part_rectangle(Victim_part,&left,&top,&right,&bottom);

	/* Victim is monster ? */
	if (Victim_part->Type == MONSTER_PART_TYPE)
	{
		/* Yes -> Start of effect */

		/* Bezauberungseffekt auf Monster */
		/* Falle entfernen */
		time=20;
		nextstar=0;
		do{
			Update_screen();

			/* n„chster Stern blitzt auf */
			if(nextstar<0){
				nextstar=stars;

				/* Zuf„llige Position im Monster */
				x=rndmm(left,right);
				y=rndmm(bottom,top);

				/* Ist Stern auf Monster */
				if(Test_COMOB_mask(Victim_part->Main_COMOB,x,y)){

					/* Stern erstellen */
					if((COMOB=Gen_COMOB(x,y,z-5, 0, 10, Star_gfx_handles[0], GC_NORM))==NULL){
						ERROR_ClearStack();
						return;
					}

					/* SizeII verhalten */
					if(!(behave = Add_COMOB_behaviour(COMOB,NULL, size_II_handler))){
						ERROR_ClearStack();
						return;
					}

					/* Set Connect behaviour data */
					behave->Data.Just_data_w.Data[0] = 500;
					behave->Data.Just_data_w.Data[1] = 500;
					behave->Data.Just_data_w.Data[2] = 100;
					behave->Data.Just_data_w.Data[3] = 100;
					behave->Data.Just_data_w.Data[4] = 1500;
					behave->Data.Just_data_w.Data[5] = 1500;
					behave->Data.Just_data_w.Data[6] = 2;
					behave->Data.Just_data_w.Data[7] = 2;

				}

			}

			nextstar-=Nr_combat_updates;
			time-=Nr_combat_updates;
		}while(time>0);

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

					/* Monster wird langsam heller und in der Gr”áe ver„ndert */




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

