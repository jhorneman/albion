/************
 * NAME     : NPCSOUND.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 23-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <BBDEF.H>
#include <BBMEM.H>

#include <NPCS.H>
#include <NPCSOUND.H>
#include <MUSIC.H>
#include <MAP.H>

/* defines */

#define MAX_NPC_SOUND_SETS		(7)

/* structure definitions */

/* NPC sound effect */
struct NPC_sound_effect {
	UNSHORT Effect_nr;
	UNSHORT Priority;
	UNSHORT Volume;
	UNSHORT Probability;
	UNSHORT Frequency;
};

/* NPC sound set */
struct NPC_sound_set {
	struct NPC_sound_effect Effects[MAX_NPC_EFFECT_TYPES];
};

/* prototypes */

void Get_NPC_effect_coordinates(UNSHORT NPC_index, SILONG *X_ptr,
 SILONG *Y_ptr);

/* global variables */

static struct NPC_sound_set NPC_sound_sets[MAX_NPC_SOUND_SETS] = {
	{
		{
			{   0, 100, 100, 100, 0 },	/* Permanent	*/
			{   0, 100, 100, 100, 0 },	/* Moving		*/
			{   0, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
	{
		{
			{   0, 100, 100, 100, 0 },	/* Permanent	*/
			{ 151,  50,  50, 100, 0 },	/* Moving		*/
			{  56, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
	{
		{
			{  11,  75,  75, 100, 0 },	/* Permanent	*/
			{   0, 100, 100, 100, 0 },	/* Moving		*/
			{   0, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
	{
		{
			{   0, 100, 100, 100, 0 },	/* Permanent	*/
			{ 154,  75,  75,  25, 0 },	/* Moving		*/
			{  56, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
	{
		{
			{ 151, 100,  50, 100, 20000 },	/* Permanent	*/
			{   0, 100, 100, 100, 0 },	/* Moving		*/
			{   0, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
	{
		{
			{  11, 100, 100, 100, 0 },	/* Permanent	*/
			{   0, 100, 100, 100, 0 },	/* Moving		*/
			{   0, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
	{
		{
			{   0, 100, 100, 100, 0 },	/* Permanent	*/
			{ 150,  50,  50, 100, 0 },	/* Moving		*/
			{   0, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	}
};

#if FALSE
	{
		{
			{   0, 100, 100, 100, 0 },	/* Permanent	*/
			{   0, 100, 100, 100, 0 },	/* Moving		*/
			{   0, 100, 100,   0, 0 },	/* Discover		*/
			{   0, 100, 100, 100, 0 }	/* Hunting		*/
		}
	},
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_NPC_sound_effects
 * FUNCTION  : Exit NPC sound effects.
 * FILE      : NPCSOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 13:44
 * LAST      : 23.08.95 12:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_NPC_sound_effects(void)
{
	UNSHORT i;

	/* Check all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Remove all sound effects for this NPC */
		Remove_all_NPC_sound_effects(i);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_all_NPC_sound_effects
 * FUNCTION  : Remove all sound effects of an NPC.
 * FILE      : NPCSOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 13:03
 * LAST      : 23.08.95 13:22
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_all_NPC_sound_effects(UNSHORT NPC_index)
{
	UNSHORT i;

	/* Check all NPC effect types */
	for (i=0;i<MAX_NPC_EFFECT_TYPES;i++)
	{
		/* Is this effect installed ? */
		if (VNPCs[NPC_index].Effect_indices[i] != 0xFFFF)
		{
			/* Yes -> Remove it */
			Remove_NPC_sound_effect(NPC_index, i);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_NPC_sound_effects
 * FUNCTION  : Update NPC sound effects.
 * FILE      : NPCSOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 13:45
 * LAST      : 29.08.95 12:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_NPC_sound_effects(void)
{
	SILONG X, Y;
	UNSHORT Effect_index;
	UNSHORT i, j;

	/* Check all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (NPC_present(i))
		{
			/* Yes -> Add permanent effect */
			Add_NPC_sound_effect(i, PERMANENT_NPC_EFFECT);

			#if FALSE
			/* Is the NPC sleeping ? */
			if (VNPCs[i].Flags & NPC_SLEEPING)
			{
				/* Yes -> Add sleeping effect */
				Add_NPC_sound_effect(i, SLEEPING_NPC_EFFECT);
			}
			else
			{
				/* No -> Remove sleeping effect */
				Remove_NPC_sound_effect(i, SLEEPING_NPC_EFFECT);
			}
			#endif

			/* Has the NPC moved / animate always ? */
			if ((VNPCs[i].Flags & NPC_MOVED) ||
			 (VNPCs[i].Flags & NPC_ANIMATE_ALWAYS))
			{
				/* Yes -> Add moving effect */
				Add_NPC_sound_effect(i, MOVING_NPC_EFFECT);
			}
			else
			{
				/* No -> Remove moving effect */
				Remove_NPC_sound_effect(i, MOVING_NPC_EFFECT);
			}

			/* Is this a chasing NPC ? */
			if (VNPCs[i].Movement_type == CHASING_MOVEMENT)
			{
				/* Yes -> Does the NPC see the party ? */
				if ((VNPCs[i].Chase_state == WAITING_NPC_CHASE_STATE) ||
				 (VNPCs[i].Chase_state == WANDERING_NPC_CHASE_STATE))
				{
					/* No -> Remove hunting effect */
					Remove_NPC_sound_effect(i, HUNTING_NPC_EFFECT);
				}
				else
				{
					/* Yes -> Add hunting effect */
					Add_NPC_sound_effect(i, HUNTING_NPC_EFFECT);
				}
			}

			/* Get this NPC's effect coordinates */
			Get_NPC_effect_coordinates(i, &X, &Y);

			/* Update the locations of all installed effects */
			for (j=0;j<MAX_NPC_EFFECT_TYPES;j++)
			{
				/* Get effect index */
				Effect_index = VNPCs[i].Effect_indices[j];

				/* Is this effect installed ? */
				if (j != 0xFFFF)
				{
					/* Yes -> Update it's position */
					Set_effect_position(Effect_index, X, Y);
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_NPC_sound_effect
 * FUNCTION  : Add an NPC's sound effect.
 * FILE      : NPCSOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 12:57
 * LAST      : 29.08.95 10:33
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 *             UNSHORT NPC_effect_type - NPC sound effect type
 *              (0...MAX_NPC_EFFECT_TYPES - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_NPC_sound_effect(UNSHORT NPC_index, UNSHORT NPC_effect_type)
{
	struct NPC_sound_effect *Effect;
	SILONG X, Y;
	UNSHORT Sound_set_index;
	UNSHORT Effect_index;

	/* Is this effect already installed ? */
	if (VNPCs[NPC_index].Effect_indices[NPC_effect_type] == 0xFFFF)
	{
		/* No -> Get sound set index */
		Sound_set_index = VNPCs[NPC_index].Sound_set_index;

		/* Does this NPC have a legal sound set ? */
		if (Sound_set_index < MAX_NPC_SOUND_SETS)
		{
			/* Yes -> Get effect data */
			Effect = &(NPC_sound_sets[Sound_set_index].Effects[NPC_effect_type]);

			/* Does this sound set have this effect ? */
			if (Effect->Effect_nr)
			{
				/* Yes -> Get effect coordinates */
				Get_NPC_effect_coordinates(NPC_index, &X, &Y);

				/* Install it */
				Effect_index = Add_located_effect(Effect->Effect_nr,
				 Effect->Priority, Effect->Volume, Effect->Probability,
				 Effect->Frequency, X, Y);

				/* Was this a one-shot effect ? */
				if (Effect->Probability)
				{
					/* No -> Store index */
					VNPCs[NPC_index].Effect_indices[NPC_effect_type] =
					 Effect_index;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_NPC_sound_effect
 * FUNCTION  : Remove an NPC's sound effect.
 * FILE      : NPCSOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 13:18
 * LAST      : 23.08.95 13:18
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 *             UNSHORT NPC_effect_type - NPC sound effect type
 *              (0...MAX_NPC_EFFECT_TYPES - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_NPC_sound_effect(UNSHORT NPC_index, UNSHORT NPC_effect_type)
{
	/* Is this effect installed ? */
	if (VNPCs[NPC_index].Effect_indices[NPC_effect_type] != 0xFFFF)
	{
		/* Yes -> Remove it */
		Remove_located_effect(VNPCs[NPC_index].Effect_indices[NPC_effect_type]);

		/* Clear index */
		VNPCs[NPC_index].Effect_indices[NPC_effect_type] = 0xFFFF;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_NPC_effect_coordinates
 * FUNCTION  : Get an NPC's sound effect coordinates.
 * FILE      : NPCSOUND.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 23.08.95 13:12
 * LAST      : 23.08.95 13:12
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 *             SILONG *X_ptr - Pointer to X-coordinate.
 *             SILONG *Y_ptr - Pointer to Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_NPC_effect_coordinates(UNSHORT NPC_index, SILONG *X_ptr, SILONG *Y_ptr)
{
	SILONG X, Y;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Get coordinates */
		X = VNPCs[NPC_index].Display_X;
		Y = (Map_height * Map_square_size) - VNPCs[NPC_index].Display_Y;
	}
	else
	{
		/* 2D -> Get coordinates */
		X = VNPCs[NPC_index].Map_X * 16;
		Y = VNPCs[NPC_index].Map_Y * 16;
	}

	/* Store coordinates */
	*X_ptr = X;
	*Y_ptr = Y;
}

