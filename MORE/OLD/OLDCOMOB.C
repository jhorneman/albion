/************
 * NAME     : COMOBS.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO : COMOBS.H
 ************/

/* includes */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBOPM.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBEVENT.H>

#include <FINDCOL.H>
#include <SORT.H>
#include <GFXFUNC.H>
#include <ZOOMFUNC.H>

#include <GAMEVAR.H>
#include <TACTICAL.H>
#include <COMBAT.H>
#include <COMOBS.H>
#include <COMSHOW.H>
#include <XFTYPES.H>

/* global variables */

UNSHORT Combat_display_detail_level = DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL;

UNSHORT Combat_projection_factor = COMBAT_PROJ_FACTOR;
UNSHORT Combat_camera_height = COMBAT_CAMERA_HEIGHT;
SISHORT Combat_Z_offset = COMBAT_Z_OFFSET;

BOOLEAN Combat_grid_flag = FALSE;

static UNSHORT Nr_COMOBs;
static UNSHORT New_nr_COMOBs;
static UNSHORT Nr_COMOB_behaviours;

static struct COMOB *COMOB_sort_list[MAX_COMOBS];

static struct COMOB COMOB_table[MAX_COMOBS];
static struct COMOB Dummy_COMOB;

static struct COMOB_behaviour COMOB_behaviour_list[MAX_COMOB_BEHAVIOURS];
static struct COMOB_behaviour Dummy_COMOB_behaviour;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_COMOB_system
 * FUNCTION  : Initialize COMbat OBject system.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:27
 * LAST      : 22.03.95 10:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_COMOB_system(void)
{
	UNSHORT i;

	/* Clear counters */
	Nr_COMOBs = 0;
	New_nr_COMOBs = 0;
	Nr_COMOB_behaviours = 0;

	/* Clear COMOB sort list */
	for (i=0;i<MAX_COMOBS;i++)
	{
		COMOB_sort_list[i] = NULL;
	}

	/* Clear COMOB table */
	BASEMEM_FillMemByte((UNBYTE *) &COMOB_table[0], MAX_COMOBS *
	 sizeof(struct COMOB), 0);
	BASEMEM_FillMemByte((UNBYTE *) &Dummy_COMOB, sizeof(struct COMOB), 0);

	/* Clear COMOB behaviour list */
	BASEMEM_FillMemByte((UNBYTE *) &COMOB_behaviour_list[0],
	 MAX_COMOB_BEHAVIOURS * sizeof(struct COMOB_behaviour), 0);
	BASEMEM_FillMemByte((UNBYTE *) &Dummy_COMOB_behaviour,
	 sizeof(struct COMOB_behaviour), 0);

	/* Initialize COMOB behaviour list */
	for (i=0;i<MAX_COMOB_BEHAVIOURS;i++)
	{
		COMOB_behaviour_list[i].Index = i;
	}
	Dummy_COMOB_behaviour.Index = 0xFFFF;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Add_COMOB
 * FUNCTION  : Add a COMOB to the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:37
 * LAST      : 06.03.95 13:37
 * INPUTS    : None.
 * RESULT    : struct COMOB *COMOB - Pointer to new COMOB.
 * BUGS      : No known.
 * NOTES     : - The COMOB data *must* be initialized before the COMOBs
 *              are drawn.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB *
Add_COMOB(void)
{
	struct COMOB *Output = &Dummy_COMOB;
	UNSHORT i;

	/* Find free slot in table */
	for (i=0;i<MAX_COMOBS;i++)
	{
		/* Free ? */
		if (!(COMOB_table[i].System_flags & COMOB_PRESENT))
		{
			/* Yes */
			Output = &COMOB_table[i];

			/* Clear COMOB data */
			BASEMEM_FillMemByte((UNBYTE *) Output, sizeof(struct COMOB), 0);

			/* Indicate this COMOB is present */
			Output->System_flags |= COMOB_PRESENT;
			Output->System_flags &= ~COMOB_ACTIVE;

			/* Set default priority */
			Output->Priority = 100;

			/* Add to sort list */
			COMOB_sort_list[New_nr_COMOBs] = &COMOB_table[i];

			/* Count up */
			New_nr_COMOBs++;

			break;
		}
	}
	return(Output);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_COMOB
 * FUNCTION  : Remove a COMOB from the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:38
 * LAST      : 24.05.95 16:21
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB that must be removed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function is re-entrant. All COMOBs attached to the
 *              given COMOB will also be deleted.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_COMOB(struct COMOB *COMOB)
{
	struct COMOB *Other_COMOB;
	UNSHORT Index;

	/* Exit if this is the dummy COMOB */
	if (COMOB == &Dummy_COMOB)
		return;

	/* Delete */
	COMOB->System_flags &= ~(COMOB_PRESENT | COMOB_ACTIVE);

	/* Delete behaviours referring to this COMOB */
	Index = 0;
	while (Index < Nr_COMOB_behaviours)
	{
		/* Does this behaviour refer to the COMOB that has been deleted ? */
		if (COMOB_behaviour_list[Index].First_COMOB == COMOB)
		{
			/* Yes -> Get pointer to other COMOB */
			Other_COMOB = COMOB_behaviour_list[Index].Second_COMOB;

			/* Delete the behaviour */
			Delete_COMOB_behaviour(&COMOB_behaviour_list[Index]);

			/* Did this behaviour have another COMOB attached (and it wasn't
			 the one we're currently deleting) ? */
			if (Other_COMOB && (Other_COMOB != COMOB))
			{
				/* Yes -> Delete this COMOB as well */
				Delete_COMOB(Other_COMOB);

				/* Start all over again */
				Index = 0;
			}
		}
		else
		{
			/* No -> Does this behaviour refer to the COMOB that has been
			 deleted ? */
			if (COMOB_behaviour_list[Index].Second_COMOB == COMOB)
			{
				/* Yes -> Get pointer to other COMOB */
				Other_COMOB = COMOB_behaviour_list[Index].First_COMOB;

				/* Delete the behaviour */
				Delete_COMOB_behaviour(&COMOB_behaviour_list[Index]);

				/* Delete the other COMOB as well */
				Delete_COMOB(Other_COMOB);

				/* Start all over again */
				Index = 0;
			}
			else
			{
				/* No -> Next behaviour */
				Index++;
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Hide_COMOB
 * FUNCTION  : Hide a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.05.95 13:21
 * LAST      : 16.05.95 13:21
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Hide_COMOB(struct COMOB *COMOB)
{
	COMOB->System_flags |= COMOB_HIDDEN;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Show_COMOB
 * FUNCTION  : Show a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 16.05.95 13:22
 * LAST      : 16.05.95 13:22
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Show_COMOB(struct COMOB *COMOB)
{
	COMOB->System_flags &= ~COMOB_HIDDEN;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Add_COMOB_behaviour
 * FUNCTION  : Add a COMOB behaviour to the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.03.95 11:05
 * LAST      : 22.03.95 11:05
 * INPUTS    : struct COMOB *First_COMOB - Pointer to behaving COMOB.
 *             struct COMOB *Second_COMOB - Pointer to attached COMOB.
 *             COMOB_behaviour_handler Handler - Pointer to handler function.
 * RESULT    : union COMOB_behaviour_data * : Pointer to behaviour data.
 * BUGS      : No known.
 * NOTES     : - The behaviour will be attached to the first COMOB. The
 *              behaviour may require a pointer to a second attached COMOB.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

union COMOB_behaviour_data *
Add_COMOB_behaviour(struct COMOB *First_COMOB, struct COMOB *Second_COMOB,
 COMOB_behaviour_handler Handler)
{
	struct COMOB_behaviour *Behaviour;

	/* Is there room in the list / is dummy COMOB ? */
	if ((Nr_COMOB_behaviours < MAX_COMOB_BEHAVIOURS) ||
	 (First_COMOB == &Dummy_COMOB))
	{
		/* Yes -> Get pointer to behaviour */
		Behaviour = &(COMOB_behaviour_list[Nr_COMOB_behaviours]);

		/* Count up */
		Nr_COMOB_behaviours++;
	}
	else
	{
		/* No -> Return dummy data */
		Behaviour = &Dummy_COMOB_behaviour;
	}

	/* Clear behaviour data */
	BASEMEM_FillMemByte((UNBYTE *) &(Behaviour->Data),
	 sizeof(union COMOB_behaviour_data), 0);

	/* Initialize behaviour */
	Behaviour->First_COMOB = First_COMOB;
	Behaviour->Second_COMOB = Second_COMOB;
	Behaviour->Handler = Handler;

	/* Return pointer to behaviour data */
	return &(Behaviour->Data);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_COMOB_behaviour
 * FUNCTION  : Remove a COMOB behaviour from the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.03.95 11:08
 * LAST      : 22.03.95 11:08
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to COMOB behaviour
 *              that must be removed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Due to the parameter type used, only behaviour handlers or
 *              functions that access the global list can delete behaviours.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_COMOB_behaviour(struct COMOB_behaviour *Behaviour)
{
	UNSHORT Index;
	UNSHORT i;

	/* Get the behaviour index */
	Index = Behaviour->Index;

	/* Is this the dummy behaviour ? */
	if (Index != 0xFFFF)
	{
		/* No -> Copy all following behaviours down */
		for (i=Index + 1;i<Nr_COMOB_behaviours;i++)
		{
			memcpy((UNBYTE *) &COMOB_behaviour_list[i-1],
			 (UNBYTE *) &COMOB_behaviour_list[i],
			 sizeof(struct COMOB_behaviour));

			COMOB_behaviour_list[i-1].Index = i - 1;
		}

		/* Count down */
		Nr_COMOB_behaviours--;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_combat_screen
 * FUNCTION  : Draw combat screen main 3D window.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:40
 * LAST      : 06.03.95 13:40
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_combat_screen(void)
{
	struct COMOB *COMOB;
	struct BBRECT Clip, Old;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Clean up COMOB list */
	Clean_up_COMOB_list();

	/* Any COMOBs ? */
	if (Nr_COMOBs)
	{
		/* Yes -> Sort COMOBs */
		Shuttlesort(Swap_COMOBs, Compare_COMOBs, Nr_COMOBs,
		 (UNBYTE *) &(COMOB_sort_list[0]));

		/* Make clipping rectangle */
		Clip.left = 0;
		Clip.top = 0;
		Clip.width = Screen_width;
		Clip.height = Panel_Y;

		/* Install clip area */
		memcpy(&Old, &(Main_OPM.clip), sizeof(struct BBRECT));
		memcpy(&(Main_OPM.clip), &Clip, sizeof(struct BBRECT));

		/* Display combat background */
		Ptr = MEM_Claim_pointer(Combat_background_handle);
		// Handle background animation
		Put_unmasked_block(&Main_OPM, COMBAT_X, COMBAT_Y, COMBAT_WIDTH,
		 COMBAT_HEIGHT, Ptr);
		MEM_Free_pointer(Combat_background_handle);

		/* Draw grid ? */
		if (Combat_grid_flag)
			Draw_combat_grid();

	 	/* Display all shadow COMOBs */
		for (i=0;i<Nr_COMOBs;i++)
		{
			/* Get COMOB data */
			COMOB = COMOB_sort_list[i];

			/* Is shadow ? */
			if (COMOB->User_flags & COMOB_SHADOW)
			{
				/* Yes -> Draw */
				Draw_COMOB(COMOB);
			}
		}

	 	/* Display all normal COMOBs */
		for (i=0;i<Nr_COMOBs;i++)
		{
			/* Get COMOB data */
			COMOB = COMOB_sort_list[i];

			/* Is shadow ? */
			if (!(COMOB->User_flags & COMOB_SHADOW))
			{
				/* No -> Draw */
				Draw_COMOB(COMOB);
			}
		}

		/* Restore clip area */
		memcpy(&(Main_OPM.clip), &Old, sizeof(struct BBRECT));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_COMOBs
 * FUNCTION  : Update all COMOBs.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 18.03.95 15:27
 * LAST      : 24.05.95 19:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function executes all COMOB behaviours.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_COMOBs(void)
{
	struct COMOB *COMOB;
	UNSHORT i;

	/* Handle standard behaviour */
	for (i=0;i<Nr_COMOBs;i++)
	{
		/* Get COMOB data */
		COMOB = COMOB_sort_list[i];

		/* Active ? */
		if (COMOB->System_flags & COMOB_ACTIVE)
		{
			/* Yes -> Limited lifespan ? */
			if (COMOB->Lifespan)
			{
				/* Yes -> Decrease */
				COMOB->Lifespan--;

				/* Time to die ? */
				if (!COMOB->Lifespan)
				{
					/* Yes -> Kill */
					Delete_COMOB(COMOB);

					/* Next COMOB */
					continue;
				}
			}

			/* Evaluate movement vector */
			COMOB->X_3D += COMOB->dX_3D;
			COMOB->Y_3D += COMOB->dY_3D;
			COMOB->Z_3D += COMOB->dZ_3D;

			/* Animated ? */
			if (COMOB->Nr_frames)
			{
				/* Yes -> Wave or circle ? */
				if (COMOB->User_flags & COMOB_WAVEANIM)
				{
					/* Wave */
					Wave_COMOB(COMOB);
				}
				else
				{
					/* Circle */
					Circle_COMOB(COMOB);
				}
			}
		}
	}

	/* Handle all behaviours */
	for (i=0;i<Nr_COMOB_behaviours;i++)
	{
		/* Is the first COMOB active ? */
		if (!(COMOB_behaviour_list[i].First_COMOB->System_flags & COMOB_ACTIVE))
			continue;

		/* Is there a second COMOB ? */
		if (COMOB_behaviour_list[i].Second_COMOB)
		{
			/* Yes -> Is it active ? */
			if (!(COMOB_behaviour_list[i].Second_COMOB->System_flags &
			 COMOB_ACTIVE))
				continue;
		}

		/* Behave */
		(COMOB_behaviour_list[i].Handler)(&COMOB_behaviour_list[i]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clean_up_COMOB_list
 * FUNCTION  : Remove all deleted COMOBs, activate new COMOBs, clean up list,
 *              and count remaining COMOBs.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:40
 * LAST      : 24.05.95 19:42
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clean_up_COMOB_list(void)
{
	struct COMOB *COMOB;
	UNSHORT Counter = 0;
	UNSHORT Target = 0;
	UNSHORT i;

	for (i=0;i<New_nr_COMOBs;i++)
	{
		/* Get COMOB data */
		COMOB = COMOB_sort_list[i];

		/* Deleted ? */
		if (COMOB->System_flags & COMOB_PRESENT)
		{
			/* No -> Active ? */
			if (!(COMOB->System_flags & COMOB_ACTIVE))
			{
				/* No -> Activate */
				COMOB->System_flags |= COMOB_ACTIVE;
			}

			/* Difference between source and target ? */
			if (Target != i)
			{
				/* Yes -> Copy this COMOB down */
				COMOB_sort_list[Target] = COMOB;
			}

			/* Count up */
			Target++;
			Counter++;
		}
	}

	/* Store new counter */
	Nr_COMOBs = Counter;
	New_nr_COMOBs = Counter;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_COMOB
 * FUNCTION  : Draw a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 14:29
 * LAST      : 17.03.95 12:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_COMOB(struct COMOB *COMOB)
{
	static UNSHORT Draw_mode_detail_levels[5] = {
		0,		/* NORMAL_COMOB_DRAWMODE		*/
 		25,	/* SILHOUETTE_COMOB_DRAWMODE	*/
 		75,	/* COLOURED_COMOB_DRAWMODE		*/
 		50,	/* COLOURING_COMOB_DRAWMODE	*/
 		100,	/* TRANSPARENT_COMOB_DRAWMODE */
	};

	struct Gfx_header *Gfx;
	SILONG X_3D, Y_3D, Z_3D;
	SILONG X_2D, Y_2D;
	SILONG Proj;
	UNLONG Target_width, Target_height;
	UNSHORT Draw_mode;
	UNSHORT Source_width, Source_height;
	UNSHORT i;
	UNBYTE *Ptr;
	UNBYTE *Table;

	/* Priority high enough ? */
	if (COMOB->Priority >= (MAX_COMBAT_DISPLAY_DETAIL_LEVEL -
	 Combat_display_detail_level))
	{
		/* Yes -> Get 3D coordinates */
		X_3D = COMOB->X_3D / COMOB_DEC_FACTOR;
		Y_3D = COMOB->Y_3D / COMOB_DEC_FACTOR;
		Z_3D = (COMOB->Z_3D / COMOB_DEC_FACTOR) + Combat_Z_offset;

		/* In front of the camera ? */
		if (Z_3D >= MINIMUM_Z)
		{
			/* Yes -> Adapt Y-coordinate to camera height */
			Y_3D -= Combat_camera_height;

			/* Calculate projection factor */
			Proj = Z_3D + Combat_projection_factor;
			if (!Proj)
				Proj = 1;

			/* Project coordinates */
			X_2D = (X_3D * Combat_projection_factor) / Proj;
			Y_2D = (Y_3D * Combat_projection_factor) / Proj;

			/* Translate coordinates to centre of the screen & flip Y-coordinate */
			X_2D += COMBAT_X + (COMBAT_WIDTH / 2);
			Y_2D = COMBAT_Y + (COMBAT_HEIGHT / 2) - Y_2D;

			/* Find current frame */
			Gfx = (struct Gfx_header *)
			 (MEM_Claim_pointer(COMOB->Graphics_handle)
			 + COMOB->Graphics_offset);

			for (i=0;i<COMOB->Frame;i++)
			{
				Ptr = (UNBYTE *) (Gfx + 1);
				Gfx = (struct Gfx_header *) (Ptr + (Gfx->Width * Gfx->Height));
			}
			Ptr = (UNBYTE *) (Gfx + 1);

			/* Get source dimensions */
			Source_width = Gfx->Width;
			Source_height = Gfx->Height;

			/* Project display size */
			Target_width = (((Source_width * COMOB->Display_width) / 100) *
			 Combat_projection_factor) / Proj;
			Target_height = (((Source_height * COMOB->Display_height) / 100) *
			 Combat_projection_factor) / Proj;

			/* Adjust coordinates to hot-spot */
			X_2D -= (Target_width * COMOB->Hotspot_X_offset) / 100;
			Y_2D -= (Target_height * COMOB->Hotspot_Y_offset) / 100;

			/* Get draw mode */
			Draw_mode = COMOB->Draw_mode;

			/* Switch to normal depending on current detail level */
			if (Draw_mode_detail_levels[Draw_mode] > Combat_display_detail_level)
				Draw_mode = NORMAL_COMOB_DRAWMODE;

			/* Act depending on draw mode */
			switch (Draw_mode)
			{
				/* Normal */
				case NORMAL_COMOB_DRAWMODE:
				{
					Put_zoomed_block(&Main_OPM, X_2D, Y_2D, Source_width,
					 Source_height, Target_width, Target_height, Ptr);
					break;
				}
				/* Silhouette */
				case SILHOUETTE_COMOB_DRAWMODE:
				{
					Put_zoomed_silhouette(&Main_OPM, X_2D, Y_2D, Source_width,
					 Source_height, Target_width, Target_height, Ptr,
					 COMOB->Colour);
					break;
				}
				/* Recoloured */
				case COLOURED_COMOB_DRAWMODE:
				{
					Table = MEM_Claim_pointer(COMOB->Special_handle) +
					 COMOB->Special_offset;

					Put_zoomed_recoloured_block(&Main_OPM, X_2D, Y_2D,
					 Source_width, Source_height, Target_width, Target_height,
					 Ptr, Table);

					MEM_Free_pointer(COMOB->Special_handle);
					break;
				}
				/* Recolouring the background */
				case COLOURING_COMOB_DRAWMODE:
				{
					Table = MEM_Claim_pointer(COMOB->Special_handle) +
					 COMOB->Special_offset;

					Put_zoomed_recoloured_silhouette(&Main_OPM, X_2D, Y_2D,
					 Source_width, Source_height, Target_width, Target_height,
					 Ptr, Table);

					MEM_Free_pointer(COMOB->Special_handle);
					break;
				}
				/* Transparent */
				case TRANSPARENT_COMOB_DRAWMODE:
				{
					Table = MEM_Claim_pointer(COMOB->Special_handle) +
					 COMOB->Special_offset;

					Put_zoomed_transparent_block(&Main_OPM, X_2D, Y_2D,
					 Source_width, Source_height, Target_width, Target_height,
					 Ptr, Table);

					MEM_Free_pointer(COMOB->Special_handle);
					break;
				}
			}

			MEM_Free_pointer(COMOB->Graphics_handle);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Swap_COMOBs
 * FUNCTION  : Swap two COMOBs (for sorting).
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 15:18
 * LAST      : 06.03.95 15:18
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Swap_COMOBs(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct COMOB *T, **List;

	List = (struct COMOB **) Data;

	T = List[A];
	List[A] = List[B];
	List[B] = T;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Compare_COMOBs
 * FUNCTION  : Compare two COMOBs (for sorting).
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 15:19
 * LAST      : 06.03.95 15:19
 *	INPUTS    : UNLONG A - Index of first element (0...).
 *	            UNLONG B - Index of second element (0...).
 *	            UNBYTE *Data - Pointer to data passed by Sort() caller.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Compare_COMOBs(UNLONG A, UNLONG B, UNBYTE *Data)
{
	struct COMOB **List;

	List = (struct COMOB **) Data;

	return(List[A]->Z_3D < List[B]->Z_3D);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Circle_COMOB
 * FUNCTION  : Do circle animation on COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 15:19
 * LAST      : 20.03.95 14:59
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB that must be animated.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Circle_COMOB(struct COMOB *COMOB)
{
	UNSHORT Nr_frames;
	UNSHORT Frame;

	/* Get number of frames */
	Nr_frames = COMOB->Nr_frames;

	/* Any animation ? */
	if (Nr_frames > 1)
	{
		/* Yes -> Get current frame */
		Frame = COMOB->Frame;

		/* Next frame */
		Frame++;

		/* Reached last frame ? */
		if (Frame >= Nr_frames)
		{
			/* Yes -> Back to the start */
			Frame = 0;
		}

		/* Store new frame */
		COMOB->Frame = Frame;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Wave_COMOB
 * FUNCTION  : Do wave animation on COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 15:22
 * LAST      : 20.03.95 14:59
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB that must be animated.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Wave_COMOB(struct COMOB *COMOB)
{
	UNSHORT Nr_frames;
	SISHORT Frame;

	/* Get number of frames */
	Nr_frames = COMOB->Nr_frames;

	/* More than 2 frames ? */
	if (Nr_frames > 2)
	{
		/* Yes -> Get current frame */
		Frame = COMOB->Frame;

		/* Going back or forth ? */
		if (COMOB->System_flags & COMOB_WAVEDIR)
		{
			/* Back -> Previous frame */
			Frame--;

			/* Reached first frame ? */
			if (Frame < 1)
			{
				/* Yes -> Reverse direction */
				COMOB->System_flags &= ~COMOB_WAVEDIR;
			}
		}
		else
		{
			/* Forth -> Next frame */
			Frame++;

			/* Reached last frame ? */
			if (Frame >= Nr_frames)
			{
				/* Yes -> Reverse direction */
				COMOB->System_flags |= COMOB_WAVEDIR;
				Frame -= 2;
			}
		}
		/* Store new frame */
		COMOB->Frame = Frame;
	}
	else
	{
		/* No -> Do circle animation */
		Circle_COMOB(COMOB);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transparency_table
 * FUNCTION  : Calculate a transparency table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.03.95 15:30
 * LAST      : 07.03.95 15:30
 * INPUTS    : UNSHORT Intensity - Intensity in %.
 * RESULT    : MEM_HANDLE : Handle of transparency table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Calculate_transparency_table(UNSHORT Intensity)
{
	MEM_HANDLE Handle;
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Allocate memory for recolouring table */
	Handle = MEM_Allocate_memory(256 * 256);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Calculate the transparency  table */
		Ptr = MEM_Claim_pointer(Handle);

		for (i=0;i<256;i++)
		{
			/* Get the current target colour */
			Target_R = Palette.color[i].red;
			Target_G = Palette.color[i].green;
			Target_B = Palette.color[i].blue;

			for (j=0;j<256;j++)
			{
				/* Same colours ? */
				if (i == j)
				{
					/* Yes -> Don't change */
					*Ptr++ = i;
				}
				else
				{
					/* No -> Get the current source colour */
					Source_R = Palette.color[j].red;
					Source_G = Palette.color[j].green;
					Source_B = Palette.color[j].blue;

					/* Interpolate towards the target colour */
					Source_R = ((Target_R - Source_R) * (100 - Intensity)) / 100 + Source_R;
					Source_G = ((Target_G - Source_G) * (100 - Intensity)) / 100 + Source_G;
					Source_B = ((Target_B - Source_B) * (100 - Intensity)) / 100 + Source_B;

					/* Find the closest matching colour in the palette */
					*Ptr++ = Find_closest_colour(Source_R, Source_G, Source_B);
				}
			}
		}
		MEM_Free_pointer(Handle);
	}

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_luminance_table
 * FUNCTION  : Calculate a luminance table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:34
 * LAST      : 20.03.95 17:34
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Handle of luminance table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Calculate_luminance_table(void)
{
	MEM_HANDLE Handle;
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT Source_brightness;
	UNSHORT Target_brightness;
	UNSHORT i, j;
	UNBYTE *Base, *Ptr1, *Ptr2;
	UNBYTE Colour;

	/* Allocate memory for recolouring table */
	Handle = MEM_Allocate_memory(256 * 256);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Calculate the transparency  table */
		Base = MEM_Claim_pointer(Handle);

		for (i=0;i<256;i++)
		{
			Ptr1 = Base + (i * 256) + i;
			Ptr2 = Ptr1;

			/* Get the current target colour */
			Target_R = Palette.color[i].red;
			Target_G = Palette.color[i].green;
			Target_B = Palette.color[i].blue;

			/* Calculate the target colour's brightness */
			Target_brightness = (Target_R + Target_G + Target_B);

			for (j=i;j<256;j++)
			{
				/* Same colours ? */
				if (i == j)
				{
					/* Yes -> Don't change */
					Colour = i;
				}
				else
				{
					/* No ->	Get the current source colour */
					Source_R = Palette.color[j].red;
					Source_G = Palette.color[j].green;
					Source_B = Palette.color[j].blue;

					/* Calculate the source colour's brightness */
					Source_brightness = (Source_R + Source_G + Source_B);

					/* Is the source colour brighter than the target colour ? */
					if (Source_brightness > Target_brightness)
					{
						/* Yes -> Draw source colour */
						Colour = j;
					}
					else
					{
						/* No -> Draw target colour */
						Colour = i;
					}
				}

				/* Store colours  and increase pointers */
				*Ptr1 = Colour;
				*Ptr2 = Colour;
				Ptr1++;
				Ptr2 += 256;
			}
		}
		MEM_Free_pointer(Handle);
	}

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Calculate_transluminance_table
 * FUNCTION  : Calculate a transluminance table.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:47
 * LAST      : 20.03.95 17:47
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Handle of transluminance table.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Calculate_transluminance_table(void)
{
	MEM_HANDLE Handle;
	SISHORT Source_R, Source_G, Source_B;
	SISHORT Target_R, Target_G, Target_B;
	UNSHORT Target_brightness;
	UNSHORT i, j;
	UNBYTE *Ptr;

	/* Allocate memory for recolouring table */
	Handle = MEM_Allocate_memory(256 * 256);

	/* Success ? */
	if (Handle)
	{
		/* Yes -> Calculate the transparency  table */
		Ptr = MEM_Claim_pointer(Handle);

		for (i=0;i<256;i++)
		{
			/* Get the current target colour */
			Target_R = Palette.color[i].red;
			Target_G = Palette.color[i].green;
			Target_B = Palette.color[i].blue;

			/* Calculate the target colour's brightness */
			Target_brightness = (Target_R + Target_G + Target_B) / 3;

			for (j=0;j<256;j++)
			{
				/* No -> Get the current source colour */
				Source_R = Palette.color[j].red;
				Source_G = Palette.color[j].green;
				Source_B = Palette.color[j].blue;

				/* Determine the target colour */
				Source_R += Target_brightness;
				Source_G += Target_brightness;
				Source_B += Target_brightness;

				/* Clip colour values */
				if (Source_R > 255)
					Source_R = 255;

				if (Source_G > 255)
					Source_G = 255;

				if (Source_B > 255)
					Source_B = 255;

				/* Find the closest matching colour in the palette */
				*Ptr++ = Find_closest_colour(Source_R, Source_G, Source_B);
			}
		}
		MEM_Free_pointer(Handle);
	}

	return(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Bounce_handler
 * FUNCTION  : Bounce COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.03.95 17:26
 * LAST      : 22.03.95 11:31
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Bounce_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;

	/* Get COMOB data */
	COMOB = Behaviour->First_COMOB;

	/* Implement air friction */
	COMOB->dX_3D = (COMOB->dX_3D * (1000 -
	 (SILONG) Behaviour->Data.Bounce_data.Air_friction)) / 1000;
	COMOB->dY_3D = (COMOB->dY_3D * (1000 -
	 (SILONG) Behaviour->Data.Bounce_data.Air_friction)) / 1000;
	COMOB->dZ_3D = (COMOB->dZ_3D * (1000 -
	 (SILONG) Behaviour->Data.Bounce_data.Air_friction)) / 1000;

	/* Implement gravity */
	COMOB->dY_3D -= (SILONG) Behaviour->Data.Bounce_data.Gravity;

	/* Evaluate movement vector */
	COMOB->X_3D += COMOB->dX_3D;
	COMOB->Y_3D += COMOB->dY_3D;
	COMOB->Z_3D += COMOB->dZ_3D;

	/* Reached the floor ? */
	if (Behaviour->Data.Bounce_data.Gravity && (COMOB->Y_3D <= 0))
	{
		/* Yes -> Bounce */
		COMOB->Y_3D = 0;
		COMOB->dY_3D = ((0 - COMOB->dY_3D) *
		 (UNLONG) Behaviour->Data.Bounce_data.Bounce) / 100;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Oscillate_handler
 * FUNCTION  : Oscillate COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.03.95 17:10
 * LAST      : 24.05.95 12:17
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Oscillate_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *COMOB;
	SILONG New_delta, Total_delta;
	UNSHORT Value;
	UNSHORT Period;

	/* Get pointer to COMOB */
	COMOB = Behaviour->First_COMOB;

	/* Get oscillate data */
	Value = Behaviour->Data.Oscillate_data.Value;
	Period = Behaviour->Data.Oscillate_data.Period;

	/* Calculate new delta value */
	New_delta = Behaviour->Data.Oscillate_data.Amplitude *
	 sin((Value * 2 * PI) / Period);

	/* Increase value */
	Value++;
	if (Value == Period)
		Value = 0;

	/* Write current value */
	Behaviour->Data.Oscillate_data.Value = Value;

	/* Calculate total delta */
	Total_delta = New_delta - Behaviour->Data.Oscillate_data.Old_delta;

	/* Store current delta for next time */
	Behaviour->Data.Oscillate_data.Old_delta = New_delta;

	/* What kind of oscillation ? */
	switch(Behaviour->Data.Oscillate_data.Type)
	{
		case OSCILLATE_X:
		{
			COMOB->X_3D += Total_delta;
			break;
		}
		case OSCILLATE_Y:
		{
			COMOB->Y_3D += Total_delta;
			break;
		}
		case OSCILLATE_Z:
		{
			COMOB->Z_3D += Total_delta;
			break;
		}
		case OSCILLATE_WIDTH:
		{
			COMOB->Display_width += (SISHORT) Total_delta;
			break;
		}
		case OSCILLATE_HEIGHT:
		{
			COMOB->Display_height += (SISHORT) Total_delta;
			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Trail_handler
 * FUNCTION  : Trail COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 24.05.95 15:41
 * LAST      : 29.05.95 15:35
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Trail_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *Trailing_COMOB;
	struct COMOB *Trailed_COMOB;
	SILONG dX_3D, dY_3D, dZ_3D;
	SILONG Length;
	SILONG Distance;

	/* Get pointer to trailing COMOB */
	Trailing_COMOB = Behaviour->First_COMOB;

 	/* Get pointer to trailed COMOB */
	Trailed_COMOB = Behaviour->Second_COMOB;

	/* Calculate vector */
	dX_3D = Trailed_COMOB->X_3D - Trailing_COMOB->X_3D;
	dY_3D = Trailed_COMOB->Y_3D - Trailing_COMOB->Y_3D;
	dZ_3D = Trailed_COMOB->Z_3D - Trailing_COMOB->Z_3D;

	/* Calculate vector length */
	Length = (dX_3D * dX_3D) + (dY_3D * dY_3D) + (dZ_3D * dZ_3D);
	if (Length)
	{
		Length = (SILONG) sqrt((double) Length);
	}

	/* Get current distance */
	Distance = (SILONG) Behaviour->Data.Trail_data.Current_distance;

	/* Is vector length zero ? */
	if (Length)
	{
		/* No -> Determine new distance */
		Distance += sgn(Length - Distance) * COMOB_DEC_FACTOR;

		/* Clip distance */
		Distance = min(max(Distance,
		 (SILONG) Behaviour->Data.Trail_data.Minimum_distance),
		 (SILONG) Behaviour->Data.Trail_data.Maximum_distance);

		/* Calculate vector components */
		dX_3D *= (Length - Distance);
		dY_3D *= (Length - Distance);
		dZ_3D *= (Length - Distance);

		dX_3D /= Length;
		dY_3D /= Length;
		dZ_3D /= Length;
	}
	else
	{
		/* Yes -> Clear vector */
		dX_3D = 0;
		dY_3D = 0;
		dZ_3D = 0;
	}

	/* Store vector components */
	Trailing_COMOB->X_3D += dX_3D;
	Trailing_COMOB->Y_3D += dY_3D;
	Trailing_COMOB->Z_3D += dZ_3D;

	/* Store current distance */
	Behaviour->Data.Trail_data.Current_distance = (UNSHORT) Distance;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Shadow_handler
 * FUNCTION  : Shadow COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.05.95 16:44
 * LAST      : 29.05.95 16:44
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only works for COMOBs with shadows interleaved
 *              between the normal animation frames.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Shadow_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB *Casting_COMOB;
	struct COMOB *Shadow_COMOB;

	/* Get pointer to casting COMOB */
	Casting_COMOB = Behaviour->Second_COMOB;

	/* Get pointer to shadow COMOB */
	Shadow_COMOB = Behaviour->First_COMOB;

	/* Copy casting COMOB's coordinates to shadow COMOB */
	Shadow_COMOB->X_3D = Casting_COMOB->X_3D;
	Shadow_COMOB->Z_3D = Casting_COMOB->Z_3D;

	/* Set Y-coordinate to 0 */
	Shadow_COMOB->Y_3D = 0;

	/* Copy animation frame */
	Shadow_COMOB->Frame = Casting_COMOB->Frame + 1;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_trail_to_COMOB
 * FUNCTION  : Add a trail of COMOBs to a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.06.95 16:49
 * LAST      : 08.06.95 16:49
 * INPUTS    : struct COMOB *Trailed_COMOB - Pointer to trailed COMOB.
 *             UNSHORT Nr_trailing_COMOBs - Number of COMOBs in trail.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_trail_to_COMOB(struct COMOB *Trailed_COMOB, UNSHORT Nr_trailing_COMOBs)
{
	union COMOB_behaviour_data *Behaviour_data;
	struct COMOB *Trailing_COMOB;
	UNSHORT Shrink_percentage;
	UNSHORT i;

	/* Calculate shrink percentage */
	Shrink_percentage = 100 - (100 / (Nr_trailing_COMOBs + 1));

	/* Create trailing COMOBs */
	for (i=0;i<Nr_trailing_COMOBs;i++)
	{
		/* Create trailing COMOB */
		Trailing_COMOB = Add_COMOB();

		/* Copy coordinates from trailed COMOB */
		Trailing_COMOB->X_3D = Trailed_COMOB->X_3D;
		Trailing_COMOB->Y_3D = Trailed_COMOB->Y_3D;
		Trailing_COMOB->Z_3D = Trailed_COMOB->Z_3D;

		/* Copy priority, minus one */
		Trailing_COMOB->Priority = max(Trailed_COMOB->Priority - 1, 0);

		/* Copy draw mode */
		Trailing_COMOB->Draw_mode = Trailed_COMOB->Draw_mode;
		Trailing_COMOB->Special_handle = Trailed_COMOB->Special_handle;
		Trailing_COMOB->Special_offset = Trailed_COMOB->Special_offset;

		/* Copy hotspot offsets */
		Trailing_COMOB->Hotspot_X_offset = Trailed_COMOB->Hotspot_X_offset;
		Trailing_COMOB->Hotspot_Y_offset = Trailed_COMOB->Hotspot_Y_offset;

		/* Copy display dimensions, but smaller */
		Trailing_COMOB->Display_width = (Trailed_COMOB->Display_width *
		 Shrink_percentage) / 100;
		Trailing_COMOB->Display_height = (Trailed_COMOB->Display_height *
		 Shrink_percentage) / 100;

		/* Copy graphics handle and offset */
		Trailing_COMOB->Graphics_handle = Trailed_COMOB->Graphics_handle;
		Trailing_COMOB->Graphics_offset = Trailed_COMOB->Graphics_offset;

		/* Copy number of animation frames */
		Trailing_COMOB->Nr_frames = Trailed_COMOB->Nr_frames;

		/* Copy current animation frame */
		Trailing_COMOB->Frame = Trailed_COMOB->Frame;

		/* Add trailing behaviour */
		Behaviour_data = Add_COMOB_behaviour(Trailing_COMOB, Trailed_COMOB,
		 Trail_handler);

		Behaviour_data->Trail_data.Minimum_distance = 10 * COMOB_DEC_FACTOR;
		Behaviour_data->Trail_data.Maximum_distance = 20 * COMOB_DEC_FACTOR;

		/* Next COMOB in the trail */
		Trailed_COMOB = Trailing_COMOB;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_shadow_to_COMOB
 * FUNCTION  : Add a shadow COMOB to a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.05.95 16:52
 * LAST      : 08.06.95 16:31
 * INPUTS    : struct COMOB *Casting_COMOB - Pointer to casting COMOB.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function only works for monster graphics COMOBs
 *              (because of the interleaved shadows).
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_shadow_to_COMOB(struct COMOB *Casting_COMOB)
{
	union COMOB_behaviour_data *Behaviour_data;
	struct COMOB *Shadow_COMOB;

	/* Create shadow COMOB */
	Shadow_COMOB = Add_COMOB();

	/* Copy casting COMOB's coordinates to shadow COMOB */
	Shadow_COMOB->X_3D = Casting_COMOB->X_3D;
	Shadow_COMOB->Z_3D = Casting_COMOB->Z_3D;

	/* Set Y-coordinate to 0 */
	Shadow_COMOB->Y_3D = 0;

	/* Copy animation frame */
	Shadow_COMOB->Frame = Casting_COMOB->Frame + 1;

	/* Copy display dimensions */
	Shadow_COMOB->Display_width = Casting_COMOB->Display_width;
	Shadow_COMOB->Display_height = Casting_COMOB->Display_height;

	/* Set shadow priority */
	Shadow_COMOB->Priority = 90;

	/* Set COMOB hot-spot */
	Shadow_COMOB->Hotspot_X_offset = 50;
	Shadow_COMOB->Hotspot_Y_offset = 50;

	/* Set shadow COMOB draw mode */
	Shadow_COMOB->User_flags |= COMOB_SHADOW;
	Shadow_COMOB->Draw_mode = COLOURING_COMOB_DRAWMODE;
	Shadow_COMOB->Special_handle = NULL;
	Shadow_COMOB->Special_offset = (UNLONG) &(Recolour_tables[0][0]);

	/* Set COMOB graphics handle and offset */
	Shadow_COMOB->Graphics_handle = Casting_COMOB->Graphics_handle;
	Shadow_COMOB->Graphics_offset = 0;

	/* Add shadow behaviour */
	Behaviour_data = Add_COMOB_behaviour(Shadow_COMOB, Casting_COMOB,
	 Shadow_handler);
}






void
Do_explosion(SILONG Source_X, SILONG Source_Y, SILONG Source_Z,
 UNSHORT Number)
{
	MEM_HANDLE Spark_gfx_handles[11], Ball_handle;
	struct COMOB *COMOB;
	union COMOB_behaviour_data *Behaviour_data;
	UNSHORT Spark_gfx_batch[11] = {2, 3, 4, 6, 7, 1, 5, 8, 9, 10, 11};
	UNSHORT i;

	/* The fight is on! */
	Fighting = TRUE;

	/* Delete interface objects */
	Delete_object(Tactical_window_object);

	/* Re-draw the screen */
	Update_screen();

 	Ball_handle = Load_subfile(COMBAT_GFX, 21);

	COMOB = Add_COMOB();

	COMOB->X_3D = Source_X;
	COMOB->Y_3D = Source_Y;
	COMOB->Z_3D = Source_Z;

	COMOB->Priority = 100;

	COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

	COMOB->Hotspot_X_offset = 50;
	COMOB->Hotspot_Y_offset = 50;

	COMOB->Lifespan = 100;

	COMOB->Display_width = 150;
	COMOB->Display_height = 150;

	COMOB->Graphics_handle = Ball_handle;

	COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);

	COMOB->Special_handle = Transluminance_table_handle;

	Load_full_batch(COMBAT_GFX, 11, &(Spark_gfx_batch[0]),
	 &(Spark_gfx_handles[0]));

	for (i=0;i<Number;i++)
	{
		COMOB = Add_COMOB();

		COMOB->dX_3D = (rand() % (4 * COMOB_DEC_FACTOR)) -
		 (2 * COMOB_DEC_FACTOR);
		COMOB->dY_3D = (rand() % (2 * COMOB_DEC_FACTOR)) + COMOB_DEC_FACTOR;
		COMOB->dZ_3D = (rand() % (4 * COMOB_DEC_FACTOR)) -
		 (2 * COMOB_DEC_FACTOR);

		COMOB->X_3D = Source_X + COMOB->dX_3D;
		COMOB->Y_3D = Source_Y + COMOB->dY_3D;
		COMOB->Z_3D = Source_Z + COMOB->dZ_3D;

		COMOB->Priority = 25 + (rand() % 50);
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

		COMOB->Special_handle = Transluminance_table_handle;

		COMOB->Lifespan = 70 + (rand() % 20);

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		COMOB->Display_width = 80 + (rand() % 20);
		COMOB->Display_height = COMOB->Display_width;

		COMOB->Graphics_handle = Spark_gfx_handles[rand() % 11];

		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);
		COMOB->Frame = rand() % COMOB->Nr_frames;

		Behaviour_data = Add_COMOB_behaviour(COMOB, NULL, Bounce_handler);

		Behaviour_data->Bounce_data.Gravity = (1 + (rand() % 10)) * 5;
		Behaviour_data->Bounce_data.Bounce = (rand() % 100);
		Behaviour_data->Bounce_data.Air_friction = 0;
	}

	for (i=0;i<100;i++)
	{
		Update_screen();
	}

	MEM_Free_memory(Ball_handle);
	for (i=0;i<11;i++)
	{
		MEM_Free_memory(Spark_gfx_handles[i]);
	}

	Fighting = FALSE;

	Update_screen();

	/* Add tactical window object */
	Tactical_window_object = Add_object(Earth_object, &Tactical_window_Class,
	 NULL, TACTICAL_X, TACTICAL_Y, TACTICAL_WIDTH, TACTICAL_HEIGHT);

	/* Draw */
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);
}




void
Do_fireball(struct Combat_participant *Part)
{
	MEM_HANDLE Firering_handle;
	MEM_HANDLE Fireball_handle;
	MEM_HANDLE Spark_handles[2];
	struct COMOB *Fireball_COMOB;
	struct COMOB *Firering_COMOB;
	struct COMOB *Trailed_COMOB;
	struct COMOB *COMOB;
	union COMOB_behaviour_data *Behaviour_data;
	SILONG dX_3D, dY_3D, dZ_3D;
	UNSHORT Nr_steps;
	UNSHORT i, j;

	/* The fight is on! */
	Fighting = TRUE;

	/* Delete interface objects */
	Delete_object(Tactical_window_object);

	/* Re-draw the screen */
	Update_screen();

	/* Load graphics */
	Firering_handle = Load_subfile(COMBAT_GFX, 20);
	Fireball_handle = Load_subfile(COMBAT_GFX, 21);
	Spark_handles[0] = Load_subfile(COMBAT_GFX, 10);
 	Spark_handles[1] = Load_subfile(COMBAT_GFX, 11);

	/* Make fireball COMOB */
	Fireball_COMOB = Add_COMOB();

	Nr_steps = Prepare_COMOB_movement(&(Party_parts[0]), Fireball_COMOB,
	 Part->Tactical_X, Part->Tactical_Y, 15);

	dX_3D = Fireball_COMOB->dX_3D;
	dY_3D = Fireball_COMOB->dY_3D;
	dZ_3D = Fireball_COMOB->dZ_3D;

	Fireball_COMOB->dX_3D = 0;
	Fireball_COMOB->dY_3D = 0;
	Fireball_COMOB->dZ_3D = 0;

	Fireball_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	Fireball_COMOB->Special_handle = Transluminance_table_handle;

	Fireball_COMOB->Hotspot_X_offset = 50;
	Fireball_COMOB->Hotspot_Y_offset = 50;

	Fireball_COMOB->Display_width = 40;
	Fireball_COMOB->Display_height = 40;

	Fireball_COMOB->Graphics_handle = Fireball_handle;

	Fireball_COMOB->Nr_frames = Get_nr_frames(Fireball_COMOB->Graphics_handle);

	/* Make firering COMOB */
	Firering_COMOB = Add_COMOB();

	Firering_COMOB->X_3D = Fireball_COMOB->X_3D;
	Firering_COMOB->Y_3D = Fireball_COMOB->Y_3D;
	Firering_COMOB->Z_3D = Fireball_COMOB->Z_3D;

	Firering_COMOB->Priority = 100;

	Firering_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	Firering_COMOB->Special_handle = Transluminance_table_handle;

	Firering_COMOB->Hotspot_X_offset = 50;
	Firering_COMOB->Hotspot_Y_offset = 50;

	Firering_COMOB->Display_width = 400;
	Firering_COMOB->Display_height = 400;

	Firering_COMOB->Graphics_handle = Firering_handle;

	Firering_COMOB->Nr_frames = Get_nr_frames(Firering_COMOB->Graphics_handle);

	for (i=0;i<15;i++)
	{
		Update_screen();

		Fireball_COMOB->Display_width += 4;
		Fireball_COMOB->Display_height += 4;
		Firering_COMOB->Display_width -= 20;
		Firering_COMOB->Display_height -= 20;
	}

	Delete_COMOB(Firering_COMOB);

	Fireball_COMOB->dX_3D = dX_3D;
	Fireball_COMOB->dY_3D = dY_3D;
	Fireball_COMOB->dZ_3D = dZ_3D;

	for (i=0;i<Nr_steps;i++)
	{
		Update_screen();
	}

	/* Make firering COMOB */
	Firering_COMOB = Add_COMOB();

	Firering_COMOB->X_3D = Fireball_COMOB->X_3D;
	Firering_COMOB->Y_3D = Fireball_COMOB->Y_3D;
	Firering_COMOB->Z_3D = Fireball_COMOB->Z_3D;

	Firering_COMOB->Priority = 100;

	Firering_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
	Firering_COMOB->Special_handle = Transluminance_table_handle;

	Firering_COMOB->Hotspot_X_offset = 50;
	Firering_COMOB->Hotspot_Y_offset = 50;

	Firering_COMOB->Display_width = 100;
	Firering_COMOB->Display_height = 100;

	Firering_COMOB->Graphics_handle = Firering_handle;

	Firering_COMOB->Nr_frames = Get_nr_frames(Firering_COMOB->Graphics_handle);

	Delete_COMOB(Fireball_COMOB);

	for (i=0;i<40;i++)
	{
		COMOB = Add_COMOB();

		COMOB->dX_3D = (rand() % (4 * COMOB_DEC_FACTOR)) - (2 * COMOB_DEC_FACTOR);
		COMOB->dY_3D = (rand() % (3 * COMOB_DEC_FACTOR)) - COMOB_DEC_FACTOR;
		COMOB->dZ_3D = (rand() % (4 * COMOB_DEC_FACTOR)) - (2 * COMOB_DEC_FACTOR);

		COMOB->X_3D = Firering_COMOB->X_3D + COMOB->dX_3D;
		COMOB->Y_3D = Firering_COMOB->Y_3D + COMOB->dY_3D;
		COMOB->Z_3D = Firering_COMOB->Z_3D + COMOB->dZ_3D;

		COMOB->Priority = 25 + (rand() % 50);
		COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;

		COMOB->Special_handle = Transluminance_table_handle;

		COMOB->Lifespan = 70 + (rand() % 20);

		COMOB->Hotspot_X_offset = 50;
		COMOB->Hotspot_Y_offset = 50;

		COMOB->Display_width = 80 + (rand() % 20);
		COMOB->Display_height = COMOB->Display_width;

		COMOB->Graphics_handle = Spark_handles[rand() % 2];

		COMOB->Nr_frames = Get_nr_frames(COMOB->Graphics_handle);
		COMOB->Frame = rand() % COMOB->Nr_frames;

		Behaviour_data = Add_COMOB_behaviour(COMOB, NULL, Bounce_handler);

		Behaviour_data->Bounce_data.Gravity = 10 + rand() % 5;
		Behaviour_data->Bounce_data.Bounce = 95;
		Behaviour_data->Bounce_data.Air_friction = 5;

		Add_trail_to_COMOB(COMOB, 5);
	}

	Set_combat_animation(Part, HIT_COMANIM);

/*	for (i=0;i<10;i++)
	{
		Update_screen();

		Firering_COMOB->Display_width += 50;
		Firering_COMOB->Display_height += 50;
	} */

	Delete_COMOB(Firering_COMOB);

	for (i=0;i<75;i++)
	{
		Update_screen();
	}

	/* Destroy graphics */
	MEM_Free_memory(Firering_handle);
	MEM_Free_memory(Fireball_handle);
	MEM_Free_memory(Spark_handles[0]);
	MEM_Free_memory(Spark_handles[1]);

	Fighting = FALSE;

	Update_screen();

	/* Add tactical window object */
	Tactical_window_object = Add_object(Earth_object, &Tactical_window_Class,
	 NULL, TACTICAL_X, TACTICAL_Y, TACTICAL_WIDTH, TACTICAL_HEIGHT);

	/* Draw */
	Execute_method(Tactical_window_object, DRAW_METHOD, NULL);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Draw_combat_grid
 * FUNCTION  : Draw combat grid.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.04.95 10:43
 * LAST      : 11.04.95 10:43
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Draw_combat_grid(void)
{
	SILONG X_3D, Y_3D, Z_3D;
	SILONG X_2D, Y_2D;
	SILONG Proj;
	SILONG X, Y;

	/* Draw horizon */
	OPM_HorLine(&Main_OPM, 0, COMBAT_Y + (COMBAT_HEIGHT / 2),
	 COMBAT_WIDTH, RED);

	Y_3D = 0 - Combat_camera_height;

	/* Draw lines along the Z-axis */
	for (Z_3D=5 * (SILONG)(Combat_square_depth / 2000);
	 Z_3D>=MINIMUM_Z;Z_3D-=10)
	{
		for (X=0;X<=NR_TACTICAL_COLUMNS;X++)
		{
			X_3D = ((X - 3) * 2) * (SILONG)(Combat_square_width /
			 COMOB_DEC_FACTOR);

			/* Calculate projection factor */
			Proj = Z_3D + Combat_projection_factor;
			if (!Proj)
				Proj = 1;

			/* Project coordinates */
			X_2D = (X_3D * Combat_projection_factor) / Proj;
			Y_2D = (Y_3D * Combat_projection_factor) / Proj;

			/* Translate coordinates to centre of the screen & flip Y-coordinate */
			X_2D += COMBAT_X + (COMBAT_WIDTH / 2);
			Y_2D = COMBAT_Y + (COMBAT_HEIGHT / 2) - Y_2D;

			OPM_SetPixel(&Main_OPM, X_2D, Y_2D, WHITE);
		}
	}

	/* Draw lines along the X-axis */
	for (Y=0;Y<=NR_TACTICAL_ROWS;Y++)
	{
		Z_3D = (((0 - (Y - 2)) * 2) + 1) * (SILONG)(Combat_square_depth /
		 (2 * COMOB_DEC_FACTOR));

		for (X_3D=(0 - 6) * (SILONG)(Combat_square_width / COMOB_DEC_FACTOR);
		 X_3D<=((NR_TACTICAL_COLUMNS - 3) * 2) * (SILONG)(Combat_square_width /
		  COMOB_DEC_FACTOR);
		 X_3D+=10)
		{
			/* Calculate projection factor */
			Proj = Z_3D + Combat_projection_factor;
			if (!Proj)
				Proj = 1;

			/* Project coordinates */
			X_2D = (X_3D * Combat_projection_factor) / Proj;
			Y_2D = (Y_3D * Combat_projection_factor) / Proj;

			/* Translate coordinates to centre of the screen & flip Y-coordinate */
			X_2D += COMBAT_X + (COMBAT_WIDTH / 2);
			Y_2D = COMBAT_Y + (COMBAT_HEIGHT / 2) - Y_2D;

			OPM_SetPixel(&Main_OPM, X_2D, Y_2D, WHITE);
		}
	}
}

