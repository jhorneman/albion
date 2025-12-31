/************
 * NAME     : 2D_PATH.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 8-8-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <stdlib.h>

#include <BBDEF.H>

#include <GFXFUNC.H>

#include <MAP.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <2D_COLL.H>
#include <2D_PATH.H>
#include <2D_DISPL.H>
#include <PRTLOGIC.H>
#include <NPCS.H>
#include <GAMETIME.H>
#include <XFTYPES.H>

/* defines */

#define PARTY_PATH_LENGTH		(256)

/* structure definitions */

struct Member_data_2D {
	MEM_HANDLE Graphics_handle;
	UNSHORT Path_distance;
	UNSHORT Counter;
	UNSHORT Path_index;
	UNSHORT Frame;
};

struct Path_entry_2D {
	UNSHORT X, Y;
	UNSHORT View_direction;
	UNSHORT State;
};

/* prototypes */

/* global variables */

BOOLEAN Hide_party = FALSE;

UNLONG Party_object_X, Party_object_Y;

static struct Member_data_2D M2_Party[6];

UNSHORT Active_member_state;

struct Object_2D Party_objects[6];

static struct Path_entry_2D Party_path[PARTY_PATH_LENGTH];

UNSHORT Party_animation[NR_PARTY_ANIMFRAMES] = {
	1, 1, 1, 2, 2, 2, 1, 1, 1, 0, 0, 0
};

UNSHORT _2D_steps_left;
UNSHORT _2D_speed;
UNSHORT _2D_scale;

static UNSHORT Party_path_index;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_party_path
 * FUNCTION  : Initialize the 2D map party and path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 15:11
 * LAST      : 08.08.95 15:11
 * INPUTS    : None.
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_2D_party_path(void)
{
	struct Character_data *Char;
	struct Gfx_header *Gfx;
	MEM_HANDLE Handle;
	UNSHORT Gfx_file_type;
	UNSHORT Nr;
	UNSHORT i;

	/* Set 2D scale factor */
	if (Big_guy)
	{
		_2D_scale = 4;
	}
	else
	{
		_2D_scale = 2;
	}

	/* Set 2D speed */
	_2D_speed = _2D_scale;

	/* Determine party graphics type */
	if (Big_guy)
		Gfx_file_type = PARTYGR_GFX;
	else
		Gfx_file_type = PARTYKL_GFX;

	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Get 2D graphics number */
			Char = (struct Character_data *)
			 MEM_Claim_pointer(Party_char_handles[i]);

			Nr = (UNSHORT) Char->Graphics_2D_nr;

			MEM_Free_pointer(Party_char_handles[i]);

			/* Load graphics */
			Handle = Load_subfile(Gfx_file_type, Nr);

			if (!Handle)
			{
				Error(ERROR_FILE_LOAD);
				Exit_program();
				return FALSE;
			}

			/* Store graphics handle */
			M2_Party[i].Graphics_handle = Handle;
			Party_objects[i].Graphics_handle	= Handle;

			/* Get graphics header */
			Gfx = (struct Gfx_header *) MEM_Claim_pointer(Handle);

			/* Set 2D object data */
			Party_objects[i].Width	= Gfx->Width;
			Party_objects[i].Height	= Gfx->Height;
			Party_objects[i].Level	= 0;

			MEM_Free_pointer(Handle);

			/* Set random start frame */
			M2_Party[i].Frame = rand() % NR_PARTY_ANIMFRAMES;
		}
		else
		{
			/* No -> Clear graphics handle */
			M2_Party[i].Graphics_handle = NULL;
		}
	}

	/* Reset path */
	Reset_2D_path();

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_2D_party_path
 * FUNCTION  : Exit the 2D map party and path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.08.95 15:12
 * LAST      : 08.08.95 15:33
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_party_path(void)
{
	UNSHORT i;

	/* Destroy 2D graphics */
	for (i=0;i<6;i++)
	{
		if (M2_Party[i].Graphics_handle)
		{
			MEM_Free_memory(M2_Party[i].Graphics_handle);
			M2_Party[i].Graphics_handle = NULL;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_2D_path
 * FUNCTION  : Reset the 2D map path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.95 19:06
 * LAST      : 09.08.95 19:06
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_2D_path(void)
{
	UNSHORT i;

	/* Reset party member data */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Reset path data */
			M2_Party[i].Path_index		= 0;
			M2_Party[i].Path_distance	= 0;
		}
	}

	/* Calculate initial party coordinates */
	Map_to_2D(PARTY_DATA.X, PARTY_DATA.Y, &Party_object_X, &Party_object_Y);

	/* Initialize party path */
	for (i=0;i<PARTY_PATH_LENGTH;i++)
	{
		Party_path[i].X					= Party_object_X;
		Party_path[i].Y					= Party_object_Y;
		Party_path[i].View_direction	= PARTY_DATA.View_direction;
		Party_path[i].State				= STANDING_STATE;
	}

	/* Reset */
	_2D_steps_left = 0;
	Party_path_index = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_party_position
 * FUNCTION  : Update the positions of the party objects in the 2D map.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.11.94 12:54
 * LAST      : 10.08.95 12:01
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function makes sure the other party members follow
 *              the active member.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_2D_party_position(void)
{
	SISHORT Current_distance;
	SISHORT Max_distance;
	SISHORT Diff;
	UNSHORT Active_index;
	UNSHORT Previous_index;
	UNSHORT Current_index;
	UNSHORT i;

	/* Get active member's path index */
	Active_index = M2_Party[PARTY_DATA.Active_member - 1].Path_index;

	/* Calculate distance between active member's and party path index */
	if (Active_index <= Party_path_index)
	{
		Current_distance = Party_path_index - Active_index;
	}
	else
	{
		Current_distance = (Party_path_index + PARTY_PATH_LENGTH) -
		 Active_index;
	}

	/* Increase active member's path index */
	if (Current_distance >= _2D_speed)
	{
		Active_index += _2D_speed;
	}
	else
	{
		Active_index += Current_distance;
	}

	if (Active_index >= PARTY_PATH_LENGTH)
	{
		Active_index -= PARTY_PATH_LENGTH;
	}

	/* Store active member's path index */
	M2_Party[PARTY_DATA.Active_member - 1].Path_index = Active_index;

	/* Set current party object coordinates */
	Party_object_X = Party_path[Active_index].X;
	Party_object_Y = Party_path[Active_index].Y;

	/* Follow the leader */
	Previous_index = Active_index;

	/* Handle other party members */
	for (i=0;i<6;i++)
	{
		/* Anyone here / not the active member ? */
		if ((PARTY_DATA.Member_nrs[i]) && (i != PARTY_DATA.Active_member - 1))
		{
			/* Yes -> Moving ? */
			if (_2D_steps_left > 0)
			{
				/* Yes -> Time to change the path distance ? */
				if (!M2_Party[i].Counter)
				{
					/* Yes -> Set new path distance */
					M2_Party[i].Path_distance = (4 * _2D_scale) + (rand() % 3) - 1;

					/* Reset counter */
					M2_Party[i].Counter = 4 + (rand() & 0x001F);
				}
				else
				{
					/* No -> Count down */
					M2_Party[i].Counter--;
				}
			}

			/* Get path index */
			Current_index = M2_Party[i].Path_index;

			/* Calculate distance to predecessor */
			if (Previous_index >= Current_index)
			{
				Current_distance = Previous_index - Current_index;
			}
			else
			{
				Current_distance = (Previous_index + PARTY_PATH_LENGTH) -
				 Current_index;
			}

			/* Get maximum distance */
			Max_distance = M2_Party[i].Path_distance;
			if (_2D_steps_left)
			{
				Max_distance = (3 * Max_distance) / 2;
			}

			/* Time to catch up ? */
			if (Current_distance > Max_distance)
			{
				/* Yes -> Calculate difference */
				Diff = Current_distance - Max_distance;

				/* Greater than movement speed ? */
				if (Diff > _2D_speed)
				{
					/* Yes -> Hurry up */
					Current_index += _2D_speed;
				}
				else
				{
					/* No -> Just do it */
					Current_index += Diff;
				}

				/* Check for overflow */
				if (Current_index >= PARTY_PATH_LENGTH)
				{
					Current_index -= PARTY_PATH_LENGTH;
				}

				/* Store new path index */
				M2_Party[i].Path_index = Current_index;
			}

			/* Follow me */
			Previous_index = Current_index;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_active_member_2D_path
 * FUNCTION  : Change the active member in the 2D map path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.08.95 19:22
 * LAST      : 09.08.95 19:22
 * INPUTS    : UNSHORT Old_active_member_index - Index of old active member
 *              (1...6).
 *             UNSHORT New_active_member_index - Index of new active member
 *              (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_active_member_2D_path(UNSHORT Old_active_member_index,
 UNSHORT New_active_member_index)
{
	UNSHORT Old_member_path_index;
	UNSHORT New_member_path_index;

	/* Get path indices of old and new active members */
	Old_member_path_index = M2_Party[Old_active_member_index - 1].Path_index;
	New_member_path_index = M2_Party[New_active_member_index - 1].Path_index;

	// etc. etc.

}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Store_2D_movement
 * FUNCTION  : Store a map-square movement in the 2D path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.11.94 21:42
 * LAST      : 10.08.95 13:22
 * INPUTS    : SISHORT dX - X-component of movement vector.
 *             SISHORT dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Exactly 16 steps will be taken.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Store_2D_movement(SISHORT dX, SISHORT dY)
{
	UNLONG X, Y;
	UNSHORT i;

	/* Any movement ? */
	if (dX || dY)
	{
		/* Yes -> Convert party coordinates */
		Map_to_2D(PARTY_DATA.X, PARTY_DATA.Y, &X, &Y);

		/* Take 16 steps */
		for (i=1;i<=16;i++)
		{
			/* Make a path entry */
			Make_2D_path_entry(X + (dX * i), Y + (dY * i));
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Store_2D_current_position
 * FUNCTION  : Store a the current position in the 2D path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.95 15:50
 * LAST      : 10.08.95 15:50
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function is needed for turning on the spot.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Store_2D_current_position(void)
{
	/* Store the current position */
	Make_2D_path_entry(Party_path[Party_path_index].X,
	 Party_path[Party_path_index].Y);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_2D_path_entry
 * FUNCTION  : Make a single entry in the 2D path.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.07.95 16:44
 * LAST      : 10.08.95 13:19
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_2D_path_entry(UNSHORT X, UNSHORT Y)
{
	UNSHORT Index;

	/* Get party path index */
	Index = Party_path_index;

	/* Increase path index */
	Index++;
	if (Index >= PARTY_PATH_LENGTH)
		Index -= PARTY_PATH_LENGTH;

	/* Store coordinates and view direction */
	Party_path[Index].X					= X;
	Party_path[Index].Y					= Y;
	Party_path[Index].View_direction	= PARTY_DATA.View_direction;
	Party_path[Index].State				= Active_member_state;

	/* Store new party path index */
	Party_path_index = Index;

	/* Another step must be taken */
	_2D_steps_left += 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_party
 * FUNCTION  : Display party members in a 2D map.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.11.94 22:00
 * LAST      : 13.08.95 14:24
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_party(void)
{
	UNSHORT Index;
	UNSHORT Frame;
	UNSHORT i;
	SISHORT dX, dY;

	/* Is the party hidden ? */
	if (!Hide_party)
	{
		/* No -> Handle party member objects */
		for (i=0;i<6;i++)
		{
			/* Anyone there ? */
			if (Member_present(i + 1))
			{
				/* Yes -> Get path index */
				Index = M2_Party[i].Path_index;

				/* Determine if this object has moved */
				dX = Party_objects[i].X - Party_path[Index].X;
				dY = Party_objects[i].Y - Party_path[Index].Y;

				/* Set object coordinates */
				Party_objects[i].X = Party_path[Index].X;
				Party_objects[i].Y = Party_path[Index].Y;

				/* Determine animation frame depending on state */
				switch (Party_path[Index].State)
				{
					/* "Special" states */
					case SITTING_NL_STATE:
					{
						Frame = 12;
						Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					}
					case SITTING_NR_STATE:
					{
						Frame = 12;
						Party_objects[i].X -= 16;
						Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					}
					case SITTING_E_STATE:
					{
						Frame = 13;
						Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					}
					case SITTING_SL_STATE:
					{
						Frame = 14;
						Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					}
					case SITTING_SR_STATE:
					{
						Frame = 14;
						Party_objects[i].X -= 16;
						Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					}
					case SITTING_W_STATE:
					{
						Frame = 15;
						Party_objects[i].Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					}
					/* Normal state : walking */
					default:
					{
						/* Set frame depending on view direction and animation phase */
						Frame = (Party_path[Index].View_direction * 3) +
						 Party_animation[M2_Party[i].Frame];

						Party_objects[i].Flags &= ~ADD_ICONS_OVER_OBJECT;

						/* Is this the active member ? */
						if (i != PARTY_DATA.Active_member - 1)
						{
							/* No -> Slight correction */
							Party_objects[i].Y--;
							dY++;
						}

						break;
					}
				}

				/* Set graphics offset */
				Party_objects[i].Graphics_offset = Frame * (Party_objects[i].Width
				 * Party_objects[i].Height) + sizeof(struct Gfx_header);

				/* Add party object to list */
				Add_2D_object(&Party_objects[i]);

				/* Did this object move ? */
				if (dX || dY)
				{
					/* Yes -> Next animation frame */
					M2_Party[i].Frame = (M2_Party[i].Frame + 1) % NR_PARTY_ANIMFRAMES;
				}
			}
		}
	}
}

