
BOOLEAN Check_line_of_movement(UNSHORT Source_X, UNSHORT Source_Y,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT Travel_mode, UNSHORT NPC_index);

void
Handle_3D_NPCs(void)
{
	SISHORT dX, dY;
	UNLONG Source_X, Source_Y;
	UNLONG Target_X, Target_Y;
	UNSHORT Old_map_nr;
	UNSHORT dTime;
	UNSHORT i, j;

	/* How much time has passed ? */
	if (Current_step < Last_NPC_step)
	{
		dTime = (Current_step + Steps_per_day) - Last_NPC_step;
	}
	else
	{
		dTime = Current_step - Last_NPC_step;
	}

	/* Remember the current time */
	Last_NPC_step = Current_step;

	/* Next step ? */
	if (dTime > 0)
	{
		/* Yes -> Clear flag */
		Monster_is_watching = FALSE;
	}

	/* Handle NPC not on paths */
	for (i=0;i<dTime;i++)
	{
		for(j=0;j<NPCS_PER_MAP;j++)
		{
			/* Anyone there ? */
			if (NPC_present(j))
			{
				/* Yes -> How does this NPC move? */
				switch (VNPCs[j].Movement_type)
				{
					/* Moving randomly */
					case RANDOM_MOVEMENT:
					{
						Do_random_NPC_3D(j);
						break;
					}
					/* Standing around */
					case WAITING_MOVEMENT:
					{
		  				Do_waiting_NPC_3D(j);
						break;
					}
					/* Chasing the party */
					case CHASING_MOVEMENT:
					{
						Do_chasing_NPC_3D(j);
						break;
					}
					/* Moving on a path */
					case ABS_PATH_MOVEMENT:
					case REL_PATH_MOVEMENT:
					{
						if (!i)
							Do_path_NPC_3D(j);
						break;
					}
				}
			}
		}
	}

	/* Update NPC positions */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Get source coordinates and convert to 3D coordinates */
			Map_to_3D(VNPCs[j].Source_X, VNPCs[j].Source_Y, &Source_X, &Source_Y);

			/* Get target coordinates and convert to 3D coordinates */
			Map_to_3D(VNPCs[j].Target_X, VNPCs[j].Target_Y, &Target_X, &Target_Y);

			/* Clear moved flag */
			VNPCs[j].Flags &= ~NPC_MOVED;

			/* Appearing or disappearing ? */
			if ((VNPCs[j].Source_X || VNPCs[j].Source_Y) &&
			 (VNPCs[j].Target_X || VNPCs[j].Target_Y))
			{
				/* No -> Calculate vector */
				dX = Target_X - Source_X;
				dY = Target_Y - Source_Y;

				/* Moved ? */
				if (dX || dY)
				{
					/* Yes -> Set moved flag */
					VNPCs[j].Flags |= NPC_MOVED;
				}

				/* Calculate new coordinates */
				VNPCs[j].Display_X = Source_X + ((SILONG) dX *
				 (SILONG) Current_substep) / SUBSTEPS_PER_STEP;
				VNPCs[j].Display_Y = Source_Y + ((SILONG) dY *
				 (SILONG) Current_substep) / SUBSTEPS_PER_STEP;
			}
			else
			{
				/* Yes -> Disappear */
				VNPCs[j].Display_X = 0;
				VNPCs[j].Display_Y = 0;
			}

			/* Determine new map coordinates */
			_3D_to_map(VNPCs[j].Display_X, VNPCs[j].Display_Y,
			 (SISHORT *) &(VNPCs[j].Map_X), (SISHORT *) &(VNPCs[j].Map_Y));
		}
	}

	/* Check for reaction with NPCs */
	Old_map_nr = PARTY_DATA.Map_nr;
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Check for reaction */
			Check_for_NPC_reaction(j);

			/* Still in the same map ? */
			if (PARTY_DATA.Map_nr != Old_map_nr)
			{
				/* No -> Break */
				break;
			}
		}
	}

	/* Update NPC sound effects */
	Update_NPC_sound_effects();
}

































	#if FALSE
	BOOLEAN Result;
	SISHORT dX, dY;
	SISHORT New_X, New_Y;
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
			/* Store direction for future random movements */
			VNPCs[NPC_index].Rnd_path_direction = Direction;

			/* Try to move */
			New_X = VNPCs[NPC_index].Map_X + Offsets8[Direction][0];
			New_Y = VNPCs[NPC_index].Map_Y + Offsets8[Direction][1];

			Result = Movement_check_3D(New_X, New_Y, NPC_index,
			 VNPCs[NPC_index].Travel_mode);

			/* Store target coordinates */
			if (Result)
			{
				VNPCs[NPC_index].Target_X = New_X;
				VNPCs[NPC_index].Target_Y = New_Y;
			}
		}
	}
	else
	{
		/* No -> Clear flag */
		VNPCs[NPC_index].Flags &= ~NPC_SEES_PARTY;

		/* Has seen already ? */
		if (VNPCs[NPC_index].Flags & NPC_HAS_SEEN_PARTY)
		{
			/* Yes -> Already moving randomly ? */
			if (VNPCs[NPC_index].Flags & NPC_MOVING_RANDOMLY)
			{
				/* Move randomly */
				Do_random_NPC_3D(NPC_index);
			}
			else
			{
				/* No -> Set random direction to last movement direction */
				VNPCs[NPC_index].Rnd_path_direction = 0;

				/* Set random path length */
				VNPCs[NPC_index].Rnd_path_length = 1 + rand() % 0x0007;

				/* Set flag */
				VNPCs[NPC_index].Flags |= NPC_MOVING_RANDOMLY;
			}
		}
	}
	#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_line_of_movement
 * FUNCTION  : Check if a line between two map positions can be moved along.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.08.95 16:29
 * LAST      : 02.08.95 16:29
 * INPUTS    : UNSHORT Source_X - Source X-coordinate.
 *             UNSHORT Source_Y - Source Y-coordinate.
 *             UNSHORT Target_X - Target X-coordinate.
 *             UNSHORT Target_Y - Target Y-coordinate.
 *             UNSHORT Travel_mode - Travel mode.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : The line is blocked.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_line_of_movement(UNSHORT Source_X, UNSHORT Source_Y, UNSHORT Target_X,
 UNSHORT Target_Y, UNSHORT Travel_mode, UNSHORT NPC_index)
{
	#if FALSE
	BOOLEAN Result = FALSE;
	UNLONG Status;
	SISHORT dX, dY;
	SISHORT Sign_dX, Sign_dY;
	UNSHORT X, Y;
	UNSHORT Var;
	UNSHORT i;

	/* Get start coordinates */
	X = Source_X;
	Y = Source_Y;

	/* Calculate vector */
	dX = Target_X - Source_X;
	dY = Target_Y - Source_Y;

	/* Length is zero ? */
	if (!dX && !dY)
	{
		/* Yes -> Not blocked */
		return FALSE;
	}

	/* Calculate vector direction */
	Sign_dX = sgn(dX);
	Sign_dY = sgn(dY);

	/* Calculate absolute vector components */
	dX = abs(dX);
	dY = abs(dY);

	/* Which way does the line go ? */
	if (dX > dY)
	{
		/* Initialize Bresenham variable */
		Var = dX / 2;

		/* Check line */
		for (i=0;i<dX - 1;i++)
		{
			/* Trace line */
			Var += dY;
			if (Var > dX)
			{
				Var -= dX;
				Y += Sign_dY;
			}
			X += Sign_dX;

			/* Blocked ? */
			if (!Double_movement_check_2D(X, Y, Sign_dX, Sign_dY, Travel_mode,
			 NPC_index))
			{
				/* Yes -> Break */
				Result = TRUE;
				break;
			}
		}
	}
	else
	{
		/* Initialize Bresenham variable */
		Var = dY / 2;

		/* Check line */
		for (i=0;i<dY - 1;i++)
		{
			/* Trace line */
			Var += dX;
			if (Var > dY)
			{
				Var -= dY;
				X += Sign_dX;
			}
			Y += Sign_dY;

			/* Blocked ? */
			if (!Double_movement_check_2D(X, Y, Sign_dX, Sign_dY, Travel_mode,
			 NPC_index))
			{
				/* Yes -> Break */
				Result = TRUE;
				break;
			}
		}
	}

	return Result;
	#endif

	return FALSE;
}

