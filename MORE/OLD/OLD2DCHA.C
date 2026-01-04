/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_chasing_NPC_2D
 * FUNCTION  : Handle an NPC in a 2D map chasing the party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.05.95 16:55
 * LAST      : 23.08.95 16:30
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_chasing_NPC_2D(UNSHORT NPC_index)
{
	BOOLEAN Result;
	SISHORT dX, dY;
	UNSHORT Direction;

	/* Set source to target coordinates */
	VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Target_X;
	VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Target_Y;

	/* Can the NPC see the party ? */
	if (Check_line_of_sight(NPC_index))
	{
		/* Yes -> Is monster or object ? */
		if ((VNPCs[NPC_index].NPC_type == MONSTER_NPC) ||
		 (VNPCs[NPC_index].NPC_type == OBJECT_NPC))
		{
			/* Yes -> Gaze! */
			Monster_is_watching = TRUE;
		}

		/* Is this a change ? */
		if (VNPCs[NPC_index].Flags & NPC_SEES_PARTY)
		{
			/* No -> Remove Aha! effect */
			Remove_NPC_sound_effect(NPC_index, DISCOVER_NPC_EFFECT);
		}
		else
		{
			/* Yes -> Start Aha! effect */
			Add_NPC_sound_effect(NPC_index, DISCOVER_NPC_EFFECT);
		}

		/* Set flags */
		VNPCs[NPC_index].Flags |= NPC_SEES_PARTY;
		VNPCs[NPC_index].Flags |= NPC_HAS_SEEN_PARTY;

		/* Reset random movement */
		VNPCs[NPC_index].Flags &= ~NPC_MOVING_RANDOMLY;

		/* Calculate vector */
		dX = PARTY_DATA.X - VNPCs[NPC_index].Map_X;
		dY = PARTY_DATA.Y - VNPCs[NPC_index].Map_Y;

		/* Get direction */
		Direction = Chase_directions[sgn(dY) + 1][sgn(dX) + 1];

		if (Direction != 0xFFFF)
		{
			/* Try to move */
			VNPCs[NPC_index].Data._2D_NPC_data.Move.X				= VNPCs[NPC_index].Source_X;
			VNPCs[NPC_index].Data._2D_NPC_data.Move.Y				= VNPCs[NPC_index].Source_Y;
			VNPCs[NPC_index].Data._2D_NPC_data.Move.Direction	= Direction;

			Result = Try_2D_move(&(VNPCs[NPC_index].Data._2D_NPC_data.Move));

			/* Store source and target coordinates */
			VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Data._2D_NPC_data.Move.X;
			VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Data._2D_NPC_data.Move.Y;

			VNPCs[NPC_index].Target_X = VNPCs[NPC_index].Source_X +
			 VNPCs[NPC_index].Data._2D_NPC_data.Move.dX;
			VNPCs[NPC_index].Target_Y = VNPCs[NPC_index].Source_Y +
			 VNPCs[NPC_index].Data._2D_NPC_data.Move.dY;
		}
	}
	else
	{
		/* No -> Clear flag */
		VNPCs[NPC_index].Flags &= ~NPC_SEES_PARTY;

		/* Remove Aha! effect */
		Remove_NPC_sound_effect(NPC_index, DISCOVER_NPC_EFFECT);

		/* Has seen already ? */
		if (VNPCs[NPC_index].Flags & NPC_HAS_SEEN_PARTY)
		{
			/* Yes -> Already moving randomly ? */
			if (VNPCs[NPC_index].Flags & NPC_MOVING_RANDOMLY)
			{
				/* Move randomly */
				Do_random_NPC_2D(NPC_index);
			}
			else
			{
				/* No -> Set random direction to last movement direction */
				VNPCs[NPC_index].Rnd_path_direction =
				 VNPCs[NPC_index].Data._2D_NPC_data.Move.Direction;

				/* Set random path length */
				VNPCs[NPC_index].Rnd_path_length = 1 + rand() % 0x0007;

				/* Set flag */
				VNPCs[NPC_index].Flags |= NPC_MOVING_RANDOMLY;
			}
		}
	}
}

