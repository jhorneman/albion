/************
 * NAME     : NPCS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 22-11-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBMEM.H>
#include <BBDSA.H>

#include <XLOAD.H>

#include <CONTROL.H>
#include <MAP.H>
#include <NPCS.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <3D_MAP.H>
#include <3DCOMMON.H>
#include <XFTYPES.H>
#include <EVELOGIC.H>
#include <POPUP.H>
#include <GAMETIME.H>
#include <GFXFUNC.H>

/* global variables */

/* Extended NPC data */
struct VNPC_data VNPCs[NPCS_PER_MAP];

/* 3D NPC array */
struct NPC_3D NPCs_3D[NPCS_PER_MAP];
UNSHORT Nr_3D_NPCs;

static UNSHORT Last_NPC_step;

BOOLEAN Monster_is_watching;

static SISHORT Random_course_changes[19] = {
	0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5, 5, 6, 6, 6, 7, 7, 7
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_NPC_present_status
 * FUNCTION  : Update the present flags of all NPCs in the current map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.05.95 15:19
 * LAST      : 31.05.95 14:57
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function should be called after the NPCs have been
 *              initialized and whenever the CD bit-array is changed.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_NPC_present_status(void)
{
	UNSHORT i;

	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> Present ? */
			VNPCs[i].Present = !(Read_bit_array(CD_BIT_ARRAY,
			 ((PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP) + i));
		}
		else
		{
			/* No -> Absent */
			VNPCs[i].Present = FALSE;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Skip_through_NPC_paths
 * FUNCTION  : Skip through NPC path data in a map data file.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.05.95 14:47
 * LAST      : 27.05.95 14:47
 * INPUTS    : struct Map_data *Map_ptr - Pointer to map data.
 *             UNLONG Offset - Offset to start of NPC path data.
 * RESULT    : UNLONG : Offset to end of NPC path data.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Skip_through_NPC_paths(struct Map_data *Map_ptr, UNLONG Offset)
{
	union NPC_data *NPC_ptr;
	BOOLEAN New_NPC_data_flag = FALSE;
	UNSHORT Nr_NPCs;
	UNSHORT i;

	/* Get number of NPCs in this map */
	if (Map_ptr->Flags & MORE_NPCS)
		Nr_NPCs = 96;
	else
		Nr_NPCs = 32;

	/* Old or new NPC data format ? */
	if (Map_ptr->Flags & NEW_NPC_FORMAT)
		New_NPC_data_flag = TRUE;

	/* Skip through NPC paths */
	NPC_ptr = (union NPC_data *) (Map_ptr + 1);
	for (i=0;i<Nr_NPCs;i++)
	{
		/* Old or new NPC data format ? */
		if (New_NPC_data_flag)
		{
			/* New -> Anyone there ? */
			if (NPC_ptr[i].New_NPC_data.Number)
			{
				/* Yes -> Does this NPC move along a path ? */
				if ((NPC_ptr[i].New_NPC_data.Movement_type == ABS_PATH_MOVEMENT)
				 || (NPC_ptr[i].New_NPC_data.Movement_type == REL_PATH_MOVEMENT))
				{
					/* Yes -> Skip path */
					Offset += NPC_PATH_LENGTH * 2;
				}
				else
				{
					/* No -> Skip coordinate pair */
					Offset += 2;
				}
			}
		}
		else
		{
			/* Old -> Anyone there ? */
			if (NPC_ptr[i].Old_NPC_data.Number)
			{
				/* Yes -> Does this NPC move along a path ? */
				if (((NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_MOVEMENT_TYPE) >>
				 OLD_NPC_MOVEMENT_TYPE_B) == ABS_PATH_MOVEMENT)
				{
					/* Yes -> Skip path */
					Offset += NPC_PATH_LENGTH * 2;
				}
				else
				{
					/* No -> Skip coordinate pair */
					Offset += 2;
				}
			}
		}
	}

	return Offset;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_NPC_data
 * FUNCTION  : Initialize NPC data.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.05.95 14:28
 * LAST      : 27.05.95 14:28
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_NPC_data(void)
{
	struct Map_data *Map_ptr;
	union NPC_data *NPC_ptr;
	BOOLEAN New_NPC_data_flag = FALSE;
	UNLONG Offset;
	UNSHORT Nr_NPCs;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clear NPC data */
	BASEMEM_FillMemByte((UNBYTE *) &(VNPCs[0]), NPCS_PER_MAP *
	 sizeof(struct VNPC_data), 0);

	/* Clear flag */
	Monster_is_watching = FALSE;

	/* Remember the time */
	Last_NPC_step = Current_step;

	/* Get NPC path / position data */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);
	NPC_ptr = (union NPC_data *) (Map_ptr + 1);
	Offset = NPC_path_offset;

	/* Get number of NPCs in this map */
	if (Map_ptr->Flags & MORE_NPCS)
		Nr_NPCs = 96;
	else
		Nr_NPCs = 32;

	/* Old or new NPC data format ? */
	if (Map_ptr->Flags & NEW_NPC_FORMAT)
		New_NPC_data_flag = TRUE;

	/* Initialize all NPCs */
	for (i=0;i<Nr_NPCs;i++)
	{
		/* Default is absent */
		VNPCs[i].Number = 0;

		/* Old or new NPC data format ? */
		if (New_NPC_data_flag)
		{
			/* New -> Anyone there ? */
			if (NPC_ptr[i].New_NPC_data.Number)
			{
				/* Yes -> Copy new NPC data */
				VNPCs[i].Number = (UNSHORT) NPC_ptr[i].New_NPC_data.Number;
				VNPCs[i].Graphics_nr = NPC_ptr[i].New_NPC_data.Graphics_nr;
				VNPCs[i].NPC_type = (UNSHORT) NPC_ptr[i].New_NPC_data.Flags &
				 NEW_NPC_TYPE;
				VNPCs[i].Travel_mode = NPC_ptr[i].New_NPC_data.Travel_mode;
				VNPCs[i].Trigger_modes = NPC_ptr[i].New_NPC_data.Trigger_modes;
				VNPCs[i].First_block_nr = NPC_ptr[i].New_NPC_data.First_block_nr;
				VNPCs[i].Movement_type =
				 (UNSHORT) NPC_ptr[i].New_NPC_data.Movement_type;

				/* Copy new flags */
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_CAN_TRIGGER)
					VNPCs[i].Flags |= NPC_CAN_TRIGGER;
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_SHORT_DIALOGUE)
					VNPCs[i].Flags |= NPC_SHORT_DIALOGUE;
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_ANIMATE_ALWAYS)
					VNPCs[i].Flags |= NPC_ANIMATE_ALWAYS;
			}
		}
		else
		{
			/* Old -> Anyone there ? */
			if (NPC_ptr[i].Old_NPC_data.Number)
			{
				/* Yes -> Copy old NPC data */
				VNPCs[i].Number = (UNSHORT) NPC_ptr[i].Old_NPC_data.Number;
				VNPCs[i].Graphics_nr = NPC_ptr[i].Old_NPC_data.Graphics_nr;
				VNPCs[i].NPC_type = (UNSHORT) NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_TYPE;
				VNPCs[i].Travel_mode = (UNSHORT) NPC_ptr[i].Old_NPC_data.Travel_mode;
				VNPCs[i].Trigger_modes = NPC_ptr[i].Old_NPC_data.Trigger_modes;
				VNPCs[i].First_block_nr = NPC_ptr[i].Old_NPC_data.First_block_nr;
				VNPCs[i].Movement_type = (UNSHORT)
				 ((NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_MOVEMENT_TYPE) >>
				  OLD_NPC_MOVEMENT_TYPE_B);

				/* Copy old flags */
				if (NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_SHORT_DIALOGUE)
					VNPCs[i].Flags |= NPC_SHORT_DIALOGUE;
			}
		}

		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> How does this NPC move? */
			switch (VNPCs[i].Movement_type)
			{
				/* Moving along a path */
				case ABS_PATH_MOVEMENT:
				{
					/* Set path offset */
					VNPCs[i].Path_offset = Offset;

					/* Get initial coordinates */
					Ptr = Offset + (Current_step * 2) + (UNBYTE *) Map_ptr;

					VNPCs[i].Map_X = (UNSHORT) *Ptr;
					VNPCs[i].Map_Y = (UNSHORT) *(Ptr + 1);

					/* Skip path */
					Offset += NPC_PATH_LENGTH * 2;

					break;
				}
				case REL_PATH_MOVEMENT:
				{
					/* Set path offset */
					VNPCs[i].Path_offset = Offset;

					/* Get initial coordinates */
					Ptr = Offset + (UNBYTE *) Map_ptr;

					VNPCs[i].Map_X = (UNSHORT) *Ptr;
					VNPCs[i].Map_Y = (UNSHORT) *(Ptr + 1);

					/* Skip path */
					Offset += NPC_PATH_LENGTH * 2;

					break;
				}
				/* Starting on a set position */
				default:
				{
					/* Get initial coordinates */
					Ptr = Offset + (UNBYTE *) Map_ptr;

					VNPCs[i].Map_X = (UNSHORT) *Ptr;
					VNPCs[i].Map_Y = (UNSHORT) *(Ptr + 1);

					/* Set initial view direction */
					VNPCs[i].Move.View_direction = SOUTH;

					/* Skip position */
					Offset += 2;

					break;
				}
			}

			/* Set next coordinates */
			VNPCs[i].Next_X = VNPCs[i].Map_X;
			VNPCs[i].Next_Y = VNPCs[i].Map_Y;

			/* Set old coordinates */
			VNPCs[i].Old_X = 0xFFFF;
			VNPCs[i].Old_Y = 0xFFFF;
		}
	}
	MEM_Free_pointer(Map_handle);

	/* Update NPC present status */
	Update_NPC_present_status();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_NPCs
 * FUNCTION  : Initialize NPCs in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.11.94 11:37
 * LAST      : 27.05.95 14:17
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_NPCs(void)
{
	struct Gfx_header *Gfx;
	MEM_HANDLE Handle;
	UNSHORT i, j;

	/* Select NPC graphics type */
	if (Big_guy)
		j = NPCGR_GFX;
	else
		j = NPCKL_GFX;

	/* Initialize all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> Set current playfield coordinates */
			VNPCs[i].Source_X = (VNPCs[i].Map_X - 1) * 16;
			VNPCs[i].Source_Y = (VNPCs[i].Map_Y - 1) * 16 + 15;

			/* Initialize 2D movement data structure */
			VNPCs[i].Move.Flags = 0;
			VNPCs[i].Move.Slide_direction = 0xFFFF;
			VNPCs[i].Move.Reverse_slide = 0xFFFF;
			VNPCs[i].Move.Travel_mode = VNPCs[i].Travel_mode;
			VNPCs[i].Move.NPC_index = i;

			/* Load NPC graphics */
			Handle = Load_subfile(j, VNPCs[i].Graphics_nr);

			/* Success ? */
			if (Handle)
			{
				/* Yes -> Set NPC 2D object dimensions */
				Gfx = (struct Gfx_header *) MEM_Claim_pointer(Handle);

				VNPCs[i].NPC_object.Width = Gfx->Width;
				VNPCs[i].NPC_object.Height = Gfx->Height;

				MEM_Free_pointer(Handle);

				/* Set object icon height */
				VNPCs[i].NPC_object.Level = 0;

				/* Set graphics handle */
				VNPCs[i].NPC_object.Graphics_handle = Handle;
			}
			else
			{
				/* No -> Disable NPC */
				VNPCs[i].Number = 0;
			}
		}
	}

	/* Update present status */
	Update_NPC_present_status();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_2D_NPCs
 * FUNCTION  : Exit NPCs in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.04.95 23:40
 * LAST      : 27.05.95 14:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_NPCs(void)
{
	UNSHORT i;

	/* Destroy NPC graphics memory */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> Destroy NPC graphics */
			MEM_Free_memory(VNPCs[i].NPC_object.Graphics_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_3D_NPCs
 * FUNCTION  : Initialize NPCs in a 3D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 20:01
 * LAST      : 27.05.95 14:32
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_3D_NPCs(void)
{
	UNLONG f;
	UNSHORT i;

	/* Get square size in cm */
	f = I3DM.Square_size;

	/* Initialize all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> Convert map to dungeon coordinates */
			Map_to_dungeon(VNPCs[i].Map_X, VNPCs[i].Map_Y, &(VNPCs[i].Source_X),
			 &(VNPCs[i].Source_Y));

			/* Move NPC away from centre of map square */
			VNPCs[i].Source_X -= (f / 2);
			VNPCs[i].Source_Y -= (f / 2);
		}
	}

	/* Update present status */
	Update_NPC_present_status();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_3D_NPCs
 * FUNCTION  : Exit NPCs in a 3D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 20:02
 * LAST      : 03.05.95 20:02
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_3D_NPCs(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_2D_NPCs
 * FUNCTION  : Handle NPCs in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.11.94 12:00
 * LAST      : 22.11.94 12:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_2D_NPCs(void)
{
	SISHORT dX, dY;
	UNLONG Current_X, Current_Y;
	UNLONG Next_X, Next_Y;
	UNSHORT dTime;
	UNSHORT i, j;

	/* Clear flag */
	Monster_is_watching = FALSE;

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
						Do_random_NPC_2D(j);
						break;
					}
					/* Standing around */
					case WAITING_MOVEMENT:
					{
						break;
					}
					/* Chasing the party */
					case CHASING_MOVEMENT:
					{
						Do_chasing_NPC_2D(j);
						break;
					}
					/* Moving on a path */
					case ABS_PATH_MOVEMENT:
					case REL_PATH_MOVEMENT:
					{
						if (!i)
							Do_path_NPC_2D(j);
						break;
					}
				}
			}
		}
	}

	#if FALSE
	/* Handle NPCs on paths */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Moves on path ? */
			if ((VNPCs[j].Movement_type == ABS_PATH_MOVEMENT) ||
			 (VNPCs[j].Movement_type == REL_PATH_MOVEMENT))
			{
				/* Yes -> Handle */
				Do_path_NPC_2D(j);
			}
		}
	}
	#endif

	/* Update NPC positions */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Get current map coordinates */
			Current_X = VNPCs[j].Map_X;
			Current_Y = VNPCs[j].Map_Y;

			/* Get next map coordinates */
			Next_X = VNPCs[j].Next_X;
			Next_Y = VNPCs[j].Next_Y;

			/* Convert to playfield coordinates */
			Current_X = (Current_X - 1) * 16;
			Current_Y = (Current_Y - 1) * 16 + 15;
			Next_X = (Next_X - 1) * 16;
			Next_Y = (Next_Y - 1) * 16 + 15;

			/* Appearing or disappearing ? */
			if ((VNPCs[j].Map_X || VNPCs[j].Map_Y) &&
			 (VNPCs[j].Next_X || VNPCs[j].Next_Y))
			{
				/* No -> Calculate vector */
				dX = Next_X - Current_X;
				dY = Next_Y - Current_Y;

				/* Calculate new coordinates */
				VNPCs[j].Source_X = Current_X + (dX * Current_substep) /
				 SUBSTEPS_PER_STEP;
				VNPCs[j].Source_Y = Current_Y + (dY * Current_substep) /
				 SUBSTEPS_PER_STEP;
			}
			else
			{
				/* Yes -> Disappear */
				VNPCs[j].Source_X = 0;
				VNPCs[j].Source_Y = 0;
			}
		}
	}

	/* Check for reaction with NPCs */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Check for reaction */
			Check_for_NPC_reaction(j);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_NPCs
 * FUNCTION  : Display NPCs in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.11.94 14:40
 * LAST      : 22.11.94 14:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_2D_NPCs(void)
{
	struct Object_2D *Object;
	SISHORT dX, dY;
	UNSHORT i, Frame;

	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (NPC_present(i))
		{
			/* Yes -> Visible ? */
			if (VNPCs[i].Source_X || VNPCs[i].Source_Y)
			{
				/* Yes */
				Object = &(VNPCs[i].NPC_object);

				/* Has this object moved ? */
				dX = Object->X - VNPCs[i].Source_X;
				dY = Object->Y - VNPCs[i].Source_Y;

				/* Set object coordinates */
				Object->X = VNPCs[i].Source_X;
			 	Object->Y = VNPCs[i].Source_Y;

		  		/* Determine animation frame depending on state */
				switch (VNPCs[i].Move.State)
				{
					/* "Special" states */
					case SITTING_NL_STATE:
						Frame = 12;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					case SITTING_NR_STATE:
						Frame = 12;
						Object->X -= 16;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					case SITTING_E_STATE:
						Frame = 13;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					case SITTING_SL_STATE:
						Frame = 14;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					case SITTING_SR_STATE:
						Frame = 14;
						Object->X -= 16;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					case SITTING_W_STATE:
						Frame = 15;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					case SLEEPING_STATE:
						Frame = 16;
						Object->Y -= 6;
						Object->Flags |= ADD_ICONS_OVER_OBJECT;
						break;
					/* Normal state : walking */
					default:
						/* Frame depends on view direction and animation phase */
						Frame = VNPCs[i].Move.View_direction * 3 +
			  			 Party_animation[VNPCs[i].Frame];

						Object->Flags &= ~ADD_ICONS_OVER_OBJECT;

						break;
				}

				/* Set graphics offset */
				Object->Graphics_offset = Frame * (Object->Width * Object->Height)
				 + sizeof(struct Gfx_header);

				/* Add NPC object to list */
				Add_2D_object(Object);

				/* Should we animate ? */
				if (dX || dY)
				{
					/* Yes -> Next animation frame */
					VNPCs[i].Frame = (VNPCs[i].Frame + 1) % 12;
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Handle_3D_NPCs
 * FUNCTION  : Handle NPCs in a 3D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 20:05
 * LAST      : 03.05.95 20:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Handle_3D_NPCs(void)
{
	SISHORT dX, dY;
	UNLONG Current_X, Current_Y;
	UNLONG Next_X, Next_Y;
	UNLONG f;
	UNSHORT dTime;
	UNSHORT i, j;

	/* Clear flag */
	Monster_is_watching = FALSE;

	/* Get square size in cm */
	f = I3DM.Square_size;

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
		  				Do_random_NPC_3D(j);
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

	#if FALSE
	/* Handle NPCs on paths */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Moves on path ? */
			if ((VNPCs[j].Movement_type == ABS_PATH_MOVEMENT) ||
			 (VNPCs[j].Movement_type == REL_PATH_MOVEMENT))
			{
				/* Yes -> Handle */
				Do_path_NPC_3D(j);
			}
		}
	}
	#endif

	/* Update NPC positions */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Get current map coordinates and convert to playfield
			 coordinates */
			Map_to_dungeon(VNPCs[j].Map_X, VNPCs[j].Map_Y, &Current_X,
			 &Current_Y);
			Current_X -= (f / 2);
			Current_Y -= (f / 2);

			/* Get next map coordinates and convert to playfield coordinates */
			Map_to_dungeon(VNPCs[j].Next_X, VNPCs[j].Next_Y, &Next_X,
			 &Next_Y);
			Next_X -= (f / 2);
			Next_Y -= (f / 2);

			/* Appearing or disappearing ? */
			if ((VNPCs[j].Map_X || VNPCs[j].Map_Y) &&
			 (VNPCs[j].Next_X || VNPCs[j].Next_Y))
			{
				/* No -> Calculate vector */
				dX = Next_X - Current_X;
				dY = Next_Y - Current_Y;

				/* Calculate new coordinates */
				VNPCs[j].Source_X = Current_X + (dX * Current_substep) /
				 SUBSTEPS_PER_STEP;
				VNPCs[j].Source_Y = Current_Y + (dY * Current_substep) /
				 SUBSTEPS_PER_STEP;
			}
			else
			{
				/* Yes -> Disappear */
				VNPCs[j].Source_X = 0;
				VNPCs[j].Source_Y = 0;
			}
		}
	}

	/* Check for reaction with NPCs */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Check for reaction */
			Check_for_NPC_reaction(j);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_3D_NPCs
 * FUNCTION  : Display NPCs in a 3D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 20:05
 * LAST      : 03.05.95 20:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Display_3D_NPCs(void)
{
	UNSHORT i;

	/* Clear counter */
	Nr_3D_NPCs = 0;

	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (NPC_present(i))
		{
			/* Yes -> Visible ? */
			if (VNPCs[i].Source_X || VNPCs[i].Source_Y)
			{
				/* Yes -> Add NPC object to 3D NPC list */
				NPCs_3D[Nr_3D_NPCs].X = VNPCs[i].Source_X;
				NPCs_3D[Nr_3D_NPCs].Z = VNPCs[i].Source_Y;
				/* (+ correction for 3DM bug) */
				NPCs_3D[Nr_3D_NPCs].Group_nr = VNPCs[i].Graphics_nr - 1;
				NPCs_3D[Nr_3D_NPCs].NPC_nr = i + 1;

				/* Count up */
				Nr_3D_NPCs++;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_path_NPC_2D
 * FUNCTION  : Handle an NPC in a 2D map walking along a path.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 14.12.94 16:27
 * LAST      : 01.05.95 16:52
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_path_NPC_2D(UNSHORT NPC_index)
{
	SISHORT dX, dY;
	UNSHORT Current_X, Current_Y;
	UNSHORT Next_X, Next_Y;
	UNSHORT VD;
	UNSHORT Current_state;
	UNSHORT Next_state;
	UNSHORT i;
	UNBYTE *Path_ptr;

	/* Get address of path */
	Path_ptr = MEM_Claim_pointer(Map_handle) + VNPCs[NPC_index].Path_offset;

	/* Absolute or relative path ? */
	if (VNPCs[NPC_index].Movement_type == ABS_PATH_MOVEMENT)
	{
		/* Absolute -> Current path index is step */
		i = Current_step;
	}
	else
	{
		/* Relative -> Get current path index */
		i = VNPCs[NPC_index].Current_path_index;
	}

	/* Get current coordinates */
	Current_X = (UNSHORT) Path_ptr[i * 2];
	Current_Y = (UNSHORT) Path_ptr[i * 2 + 1];

	/* Get next coordinates */
	i++;
	if (i >= NPC_PATH_LENGTH)
		i = 0;
	Next_X = (UNSHORT) Path_ptr[i * 2];
	Next_Y = (UNSHORT) Path_ptr[i * 2 + 1];

	/* Relative path ? */
	if (VNPCs[NPC_index].Movement_type == REL_PATH_MOVEMENT)
	{
		/* Yes -> Store new path index */
		VNPCs[NPC_index].Current_path_index = i;
	}

	/* Store coordinates in NPC data */
	VNPCs[NPC_index].Map_X = Current_X;
	VNPCs[NPC_index].Map_Y = Current_Y;
	VNPCs[NPC_index].Next_X = Next_X;
	VNPCs[NPC_index].Next_Y = Next_Y;

	/* Appearing or disappearing ? */
	if ((Current_X || Current_Y) && (Next_X || Next_Y))
	{
		/* No -> Calculate vector */
		dX = Next_X - Current_X;
		dY = Next_Y - Current_Y;

		/* Set view direction */
		VD = View_directions[sgn(dY) + 1][sgn(dX) + 1];
		if (VD != 0xFFFF)
		{
			VNPCs[NPC_index].Move.View_direction = VD;
		}

		/* Big guy ? */
		if (Big_guy)
		{
			/* Yes -> Get state of current and next position */
			Current_state = (Get_location_status(Current_X, Current_Y, NPC_index)
			 & STATE) >> STATE_B;
			Next_state = (Get_location_status(Next_X, Next_Y, NPC_index)
			 & STATE) >> STATE_B;

			/* Moving from one state to the next ? */
			if (Current_state != Next_state)
			{
				/* Yes -> Jump */
				VNPCs[NPC_index].Map_X = Next_X;
				VNPCs[NPC_index].Map_Y = Next_Y;
			}

			/* Set state */
			VNPCs[NPC_index].Move.State = Next_state;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_random_NPC_2D
 * FUNCTION  : Handle an NPC in a 2D map moving randomly.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.04.95 23:24
 * LAST      : 01.05.95 16:52
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_random_NPC_2D(UNSHORT NPC_index)
{
	BOOLEAN Choose_next_path = TRUE;

	/* Set current to next coordinates */
	VNPCs[NPC_index].Map_X = VNPCs[NPC_index].Next_X;
	VNPCs[NPC_index].Map_Y = VNPCs[NPC_index].Next_Y;

	/* End of path ? */
	if (VNPCs[NPC_index].Rnd_path_length)
	{
		/* No -> Try to move along current path */
		VNPCs[NPC_index].Move.X = VNPCs[NPC_index].Map_X;
		VNPCs[NPC_index].Move.Y = VNPCs[NPC_index].Map_Y;
		VNPCs[NPC_index].Move.Direction = VNPCs[NPC_index].Rnd_path_direction;

		Choose_next_path = !(Try_2D_move(&(VNPCs[NPC_index].Move)));

		/* Store current & next coordinates */
		VNPCs[NPC_index].Map_X = VNPCs[NPC_index].Move.X;
		VNPCs[NPC_index].Map_Y = VNPCs[NPC_index].Move.Y;

		VNPCs[NPC_index].Next_X = VNPCs[NPC_index].Map_X +
		 VNPCs[NPC_index].Move.dX;
		VNPCs[NPC_index].Next_Y = VNPCs[NPC_index].Map_Y +
		 VNPCs[NPC_index].Move.dY;
	}

	/* Choose next path ? */
	while (Choose_next_path)
	{
		/* Yes -> Choose next path */
		VNPCs[NPC_index].Rnd_path_direction =
		 (VNPCs[NPC_index].Rnd_path_direction +
		 Random_course_changes[rand() % 19]) % 8;
		VNPCs[NPC_index].Rnd_path_length = 1 + rand() % 0x0007;

		/* Try to move along new path */
		VNPCs[NPC_index].Move.X = VNPCs[NPC_index].Map_X;
		VNPCs[NPC_index].Move.Y = VNPCs[NPC_index].Map_Y;
		VNPCs[NPC_index].Move.Direction = VNPCs[NPC_index].Rnd_path_direction;

		Choose_next_path = !(Try_2D_move(&(VNPCs[NPC_index].Move)));

		/* Store current & next coordinates */
		VNPCs[NPC_index].Map_X = VNPCs[NPC_index].Move.X;
		VNPCs[NPC_index].Map_Y = VNPCs[NPC_index].Move.Y;

		VNPCs[NPC_index].Next_X = VNPCs[NPC_index].Map_X +
		 VNPCs[NPC_index].Move.dX;
		VNPCs[NPC_index].Next_Y = VNPCs[NPC_index].Map_Y +
		 VNPCs[NPC_index].Move.dY;
	}

	/* Count down */
	VNPCs[NPC_index].Rnd_path_length--;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_chasing_NPC_2D
 * FUNCTION  : Handle an NPC in a 2D map chasing the party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.05.95 16:55
 * LAST      : 01.05.95 16:55
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
	/* Can the NPC see the party ? */
	if (Check_line_of_sight(NPC_index))
	{
		/* Yes -> Gaze! */
		Monster_is_watching = TRUE;



/*
	ori.b	#3,VFlags(a1)		; Set bit 0 & 1
	move.w	d0,d2			; Duplicate coordinates
	move.w	d1,d3
	sub.w	Map_Xcoord,d2		; Calculate dX & dY
	sub.w	Map_Ycoord,d3

	move.w	d2,d4
	bpl.s	.Pos1
	neg.w	d4
.Pos1:

	move.w	d3,d5
	bpl.s	.Pos2
	neg.w	d5
.Pos2:

	ext.l	d2			; Mega SGN function
	beq.s	.Zero1
	swap.w	d2
	bmi.s	.Zero1
	moveq.l	#1,d2
.Zero1:

	ext.l	d3			; Mega SGN function
	beq.s	.Zero2
	swap.w	d3
	bmi.s	.Zero2
	moveq.l	#1,d3
.Zero2:

	sub.w	d5,d4			; dX - dY

	ext.l	d4			; Mega SGN function
	beq.s	.Zero3
	swap.w	d4
	bmi.s	.Zero3
	moveq.l	#1,d4
.Zero3:

	neg.w	d2			; Invert
	neg.w	d3

	addq.w	#1,d2			; Calculate index
	addq.w	#1,d3
	addq.w	#1,d4
	add.w	d3,d2
	add.w	d3,d3
	add.w	d3,d2
	move.w	d4,d3
	lsl.w	#3,d3
	add.w	d3,d2
	add.w	d4,d2
	add.w	d2,d2			; Get direction priorities
	add.w	d2,d2
	move.l	.Dir_table(pc,d2.w),d2
	bmi	.Move
	move.w	d0,d4			; Copy coordinates
	move.w	d1,d5
	rol.l	#8,d2			; Get 1st direction
	tst.b	d2
	bmi.s	.No_move2
	jsr	Move_in_direction		; Try move
	jsr	NPC_movement_check_2D
	beq.s	.Move
	move.w	d4,d0			; Restore coordinates
	move.w	d5,d1
	rol.l	#8,d2			; Get 2nd direction
	tst.b	d2
	bmi.s	.No_move2
	jsr	Move_in_direction		; Try move
	jsr	NPC_movement_check_2D
	beq.s	.Move
	move.w	d4,d0			; Restore coordinates
	move.w	d5,d1
	rol.l	#8,d2			; Get 3rd direction
	tst.b	d2
	bmi.s	.No_move2
	jsr	Move_in_direction		; Try move
	jsr	NPC_movement_check_2D
	beq.s	.Move
.No_move2:	move.w	d4,d0			; Restore coordinates
	move.w	d5,d1
	bra.s	.Exit

.Move:	move.w	d0,VMap_X(a1)		; Store new coordinates
	move.w	d1,VMap_Y(a1)
	andi.w	#$00ff,d2			; Store direction
	move.w	d2,VDir(a1)
.Exit:	movem.l	(sp)+,d0-d6
	rts

.Dir_table:
	dc.l $00030100,$00030100,$00010300	; dX < dY
	dc.l $03000200,-1,$01000200
	dc.l $02030100,$02010300,$02010300
	dc.l $0003ff00,-1,$0001ff00		; dX = dY
	dc.l -1,-1,-1
	dc.l $0203ff00,-1,$0201ff00
	dc.l $03000200,$00030100,$01000200	; dX > dY
	dc.l $03000200,-1,$01000200
	dc.l $03020000,$02030100,$01020000
*/

	}
	else
	{
		/* No -> */

	}
}

/*
	move.w	VMap_X(a1),d0		; Get position
	move.w	VMap_Y(a1),d1
	jsr	Check_line_of_sight		; Check line of sight
	beq	.Visible
	tst.w	Monster_move_delay		; Waiting ?
	bne	.Exit
	tst.b	New_step			; Update position ?
	beq	.Exit
	btst	#0,VFlags(a1)		; Moving at random ?
	bne.s	.Not_random
	jsr	Random_person_2D
	bra	.Exit
.Not_random:
	move.w	VDir(a1),d2		; Get direction
	jsr	Move_in_direction
	tst.w	d0			; X =< 0 ?
	ble.s	.No_move1
	cmp.w	Width_of_map,d0		; X > Width ?
	bhi.s	.No_move1
	tst.w	d1			; Y =< 0 ?
	ble.s	.No_move1
	cmp.w	Height_of_map,d1		; Y > Height ?
	bhi.s	.No_move1
	jsr	NPC_movement_check_2D
	beq	.Move
.No_move1:	bclr	#0,VFlags(a1)		; Random movement
	clr.w	VPathlen(a1)		; Force new path
	jsr	Random_person_2D
	bra	.Exit
*/

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_path_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map walking along a path.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 22:14
 * LAST      : 03.05.95 22:14
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_path_NPC_3D(UNSHORT NPC_index)
{
	UNSHORT Current_X, Current_Y;
	UNSHORT Next_X, Next_Y;
	UNSHORT i;
	UNBYTE *Path_ptr;

	/* Get address of path */
	Path_ptr = MEM_Claim_pointer(Map_handle) + VNPCs[NPC_index].Path_offset;

	/* Absolute or relative path ? */
	if (VNPCs[NPC_index].Movement_type == ABS_PATH_MOVEMENT)
	{
		/* Absolute -> Current path index is step */
		i = Current_step;
	}
	else
	{
		/* Relative -> Get current path index */
		i = VNPCs[NPC_index].Current_path_index;
	}

	/* Get current coordinates */
	Current_X = (UNSHORT) Path_ptr[i * 2];
	Current_Y = (UNSHORT) Path_ptr[i * 2 + 1];

	/* Get next coordinates */
	i++;
	if (i >= NPC_PATH_LENGTH)
		i = 0;
	Next_X = (UNSHORT) Path_ptr[i * 2];
	Next_Y = (UNSHORT) Path_ptr[i * 2 + 1];

	/* Relative path ? */
	if (VNPCs[NPC_index].Movement_type == REL_PATH_MOVEMENT)
	{
		/* Yes -> Store new path index */
		VNPCs[NPC_index].Current_path_index = i;
	}

	/* Store coordinates in NPC data */
	VNPCs[NPC_index].Map_X = Current_X;
	VNPCs[NPC_index].Map_Y = Current_Y;
	VNPCs[NPC_index].Next_X = Next_X;
	VNPCs[NPC_index].Next_Y = Next_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_random_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map moving randomly.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 22:14
 * LAST      : 03.05.95 22:14
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_random_NPC_3D(UNSHORT NPC_index)
{
	BOOLEAN Choose_next_path = TRUE;

	/* Set current to next coordinates */
	VNPCs[NPC_index].Map_X = VNPCs[NPC_index].Next_X;
	VNPCs[NPC_index].Map_Y = VNPCs[NPC_index].Next_Y;

	#ifdef FALSE
	/* End of path ? */
	if (VNPCs[NPC_index].Rnd_path_length)
	{
		/* No -> Try to move along current path */

		Choose_next_path = FALSE;

		/* Store next coordinates */
	}

	/* Choose next path ? */
	while (Choose_next_path)
	{
		/* Yes -> Choose next path */
		VNPCs[NPC_index].Rnd_path_direction =
		 (VNPCs[NPC_index].Rnd_path_direction +
		 Random_course_changes[rand() % 19]) % 8;
		VNPCs[NPC_index].Rnd_path_length = 1 + rand() % 0x0007;

		/* Try to move along new path */
		Choose_next_path = FALSE;

		/* Store next coordinates */
	}

	/* Count down */
	VNPCs[NPC_index].Rnd_path_length--;
	#endif
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_chasing_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map chasing the party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 22:14
 * LAST      : 03.05.95 22:14
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_chasing_NPC_3D(UNSHORT NPC_index)
{
	/* Can the NPC see the party ? */
	if (Check_line_of_sight(NPC_index))
	{
		/* Yes -> Gaze! */
		Monster_is_watching = TRUE;
	}
	else
	{
		/* No -> */

	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_line_of_sight
 * FUNCTION  : Check if the party is an NPC's line of sight.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.05.95 16:58
 * LAST      : 01.05.95 16:58
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : BOOLEAN : In line of sight.
 * BUGS      : No known.
 * NOTES     :
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_line_of_sight(UNSHORT NPC_index)
{
	BOOLEAN Result = TRUE;
	SILONG Length;
	SISHORT dX, dY;
	SISHORT Sign_dX, Sign_dY;
	UNSHORT X, Y;
	UNSHORT Var;
	UNSHORT i;

	/* Get NPC coordinates */
	X = VNPCs[NPC_index].Map_X;
	Y = VNPCs[NPC_index].Map_Y;

	/* Calculate vector */
	dX = PARTY_DATA.X - X;
	dY = PARTY_DATA.Y - Y;

	/* At same location as party ? */
	if (!dX && !dY)
	{
		/* Yes -> Seen */
		return(TRUE);
	}

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Calculate vector direction */
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

			/* Check line of sight */
			for (i=0;i<dX - 1;i++)
			{
				/* Vision blocked ? */
				if (Vision_blocked(X, Y))
				{
					/* Yes -> Can't see */
					Result = FALSE;
					break;
				}

				/* Trace line */
				Var += dY;
				if (Var > dX)
				{
					Var -= dX;
					Y += Sign_dY;
				}
				X += Sign_dX;
			}
		}
		else
		{
			/* Initialize Bresenham variable */
			Var = dY / 2;

			/* Check line of sight */
			for (i=0;i<dY - 1;i++)
			{
				/* Vision blocked ? */
				if (Vision_blocked(X, Y))
				{
					/* Yes -> Can't see */
					Result = FALSE;
					break;
				}

				/* Trace line */
				Var += dX;
				if (Var > dY)
				{
					Var -= dY;
					X += Sign_dX;
				}
				Y += Sign_dY;
			}
		}
	}
	else
	{
		/* 2D -> Calculate vector length */
		Length = (dX * dX) + (dY * dY);
		Length = (SILONG) sqrt((double) Length);

		/* Distance too large ? */
		if (Length > MAX_2D_LOS_LENGTH)
		{
			/* Yes -> Out of sight */
			Result = FALSE;
		}
	}

	return(Result);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_for_NPC_reaction
 * FUNCTION  : Check for a reaction between NPC and party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.11.94 12:01
 * LAST      : 22.11.94 12:01
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Check_for_NPC_reaction(UNSHORT NPC_index)
{
	/* Did this NPC move ? */
	if ((VNPCs[NPC_index].Map_X != VNPCs[NPC_index].Old_X) ||
	 (VNPCs[NPC_index].Map_Y != VNPCs[NPC_index].Old_Y))
	{
		/* Yes -> Store new coordinates */
		VNPCs[NPC_index].Old_X = VNPCs[NPC_index].Map_X;
		VNPCs[NPC_index].Old_Y = VNPCs[NPC_index].Map_Y;

		/* Can this NPC trigger events / on a legal position ? */
		if ((VNPCs[NPC_index].Flags & NPC_CAN_TRIGGER) &&
		 (VNPCs[NPC_index].Map_X) && (VNPCs[NPC_index].Map_Y))
		{
			/* Yes -> Trigger */
			Trigger_map_event_chain(VNPCs[NPC_index].Map_X,
			 VNPCs[NPC_index].Map_Y, NPC_index, NPC_TRIGGER, NPC_index);
		}
	}
}

#if FALSE
/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : NPC_present
 * FUNCTION  : Check if an NPC in the current map is present.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.05.95 21:18
 * LAST      : 08.05.95 21:34
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : BOOLEAN : TRUE (present) or FALSE (not present).
 * BUGS      : No known.
 * NOTES     : - This function will check both if an NPC with this index
 *              exists, and if it has been deleted.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
NPC_present(UNSHORT NPC_index)
{
	UNLONG Bit_number;

	/* Anyone there ? */
	if (VNPCs[NPC_index].Number)
	{
		/* Yes -> Calculate bit-array index */
		Bit_number = (PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP + NPC_index;

		/* Check CD bit array */
		return (!(Read_bit_array(CD_BIT_ARRAY, Bit_number)));
	}
	else
	{
		/* No -> Exit */
		return(FALSE);
	}
}
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_2D_NPC
 * FUNCTION  :	Search for an NPC in the current map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 10:56
 * LAST      : 22.12.94 10:56
 * INPUTS    : UNSHORT X - Map X-coordinate.
 *             UNSHORT Y - Map Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : UNSHORT : Index of found NPC / 0xFFFF means nothing was found.
 * BUGS      : No known.
 * NOTES     : - NPCs of type Object won't be found.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_2D_NPC(UNSHORT X, UNSHORT Y, UNSHORT NPC_index)
{
	UNSHORT Found_NPC = 0xFFFF;
	UNSHORT i;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return(0xFFFF);

	/* Convert coordinates */
	X = (X - 1) * 16;
	Y = (Y - 1) * 16 + 15;

	/* Search NPCs */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself / not object ? */
		if ((NPC_present(i)) && (i != NPC_index) && ((VNPCs[i].NPC_type
		 != OBJECT_NPC)))
		{
			/* Yes -> In the right place ? */
			if ((X >= VNPCs[i].NPC_object.X) && (Y <= VNPCs[i].NPC_object.Y) &&
			 (X < VNPCs[i].NPC_object.X + VNPCs[i].NPC_object.Width) &&
			 (Y > VNPCs[i].NPC_object.Y - VNPCs[i].NPC_object.Height))
			{
				/* Yes -> Did we already find an NPC ? */
				if (Found_NPC != 0xFFFF)
				{
					/* Yes -> Is this one better ? */
					if (VNPCs[i].NPC_object.Y > VNPCs[Found_NPC].NPC_object.Y)
					{
						/* Yes -> Mark */
						Found_NPC = i;
					}
					else
					{
						/* Well, is it ? */
						if ((VNPCs[i].NPC_object.Y == VNPCs[Found_NPC].NPC_object.Y)
						 && (VNPCs[i].NPC_object.X > VNPCs[Found_NPC].NPC_object.X))
						{
							/* Yes -> Mark */
							Found_NPC = i;
						}
					}
				}
				else
				{
					/* No -> Mark */
					Found_NPC = i;
				}
			}
		}
	}

	/* Find anything ? */
	return(Found_NPC);
}

