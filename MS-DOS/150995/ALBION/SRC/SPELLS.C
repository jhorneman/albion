/************
 * NAME     : SPELLS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 4-5-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : SPELLS.H
 ************/

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
#include <MAP.H>
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

//void Do_C0_Spell_2(UNSHORT Member_nr, UNSHORT Strength);
void Do_C0_Spell_9(UNSHORT Member_nr, UNSHORT Strength);

void Do_C1_Spell_1(UNSHORT Member_nr, UNSHORT Strength);

/* global variables */

#if FALSE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_2_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 17:40
 * LAST      : 02.05.95 17:40
 * INPUTS    : UNSHORT Strength - Strength of spell (1...100).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
C0_Spell_2_handler(UNSHORT Strength)
{
	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_2);
}

void
Do_C0_Spell_2(UNSHORT Member_nr, UNSHORT Strength)
{
	struct Character_data *Char;
	struct Item_data *Item_data;
	MEM_HANDLE Char_handle;
	UNSHORT Counter;
	UNSHORT i;

	/* Get character data */
	Char_handle = Party_char_handles[Member_nr - 1];
	Char = (struct Character_data *) MEM_Claim_pointer(Char_handle);

	/* Check body items */
	Counter = 0;
	for (i=0;i<ITEMS_ON_BODY;i++)
	{
		/* Is this packet empty ? */
		if (!Packet_empty(&(Char->Body_items[i])))
		{
			/* No -> Get item data */
			Item_data = Get_item_data(&(Char->Body_items[i]));

			/* Is cursed / organic item ? */
			if ((Char->Body_items[i].Flags & CURSED) &&
			 (Item_data->Flags & METALLIC))
			{
				/* Yes -> De-curse */
				Char->Body_items[i].Flags &= ~CURSED;

				/* Show */

				/* Count up */
				Counter++;
			}
			Free_item_data();
		}
	}
	MEM_Free_pointer(Char_handle);

	/* Any objects de-cursed ? */
	if (!Counter)
	{
		/* No -> Tell player */
	}
}
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_9_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.04.95 14:50
 * LAST      : 28.04.95 15:11
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
	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C0_Spell_9);

	/* Wait for magical animations to end */
}

void
Do_C0_Spell_9(UNSHORT Member_nr, UNSHORT Strength)
{
	MEM_HANDLE Char_handle;
	UNSHORT LP;

	/* Heal 25 % */
	Char_handle = Party_char_handles[Member_nr - 1];
	LP = Get_LP(Char_handle);
	Set_LP(Char_handle, LP + max((LP * 25) / 100, 1));

	/* Sparkle */
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_16_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C0_Spell_16_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_17_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C0_Spell_17_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_18_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C0_Spell_18_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C0_Spell_19_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C0_Spell_19_handler(UNSHORT Strength)
{
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
 * LAST      : 31.08.95 12:48
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
	/* Do spell for each target */
	Do_all_magic_party_targets(Strength, Do_C1_Spell_1);

	/* Wait for magical animations to end */
}

void
Do_C1_Spell_1(UNSHORT Member_nr, UNSHORT Strength)
{
	MEM_HANDLE Char_handle;
	UNSHORT LP;

	/* Heal 25 % */
	Char_handle = Party_char_handles[Member_nr - 1];
	LP = Get_LP(Char_handle);
	Set_LP(Char_handle, LP + max((LP * 25) / 100, 1));

	/* Sparkle */
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
		/* Yes -> Enter automap with extended functions */
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
C1_Spell_3_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_5_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C1_Spell_5_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C1_Spell_7_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C1_Spell_11_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C1_Spell_11_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C2_Spell_7_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C2_Spell_7_handler(UNSHORT Strength)
{
}

#if FALSE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_2_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C5_Spell_2_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_4_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C5_Spell_4_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_6_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
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
C5_Spell_6_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_8_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C5_Spell_8_handler(UNSHORT Strength)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : C5_Spell_9_handler
 * FUNCTION  : Spell handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
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
C5_Spell_9_handler(UNSHORT Strength)
{
}
#endif

#if FALSE

UNSHORT Enter_C0_Spell_1_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 UNSHORT *Target_item_slot_index_ptr);
UNSHORT Charge_organic_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet);

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Enter_C0_Spell_1_target
 * FUNCTION  : Spell target entry exception handler.
 * FILE      : SPELLS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 17:46
 * LAST      : 02.05.95 17:46
 * INPUTS    : UNSHORT Class_nr - Spell class number (0...SPELL_CLASSES_MAX - 1).
 *             UNSHORT Spell_nr - Spell number (1...SPELLS_PER_CLASS).
 *             UNSHORT *Target_item_slot_index_ptr - Pointer to target item
 *              slot index.
 * RESULT    : UNSHORT : Target mask.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Enter_C0_Spell_1_target(UNSHORT Class_nr, UNSHORT Spell_nr,
 UNSHORT *Target_item_slot_index_ptr)
{
	UNSHORT Member_index;
	UNSHORT Target_item_slot_index;
	UNSHORT Party_mask = 0;

	/* Select a party member */
	Member_index = Select_party_member(System_text_ptrs[90], NULL, 0);

	/* Any member selected ? */
	if (Member_index && (Member_index != 0xFFFF))
	{
		/* Yes -> Select item */
		Target_item_slot_index =
		 Select_character_item(Party_char_handles[Member_index - 1],
		 System_text_ptrs[91], Charge_organic_item_evaluator);

		/* Any item selected ? */
		if (Target_item_slot_index != 0xFFFF)
		{
			/* Yes -> Build target mask */
			Party_mask = (1 << Member_index);
		}
	}

	return Party_mask;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Charge organic_item_evaluator
 * FUNCTION  : Check if item is organic and can be charged
 *              (item select window evaluator).
 * FILE      : GAME.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.05.95 17:50
 * LAST      : 03.05.95 10:44
 * INPUTS    : struct Character_data *Char - Pointer to character data.
 *             struct Item_packet *Packet - Pointer to item packet.
 * RESULT    : UNSHORT : Message nr / 0 = item is OK.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Charge_organic_item_evaluator(struct Character_data *Char,
 struct Item_packet *Packet)
{
	struct Item_data *Item_data;
	UNSHORT Message_nr;

	/* Get item data */
	Item_data = Get_item_data(Packet);

	/* Magical item ? */
	if (Item_data->Spell_nr)
	{
		/* Yes -> Organic item ? */
		if (!(Item_data->Flags & METALLIC))
		{
			/* Yes -> Maximum charges ? */
			if (Packet->Charges < Item_data->Max_charges)
			{
				/* No -> Item is OK */
				Message_nr = 0;
			}
			else
			{
				/* Yes -> "Maximum charges" */
				Message_nr = 129;
			}
		}
		else
		{
			/* No -> "Item is not organic" */
			Message_nr = 128;
		}
	}
	else
	{
		/* No -> "Item is not magical" */
		Message_nr = 93;
	}

	Free_item_data();

	return Message_nr;
}
#endif

