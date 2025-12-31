/************
 * NAME     : 2D_COLL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 8-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>

#include <BBDEF.H>

#include <ALBION.H>

#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <2D_COLL.H>
#include <2D_PATH.H>
#include <2D_DISPL.H>
#include <PRTLOGIC.H>
#include <NPCS.H>

/* prototypes */

/* 2D movement and collision detection */

BOOLEAN Try_big_guy_2D_move(struct Movement_2D *Move);
BOOLEAN Try_small_guy_2D_move(struct Movement_2D *Move);

BOOLEAN Try_double_slide_2D(struct Movement_2D *Move, UNSHORT Direction4);
BOOLEAN Try_to_slide_2D(struct Movement_2D *Move, UNSHORT Direction4);
void Slide_2D(struct Movement_2D *Move, UNSHORT Direction4);

BOOLEAN Special_2D_high_check(UNSHORT X, UNSHORT Y);
BOOLEAN Special_2D_low_check(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY);

BOOLEAN Extra_vertical_movement_check_2D(UNSHORT X, UNSHORT Y,
 UNSHORT NPC_index);

BOOLEAN Double_movement_check_2D(struct Movement_2D *Move, UNSHORT X,
 UNSHORT Y, SISHORT dX, SISHORT dY);

BOOLEAN Do_check_2D(struct Movement_2D *Move, UNSHORT X, UNSHORT Y,
 SISHORT dX, SISHORT dY);

BOOLEAN Check_2D_NPC_collision(UNSHORT X, UNSHORT Y, UNSHORT NPC_index);

BOOLEAN Sucking_chairs_2D(struct Movement_2D *Move);

/* global variables */

static BOOLEAN Slide_flag;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_2D_move
 * FUNCTION  : Make a move in a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 10.08.95 15:53
 * INPUTS    : UNSHORT Direction - Direction (0...7).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_2D_move(UNSHORT Direction)
{
	static struct Movement_2D Party_2D_move_data = {
		0,
		0, 0,
		0,
		0, 0,
		0,
		0xFFFF,
		0xFFFF,
		0,
		0xFFFF,
		0
	};

	BOOLEAN Result;
	UNSHORT Old_view_direction;

	/* Clear flags */
	Moved = FALSE;

	/* Exit if the party is still moving */
	if (_2D_steps_left > 0)
		return;

	/* Allowed by before-move logic ? */
	if (Before_move())
	{
		/* Yes -> Store old view direction */
		Old_view_direction = PARTY_DATA.View_direction;

		/* Try to move */
		Party_2D_move_data.X						= PARTY_DATA.X;
		Party_2D_move_data.Y						= PARTY_DATA.Y;
		Party_2D_move_data.Direction			= Direction;
		Party_2D_move_data.View_direction	= PARTY_DATA.View_direction;
		Party_2D_move_data.Travel_mode		= PARTY_DATA.Travel_mode;
		Party_2D_move_data.State				= Active_member_state;

		Result = Try_2D_move(&Party_2D_move_data);

		/* Enter move */
		PARTY_DATA.X					= Party_2D_move_data.X;
		PARTY_DATA.Y					= Party_2D_move_data.Y;
		PARTY_DATA.View_direction	= Party_2D_move_data.View_direction;
		Active_member_state			= Party_2D_move_data.State;

		/* Any movement ? */
		if (Party_2D_move_data.dX || Party_2D_move_data.dY)
		{
			/* Yes -> Store movement in path */
			Store_2D_movement(Party_2D_move_data.dX, Party_2D_move_data.dY);

			/* Update map coordinates */
			PARTY_DATA.X += Party_2D_move_data.dX;
			PARTY_DATA.Y += Party_2D_move_data.dY;
		}
		else
		{
			/* No -> Update anyway / change of view direction ? */
			if ((Party_2D_move_data.Flags & UPDATE_AFTER_MOVE) ||
			 (PARTY_DATA.View_direction != Old_view_direction))
			{
				/* Yes -> Store current position */
				Store_2D_current_position();
			}
		}

		/* Success ? */
		if (Result)
		{
			/* Yes -> Set flag */
			Moved = TRUE;

			/* Do after-move logic */
			After_move();
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_2D_move
 * FUNCTION  : Check if a certain move can be made in a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 09.08.95 12:27
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_2D_move(struct Movement_2D *Move)
{
	static UNSHORT Target_VDs[8] = {
		NORTH, NORTH, EAST, SOUTH, SOUTH, SOUTH, WEST, NORTH
	};

	BOOLEAN Result = TRUE;
	SISHORT ddDir;
	UNSHORT cDir;
	UNSHORT dDir;
	UNSHORT tVD = 0xFFFF;

	/* No sliding has occurred */
	Slide_flag = FALSE;

	/* Get current direction */
	cDir = Move->View_direction * 2;

	/* Get reverse slide direction (if any) */
	if (Move->Slide_direction != 0xFFFF)
	{
		Move->Reverse_slide = (Move->Slide_direction + 2) & 0x0003;
	}

	/* Calculate direction difference */
	if (Move->Direction >= cDir)
	{
		dDir = Move->Direction - cDir;
		ddDir = 1;
	}
	else
	{
		dDir = cDir - Move->Direction;
		ddDir = -1;
	}

	if (dDir > 4)
	{
		dDir = 8 - dDir;
		ddDir = 0 - ddDir;
	}

	/* Does the player want to move orthogonally and is the difference in
	 directions equal to 90 degrees ? */
	if ((dDir == 2) && !(Move->Direction & 1))
	{
		/* Yes -> The player must be rotated */
		tVD = Target_VDs[Move->Direction];
	}

	/* Is the difference in directions greater than 90 degrees ? */
	if (dDir > 2)
	{
		/* Yes -> The player must be rotated. An extra rotation step is needed
		 between the current and the target direction. This extra view (!)
		 direction must be determined. */
		tVD = Target_VDs[(cDir + (ddDir * ((dDir + 1) / 2))) & 0x0007];

	}

	/* Is rotation necessary ? */
	if (tVD != 0xFFFF)
	{
		/* Yes -> Rotate */
		Move->Slide_direction	= 0xFFFF;
		Move->Reverse_slide		= 0xFFFF;

		/* Are we standing ? */
		if (Move->State == STANDING_STATE)
		{
			/* Yes -> Rotate on the spot */
			Move->View_direction	= tVD;
			Move->dX					= 0;
			Move->dY					= 0;

			return TRUE;
		}
		else
		{
			/* No -> Switch directly to the desired view direction */
			Move->View_direction	= Target_VDs[Move->Direction];
		}
	}

	/* Big or small guy ? */
	if (Big_guy)
	{
		Result = Try_big_guy_2D_move(Move);
	}
	else
	{
		Result = Try_small_guy_2D_move(Move);
	}

	/* Movement blocked ? */
	if (!Result)
	{
		/* Yes -> Clear vector */
		Move->dX = 0;
		Move->dY = 0;
		Move->Flags &= ~UPDATE_AFTER_MOVE;
	}

	/* Any movement at all ? */
	if (Move->dX || Move->dY)
	{
		/* Yes -> Suck ? */
		if (!(Move->Flags & DONT_SUCK))
		{
			/* Yes -> Suck */
			Result = Sucking_chairs_2D(Move);
		}
	}

	/* Any sliding this time ? */
	if (!Slide_flag)
	{
		/* No -> Clear */
		Move->Slide_direction = 0xFFFF;

		/* Any movement ? */
		if (Move->dX || Move->dY)
		{
			/* Yes */
 			Move->Reverse_slide = 0xFFFF;
		}
 	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_big_guy_2D_move
 * FUNCTION  : Check if a big guy can make a certain move in a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 16:50
 * LAST      : 09.08.95 10:42
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_big_guy_2D_move(struct Movement_2D *Move)
{
	static SISHORT Big_diagonal_check_offsets[4][2] = {
		{0, 1}, {1, 1}, {2, 1}, {2, 0}
	};

	BOOLEAN Result = TRUE;
	SISHORT corrX = 0;
	SISHORT dX, dY;
	UNSHORT X, Y;
 	UNSHORT cVD;
	UNSHORT cDir;
	UNSHORT Q;
	UNSHORT i;

	/* Get important data */
	X = Move->X;
	Y = Move->Y;

	/* Get movement vector */
	dX = Offsets8[Move->Direction][0];
	dY = Offsets8[Move->Direction][1];

	/* Get current direction */
	cVD = Move->View_direction;
	cDir = cVD * 2;

	/* Does the player want to move diagonally ? */
	if (Move->Direction & 1)
	{
		/* Yes -> Determine horizontal correction */
		if (dX < 0)
			corrX = 1;

		/* First we must determine the block state for the current position */
		/* (here corrX must be added to dX!) */
		Q = 0;

		for (i=0;i<4;i++)
		{
			if (!Do_check_2D(Move, X, Y,
			 (Big_diagonal_check_offsets[i][0] * dX) + corrX,
			 (Big_diagonal_check_offsets[i][1] * dY)))
			{
				Q |= (1 << i);
			}

			if ((i == 1) || (i == 2))
			{
				if (!Check_2D_NPC_collision(X +
				 (Big_diagonal_check_offsets[i][0] * dX) + corrX, Y +
				 (Big_diagonal_check_offsets[i][1] * dY), Move->NPC_index))
				{
					Q |= (1 << i);
				}
			}
		}

		/* Then we act according to the block state */
		switch (Q)
		{
			/*** Movement may be possible ***/
			case 0:
			{
				/* Is vertical movement possible here ? */
				if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					Move->NPC_index))
				{
					/* Yes -> Move */
					Move->dX = dX;
					Move->dY = dY;
				}
				else
				{
					/* No -> Movement is blocked */
					Result = FALSE;
				}
				break;
			}
			/*** Movement is blocked ***/
			case 10:
			case 11:
			case 13:
			case 14:
			case 15:
			{
				Result = FALSE;
				break;
			}
			/*** Try a diagonal move ***/
			case 9:
			{
				/* Is vertical movement possible here ? */
				if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
				 Move->NPC_index))
				{
					/* Yes -> Moving up ? */
					if (dY < 0)
					{
						/* Yes -> Can we slip through ? */
						if ((Special_2D_high_check(X + 2 * dX + corrX, Y)) &&
						 (Special_2D_low_check(X + corrX, Y - 1, dX, dY)))
						{
							/* Yes */
							Move->dX = dX;
							Move->dY = dY;
							break;
						}
					}
					else
					{
						/* No -> Can we slip through ? */
						if ((Special_2D_high_check(X + corrX, Y + 1)) &&
						 (Special_2D_low_check(X + 2 * dX + corrX, Y, dX, dY)))
						{
							/* Yes */
							Move->dX = dX;
							Move->dY = dY;
							break;
						}
					}
				}

				Result = FALSE;
				break;
			}
			/*** Try a diagonal move ***/
			case 8:
			{
				/* Is vertical movement possible here ? */
				if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
				 Move->NPC_index))
				{
					/* Yes -> Moving up ? */
					if (dY < 0)
					{
						/* Yes -> Can we pass behind ? */
						if (Special_2D_high_check(X + 2 * dX + corrX, Y))
						{
							/* Yes */
							Move->dX = dX;
							Move->dY = dY;
							break;
						}
					}
					else
					{
						/* No -> Can we pass over ? */
						if (Special_2D_low_check(X + 2 * dX + corrX, Y, dX, dY))
						{
							/* Yes */
							Move->dX = dX;
							Move->dY = dY;
							break;
						}
					}
				}

				/* Try a vertical slide */
				if (dY < 0)
				{
					Result = Try_to_slide_2D(Move, NORTH);
				}
				else
				{
					Result = Try_to_slide_2D(Move, SOUTH);
				}
				break;
			}
			/*** Try a diagonal move ***/
			case 1:
			{
				/* Is vertical movement possible here ? */
				if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
				 Move->NPC_index))
				{
					/* Yes -> Moving up ? */
					if (dY < 0)
					{
						/* Yes -> Can we pass behind ? */
						if (Special_2D_low_check(X + corrX, Y - 1, dX, dY))
						{
							/* Yes */
							Move->dX = dX;
							Move->dY = dY;
							break;
						}
					}
					else
					{
						/* No -> Can we pass over ? */
						if (Special_2D_high_check(X + corrX, Y + 1))
						{
							/* Yes */
							Move->dX = dX;
							Move->dY = dY;
							break;
						}
					}
				}

				/* Try a horizontal slide */
				if (dX < 0)
				{
					Result = Try_to_slide_2D(Move, WEST);
				}
				else
				{
					Result = Try_to_slide_2D(Move, EAST);
				}
				break;
			}
			/*** Try a horizontal slide ***/
			case 2:
			case 3:
			case 5:
			case 6:
			case 7:
			{
				if (dX < 0)
				{
					Result = Try_to_slide_2D(Move, WEST);
				}
				else
				{
					Result = Try_to_slide_2D(Move, EAST);
				}
				break;
			}
			/*** Try a vertical slide ***/
			case 12:
			{
				if (dY < 0)
				{
					Result = Try_to_slide_2D(Move, NORTH);
				}
				else
				{
					Result = Try_to_slide_2D(Move, SOUTH);
				}
				break;
			}
			/*** Try to slide perpendicular to the current view direction ***/
			case 4:
			{
				/* Determine perpendicular direction */
				cDir = (Move->Direction - (cDir - Move->Direction)) & 0x0007;
				cVD = cDir / 2;

				/* Slide */
				if (Move->Reverse_slide == cVD)
				{
					Slide_2D(Move, Move->View_direction);
				}
				else
				{
					Slide_2D(Move, cVD);
				}
				break;
			}
		}
	}
	else
	{
		/* No -> Is the current view direction horizontal or vertical ? */
		if (cVD & 1)
		{
			/* Horizontal movement */
			/* Determine horizontal correction */
			if (dX > 0)
				corrX = 1;

			/* Determine the block state for the current position */
			/* (here corrX must be added to the X-coordinate!) */
			Q = 0;

			for (i=0;i<3;i++)
			{
				if (!Do_check_2D(Move, X + corrX, Y, dX, i - 1))
				{
					Q |= (1 << i);
				}

				if (i == 1)
				{
					if (!Check_2D_NPC_collision(X + corrX + dX, Y + i - 1,
					 Move->NPC_index))
					{
						Q |= (1 << i);
					}
				}
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/*** Movement is possible ***/
				case 0:
				case 1:
				case 4:
				case 5:
				{
					Move->dX = dX;
					Move->dY = dY;
					break;
				}
				/*** Movement is blocked ***/
				case 7:
				{
					Result = FALSE;
					break;
				}
				/*** Try a vertical slide up or down ***/
				case 2:
				case 6:
				{
					Result = Try_double_slide_2D(Move, NORTH);
					break;
				}
				/*** Try a vertical slide down or up ***/
				case 3:
				{
					Result = Try_double_slide_2D(Move, SOUTH);
					break;
				}
			}
		}
		else
		{
			/* Vertical movement */
			/* Determine the block state for the current position */
			Q = 0;

			for (i=0;i<4;i++)
			{
				if (!Do_check_2D(Move, X, Y, i - 1, dY))
				{
					Q |= (1 << i);
				}
				if (!Do_check_2D(Move, X + 1, Y, i - 2, dY))
				{
					Q |= (1 << i);
				}

				if (i == 1)
				{
					if (!Check_2D_NPC_collision(X + i - 1, Y + dY,
					 Move->NPC_index))
					{
						Q |= (1 << i);
					}
				}
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/*** Movement may be possible ***/
				case 0:
				case 9:
				{
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Move */
						Move->dX = dX;
						Move->dY = dY;
					}
					else
					{
						/* No -> Try a horizontal slide */
						if (Move->Slide_direction == EAST)
						{
							Result = Try_double_slide_2D(Move, EAST);
						}
						else
						{
							if (Move->Slide_direction == WEST)
							{
								Result = Try_double_slide_2D(Move, WEST);
							}
							else
							{
								if (X < Map_width / 2)
								{
									Result = Try_double_slide_2D(Move, WEST);
								}
								else
								{
									Result = Try_double_slide_2D(Move, EAST);
								}
							}
						}
					}
					break;
				}
				/*** Movement may be possible ***/
				case 1:
				{
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Move */
						Move->dX = dX;
						Move->dY = dY;
					}
					else
					{
						/* No -> Try a horizontal slide to the right */
	 					Result = Try_double_slide_2D(Move, EAST);
					}
					break;
				}
				/*** Movement may be possible ***/
				case 8:
				{
					/* Is vertical movement possible here ? */
					if (Extra_vertical_movement_check_2D(X + dX, Y + dY,
					 Move->NPC_index))
					{
						/* Yes -> Move */
						Move->dX = dX;
						Move->dY = dY;
					}
					else
					{
						/* No -> Try a horizontal slide to the left */
	 					Result = Try_double_slide_2D(Move, WEST);
					}
					break;
				}
				/*** Movement is blocked ***/
				case 5:
				case 6:
				case 10:
				case 11:
				case 13:
				case 15:
				{
					Result = FALSE;
					break;
				}
				/*** Try a horizontal slide to the right ***/
				case 2:
				case 3:
				case 7:
				{
					Result = Try_double_slide_2D(Move, EAST);
					break;
				}
				/*** Try a horizontal slide to the left ***/
				case 4:
				case 12:
				case 14:
				{
 					Result = Try_double_slide_2D(Move, WEST);
					break;
				}
			}
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_small_guy_2D_move
 * FUNCTION  : Check if a small guy can make a certain move in a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 16:50
 * LAST      : 09.08.95 10:43
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_small_guy_2D_move(struct Movement_2D *Move)
{
	static SISHORT Small_diagonal_check_offsets[3][2] = {
		{0,1}, {1,1}, {1,0}
	};

	BOOLEAN Result = TRUE;
	SISHORT dX, dY;
	UNSHORT X, Y;
 	UNSHORT cVD;
	UNSHORT cDir;
	UNSHORT Q;
	UNSHORT i;

	/* Get important data */
	X = Move->X;
	Y = Move->Y;

	/* Get movement vector */
	dX = Offsets8[Move->Direction][0];
	dY = Offsets8[Move->Direction][1];

	/* Get current direction */
	cVD = Move->View_direction;
	cDir = cVD * 2;

	/* Does the player want to move diagonally ? */
	if (Move->Direction & 1)
	{
		/* Yes -> First we must determine the block state for the
		 current position */
		Q = 0;

		for (i=0;i<3;i++)
		{
			if (!Do_check_2D(Move, X, Y,
			 (Small_diagonal_check_offsets[i][0] * dX),
			 (Small_diagonal_check_offsets[i][1] * dY)))
			{
				Q |= (1 << i);
			}
			if (i == 1)
			{
				if (!Check_2D_NPC_collision(X +
					(Small_diagonal_check_offsets[i][0] * dX),
					Y + (Small_diagonal_check_offsets[i][1] * dY),
					Move->NPC_index))
				{
					Q |= (1 << i);
				}
			}
		}

		/* Then we act according to the block state */
		switch (Q)
		{
			/*** Movement is possible ***/
			case 0:
			{
				Move->dX = dX;
				Move->dY = dY;
				break;
			}
			/*** Movement is blocked ***/
			case 5:
			case 7:
			{
				Result = FALSE;
				break;
			}
			/*** Try a horizontal slide ***/
			case 1:
			case 3:
			{
				if (dX < 0)
				{
					Slide_2D(Move, WEST);
				}
				else
				{
					Slide_2D(Move, EAST);
				}
				break;
			}
			/*** Try a vertical slide ***/
			case 4:
			case 6:
			{
				if (dY < 0)
				{
					Slide_2D(Move, NORTH);
				}
				else
				{
					Slide_2D(Move, SOUTH);
				}
				break;
			}
			/*** Try to slide perpendicular to the current view direction ***/
			case 2:
			{
				/* Determine perpendicular direction */
				cDir = (Move->Direction - (cDir - Move->Direction)) & 0x0007;
				cVD = cDir / 2;

				/* Slide */
				if (Move->Reverse_slide == cVD)
				{
 					Slide_2D(Move, Move->View_direction);
				}
				else
				{
  					Slide_2D(Move, cVD);
				}
				break;
			}
		}
	}
	else
	{
		/* No -> Is the current view direction horizontal or vertical ? */
		if (cVD & 1)
		{
			/* Horizontal movement */
			/* Determine the block state for the current position */
			Q = 0;

			for (i=0;i<3;i++)
			{
				if (!Do_check_2D(Move, X, Y, dX, i - 1))
				{
					Q |= (1<<i);
				}
				if (i == 1)
				{
					if (!Check_2D_NPC_collision(X + dX, Y + i - 1,
						Move->NPC_index))
					{
						Q |= (1 << i);
					}
				}
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/*** Movement is possible ***/
				case 0:
				case 1:
				case 4:
				case 5:
				{
					Move->dX = dX;
					Move->dY = dY;
					break;
				}
				/*** Movement is blocked ***/
				case 7:
				{
					Result = FALSE;
					break;
				}
				/*** Try a vertical slide down ***/
				case 3:
				{
					Result = Try_double_slide_2D(Move, SOUTH);
					break;
				}
				/*** Try a vertical slide up ***/
				case 2:
				case 6:
				{
					Result = Try_double_slide_2D(Move, NORTH);
					break;
				}
			}
		}
		else
		{
			/* Vertical movement */
			/* Determine the block state for the current position */
			Q = 0;

			for (i=0;i<3;i++)
			{
				if (!Do_check_2D(Move, X, Y, i - 1, dY))
				{
					Q |= (1 << i);
				}
				if (i == 1)
				{
					if (!Check_2D_NPC_collision(X + i - 1, Y + dY,
					 Move->NPC_index))
					{
						Q |= (1 << i);
					}
				}
			}

			/* Then we act according to the block state */
			switch (Q)
			{
				/*** Movement is possible ***/
				case 0:
				case 1:
				case 4:
				case 5:
				{
					Move->dX = dX;
					Move->dY = dY;
					break;
				}
				/*** Movement is blocked ***/
				case 7:
				{
					Result = FALSE;
					break;
				}
				/*** Try a horizontal slide to the right ***/
				case 3:
				{
					Result = Try_double_slide_2D(Move, EAST);
					break;
				}
				/*** Try a horizontal slide to the left ***/
				case 2:
				case 6:
				{
					Result = Try_double_slide_2D(Move, WEST);
					break;
				}
			}
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_double_slide_2D
 * FUNCTION  : Try to slide first in one, then in the opposite direction in
 *              a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 17:28
 * LAST      : 09.08.95 14:21
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 *             UNSHORT Direction4 - Direction in which to slide (0...3).
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - This function will make sure that the moving person will
 *              not slide back the way it came.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_double_slide_2D(struct Movement_2D *Move, UNSHORT Direction4)
{
	BOOLEAN Result = FALSE;

	/* Sliding back ? */
	if (Move->Reverse_slide != Direction4)
	{
		/* No -> Try to slide */
		Result = Try_to_slide_2D(Move, Direction4);
	}

	/* Success ? */
	if (!Result)
	{
		/* No -> Try the opposite direction */
		Direction4 = (Direction4 + 2) & 0x0003;

		/* Sliding back ? */
		if (Move->Reverse_slide != Direction4)
		{
			/* No -> Try to slide */
			Result = Try_to_slide_2D(Move, Direction4);
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Try_to_slide_2D
 * FUNCTION  : Try to slide in a direction in a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 17:22
 * LAST      : 09.08.95 12:31
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 *             UNSHORT Direction4 - Direction in which to slide (0...3).
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - Unlike Slide_2D, this function performs a collision check
 *              to determine whether the target position can be moved to.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Try_to_slide_2D(struct Movement_2D *Move, UNSHORT Direction4)
{
	BOOLEAN Result;
	SISHORT dX, dY;

	/* Get movement vector */
	dX = Offsets4[Direction4][0];
	dY = Offsets4[Direction4][1];

	/* Destination blocked ? */
	Result = Double_movement_check_2D(Move, Move->X, Move->Y, dX, dY);
	if (Result)
	{
		/* No -> Slide */
		Slide_2D(Move, Direction4);
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Slide_2D
 * FUNCTION  : Slide in a direction in a 2D map.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.95 10:31
 * LAST      : 09.08.95 12:32
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 *             UNSHORT Direction4 - Direction in which to slide (0...3).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Unlike Try_to_slide_2D, this function does not perform a
 *              collision check to determine whether the target position can
 *              be moved to.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Slide_2D(struct Movement_2D *Move, UNSHORT Direction4)
{
	SISHORT dX, dY;

	/* Get movement vector */
	dX = Offsets4[Direction4][0];
	dY = Offsets4[Direction4][1];

	/* Did we slide in this direction last time ? */
	if (Move->Slide_direction == Direction4)
	{
		/* Yes -> Turn around */
		Move->View_direction = Direction4;
	}

	/* Store movement vector */
	Move->dX = dX;
	Move->dY = dY;

	/* Sliding occurred */
	Slide_flag = TRUE;

	/* Store slide direction */
	Move->Slide_direction = Direction4;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Special_2D_high_check
 * FUNCTION  : Check if an icon in a 2D map can be passed behind diagonally.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.94 13:04
 * LAST      : 27.07.95 10:35
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Special_2D_high_check(UNSHORT X, UNSHORT Y)
{
	struct Icon_2D_data *Icon_data;
	struct Square_2D *Map_ptr;
	UNLONG Status;
	UNSHORT B1, B2, B3;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;
	UNSHORT Under_h = 0, Over_h = 0;

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return FALSE;

	/* Read bytes from map */
	Map_ptr = (struct Square_2D *) (MEM_Claim_pointer(Map_handle)
	 + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);

	B1 = (UNSHORT) Map_ptr->m[0];
	B2 = (UNSHORT) Map_ptr->m[1];
	B3 = (UNSHORT) Map_ptr->m[2];

	MEM_Free_pointer(Map_handle);

	/* Build overlay and underlay number */
	Underlay_nr = ((B2 & 0x0F) << 8) | B3;
	Overlay_nr = (B1 << 4) | (B2 >> 4);

	Icon_data = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);

	/* Any underlay ? */
	if (Underlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Underlay_nr-2].Flags;
		Under_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
	}

	/* Any overlay ? */
	if (Overlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Overlay_nr-2].Flags;
		Over_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
	}

	MEM_Free_pointer(Icondata_handle);

	/* Does the under- or the overlay have a height greater than 0 ? */
	if (Under_h || Over_h)
	{
		/* Yes -> You can pass behind */
		return TRUE;
	}
	else
	{
		/* No -> Don't even try */
		return FALSE;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Special_2D_low_check
 * FUNCTION  : Check if an icon in a 2D map can be passed over diagonally.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.09.94 13:04
 * LAST      : 11.08.95 15:00
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             SISHORT dX - Horizontal component of movement vector.
 *             SISHORT dY - Horizontal component of movement vector.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Special_2D_low_check(UNSHORT X, UNSHORT Y, SISHORT dX, SISHORT dY)
{
	struct Icon_2D_data *Icon_data;
	struct Square_2D *Map_ptr;
	UNLONG Status;
	UNSHORT B1, B2, B3;
	UNSHORT Underlay_nr;
	UNSHORT Overlay_nr;
	UNSHORT Under_h = 0;
	UNSHORT Over_h = 0;
	UNSHORT Frame = 0;
	UNSHORT Pixel;
	UNSHORT Side;
	UNBYTE *Icon_gfx_ptr;

	/* Exit if coordinates lie outside the map */
	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return FALSE;

	/* Read bytes from map */
	Map_ptr = (struct Square_2D *) (MEM_Claim_pointer(Map_handle)
	 + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);

	B1 = (UNSHORT) Map_ptr->m[0];
	B2 = (UNSHORT) Map_ptr->m[1];
	B3 = (UNSHORT) Map_ptr->m[2];

	MEM_Free_pointer(Map_handle);

	/* Build overlay and underlay number */
	Underlay_nr = ((B2 & 0x0F) << 8) | B3;
	Overlay_nr = (B1 << 4) | (B2 >> 4);

	Icon_data = (struct Icon_2D_data *) MEM_Claim_pointer(Icondata_handle);

	/* Any underlay ? */
	if (Underlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Underlay_nr-2].Flags;
		Under_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;
	}

	/* Any overlay ? */
	if (Overlay_nr > 1)
	{
		/* Yes -> Get icon height */
		Status = Icon_data[Overlay_nr-2].Flags;
		Over_h = (Status & ICON_HEIGHT) >> ICON_HEIGHT_B;

		/* And first frame number */
		Frame = Icon_data[Overlay_nr-2].First_frame;
	}

	MEM_Free_pointer(Icondata_handle);

	/* Exit if there is no overlay */
	if (Overlay_nr < 2)
		return FALSE;

	/* Does the under- or the overlay have a height greater than 1 ? */
	if ((Under_h > 1) || (Over_h > 1))
	{
		/* Yes -> Forget it */
		return FALSE;
	}
	else
	{
		/* No -> Test the bottom pixel of the overlay */
		Icon_gfx_ptr = MEM_Claim_pointer(Icongfx_handle);

		/* Which side ? */
		if (dY < 0)
		{
			if (dX < 0)
				Side = 0;
			else
				Side = 15;
		}
		else
		{
			if (dX < 0)
				Side = 15;
			else
				Side = 0;
		}

		/* Get the correct pixel */
		Pixel = *(Icon_gfx_ptr + (256 * Frame) + (15 * 16) + Side);

		MEM_Free_pointer(Icongfx_handle);

		/* Is it transparent ? */
		if (Pixel)
		{
			/* No -> Forget it */
			return FALSE;
		}
		else
		{
			/* Yes -> Pass over it */
			return TRUE;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Double_movement_check_2D
 * FUNCTION  : Check if a certain position in a 2D map can be moved to.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 09.08.95 13:45
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 *             UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             SISHORT dX - delta X.
 *             SISHORT dY - delta Y.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - This function will also check the square next to the given
 *              location in 2D maps where big characters are shown.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Double_movement_check_2D(struct Movement_2D *Move, UNSHORT X, UNSHORT Y,
 SISHORT dX, SISHORT dY)
{
	/* Any movement ? */
	if (dX || dY)
	{
		/* Yes -> Check for collision with NPC's */
		if (!(Check_2D_NPC_collision(X + dX, Y + dY, Move->NPC_index)))
			return FALSE;
	}

	/* Big guy ? */
	if (Big_guy)
	{
		/* Yes -> Check two squares */
		if (!Do_check_2D(Move, X, Y, dX, dY) ||
		 !Do_check_2D(Move, X + 1, Y, dX, dY))
		{
			return FALSE;
		}

		/* Moving vertically ? */
		if (dY)
		{
			/* Yes -> Extra check */
			if (!Extra_vertical_movement_check_2D(X + dX, Y + dY,
			 Move->NPC_index))
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	else
	{
		/* No -> Check one square */
		return (Do_check_2D(Move, X, Y, dX, dY));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Extra_vertical_movement_check_2D
 * FUNCTION  : Check if a vertical movement in a large-scale 2D map is possible.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.11.94 16:38
 * LAST      : 26.11.94 16:38
 * INPUTS    : UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * NOTES     : - This function checks if there's a "wall" between the two
 *              target squares.
 *             - Only call this when moving vertically.
 *             - The function can handle small-scale maps by itself.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Extra_vertical_movement_check_2D(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	UNLONG StatusL, StatusR;

	/* Large-scale ? */
	if (Big_guy)
	{
		/* Yes -> Do extra check for both target squares */
		StatusL = Get_location_status(X, Y, NPC_index);
		StatusR = Get_location_status(X + 1, Y, NPC_index);

		if ((StatusL & (1 << (BLOCKED_DIRECTIONS_B + EAST))) ||
		 (StatusR & (1 << (BLOCKED_DIRECTIONS_B + WEST))))
			return FALSE;
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_check_2D
 * FUNCTION  : Check if a certain position in a 2D map can be moved to.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 17:54
 * LAST      : 09.08.95 13:45
 * INPUTS    : struct Movement_2D *Move - Pointer to 2D movement structure.
 *             UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             SISHORT dX - X-component of movement vector.
 *             SISHORT dY - Y-component of movement vector.
 * RESULT    : BOOLEAN : TRUE or FALSE.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Do_check_2D(struct Movement_2D *Move, UNSHORT X, UNSHORT Y, SISHORT dX,
 SISHORT dY)
{
	static UNSHORT Directions[3][3] = {
		{7, 0, 1},
		{6, 8, 2},
		{5, 4, 3}
	};
	static UNSHORT Direction_pairs[8][2] = {
		{NORTH, NORTH}, {NORTH, EAST}, {EAST, EAST}, {SOUTH, EAST},
		{SOUTH, SOUTH}, {SOUTH, WEST}, {WEST, WEST}, {NORTH, WEST}
	};

	UNLONG Status;
	UNSHORT State;
	UNSHORT Dir1, Dir2;
	UNSHORT X2, Y2;

	/* Exit if no movement */
	if (!dX && !dY)
		return TRUE;

	/* Calculate target coordinates */
	X2 = X + dX;
	Y2 = Y + dY;

	/* Exit if target coordinates lie outside the map */
	if ((X2 < 1) || (X2 > Map_width) || (Y2 < 1) || (Y2 > Map_height))
		return FALSE;

	/* Exit if cheat mode is on / not NPC */
	if ((Move->NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	/* Get direction */
	Dir1 = Directions[sgn(dY) + 1][sgn(dX) + 1];

	/* Get the original location's status */
	Status = Get_location_status(X, Y, Move->NPC_index);

	/* Check if the original location can be left in this direction */
	if ((Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir1][0])))
	 && (Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir1][1]))))
		return FALSE;

	/* Get the new location's status */
	Status = Get_location_status(X2, Y2, Move->NPC_index);

	/* Checking the party ? */
	if (Move->NPC_index == 0xFFFF)
	{
		/* Yes -> Get state */
		State = (Status & STATE) >> STATE_B;

		/*  Sitting or sleeping state ? */
		if ((State == SITTING_NL_STATE) || (State == SITTING_NR_STATE) ||
		 (State == SITTING_E_STATE) || (State == SITTING_SL_STATE) ||
		 (State == SITTING_SR_STATE) || (State == SITTING_W_STATE) ||
		 (State == SLEEPING1_STATE) || (State == SLEEPING2_STATE))
		{
			/* Yes -> Exit */
			return FALSE;
		}
	}
	else
	{
		/* No -> Checking a hunting NPC ? */
		if (VNPCs[Move->NPC_index].Movement_type == CHASING_MOVEMENT)
		{
			/* Yes -> Get state */
			State = (Status & STATE) >> STATE_B;

			/*  Sitting or sleeping state ? */
			if ((State == SITTING_NL_STATE) || (State == SITTING_NR_STATE) ||
			 (State == SITTING_E_STATE) || (State == SITTING_SL_STATE) ||
			 (State == SITTING_SR_STATE) || (State == SITTING_W_STATE) ||
			 (State == SLEEPING1_STATE) || (State == SLEEPING2_STATE))
			{
				/* Yes -> Exit */
				return FALSE;
			}
		}
	}

	/* Get reverse direction */
	Dir2 = (Dir1 + 4) & 0x0007;

	/* Check if the new location can be entered from this direction */
	if ((Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir2][0])))
	 && (Status & (1 << (BLOCKED_DIRECTIONS_B + Direction_pairs[Dir2][1]))))
		return FALSE;

	/* Check if the new location is accessible for the current travelmode */
	if (Status & (1 << (BLOCKED_TRAVELMODES_B + Move->Travel_mode)))
		return FALSE;
	else
		return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_2D_NPC_collision
 * FUNCTION  : Check if a certain position in the 2D map is blocked by an NPC.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.11.94 12:32
 * LAST      : 08.05.95 14:39
 * INPUTS    : UNSHORT X - Original X-coordinate.
 *             UNSHORT Y - Original Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : BOOLEAN : TRUE (no collision) or FALSE (blocked).
 * BUGS      : No known.
 * NOTES     : - This function will also check if an NPC collides with the
 *              party.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_2D_NPC_collision(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	UNSHORT i;

	/* Exit if cheat mode is on / not NPC */
	if ((NPC_index == 0xFFFF) && Cheat_mode)
		return TRUE;

	#if FALSE
	/* Is NPC ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Collision with party ? */
		if ((PARTY_DATA.X == X) && (PARTY_DATA.Y == Y))
		{
			/* Yes -> Exit */
			return FALSE;
		}
	}
	#endif

	/* Check if there are any NPCs on this location */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself ? */
		if ((NPC_present(i)) && (i != NPC_index))
		{
			/* Yes -> Collision ? */
			if ((VNPCs[i].Map_X == X) && (VNPCs[i].Map_Y == Y))
			{
				/* Yes -> Exit */
				return FALSE;
			}

			#if FALSE
			if ((VNPCs[i].Next_X == X) && (VNPCs[i].Next_Y == Y))
			{
				/* Yes -> Exit */
				return FALSE;
			}
			#endif
		}
	}

	/* Everything's OK */
	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Sucking_chairs_2D
 * FUNCTION  : Handle sucking chairs in 2D maps.
 * FILE      : 2D_COLL.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.12.94 12:23
 * LAST      : 02.12.94 12:23
 * INPUTS    : struct Movement_2D *Move - Pointer to movement data.
 * RESULT    : BOOLEAN : TRUE (no collision) or FALSE (blocked).
 * BUGS      : No known.
 * NOTES     : - This function may change coordinates, movement vector and
 *              state.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Sucking_chairs_2D(struct Movement_2D *Move)
{
	struct Movement_2D Extra;
	BOOLEAN Update = FALSE, Result = TRUE;
	UNSHORT X, Y;
	UNSHORT Current_state, Left_state = 0, Right_state = 0;
	SISHORT dX, dY;

	/* Get movement vector */
	dX = Move->dX;
	dY = Move->dY;

	/* Exit if no movement */
	if (!dX && !dY)
		return TRUE;

	/* Get destination map coordinates */
	X = Move->X + dX;
	Y = Move->Y + dY;

	/* Copy movement data */
	memcpy(&Extra, Move, sizeof(struct Movement_2D));

	/* Set new coordinates */
	Extra.X = X;
	Extra.Y = Y;

	/* Don't suck ! */
	Extra.Flags |= DONT_SUCK;

	/* Big guy ? */
	if (Big_guy)
	{
		/* Yes -> Get current state */
		Current_state = Move->State;

		/* Get state at destination location and the location to the right */
		Left_state = ((Get_location_status(X, Y, Move->NPC_index) & STATE) >>
		 STATE_B);
		Right_state = ((Get_location_status(X + 1, Y, Move->NPC_index) &
		 STATE) >> STATE_B);

		/* Currently sitting on an east- or west-chair, but the new left foot
		 state is standing ? */
		if (((Current_state == SITTING_W_STATE) || (Current_state == SITTING_E_STATE))
		 && (Left_state == STANDING_STATE))
		{
			/* Yes -> Try to jump off */
			Extra.dX = dX;
			Extra.dY = 0;

			/* Determine jump direction */
			if (dX < 0)
				Extra.Direction = WEST8;
			else
				Extra.Direction = EAST8;

			/* Possible ? */
			if (Try_2D_move(&Extra))
			{
	  			/* Yes -> Set new coordinates */
				Move->X = X + Extra.dX;
				Move->Y = Y + Extra.dY;
				Update = TRUE;

				/* Set states */
				Left_state = Right_state = STANDING_STATE;
			}
			else
			{
				/* No */
				Result = FALSE;
			}

			/* Clear movement vector */
			dX = 0;
			dY = 0;
		}

		/* Left foot is on a sitting-east or -west state ? */
		if ((Left_state == SITTING_E_STATE) || (Left_state == SITTING_W_STATE))
		{
			/* Yes -> Just jump */
			Move->X = X;
			Move->Y = Y;
			dX = 0;
			dY = 0;
			Update = TRUE;
		}

		/* Right foot is on a sitting-east or -west state ? */
		if ((Right_state == SITTING_E_STATE) || (Right_state == SITTING_W_STATE))
		{
			/* Yes -> Jump right */
			Move->X = X + 1;
			Move->Y = Y;
			dX = 0;
			dY = 0;
			Update = TRUE;

			/* Set state */
			Left_state = Right_state;
		}

		/* Is a foot on the correct north- or south-state ? */
		if ((Left_state == SITTING_NL_STATE) || (Right_state == SITTING_NR_STATE)
		 || (Left_state == SITTING_SL_STATE) || (Right_state == SITTING_SR_STATE))
		{
			/* Yes -> Just jump */
			Move->X = X;
			Move->Y = Y;
			dX = 0;
			dY = 0;
			Update = TRUE;
		}
		else
		{
			/* No -> Currently sitting on a north- or south-chair ? */
			if ((Current_state == SITTING_NL_STATE) || (Current_state == SITTING_NR_STATE)
			 || (Current_state == SITTING_SL_STATE) || (Current_state == SITTING_SR_STATE))
			{
				/* Yes -> New left or right foot state is standing ? */
				if ((Left_state == STANDING_STATE) || (Right_state == STANDING_STATE))
				{
					/* Yes -> Try to jump off */
					Extra.dX = dX;
					Extra.dY = 0;

					/* Determine jump direction */
					if (dX < 0)
						Extra.Direction = WEST8;
					else
						Extra.Direction = EAST8;

					/* Possible ? */
					if (Try_2D_move(&Extra))
					{
						/* Yes -> Set new coordinates */
						Move->X = X + Extra.dX;
						Move->Y = Y + Extra.dY;
						Update = TRUE;

						/* Set states */
						Left_state = Right_state = STANDING_STATE;
					}
					else
					{
						/* No */
						Result = FALSE;
					}

					/* Clear movement vector */
					dX = 0;
					dY = 0;
				}
			}

			/* Moving right or vertically and the right foot is on a
			 sitting-left state ? */
			if (((dX > 0) || (dY)) && ((Right_state == SITTING_NL_STATE)
			 || (Right_state == SITTING_SL_STATE)))
			{
				/* Yes -> Jump right */
				Move->X = X + 1;
				Move->Y = Y;
				dX = 0;
				dY = 0;
				Update = TRUE;

				/* Set states */
				Left_state = Right_state;
				Right_state++;
			}

			/* Moving left or vertically and the left foot is on a
			 sitting-right state ? */
			if (((dX < 0) || (dY)) && ((Left_state == SITTING_NR_STATE)
			 || (Left_state == SITTING_SR_STATE)))
			{
				/* Yes -> Jump left */
				Move->X = X - 1;
				Move->Y = Y;
				dX = 0;
				dY = 0;
				Update = TRUE;

				/* Set states */
				Right_state = Left_state;
				Left_state--;
			}
		}
	}

	/* Store new state */
	Move->State = Left_state;

	/* Set flag if update is needed */
	if (Update)
		Move->Flags |= UPDATE_AFTER_MOVE;

	/* Store new movement vector */
	Move->dX = dX;
	Move->dY = dY;

	return Result;
}

