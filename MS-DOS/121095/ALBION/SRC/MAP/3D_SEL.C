/************
 * NAME     : 3D_SEL.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 3-10-1995
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <BBDEF.H>
#include <BBDSA.H>
#include <BBERROR.H>
#include <BBOPM.H>
#include <BBEVENT.H>
#include <BBBASMEM.H>
#include <BBMEM.H>

#include <ALBION.H>

#include <XLOAD.H>
#include <GFXFUNC.H>

#include <CONTROL.H>
#include <MAP.H>
#include <MAP_PUM.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <PRTLOGIC.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <3DM.H>
#include <XFTYPES.H>
#include <COLOURS.H>
#include <GRAPHICS.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <EVELOGIC.H>
#include <3D_SEL.H>

/* prototypes */

/* 3D map select mode */
void Select_M3_ModInit(void);
void Select_M3_ModExit(void);
void Select_M3_MainLoop(void);

/* 3D map select support functions */
void Interpret_3D_identification_data(void);

UNLONG Get_distance_to_selected_map_square(void);

BOOLEAN Check_line_to_selected_map_square(void);

/* global variables */

/* 3D map select mode module */
struct Module Select_M3_Mod = {
	LOCAL_MOD | NO_INPUT_HANDLING, MODE_MOD, NO_SCREEN,
	Select_M3_MainLoop,
	Select_M3_ModInit,
	Select_M3_ModExit,
	NULL,
	NULL,
	Update_map_display
};

/* 3D identification data */
static struct Identify_3D Identify_data;

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M3_ModInit
 * FUNCTION  : Initialize 3D map select mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 16:09
 * LAST      : 20.07.95 16:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M3_ModInit(void)
{
	struct BBRECT MA;

	/* Create mouse area */
	MA.left = Window_3D_X;
	MA.top = Window_3D_Y;
	MA.width = I3DM.Window_3D_width;
	MA.height = I3DM.Window_3D_height;

	/* Install mouse area */
	Push_MA(&MA);

	/* Fill identify data */
	Identify_data.testColor = 193;
	Identify_data.screenX = Mouse_X - Window_3D_X;
	Identify_data.screenY = Mouse_Y - Window_3D_Y;

	/* Call identification function */
	Identify_3DM(&Identify_data);

	/* "Render" the 3D map */
	Render_3D_map();

	/* Interpret identification data */
	Interpret_3D_identification_data();

	/* Any action possible? */
	if (Current_map_selection_data.State != NO_MAP_SELECTION)
	{
		/* Yes -> Show */
		Push_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));
	}
	else
	{
		/* No -> Show */
		Push_mouse(&(Mouse_pointers[NO_ACTION_3D_MPTR]));
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M3_ModExit
 * FUNCTION  : Exit 3D map select mode module.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 16:09
 * LAST      : 20.07.95 16:09
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M3_ModExit(void)
{
	/* Remove mouse */
	Pop_mouse();

	/* Remove mouse area */
	Pop_MA();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Select_M3_MainLoop
 * FUNCTION  : 3D map selection mode main loop.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 14:05
 * LAST      : 20.07.95 20:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Select_M3_MainLoop(void)
{
	/* Fill identify data */
	Identify_data.testColor = 193;
	Identify_data.screenX = Mouse_X - Window_3D_X;
	Identify_data.screenY = Mouse_Y - Window_3D_Y;

	/* Call identification function */
	Identify_3DM(&Identify_data);

	/* Update display (thus rendering the 3D map) */
	Update_display();

	/* Update input handling */
	Update_input();

	/* Is the right mouse button pressed ? */
	if (Button_state & 0x0010)
	{
		/* Yes -> Interpret identification data */
		Interpret_3D_identification_data();

		/* Act depending on selection state */
		switch (Current_map_selection_data.State)
		{
			case NO_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[NO_ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD, NULL);

				break;
			}
			case A_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD, NULL);

				break;
			}
			case OUT_OF_RANGE_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[532]);

				break;
			}
			case OUT_OF_SPEAK_RANGE_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[533]);

				break;
			}
			case OUT_OF_TOUCH_RANGE_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[534]);

				break;
			}
			case BLOCKED_MAP_SELECTION:
			{
				/* Show */
				Change_mouse(&(Mouse_pointers[ACTION_3D_MPTR]));

				Execute_method(Help_line_object, SET_METHOD,
				 (void *) System_text_ptrs[535]);

				break;
			}
		}
	}
	else
	{
		/* No -> Exit select mode */
		Pop_module();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Interpret_3D_identifiation_data
 * FUNCTION  : Interpret 3D identification data.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 20.07.95 20:30
 * LAST      : 10.08.95 17:10
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The result of the interpretation can be found in
 *              Current_map_selection_data.State.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Interpret_3D_identification_data(void)
{
	UNLONG Distance = 0;
	UNSHORT State = NO_MAP_SELECTION;
	UNSHORT NPC_index = 0xFFFF;
	UNSHORT Trigger_modes = 0;

	/* Store found coordinates */
	Current_map_selection_data.Map_X = Identify_data.mapX;
	Current_map_selection_data.Map_Y = Identify_data.mapY;

	/* Act depending on found type */
	switch (Identify_data.type)
	{
		/* Nothing */
		case T_NOTHING:
		{
			break;
		}
		/* Object, wall, floor or ceiling */
		case T_OBJECT:
		case T_WALL:
		case T_SKY:
		case T_FLOOR:
		{
			/* Get trigger modes for this position */
			Trigger_modes = Get_event_triggers(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF);

			break;
		}
		/* NPC */
		case T_NPC:
		{
			/* Get NPC index */
			/* (+ correction for 3DM bug) */
			NPC_index = (UNSHORT) Identify_data.id_nr - 1;

			/* Get NPC coordinates */
			/* (another 3DM bug) */
			Current_map_selection_data.Map_X = VNPCs[NPC_index].Map_X;
			Current_map_selection_data.Map_Y = VNPCs[NPC_index].Map_Y;

			/* Has event ? */
			if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
			{
				/* Yes -> Get triggermodes */
				Trigger_modes = VNPCs[NPC_index].Trigger_modes;
			}
			else
			{
				/* No -> Can this NPC be talked to ? */
				if ((VNPCs[NPC_index].NPC_type != MONSTER_NPC) &&
				 (VNPCs[NPC_index].NPC_type != OBJECT_NPC))
				{
					/* Yes -> Use standard "triggermodes" */
					Trigger_modes = (1 << SPEAK_TRIGGER);
				}
				else
				{
					/* No -> Ignore this NPC */
					NPC_index = 0xFFFF;
				}
			}
			break;
		}
	}

	/* Determine distance between party and selected map square */
	Distance = Get_distance_to_selected_map_square();

	/* Is examining possible / is this too far away for examining ? */
	if (Trigger_modes & (1 << EXAMINE_TRIGGER))
	{
		if (Distance > MAX_EXAMINE_DISTANCE)
		{
			/* Yes -> Set state */
			State = OUT_OF_RANGE_MAP_SELECTION;
		}
		else
		{
			/* No */
			State = A_MAP_SELECTION;
		}
	}

	if (State != OUT_OF_RANGE_MAP_SELECTION)
	{
		/* Is talking possible ? */
		if (Trigger_modes & (1 << SPEAK_TRIGGER))
		{
			/* Yes -> Is this too far away for talking ? */
			if (Distance > MAX_TALK_DISTANCE)
			{
				/* Yes -> Set state */
				State = OUT_OF_SPEAK_RANGE_MAP_SELECTION;
			}
			else
			{
				/* No */
				State = A_MAP_SELECTION;
			}
		}
	}

	if ((State != OUT_OF_RANGE_MAP_SELECTION) &&
	 (State != OUT_OF_SPEAK_RANGE_MAP_SELECTION))
	{
		/* Is manipulating possible ? */
		if (Trigger_modes & ((1 << TOUCH_TRIGGER) | (1 << USE_ITEM_TRIGGER) |
		 (1 << TAKE_TRIGGER)))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if (Distance > MAX_MANIPULATE_DISTANCE)
			{
				/* Yes -> Set state */
				State = OUT_OF_TOUCH_RANGE_MAP_SELECTION;
			}
			else
			{
				/* No -> Check the line between the party and the selected map
					square */
				if (Check_line_to_selected_map_square())
				{
					/* Blocked */
					State = BLOCKED_MAP_SELECTION;
				}
				else
				{
					/* Not blocked */
					State = A_MAP_SELECTION;
				}
			}
		}
	}

	/* Store selection data */
	Current_map_selection_data.State				= State;
	Current_map_selection_data.NPC_index		= NPC_index;
	Current_map_selection_data.Trigger_modes	= Trigger_modes;
	Current_map_selection_data.Distance			= Distance;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_distance_to_selected_map_square
 * FUNCTION  : Calculate_distance between party and selected map square.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.07.95 14:29
 * LAST      : 21.07.95 14:29
 * INPUTS    : None.
 * RESULT    : UNLONG : Distance to selected map square in 3D units.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_distance_to_selected_map_square(void)
{
	UNLONG Distance;
	SISHORT dX, dY;

	/* Determine distance between party and selected map square */
	dX = Current_map_selection_data.Map_X - PARTY_DATA.X;
	dY = Current_map_selection_data.Map_Y - PARTY_DATA.Y;

	Distance = (dX * dX) + (dY * dY);
	Distance = (UNLONG) sqrt((double) Distance);

	Distance *= Map_square_size;

	return Distance;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Check_line_to_selected_map_square
 * FUNCTION  : Check the line between party and selected map square.
 * FILE      : 3D_MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.07.95 15:21
 * LAST      : 22.07.95 15:21
 * INPUTS    : None.
 * RESULT    : BOOLEAN : The line is blocked.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Check_line_to_selected_map_square(void)
{
	BOOLEAN Result = FALSE;
	UNLONG Status;
	SISHORT dX, dY;
	SISHORT Sign_dX, Sign_dY;
	UNSHORT X, Y;
	UNSHORT Var;
	UNSHORT i;

	/* Get selected map square coordinates */
	X = Current_map_selection_data.Map_X;
	Y = Current_map_selection_data.Map_Y;

	/* Calculate vector */
	dX = PARTY_DATA.X - X;
	dY = PARTY_DATA.Y - Y;

	/* Same location as party ? */
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
			Status = Get_location_status(X, Y, 0xFFFF);
			if (Status & WALL_VISION_BLOCKED)
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
			Status = Get_location_status(X, Y, 0xFFFF);
			if (Status & WALL_VISION_BLOCKED)
			{
				/* Yes -> Break */
				Result = TRUE;
				break;
			}
		}
	}

	return Result;
}

