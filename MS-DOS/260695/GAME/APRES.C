/************
 * NAME     : APRES.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 07-03-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : APRES.H
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <TEXT.H>
#include <GFXFUNC.H>

#include <GAMEVAR.H>
#include <FONT.H>
#include <CONTROL.H>
#include <GRAPHICS.H>
#include <USERFACE.H>
#include <SCROLBAR.H>
#include <APRES.H>
#include <EVELOGIC.H>

/* global variables */

static UNSHORT Nr_apres_items;

static UNLONG Apres_gold_coins;
static UNLONG Apres_food_rations;

/* Change later !!! */
// static MEM_HANDLE Apres_pool_handle;
MEM_HANDLE Apres_pool_handle;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_apres_pool
 * FUNCTION  : Init apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 10:46
 * LAST      : 07.03.95 10:46
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Apres pool was initialized successfully.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_apres_pool(void)
{
	/* Clear counter */
	Nr_apres_items = 0;

	/* Allocate apres pool memory */
	Apres_pool_handle = MEM_Allocate_memory(MAX_APRES_ITEMS *
	 sizeof(struct Item_packet));

	/* Clear apres pool */
	MEM_Clear_memory(Apres_pool_handle);

	/* Clear gold and food */
	Apres_gold_coins = 0;
	Apres_food_rations = 0;

	return(TRUE);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_item_in_apres_pool
 * FUNCTION  : Put item in apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 16:15
 * LAST      : 08.03.95 16:15
 * INPUTS    : struct Item_packet *Source_packet - Pointer to item packet.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_item_in_apres_pool(struct Item_packet *Source_packet)
{
	struct Item_packet *Apres_pool;

	/* Is there room in the apres pool ? */
	if (Nr_apres_items < MAX_APRES_ITEMS)
	{
		/* Yes -> Put item in apres pool */
		Apres_pool = (struct Item_packet *) MEM_Claim_pointer(Apres_pool_handle);

		memcpy((UNBYTE *) &Apres_pool[Nr_apres_items], Source_packet,
		 sizeof(struct Item_packet));

		MEM_Free_pointer(Apres_pool_handle);

		/* Count up */
		Nr_apres_items++;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_gold_in_apres_pool
 * FUNCTION  : Put a number of gold coins in the apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 20:27
 * LAST      : 11.05.95 20:27
 * INPUTS    : UNSHORT Nr_gold_coins - Number of gold coins.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_gold_in_apres_pool(UNSHORT Nr_gold_coins)
{
	/* Add up */
	Apres_gold_coins += (UNLONG) Nr_gold_coins;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Put_food_in_apres_pool
 * FUNCTION  : Put a number of food rations in the apres combat pool.
 * FILE      : APRES.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.05.95 20:28
 * LAST      : 11.05.95 20:28
 * INPUTS    : UNSHORT Nr_food_rations - Number of food rations.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Put_food_in_apres_pool(UNSHORT Nr_food_rations)
{
	/* Add up */
	Apres_food_rations += (UNLONG) Nr_food_rations;
}


