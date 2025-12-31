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
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBDSA.H>

#include <ALBION.H>

#include <XLOAD.H>

#include <CONTROL.H>
#include <PRTLOGIC.H>
#include <MAP.H>
#include <NPCS.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <2D_PATH.H>
#include <2D_COLL.H>
#include <3D_MAP.H>
#include <3DCOMMON.H>
#include <XFTYPES.H>
#include <EVELOGIC.H>
#include <POPUP.H>
#include <GAMETIME.H>
#include <GFXFUNC.H>
#include <COMBAT.H>

/* defines */

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

void Do_path_NPC_2D(UNSHORT NPC_index);
void Do_random_NPC_2D(UNSHORT NPC_index);
void Do_waiting_NPC_2D(UNSHORT NPC_index);
void Do_chasing_NPC_2D(UNSHORT NPC_index);
void Do_locked_NPC_2D(UNSHORT NPC_index);

void Do_path_NPC_3D(UNSHORT NPC_index);
void Do_random_NPC_3D(UNSHORT NPC_index);
void Do_waiting_NPC_3D(UNSHORT NPC_index);
void Do_chasing_NPC_3D(UNSHORT NPC_index);

BOOLEAN Check_line_of_sight(UNSHORT NPC_index);
void Check_for_NPC_reaction(UNSHORT NPC_index);

void Choose_next_random_NPC_path(UNSHORT NPC_index);

UNSHORT Get_target_VD(UNSHORT Source_X, UNSHORT Source_Y, UNSHORT Target_X,
 UNSHORT Target_Y, UNSHORT Current_VD);

BOOLEAN Check_line_of_movement(UNSHORT Source_X, UNSHORT Source_Y,
 UNSHORT Target_X, UNSHORT Target_Y, UNSHORT Travel_mode, UNSHORT NPC_index);

void Add_NPC_sound_effects(UNSHORT NPC_index);
void Remove_NPC_sound_effects(UNSHORT NPC_index);

/* global variables */

BOOLEAN Hide_NPCs = FALSE;

/* Extended NPC data */
struct VNPC_data VNPCs[NPCS_PER_MAP];

/* 3D NPC array */
struct NPC_3D NPCs_3D[NPCS_PER_MAP];
UNSHORT Nr_3D_NPCs;

static UNSHORT Last_NPC_step;

BOOLEAN Monster_is_watching;

static UNSHORT Chase_directions[3][3] = {
	NORTH_WEST,	NORTH8,	NORTH_EAST,
	WEST8,		0xFFFF,	EAST8,
	SOUTH_WEST,	SOUTH8,	SOUTH_EAST
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_NPC_data
 * FUNCTION  : Initialize NPC data.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 27.05.95 14:28
 * LAST      : 12.08.95 11:14
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
	UNSHORT Map_X, Map_Y;
	UNSHORT i, j;
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
				VNPCs[i].Number			= (UNSHORT) NPC_ptr[i].New_NPC_data.Number;
				VNPCs[i].Graphics_nr		= NPC_ptr[i].New_NPC_data.Graphics_nr;
				VNPCs[i].NPC_type			= (NPC_TYPE_T) (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_TYPE);
				VNPCs[i].Sound_index		= NPC_ptr[i].New_NPC_data.Sound_index;
				VNPCs[i].Trigger_modes	= NPC_ptr[i].New_NPC_data.Trigger_modes;
				VNPCs[i].First_block_nr	= NPC_ptr[i].New_NPC_data.First_block_nr;
				VNPCs[i].Movement_type	= (UNSHORT) NPC_ptr[i].New_NPC_data.Movement_type;

				/* Copy new flags */
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_CAN_TRIGGER)
					VNPCs[i].Flags |= NPC_CAN_TRIGGER;
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_SHORT_DIALOGUE)
					VNPCs[i].Flags |= NPC_SHORT_DIALOGUE;
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_ANIMATE_ALWAYS)
					VNPCs[i].Flags |= NPC_ANIMATE_ALWAYS;

				/* Set travel mode */
				if (NPC_ptr[i].New_NPC_data.Flags & NEW_NPC_NOT_ON_FOOT)
					VNPCs[i].Travel_mode	= RIDING_1;
				else
					VNPCs[i].Travel_mode	= ON_FOOT;
			}
		}
		else
		{
			/* Old -> Anyone there ? */
			if (NPC_ptr[i].Old_NPC_data.Number)
			{
				/* Yes -> Copy old NPC data */
				VNPCs[i].Number			= (UNSHORT) NPC_ptr[i].Old_NPC_data.Number;
				VNPCs[i].Graphics_nr		= NPC_ptr[i].Old_NPC_data.Graphics_nr;
				VNPCs[i].NPC_type			= (NPC_TYPE_T) (NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_TYPE);
				VNPCs[i].Travel_mode		= (UNSHORT) NPC_ptr[i].Old_NPC_data.Travel_mode;
				VNPCs[i].Trigger_modes	= NPC_ptr[i].Old_NPC_data.Trigger_modes;
				VNPCs[i].First_block_nr	= NPC_ptr[i].Old_NPC_data.First_block_nr;
				VNPCs[i].Movement_type	= (UNSHORT) ((NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_MOVEMENT_TYPE) >> OLD_NPC_MOVEMENT_TYPE_B);

				/* Copy old flags */
				if (NPC_ptr[i].Old_NPC_data.Flags & OLD_NPC_SHORT_DIALOGUE)
					VNPCs[i].Flags |= NPC_SHORT_DIALOGUE;

				/* Set sound index */
				VNPCs[i].Sound_index		= 0;
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

					Map_X = (UNSHORT) *Ptr;
					Map_Y = (UNSHORT) *(Ptr + 1);

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

					Map_X = (UNSHORT) *Ptr;
					Map_Y = (UNSHORT) *(Ptr + 1);

					/* Skip path */
					Offset += NPC_PATH_LENGTH * 2;

					break;
				}
				/* Starting on a set position */
				default:
				{
					/* Get initial coordinates */
					Ptr = Offset + (UNBYTE *) Map_ptr;

					Map_X = (UNSHORT) *Ptr;
					Map_Y = (UNSHORT) *(Ptr + 1);

					/* Set initial view direction */
					VNPCs[i].Data._2D_NPC_data.Move.View_direction = SOUTH;

					/* Skip position */
					Offset += 2;

					break;
				}
			}

			/* Clear random path length */
			VNPCs[i].Rnd_path_length = 0;

			/* Clear relative path offset */
			VNPCs[i].Current_path_index = 0;

			/* Set NPC position */
			Set_NPC_position(i, Map_X, Map_Y);

			/* Set effect indices */
			for (j=0;j<MAX_NPC_EFFECT_TYPES;j++)
			{
				VNPCs[i].Effect_indices[j] = 0xFFFF;
			}
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
 * LAST      : 02.08.95 13:56
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
	UNSHORT i;

	/* Initialize all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> Set current playfield coordinates */
			Map_to_2D(VNPCs[i].Map_X, VNPCs[i].Map_Y,
			 &(VNPCs[i].Display_X), &(VNPCs[i].Display_Y));

			/* Initialize 2D movement data structure */
			VNPCs[i].Data._2D_NPC_data.Move.Flags				= 0;
			VNPCs[i].Data._2D_NPC_data.Move.Slide_direction	= 0xFFFF;
			VNPCs[i].Data._2D_NPC_data.Move.Reverse_slide	= 0xFFFF;
			VNPCs[i].Data._2D_NPC_data.Move.Travel_mode		= VNPCs[i].Travel_mode;
			VNPCs[i].Data._2D_NPC_data.Move.NPC_index			= i;

			/* Initialize 2D graphics */
			Init_2D_NPC_graphics(i);
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
 * LAST      : 02.08.95 14:02
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
			/* Yes -> Exit graphics */
			Exit_2D_NPC_graphics(i);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_2D_NPC_graphics
 * FUNCTION  : Initialize NPC graphics in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.08.95 14:00
 * LAST      : 02.08.95 14:00
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_2D_NPC_graphics(UNSHORT NPC_index)
{
	struct Gfx_header *Gfx;
	MEM_HANDLE Handle;
	UNSHORT Gfx_file_type;

	/* Determine NPC graphics type */
	if (Big_guy)
		Gfx_file_type = NPCGR_GFX;
	else
		Gfx_file_type = NPCKL_GFX;

	/* Load NPC graphics */
	Handle = Load_subfile(Gfx_file_type, VNPCs[NPC_index].Graphics_nr);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Set NPC 2D object dimensions */
		Gfx = (struct Gfx_header *) MEM_Claim_pointer(Handle);

		VNPCs[NPC_index].Data._2D_NPC_data.NPC_object.Width	= Gfx->Width;
		VNPCs[NPC_index].Data._2D_NPC_data.NPC_object.Height	= Gfx->Height;

		MEM_Free_pointer(Handle);

		/* Set object icon height */
		VNPCs[NPC_index].Data._2D_NPC_data.NPC_object.Level = 0;

		/* Set graphics handle */
		VNPCs[NPC_index].Data._2D_NPC_data.NPC_object.Graphics_handle = Handle;
	}
	else
	{
		/* No -> Disable NPC */
		VNPCs[NPC_index].Number = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_2D_NPC_graphics
 * FUNCTION  : Exit NPC graphics in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.08.95 14:01
 * LAST      : 02.08.95 14:01
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_2D_NPC_graphics(UNSHORT NPC_index)
{
	/* Destroy NPC graphics */
	MEM_Free_memory(VNPCs[NPC_index].Data._2D_NPC_data.NPC_object.Graphics_handle);
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
			/* Yes -> Convert map to 3D coordinates */
			Map_to_3D(VNPCs[i].Map_X, VNPCs[i].Map_Y, &(VNPCs[i].Display_X),
			 &(VNPCs[i].Display_Y));

			/* Move NPC away from centre of map square */
			VNPCs[i].Display_X -= (f / 2);
			VNPCs[i].Display_Y -= (f / 2);

			/* Initialize 3D movement data structure */
			VNPCs[i].Data._3D_NPC_data.Move.Travel_mode	= VNPCs[i].Travel_mode;
			VNPCs[i].Data._3D_NPC_data.Move.NPC_index		= i;
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
 * LAST      : 02.08.95 10:03
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
	SILONG dX, dY;
	UNLONG Source_X, Source_Y;
	UNLONG Target_X, Target_Y;
	UNSHORT Old_map_nr;
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

	/* Handle NPCs */
	for (i=0;i<dTime;i++)
	{
		for(j=0;j<NPCS_PER_MAP;j++)
		{
			/* Anyone there ? */
			if (NPC_present(j))
			{
				/* Yes -> Locked ? */
				if (!(VNPCs[j].Flags & NPC_LOCKED))
				{
					/* No -> How does this NPC move? */
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
							Do_waiting_NPC_2D(j);
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
	}

	/* Update NPC positions */
	for(j=0;j<NPCS_PER_MAP;j++)
	{
		/* Anyone there ? */
		if (NPC_present(j))
		{
			/* Yes -> Locked ? */
			if (VNPCs[j].Flags & NPC_LOCKED)
			{
				/* Yes -> Handle differently */
				Do_locked_NPC_2D(j);
			}
			else
			{
				/* No -> Get source and target coordinates and convert to
				 playfield coordinates */
				Map_to_2D(VNPCs[j].Source_X, VNPCs[j].Source_Y, &Source_X,
				 &Source_Y);
				Map_to_2D(VNPCs[j].Target_X, VNPCs[j].Target_Y, &Target_X,
				 &Target_Y);

				/* Appearing or disappearing ? */
				if ((VNPCs[j].Source_X || VNPCs[j].Source_Y) &&
				 (VNPCs[j].Target_X || VNPCs[j].Target_Y))
				{
					/* No -> Calculate vector */
					dX = Target_X - Source_X;
					dY = Target_Y - Source_Y;

					/* Calculate new coordinates */
					VNPCs[j].Display_X = Source_X + (dX * (SILONG) Current_substep) /
					 SUBSTEPS_PER_STEP;
					VNPCs[j].Display_Y = Source_Y + (dY * (SILONG) Current_substep) /
					 SUBSTEPS_PER_STEP;
				}
				else
				{
					/* Yes -> Disappear */
					VNPCs[j].Display_X = 0;
					VNPCs[j].Display_Y = 0;
				}

				/* Determine new map coordinates */
				_2D_to_map(VNPCs[j].Display_X, VNPCs[j].Display_Y,
				 (SISHORT *) &(VNPCs[j].Map_X), (SISHORT *) &(VNPCs[j].Map_Y));
			}
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Display_2D_NPCs
 * FUNCTION  : Display NPCs in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.11.94 14:40
 * LAST      : 15.08.95 12:02
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

	/* Are the NPCs hidden ? */
	if (!Hide_NPCs)
	{
		/* No -> Display NPCs */
		for(i=0;i<NPCS_PER_MAP;i++)
		{
			/* Anyone there ? */
			if (NPC_present(i))
			{
				/* Yes -> Visible ? */
				if (VNPCs[i].Display_X || VNPCs[i].Display_Y)
				{
					/* Yes */
					Object = &(VNPCs[i].Data._2D_NPC_data.NPC_object);

					/* Has this object moved ? */
					dX = Object->X - VNPCs[i].Display_X;
					dY = Object->Y - VNPCs[i].Display_Y;

					/* Set object coordinates */
					Object->X = VNPCs[i].Display_X;
				 	Object->Y = VNPCs[i].Display_Y;

			  		/* Determine animation frame depending on state */
					switch (VNPCs[i].Data._2D_NPC_data.Move.State)
					{
						/* "Special" states */
						case SITTING_NL_STATE:
						{
							Frame = 12;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SITTING_NR_STATE:
						{
							Frame = 12;
							Object->X -= 16;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SITTING_E_STATE:
						{
							Frame = 13;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SITTING_SL_STATE:
						{
							Frame = 14;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SITTING_SR_STATE:
						{
							Frame = 14;
							Object->X -= 16;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SITTING_W_STATE:
						{
							Frame = 15;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SLEEPING1_STATE:
						{
							Frame = 16;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						case SLEEPING2_STATE:
						{
							Frame = 16;
							Object->Y -= 9;
							Object->Flags |= ADD_ICONS_OVER_OBJECT;
							break;
						}
						/* Normal state : walking */
						default:
						{
							/* Frame depends on view direction and animation phase */
							Frame = VNPCs[i].Data._2D_NPC_data.Move.View_direction *
							 3 + Party_animation[VNPCs[i].Frame];

							Object->Flags &= ~ADD_ICONS_OVER_OBJECT;

							break;
						}
					}

					/* Set graphics offset */
					Object->Graphics_offset = Frame * (Object->Width *
					 Object->Height) + sizeof(struct Gfx_header);

					#if FALSE
					Object->Graphics_offset = sizeof(struct Gfx_header);

					Object->Width = 32;
					Object->Height = 48;

					Object->Level = 0;

					Object->X = 32 + (i % 16) * 32;
					Object->Y = 64 + (i / 16) * 32;
					#endif

					/* Add NPC object to list */
					Add_2D_object(Object);

					/* Animate always ? */
					if (VNPCs[i].Flags & NPC_ANIMATE_ALWAYS)
					{
						/* Yes -> Next animation frame */
						VNPCs[i].Frame = (VNPCs[i].Frame + 1) % NR_PARTY_ANIMFRAMES;
					}
					else
					{
						/* No -> Did this NPC move ? */
						if (dX || dY)
						{
							/* Yes -> Next animation frame */
							VNPCs[i].Frame = (VNPCs[i].Frame + 1) % NR_PARTY_ANIMFRAMES;
						}
					}
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
 * LAST      : 02.08.95 10:03
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
	UNLONG Source_X, Source_Y;
	UNLONG Target_X, Target_Y;
	UNLONG f;
	UNSHORT Old_map_nr;
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
			Source_X -= (f / 2);
			Source_Y -= (f / 2);

			/* Get target coordinates and convert to 3D coordinates */
			Map_to_3D(VNPCs[j].Target_X, VNPCs[j].Target_Y, &Target_X, &Target_Y);
			Target_X -= (f / 2);
			Target_Y -= (f / 2);

			/* Appearing or disappearing ? */
			if ((VNPCs[j].Source_X || VNPCs[j].Source_Y) &&
			 (VNPCs[j].Target_X || VNPCs[j].Target_Y))
			{
				/* No -> Calculate vector */
				dX = Target_X - Source_X;
				dY = Target_Y - Source_Y;

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
			if (VNPCs[i].Display_X || VNPCs[i].Display_Y)
			{
				/* Yes -> Add NPC object to 3D NPC list */
				NPCs_3D[Nr_3D_NPCs].X			= VNPCs[i].Display_X;
				NPCs_3D[Nr_3D_NPCs].Z			= VNPCs[i].Display_Y;
				/* (+ correction for 3DM bug) */
				NPCs_3D[Nr_3D_NPCs].Group_nr	= VNPCs[i].Graphics_nr - 1;
				NPCs_3D[Nr_3D_NPCs].NPC_nr		= i + 1;

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
 * LAST      : 14.08.95 07:15
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
	UNSHORT Source_X, Source_Y;
	UNSHORT Target_X, Target_Y;
	UNSHORT VD;
	UNSHORT Source_state;
	UNSHORT Target_state;
	UNSHORT Path_index;
	UNBYTE *Path_ptr;

	/* Get address of path */
	Path_ptr = MEM_Claim_pointer(Map_handle) + VNPCs[NPC_index].Path_offset;

	/* Absolute or relative path ? */
	if (VNPCs[NPC_index].Movement_type == ABS_PATH_MOVEMENT)
	{
		/* Absolute -> Current path index is step */
		Path_index = Current_step;
	}
	else
	{
		/* Relative -> Get current path index */
		Path_index = VNPCs[NPC_index].Current_path_index;
	}

	/* Get source coordinates */
	Source_X = (UNSHORT) Path_ptr[Path_index * 2];
	Source_Y = (UNSHORT) Path_ptr[Path_index * 2 + 1];

	/* Are the source coordinates legal ? */
	if ((Source_X > Map_width) || (Source_Y > Map_height))
	{
		/* No -> Clear / invisible */
		Source_X = 0;
		Source_Y = 0;
	}

	/* Go to next path entry */
	Path_index++;
	if (Path_index >= NPC_PATH_LENGTH)
		Path_index = 0;

	/* Get target coordinates */
	Target_X = (UNSHORT) Path_ptr[Path_index * 2];
	Target_Y = (UNSHORT) Path_ptr[Path_index * 2 + 1];

	/* Are the target coordinates legal ? */
	if ((Target_X > Map_width) || (Target_Y > Map_height))
	{
		/* No -> Clear / invisible */
		Target_X = 0;
		Target_Y = 0;
	}

	MEM_Free_pointer(Map_handle);

	/* Relative path ? */
	if (VNPCs[NPC_index].Movement_type == REL_PATH_MOVEMENT)
	{
		/* Yes -> Store new path index */
		VNPCs[NPC_index].Current_path_index = Path_index;
	}

	/* Store coordinates in NPC data */
	VNPCs[NPC_index].Source_X = Source_X;
	VNPCs[NPC_index].Source_Y = Source_Y;
	VNPCs[NPC_index].Target_X = Target_X;
	VNPCs[NPC_index].Target_Y = Target_Y;

	/* Appearing or disappearing ? */
	if ((Source_X || Source_Y) && (Target_X || Target_Y))
	{
		/* No -> Calculate vector */
		dX = Target_X - Source_X;
		dY = Target_Y - Source_Y;

		/* Set view direction */
		VD = View_directions[sgn(dY) + 1][sgn(dX) + 1];
		if (VD != 0xFFFF)
		{
			VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction = VD;
		}

		/* Big guy ? */
		if (Big_guy)
		{
			/* Yes -> Get state of source and target position */
			Source_state = (Get_location_status(Source_X, Source_Y,
			 NPC_index) & STATE) >> STATE_B;
			Target_state = (Get_location_status(Target_X, Target_Y, NPC_index)
			 & STATE) >> STATE_B;

			/* Moving from one state to the next ? */
			if (Source_state != Target_state)
			{
				/* Yes -> Jump */
				VNPCs[NPC_index].Source_X = Target_X;
				VNPCs[NPC_index].Source_Y = Target_Y;
			}

			/* Set state */
			VNPCs[NPC_index].Data._2D_NPC_data.Move.State = Target_state;
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
 * LAST      : 02.08.95 10:11
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

	/* Set source to target coordinates */
	VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Target_X;
	VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Target_Y;

	/* End of path ? */
	if (VNPCs[NPC_index].Rnd_path_length)
	{
		/* No -> Try to move along current path */
		VNPCs[NPC_index].Data._2D_NPC_data.Move.X				= VNPCs[NPC_index].Source_X;
		VNPCs[NPC_index].Data._2D_NPC_data.Move.Y				= VNPCs[NPC_index].Source_Y;
		VNPCs[NPC_index].Data._2D_NPC_data.Move.Direction	= VNPCs[NPC_index].Rnd_path_direction;

		Choose_next_path =
		 !(Try_2D_move(&(VNPCs[NPC_index].Data._2D_NPC_data.Move)));

		/* Store source and target coordinates */
		VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Data._2D_NPC_data.Move.X;
		VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Data._2D_NPC_data.Move.Y;

		VNPCs[NPC_index].Target_X = VNPCs[NPC_index].Source_X +
		 VNPCs[NPC_index].Data._2D_NPC_data.Move.dX;
		VNPCs[NPC_index].Target_Y = VNPCs[NPC_index].Source_Y +
		 VNPCs[NPC_index].Data._2D_NPC_data.Move.dY;
	}

	/* Choose next path ? */
	if (Choose_next_path)
	{
		/* Yes -> Choose next path */
		Choose_next_random_NPC_path(NPC_index);

		/* Try to move along new path */
		VNPCs[NPC_index].Data._2D_NPC_data.Move.X				= VNPCs[NPC_index].Source_X;
		VNPCs[NPC_index].Data._2D_NPC_data.Move.Y				= VNPCs[NPC_index].Source_Y;
		VNPCs[NPC_index].Data._2D_NPC_data.Move.Direction	= VNPCs[NPC_index].Rnd_path_direction;

		Choose_next_path =
		 !(Try_2D_move(&(VNPCs[NPC_index].Data._2D_NPC_data.Move)));

		/* Store source and target coordinates */
		VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Data._2D_NPC_data.Move.X;
		VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Data._2D_NPC_data.Move.Y;

		VNPCs[NPC_index].Target_X = VNPCs[NPC_index].Source_X +
		 VNPCs[NPC_index].Data._2D_NPC_data.Move.dX;
		VNPCs[NPC_index].Target_Y = VNPCs[NPC_index].Source_Y +
		 VNPCs[NPC_index].Data._2D_NPC_data.Move.dY;
	}

	/* Count down */
	VNPCs[NPC_index].Rnd_path_length--;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_waiting_NPC_2D
 * FUNCTION  : Handle an NPC in a 2D map standing around.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.07.95 15:39
 * LAST      : 12.08.95 19:38
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_waiting_NPC_2D(UNSHORT NPC_index)
{
	UNSHORT State;

	/* Big guy ? */
	if (Big_guy)
	{
		/* Yes -> Get state of current position */
		State = (Get_location_status(VNPCs[NPC_index].Map_X,
		 VNPCs[NPC_index].Map_Y, NPC_index)	& STATE) >> STATE_B;

		/* Set state */
		VNPCs[NPC_index].Data._2D_NPC_data.Move.State = State;
	}

	/* Set source to target coordinates */
	/* (this is necessary in case the NPC was moved by a script) */
	VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Target_X;
	VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Target_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_chasing_NPC_2D
 * FUNCTION  : Handle an NPC in a 2D map chasing the party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.05.95 16:55
 * LAST      : 13.08.95 20:15
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

		/* Set flag */
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
		/* No -> Has seen already ? */
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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_locked_NPC_2D
 * FUNCTION  : Handle a locked NPC in a 2D map (scripts only).
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 19:40
 * LAST      : 12.08.95 23:26
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function will handle all of the movement and
 *              positioning of locked NPCs.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_locked_NPC_2D(UNSHORT NPC_index)
{
	SISHORT dX, dY;
	UNLONG Source_X, Source_Y;
	UNLONG Target_X, Target_Y;
	UNSHORT VD;
	UNSHORT Source_state;
	UNSHORT Target_state;
	UNSHORT Step;

	/* Get source and target coordinates */
	Source_X = (UNLONG) VNPCs[NPC_index].Source_X;
	Source_Y = (UNLONG) VNPCs[NPC_index].Source_Y;
	Target_X = (UNLONG) VNPCs[NPC_index].Target_X;
	Target_Y = (UNLONG) VNPCs[NPC_index].Target_Y;

	/* Appearing or disappearing ? */
	if ((Source_X || Source_Y) && (Target_X || Target_Y))
	{
		/* Big guy ? */
		if (Big_guy)
		{
			/* Yes -> Get state of source and target position */
			Source_state = (Get_location_status((UNSHORT) Source_X,
			 (UNSHORT) Source_Y, NPC_index) & STATE) >> STATE_B;
			Target_state = (Get_location_status((UNSHORT) Target_X,
			 (UNSHORT) Target_Y, NPC_index) & STATE) >> STATE_B;

			/* Moving from one state to the next ? */
			if (Source_state != Target_state)
			{
				/* Yes -> Jump */
				VNPCs[NPC_index].Source_X = (UNSHORT) Target_X;
				VNPCs[NPC_index].Source_Y = (UNSHORT) Target_Y;
			}

			/* Set state */
			VNPCs[NPC_index].Data._2D_NPC_data.Move.State = Target_state;
		}

		/* Get source and target coordinates and convert to playfield
		 coordinates */
		Map_to_2D(Source_X, Source_Y, &Source_X, &Source_Y);
		Map_to_2D(Target_X, Target_Y, &Target_X, &Target_Y);

		/* Calculate vector */
		dX = Target_X - Source_X;
		dY = Target_Y - Source_Y;

		/* Set view direction */
		VD = View_directions[sgn(dY) + 1][sgn(dX) + 1];
		if (VD != 0xFFFF)
		{
			VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction = VD;
		}

		/* Get current movement step */
		Step = VNPCs[NPC_index].Locked_move_step;

		/* Count up (1...4) */
		Step++;

		/* Calculate new coordinates */
		VNPCs[NPC_index].Display_X = Source_X + (dX * (SILONG) Step) / 4;
		VNPCs[NPC_index].Display_Y = Source_Y + (dY * (SILONG) Step) / 4;

		/* Final step reached ? */
		if (Step == 4)
		{
			/* Yes -> Reset */
			Step = 0;

			/* Set source to target coordinates */
			VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Target_X;
			VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Target_Y;
		}

		/* Store new movement step */
		VNPCs[NPC_index].Locked_move_step = Step;
	}
	else
	{
		/* Yes -> Disappear */
		VNPCs[NPC_index].Display_X = 0;
		VNPCs[NPC_index].Display_Y = 0;
	}

	/* Determine new map coordinates */
	_2D_to_map(VNPCs[NPC_index].Display_X, VNPCs[NPC_index].Display_Y,
	 (SISHORT *) &(VNPCs[NPC_index].Map_X), (SISHORT *) &(VNPCs[NPC_index].Map_Y));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_path_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map walking along a path.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 22:14
 * LAST      : 02.08.95 10:14
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
	UNSHORT Source_X, Source_Y;
	UNSHORT Target_X, Target_Y;
	UNSHORT Path_index;
	UNBYTE *Path_ptr;

	/* Get address of path */
	Path_ptr = MEM_Claim_pointer(Map_handle) + VNPCs[NPC_index].Path_offset;

	/* Absolute or relative path ? */
	if (VNPCs[NPC_index].Movement_type == ABS_PATH_MOVEMENT)
	{
		/* Absolute -> Current path index is step */
		Path_index = Current_step;
	}
	else
	{
		/* Relative -> Get current path index */
		Path_index = VNPCs[NPC_index].Current_path_index;
	}

	/* Get source coordinates */
	Source_X = (UNSHORT) Path_ptr[Path_index * 2];
	Source_Y = (UNSHORT) Path_ptr[Path_index * 2 + 1];

	/* Go to next path entry */
	Path_index++;
	if (Path_index >= NPC_PATH_LENGTH)
		Path_index = 0;

	/* Get target coordinates */
	Target_X = (UNSHORT) Path_ptr[Path_index * 2];
	Target_Y = (UNSHORT) Path_ptr[Path_index * 2 + 1];

	MEM_Free_pointer(Map_handle);

	/* Relative path ? */
	if (VNPCs[NPC_index].Movement_type == REL_PATH_MOVEMENT)
	{
		/* Yes -> Store new path index */
		VNPCs[NPC_index].Current_path_index = Path_index;
	}

	/* Store coordinates in NPC data */
	VNPCs[NPC_index].Source_X = Source_X;
	VNPCs[NPC_index].Source_Y = Source_Y;
	VNPCs[NPC_index].Target_X = Target_X;
	VNPCs[NPC_index].Target_Y = Target_Y;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_random_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map moving randomly.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 22:14
 * LAST      : 22.07.95 19:54
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
	SISHORT New_X, New_Y;

	/* Set source to target coordinates */
	VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Target_X;
	VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Target_Y;

	/* End of path ? */
	if (VNPCs[NPC_index].Rnd_path_length)
	{
		/* No -> Try to move along current path */
		New_X = VNPCs[NPC_index].Map_X +
		 Offsets8[VNPCs[NPC_index].Rnd_path_direction][0];
		New_Y = VNPCs[NPC_index].Map_Y +
		 Offsets8[VNPCs[NPC_index].Rnd_path_direction][1];

		Choose_next_path = !(Movement_check_3D(New_X, New_Y, NPC_index,
		 VNPCs[NPC_index].Travel_mode));

		/* Store target coordinates */
		if (!Choose_next_path)
		{
			VNPCs[NPC_index].Target_X = New_X;
			VNPCs[NPC_index].Target_Y = New_Y;
		}
	}

	/* Choose next path ? */
	if (Choose_next_path)
	{
		/* Yes -> Choose next path */
		Choose_next_random_NPC_path(NPC_index);

		/* Try to move along current path */
		New_X = VNPCs[NPC_index].Map_X +
		 Offsets8[VNPCs[NPC_index].Rnd_path_direction][0];
		New_Y = VNPCs[NPC_index].Map_Y +
		 Offsets8[VNPCs[NPC_index].Rnd_path_direction][1];

		Choose_next_path = !(Movement_check_3D(New_X, New_Y, NPC_index,
		 VNPCs[NPC_index].Travel_mode));

		/* Store target coordinates */
		if (!Choose_next_path)
		{
			VNPCs[NPC_index].Target_X = New_X;
			VNPCs[NPC_index].Target_Y = New_Y;
		}
	}

	/* Count down */
	VNPCs[NPC_index].Rnd_path_length--;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_waiting_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map standing around.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.07.95 15:42
 * LAST      : 24.07.95 15:42
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_waiting_NPC_3D(UNSHORT NPC_index)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_chasing_NPC_3D
 * FUNCTION  : Handle an NPC in a 3D map chasing the party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 22:14
 * LAST      : 13.08.95 13:29
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

		/* Clear flag */
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
		/* No -> Not moving randomly ? */
		if (!(VNPCs[NPC_index].Flags & NPC_MOVING_RANDOMLY))
		{
			/* Yes -> Set random path length */
			VNPCs[NPC_index].Rnd_path_length = 1 + rand() % 0x0007;

			/* Set flag */
			VNPCs[NPC_index].Flags |= NPC_MOVING_RANDOMLY;
		}

		/* Move randomly */
		Do_random_NPC_3D(NPC_index);
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
		return TRUE;
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

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_for_NPC_reaction
 * FUNCTION  : Check for a reaction between NPC and party.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.11.94 12:01
 * LAST      : 15.08.95 11:19
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
	UNLONG Bit_index;
	UNSHORT Combat_background_nr;
	UNSHORT Combat_status;

	/* Are the NPC and the party on the same position ? */
	if ((VNPCs[NPC_index].Map_X == PARTY_DATA.X) &&
	 (VNPCs[NPC_index].Map_Y == PARTY_DATA.Y))
	{
		/* Yes -> Is this NPC a monster / not in cheat mode ? */
		if ((VNPCs[NPC_index].NPC_type == MONSTER_NPC) && !Cheat_mode)
		{
			/* Yes -> Turn towards NPC */
			Turn_towards_NPC(NPC_index);

			/* Get combat background number */
			Combat_background_nr = Get_combat_background_nr();

			/* Enter combat */
			Combat_status =
			 Enter_Combat(VNPCs[NPC_index].Number, Combat_background_nr);

			/* Did the party win ? */
			if (Combat_status == COMBAT_PARTY_WON)
			{
				/* Yes -> Calculate bit index */
				Bit_index = ((PARTY_DATA.Map_nr - 1) * NPCS_PER_MAP) + NPC_index;

				/* Disable the NPC */
				Write_bit_array(CD_BIT_ARRAY, Bit_index, SET_BIT_ARRAY);
			}
			return;
		}
	}

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
		return FALSE;
	}
}
#endif

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_2D_NPC
 * FUNCTION  :	Search for an NPC in the current 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 10:56
 * LAST      : 02.08.95 15:06
 * INPUTS    : UNSHORT Map_X - Map X-coordinate.
 *             UNSHORT Map_Y - Map Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : UNSHORT : Index of found NPC / 0xFFFF means nothing was found.
 * BUGS      : No known.
 * NOTES     : - NPCs of type Object won't be found.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_2D_NPC(UNSHORT Map_X, UNSHORT Map_Y, UNSHORT NPC_index)
{
	SILONG X, Y;
	UNSHORT Found_NPC = 0xFFFF;
	UNSHORT i;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0xFFFF;

	/* Convert coordinates */
	Map_to_2D(Map_X, Map_Y, (UNLONG *) &X, (UNLONG *) &Y);

	/* Search NPCs */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself / not object ? */
		if ((NPC_present(i)) && (i != NPC_index) &&
		 ((VNPCs[i].NPC_type != OBJECT_NPC)))
		{
			/* Yes -> In the right place ? */
			if ((X >= VNPCs[i].Data._2D_NPC_data.NPC_object.X) &&
			 (Y <= VNPCs[i].Data._2D_NPC_data.NPC_object.Y) &&
			 (X < VNPCs[i].Data._2D_NPC_data.NPC_object.X +
			  VNPCs[i].Data._2D_NPC_data.NPC_object.Width) &&
			 (Y > VNPCs[i].Data._2D_NPC_data.NPC_object.Y -
			  VNPCs[i].Data._2D_NPC_data.NPC_object.Height))
			{
				/* Yes -> Did we already find an NPC ? */
				if (Found_NPC != 0xFFFF)
				{
					/* Yes -> Is this one better ? */
					if (VNPCs[i].Data._2D_NPC_data.NPC_object.Y >
					 VNPCs[Found_NPC].Data._2D_NPC_data.NPC_object.Y)
					{
						/* Yes -> Mark */
						Found_NPC = i;
					}
					else
					{
						/* Well, is it ? */
						if ((VNPCs[i].Data._2D_NPC_data.NPC_object.Y ==
						  VNPCs[Found_NPC].Data._2D_NPC_data.NPC_object.Y) &&
						 (VNPCs[i].Data._2D_NPC_data.NPC_object.X >
						  VNPCs[Found_NPC].Data._2D_NPC_data.NPC_object.X))
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
	return Found_NPC;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Search_3D_NPC
 * FUNCTION  :	Search for an NPC in the current 3D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.07.95 14:58
 * LAST      : 19.07.95 14:58
 * INPUTS    : UNSHORT Map_X - Map X-coordinate.
 *             UNSHORT Map_Y - Map Y-coordinate.
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : UNSHORT : Index of found NPC / 0xFFFF means nothing was found.
 * BUGS      : No known.
 * NOTES     : - NPCs of type Object won't be found.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Search_3D_NPC(UNSHORT Map_X, UNSHORT Map_Y, UNSHORT NPC_index)
{
	UNSHORT Found_NPC = 0xFFFF;
	UNSHORT i;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return 0xFFFF;

	/* Search NPCs */
	for(i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there / not checking myself / not object ? */
		if ((NPC_present(i)) && (i != NPC_index) && ((VNPCs[i].NPC_type
		 != OBJECT_NPC)))
		{
			/* Yes -> In the right place ? */
			if ((VNPCs[i].Map_X == Map_X) && (VNPCs[i].Map_Y == Map_Y))
			{
				/* Yes -> Found */
				Found_NPC = i;
				break;
			}
		}
	}

	/* Find anything ? */
	return Found_NPC;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Choose_next_random_NPC_path
 * FUNCTION  : Choose the next random path for an NPC.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.95 19:56
 * LAST      : 22.07.95 19:56
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Choose_next_random_NPC_path(UNSHORT NPC_index)
{
	static SISHORT Random_course_changes[19] = {
		0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5, 5, 6, 6, 6, 7, 7, 7
	};

	/* Choose a new direction */
	VNPCs[NPC_index].Rnd_path_direction =
	 (VNPCs[NPC_index].Rnd_path_direction +
	 Random_course_changes[rand() % 19]) % 8;

	/* Choose a new path length */
	VNPCs[NPC_index].Rnd_path_length = 1 + rand() % 0x0007;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Turn_towards_NPC
 * FUNCTION  : Turn towards an NPC.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.07.95 15:48
 * LAST      : 10.08.95 14:27
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Turn_towards_NPC(UNSHORT NPC_index)
{
	UNSHORT Party_VD;
	UNSHORT NPC_VD;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D */

	}
	else
	{
		/* 2D */
		for (;;)
		{
			/* Get party target view direction */
			if (Active_member_state == STANDING_STATE)
			{
				Party_VD = Get_target_VD(PARTY_DATA.X, PARTY_DATA.Y,
				 VNPCs[NPC_index].Map_X, VNPCs[NPC_index].Map_Y,
				 PARTY_DATA.View_direction);
			}
			else
			{
				Party_VD = PARTY_DATA.View_direction;
			}

			/* Get NPC target view direction */
			if (VNPCs[NPC_index].Data._2D_NPC_data.Move.State == STANDING_STATE)
			{
				NPC_VD = Get_target_VD(VNPCs[NPC_index].Map_X,
				 VNPCs[NPC_index].Map_Y,
				 PARTY_DATA.X, PARTY_DATA.Y,
				 VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction);
			}
			else
			{
				NPC_VD = VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction;
			}

			/* Is there a need for rotation ? */
			if ((Party_VD != PARTY_DATA.View_direction) ||
			 (NPC_VD != VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction))
			{
				/* Yes -> Set new view directions for party and NPC */
				PARTY_DATA.View_direction = Party_VD;
				VNPCs[NPC_index].Data._2D_NPC_data.Move.View_direction = NPC_VD;

				/* Store current position in path */
				Store_2D_current_position();

				/* Update party positions */
				Update_2D_party_position();

				/* Reduce steps left */
				_2D_steps_left = max(0, _2D_steps_left - _2D_speed);

				/* Display map */
				Update_display();

				Switch_screens();
			}
			else
			{
				/* No -> Break */
				break;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_target_VD
 * FUNCTION  : Get target view direction when rotating in a 2D map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 02.08.95 16:03
 * LAST      : 02.08.95 16:03
 * INPUTS    : UNSHORT Source_X - Source X-coordinate.
 *             UNSHORT Source_Y - Source Y-coordinate.
 *             UNSHORT Target_X - Target X-coordinate.
 *             UNSHORT Target_Y - Target Y-coordinate.
 *             UNSHORT Current_VD - Current view direction.
 * RESULT    : UNSHORT : Target view direction.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_target_VD(UNSHORT Source_X, UNSHORT Source_Y, UNSHORT Target_X,
 UNSHORT Target_Y, UNSHORT Current_VD)
{
	static UNSHORT Turn_view_directions[2][3][3] = {
		{
				/* dX <= dY */
			{	NORTH,	NORTH,	NORTH	},
			{	WEST,		NORTH,	EAST	},
			{	SOUTH,	SOUTH,	SOUTH	}
		},
		{
				/* dX > dY */
			{	WEST,		NORTH,	EAST	},
			{	WEST,		NORTH,	EAST	},
			{	WEST,		SOUTH,	EAST	}
		}
	};

	SISHORT dX, dY;
	UNSHORT Target_VD = 0xFFFF;

	/* Calculate vector */
	dX = Target_X - Source_X;
	dY = Target_Y - Source_Y;

	/* Get target view direction */
	if (abs(dX) <= abs(dY))
	{
		Target_VD = Turn_view_directions[0][sgn(dY) + 1][sgn(dX) + 1];
	}
	else
	{
		Target_VD = Turn_view_directions[1][sgn(dY) + 1][sgn(dX) + 1];
	}

	/* Is there a need for rotation ? */
	if (Target_VD != Current_VD)
	{
		/* Yes -> Is the angle 180 degrees ? */
		if (abs((Target_VD + 2) - (Current_VD + 2)) == 2)
		{
			/* Yes -> Select extra rotation step */
			if ((Target_VD == NORTH) || (Target_VD == SOUTH))
			{
				if (dX > 0)
					Target_VD = EAST;
				else
					Target_VD = WEST;
			}
			else
			{
				if (dY > 0)
					Target_VD = SOUTH;
				else
					Target_VD = NORTH;
			}
		}
	}

	return Target_VD;
}


/*
; ---------- 3D map -------------------------------
.3D:	moveq.l	#4-1,d7			; Try 4 times
.Loop:	move.w	VSource_X(a0),d2		; Get vector
	move.w	VSource_Y(a0),d3
	sub.w	Player_X,d2
	sub.w	Player_Y,d3
	tst.w	d2			; Is zero ?
	bne.s	.Not_zero1
	tst.w	d3
	beq	.Done
.Not_zero1:
	move.w	d2,d0			; Calculate length
	move.w	d3,d1
	mulu.w	d0,d0
	mulu.w	d1,d1
	add.l	d1,d0
	jsr	Square_root
	tst.w	d0			; At least one
	bne.s	.Not_zero2
	moveq.l	#1,d0
.Not_zero2:
	move.w	d0,d5			; Save length
	move.w	d2,d0			; Get angle
	move.w	d3,d1
	jsr	Calculate_ATN
	move.w	d0,d4			; Save angle
	cmp.w	#patt_size/2,d5		; Too close ?
	bpl	.Go_on
	move.w	d2,d0			; Yes -> Get vector
	mulu.w	#patt_size/2,d0
	divu.w	d5,d0
	move.w	d3,d1
	mulu.w	#patt_size/2,d1
	divu.w	d5,d1
	move.w	Player_X,d2		; Move
	move.w	Player_Y,d3
	sub.w	d0,d2
	add.w	d1,d3
	swap	d2
	swap	d3
	clr.w	d2
	clr.w	d3
	move.l	d2,d0
	move.l	d3,d1
	jsr	Movement_check_3D		; Try
	bne.s	.Go_on
	swap	d0			; Calculate map coordinates
	swap	d1
	jsr	Dungeon_to_map
	cmp.w	Map_Xcoord,d0		; Any change ?
	bne.s	.Go_on
	cmp.w	Map_Ycoord,d1
	bne.s	.Go_on
	move.l	d2,Player_X		; No -> Set new X & Y
	move.l	d3,Player_Y
	jsr	M3_DisUpd			; Display
	jsr	Map_DisUpd
	jsr	Switch_screens
	dbra	d7,.Loop			; Try again
.Go_on:	move.w	d4,d0			; Restore angle
	sub.w	#slang,d0			; Invert
	neg.w	d0
	and.w	#slang-1,d0
	swap	d0			; Set final angle
	clr.w	d0
	move.l	d0,Y_angle
.Done:	jsr	M3_DisUpd			; Display
	jsr	Map_DisUpd
	jsr	Update_screen
	jsr	Save_coordinates		; To avoid sudden jumps
.Exit:	moveq.l	#25,d0			; Wait
	jsr	Delay
.Exit2:	movem.l	(sp)+,d0-d5/d7
	rts
*/

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

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_NPC_sound_effects
 * FUNCTION  : Initialize NPC sound effects.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 13:44
 * LAST      : 26.07.95 13:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_NPC_sound_effects(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_NPC_sound_effects
 * FUNCTION  : Exit NPC sound effects.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 13:44
 * LAST      : 26.07.95 13:44
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_NPC_sound_effects
 * FUNCTION  : Add an NPC's sound effects.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 13:03
 * LAST      : 28.07.95 13:03
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_NPC_sound_effects(UNSHORT NPC_index)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Remove_NPC_sound_effects
 * FUNCTION  : Remove an NPC's sound effects.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 13:03
 * LAST      : 28.07.95 13:03
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Remove_NPC_sound_effects(UNSHORT NPC_index)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_NPC_sound_effects
 * FUNCTION  : Update NPC sound effects.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 13:45
 * LAST      : 26.07.95 13:45
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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_NPC_present_status
 * FUNCTION  : Update the present flags of all NPCs in the current map.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.05.95 15:19
 * LAST      : 02.08.95 13:37
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
	BOOLEAN Present;
	UNSHORT i;

	for (i=0;i<NPCS_PER_MAP;i++)
	{
		/* Anyone there ? */
		if (VNPCs[i].Number)
		{
			/* Yes -> Get presence status */
			Present = !(Read_bit_array(CD_BIT_ARRAY, ((PARTY_DATA.Map_nr - 1) *
			 NPCS_PER_MAP) + i));

			/* Is there a change ? */
			if (VNPCs[i].Present != Present)
			{
				/* Yes -> Appearing or disappearing ? */
				if (Present)
				{
					/* Appearing -> Relative path ? */
					if (VNPCs[i].Movement_type == REL_PATH_MOVEMENT)
					{
						/* Yes -> Reset relative path */
						VNPCs[i].Current_path_index = 0;
					}
				}
				else
				{
					/* Disappearing -> On a path ? */
					if ((VNPCs[i].Movement_type == ABS_PATH_MOVEMENT) ||
					 (VNPCs[i].Movement_type == REL_PATH_MOVEMENT))
					{
						/* Yes -> Reset coordinates */
						VNPCs[i].Map_X			= 0;
						VNPCs[i].Map_Y			= 0;
						VNPCs[i].Source_X		= 0;
						VNPCs[i].Source_Y		= 0;
						VNPCs[i].Target_X		= 0;
						VNPCs[i].Target_Y		= 0;
						VNPCs[i].Old_X			= 0;
						VNPCs[i].Old_Y			= 0;
						VNPCs[i].Display_X	= 0;
						VNPCs[i].Display_Y	= 0;
					}
				}

				/* Store new present status */
				VNPCs[i].Present = Present;
			}
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
 * NAME      : Lock_NPC
 * FUNCTION  : Lock an NPC.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 12:19
 * LAST      : 12.08.95 23:30
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Lock_NPC(UNSHORT NPC_index)
{
	/* Legal NPC index ? */
	if (NPC_index < NPCS_PER_MAP)
	{
		/* Yes -> Lock NPC */
		VNPCs[NPC_index].Flags |= NPC_LOCKED;

		/* Clear move step */
		VNPCs[NPC_index].Locked_move_step = 0;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unlock_NPC
 * FUNCTION  : Unlock an NPC.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 12:19
 * LAST      : 11.08.95 12:19
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Unlock_NPC(UNSHORT NPC_index)
{
	/* Legal NPC index ? */
	if (NPC_index < NPCS_PER_MAP)
	{
		/* Yes -> Unlock NPC */
		VNPCs[NPC_index].Flags &= ~NPC_LOCKED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Unlock_all_NPCs
 * FUNCTION  : Unlock all NPCs.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.95 12:19
 * LAST      : 11.08.95 12:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Unlock_all_NPCs(void)
{
	UNSHORT i;

	/* Unlock all NPCs */
	for (i=0;i<NPCS_PER_MAP;i++)
	{
		Unlock_NPC(i);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_NPC_position
 * FUNCTION  : Set the position of an NPC.
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 11:10
 * LAST      : 12.08.95 11:10
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 *             UNSHORT New_X - New X-coordinate.
 *             UNSHORT New_Y - New Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_NPC_position(UNSHORT NPC_index, UNSHORT New_X, UNSHORT New_Y)
{
	/* Legal NPC index ? */
	if (NPC_index < NPCS_PER_MAP)
	{
		/* Yes -> Change NPC position */
		VNPCs[NPC_index].Map_X = New_X;
		VNPCs[NPC_index].Map_Y = New_Y;

		/* Set source and target coordinates */
		VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Map_X;
		VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Map_Y;
		VNPCs[NPC_index].Target_X = VNPCs[NPC_index].Map_X;
		VNPCs[NPC_index].Target_Y = VNPCs[NPC_index].Map_Y;

		/* Set old coordinates */
		VNPCs[NPC_index].Old_X = 0xFFFF;
		VNPCs[NPC_index].Old_Y = 0xFFFF;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Move_locked_NPC
 * FUNCTION  : Move a locked NPC (for scripts only).
 * FILE      : NPCS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.08.95 23:03
 * LAST      : 12.08.95 23:23
 * INPUTS    : UNSHORT NPC_index - NPC index (0...NPCS_PER_MAP - 1).
 *             SISHORT dX - X-component of movement vector.
 *             SISHORT dY - Y-component of movement vector.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Move_locked_NPC(UNSHORT NPC_index, SISHORT dX, SISHORT dY)
{
	/* Legal NPC index ? */
	if (NPC_index < NPCS_PER_MAP)
	{
		/* Set source to target coordinates */
		VNPCs[NPC_index].Source_X = VNPCs[NPC_index].Target_X;
		VNPCs[NPC_index].Source_Y = VNPCs[NPC_index].Target_Y;

		/* Set new target coordinates */
		VNPCs[NPC_index].Target_X = VNPCs[NPC_index].Map_X + dX;
		VNPCs[NPC_index].Target_Y = VNPCs[NPC_index].Map_Y + dY;

		/* Clear movement counter */
		VNPCs[NPC_index].Locked_move_step = 0;
	}
}

