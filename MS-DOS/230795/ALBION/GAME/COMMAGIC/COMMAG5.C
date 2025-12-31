/************
 * NAME     : COMMAG5.C
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

