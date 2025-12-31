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
//#include <STATAREA.H>

/* global variables */

UNSHORT Combat_display_detail_level = DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL;

UNSHORT Combat_projection_factor = COMBAT_PROJ_FACTOR;
UNSHORT Combat_camera_height = COMBAT_CAMERA_HEIGHT;
SISHORT Combat_Z_offset = COMBAT_Z_OFFSET;

BOOLEAN Combat_grid_flag = FALSE;

MEM_HANDLE Star_gfx_handles[NR_COMBAT_STARS];
MEM_HANDLE Spark_gfx_handles[NR_COMBAT_SPARKS];

static UNSHORT Nr_COMOBs;
static UNSHORT New_nr_COMOBs;
static UNSHORT Nr_COMOB_behaviours;

static struct COMOB *COMOB_sort_list[MAX_COMOBS];

static MEM_HANDLE COMOB_table_handle;
static struct COMOB *COMOB_table_ptr;

static struct COMOB_behaviour COMOB_behaviour_list[MAX_COMOB_BEHAVIOURS];

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
	static UNSHORT Star_gfx_batch[NR_COMBAT_STARS] = {
		1, 5, 8
	};
	static UNSHORT Spark_gfx_batch[NR_COMBAT_SPARKS] = {
		2, 3, 4, 6, 7, 9, 10, 11
	};
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

	/* Allocate memory for COMOB table */
	COMOB_table_handle = MEM_Allocate_memory(MAX_COMOBS *
	 sizeof(struct COMOB));

	/* Clear COMOB table */
	MEM_Clear_memory(COMOB_table_handle);

	/* Get pointer */
	COMOB_table_ptr = (struct COMOB *) MEM_Claim_pointer(COMOB_table_handle);

	/* Clear COMOB behaviour list */
	BASEMEM_FillMemByte((UNBYTE *) &COMOB_behaviour_list[0],
	 MAX_COMOB_BEHAVIOURS * sizeof(struct COMOB_behaviour), 0);

	/* Load stars */
	Load_full_batch(COMBAT_GFX, NR_COMBAT_STARS, &(Star_gfx_batch[0]),
	 &(Star_gfx_handles[0]));

	/* Load sparks */
	Load_full_batch(COMBAT_GFX, NR_COMBAT_SPARKS, &(Spark_gfx_batch[0]),
	 &(Spark_gfx_handles[0]));
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_COMOB_system
 * FUNCTION  : Exit COMbat OBject system.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.95 16:38
 * LAST      : 12.06.95 16:38
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_COMOB_system(void)
{
	UNSHORT i;

	/* Free sparks */
	for (i=0;i<NR_COMBAT_SPARKS;i++)
	{
		MEM_Free_memory(Spark_gfx_handles[i]);
	}

	/* Free stars */
	for (i=0;i<NR_COMBAT_STARS;i++)
	{
		MEM_Free_memory(Star_gfx_handles[i]);
	}


	/* Free COMOB table */
	MEM_Free_pointer(COMOB_table_handle);
	MEM_Free_memory(COMOB_table_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Add_COMOB
 * FUNCTION  : Add a COMOB to the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:37
 * LAST      : 16.06.95 11:30
 * INPUTS    : UNSHORT Priority - Priority of COMOB (0...100).
 * RESULT    : struct COMOB *COMOB : Pointer to new COMOB / NULL = no
 *              more room.
 * BUGS      : No known.
 * NOTES     : - The COMOB data *must* be initialized before the COMOBs
 *              are drawn.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB *
Add_COMOB(UNSHORT Priority)
{
	struct COMOB *Output = NULL;
	UNSHORT i;

	/* Is there any room ? */
	if (New_nr_COMOBs < MAX_COMOBS)
	{
		/* Yes -> Find free slot in table */
		for (i=0;i<MAX_COMOBS;i++)
		{
			/* Free ? */
			if (!((COMOB_table_ptr + i)->System_flags & COMOB_PRESENT))
			{
				/* Yes */
				Output = COMOB_table_ptr + i;

				/* Clear COMOB data */
				BASEMEM_FillMemByte((UNBYTE *) Output, sizeof(struct COMOB), 0);

				/* Indicate this COMOB is present */
				Output->System_flags |= COMOB_PRESENT;
				Output->System_flags &= ~COMOB_ACTIVE;

				/* Set priority */
				Output->Priority = Priority;

				/* Add to sort list */
				COMOB_sort_list[New_nr_COMOBs] = Output;

				/* Count up */
				New_nr_COMOBs++;

				break;
			}
		}
	}

	/* Found anything ? */
	if (!Output)
	{
		/* No -> Report error */
		Error(ERROR_OUT_OF_COMOBS);
	}

	return Output;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_COMOB
 * FUNCTION  : Remove a COMOB from the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.03.95 13:38
 * LAST      : 11.06.95 19:27
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
	UNSHORT i;

	/* Exit if NULL */
	if (!COMOB)
		return;

	/* Delete */
	COMOB->System_flags &= ~(COMOB_PRESENT | COMOB_ACTIVE);

	/* Free graphics ? */
	if (COMOB->User_flags & COMOB_FREE_GFX_ON_DELETE)
	{
		/* Yes -> Any graphics loaded ? */
		if (COMOB->Graphics_handle)
		{
			/* Yes -> Free it */
			MEM_Free_memory(COMOB->Graphics_handle);

			/* Clear handle */
			COMOB->Graphics_handle = NULL;
		}
	}

	/* Delete behaviours referring to this COMOB */
	for (i=0;i<MAX_COMOB_BEHAVIOURS;i++)
	{
		/* Does this behaviour refer to the COMOB that has been deleted ? */
		if (COMOB_behaviour_list[i].First_COMOB == COMOB)
		{
			/* Yes -> Delete the behaviour */
			Delete_COMOB_behaviour(&COMOB_behaviour_list[i]);
		}
		else
		{
			/* No -> Does this behaviour refer to the COMOB that has been
			 deleted ? */
			if (COMOB_behaviour_list[i].Second_COMOB == COMOB)
			{
				/* Yes -> Get pointer to other COMOB */
				Other_COMOB = COMOB_behaviour_list[i].First_COMOB;

				/* Delete the behaviour */
				Delete_COMOB_behaviour(&COMOB_behaviour_list[i]);

				/* Delete the other COMOB as well */
				Delete_COMOB(Other_COMOB);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      :	Duplicate_COMOB
 * FUNCTION  : Duplicate a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.06.95 14:51
 * LAST      : 20.06.95 14:51
 * INPUTS    : struct COMOB *Source_COMOB - Pointer to COMOB that must be
 *              duplicated.
 * RESULT    : struct COMOB *COMOB : Pointer to new COMOB / NULL = no
 *              more room.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB *
Duplicate_COMOB(struct COMOB *Source_COMOB)
{
	struct COMOB *Duplicate;

	/* Legal input ? */
	if (!Source_COMOB)
	{
		/* No -> Return NULL */
		return NULL;
	}

	/* Create a new COMOB */
	Duplicate = Add_COMOB(Source_COMOB->Priority);

	/* Success ? */
	if (Duplicate)
	{
		/* Yes -> Duplicate data */
		memcpy((UNBYTE *) Duplicate, (UNBYTE *) Source_COMOB,
		 sizeof(struct COMOB));

		/* Adjust flags */
		Duplicate->System_flags |= COMOB_PRESENT;
		Duplicate->System_flags &= ~COMOB_ACTIVE;
	}

	return Duplicate;
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
 * LAST      : 20.06.95 19:28
 * INPUTS    : struct COMOB *First_COMOB - Pointer to behaving COMOB.
 *             struct COMOB *Second_COMOB - Pointer to attached COMOB.
 *             COMOB_behaviour_handler Handler - Pointer to handler function.
 * RESULT    : struct COMOB_behaviour_data * : Pointer to behaviour data /
 *              NULL = no more room.
 * BUGS      : No known.
 * NOTES     : - The behaviour will be attached to the first COMOB. The
 *              behaviour may require a pointer to a second attached COMOB.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

struct COMOB_behaviour*
Add_COMOB_behaviour(struct COMOB *First_COMOB, struct COMOB *Second_COMOB,
 COMOB_behaviour_handler Handler)
{
	struct COMOB_behaviour *Behaviour;
	UNSHORT i;

	/* Is the first COMOB NULL ? */
	if (!First_COMOB)
	{
		/* Yes -> Report error */
		Error(ERROR_NULL_COMOB_PASSED);

		/* Exit */
		return NULL;
	}

	/* Is the COMOB behaviour list full ? */
	if (Nr_COMOB_behaviours >= MAX_COMOB_BEHAVIOURS)
	{
		/* Yes -> Report error */
		Error(ERROR_OUT_OF_COMOB_BEHAVIOURS);

		/* Exit */
		return NULL;
	}

	/* Search for a free behaviour */
	for (i=0;i<MAX_COMOB_BEHAVIOURS;i++)
	{
		/* Free ? */
		if (COMOB_behaviour_list[i].First_COMOB == NULL)
		{
			/* Yes */
			Behaviour = &(COMOB_behaviour_list[i]);

			/* Count up */
			Nr_COMOB_behaviours++;

			/* Clear behaviour data */
			BASEMEM_FillMemByte((UNBYTE *) &(Behaviour->Data),
			 sizeof(union COMOB_behaviour_data), 0);

			/* Initialize behaviour */
			Behaviour->First_COMOB = First_COMOB;
			Behaviour->Second_COMOB = Second_COMOB;
			Behaviour->Handler = Handler;

			/* Return pointer to behaviour data */
			return Behaviour;
		}
	}

	return NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Delete_COMOB_behaviour
 * FUNCTION  : Remove a COMOB behaviour from the current list.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.03.95 11:08
 * LAST      : 10.06.95 17:35
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to COMOB behaviour
 *              that must be removed.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Delete_COMOB_behaviour(struct COMOB_behaviour *Behaviour)
{
	/* Exit if NULL or behaviour has already been deleted */
	if ((Behaviour == NULL) || (Behaviour->First_COMOB == NULL))
		return;

	/* Delete the behaviour */
	Behaviour->First_COMOB = NULL;

	/* Count down */
	Nr_COMOB_behaviours--;
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

/*
	OPM_FillBox(&Status_area_OPM, 181, 3, 100, 44, BLACK);

	xprintf(&Status_area_OPM, 182, 4, "COMOBs : %u", Nr_COMOBs);
	xprintf(&Status_area_OPM, 182, 14, "Behaviours : %u", Nr_COMOB_behaviours);
*/
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
		}
	}

	/* Handle all behaviours */
	for (i=0;i<MAX_COMOB_BEHAVIOURS;i++)
	{
		/* Present ? */
		if (COMOB_behaviour_list[i].First_COMOB)
		{
			/* Yes -> Is the first COMOB active ? */
			if (COMOB_behaviour_list[i].First_COMOB->System_flags &
			 COMOB_ACTIVE)
			{
				/* Yes -> Is there a second COMOB ? */
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

	/* Priority high enough / not hidden ? */
	if ((COMOB->Priority >= (MAX_COMBAT_DISPLAY_DETAIL_LEVEL -
	 Combat_display_detail_level)) && !(COMOB->System_flags & COMOB_HIDDEN))
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
 * NAME      : Get_COMOB_source_size
 * FUNCTION  : Get size of COMOB source graphics.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 17.06.95 13:22
 * LAST      : 17.06.95 13:22
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB data.
 *             UNSHORT *Width_ptr - Pointer to width.
 *             UNSHORT *Height_ptr - Pointer to height.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_COMOB_source_size(struct COMOB *COMOB, UNSHORT *Width_ptr,
 UNSHORT *Height_ptr)
{
	struct Gfx_header *Gfx;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Find current frame */
	Gfx = (struct Gfx_header *) (MEM_Claim_pointer(COMOB->Graphics_handle) +
	 COMOB->Graphics_offset);

	for (i=0;i<COMOB->Frame;i++)
	{
		Ptr = (UNBYTE *) (Gfx + 1);
		Gfx = (struct Gfx_header *) (Ptr + (Gfx->Width * Gfx->Height));
	}
	Ptr = (UNBYTE *) (Gfx + 1);

	/* Store source dimensions */
	*Width_ptr = Gfx->Width;
	*Height_ptr = Gfx->Height;

	MEM_Free_pointer(COMOB->Graphics_handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_COMOB_rectangle
 * FUNCTION  : Get the 3D rectangle that covers a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 15:16
 * LAST      : 21.06.95 15:16
 * INPUTS    : struct COMOB *COMOB - Pointer to COMOB data.
 *             SILONG *Left_3D_ptr - Pointer to left 3D X-coordinate.
 *             SILONG *Top_3D_ptr - Pointer to top 3D Y-coordinate.
 *             SILONG *Right_3D_ptr - Pointer to right 3D X-coordinate.
 *             SILONG *Bottom_3D_ptr - Pointer to bottom 3D Y-coordinate.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The coordinates are multiplied with COMOB_DEC_FACTOR.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_COMOB_rectangle(struct COMOB *COMOB, SILONG *Left_3D_ptr,
 SILONG *Top_3D_ptr, SILONG *Right_3D_ptr, SILONG *Bottom_3D_ptr)
{
	SILONG Left_3D, Top_3D, Right_3D, Bottom_3D;
	UNSHORT Source_width, Source_height;
	UNSHORT Target_width, Target_height;

	/* Get COMOB coordinates */
	Left_3D = COMOB->X_3D;
	Top_3D = COMOB->Y_3D;

	/* Get source dimensions of COMOB */
	Get_COMOB_source_size(COMOB, &Source_width, &Source_height);

	/* Calculate target dimensions of COMOB */
	Target_width = (Source_width * COMOB->Display_width) / 100;
	Target_height = (Source_height * COMOB->Display_height) / 100;

	/* Make adjustments for hot-spot */
	Left_3D -= ((Target_width * COMOB->Hotspot_X_offset) / 100) *
	 COMOB_DEC_FACTOR;
	Top_3D += ((Target_height * COMOB->Hotspot_Y_offset) / 100) *
	 COMOB_DEC_FACTOR;

	/* Store coordinates */
	*Left_3D_ptr = Left_3D;
	*Top_3D_ptr = Top_3D;
	*Right_3D_ptr = Left_3D + (Target_width * COMOB_DEC_FACTOR);
	*Bottom_3D_ptr = Top_3D - (Target_height * COMOB_DEC_FACTOR);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Animate_all_COMOBs
 * FUNCTION  : Animate all COMOBs.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 11:30
 * LAST      : 21.06.95 11:30
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Animate_all_COMOBs(void)
{
	struct COMOB *COMOB;
	UNSHORT i;

	for (i=0;i<Nr_COMOBs;i++)
	{
		/* Get COMOB data */
		COMOB = COMOB_sort_list[i];

		/* Active ? */
		if (COMOB->System_flags & COMOB_ACTIVE)
		{
			/* Yes -> Animated ? */
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
		case OSCILLATE_SIZE:
		{
			COMOB->Display_width += (SISHORT) Total_delta;
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
 * NAME      : Fairy_handler
 * FUNCTION  : Fairy COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.95 16:28
 * LAST      : 12.06.95 16:28
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Fairy_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Fairy_COMOB;
	struct COMOB *Dust_COMOB;
	UNSHORT i;

	/* Get pointer to fairy COMOB */
	Fairy_COMOB = Behaviour->First_COMOB;

	/* Generate dust particles */
	for (i=0;i<2;i++)
	{
		/* Create COMOB */
		Dust_COMOB = Add_COMOB(max(Fairy_COMOB->Priority - 10, 0));
		if (!Dust_COMOB)
			break;

		/* Add dust behaviour */
		Behaviour_data = Add_COMOB_behaviour(Dust_COMOB, NULL, Dust_handler);
		if (!Behaviour_data)
		{
			Delete_COMOB(Dust_COMOB);
			break;
		}

		/* Set dust parameters */
		Behaviour_data->Data.Dust_data.Type = DUST_LEAD;

		/* Copy coordinates from fairy COMOB */
		Dust_COMOB->X_3D = Fairy_COMOB->X_3D + (rand() % (10 *
		 COMOB_DEC_FACTOR)) - (5 * COMOB_DEC_FACTOR);
		Dust_COMOB->Y_3D = 100 * COMOB_DEC_FACTOR; //Fairy_COMOB->Y_3D;
		Dust_COMOB->Z_3D = Fairy_COMOB->Z_3D + (rand() % (10 *
		 COMOB_DEC_FACTOR)) - (5 * COMOB_DEC_FACTOR);

		/* Set drop speed */
		Dust_COMOB->dY_3D = 0 - ((rand() % (3 * COMOB_DEC_FACTOR)) +
		 COMOB_DEC_FACTOR);

		/* Set lifespan */
		Dust_COMOB->Lifespan = 40 + (rand() % 20);

		/* Set draw mode */
		Dust_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
		Dust_COMOB->Special_handle = Transluminance_table_handle;
		Dust_COMOB->Special_offset = 0;

		/* Set hotspot offsets */
		Dust_COMOB->Hotspot_X_offset = 50;
		Dust_COMOB->Hotspot_Y_offset = 50;

		/* Set display dimensions */
		Dust_COMOB->Display_width = 100;
		Dust_COMOB->Display_height = 100;

		/* Set graphics handle and offset */
		Dust_COMOB->Graphics_handle =
		 Spark_gfx_handles[0];
//		 Spark_gfx_handles[rand() % NR_COMBAT_SPARKS];
		Dust_COMOB->Graphics_offset = 0;

		/* Set number of animation frames */
		Dust_COMOB->Nr_frames = Get_nr_frames(Dust_COMOB->Graphics_handle);

		/* Set initial animation frame */
		Dust_COMOB->Frame = rand() % Dust_COMOB->Nr_frames;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Dust_handler
 * FUNCTION  : Fairy dust COMOB behaviour handler.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 12.06.95 16:30
 * LAST      : 12.06.95 16:30
 * INPUTS    : struct COMOB_behaviour *Behaviour - Pointer to behaviour data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Dust_handler(struct COMOB_behaviour *Behaviour)
{
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Dust_COMOB;
	struct COMOB *New_COMOB;

	/* Get pointer to dust COMOB */
	Dust_COMOB = Behaviour->First_COMOB;

	/* Reached the floor ? */
	if (Dust_COMOB->Y_3D <= 0)
	{
		/* Yes -> Kill */
		Delete_COMOB(Dust_COMOB);
		return;
	}

	/* Is this a big dust particle ? */
	if (Behaviour->Data.Dust_data.Type == DUST_LEAD)
	{
		/* Yes -> Generate new dust particle ? */
		if (!(rand() & 0x0003))
		{
			/* Yes -> Create COMOB */
			New_COMOB = Add_COMOB(Dust_COMOB->Priority);
			if (New_COMOB)
			{
				/* Tiny or sparkle particle ? */
//				if (rand() & 0x001f)
				{
					/* Tiny -> Copy coordinates from lead dust COMOB */
					New_COMOB->X_3D = Dust_COMOB->X_3D + (rand() % (10 *
					 COMOB_DEC_FACTOR)) - (5 * COMOB_DEC_FACTOR);
					New_COMOB->Y_3D = Dust_COMOB->Y_3D;
					New_COMOB->Z_3D = Dust_COMOB->Z_3D + (rand() % (10 *
					 COMOB_DEC_FACTOR)) - (5 * COMOB_DEC_FACTOR);

					/* Copy drop speed, but slower */
					New_COMOB->dY_3D = Dust_COMOB->dY_3D;
/*					New_COMOB->dY_3D = max(Dust_COMOB->dY_3D + COMOB_DEC_FACTOR,
					 (0 - (COMOB_DEC_FACTOR / 2))); */

					/* Set lifespan */
					New_COMOB->Lifespan = 20 + (rand() % 10);

					/* Set draw mode */
					New_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
					New_COMOB->Special_handle = Transluminance_table_handle;
					New_COMOB->Special_offset = 0;

					/* Set hotspot offsets */
					New_COMOB->Hotspot_X_offset = 50;
					New_COMOB->Hotspot_Y_offset = 50;

					/* Set display dimensions */
					New_COMOB->Display_width = 50;
					New_COMOB->Display_height = 50;

					/* Set graphics handle and offset */
					New_COMOB->Graphics_handle =
					 Spark_gfx_handles[NR_COMBAT_SPARKS - 1];
//					 Spark_gfx_handles[rand() % NR_COMBAT_SPARKS];
					New_COMOB->Graphics_offset = 0;

					/* Set number of animation frames */
					New_COMOB->Nr_frames = Get_nr_frames(New_COMOB->Graphics_handle);

					/* Set initial animation frame */
					New_COMOB->Frame = rand() % New_COMOB->Nr_frames;

					/* Add dust behaviour */
					Behaviour_data = Add_COMOB_behaviour(New_COMOB, NULL,
					 Dust_handler);
					if (!Behaviour_data)
					{
						Delete_COMOB(New_COMOB);
					}
					else
					{
						Behaviour_data->Data.Dust_data.Type = DUST_TINY;
					}
				}
/*				else
				{ */
					/* Sparkle -> */
//				}
			}
		}
	}
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
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Trailing_COMOB;
	UNSHORT Shrink_percentage;
	UNSHORT i;

	/* Trailed COMOB is NULL / number of COMOBs in the trail is zero ? */
	if (!Trailed_COMOB || !Nr_trailing_COMOBs)
	{
		/* Yes -> Exit */
		return;
	}

	/* Calculate shrink percentage */
	Shrink_percentage = 100 - (100 / (Nr_trailing_COMOBs + 1));

	/* Create trailing COMOBs */
	for (i=0;i<Nr_trailing_COMOBs;i++)
	{
		/* Create trailing COMOB */
		Trailing_COMOB = Duplicate_COMOB(Trailed_COMOB);
		if (!Trailing_COMOB)
			break;

		/* Add trailing behaviour */
		Behaviour_data = Add_COMOB_behaviour(Trailing_COMOB, Trailed_COMOB,
		 Trail_handler);
		if (!Behaviour_data)
		{
			Delete_COMOB(Trailing_COMOB);
			break;
		}

		/* Set trail parameters */
		Behaviour_data->Data.Trail_data.Minimum_distance = 10 * COMOB_DEC_FACTOR;
		Behaviour_data->Data.Trail_data.Maximum_distance = 20 * COMOB_DEC_FACTOR;

		if (Trailing_COMOB->Priority)
			Trailing_COMOB->Priority -= 1;

		/* Copy display dimensions, but smaller */
		Trailing_COMOB->Display_width = (Trailing_COMOB->Display_width *
		 Shrink_percentage) / 100;
		Trailing_COMOB->Display_height = (Trailing_COMOB->Display_height *
		 Shrink_percentage) / 100;

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
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Shadow_COMOB;

	/* Create shadow COMOB */
	Shadow_COMOB = Add_COMOB(90);
	if (!Shadow_COMOB)
		return;

	/* Add shadow behaviour */
	Behaviour_data = Add_COMOB_behaviour(Shadow_COMOB, Casting_COMOB,
	 Shadow_handler);
	if (!Behaviour_data)
	{
		Delete_COMOB(Shadow_COMOB);
		return;
	}

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
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_halo_to_COMOB
 * FUNCTION  : Add a halo to a COMOB.
 * FILE      : COMOBS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.06.95 13:50
 * LAST      : 21.06.95 13:50
 * INPUTS    : struct COMOB *Source_COMOB - Pointer to source COMOB.
 *             UNSHORT Nr_halo_COMOBs - Number of COMOBs in halo.
 *             UNSHORT Halo_width - Width of halo (in %).
 *             UNSHORT Halo_lifespan - Lifespan of halo.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_halo_to_COMOB(struct COMOB *Source_COMOB, UNSHORT Nr_halo_COMOBs,
 UNSHORT Halo_width, UNSHORT Halo_lifespan)
{
	struct COMOB_behaviour *Behaviour_data;
	struct COMOB *Halo_COMOB;
	SILONG Left_3D, Top_3D, Right_3D, Bottom_3D;
	UNSHORT Delta_width, Delta_height;
	UNSHORT i;

	/* Source COMOB is NULL / number of COMOBs in the halo is zero /
	 halo width is zero ? */
	if (!Source_COMOB || !Nr_halo_COMOBs || !Halo_width)
	{
		/* Yes -> Exit */
		return;
	}

	/* Calculate width and height deltas */
	Delta_width = ((Source_COMOB->Display_width * Halo_width) / 100) /
	 Nr_halo_COMOBs;
	Delta_height = ((Source_COMOB->Display_height * Halo_width) / 100) /
	 Nr_halo_COMOBs;

	for (i=0;i<Nr_halo_COMOBs;i++)
	{
		/* Duplicate source COMOB */
		Halo_COMOB = Duplicate_COMOB(Source_COMOB);
		if (!Halo_COMOB)
			return;

		/* Add behaviour */
		Behaviour_data = Add_COMOB_behaviour(Halo_COMOB, Source_COMOB,
		 Oscillate_handler);
		if (!Behaviour_data)
		{
			Delete_COMOB(Halo_COMOB);
			break;
		}

		/* Set oscillate data */
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_WIDTH;
		Behaviour_data->Data.Oscillate_data.Period = Halo_lifespan;
		Behaviour_data->Data.Oscillate_data.Amplitude = 40;

		/* Add behaviour */
		Behaviour_data = Add_COMOB_behaviour(Halo_COMOB, Source_COMOB,
		 Oscillate_handler);
		if (!Behaviour_data)
		{
			Delete_COMOB(Halo_COMOB);
			break;
		}

		/* Set oscillate data */
		Behaviour_data->Data.Oscillate_data.Type = OSCILLATE_HEIGHT;
		Behaviour_data->Data.Oscillate_data.Period = Halo_lifespan;
		Behaviour_data->Data.Oscillate_data.Amplitude = 40;

		/* Set halo priority */
		if (Halo_COMOB->Priority)
			Halo_COMOB->Priority -= 1;

		/* Copy display dimensions, but bigger */
		Halo_COMOB->Display_width = Source_COMOB->Display_width +
		 Delta_width;
		Halo_COMOB->Display_height = Source_COMOB->Display_height +
		 Delta_height;

		/* Copy Z-coordinate, but "behinder" */
		Halo_COMOB->Z_3D = Source_COMOB->Z_3D + 1;

		/* First halo COMOB ? */
		if (!i)
		{
			/* Yes -> Set halo COMOB draw mode */
			Halo_COMOB->Draw_mode = TRANSPARENT_COMOB_DRAWMODE;
			Halo_COMOB->Special_handle = Transluminance_table_handle;
			Halo_COMOB->Special_offset = 0;

			/* Set halo lifespan */
			Halo_COMOB->Lifespan = Halo_lifespan;

			/* Get rectangle covering source COMOB */
			Get_COMOB_rectangle(Source_COMOB, &Left_3D, &Top_3D, &Right_3D,
			 &Bottom_3D);

			/* Set halo COMOB coordinates in the middle of this rectangle */
			Halo_COMOB->X_3D = Left_3D + (Right_3D - Left_3D) / 2;
			Halo_COMOB->Y_3D = Bottom_3D + (Top_3D - Bottom_3D) / 2;

			/* Set hotspot */
			Halo_COMOB->Hotspot_X_offset = 50;
			Halo_COMOB->Hotspot_Y_offset = 50;
		}

		/* Next halo COMOB */
		Source_COMOB = Halo_COMOB;
	}
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

