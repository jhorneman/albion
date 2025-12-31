/************
 * NAME     : COMACTS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 8-3-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMACTS.H
 ************/

/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <COMBAT.H>
#include <COMACTS.H>
#include <COMOBS.H>
#include <COMSHOW.H>
#include <TACTICAL.H>
#include <EVELOGIC.H>
#include <MAGIC.H>
#include <MUSIC.H>
#include <APRES.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <INVITEMS.H>
#include <GAMETEXT.H>

/* global variables */

BOOLEAN Abort_attack;

/* Combat action handler table */
struct Combat_action Combat_action_table[MAX_COMACTS] = {
	{ 0, NULL },													/* NO_COMACT */
	{ MOVE_MASK, Move_combat_action },   					/* MOVE_COMACT */
	{ ATTACK_MASK, Close_range_combat_action },   		/* CLOSE_RANGE_COMACT */
	{ ATTACK_MASK, Long_range_combat_action },   		/* LONG_RANGE_COMACT */
	{ FLEE_MASK, Flee_combat_action },   					/* FLEE_COMACT */
	{ MAGIC_MASK, Cast_spell_combat_action },   			/* CAST_SPELL_COMACT */
	{ CONTROL_MASK, Use_magic_object_combat_action },	/* USE_MAGIC_ITEM_COMACT */
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_combat_action
 * FUNCTION  : Move combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_combat_action(struct Combat_participant *Part)
{
	UNSHORT X, Y;

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Tell the player */
		Set_permanent_message_nr(444);

		/* Get destination */
		X = Part->Target.Move_target_data.Target_square_index %
		 NR_TACTICAL_COLUMNS;
		Y = Part->Target.Move_target_data.Target_square_index /
		 NR_TACTICAL_COLUMNS;

		/* Is empty ? */
		if (Combat_matrix[Y][X].Part)
		{
			/* No -> Tell the player */
			Set_permanent_message_nr(443);
		}
		else
		{
			/* Yes -> Copy to destination */
			Combat_matrix[Y][X].Part = Part;

			/* Clear source */
			Combat_matrix[Part->Tactical_Y][Part->Tactical_X].Part = NULL;

			/* Set new tactical coordinates */
			Part->Tactical_X = X;
			Part->Tactical_Y = Y;

			/* Reset action */
			Part->Current_action = NO_COMACT;
		}
	}
	else
	{
		/* Monster -> */
	}
}

/*
; ---------- Move monster -------------------------
.Monster:	cmp.b	#Stand_anim,Part_anim(a0)	; Already animating ?
	beq.s	.No
	jsr	Wait_4_animation		; Yes -> Wait
.No:	moveq.l	#0,d0			; Get destination Y
	move.w	Part_target(a0),d0
	add.w	d0,d0
	move.w	Part_target(a0,d0.w),d0
	divu.w	#6,d0
	cmp.w	Part_Y(a0),d0		; Move or retreat ?
	bmi.s	.Retreat
	move.w	#228,d0			; " moves!"
	jsr	Do_part_prompt
	bra.s	.Go_on
.Retreat:	move.w	#232,d0			; " retreats!"
	jsr	Do_part_prompt
.Go_on:

.Exit:	clr.b	Part_action(a0)		; Reset action
	move.w	#-1,Part_target(a0)
	rts
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Close_range_combat_action
 * FUNCTION  : Close-range attack combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 10:32
 * LAST      : 12.05.95 10:32
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Close_range_combat_action(struct Combat_participant *Part)
{
	static struct Event_action Close_range_attack_action =
	{
		0,	0, 0,
		PART_ATTACKS_CLOSE_ACTION, 0, 0,
		Do_close_range_attack, NULL, NULL
	};

	struct Character_data *Char;
	UNSHORT Nr_attacks;
	UNSHORT i;

	/* Initialize event action data */
	if (Part->Type == PARTY_PART_TYPE)
		Close_range_attack_action.Actor_type = PARTY_ACTOR_TYPE;
	else
		Close_range_attack_action.Actor_type = MONSTER_ACTOR_TYPE;
	Close_range_attack_action.Actor_index = Part->Number;

	/* Get number of attacks per round */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);
	Nr_attacks = (UNSHORT) Char->Attacks_per_round;
	MEM_Free_pointer(Part->Char_handle);

	/* Is the attacker hurried ? */
	if (Part->Flags & PART_HURRIED)
	{
		/* Yes -> Double the number of attacks */
		Nr_attacks *= 2;
	}

	/* For each attack */
	for (i=0;i<Nr_attacks;i++)
	{
		/* Perform action : Close range attack */
		Perform_action(&Close_range_attack_action);

		/* Continue attack ? */
		if (Abort_attack)
		{
			/* No -> Reset action */
			Part->Current_action = NO_COMACT;

			/* Abort attack */
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_close_range_attack
 * FUNCTION  : Do close-range attack action.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 10:31
 * LAST      : 12.05.95 10:31
 * INPUTS    : struct Event_action *Action - Pointer to event action data.
 * RESULT    : BOOLEAN : TRUE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Do_close_range_attack(struct Event_action *Action)
{
	union Combat_show_parms P;
	struct Combat_participant *Attacker_part;
	struct Combat_participant *Defender_part;
	struct Character_data *Attacker_char;
	struct Character_data *Defender_char;
	UNSHORT Afflicted_damage;
	UNSHORT Current_weapon_slot_index;
	UNSHORT Current_weapon_item_index;
	UNSHORT Target_X, Target_Y;

	/* Get attacker data */
	if (Action->Actor_type == PARTY_ACTOR_TYPE)
		Attacker_part = &(Party_parts[Action->Actor_index - 1]);
	else
		Attacker_part = &(Monster_parts[Action->Actor_index - 1]);

	/* Find defender */
	Target_X = Attacker_part->Target.Attack_target_data.Target_square_index % 6;
	Target_Y = Attacker_part->Target.Attack_target_data.Target_square_index / 6;
	Defender_part = Combat_matrix[Target_Y][Target_X].Part;

	/* Clear abort attack flag */
	Abort_attack = FALSE;

	/* Get attacker's character data */
	Attacker_char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Select (next) weapon */
	Current_weapon_slot_index = RIGHT_HAND;

	/* Get weapon item index */
	Current_weapon_item_index =
	 Attacker_char->Body_items[Current_weapon_slot_index - 1].Index;

	/* Store */
	Attacker_part->Weapon_item_index = Current_weapon_item_index;

	/* Show close-range attack */
	P.Attack_parms.Target_X = Target_X;
	P.Attack_parms.Target_Y = Target_Y;
	P.Attack_parms.Weapon_slot_index = Current_weapon_slot_index;
	Combat_show(Attacker_part, SHOW_CLOSE_RANGE, &P);

	/* Is there a defender ? */
	if (Defender_part)
	{
		/* Yes -> Get defender's character data */
		Defender_char = (struct Character_data *)
		 MEM_Claim_pointer(Defender_part->Char_handle);

		/* Any weapon used ? */
		if (Current_weapon_item_index)
		{
			/* Yes -> Print "<attacker> attacks <defender> with <weapon>." */
			Print_combat_message(Attacker_part, Defender_part, 0, 446);
		}
		else
		{
			/* No -> Print "<attacker> attacks <defender>." */
			Print_combat_message(Attacker_part, Defender_part, 0, 455);
		}

		/* Attacker's attack skill successfully probed ? */
		if (Probe_skill(Attacker_part->Char_handle, LONG_RANGE_ATTACK))
		{
			/* Yes -> Can the defender be damaged ? */
			if (Get_conditions(Defender_part->Char_handle) & DAMAGE_MASK)
			{
				/* No -> Print "<defender> cannot be damaged." */
				Print_combat_message(Attacker_part, Defender_part, 0, 447);

				/* Abort attack */
				Abort_attack = TRUE;
			}
			else
			{
				/* Yes -> Is the defender doing nothing /
				 is the defender's dexterity attribute successfully probed ? */
				if ((Defender_part->Current_action == NO_COMACT) &&
				 (Probe_attribute(Defender_part->Char_handle, DEXTERITY)))
				{
					/* Yes -> Print "Attack was deflected!" */
					Print_combat_message(Attacker_part, Defender_part, 0, 448);

					/* Try to break the attacker's weapon */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1 << LONG_RANGE_IT));

					/* Try to break the defender's weapon */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << RIGHT_HAND), (1 << CLOSE_RANGE_IT));

					/* Try to break the defender's shield */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << LEFT_HAND), (1 << SHIELD_IT));
				}
				else
				{
					/* No -> Try to break the attacker's weapon */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1 << LONG_RANGE_IT));

					/* Try to break the defender's armour */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << BODY), (1 << ARMOUR_IT));

					/* Try to break the defender's helmet */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << HEAD), (1 << HELMET_IT));

					/* Is the defender NOT an END MONSTER /
					 was the attacker's critical hit skill successfully probed ? */
					if (!(Defender_char->Flags & END_MONSTER) &&
					 (Probe_skill(Attacker_part->Char_handle, CRITICAL_HIT)))
					{
						/* Yes -> Print "<attacker> made a critical hit!" */
						Print_combat_message(Attacker_part, Defender_part, 0, 449);

						/* Set afflicted damage to maximum */
						Afflicted_damage = Get_LP(Defender_part->Char_handle);
					}
					else
					{
						/* No -> Calculate afflicted damage */
						Afflicted_damage =
						 Calculate_afflicted_damage(Attacker_part, Defender_part);
					}

					/* Any damage done ? */
					if (Afflicted_damage)
					{
						/* Yes -> Print "<attacker> does <damage> damage! */
						Print_combat_message(Attacker_part, Defender_part,
						 Afflicted_damage, 450);

						/* Process damage for danger evaluation */

						/* Do damage */
						Do_combat_damage(Defender_part, Afflicted_damage);
					}
					else
					{
						/* No -> Print "No damage done!" */
						Print_combat_message(Attacker_part, Defender_part, 0, 451);
					}
				}
			}
		}
		else
		{
			/* No -> Print "<attacker> failed!" */
			Print_combat_message(Attacker_part, Defender_part, 0, 452);

		}

		MEM_Free_pointer(Defender_part->Char_handle);
	}
	else
	{
		/* No -> Print "<attacker> missed!" */
		Print_combat_message(Attacker_part, Defender_part, 0, 453);

		/* Reset action */
		Attacker_part->Current_action = NO_COMACT;

		/* Abort attack */
		Abort_attack = TRUE;
	}

	MEM_Free_pointer(Attacker_part->Char_handle);

	return(TRUE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_afflicted_damage
 * FUNCTION  : Calculate the damage afflicted by an attack.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 22:02
 * LAST      : 11.05.95 22:02
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to the
 *              attacker's combat participant data.
 *             struct Combat_participant *Defender_part - Pointer to the
 *              defender's combat participant data.
 * RESULT    : UNSHORT : Afflicted damage.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Calculate_afflicted_damage(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part)
{
	SISHORT Damage;
	SISHORT Protection;

	/* Calculate maximum damage */
	Damage = Get_damage(Attacker_part->Char_handle) +
	 Get_attribute(Attacker_part->Char_handle, STRENGTH) / 25;

	/* Is party member ? */
	if (Attacker_part->Type == PARTY_PART_TYPE)
	{
		/* Yes -> Add effect of spell */
	}

	/* Randomize damage */
	Damage = Get_rnd_50_100(Damage);

	/* Calculate maximum protection */
	Protection = Get_protection(Defender_part->Char_handle) +
	 Get_attribute(Defender_part->Char_handle, STAMINA) / 25;

	/* Is party member ? */
	if (Defender_part->Type == PARTY_PART_TYPE)
	{
		/* Yes -> Add effect of spell */
	}

	/* Randomize protection */
	Protection = Get_rnd_50_100(Protection);

	/* Return the difference */
	return(max((Damage - Protection), 1));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Long_range_combat_action
 * FUNCTION  : Long-range attack combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.04.95 14:58
 * LAST      : 18.04.95 14:58
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Long_range_combat_action(struct Combat_participant *Part)
{
	static struct Event_action Long_range_attack_action =
	{
		0,	0, 0,
		PART_ATTACKS_LONG_ACTION, 0, 0,
		Do_long_range_attack, NULL, NULL
	};

	struct Character_data *Char;
	UNSHORT Nr_attacks;
	UNSHORT i;

	/* Initialize event action data */
	if (Part->Type == PARTY_PART_TYPE)
		Long_range_attack_action.Actor_type = PARTY_ACTOR_TYPE;
	else
		Long_range_attack_action.Actor_type = MONSTER_ACTOR_TYPE;
	Long_range_attack_action.Actor_index = Part->Number;

	/* Get number of attacks per round */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);
	Nr_attacks = (UNSHORT) Char->Attacks_per_round;
	MEM_Free_pointer(Part->Char_handle);

	/* Is the attacker hurried ? */
	if (Part->Flags & PART_HURRIED)
	{
		/* Yes -> Double the number of attacks */
		Nr_attacks *= 2;
	}

	/* For each attack */
	for (i=0;i<Nr_attacks;i++)
	{
		/* Perform action : Long range attack */
		Perform_action(&Long_range_attack_action);

		/* Continue attack ? */
		if (Abort_attack)
		{
			/* No -> Reset action */
			Part->Current_action = NO_COMACT;

			/* Abort attack */
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_long_range_attack
 * FUNCTION  : Do long-range attack action.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.04.95 14:58
 * LAST      : 18.04.95 14:58
 * INPUTS    : struct Event_action *Action - Pointer to event action data.
 * RESULT    : BOOLEAN : TRUE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Do_long_range_attack(struct Event_action *Action)
{
	union Combat_show_parms P;
	struct Combat_participant *Attacker_part;
	struct Combat_participant *Defender_part;
	struct Character_data *Attacker_char;
	struct Character_data *Defender_char;
	UNSHORT Afflicted_damage;
	UNSHORT Current_weapon_slot_index;
	UNSHORT Current_weapon_item_index;
	UNSHORT Target_X, Target_Y;

	/* Get attacker data */
	if (Action->Actor_type == PARTY_ACTOR_TYPE)
		Attacker_part = &(Party_parts[Action->Actor_index - 1]);
	else
		Attacker_part = &(Monster_parts[Action->Actor_index - 1]);

	/* Find defender */
	Target_X = Attacker_part->Target.Attack_target_data.Target_square_index % 6;
	Target_Y = Attacker_part->Target.Attack_target_data.Target_square_index / 6;
	Defender_part = Combat_matrix[Target_Y][Target_X].Part;

	/* Clear abort attack flag */
	Abort_attack = FALSE;

	/* Get attacker's character data */
	Attacker_char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Select (next) weapon */
	Current_weapon_slot_index = RIGHT_HAND;

	/* Get weapon item index */
	Current_weapon_item_index =
	 Attacker_char->Body_items[Current_weapon_slot_index - 1].Index;

	/* Store */
	Attacker_part->Weapon_item_index = Current_weapon_item_index;

	/* Remove ammunition */
	Remove_ammo_and_reload(Attacker_part, Current_weapon_slot_index);

	/* Show long-range attack / flying projectile */
	P.Attack_parms.Target_X = Target_X;
	P.Attack_parms.Target_Y = Target_Y;
	P.Attack_parms.Weapon_slot_index = Current_weapon_slot_index;
	Combat_show(Attacker_part, SHOW_LONG_RANGE, &P);

	/* Is there a defender ? */
	if (Defender_part)
	{
		/* Yes -> Get defender's character data */
		Defender_char = (struct Character_data *)
		 MEM_Claim_pointer(Defender_part->Char_handle);

		/* Any weapon used ? */
		if (Current_weapon_item_index)
		{
			/* Yes -> Print "<attacker> attacks <defender> with <weapon>." */
			Print_combat_message(Attacker_part, Defender_part, 0, 446);
		}
		else
		{
			/* No -> Print "<attacker> attacks <defender>." */
			Print_combat_message(Attacker_part, Defender_part, 0, 455);
		}

		/* Attacker's attack skill successfully probed ? */
		if (Probe_skill(Attacker_part->Char_handle, LONG_RANGE_ATTACK))
		{
			/* Yes -> Can the defender be damaged ? */
			if (Get_conditions(Defender_part->Char_handle) & DAMAGE_MASK)
			{
				/* No -> Print "<defender> cannot be damaged." */
				Print_combat_message(Attacker_part, Defender_part, 0, 447);

				/* Abort attack */
				Abort_attack = TRUE;
			}
			else
			{
				/* Yes -> Is the defender doing nothing /
				 is the defender's dexterity attribute successfully probed ? */
				if ((Defender_part->Current_action == NO_COMACT) &&
				 (Probe_attribute(Defender_part->Char_handle, DEXTERITY)))
				{
					/* Yes -> Print "Attack was deflected!" */
					Print_combat_message(Attacker_part, Defender_part, 0, 448);

					/* Try to break the attacker's weapon */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1 << LONG_RANGE_IT));

					/* Try to break the defender's weapon */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << RIGHT_HAND), (1 << CLOSE_RANGE_IT));

					/* Try to break the defender's shield */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << LEFT_HAND), (1 << SHIELD_IT));
				}
				else
				{
					/* No -> Try to break the attacker's weapon */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1 << LONG_RANGE_IT));

					/* Try to break the defender's armour */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << BODY), (1 << ARMOUR_IT));

					/* Try to break the defender's helmet */
					Break_combat_items(Attacker_part->Char_handle,
					 (1 << HEAD), (1 << HELMET_IT));

					/* Is the defender NOT an END MONSTER /
					 was the attacker's critical hit skill successfully probed ? */
					if (!(Defender_char->Flags & END_MONSTER) &&
					 (Probe_skill(Attacker_part->Char_handle, CRITICAL_HIT)))
					{
						/* Yes -> Print "<attacker> made a critical hit!" */
						Print_combat_message(Attacker_part, Defender_part, 0, 449);

						/* Set afflicted damage to maximum */
						Afflicted_damage = Get_LP(Defender_part->Char_handle);
					}
					else
					{
						/* No -> Calculate afflicted damage */
						Afflicted_damage =
						 Calculate_afflicted_damage(Attacker_part, Defender_part);
					}

					/* Any damage done ? */
					if (Afflicted_damage)
					{
						/* Yes -> Print "<attacker> does <damage> damage! */
						Print_combat_message(Attacker_part, Defender_part,
						 Afflicted_damage, 450);

						/* Process damage for danger evaluation */

						/* Do damage */
						Do_combat_damage(Defender_part, Afflicted_damage);
					}
					else
					{
						/* No -> Print "No damage done!" */
						Print_combat_message(Attacker_part, Defender_part, 0, 451);
					}
				}
			}
		}
		else
		{
			/* No -> Print "<attacker> failed!" */
			Print_combat_message(Attacker_part, Defender_part, 0, 452);

		}

		MEM_Free_pointer(Defender_part->Char_handle);
	}
	else
	{
		/* No -> Print "<attacker> missed!" */
		Print_combat_message(Attacker_part, Defender_part, 0, 453);

		/* Reset action */
		Attacker_part->Current_action = NO_COMACT;

		/* Abort attack */
		Abort_attack = TRUE;
	}

	MEM_Free_pointer(Attacker_part->Char_handle);

	return(TRUE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_ammo_and_reload
 * FUNCTION  : Remove the current weapon's ammunition and try to reload.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.03.95 18:08
 * LAST      : 10.03.95 18:08
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to the
 *              attacker's combat participant data.
 *             UNSHORT Current_weapon_slot_index - Index of slot containing
 *              currently used weapon (1...9 / 0 for no weapon).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function can be safely called when
 *              Current_weapon_slot_index is set to NO_BODY_PLACE (0).
 *             - No matter in *which* slot the weapon is, the ammunition (if
 *              any) is always assumed to be in the left hand. However, the
 *              function will check if this is the case.
 *             - To make a long-range weapon without ammo, set the ammo ID
 *              to zero. To make a long-range weapon which is thrown, set
 *              the DESTROY_AFTER_USE flag.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_ammo_and_reload(struct Combat_participant *Attacker_part,
 UNSHORT Current_weapon_slot_index)
{
	struct Character_data *Attacker_char;
	struct Item_data *Weapon_item_data;
	struct Item_data *Ammo_item_data;
	BOOLEAN Found;
	UNSHORT Ammo_item_index;
	UNSHORT i;

	/* Get attacker's character data */
	Attacker_char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Any weapon ? */
	if (Current_weapon_slot_index != NO_BODY_PLACE)
	{
		/* Yes -> Get weapon item data */
		Weapon_item_data =
		 Get_item_data(&(Attacker_char->Body_items[Current_weapon_slot_index -
		 1]));

		/* Does this weapon require ammunition / anything in the left hand ? */
		if ((Weapon_item_data->Ammo_ID) &&
		 !(Packet_empty(&(Attacker_char->Body_items[LEFT_HAND - 1]))))
		{
			/* Yes -> Get ammunition item index */
			Ammo_item_index = Attacker_char->Body_items[LEFT_HAND - 1].Index;

			/* Get ammunition item data */
			Ammo_item_data =
			 Get_item_data(&(Attacker_char->Body_items[LEFT_HAND - 1]));

			/* Is this item ammunition / is it the RIGHT ammunition ? */
			if ((Ammo_item_data->Type == AMMO_IT) &&
			 (Weapon_item_data->Ammo_ID == Ammo_item_data->Ammo_ID))
			{
				/* Yes -> Search backpack for item with same index */
				Found = FALSE;
				for (i=0;i<ITEMS_PER_CHAR;i++)
				{
					/* Does the item have the same index ? */
					if (Attacker_char->Backpack_items[i].Index == Ammo_item_index)
					{
						/* Yes -> Remove the item */
						Remove_item(Attacker_part->Char_handle, i + ITEMS_ON_BODY + 1,
						 1);

						/* Found it! */
						Found = TRUE;
						break;
					}
				}

				/* Found something ? */
				if (!Found)
				{
					/* No -> Remove item in left hand */
					Remove_item(Attacker_part->Char_handle, LEFT_HAND, 1);

					/* Print "<attacker> used last ammo!" */
					Print_combat_message(Attacker_part, NULL, 0, 454);

					/* Reset participant action */
					Attacker_part->Current_action = NO_COMACT;

					/* Abort attack */
					Abort_attack = TRUE;
				}
			}
			Free_item_data();
		}
		else
		{
			/* No -> Should this weapon be destroyed after use ? */
			if (Weapon_item_data->Flags & DESTROY_AFTER_USE)
			{
				/* Yes -> Set ammunition item index */
				Ammo_item_index =
				 Attacker_char->Body_items[Current_weapon_slot_index - 1].Index;

				/* Search backpack for item with same index */
				Found = FALSE;
				for (i=0;i<ITEMS_PER_CHAR;i++)
				{
					/* Does the item have the same index ? */
					if (Attacker_char->Backpack_items[i].Index == Ammo_item_index)
					{
						/* Yes -> Remove the item */
						Remove_item(Attacker_part->Char_handle, i + ITEMS_ON_BODY + 1,
						 1);

						/* Found it! */
						Found = TRUE;
						break;
					}
				}

				/* Found something ? */
				if (!Found)
				{
					/* No -> Remove weapon */
					Remove_item(Attacker_part->Char_handle,
					 Current_weapon_slot_index, 1);

					/* Print "<attacker> used last ammo!" */
					Print_combat_message(Attacker_part, NULL, 0, 454);

					/* Reset participant action */
					Attacker_part->Current_action = NO_COMACT;

					/* Abort attack */
					Abort_attack = TRUE;
				}
			}
		}
		Free_item_data();
	}

	MEM_Free_pointer(Attacker_part->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Flee_combat_action
 * FUNCTION  : Flee combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:31
 * LAST      : 08.03.95 16:31
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Flee_combat_action(struct Combat_participant *Part)
{
	struct Character_data *Char;

	/* Tell the player */
	Set_permanent_message_nr(445);

	/* Set fleeing condition */
	Set_condition(Part->Char_handle, FLEEING);

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Count up */
		Fled_members++;
	}
	else
	{
		/* Monster -> Increase gained experience */
		Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

		Gained_experience_points += (UNLONG) Char->Experience_bonus;

		MEM_Free_pointer(Part->Char_handle);
	}

	/* Show fleeing */
	Combat_show(Part, SHOW_FLEE, NULL);

	/* Remove participant */
	Remove_participant(Part);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Cast_spell_combat_action
 * FUNCTION  : Cast spell combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 10:21
 * LAST      : 21.04.95 10:21
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Cast_spell_combat_action(struct Combat_participant *Part)
{
	UNCHAR String[300];
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	/* Get character name */
	Get_char_name(Part->Char_handle, Name);

	/* Tell the player */
	sprintf(String, System_text_ptrs[92], Name,
	 Get_spell_name(Part->Target.Magic_target_data.Class_nr,
	 Part->Target.Magic_target_data.Spell_nr));

	Set_permanent_text(String);

	/* Cast the spell _*/
	Do_cast_combat_spell(Part);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Use_magic_object_combat_action
 * FUNCTION  : Use magic object combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 10:35
 * LAST      : 12.05.95 10:35
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Use_magic_object_combat_action(struct Combat_participant *Part)
{
	struct Character_data *Char;
	struct Item_packet *Packet;
	UNCHAR String[300];
	UNCHAR Char_name[CHAR_NAME_LENGTH + 1];
	UNCHAR Item_name[ITEM_NAME_LENGTH + 1];

	/* Get character name */
	Get_char_name(Part->Char_handle, Char_name);

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

	if (Part->Target.Magic_target_data.Source_item_slot_index <= ITEMS_ON_BODY)
	{
		Packet = &(Char->Body_items[Part->Target.Magic_target_data.Source_item_slot_index
		 - 1]);
	}
	else
	{
		Packet = &(Char->Backpack_items[Part->Target.Magic_target_data.Source_item_slot_index
		 - ITEMS_ON_BODY - 1]);
	}

	/* Get item name */
	Get_item_name(Packet, Item_name);

	MEM_Free_pointer(Part->Char_handle);

	/* Tell the player */
	sprintf(String, System_text_ptrs[134], Char_name,
	 Get_spell_name(Part->Target.Magic_target_data.Class_nr,
	 Part->Target.Magic_target_data.Spell_nr), Item_name);

	Set_permanent_text(String);

	/* Use the magic item */
	Do_cast_combat_spell(Part);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Break_combat_items
 * FUNCTION  : Try to break items used in combat.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 13.03.95 13:32
 * LAST      : 13.03.95 13:32
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Body_item_slot_mask - Bit-list indicating which
 *              body item slots should be checked.
 *             UNLONG Item_type_mask - Bit-list indicating which item types
 *              can break.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - It is assumed the items are used. Therefore appropriate
 *              sound effects are played.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Break_combat_items(MEM_HANDLE Char_handle, UNSHORT Body_item_slot_mask,
 UNLONG Item_type_mask)
{
	struct Character_data *Char;
	struct Item_data *Body_item_data;
	UNSHORT i;

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check all body slots */
	for (i=1;i<=ITEMS_ON_BODY;i++)
	{
		/* Check this slot ? */
		if (Body_item_slot_mask & (1 << i))
		{
			/* Yes -> Anything in this slot ? */
			if (!Packet_empty(&(Char->Body_items[i - 1])))
			{
				/* Yes -> Get item data */
				Body_item_data = Get_item_data(&(Char->Body_items[i - 1]));

				/* Is it of the right type / not already broken ? */
				if ((Item_type_mask & (1 << Body_item_data->Type)) ||
				 !(Char->Body_items[i - 1].Flags & BROKEN_ITEM))
				{
					/* Yes -> Any sound effect ? */
					if (Body_item_data->Misc[1])
					{
						/* Yes -> Play sound effect */
						Play_sound_effect((UNSHORT) Body_item_data->Misc[1],
						 COMBAT_ITEM_SOUND_PRIORITY, COMBAT_ITEM_SOUND_VOLUME,
						 COMBAT_ITEM_SOUND_VARIABILITY, 0);
					}

					/* Try to break it */
					if (Probe((UNSHORT) Body_item_data->Break_chance, 1000))
					{
						/* Broken -> Print message */

						/* Break item */
						Char->Body_items[i - 1].Flags |= BROKEN_ITEM;

						/* Put away */
						Put_item_in_apres_pool(&(Char->Body_items[i - 1]));

						/* Remove item from character */
						Remove_item(Char_handle, i, 1);
					}
				}
				Free_item_data();
			}
		}
	}
	MEM_Free_pointer(Char_handle);
}

