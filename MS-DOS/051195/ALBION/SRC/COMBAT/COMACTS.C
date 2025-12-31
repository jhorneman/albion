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
#include <string.h>

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
#include <SOUND.H>
#include <APRES.H>
#include <XFTYPES.H>
#include <STATAREA.H>
#include <ITMLOGIC.H>
#include <PRTLOGIC.H>
#include <MONLOGIC.H>
#include <GAMETEXT.H>
#include <TEXTWIN.H>

/* prototypes */

BOOLEAN Do_close_range_attack(struct Event_action *Action);

UNSHORT Calculate_afflicted_damage(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

BOOLEAN Do_long_range_attack(struct Event_action *Action);

void Remove_ammo_and_reload(struct Combat_participant *Attacker_part,
 UNSHORT Current_weapon_slot_index);

void Search_and_remove_ammo(struct Combat_participant *Part,
 UNSHORT Ammo_item_slot_index);

BOOLEAN Break_combat_items(MEM_HANDLE Char_handle, UNSHORT Body_item_slot_mask,
 UNLONG Item_type_mask);

/* global variables */

UNSHORT Global_monster_damage = 0;
UNSHORT Global_monster_protection = 0;

BOOLEAN Abort_attack;

/* Combat action handler table */
struct Combat_action Combat_action_table[MAX_COMACTS] = {
	{ 0, NULL },														/* NO_COMACT */
	{ MOVE_MASK,		Move_combat_action },   				/* MOVE_COMACT */
	{ ATTACK_MASK,		Close_range_combat_action },   		/* CLOSE_RANGE_COMACT */
	{ ATTACK_MASK,		Long_range_combat_action },   		/* LONG_RANGE_COMACT */
	{ FLEE_MASK,		Flee_combat_action },   				/* FLEE_COMACT */
	{ MAGIC_MASK,		Cast_spell_combat_action },   		/* CAST_SPELL_COMACT */
	{ CONTROL_MASK,	Use_magic_object_combat_action },	/* USE_MAGIC_ITEM_COMACT */
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_combat_action
 * FUNCTION  : Move combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.05.95 10:32
 * LAST      : 25.10.95 13:59
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
	union Combat_show_parms P;
	UNSHORT Target_square_index;
	UNSHORT X, Y;
	UNSHORT Nr_moves;
	UNSHORT Actual_nr_moves;
	UNSHORT i;

	/* Get number of moves */
	Nr_moves = Part->Target.Move_target_data.Nr_moves;

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Tell the player */
		Set_permanent_message_nr(444);

		/* Get target square index */
		Target_square_index = Part->Target.Move_target_data.Target_square_indices[Nr_moves - 1];

		/* Get target square coordinates */
		X = Target_square_index % NR_TACTICAL_COLUMNS;
		Y = Target_square_index / NR_TACTICAL_COLUMNS;

		/* Are the coordinates inside the combat matrix ? */
		if ((X < NR_TACTICAL_COLUMNS) && (Y < NR_TACTICAL_ROWS))
		{
			/* Yes -> Is empty ? */
			if (Combat_matrix[Y][X].Part)
			{
				/* No -> Tell the player */
				Set_permanent_message_nr(443);
			}
			else
			{
				/* Yes -> Is the participant inside the combat matrix ? */
				if ((Part->Tactical_X < NR_TACTICAL_COLUMNS) &&
				 (Part->Tactical_Y < NR_TACTICAL_ROWS))
				{
					/* Yes -> Clear source square */
					Combat_matrix[Part->Tactical_Y][Part->Tactical_X].Part = NULL;
				}

				/* Put participant in target square */
				Combat_matrix[Y][X].Part = Part;

				/* Set new tactical coordinates */
				Part->Tactical_X = X;
				Part->Tactical_Y = Y;
			}
		}
	}
	else
	{
		/* Monster -> Tell the player */
		Set_permanent_message_nr(444);

		/* Build list of all moves */
		Actual_nr_moves = 0;
		for (i=0;i<Nr_moves;i++)
		{
			/* Get target square index */
			Target_square_index = Part->Target.Move_target_data.Target_square_indices[i];

			/* Get target square coordinates */
			X = Target_square_index % NR_TACTICAL_COLUMNS;
			Y = Target_square_index / NR_TACTICAL_COLUMNS;

			/* Are the coordinates inside the combat matrix ? */
			if ((X < NR_TACTICAL_COLUMNS) && (Y < NR_TACTICAL_ROWS))
			{
				/* Yes -> Is empty ? */
				if (Combat_matrix[Y][X].Part)
				{
					/* No -> Break */
					break;
				}
				else
				{
					/* Yes -> Insert in list */
					P.Move_parms.Positions[i][0] = X;
					P.Move_parms.Positions[i][1] = Y;

					/* Count up */
					Actual_nr_moves++;
				}
			}
			else
			{
				/* No -> Break */
				break;
			}
		}

		/* Any real moves ? */
		if (Actual_nr_moves > 0)
		{
			/* Yes -> Show movement */
			P.Move_parms.Nr_moves = Actual_nr_moves;
			Combat_show(Part, SHOW_MOVE, &P);

			/* Get target square index */
			Target_square_index = Part->Target.Move_target_data.Target_square_indices[Actual_nr_moves - 1];

			/* Get target square coordinates */
			X = Target_square_index % NR_TACTICAL_COLUMNS;
			Y = Target_square_index / NR_TACTICAL_COLUMNS;

			/* Are the coordinates inside the combat matrix ? */
			if ((X < NR_TACTICAL_COLUMNS) && (Y < NR_TACTICAL_ROWS))
			{
				/* Yes -> Is the participant inside the combat matrix ? */
				if ((Part->Tactical_X < NR_TACTICAL_COLUMNS) &&
				 (Part->Tactical_Y < NR_TACTICAL_ROWS))
				{
					/* Yes -> Clear source square */
					Combat_matrix[Part->Tactical_Y][Part->Tactical_X].Part = NULL;
				}

				/* Put participant in target square */
				Combat_matrix[Y][X].Part = Part;

				/* Set new tactical coordinates */
				Part->Tactical_X = X;
				Part->Tactical_Y = Y;
			}
		}
	}

	/* Reset action */
	Part->Current_action = NO_COMACT;
}

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
 * LAST      : 28.10.95 15:19
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
	struct Monster_tactic *Tactic_ptr;
	struct Combat_participant *Attacker_part;
	struct Combat_participant *Defender_part;
	struct Character_data *Attacker_char;
	struct Character_data *Defender_char;
	struct Item_data *Weapon_item_data;
	BOOLEAN Broken;
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
	Target_X = Attacker_part->Target.Attack_target_data.Target_square_index %
	 NR_TACTICAL_COLUMNS;
	Target_Y = Attacker_part->Target.Attack_target_data.Target_square_index /
	 NR_TACTICAL_COLUMNS;

	Defender_part = Combat_matrix[Target_Y][Target_X].Part;

	/* Clear abort attack flag */
	Abort_attack = FALSE;

	/* Get attacker's character data */
	Attacker_char = (struct Character_data *)
	 MEM_Claim_pointer(Attacker_part->Char_handle);

	/* Select (next) weapon */
	Current_weapon_slot_index = RIGHT_HAND;

	/* Anything in the current weapon slot ? */
	if (!Packet_empty(&(Attacker_char->Body_items[Current_weapon_slot_index - 1])))
	{
		/* Yes -> Get item data */
		Weapon_item_data = Get_item_data(&(Attacker_char->Body_items[Current_weapon_slot_index - 1]));

		/* Is a close-range weapon ? */
		if (Weapon_item_data->Type == CLOSE_RANGE_IT)
		{
			/* Yes -> This is the used weapon */
			Current_weapon_item_index =
			 Attacker_char->Body_items[Current_weapon_slot_index - 1].Index;
		}
		else
		{
			/* No -> No weapon */
			Current_weapon_item_index = 0;
		}

		Free_item_data();
	}
	else
	{
		/* No -> No weapon */
		Current_weapon_item_index = 0;
	}

	/* Store */
	Attacker_part->Weapon_item_index = Current_weapon_item_index;

	/* Show close-range attack */
	P.Attack_parms.Target_X				= Target_X;
	P.Attack_parms.Target_Y				= Target_Y;
	P.Attack_parms.Weapon_slot_index	= Current_weapon_slot_index;

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
		if (Probe_skill(Attacker_part->Char_handle, CLOSE_RANGE_ATTACK))
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
				#if FALSE
				/* Yes -> Is the defender doing nothing /
				 can the defender do something /
				 is the defender's dexterity attribute successfully probed ? */
				if ((Defender_part->Current_action == NO_COMACT) &&
				 !(Get_conditions(Defender_part->Char_handle) & DEFEND_MASK) &&
				 Probe(40, 100))
//				 (Probe_attribute(Defender_part->Char_handle, DEXTERITY)))
				{
					/* Yes -> Print "Attack was deflected!" */
					Print_combat_message(Attacker_part, Defender_part, 0, 448);

					/* Try to break the attacker's weapon */
					Broken = Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1L << CLOSE_RANGE_IT));

					if (Broken)
					{
						Abort_attack = TRUE;
					}

					/* Try to break the defender's weapon */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << RIGHT_HAND), (1L << CLOSE_RANGE_IT));

					/* Try to break the defender's shield */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << LEFT_HAND), (1L << SHIELD_IT));
				}
				else
				#endif
				{
					/* No -> Try to break the attacker's weapon */
					Broken = Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1L << CLOSE_RANGE_IT));

					if (Broken)
					{
						Abort_attack = TRUE;
					}

					/* Try to break the defender's armour */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << BODY), (1L << ARMOUR_IT));

					/* Try to break the defender's helmet */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << HEAD), (1L << HELMET_IT));

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
//						Print_combat_message(Attacker_part, Defender_part,
//						 Afflicted_damage, 450);

						/* Do damage */
						Do_combat_damage(Defender_part, Afflicted_damage);

						/* Is the attacker a monster ? */
						if (Attacker_part->Type == MONSTER_PART_TYPE)
						{
							/* Yes -> Get attacker tactic */
							Tactic_ptr = Get_monster_tactic(Attacker_part);

							/* Was an attack result given ? */
							if (Tactic_ptr->Attack_result)
							{
								/* Yes -> Execute it */
								(Tactic_ptr->Attack_result)(Attacker_part, Defender_part);
							}
						}
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

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_afflicted_damage
 * FUNCTION  : Calculate the damage afflicted by an attack.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 22:02
 * LAST      : 23.09.95 23:00
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
		/* Yes -> Add temporary spell effect */
		Damage += (Get_member_temporary_spell_strength(Attacker_part->Number,
		 ATTACK_TEMP_SPELL) * Damage) / 100;
	}
	else
	{
		/* No -> Add global monster damage value */
		Damage += Global_monster_damage;
	}

	/* Randomize damage */
	Damage = Get_rnd_50_100(Damage);

	/* Calculate maximum protection */
	Protection = Get_protection(Defender_part->Char_handle);

	/* Is party member ? */
	if (Defender_part->Type == PARTY_PART_TYPE)
	{
		/* Yes -> Add temporary spell effect */
		Protection += (Get_member_temporary_spell_strength(Defender_part->Number,
		 DEFENCE_TEMP_SPELL) * Protection) / 100;
	}
	else
	{
		/* No -> Add global monster protection value */
		Protection += Global_monster_protection;
	}

	/* Randomize protection */
	Protection = Get_rnd_50_100(Protection);

	/* Return the difference */
	return(max((Damage - Protection), 0));
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
 * LAST      : 07.10.95 22:40
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
	struct Monster_tactic *Tactic_ptr;
	struct Combat_participant *Attacker_part;
	struct Combat_participant *Defender_part;
	struct Character_data *Attacker_char;
	struct Character_data *Defender_char;
	struct Item_data *Weapon_item_data;
	BOOLEAN Broken;
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

	/* Anything in the current weapon slot ? */
	if (!Packet_empty(&(Attacker_char->Body_items[Current_weapon_slot_index - 1])))
	{
		/* Yes -> Get item data */
		Weapon_item_data = Get_item_data(&(Attacker_char->Body_items[Current_weapon_slot_index - 1]));

		/* Is a long-range weapon ? */
		if (Weapon_item_data->Type == LONG_RANGE_IT)
		{
			/* Yes -> This is the used weapon */
			Current_weapon_item_index =
			 Attacker_char->Body_items[Current_weapon_slot_index - 1].Index;
		}
		else
		{
			/* No -> No weapon */
			Current_weapon_item_index = 0;
		}

		Free_item_data();
	}
	else
	{
		/* No -> No weapon */
		Current_weapon_item_index = 0;
	}

	/* Store */
	Attacker_part->Weapon_item_index = Current_weapon_item_index;

	/* Show long-range attack / flying projectile */
	P.Attack_parms.Target_X = Target_X;
	P.Attack_parms.Target_Y = Target_Y;
	P.Attack_parms.Weapon_slot_index = Current_weapon_slot_index;
	Combat_show(Attacker_part, SHOW_LONG_RANGE, &P);

	/* Remove ammunition */
	Remove_ammo_and_reload(Attacker_part, Current_weapon_slot_index);

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
				#if FALSE
				/* Yes -> Is the defender doing nothing /
				 can the defender do something /
				 is the defender's dexterity attribute successfully probed ? */
				if ((Defender_part->Current_action == NO_COMACT) &&
				 !(Get_conditions(Defender_part->Char_handle) & DEFEND_MASK) &&
				 Probe(40, 100))
//				 (Probe_attribute(Defender_part->Char_handle, DEXTERITY)))
				{
					/* Yes -> Print "Attack was deflected!" */
					Print_combat_message(Attacker_part, Defender_part, 0, 448);

					/* Try to break the attacker's weapon */
					Broken = Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1L << LONG_RANGE_IT));

					if (Broken)
					{
						Abort_attack = TRUE;
					}

					/* Try to break the defender's weapon */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << RIGHT_HAND), (1L << CLOSE_RANGE_IT));

					/* Try to break the defender's shield */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << LEFT_HAND), (1L << SHIELD_IT));
				}
				else
				#endif
				{
					/* No -> Try to break the attacker's weapon */
					Broken = Break_combat_items(Attacker_part->Char_handle,
					 (1 << Current_weapon_slot_index), (1L << LONG_RANGE_IT));

					if (Broken)
					{
						Abort_attack = TRUE;
					}

					/* Try to break the defender's armour */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << BODY), (1L << ARMOUR_IT));

					/* Try to break the defender's helmet */
					Break_combat_items(Defender_part->Char_handle,
					 (1 << HEAD), (1L << HELMET_IT));

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
//						Print_combat_message(Attacker_part, Defender_part,
//						 Afflicted_damage, 450);

						/* Do damage */
						Do_combat_damage(Defender_part, Afflicted_damage);

						/* Is the attacker a monster ? */
						if (Attacker_part->Type == MONSTER_PART_TYPE)
						{
							/* Yes -> Get attacker tactic */
							Tactic_ptr = Get_monster_tactic(Attacker_part);

							/* Was an attack result given ? */
							if (Tactic_ptr->Attack_result)
							{
								/* Yes -> Execute it */
								(Tactic_ptr->Attack_result)(Attacker_part, Defender_part);
							}
						}
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

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_ammo_and_reload
 * FUNCTION  : Remove the current weapon's ammunition and try to reload.
 * FILE      : INVITEMS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.03.95 18:08
 * LAST      : 12.10.95 14:38
 * INPUTS    : struct Combat_participant *Attacker_part - Pointer to the
 *              attacker's combat participant data.
 *             UNSHORT Weapon_slot_index - Index of slot containing
 *              currently used weapon (1...9 / 0 for no weapon).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function can be safely called when
 *              Current_weapon_slot_index is set to NO_BODY_PLACE (0).
 *             - No matter in *which* slot the weapon is, the ammunition (if
 *              any) is always assumed to be in the left hand. However, the
 *              function will check if this is the case.
 *             - To make a long-range weapon without ammo, set the ammo ID
 *              to zero.
 *             - To make a long-range weapon which is thrown, set
 *              the ammo ID to zero and the DESTROY_AFTER_USE flag in the
 *              weapon's item data.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_ammo_and_reload(struct Combat_participant *Attacker_part,
 UNSHORT Weapon_slot_index)
{
	struct Character_data *Attacker_char;
	struct Item_data *Weapon_item_data;
	struct Item_data *Ammo_item_data;
	UNSHORT Ammo_ID;

	/* Any weapon ? */
	if (Weapon_slot_index != NO_BODY_PLACE)
	{
		/* Yes -> Get attacker's character data */
		Attacker_char = (struct Character_data *)
		 MEM_Claim_pointer(Attacker_part->Char_handle);

		/* Get weapon item data */
		Weapon_item_data = Get_item_data(&(Attacker_char->Body_items[Weapon_slot_index - 1]));

		/* Get ammo ID from weapon */
		Ammo_ID = (UNSHORT) Weapon_item_data->Ammo_ID;

		/* Is ammunition required / anything in the left hand ? */
		if (Ammo_ID && !(Packet_empty(&(Attacker_char->Body_items[LEFT_HAND - 1]))))
		{
			/* Yes -> Get ammunition item data */
			Ammo_item_data = Get_item_data(&(Attacker_char->Body_items[LEFT_HAND - 1]));

			/* Is this item ammunition / is it the RIGHT ammunition ? */
			if ((Ammo_item_data->Type == AMMO_IT) &&
			 (Ammo_item_data->Ammo_ID == Ammo_ID))
			{
				/* Yes -> Search and remove ammo */
				Search_and_remove_ammo(Attacker_part, LEFT_HAND);
			}
			Free_item_data();
		}
		else
		{
			/* No -> Should this weapon be destroyed after use ? */
			if (Weapon_item_data->Flags & DESTROY_AFTER_USE)
			{
				/* Yes -> Search and remove "ammo" */
				Search_and_remove_ammo(Attacker_part, Weapon_slot_index);
			}
		}
		Free_item_data();

		MEM_Free_pointer(Attacker_part->Char_handle);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_and_remove_ammo
 * FUNCTION  : Search and remove ammo from a character.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.10.95 13:37
 * LAST      : 12.10.95 13:37
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Ammo_item_slot_index - Item slot index of ammo (1...9).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : -
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Search_and_remove_ammo(struct Combat_participant *Part,
 UNSHORT Ammo_item_slot_index)
{
	struct Character_data *Char;
	BOOLEAN Found;
	UNSHORT Ammo_item_index;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

	/* Get ammo item index */
	Ammo_item_index = Char->Body_items[Ammo_item_slot_index - 1].Index;

	/* Search backpack for item with same index */
	Found = FALSE;
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Does the item have the same index ? */
		if (Char->Backpack_items[i].Index == Ammo_item_index)
		{
			/* Yes -> Remove the item */
			Remove_item
			(
				Part->Char_handle,
				i + ITEMS_ON_BODY + 1,
				1
			);

			/* Found it! */
			Found = TRUE;
			break;
		}
	}

	/* Found something ? */
	if (!Found)
	{
		/* No -> Remove weapon */
		Remove_item
		(
			Part->Char_handle,
			Ammo_item_slot_index,
			1
		);

		/* Print "<attacker> used last ammo!" */
		Print_combat_message(Part, NULL, 0, 454);

		/* Reset participant action */
		Part->Current_action = NO_COMACT;

		/* Abort attack */
		Abort_attack = TRUE;
	}

	MEM_Free_pointer(Part->Char_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Flee_combat_action
 * FUNCTION  : Flee combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:31
 * LAST      : 23.10.95 21:13
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Something similar is done in the Flee combat spell.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Flee_combat_action(struct Combat_participant *Part)
{
	struct Character_data *Char;

	/* Party or monster ? */
	if (Part->Type == PARTY_PART_TYPE)
	{
		/* Party -> Still in the bottom row ? */
		if (Part->Tactical_Y == 4)
		{
			/* Yes -> Tell the player */
			Set_permanent_message_nr(445);

			/* Set fleeing condition */
			Set_condition(Part->Char_handle, FLEEING);

			/* Remove participant */
			Remove_participant(Part);
		}
	}
	else
	{
		/* Monster -> In the top row ? */
		if (Part->Tactical_Y == 0)
		{
			/* Yes -> Tell the player */
			Set_permanent_message_nr(445);

			/* Set fleeing condition */
			Set_condition(Part->Char_handle, FLEEING);

			/* Increase gained experience */
			Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

			Added_experience_points += (UNLONG) Char->Experience_bonus;

			MEM_Free_pointer(Part->Char_handle);

			/* Show fleeing */
			Combat_show(Part, SHOW_FLEE, NULL);

			/* Remove participant */
			Remove_participant(Part);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Cast_spell_combat_action
 * FUNCTION  : Cast spell combat action handler.
 * FILE      : COMACTS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 10:21
 * LAST      : 09.10.95 23:00
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
	union Combat_show_parms P;
	UNCHAR String[300];
	UNCHAR Name[CHAR_NAME_LENGTH + 1];

	/* Is combat magic reversed ? */
	if (Reverse_combat_magic_flag)
	{
		struct Combat_participant *Victim_part;
		UNSHORT i;

		/* Yes -> The caster becomes the victim */
		Victim_part = Part;

		/* Look for a living monster */
		Part = NULL;
		for (i=0;i<Nr_monsters;i++)
		{
			if (Monster_parts[i].Type == MONSTER_PART_TYPE)
			{
				Part = &(Monster_parts[i]);
				break;
			}
		}

		/* Found one ? */
		if (Part)
		{
			/* Yes -> Copy magic target data from victim to monster */
			memcpy
			(
				(UNBYTE *) &(Part->Target.Magic_target_data),
				(UNBYTE *) &(Victim_part->Target.Magic_target_data),
				sizeof(struct Use_magic_data)
			);

			/* Clear the victim's action */
			Victim_part->Current_action = NO_COMACT;

			/* Make the victim the monster's target */
			Part->Target.Magic_target_data.Combat_target_mask =
			 1L << (Victim_part->Tactical_Y * NR_TACTICAL_COLUMNS +
			 Victim_part->Tactical_X);

			/* Let the monster use the caster's character data */
			Part->Target.Magic_target_data.Casting_handle = Victim_part->Char_handle;
			Part->Target.Magic_target_data.Casting_participant = Part;
		}
		else
		{
			/* No -> Everything stays as it was */
			Part = Victim_part;
		}
	}

	/* Get character name */
	Get_char_name(Part->Char_handle, Name);

	/* Tell the player */
	_bprintf
	(
		String,
		300,
		System_text_ptrs[92],
		Name,
		Get_spell_name
		(
			Part->Target.Magic_target_data.Class_nr,
			Part->Target.Magic_target_data.Spell_nr
		)
	);
	Set_permanent_text(String);

	/* Show casting */
	Combat_show(Part, SHOW_CAST_SPELL, &P);

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
	_bprintf
	(
		String,
		300,
		System_text_ptrs[134],
		Char_name,
		Get_spell_name
		(
			Part->Target.Magic_target_data.Class_nr,
	 		Part->Target.Magic_target_data.Spell_nr
		),
		Item_name
	);

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
 * LAST      : 24.10.95 15:34
 * INPUTS    : MEM_HANDLE Char_handle - Handle of character data.
 *             UNSHORT Body_item_slot_mask - Bit-list indicating which
 *              body item slots should be checked.
 *             UNLONG Item_type_mask - Bit-list indicating which item types
 *              can break.
 * RESULT    : BOOLEAN : Item broke.
 * BUGS      : No known.
 * NOTES     : - It is assumed the items are used. Therefore appropriate
 *              sound effects are played.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Break_combat_items(MEM_HANDLE Char_handle, UNSHORT Body_item_slot_mask,
 UNLONG Item_type_mask)
{
	struct Character_data *Char;
	struct Item_data *Body_item_data;
	BOOLEAN Broke_flag;
	UNSHORT i;
	UNCHAR Item_name[ITEM_NAME_LENGTH + 1];
	UNCHAR String[300];

	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check all body slots */
	Broke_flag = FALSE;
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
				if ((Item_type_mask & (1L << Body_item_data->Type)) ||
				 !(Char->Body_items[i - 1].Flags & BROKEN_ITEM))
				{
					#if FALSE
					/* Yes -> Any sound effect ? */
					if (Body_item_data->Misc[1])
					{
						/* Yes -> Play sound effect */
						Play_sound_effect
						(
							(UNSHORT) Body_item_data->Misc[1],
							COMBAT_ITEM_SOUND_PRIORITY,
							COMBAT_ITEM_SOUND_VOLUME,
							COMBAT_ITEM_SOUND_VARIABILITY,
							0
						);
					}
					#endif

					/* Try to break it */
					if (Probe((SISHORT) Body_item_data->Break_chance, 1000))
					{
						/* Broken -> Print message */
						Get_item_name(&(Char->Body_items[i - 1]), Item_name);

						_bprintf
						(
							String,
							300,
							System_text_ptrs[736],
							Item_name
						);

						Do_text_window(String);

						/* Break item */
						Char->Body_items[i - 1].Flags |= BROKEN_ITEM;

						/* Put away */
						Store_item_in_Apres_pool(&(Char->Body_items[i - 1]));

						/* Remove item from character */
						Remove_item(Char_handle, i, 1);

						/* Set flag */
						Broke_flag = TRUE;
					}
				}
				Free_item_data();
			}
		}
	}
	MEM_Free_pointer(Char_handle);

	return Broke_flag;
}

