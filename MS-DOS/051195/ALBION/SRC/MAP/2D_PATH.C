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
#include <string.h>

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

#define PARTY_PATH_LENGTH			(256)
#define INITIAL_PATH_DISTANCE		(10)

/* structure definitions */

struct Member_data_2D {
	MEM_HANDLE Graphics_handle;
	UNSHORT Frame;
};

struct Trail_data_2D {
	UNSHORT Member_nr;
	SISHORT Path_distance;
	UNSHORT Counter;
	SISHORT Path_index;
};

struct Path_entry_2D {
	UNSHORT X, Y;
	UNSHORT View_direction;
	UNSHORT State;
};

/* prototypes */

BOOLEAN Init_2D_party_member(UNSHORT Member_index);
void Exit_2D_party_member(UNSHORT Member_index);

void Init_2D_trail_data(void);

/* global variables */

BOOLEAN Hide_party = FALSE;

UNLONG Party_object_X, Party_object_Y;

static struct Member_data_2D M2_Party[6];
static struct Trail_data_2D M2_Trail_data[6];

UNSHORT Active_member_state;

static UNSHORT Nr_2D_trail_entries;

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
 * LAST      : 04.10.95 15:36
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
	BOOLEAN Result;
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

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Initialize */
			Result = Init_2D_party_member(i + 1);
			if (!Result)
			{
				return FALSE;
			}
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
 * LAST      : 04.10.95 15:34
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

	/* Destroy graphics */
	for (i=0;i<6;i++)
	{
		Exit_2D_party_member(i + 1);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_party_member
 * FUNCTION  : Initialize a 2D map party member.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 15:32
 * LAST      : 04.10.95 15:32
 * INPUTS    : UNSHORT Member_index - Party member index (1...6).
 * RESULT    : BOOLEAN : Success.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_2D_party_member(UNSHORT Member_index)
{
	struct Character_data *Char;
	struct Gfx_header *Gfx;
	MEM_HANDLE Handle;
	UNSHORT Gfx_file_type;
	UNSHORT Nr;

	/* Determine party graphics type */
	if (Big_guy)
		Gfx_file_type = PARTYGR_GFX;
	else
		Gfx_file_type = PARTYKL_GFX;

	/* Get 2D graphics number */
	Char = (struct Character_data *)	MEM_Claim_pointer(Party_char_handles[Member_index - 1]);

	Nr = (UNSHORT) Char->Graphics_2D_nr;

	MEM_Free_pointer(Party_char_handles[Member_index - 1]);

	/* Load graphics */
	Handle = Load_subfile(Gfx_file_type, Nr);

	if (!Handle)
	{
		Error(ERROR_FILE_LOAD);
		Exit_program();
		return FALSE;
	}

	/* Store graphics handle */
	M2_Party[Member_index - 1].Graphics_handle		= Handle;
	Party_objects[Member_index - 1].Graphics_handle	= Handle;

	/* Get graphics header */
	Gfx = (struct Gfx_header *) MEM_Claim_pointer(Handle);

	/* Set 2D object data */
	Party_objects[Member_index - 1].Width	= Gfx->Width;
	Party_objects[Member_index - 1].Height	= Gfx->Height;
	Party_objects[Member_index - 1].Level	= 0;

	MEM_Free_pointer(Handle);

	/* Set random start frame */
	M2_Party[Member_index - 1].Frame = rand() % NR_PARTY_ANIMFRAMES;

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_2D_party_member
 * FUNCTION  : Exit a 2D map party member.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 15:33
 * LAST      : 04.10.95 15:40
 * INPUTS    : UNSHORT Member_index - Member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_party_member(UNSHORT Member_index)
{
	/* Anything there ? */
	if (M2_Party[Member_index - 1].Graphics_handle)
	{
		/* Yes -> Free graphics memory */
		MEM_Free_memory(M2_Party[Member_index - 1].Graphics_handle);
		M2_Party[Member_index - 1].Graphics_handle = NULL;
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
 * LAST      : 04.10.95 16:26
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
	UNLONG X, Y;
	UNSHORT VD;
	UNSHORT i;

	/* Initialise trail */
	Init_2D_trail_data();

	/* Calculate initial party coordinates */
	Map_to_2D
	(
		PARTY_DATA.X,
		PARTY_DATA.Y,
		&Party_object_X,
		&Party_object_Y
	);

	/* Get view direction */
	VD = PARTY_DATA.View_direction;

	/* Initialize non-position party path data */
	for (i=0;i<PARTY_PATH_LENGTH;i++)
	{
		Party_path[i].View_direction	= VD;
		Party_path[i].State				= STANDING_STATE;
	}

	/* Initialize position party path data */
	X = Party_object_X;
	Y = Party_object_Y;

	for (i=0;i<PARTY_PATH_LENGTH;i++)
	{
		if (i < 5 * 16)
		{
			X -= Offsets4[VD][0];
			Y -= Offsets4[VD][1];
		}

		Party_path[PARTY_PATH_LENGTH - i - 1].X = X;
		Party_path[PARTY_PATH_LENGTH - i - 1].Y = Y;
	}

	Party_path[0].X = Party_object_X;
	Party_path[0].Y = Party_object_Y;

	/* Reset */
	_2D_steps_left = 0;
	Party_path_index = 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_trail_data
 * FUNCTION  : Init the 2D map trail data.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 14:33
 * LAST      : 06.10.95 19:49
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_trail_data(void)
{
	UNSHORT i;

	/* No trail entries right now */
	Nr_2D_trail_entries = 0;

	/* Any active member ? */
	if (PARTY_DATA.Active_member != 0xFFFF)
	{
		/* Yes -> Create trail entry for active member */
		M2_Trail_data[0].Member_nr			= PARTY_DATA.Active_member;
		M2_Trail_data[0].Path_index		= 0;
		M2_Trail_data[0].Path_distance	= 0;
		M2_Trail_data[0].Counter			= 0;

		/* Count up */
		Nr_2D_trail_entries++;
	}

	/* Check other party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there / not the active member / alive ? */
		if ((Member_present(i + 1)) &&
		 (PARTY_DATA.Active_member != i + 1) &&
		 Character_alive(Party_char_handles[i]))
		{
			/* Yes -> Create trail entry */
			M2_Trail_data[Nr_2D_trail_entries].Member_nr			= i + 1;
			M2_Trail_data[Nr_2D_trail_entries].Path_index		= PARTY_PATH_LENGTH - (Nr_2D_trail_entries * INITIAL_PATH_DISTANCE);
			M2_Trail_data[Nr_2D_trail_entries].Path_distance	= (4 * _2D_scale) + (rand() % 3) - 1;
			M2_Trail_data[Nr_2D_trail_entries].Counter			= 4 + (rand() & 0x001F);

			/* Count up */
			Nr_2D_trail_entries++;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_member_to_2D_trail
 * FUNCTION  : Add a party member to the 2D map trail.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 14:43
 * LAST      : 04.10.95 15:39
 * INPUTS    : UNSHORT Member_nr - New party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_member_to_2D_trail(UNSHORT Member_nr)
{
	BOOLEAN Result;
	SISHORT Path_index;

	/* Is the trail not empty / is there room in the trail /
	  does the member exist ? */
	if (Nr_2D_trail_entries &&
	 (Nr_2D_trail_entries < 6) &&
	 Member_present(Member_nr))
	{
		/* Yes -> Initialize party member */
		Result = Init_2D_party_member(Member_nr);
		if (Result)
		{
			/* Create trail entry */
			M2_Trail_data[Nr_2D_trail_entries].Member_nr			= Member_nr;
			M2_Trail_data[Nr_2D_trail_entries].Path_distance	= (4 * _2D_scale) + (rand() % 3) - 1;
			M2_Trail_data[Nr_2D_trail_entries].Counter			= 4 + (rand() & 0x001F);

			/* Determine path index */
			Path_index = M2_Trail_data[Nr_2D_trail_entries - 1].Path_index - INITIAL_PATH_DISTANCE;
			if (Path_index < 0)
				Path_index += PARTY_PATH_LENGTH;

			/* Store it */
			M2_Trail_data[Nr_2D_trail_entries].Path_index = Path_index;

			/* Count up */
			Nr_2D_trail_entries++;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_member_from_2D_trail
 * FUNCTION  : Remove a party member from the 2D map trail.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.10.95 14:49
 * LAST      : 04.10.95 14:49
 * INPUTS    : UNSHORT Member_nr - New party member index (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_member_from_2D_trail(UNSHORT Member_nr)
{
	UNSHORT i;

	/* Are there at least two party members in the trail ? */
	if (Nr_2D_trail_entries > 1)
	{
		/* Yes -> Find this party member's entry */
		for (i=1;i<Nr_2D_trail_entries;i++)
		{
			/* Is this the one ? */
			if (M2_Trail_data[i].Member_nr == Member_nr)
			{
				/* Yes -> Copy all following entries down */
				memmove
				(
					(UNBYTE *) &(M2_Trail_data[i]),
					(UNBYTE *) &(M2_Trail_data[i + 1]),
					(Nr_2D_trail_entries - i - 1) * sizeof(struct Trail_data_2D)
				);

				/* Count down */
				Nr_2D_trail_entries--;

				/* Remove graphics */
				Exit_2D_party_member(Member_nr);

				break;
			}
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
 * LAST      : 12.10.95 22:31
 * INPUTS    : UNSHORT New_active_member_index - Index of new active member
 *              (1...6).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_active_member_2D_path(UNSHORT New_active_member_index)
{
	struct Trail_data_2D Temp;
	SISHORT Path_index;
	SISHORT Current_distance;
	SISHORT Added_distance;
	UNSHORT i;

	/* Are there at least two party members in the trail ? */
	if (Nr_2D_trail_entries > 1)
	{
		/* Yes -> Find this party member's entry */
		for (i=1;i<Nr_2D_trail_entries;i++)
		{
			/* Is this the one ? */
			if (M2_Trail_data[i].Member_nr == New_active_member_index)
			{
				/* Yes -> Move the new active member to the front */
				Path_index = M2_Trail_data[i].Path_index;
				while (Path_index != M2_Trail_data[0].Path_index)
				{
					/* Calculate distance between the old and the new active
					  members */
					if (Path_index <= M2_Trail_data[0].Path_index)
					{
						Current_distance = M2_Trail_data[0].Path_index - Path_index;
					}
					else
					{
						Current_distance = (M2_Trail_data[0].Path_index +
						 PARTY_PATH_LENGTH) - Path_index;
					}

					/* Increase new active member's path index */
					if (Current_distance >= _2D_speed)
					{
						Path_index += _2D_speed;
					}
					else
					{
						Path_index += Current_distance;
					}

					if (Path_index >= PARTY_PATH_LENGTH)
					{
						Path_index -= PARTY_PATH_LENGTH;
					}

					/* Store new active member's path index */
					M2_Trail_data[i].Path_index = Path_index;

					/* Current screen is 2D map ? */
					if (Current_screen_type(0) == MAP_2D_SCREEN)
					{
						/* Yes -> Show */
						Display_2D_map();
						Switch_screens();
					}
				}

				/* Move new active member trail entry to temporary data */
				memmove
				(
					(UNBYTE *) &Temp,
					(UNBYTE *) &(M2_Trail_data[i]),
					sizeof(struct Trail_data_2D)
				);

				/* Copy all following entries down */
				memmove
				(
					(UNBYTE *) &(M2_Trail_data[i]),
					(UNBYTE *) &(M2_Trail_data[i + 1]),
					(Nr_2D_trail_entries - i - 1) * sizeof(struct Trail_data_2D)
				);

				/* Copy everything back up, making place for one entry at
				  the front */
				memmove
				(
					(UNBYTE *) &(M2_Trail_data[1]),
					(UNBYTE *) &(M2_Trail_data[0]),
					(Nr_2D_trail_entries - 1) * sizeof(struct Trail_data_2D)
				);

				/* Insert new active member trail entry at the front */
				memmove
				(
					(UNBYTE *) &(M2_Trail_data[0]),
					(UNBYTE *) &Temp,
					sizeof(struct Trail_data_2D)
				);

				/* Get old active member's path index */
				Path_index = M2_Trail_data[1].Path_index;

				/* Are there more than two members in the trail ? */
				if (Nr_2D_trail_entries > 2)
				{
					/* Yes -> Calculate the distance between the old active
					  member and the next party member */
					if (M2_Trail_data[2].Path_index <= Path_index)
					{
						Current_distance = Path_index - M2_Trail_data[2].Path_index;
					}
					else
					{
						Current_distance = (Path_index + PARTY_PATH_LENGTH) -
						 M2_Trail_data[2].Path_index;
					}

					/* Push the old active member back half this distance */
					Added_distance = Current_distance / 2;
				}
				else
				{
					/* No -> Push the old active member back a fixed distance */
					Added_distance = 16;
				}

				/* Push back */
				Path_index -= Added_distance;
				if (Path_index < 0)
				{
					Path_index += PARTY_PATH_LENGTH;
				}

				/* Store new path index */
				M2_Trail_data[1].Path_index = Path_index;
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_2D_party_position
 * FUNCTION  : Update the positions of the party objects in the 2D map.
 * FILE      : 2D_PATH.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.11.94 12:54
 * LAST      : 04.10.95 14:41
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
	SISHORT Active_index;
	SISHORT Previous_index;
	SISHORT Current_index;
	UNSHORT i;

	/* Get active member's path index */
	Active_index = M2_Trail_data[0].Path_index;

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
	M2_Trail_data[0].Path_index = Active_index;

	/* Set current party object coordinates */
	Party_object_X = Party_path[Active_index].X;
	Party_object_Y = Party_path[Active_index].Y;

	/* Follow the leader */
	Previous_index = Active_index;

	/* Handle other party members */
	for (i=1;i<Nr_2D_trail_entries;i++)
	{
		/* Yes -> Moving ? */
		if (_2D_steps_left > 0)
		{
			/* Yes -> Time to change the path distance ? */
			if (!M2_Trail_data[i].Counter)
			{
				/* Yes -> Set new path distance */
				M2_Trail_data[i].Path_distance = (4 * _2D_scale) + (rand() % 3) - 1;

				/* Reset counter */
				M2_Trail_data[i].Counter = 4 + (rand() & 0x001F);
			}
			else
			{
				/* No -> Count down */
				M2_Trail_data[i].Counter--;
			}
		}

		/* Get path index */
		Current_index = M2_Trail_data[i].Path_index;

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
		Max_distance = M2_Trail_data[i].Path_distance;
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
			M2_Trail_data[i].Path_index = Current_index;
		}

		/* Follow me */
		Previous_index = Current_index;
	}
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
			Make_2D_path_entry
			(
				X + (dX * i),
				Y + (dY * i)
			);
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
	Make_2D_path_entry
	(
		Party_path[Party_path_index].X,
		Party_path[Party_path_index].Y
	);
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
 * LAST      : 04.10.95 14:57
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
	UNSHORT Path_index;
	UNSHORT Frame;
	UNSHORT Member_nr;
	UNSHORT i;
	SISHORT dX, dY;

	/* Is the party hidden ? */
	if (!Hide_party)
	{
		/* No -> Handle party member objects */
		for (i=0;i<Nr_2D_trail_entries;i++)
		{
			/* Get path index */
			Path_index = M2_Trail_data[i].Path_index;

			/* Get party member index */
			Member_nr = M2_Trail_data[i].Member_nr;

			/* Determine if this object has moved */
			dX = Party_objects[Member_nr - 1].X - Party_path[Path_index].X;
			dY = Party_objects[Member_nr - 1].Y - Party_path[Path_index].Y;

			/* Set object coordinates */
			Party_objects[Member_nr - 1].X = Party_path[Path_index].X;
			Party_objects[Member_nr - 1].Y = Party_path[Path_index].Y;

			/* Determine animation frame depending on state */
			switch (Party_path[Path_index].State)
			{
				/* "Special" states */
				case SITTING_NL_STATE:
				{
					Frame = 12;
					Party_objects[Member_nr - 1].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_NR_STATE:
				{
					Frame = 12;
					Party_objects[Member_nr - 1].X -= 16;
					Party_objects[Member_nr - 1].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_E_STATE:
				{
					Frame = 13;
					Party_objects[Member_nr - 1].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_SL_STATE:
				{
					Frame = 14;
					Party_objects[Member_nr - 1].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_SR_STATE:
				{
					Frame = 14;
					Party_objects[Member_nr - 1].X -= 16;
					Party_objects[Member_nr - 1].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				case SITTING_W_STATE:
				{
					Frame = 15;
					Party_objects[Member_nr - 1].Flags |= ADD_ICONS_OVER_OBJECT;
					break;
				}
				/* Normal state : walking */
				default:
				{
					/* Set frame depending on view direction and animation phase */
					Frame = (Party_path[Path_index].View_direction * 3) +
					 Party_animation[M2_Party[Member_nr - 1].Frame];

					Party_objects[Member_nr - 1].Flags &= ~ADD_ICONS_OVER_OBJECT;

					/* Is this the active member ? */
					if (Member_nr != PARTY_DATA.Active_member)
					{
						/* No -> Move the object up so it will not appear on top */
						Party_objects[Member_nr - 1].Y -= 1;
						dY += 1;
					}

					break;
				}
			}

			/* Set graphics offset */
			Party_objects[Member_nr - 1].Graphics_offset = Frame *
			 (Party_objects[Member_nr - 1].Width	*
			 Party_objects[Member_nr - 1].Height) + sizeof(struct Gfx_header);

			/* Add party object to list */
			Add_2D_object(&Party_objects[Member_nr - 1]);

			/* Did this object move ? */
			if (dX || dY)
			{
				/* Yes -> Next animation frame */
				M2_Party[Member_nr - 1].Frame = (M2_Party[Member_nr - 1].Frame + 1) %
				 NR_PARTY_ANIMFRAMES;
			}
		}
	}
}

