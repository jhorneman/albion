/************
 * NAME     : MONLOGIC.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 17-9-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : MONLOGIC.H
 ************/

/*
 ** Pragmas ****************************************************************
 */

#pragma off (unreferenced);

/*
 ** Includes ***************************************************************
 */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBMEM.H>

#include <MAIN.H>
#include <GAMEVAR.H>

#include <MAGIC.H>
#include <COMBAT.H>
#include <COMBVAR.H>
#include <COMACTS.H>
#include <TACTICAL.H>
#include <MONLOGIC.H>
#include <ITMLOGIC.H>
#include <PRTLOGIC.H>
#include <APRES.H>

/*
 ** Defines ****************************************************************
 */

#define MAX_MONSTER_TACTICS		(9)

/*
 ** Prototypes *************************************************************
 */

void Select_offensive_monster_action(struct Combat_participant *Part);

void Try_magical_attack(struct Combat_participant *Part);

UNSHORT Count_enemies_in_row(struct Combat_participant *Part, UNSHORT Row_number);

UNSHORT Select_random_enemy(struct Combat_participant *Part);

UNSHORT Choose_monster_spell(UNSHORT Class_nr, UNLONG Possible_spells,
 UNSHORT Spell_target);

void Try_long_range_attack(struct Combat_participant *Part);

void Select_long_range_target(struct Combat_participant *Part,
 UNSHORT Weapon_item_slot_index);

void Try_close_range_attack(struct Combat_participant *Part);

void Select_close_range_target(struct Combat_participant *Part,
 UNSHORT Weapon_item_slot_index);

void Make_offensive_move(struct Combat_participant *Part, UNSHORT Target_X);

UNSHORT Make_offensive_horizontal_move(UNSHORT Tactical_X,
 UNSHORT Tactical_Y, UNSHORT Target_X, UNLONG Movement_mask);

UNLONG Get_targets_within_movement_range(struct Combat_participant *Part);

BOOLEAN Check_move_left(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNLONG Movement_mask);

BOOLEAN Check_move_right(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNLONG Movement_mask);

UNSHORT Search_and_equip_new_weapon(struct Combat_participant *Part);

BOOLEAN Default_decider(struct Combat_participant *Part);

BOOLEAN Rinrii_decider(struct Combat_participant *Part);

BOOLEAN Kritha_decider(struct Combat_participant *Part);

BOOLEAN Mage_decider(struct Combat_participant *Part);

struct Combat_participant *Kizz_target_select(struct Combat_participant *Part);

struct Combat_participant *AI_target_select(struct Combat_participant *Part);

void Warniak_1_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

void Warniak_2_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

void Warniak_3_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

void AI_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part);

/*
 ** Global variables *******************************************************
 */

static struct Monster_tactic Tactic_table[MAX_MONSTER_TACTICS] = {
	/* Default tactic */
	{
		Default_decider,
		NULL,
		NULL,
		0
	},
	/* Special tactic 2 : Rinrii */
	{
		Rinrii_decider,
		NULL,
		NULL,
		0
	},
	/* Special tactic 3 : Kizz */
	{
		Default_decider,
		Kizz_target_select,
		NULL,
		0
	},
	/* Special tactic 4 : Warniak 1 */
	{
		Default_decider,
		NULL,
		Warniak_1_attack_result,
		0
	},
	/* Special tactic 5 : Warniak 2 */
	{
		Default_decider,
		NULL,
		Warniak_2_attack_result,
		0
	},
	/* Special tactic 6 : Warniak 3 */
	{
		Default_decider,
		NULL,
		Warniak_3_attack_result,
		0
	},
	/* Special tactic 7 : Kritha */
	{
		Kritha_decider,
		NULL,
		NULL,
		0
	},
	/* Special tactic 8 : Mage */
	{
		Mage_decider,
		NULL,
		NULL,
		0
	},
	/* Special tactic 9 : AI */
	{
		Default_decider,
		AI_target_select,
		AI_attack_result,
		0
	}
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_monster_tactic
 * FUNCTION  : Get the pointer to a monster's tactic data.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 18:20
 * LAST      : 08.10.95 19:41
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : struct Monster_tactic * : Pointer to tactic.
 * BUGS      : No known.
 * NOTES     : - This function may be safely called for party members.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Monster_tactic *
Get_monster_tactic(struct Combat_participant *Part)
{
	SISHORT Tactic_index;

	/* Get tactic index */
	Tactic_index = max(0, Part->Tactic_index - 1);

	/* Legal tactic index ? */
	if (Tactic_index >= MAX_MONSTER_TACTICS)
	{
		/* No -> Use default tactic */
		Tactic_index = 0;
	}

	/* Return pointer to tactic */
	return &(Tactic_table[Tactic_index]);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_monster_action
 * FUNCTION  : Select an action for a monster.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 18:11
 * LAST      : 16.10.95 20:08
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_monster_action(struct Combat_participant *Part)
{
	struct Monster_tactic *Tactic_ptr;
	BOOLEAN Decision;
	UNSHORT Char_bits;

	/* Clear monster action */
	Part->Current_action = NO_COMACT;

	/* Get pointer to tactic */
	Tactic_ptr = Get_monster_tactic(Part);

	/* Get character type */
	Char_bits = Get_character_type(Part->Char_handle);

	/* Is this an end monster ? */
	if (Char_bits & END_MONSTER)
	{
		/* Yes -> Always take offensive action */
		Decision = TRUE;
	}
	else
	{
		/* No -> Is there a decider function ? */
		if (Tactic_ptr->Decider)
		{
			/* Yes -> Call it */
			Decision = Tactic_ptr->Decider(Part);
		}
		else
		{
			/* No -> Take offensive action */
			Decision = TRUE;
		}
	}

	/* Offensive or defensive action ? */
	if (Decision)
	{
		/* Offensive -> Has a target already been selected ? */
		if (Part->Target_part)
		{
			/* Yes -> Is this target still valid /
			  still inside the combat matrix ? */
			if ((Part->Target_part->Flags & PART_REMOVED) ||
			 (Part->Target_part->Tactical_X >= NR_TACTICAL_COLUMNS) ||
			 (Part->Target_part->Tactical_Y >= NR_TACTICAL_ROWS))
			{
				/* No -> Select a new target */
				if (Tactic_ptr->Target_selector)
				{
					Part->Target_part = (Tactic_ptr->Target_selector)(Part);
				}
				else
				{
					Part->Target_part = NULL;
				}
			}
		}
		else
		{
			/* No -> Select a new target */
			if (Tactic_ptr->Target_selector)
			{
				Part->Target_part = (Tactic_ptr->Target_selector)(Part);
			}
			else
			{
				Part->Target_part = NULL;
			}
		}

		/* Select an action */
		Select_offensive_monster_action(Part);
	}
	else
	{
		/* Defensive -> Select an action */
		Select_defensive_monster_action(Part);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_offensive_monster_action
 * FUNCTION  : Select an offensive monster action.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 18:32
 * LAST      : 18.09.95 22:59
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_offensive_monster_action(struct Combat_participant *Part)
{
	static const UNSHORT Capability_table[16] = {
		PART_MAGIC_CAP,	PART_MAGIC_CAP,	PART_MAGIC_CAP,	PART_MAGIC_CAP,
		PART_MAGIC_CAP,	PART_MAGIC_CAP,	PART_CLOSE_CAP,	PART_CLOSE_CAP,
		PART_LONG_CAP,		PART_LONG_CAP,		PART_LONG_CAP,		PART_LONG_CAP,
		PART_LONG_CAP,		PART_LONG_CAP,		PART_CLOSE_CAP,	PART_CLOSE_CAP
	};

	UNSHORT Capabilities;
	UNSHORT Selected_capability;

	/* Clear monster action */
	Part->Current_action = NO_COMACT;

	/* Get monster's capabilities */
	Capabilities = Part->Capabilities;

	/* Try all capabilities */
	for (;;)
	{
		/* Any capabilities left ? */
		if (!Capabilities)
		{
			/* No -> Exit */
			break;
		}

		/* Select a random capability */
		Selected_capability = Capability_table[rand() & 0x000F];

		/* Does the monster have this capability ? */
		if (Capabilities & Selected_capability)
		{
			/* Yes -> Never select it again */
			Capabilities &= ~Selected_capability;

			/* Act depending on selected capability */
			switch (Selected_capability)
			{
				/* Try a magical attack */
				case PART_MAGIC_CAP:
				{
					Try_magical_attack(Part);
					break;
				}
				/* Try a long-range attack */
				case PART_LONG_CAP:
				{
					Try_long_range_attack(Part);
					break;
				}
				/* Try a close-range attack */
				case PART_CLOSE_CAP:
				{
					Try_close_range_attack(Part);
					break;
				}
			}

			/* Any action selected ? */
			if (Part->Current_action != NO_COMACT)
			{
				/* Yes -> Exit */
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_magical_attack
 * FUNCTION  : Try making a magical attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 19:23
 * LAST      : 01.10.95 17:10
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function can handle participants completely incapable
 *              of using magic.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Try_magical_attack(struct Combat_participant *Part)
{
	struct Use_magic_data *Use_magic_data_ptr;
	UNLONG Possible_spells;
	UNLONG Target_mask = 0;
	UNSHORT Selected_class;
	UNSHORT Selected_spell = 0;
	UNSHORT SP;
	UNSHORT Possible_spell_targets;
	UNSHORT Selected_row;
	UNSHORT i;

	Use_magic_data_ptr = &(Part->Target.Magic_target_data);

	/* Clear monster action */
	Part->Current_action = NO_COMACT;

	/* Get monster's SP */
	SP = Get_SP(Part->Char_handle);

	/* Is this participant still capable of magic / any SP left ? */
	if ((Get_conditions(Part->Char_handle) & MAGIC_MASK) || !SP)
	{
		/* No -> Deactivate magic capability */
		Part->Capabilities &= ~PART_MAGIC_CAP;
		return;
	}

	/* Find spell class */
	Selected_class = 0xFFFF;
	for (i=0;i<MAX_SPELL_CLASSES;i++)
	{
		/* Does this spell class exist ? */
		if (Spell_class_exists(i))
		{
			/* Yes -> Is this spell class known ? */
			if (Spell_class_known(Part->Char_handle, i))
			{
				/* Yes -> Found */
				Selected_class = i;
				break;
			}
		}
	}

	/* Found no spell class at all ? */
	if (Selected_class == 0xFFFF)
	{
		/* Yes -> Deactivate magic capability */
		Part->Capabilities &= ~PART_MAGIC_CAP;
		return;
	}

	/* Determine possible spells */
	Possible_spells = 0;
	Possible_spell_targets = 0;
	for (i=1;i<=Spells_per_class[Selected_class];i++)
	{
		/* Does this spell exist ? */
		if (Spell_exists(Selected_class, i))
		{
			/* Yes -> Is this spell known ? */
			if (Spell_known(Part->Char_handle, Selected_class, i))
			{
				/* Yes -> Is a combat spell / enough SP ? */
				if ((Get_spell_areas(Selected_class, i) & (1 << COMBAT_AREA)) &&
				 (Get_spell_required_SP(Selected_class, i)))
				{
					/* Yes -> Mark this spell */
					Possible_spells |= (1L << i);

					/* Mark spell target */
					Possible_spell_targets |= (Get_spell_targets(Selected_class, i) &
					 ENEMY_SPELL_TARGET_MASK);
				}
			}
		}
	}

	/* Any spells / any targets ? */
	if (!Possible_spells || !Possible_spell_targets)
	{
		/* No -> Deactivate magic capability */
		Part->Capabilities &= ~PART_MAGIC_CAP;
		return;
	}

	/* Any specific target given ? */
	if (Part->Target_part)
	{
		/* Yes -> Can cast on single party member ? */
		if (Possible_spell_targets & ONE_ENEMY_TARGET)
		{
			/* Yes ->  Choose spell */
			Selected_spell = Choose_monster_spell
			(
				Selected_class,
				Possible_spells,
				ONE_ENEMY_TARGET
			);

			/* Select target */
			Use_magic_data_ptr->Combat_target_mask	= 1L <<
			 ((Part->Target_part->Tactical_Y * NR_TACTICAL_COLUMNS) +
			 Part->Target_part->Tactical_X);
		}
		else
		{
			/* No -> Can cast on row of party members ? */
			if (Possible_spell_targets & ROW_ENEMIES_TARGET)
			{
				/* Yes -> Choose spell */
				Selected_spell = Choose_monster_spell
				(
					Selected_class,
					Possible_spells,
					ROW_ENEMIES_TARGET
				);

				/* Select the row containing the target */
				Selected_row = Part->Target_part->Tactical_Y;

				/* Build target mask */
				for (i=0;i<NR_TACTICAL_COLUMNS;i++)
				{
					Target_mask |= 1L << ((NR_TACTICAL_COLUMNS * Selected_row) + i);
				}

				/* Store target mask */
				Use_magic_data_ptr->Combat_target_mask	= Target_mask;
			}
			else
			{
				/* Yes -> Choose spell */
				Selected_spell = Choose_monster_spell
				(
					Selected_class,
					Possible_spells,
					ALL_ENEMIES_TARGET
				);

				/* Select all party members */
				Use_magic_data_ptr->Combat_target_mask	= COMBAT_PARTY_MASK;
			}
		}
	}
	else
	{
		/* No -> Choose spell */
		Selected_spell = Choose_monster_spell
		(
			Selected_class,
			Possible_spells,
			ONE_ENEMY_TARGET | ROW_ENEMIES_TARGET | ALL_ENEMIES_TARGET
		);

		/* Act depending on target of selected spell */
		switch (Get_spell_targets(Selected_class, Selected_spell))
		{
			/* One enemy */
			case ONE_ENEMY_TARGET:
			{
				/* Select a party member */
				Use_magic_data_ptr->Combat_target_mask	= (1L << Select_random_enemy(Part));
				break;
			}
			/* A row of enemies */
			case ROW_ENEMIES_TARGET:
			{
				/* Select the row with the most party members */
				if (Count_enemies_in_row(Part, 3) > Count_enemies_in_row(Part, 4))
					Selected_row = 3;
				else
					Selected_row = 4;

				/* Build target mask */
				for (i=0;i<NR_TACTICAL_COLUMNS;i++)
				{
					Target_mask |= 1L << ((NR_TACTICAL_COLUMNS * Selected_row) + i);
				}

				/* Store target mask */
				Use_magic_data_ptr->Combat_target_mask	= Target_mask;
				break;
			}
			/* All enemies */
			case ALL_ENEMIES_TARGET:
			{
				/* Select all party members */
				Use_magic_data_ptr->Combat_target_mask	= COMBAT_PARTY_MASK;
				break;
			}
		}
	}

	/* Was any spell chosen ? */
	if (Selected_spell)
	{
		/* Yes -> Enter magic action */
		Use_magic_data_ptr->Class_nr						= Selected_class;
		Use_magic_data_ptr->Spell_nr						= Selected_spell;
		Use_magic_data_ptr->Source_item_slot_index	= 0xFFFF;
		Use_magic_data_ptr->Target_item_slot_index	= 0xFFFF;
		Use_magic_data_ptr->Casting_member_index		= 0xFFFF;
		Use_magic_data_ptr->Casting_handle				= Part->Char_handle;
		Use_magic_data_ptr->Casting_participant		= Part;
		Use_magic_data_ptr->Combat_target_mode			= ENEMY_TARGMODE;

		/* Set action */
		Part->Current_action = CAST_SPELL_COMACT;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Count_enemies_in_row
 * FUNCTION  : Count a participant's enemies in a particular row.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 19:24
 * LAST      : 18.09.95 19:24
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Row_number - Row number (0...NR_TACTICAL_ROWS - 1).
 * RESULT    : UNSHORT : Number of enemies in this row.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Count_enemies_in_row(struct Combat_participant *Part, UNSHORT Row_number)
{
	struct Combat_participant *Current_part;
	UNSHORT Nr_enemies;
	UNSHORT i;

	/* Count enemies in row */
	Nr_enemies = 0;
	for (i=0;i<NR_TACTICAL_COLUMNS;i++)
	{
		/* Examine matrix entry */
		Current_part = Combat_matrix[Row_number][i].Part;

		/* Anyone there ? */
		if (Current_part)
		{
			/* Yes -> Is this participant an enemy ? */
			if (Current_part->Type != Part->Type)
			{
				/* Yes -> Count up */
				Nr_enemies++;
			}
		}
	}

	return Nr_enemies;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_random_enemy
 * FUNCTION  : Randomly select one of a participant's enemies.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 20:14
 * LAST      : 18.09.95 20:14
 * INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNSHORT : Tactical index of chosen enemy.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Select_random_enemy(struct Combat_participant *Part)
{
	struct Combat_participant *Current_part;
	UNSHORT Nr_enemies;
	UNSHORT Chosen_index;
	UNSHORT Tactical_index;
	UNSHORT i, j;

	/* Count enemies */
	Nr_enemies = 0;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Examine matrix entry */
			Current_part = Combat_matrix[i][j].Part;

			/* Anyone there ? */
			if (Current_part)
			{
				/* Yes -> Is this participant an enemy ? */
				if (Current_part->Type != Part->Type)
				{
					/* Yes -> Count up */
					Nr_enemies++;
				}
			}
		}
	}

	/* Randomly select an enemy */
	Chosen_index = rand() % Nr_enemies;

	/* Scan the combat matrix for this enemy */
	Tactical_index = 0xFFFF;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Examine matrix entry */
			Current_part = Combat_matrix[i][j].Part;

			/* Anyone there ? */
			if (Current_part)
			{
				/* Yes -> Is this participant an enemy ? */
				if (Current_part->Type != Part->Type)
				{
					/* Yes -> Is this the Chosen One ? */
					if (Chosen_index)
					{
						/* No -> Count down */
						Chosen_index--;
					}
					else
					{
						/* Yes -> Found */
						Tactical_index = (i * NR_TACTICAL_COLUMNS) + j;
						break;
					}
				}
			}
		}
		/* Found the chosen one ? */
		if (Tactical_index != 0xFFFF)
		{
			/* Yes -> Exit */
			break;
		}
	}

	return Tactical_index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Choose_monster_spell
 * FUNCTION  : Choose a spell for a monster.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 17:21
 * LAST      : 18.09.95 17:21
 * INPUTS    : UNSHORT Class_nr - Spell class index (0...MAX_SPELL_CLASSES).
 *             UNLONG Possible_spells - Bit-list of possible spells.
 *             UNSHORT Spell_target - Possible targets.
 * RESULT    : UNSHORT : Spell index (1...SPELLS_PER_CLASS) / 0.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Choose_monster_spell(UNSHORT Class_nr, UNLONG Possible_spells,
 UNSHORT Spell_target)
{
	UNLONG Choosable_spells;
	UNSHORT Nr_spells;
	UNSHORT Chosen_spell = 0;
	UNSHORT Chosen_spell_index;
	UNSHORT i;

	/* Determine possible spells with the right target */
	Nr_spells = 0;
	Choosable_spells = 0;
	for (i=1;i<=Spells_per_class[Class_nr];i++)
	{
		/* Is this spell possible ? */
		if (Possible_spells & (1L << i))
		{
			/* Yes -> Has the right target ? */
			if (Get_spell_targets(Class_nr, i) & Spell_target)
			{
				/* Yes -> Mark this spell */
				Choosable_spells |= (1L << i);

				/* Count up */
				Nr_spells++;
			}
		}
	}

	/* Can any spells be selected ? */
	if (Nr_spells)
	{
		/* Yes -> Choose one */
		Chosen_spell_index = (rand() % Nr_spells);

		/* Find the spell number */
		for (i=1;i<=Spells_per_class[Class_nr];i++)
		{
			/* Is this spell choosable ? */
			if (Choosable_spells & (1L << i))
			{
				/* Yes -> Is this the Chosen One ? */
				if (Chosen_spell_index)
				{
					/* No -> Count down */
					Chosen_spell_index--;
				}
				else
				{
					/* Yes -> Choose it */
					Chosen_spell = i;
					break;
				}
			}
		}
	}

	return Chosen_spell;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_long_range_attack
 * FUNCTION  : Try making a long-range attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 22:14
 * LAST      : 19.09.95 18:46
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Try_long_range_attack(struct Combat_participant *Part)
{
	BOOLEAN Search_for_new_weapon = FALSE;
	UNSHORT Weapon_slot_index;
	UNSHORT Weapon_item_type;

	/* Is this participant still capable of fighting ? */
	if (Get_conditions(Part->Char_handle) & ATTACK_MASK)
	{
		/* No -> Exit */
		return;
	}

	/* Search the participant's body for long-range weapons */
	Weapon_slot_index = Search_body_for_item_type
	(
		Part->Char_handle,
		LONG_RANGE_IT
	);

	/* Is the participant carrying a long-range weapon ? */
	if (Weapon_slot_index != 0xFFFF)
	{
		/* Yes -> Does it have the right ammo ? */
		if (Check_if_long_range_weapon_has_ammo
		(
			Part,
			Weapon_slot_index
		))
		{
			/* Yes -> Select a target */
			Select_long_range_target(Part, Weapon_slot_index);
		}
		else
		{
			/* No -> Search for a new weapon */
			Search_for_new_weapon = TRUE;
		}
	}
	else
	{
		/* No -> Search for a new weapon */
		Search_for_new_weapon = TRUE;
	}

	/* Search for a new weapon ? */
	if (Search_for_new_weapon)
	{
		/* Yes -> Search */
		Weapon_item_type = Search_and_equip_new_weapon(Part);

		/* Act depending on result */
		switch (Weapon_item_type)
		{
			/* Close-range weapon */
			case CLOSE_RANGE_IT:
			{
				/* Get weapon slot index */
				Weapon_slot_index = Search_body_for_item_type
				(
					Part->Char_handle,
					LONG_RANGE_IT
				);

				/* Select a target */
				Select_close_range_target
				(
					Part,
					Weapon_slot_index
				);
				break;
			}
			/* Long-range weapon */
			case LONG_RANGE_IT:
			{
				/* Get weapon slot index */
				Weapon_slot_index = Search_body_for_item_type
				(
					Part->Char_handle,
					LONG_RANGE_IT
				);

				/* Select a target */
				Select_long_range_target
				(
					Part,
					Weapon_slot_index
				);
				break;
			}
			/* Anything else (no new weapon found) */
			default:
			{
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_long_range_target
 * FUNCTION  : Select a long-range target.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 13:58
 * LAST      : 19.09.95 18:06
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Weapon_item_slot_index - Index of weapon item slot.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_long_range_target(struct Combat_participant *Part,
 UNSHORT Weapon_item_slot_index)
{
	UNLONG Possible_targets;
	UNSHORT Tactic_index;

	/* Get possible targets */
	Possible_targets = Get_long_range_targets(Part);

	/* Any ? */
	if (Possible_targets)
	{
		/* Yes -> Any specific target given ? */
		if (Part->Target_part)
		{
			/* Yes -> Calculate tactic index */
			Tactic_index = (Part->Target_part->Tactical_Y *
			 NR_TACTICAL_COLUMNS) + Part->Target_part->Tactical_X;

			/* Is this target in range ? */
			if (Possible_targets & (1L << Tactic_index))
			{
				/* Yes -> Attack it */
				Part->Current_action = LONG_RANGE_COMACT;
				Part->Target.Attack_target_data.Target_square_index		= Tactic_index;
				Part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_item_slot_index;
			}
		}
		else
		{
			/* No -> Just select a target */
			Tactic_index = Choose_monster_target(Possible_targets);

			/* Attack it */
			Part->Current_action = LONG_RANGE_COMACT;
			Part->Target.Attack_target_data.Target_square_index		= Tactic_index;
			Part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_item_slot_index;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_close_range_attack
 * FUNCTION  : Try making a close-range attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 18:46
 * LAST      : 19.09.95 18:46
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Try_close_range_attack(struct Combat_participant *Part)
{
	UNSHORT Weapon_slot_index;
	UNSHORT Weapon_item_type;

	/* Is this participant still capable of fighting ? */
	if (Get_conditions(Part->Char_handle) & ATTACK_MASK)
	{
		/* No -> Exit */
		return;
	}

	/* Search the participant's body for close-range weapons */
	Weapon_slot_index = Search_body_for_item_type
	(
		Part->Char_handle,
		CLOSE_RANGE_IT
	);

	/* Is the participant carrying a close-range weapon ? */
	if (Weapon_slot_index != 0xFFFF)
	{
		/* Yes -> Select a target */
		Select_close_range_target
		(
			Part,
			Weapon_slot_index
		);
	}
	else
	{
		/* No -> Can the participant do any damage ? */
		if (Get_damage(Part->Char_handle))
		{
			/* Yes  -> Select a target */
			Select_close_range_target
			(
				Part,
				Weapon_slot_index
			);
		}
		else
		{
			/* No -> Search for a new weapon */
			Weapon_item_type = Search_and_equip_new_weapon(Part);

			/* Act depending on result */
			switch (Weapon_item_type)
			{
				/* Close-range weapon */
				case CLOSE_RANGE_IT:
				{
					/* Get weapon slot index */
					Weapon_slot_index = Search_body_for_item_type
					(
						Part->Char_handle,
						LONG_RANGE_IT
					);

					/* Select a target */
					Select_close_range_target
					(
						Part,
						Weapon_slot_index
					);
					break;
				}
				/* Long-range weapon */
				case LONG_RANGE_IT:
				{
					/* Get weapon slot index */
					Weapon_slot_index = Search_body_for_item_type
					(
						Part->Char_handle,
						LONG_RANGE_IT
					);

					/* Select a target */
					Select_long_range_target
					(
						Part,
						Weapon_slot_index
					);
					break;
				}
				/* Anything else (no new weapon found) */
				default:
				{
					break;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_close_range_target
 * FUNCTION  : Select a close-range target.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 18:51
 * LAST      : 13.10.95 22:23
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Weapon_item_slot_index - Index of weapon item slot.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_close_range_target(struct Combat_participant *Part,
 UNSHORT Weapon_item_slot_index)
{
	UNLONG Possible_targets;
	UNSHORT Tactic_index;

	/* Get possible targets */
	Possible_targets = Get_close_range_targets(Part);

	/* Any ? */
	if (Possible_targets)
	{
		/* Yes -> Any specific target given ? */
		if (Part->Target_part)
		{
			/* Yes -> Calculate tactic index */
			Tactic_index = (Part->Target_part->Tactical_Y *
			 NR_TACTICAL_COLUMNS) + Part->Target_part->Tactical_X;

			/* Is this target in range ? */
			if (Possible_targets & (1L << Tactic_index))
			{
				/* Yes -> Attack it */
				Part->Current_action = CLOSE_RANGE_COMACT;
				Part->Target.Attack_target_data.Target_square_index		= Tactic_index;
				Part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_item_slot_index;
			}
			else
			{
				/* No -> Just select a target */
				Tactic_index = Choose_monster_target(Possible_targets);

				/* Attack it */
				Part->Current_action = CLOSE_RANGE_COMACT;
				Part->Target.Attack_target_data.Target_square_index		= Tactic_index;
				Part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_item_slot_index;
			}
		}
		else
		{
			/* No -> Just select a target */
			Tactic_index = Choose_monster_target(Possible_targets);

			/* Attack it */
			Part->Current_action = CLOSE_RANGE_COMACT;
			Part->Target.Attack_target_data.Target_square_index		= Tactic_index;
			Part->Target.Attack_target_data.Weapon_item_slot_index	= Weapon_item_slot_index;
		}
	}
	else
	{
		/* No -> Any specific target given ? */
		if (Part->Target_part)
		{
			/* Yes -> Move towards it */
			Make_offensive_move
			(
				Part,
				Part->Target_part->Tactical_X
			);
		}
		else
		{
			/* No -> Just select a target */
			Tactic_index = Choose_monster_target(Get_party_targets());

			/* Move towards it */
			Make_offensive_move
			(
				Part,
				Tactic_index % NR_TACTICAL_COLUMNS
			);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_offensive_move
 * FUNCTION  : Make an offensive move.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.09.95 12:27
 * LAST      : 20.09.95 12:48
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Target_X - Target X-coordinate / 0xFFFF = no target.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_offensive_move(struct Combat_participant *Part, UNSHORT Target_X)
{
	UNLONG Movement_range_mask;
	UNSHORT X, Y;
	UNSHORT New_X;
	UNSHORT Tactic_index;
	UNSHORT Step_index;
	UNSHORT Nr_steps;

	/* Reset number of moves */
	Part->Target.Move_target_data.Nr_moves = 0;

	/* Get coordinates */
	X = Part->Tactical_X;
	Y = Part->Tactical_Y;

	/* Calculate tactic index */
	Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

	/* Get movement range */
	Movement_range_mask = Get_movement_range(Part) &
	 ~(Get_occupied_move_targets(Part));

	/* Get number of moves of participant */
	Nr_steps = Get_nr_moves(Part);

	/* Move */
	Step_index = 0;
	while (Step_index < Nr_steps)
	{
		/* Already as far down as possible ? */
		if (Y >= 3)
		{
			/* Yes -> Is a horizontal move possible ? */
			New_X = Make_offensive_horizontal_move
			(
				X,
				Y,
				Target_X,
				Movement_range_mask
			);
			if (X != New_X)
			{
				/* Yes -> Make horizontal move */
				X = New_X;
			}
			else
			{
				/* No -> Break */
				break;
			}
		}
		else
		{
			/* No -> Moving down possible ? */
			if (Movement_range_mask &
			 (1L << (Tactic_index + NR_TACTICAL_COLUMNS)))
			{
				/* Yes -> Move down */
				Y++;
			}
			else
			{
				/* No -> Is a diagonal move down possible ? */
				New_X = Make_offensive_horizontal_move
				(
					X,
					Y + 1,
					Target_X,
					Movement_range_mask
				);
				if (X != New_X)
				{
					/* Yes -> Move diagonally */
					X = New_X;
					Y++;
				}
				else
				{
					/* No -> Is a horizontal move possible ? */
					New_X = Make_offensive_horizontal_move
					(
						X,
						Y,
						Target_X,
						Movement_range_mask
					);
					if (X != New_X)
					{
						/* Yes -> Move horizontally */
						X = New_X;
					}
					else
					{
						/* No -> Exit */
						break;
					}
				}
			}
		}

		/* No turning back! */
		Movement_range_mask &= ~(1L << Tactic_index);

		/* Calculate new tactical square index */
		Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

		/* Store */
		Part->Target.Move_target_data.Target_square_indices[Step_index] = Tactic_index;

		/* Count up */
		Step_index++;
	}

	/* Were any moves entered ? */
	if (Step_index)
	{
		/* Yes -> Set action */
		Part->Current_action = MOVE_COMACT;

		/* Store number of moves */
		Part->Target.Move_target_data.Nr_moves = Step_index;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_offensive_horizontal_move
 * FUNCTION  : Make an offensive horizontal move.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.09.95 12:04
 * LAST      : 20.09.95 12:04
 *	INPUTS    : UNSHORT Tactical_X - Current X-coordinate.
 *             UNSHORT Tactical_Y - Current Y-coordinate.
 *             UNSHORT Target_X - Target X-coordinate / 0xFFFF = no target.
 *             UNLONG Movement_mask - Possible movement mask.
 * RESULT    : UNSHORT : New X-coordinate.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Make_offensive_horizontal_move(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNSHORT Target_X, UNLONG Movement_mask)
{
	/* Any target given ? */
	if (Target_X != 0xFFFF)
	{
		/* Yes -> Left or right ? */
		if (Target_X < Tactical_X)
		{
			/* Left -> Possible ? */
			if (Check_move_left(Tactical_X, Tactical_Y, Movement_mask))
			{
				/* Yes -> Move left */
				Tactical_X--;
			}
		}
		else
		{
			/* Right -> Possible ? */
			if (Check_move_right(Tactical_X, Tactical_Y, Movement_mask))
			{
				/* Yes -> Move right */
				Tactical_X++;
			}
		}
	}
	else
	{
		/* Left or right ? */
		if ((rand() % 100) >= 50)
		{
			/* Left -> Possible ? */
			if (Check_move_left(Tactical_X, Tactical_Y, Movement_mask))
			{
				/* Yes -> Move left */
				Tactical_X--;
			}
			else
			{
				/* No -> Right possible ? */
				if (Check_move_right(Tactical_X, Tactical_Y, Movement_mask))
				{
					/* Yes -> Move right */
					Tactical_X++;
				}
			}
		}
		else
		{
			/* Right -> Possible ? */
			if (Check_move_right(Tactical_X, Tactical_Y, Movement_mask))
			{
				/* Yes -> Move right */
				Tactical_X++;
			}
			else
			{
				/* No -> Left possible ? */
				if (Check_move_left(Tactical_X, Tactical_Y, Movement_mask))
				{
					/* Yes -> Move left */
					Tactical_X--;
				}
			}
		}
	}

	return Tactical_X;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_targets_within_movement_range
 * FUNCTION  : Get all close-range targets within a participant's movement
 *              range.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.09.95 11:52
 * LAST      : 20.09.95 11:52
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNLONG : Target mask.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_targets_within_movement_range(struct Combat_participant *Part)
{
	UNLONG Movement_range_mask;
	UNLONG Targets_in_range_mask;
	UNSHORT Tactic_index;
	UNSHORT X, Y;

	/* Get movement range */
	Movement_range_mask = Get_movement_range(Part) &
	 ~(Get_occupied_move_targets(Part));

	/* Check all squares */
	Targets_in_range_mask = 0;
	for (Y=0;Y<NR_TACTICAL_ROWS;Y++)
	{
		for (X=0;X<NR_TACTICAL_COLUMNS;X++)
		{
			/* Calculate tactical index for these coordinates */
			Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

			/* Is this square within movement range ? */
			if (Movement_range_mask & (1L << Tactic_index))
			{
				/* Yes -> Get all close-range targets for this position */
				Targets_in_range_mask |= Do_close_range_targets
				(
					X,
					Y,
					Part->Type
				);
			}
		}
	}

	return Targets_in_range_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_defensive_monster_action
 * FUNCTION  : Select a defensive monster action.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 14:22
 * LAST      : 20.09.95 12:25
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function may also be called for panicking monsters.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_defensive_monster_action(struct Combat_participant *Part)
{
	UNLONG Movement_range_mask;
	UNSHORT Nr_steps;
	UNSHORT X, Y;
	UNSHORT Tactic_index;
	UNSHORT Step_index;
	UNSHORT New_X;

	/* Clear monster action */
	Part->Current_action = NO_COMACT;

	/* Is the monster in the top row / can flee ? */
	if ((Part->Tactical_Y == 0) &&
	 !(Get_conditions(Part->Char_handle) & FLEE_MASK))
	{
		/* Yes -> Flee */
		Part->Current_action = FLEE_COMACT;
	}
	else
	{
		/* No -> Get movement range */
		Movement_range_mask = Get_movement_range(Part) &
		 ~(Get_occupied_move_targets(Part));

		/* Any ? */
		if (Movement_range_mask)
		{
			/* Yes -> Clear number of moves */
			Part->Target.Move_target_data.Nr_moves = 0;

			/* Get number of moves of participant */
			Nr_steps = Get_nr_moves(Part);

			/* Get coordinates of monster */
			X = Part->Tactical_X;
			Y = Part->Tactical_Y;

			/* Calculate tactical square index */
			Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

			/* Try to retreat */
			Step_index = 0;
			while (Step_index < Nr_steps)
			{
				/* In the top row ? */
				if (!Y)
				{
					/* Yes -> Exit */
					break;
				}
				else
				{
					/* No -> Moving up possible ? */
					if (Movement_range_mask &
					 (1L << (Tactic_index - NR_TACTICAL_COLUMNS)))
					{
						/* Yes -> Move up */
						Y--;
					}
					else
					{
						/* No -> Is a diagonal move up possible ? */
						New_X = Make_defensive_horizontal_move
						(
							X,
							Y - 1,
							Movement_range_mask
						);
						if (X != New_X)
						{
							/* Yes -> Move diagonally */
							X = New_X;
							Y--;
						}
						else
						{
							/* No -> Is a horizontal move possible ? */
							New_X = Make_defensive_horizontal_move
							(
								X,
								Y,
								Movement_range_mask
							);
							if (X != New_X)
							{
								/* Yes -> Move horizontally */
								X = New_X;
							}
							else
							{
								/* No -> Exit */
								break;
							}
						}
					}
				}

				/* No turning back! */
				Movement_range_mask &= ~(1L << Tactic_index);

				/* Calculate new tactical square index */
				Tactic_index = (Y * NR_TACTICAL_COLUMNS) + X;

				/* Store */
				Part->Target.Move_target_data.Target_square_indices[Step_index] = Tactic_index;

				/* Count up */
				Step_index++;
			}

			/* Were any moves entered ? */
			if (Step_index)
			{
				/* Yes -> Set action */
				Part->Current_action = MOVE_COMACT;

				/* Store number of moves */
				Part->Target.Move_target_data.Nr_moves = Step_index;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_defensive_horizontal_move
 * FUNCTION  : Make a defensive horizontal move.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 14:09
 * LAST      : 18.09.95 14:09
 *	INPUTS    : UNSHORT Tactical_X - Current X-coordinate.
 *             UNSHORT Tactical_Y - Current Y-coordinate.
 *             UNLONG Movement_mask - Possible movement mask.
 * RESULT    : UNSHORT : New X-coordinate.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Make_defensive_horizontal_move(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNLONG Movement_mask)
{
	/* Left or right ? */
	if ((rand() % 100) >= 50)
	{
		/* Left -> Possible ? */
		if (Check_move_left(Tactical_X, Tactical_Y, Movement_mask))
		{
			/* Yes -> Move left */
			Tactical_X--;
		}
		else
		{
			/* No -> Right possible ? */
			if (Check_move_right(Tactical_X, Tactical_Y, Movement_mask))
			{
				/* Yes -> Move right */
				Tactical_X++;
			}
		}
	}
	else
	{
		/* Right -> Possible ? */
		if (Check_move_right(Tactical_X, Tactical_Y, Movement_mask))
		{
			/* Yes -> Move right */
			Tactical_X++;
		}
		else
		{
			/* No -> Left possible ? */
			if (Check_move_left(Tactical_X, Tactical_Y, Movement_mask))
			{
				/* Yes -> Move left */
				Tactical_X--;
			}
		}
	}

	return Tactical_X;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_move_left
 * FUNCTION  : Check if move to the left is possible.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 14:02
 * LAST      : 18.09.95 14:02
 *	INPUTS    : UNSHORT Tactical_X - Current X-coordinate.
 *             UNSHORT Tactical_Y - Current Y-coordinate.
 *             UNLONG Movement_mask - Possible movement mask.
 * RESULT    : BOOLEAN : Move left is possible.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_move_left(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNLONG Movement_mask)
{
	BOOLEAN Move = FALSE;
	UNSHORT Tactical_index;

	/* Calculate tactical square index */
	Tactical_index = (Tactical_Y * NR_TACTICAL_COLUMNS) + Tactical_X;

	/* As left as possible ? */
	if (Tactical_X > 0)
	{
		/* No -> Movement possible ? */
		if (Movement_mask & (1L << (Tactical_index - 1)))
		{
			/* Yes -> Move is possible */
			Move = TRUE;
		}
	}

	return Move;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_move_right
 * FUNCTION  : Check if move to the right is possible.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 13:59
 * LAST      : 18.09.95 13:59
 *	INPUTS    : UNSHORT Tactical_X - Current X-coordinate.
 *             UNSHORT Tactical_Y - Current Y-coordinate.
 *             UNLONG Movement_mask - Possible movement mask.
 * RESULT    : BOOLEAN : Move right is possible.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_move_right(UNSHORT Tactical_X, UNSHORT Tactical_Y,
 UNLONG Movement_mask)
{
	BOOLEAN Move = FALSE;
	UNSHORT Tactical_index;

	/* Calculate tactical square index */
	Tactical_index = (Tactical_Y * NR_TACTICAL_COLUMNS) + Tactical_X;

	/* As right as possible ? */
	if (Tactical_X < (NR_TACTICAL_COLUMNS - 1))
	{
		/* No -> Movement possible ? */
		if (Movement_mask & (1L << (Tactical_index + 1)))
		{
			/* Yes -> Move is possible */
			Move = TRUE;
		}
	}

	return Move;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_and_equip_new_weapon
 * FUNCTION  : Search and equip a new weapon for a combat participant.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.09.95 22:27
 * LAST      : 19.09.95 13:21
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : UNSHORT : Item type of new weapon / 0xFFFF = nothing found.
 * BUGS      : No known.
 * NOTES     : - The item type is needed to determine whether a long- or
 *              close-range attack should follow.
 *             - This function assumes the hands (and perhaps tail) of the
 *             participant are free, unless a long-range weapon without ammo
 *             is carried in the right hand.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_and_equip_new_weapon(struct Combat_participant *Part)
{
	struct Character_data *Char;
	struct Item_packet Trans_packet;
	struct Item_data *Weapon_item_data;
	struct Item_data *Ammo_item_data;
	UNSHORT Found_weapon_slot_index;
	UNSHORT Found_ammo_slot_index;
	UNSHORT Target_slot_index;
	UNSHORT Weapon_item_type;
	UNSHORT i, j;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

	/* Any item in the right hand ? */
	if (!Packet_empty(&(Char->Body_items[RIGHT_HAND - 1])))
	{
		/* Yes -> Drop it */
		Store_item_in_Apres_pool(&(Char->Body_items[RIGHT_HAND - 1]));

		Remove_item
		(
			Part->Char_handle,
			RIGHT_HAND,
			1
		);
	}

	/* Search all backpack slots for a weapon */
	Weapon_item_type 			= 0xFFFF;
	Found_weapon_slot_index	= 0xFFFF;
	for (i=0;i<ITEMS_PER_CHAR;i++)
	{
		/* Anything there ? */
		if (!Packet_empty(&(Char->Backpack_items[i])))
		{
			/* Yes -> Broken ? */
			if (!(Char->Backpack_items[i].Flags & BROKEN_ITEM))
			{
				/* No -> Get item data */
				Weapon_item_data = Get_item_data(&(Char->Backpack_items[i]));

				/* Is it a close-range weapon ? */
				if (Weapon_item_data->Type == CLOSE_RANGE_IT)
				{
					/* Yes -> Found a weapon! */
					Found_weapon_slot_index	= i;
					Weapon_item_type	 				= Weapon_item_data->Type;

					Free_item_data();
					break;
				}

				/* Is it a long-range weapon ? */
				if (Weapon_item_data->Type == LONG_RANGE_IT)
				{
					/* Yes -> Any ammo required ? */
					if (Weapon_item_data->Ammo_ID)
					{
						/* Yes -> Search all backpack slots for the right ammo */
						Found_ammo_slot_index = 0xFFFF;
						for (j=0;j<ITEMS_PER_CHAR;j++)
						{
							/* Anything there ? */
							if (!Packet_empty(&(Char->Backpack_items[j])))
							{
								/* Yes -> Broken ? */
								if (!(Char->Backpack_items[j].Flags & BROKEN_ITEM))
								{
									/* No -> Get item data */
									Ammo_item_data = Get_item_data(&(Char->Backpack_items[j]));

									/* Is ammo / the right ammo ? */
									if ((Ammo_item_data->Type == AMMO_IT) &&
									 (Ammo_item_data->Ammo_ID == Weapon_item_data->Ammo_ID))
									{
										/* Yes -> Found the ammo! */
										Found_ammo_slot_index = j;

										Free_item_data();
										break;
									}

									Free_item_data();
								}
							}
						}

						/* Found any ammo ? */
						if (Found_ammo_slot_index != 0xFFFF)
						{
							/* Yes -> Determine target slot index */
							Target_slot_index = Find_target_body_slot_for_item(Ammo_item_data);

							/* Is the target slot free ? */
							if (Target_slot_index == 0xFFFF)
							{
								/* Yes -> Remove ammo from backpack */
								memcpy
								(
									(UNBYTE *) &Trans_packet,
									(UNBYTE *) &(Char->Backpack_items[Found_ammo_slot_index]),
									sizeof(struct Item_packet)
								);
								Trans_packet.Quantity = 1;

								Remove_item
								(
									Part->Char_handle,
									ITEMS_ON_BODY + Found_ammo_slot_index + 1,
									1
								);

								/* Equip ammo */
								Add_item_to_body
								(
									Part->Char_handle,
									&Trans_packet,
									Target_slot_index
								);

								/* Found a weapon! */
								Found_weapon_slot_index	= i;
								Weapon_item_type	 		= Weapon_item_data->Type;

								Free_item_data();
								break;
  							}
							else
							{
								/* No */
								Free_item_data();
							}
						}
					}
					else
					{
						/* No -> Found a weapon! */
						Found_weapon_slot_index	= i;
						Weapon_item_type	 		= Weapon_item_data->Type;

						Free_item_data();
						break;
					}
				}

				Free_item_data();
			}
		}
	}

	/* Found any weapon ? */
	if (Found_weapon_slot_index != 0xFFFF)
	{
		/* Yes -> Determine target slot index */
		Weapon_item_data = Get_item_data(&(Char->Backpack_items[Found_weapon_slot_index]));
		Target_slot_index = Find_target_body_slot_for_item(Weapon_item_data);
		Free_item_data();

		/* Is the target slot free ? */
		if (Target_slot_index == 0xFFFF)
		{
			/* Yes -> Remove weapon from backpack */
			memcpy
			(
				(UNBYTE *) &Trans_packet,
				(UNBYTE *) &(Char->Backpack_items[Found_weapon_slot_index]),
				sizeof(struct Item_packet)
			);
			Trans_packet.Quantity = 1;

			Remove_item
			(
				Part->Char_handle,
				ITEMS_ON_BODY + Found_weapon_slot_index + 1,
				1
			);

			/* Equip weapon */
			Add_item_to_body
			(
				Part->Char_handle,
				&Trans_packet,
				Target_slot_index
			);
		}
		else
		{
			/* No */
			Weapon_item_type = 0xFFFF;
		}
	}
	else
	{
		/* No -> Can this participant do any damage ? */
		if (Get_damage(Part->Char_handle))
		{
			/* Yes -> Fake a close-range attack */
			Weapon_item_type = CLOSE_RANGE_IT;
		}
		else
		{
			/* No -> Disable close- and long-range capabilities */
			Part->Capabilities &= ~(PART_CLOSE_CAP | PART_LONG_CAP);
		}
	}

	MEM_Free_pointer(Part->Char_handle);

	return Weapon_item_type;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Choose_monster_target
 * FUNCTION  : Choose a target for a monster.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 14:47
 * LAST      : 19.09.95 14:47
 * INPUTS    : UNLONG Possible_targets - Possible target bitlist.
 * RESULT    : UNSHORT : Tactical square index.
 * BUGS      : No known.
 * NOTES     : - This function is also used by Try_crazy_move and
 *              Try_crazy_attack.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Choose_monster_target(UNLONG Possible_targets)
{
	UNSHORT Nr_targets;
	UNSHORT Chosen_target;
	UNSHORT Tactic_index = 0xFFFF;
	UNSHORT i;

	/* Count possible targets */
	Nr_targets = 0;
	for (i=0;i<(NR_TACTICAL_ROWS * NR_TACTICAL_COLUMNS);i++)
	{
		/* Is this a possible target ? */
		if (Possible_targets & (1L << i))
		{
			/* Yes -> Count up */
			Nr_targets++;
		}
	}

	/* Are there any targets ? */
	if (Nr_targets)
	{
		/* Yes -> Choose one */
		Chosen_target = (rand() % Nr_targets);

		/* Find the tactical index */
		for (i=0;i<(NR_TACTICAL_ROWS * NR_TACTICAL_COLUMNS);i++)
		{
			/* Is this a possible target ? */
			if (Possible_targets & (1L << i))
			{
				/* Yes -> Is this the Chosen One ? */
				if (Chosen_target)
				{
					/* No -> Count down */
					Chosen_target--;
				}
				else
				{
					/* Yes -> Choose it */
					Tactic_index = i;
					break;
				}
			}
		}
	}

	return Tactic_index;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_if_long_range_weapon_has_ammo
 * FUNCTION  : Check if a long-range weapon has the right ammo, if any.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.09.95 18:19
 * LAST      : 19.09.95 18:19
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 *             UNSHORT Weapon_item_slot_index - Index of weapon item slot
 *              (1...9).
 * RESULT    : BOOLEAN : Weapon has ammo.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_if_long_range_weapon_has_ammo(struct Combat_participant *Part,
 UNSHORT Weapon_item_slot_index)
{
	struct Character_data *Char;
	struct Item_data *Weapon_item_data;
	struct Item_data *Ammo_item_data;
	BOOLEAN Result = FALSE;
	UNSHORT i;

	/* Get character data */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);

	/* Get weapon's item data */
	Weapon_item_data = Get_item_data(&(Char->Body_items[Weapon_item_slot_index - 1]));

	/* Is it a long-range weapon ? */
	if (Weapon_item_data->Type == LONG_RANGE_IT)
	{
		/* Yes -> Any ammo required ? */
		if (Weapon_item_data->Ammo_ID)
		{
			/* Yes -> Search all body slots for the right ammo */
			for (i=0;i<ITEMS_ON_BODY;i++)
			{
				/* Anything there ? */
				if (!Packet_empty(&(Char->Body_items[i])))
				{
					/* Yes -> Broken ? */
					if (!(Char->Body_items[i].Flags & BROKEN_ITEM))
					{
						/* No -> Get item data */
						Ammo_item_data = Get_item_data(&(Char->Body_items[i]));

						/* Is ammo / the right ammo ? */
						if ((Ammo_item_data->Type == AMMO_IT) &&
						 (Ammo_item_data->Ammo_ID == Weapon_item_data->Ammo_ID))
						{
							/* Yes -> Found the right ammo! */
							Result = TRUE;

							Free_item_data();
							break;
						}

						Free_item_data();
					}
				}
			}
		}
		else
		{
			/* No -> OK! */
			Result = TRUE;
		}
	}

	Free_item_data();
	MEM_Free_pointer(Part->Char_handle);

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Default_decider
 * FUNCTION  : Default monster logic flee-or-attack decision function.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.09.95 10:08
 * LAST      : 24.10.95 16:48
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : BOOLEAN : TRUE (attack) or FALSE (flee).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Default_decider(struct Combat_participant *Part)
{
	struct Character_data *Char;
	SISHORT Global_danger;
	SISHORT Local_danger;
	SISHORT Danger;
	SISHORT Courage;

	/* Calculate global danger */
	Global_danger = 100 - ((Count_remaining_monsters() * 100) / Nr_monsters);

	/* Calculate local danger */
	Local_danger = 100 - ((Get_LP(Part->Char_handle) * 100) /
	 Get_max_LP(Part->Char_handle));

	/* Calculate mean danger */
	Danger = (Global_danger + Local_danger) / 2;

	/* Get participant's courage */
	Char = (struct Character_data *) MEM_Claim_pointer(Part->Char_handle);
	Courage = Char->Courage;
	MEM_Free_pointer(Part->Char_handle);

	/* Decide */
	if (Danger >= Courage)
		return FALSE;
	else
		return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Rinrii_decider
 * FUNCTION  : Special monster logic flee-or-attack decision function :
 *              only flees if one or more monsters were killed.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.10.95 22:36
 * LAST      : 23.10.95 21:14
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : BOOLEAN : TRUE (attack) or FALSE (flee).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Rinrii_decider(struct Combat_participant *Part)
{
	/* Are all monsters still there ? */
	if (Nr_monsters == Count_remaining_monsters())
	{
		/* Yes -> Attack */
		return TRUE;
	}
	else
	{
		/* No -> Flee */
		return FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Kritha_decider
 * FUNCTION  : Special monster logic flee-or-attack decision function :
 *              only flees if there are less monsters than party members.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.10.95 23:29
 * LAST      : 08.10.95 12:14
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : BOOLEAN : TRUE (attack) or FALSE (flee).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Kritha_decider(struct Combat_participant *Part)
{
	SISHORT Health;

	/* Are there less monsters than party members ? */
	if (Count_remaining_monsters() >= Count_remaining_party_members())
	{
		/* Yes -> Calculate relative health */
		Health = (Get_LP(Part->Char_handle) * 100) /
		 Get_max_LP(Part->Char_handle);

		/* Move back if danger is too high */
		if (Health >= (Part->Tactical_Y + 1) * 25)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		/* No -> Flee */
		return FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Mage_decider
 * FUNCTION  : Special monster logic flee-or-attack decision function :
 *              only flees if it is in close-range of the party.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.10.95 23:18
 * LAST      : 15.10.95 23:18
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : BOOLEAN : TRUE (attack) or FALSE (flee).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Mage_decider(struct Combat_participant *Part)
{
	/* Any SP left ? */
	if (Get_SP(Part->Char_handle))
	{
		/* Yes -> Too far in front ? */
		if (Part->Tactical_Y >= 2)
		{
			/* Yes -> Retreat */
			return FALSE;
		}
		else
		{
			/* No -> Stay and attack */
			return TRUE;
		}
	}
	else
	{
		/* No -> Use normal tactic */
		return Default_decider(Part);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Kizz_target_select
 * FUNCTION  : Select the weakest target.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.95 17:52
 * LAST      : 27.10.95 17:52
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : struct Combat_participant * : Pointer to target participant.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Combat_participant *
Kizz_target_select(struct Combat_participant *Part)
{
	struct Combat_participant *Current_part;
	struct Combat_participant *Target_part;
	UNSHORT Strength;
	UNSHORT Min_strength;
	UNSHORT i, j;

	/* Scan the combat matrix */
	Target_part		= NULL;
	Min_strength	= 65535;
	for (i=0;i<NR_TACTICAL_ROWS;i++)
	{
		for (j=0;j<NR_TACTICAL_COLUMNS;j++)
		{
			/* Anyone there ? */
			Current_part = Combat_matrix[i][j].Part;
			if (Current_part)
			{
				/* Yes -> Enemy ? */
				if (Current_part->Type != Part->Type)
				{
					/* Yes -> Get strength */
					Strength = Get_LP(Current_part->Char_handle);

					/* Weaker than current minimum ? */
					if (Strength < Min_strength)
					{
						/* Yes -> This is the new minimum */
						Min_strength	= Strength;
						Target_part		= Current_part;
					}
				}
			}
		}
	}

	return Target_part;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : AI_target_select
 * FUNCTION  : Do not select Tom.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.95 17:56
 * LAST      : 27.10.95 18:00
 *	INPUTS    : struct Combat_participant *Part - Pointer to participant data.
 * RESULT    : struct Combat_participant * : Pointer to target participant.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct Combat_participant *
AI_target_select(struct Combat_participant *Part)
{
	struct Combat_participant *Target_part = NULL;
	UNLONG Possible_targets;
	UNSHORT Tactic_index;
	UNSHORT X, Y;

	/* Get possible targets */
	Possible_targets = Get_long_range_targets(Part);

	/* Get Tom's coordinates */
	X = Party_parts[0].Tactical_X;
	Y = Party_parts[0].Tactical_Y;

	/* Is Tom present in the combat matrix ? */
	if ((X != 0xFFFF) && (Y != 0xFFFF))
	{
		/* Yes -> Remove him from the possible targets */
		Possible_targets &= ~(1 << ((Y * NR_TACTICAL_COLUMNS) + X));
	}

	/* Any ? */
	if (Possible_targets)
	{
		/* Yes -> Just select a target */
		Tactic_index = Choose_monster_target(Possible_targets);

		/* Get target coordinates */
		X = Tactic_index % NR_TACTICAL_COLUMNS;
		Y = Tactic_index / NR_TACTICAL_COLUMNS;

		/* Get participant data */
		Target_part = Combat_matrix[Y][X].Part;
	}

	return Target_part;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Warniak_1_attack_result
 * FUNCTION  : Cause condition as the result of an attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.10.95 23:12
 * LAST      : 07.10.95 23:12
 *	INPUTS    : struct Combat_participant *Attacker_part - Pointer to
 *              attacker's participant data.
 *	            struct Combat_participant *Defender_part - Pointer to
 *              defender's participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Warniak_1_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part)
{
	/* Chance ? */
	if (Probe(10, 100))
	{
		/* Yes -> Cause panic */
		Set_condition(Defender_part->Char_handle, PANICKED);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Warniak_2_attack_result
 * FUNCTION  : Cause condition as the result of an attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.10.95 23:20
 * LAST      : 07.10.95 23:20
 *	INPUTS    : struct Combat_participant *Attacker_part - Pointer to
 *              attacker's participant data.
 *	            struct Combat_participant *Defender_part - Pointer to
 *              defender's participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Warniak_2_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part)
{
	/* Chance ? */
	if (Probe(10, 100))
	{
		/* Yes -> Choose random condition */
		switch (rand() % 2)
		{
			case 0:
			{
				/* Cause panic */
				Set_condition(Defender_part->Char_handle, PANICKED);
				break;
			}
			case 1:
			{
				/* Cause poisoning */
				Set_condition(Defender_part->Char_handle, POISONED);
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Warniak_3_attack_result
 * FUNCTION  : Cause condition as the result of an attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.10.95 23:21
 * LAST      : 07.10.95 23:21
 *	INPUTS    : struct Combat_participant *Attacker_part - Pointer to
 *              attacker's participant data.
 *	            struct Combat_participant *Defender_part - Pointer to
 *              defender's participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Warniak_3_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part)
{
	/* Chance ? */
	if (Probe(10, 100))
	{
		/* Yes -> Choose random condition */
		switch (rand() % 3)
		{
			case 0:
			{
				/* Cause disease */
				Set_condition(Defender_part->Char_handle, DISEASED);
				break;
			}
			case 1:
			{
				/* Cause insanity */
				Set_condition(Defender_part->Char_handle, INSANE);
				break;
			}
			case 2:
			{
				/* Cause poisoning */
				Set_condition(Defender_part->Char_handle, POISONED);
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : AI_attack_result
 * FUNCTION  : Kill as the result of an attack.
 * FILE      : MONLOGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.10.95 18:01
 * LAST      : 27.10.95 18:01
 *	INPUTS    : struct Combat_participant *Attacker_part - Pointer to
 *              attacker's participant data.
 *	            struct Combat_participant *Defender_part - Pointer to
 *              defender's participant data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
AI_attack_result(struct Combat_participant *Attacker_part,
 struct Combat_participant *Defender_part)
{
	UNSHORT Minimum_members;

	/* Calculate how many members should be left */
	Minimum_members = max(1, (Count_party_members() - 2));

	/* Have enough members been killed ? */
	if (Count_alive_party_members() <= Minimum_members)
	{
		/* Yes -> End of combat */
		Combat_status = COMBAT_ENDED;

		/* Exit combat screen */
		End_combat_flag = TRUE;
	}
}

