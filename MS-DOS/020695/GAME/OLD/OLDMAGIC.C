
void Do_all_magic_targets(UNSHORT Party_target_mask, UNSHORT Strength,
 void (*Do_spell)(UNSHORT Member_nr, UNSHORT Strength));

UNSHORT Get_spell_targets(UNSHORT Class_nr, UNSHORT Spell_nr);

UNLONG Get_combat_magic_target(UNSHORT *Item_slot_index_ptr);

	/* Clear target mask */
	Magic_target_mask = 0;

	/* Get target data */
	Spell_targets = Get_spell_targets(Selected_class, Selected_spell);

	/* Act depending on target type */
	switch (Spell_targets)
	{
		/* One party member */
		case ONE_FRIEND_TARGET:
		{
			/* Select a party member */
			Member_index = Select_party_member(System_text_ptrs[87], NULL);

			/* Anyone selected ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Build target mask */
				Magic_target_mask = (1 << Member_index);
			}
			break;
		}
		/* All party members */
		case ALL_FRIENDS_TARGET:
		{
			/* Select all party members */
			Magic_target_mask = 0x007e;
			break;
		}
		/* Item */
		case ITEM_TARGET:
		{
			/* Select a party member */
			Member_index = Select_party_member(System_text_ptrs[90], NULL);

			/* Any member selected ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Select item */
				Item_slot_index =
				 Select_character_item(Party_char_handles[Member_index - 1],
				 System_text_ptrs[91], NULL);

				/* Any item selected ? */
				if (Item_slot_index != 0xFFFF)
				{
					/* Yes -> Build target mask */
					Magic_target_mask = (1 << Member_index);
				}
			}
			break;
		}
	}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_all_magic_targets
 * FUNCTION  : Execute a spell on all targeted party members.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.04.95 12:12
 * LAST      : 21.04.95 12:12
 * INPUTS    : UNSHORT Party_target_mask - Targeted party member mask.
 *             UNSHORT Strength - Spell strength.
 *             void (*Do_spell)(UNSHORT Member_nr, UNSHORT Strength) -
 *              Pointer to spell function.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_all_magic_targets(UNSHORT Party_target_mask, UNSHORT Strength,
 void (*Do_spell)(UNSHORT Member_nr, UNSHORT Strength))
{
	struct Character_data *Char;
	UNSHORT i;

	/* Check party */
	for (i=0;i<6;i++)
	{
		/* Anyone there / targeted ? */
		if ((PARTY_DATA.Member_nrs[i]) && (Party_target_mask & (1 << (i + 1))))
		{
			/* Yes -> Is target of spell */
			Subject_char_handle = Party_char_handles[i];

			/* Do spell */
			(Do_spell)(i + 1, Strength);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_spell_targets
 * FUNCTION  : Get a spell's targets.
 * FILE      : MAGIC.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.04.95 14:27
 * LAST      : 03.04.95 14:27
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 * RESULT    : UNSHORT : Spell target bitlist.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_spell_targets(UNSHORT Class_nr, UNSHORT Spell_nr)
{
	struct Spell_data *Spell_data;
	UNSHORT Value;

	Spell_data = (struct Spell_data *) MEM_Claim_pointer(Spell_data_handle);

	Value = (UNSHORT) Spell_data[Class_nr * SPELLS_PER_CLASS +
	 Spell_nr - 1].Target_bits;

	MEM_Free_pointer(Spell_data_handle);

	return(Value);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_combat_magic_target
 * FUNCTION  : Get target of spell in combat.
 * FILE      : TACTICAL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.04.95 13:06
 * LAST      : 25.04.95 13:06
 * INPUTS    : UNSHORT *Item_slot_index_ptr - Pointer to item slot index (if
 *              the spell's target is an item).
 * RESULT    : UNLONG : Target mask (0 = no spell).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_combat_magic_target(UNSHORT *Item_slot_index_ptr)
{
	UNLONG Mask = 0;
	UNSHORT Spell_targets;
	UNSHORT Square_index;
	UNSHORT Member_index;
	UNSHORT Item_slot_index;
	UNSHORT i;

	/* Get spell target data */
	Spell_targets = Get_spell_targets(Selected_class, Selected_spell);

	/* Act depending on target type */
	switch (Spell_targets)
	{
		case ONE_FRIEND_TARGET:
		{
			/* Get party member targets */
			Mask = Get_party_targets();

			/* Any ? */
			if (Mask)
			{
				/* Yes -> Select one */
				Square_index = Select_tactical_square(87,
				 Show_potential_attack_targets, Mask, 0);

				/* Any square selected ? */
				if (Square_index != 0xFFFF)
				{
					/* Yes -> Build target mask */
					Mask = (1 << Square_index);
				}
			}
			break;
		}
		case ROW_FRIENDS_TARGET:
		{
			/* Select a row of party members */
			Square_index = Select_tactical_square(89,
			 Show_potential_row_targets, COMBAT_PARTY_MASK, 0);

			/* Any square selected ? */
			if (Square_index != 0xFFFF)
			{
				/* Yes -> Build target mask */
				Square_index -= (Square_index % NR_TACTICAL_COLUMNS);
				for (i=0;i<NR_TACTICAL_COLUMNS;i++)
				{
					Mask |= (1 << (Square_index + i));
				}
			}
			break;
		}
		case ALL_FRIENDS_TARGET:
		{
			/* Select all party members */
			Mask = COMBAT_PARTY_MASK;
			break;
		}
		case ONE_ENEMY_TARGET:
		{
			/* Get monster targets */
			Mask = Get_monster_targets();

			/* Any ? */
			if (Mask)
			{
				/* Yes -> Select one */
				Square_index = Select_tactical_square(88,
				 Show_potential_attack_targets, Mask, 0);

				/* Any square selected ? */
				if (Square_index != 0xFFFF)
				{
					/* Yes -> Build target mask */
					Mask = (1 << Square_index);
				}
			}
			break;
		}
		case ROW_ENEMIES_TARGET:
		{
			/* Select a row of monsters */
			Square_index = Select_tactical_square(89,
			 Show_potential_row_targets, COMBAT_MONSTER_MASK, 0);

			/* Any square selected ? */
			if (Square_index != 0xFFFF)
			{
				/* Yes -> Build target mask */
				Square_index -= (Square_index % NR_TACTICAL_COLUMNS);
				for (i=0;i<NR_TACTICAL_COLUMNS;i++)
				{
					Mask |= (1 << (Square_index + i));
				}
			}
			break;
		}
		case ALL_ENEMIES_TARGET:
		{
			/* Select all monsters */
			Mask = COMBAT_MONSTER_MASK;
			break;
		}
		case ITEM_TARGET:
		{
			/* Select a party member */
			Member_index = Select_party_member(System_text_ptrs[90], NULL);

			/* Any member selected ? */
			if (Member_index != 0xFFFF)
			{
				/* Yes -> Select item */
				Item_slot_index =
				 Select_character_item(Party_char_handles[Member_index - 1],
				 System_text_ptrs[91], NULL);

				/* Any item selected ? */
				if (Item_slot_index != 0xFFFF)
				{
					/* Yes -> Build target mask */
					Square_index = (Party_parts[Member_index - 1].Tactical_Y *
					 NR_TACTICAL_COLUMNS) + Party_parts[Member_index - 1].Tactical_Y;
					Mask = (1 << Square_index);

					/* Store item slot index */
					*Item_slot_index_ptr = Item_slot_index;
				}
			}
			break;
		}
		case SPECIAL_TARGET:
		{
			/* Search spell exception table */
			i = 0;
			for (;;)
			{
				/* End of spell exception table ? */
				if ((Spell_exceptions[i].Class_nr == 0xFFFF) &&
				 (Spell_exceptions[i].Spell_nr == 0xFFFF))
				{
					/* Yes -> Spell wasn't found */
					break;
				}

				/* Is this the spell ? */
				if ((Spell_exceptions[i].Class_nr == Selected_class) &&
				 (Spell_exceptions[i].Spell_nr == Selected_spell))
				{
					/* Yes -> Call spell exception handler */
					Mask = (Spell_exceptions[i].XSpell_handler)(Selected_class,
					 Selected_spell);

					break;
				}
			}
			break;
		}
	}

	return(Mask);
}

