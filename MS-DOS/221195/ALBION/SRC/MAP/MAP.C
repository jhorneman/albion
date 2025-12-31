/************
 * NAME     : MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 10-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* pragmas */

#pragma off (unreferenced);

/* includes */

#include <stdlib.h>

#include <BBDEF.H>
#include <BBBASMEM.H>
#include <BBMEM.H>
#include <BBDSA.H>
#include <BBSYSTEM.H>

#include <ALBION.H>

#include <XLOAD.H>
#include <GFXFUNC.H>
#include <FINDCOL.H>

#include <CONTROL.H>
#include <MAP.H>
#include <AUTOMAP.H>
#include <NPCS.H>
#include <GAME.H>
#include <GAMEVAR.H>
#include <2D_MAP.H>
#include <3D_MAP.H>
#include <3D_PREP.H>
#include <3DCOMMON.H>
#include <ANIMATE.H>
#include <XFTYPES.H>
#include <PRTLOGIC.H>
#include <EVELOGIC.H>
#include <SOUND.H>
#include <FONT.H>
#include <POPUP.H>
#include <INPUT.H>
#include <DIALOGUE.H>
#include <GAMETIME.H>
#include <STATAREA.H>
#include <GAMETEXT.H>
#include <MAGIC.H>
#include <GRAPHICS.H>
#include <TEXTWIN.H>
#include <ITEMSEL.H>
#include <SPECITEM.H>
#include <ICCHANGE.H>
#include <MAP_PUM.H>
#include <COLOURS.H>
#include <3D_MOVE.H>

/* defines */

/* prototypes */

void Get_map_data_offsets(struct Map_data *Map_ptr);

/* Map palette functions */
void Update_map_palette(void);
void Blend_map_palette(BOOLEAN Update_flag);
UNSHORT Get_target_map_blending_percentage(void);

/* global variables */

MEM_HANDLE Map_handle;
MEM_HANDLE Map_text_handle;

/* Global flags */
BOOLEAN _3D_map;
BOOLEAN Spaceship_map;
BOOLEAN Map_initialized = FALSE;
BOOLEAN Map_display_initialized = FALSE;
BOOLEAN Map_changed = FALSE;
BOOLEAN Map_events_changed = FALSE;
BOOLEAN Move_mode_flag = FALSE;
BOOLEAN Moved;

UNSHORT Map_square_size = 1;

UNSHORT Special_map_exit_mode = NO_MX_MODE;

UNSHORT NPC_reaction_radius = 1;

struct Position_data Old_position;

struct BBPALETTE Day_map_palette;
static struct BBPALETTE Night_map_palette;

/* Data from the current map */
UNSHORT Current_map_type;
UNSHORT Current_map_palette_nr;
UNSHORT Current_map_music_nr;
UNSHORT Current_map_light_state;
UNSHORT Map_width;
UNSHORT Map_height;
UNLONG Map_size;

/* Offsets in map data */
UNLONG Map_layers_offset;
UNLONG Event_data_offset;
UNLONG Event_entry_offset;
UNLONG NPC_path_offset;
UNLONG Goto_point_offset;
UNLONG Event_automap_offset;
UNLONG Event_chain_offset;

UNSHORT Nr_event_blocks;
UNSHORT Nr_goto_points;

static UNLONG Animation_timer;
static UNSHORT Remaining_ticks;
static UNSHORT Animation_period;

/* Blending percentage :
	  0...100	From darkness to night palette
	100...200	From night palette to day palette
*/
static UNSHORT Current_map_blending_percentage;

/* Offsets for movement in 4 directions */
SISHORT Offsets4[4][2] = {
	{0, -1}, {1, 0}, {0, 1}, {-1, 0}
};

/* Offsets for movement in 8 directions */
SISHORT Offsets8[8][2] = {
	{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
};

/* 2D view direction table */
UNSHORT View_directions[3][3] = {
	NORTH,	NORTH,	NORTH,
	WEST,		0xFFFF,	EAST,
	SOUTH,	SOUTH,	SOUTH
};

/* 2D movement direction table
  (used by 2D map to determine movement direction from vector) */
UNSHORT Move_directions[3][3] = {
	NORTH_WEST,	NORTH8,	NORTH_EAST,
	WEST8,	 	0xFFFF,	EAST8,
	SOUTH_WEST,	SOUTH8,	SOUTH_EAST
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map
 * FUNCTION  : Initialize a new map
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.94 13:30
 * LAST      : 22.10.95 20:11
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE (success) or FALSE (failure).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Init_map(void)
{
	struct Map_data *Map_ptr;
	BOOLEAN Screen_was_faded_out = FALSE;
	UNSHORT Old_map_nr;
	UNSHORT Colour;

	/* Has the screen been faded out ? */
	if (Current_fade_percentage == 100)
	{
		/* Yes -> Find the fade target colour */
		Colour = Find_closest_colour
		(
			Current_fade_R,
			Current_fade_G,
			Current_fade_B
		);

		/* Clear the screen */
		OPM_FillBox
		(
			&Main_OPM,
			0,
			0,
			Screen_width,
			Panel_Y,
			(UNBYTE) Colour
		);

		/* Un-fade */
		Recolour_palette
		(
			Current_fade_first_colour,
			Current_fade_nr_colours,
			Current_fade_R,
			Current_fade_G,
			Current_fade_B,
			0
		);

		Switch_screens();

		/* Set flag for later */
		Screen_was_faded_out = TRUE;
	}

	/* Clear flag */
	Map_initialized = FALSE;

	/* Load map */
	Map_handle = Load_subfile(MAP_DATA, PARTY_DATA.Map_nr);
	if (!Map_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Load map texts */
	Map_text_handle = Load_subfile(MAP_TEXT, PARTY_DATA.Map_nr);
	if (!Map_text_handle)
	{
		Error(ERROR_FILE_LOAD);
		return FALSE;
	}

	/* Analyze map data */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	/* Is it a 2D or a 3D map ? */
	if (Map_ptr->Display_type == 1)
		_3D_map = TRUE;
	else
		_3D_map = FALSE;

	/* Is it a planet or a spaceship map ? */
	if (Map_ptr->Flags & PLANET_SPACESHIP)
		Spaceship_map = TRUE;
	else
		Spaceship_map = FALSE;

	/* Get the map type */
	Current_map_type = (Map_ptr->Flags & MAP_TYPE) >> MAP_TYPE_B;

	/* Get the palette number */
	Current_map_palette_nr = (UNSHORT) Map_ptr->Colour_pal_nr;

	/* Loading a game ? */
	if (Loading_game)
	{
		/* Yes -> Get the current map light state */
		Current_map_light_state = PARTY_DATA.Light_state;

		/* Get the current map music number */
		Current_map_music_nr = PARTY_DATA.Map_music_nr;
	}
	else
	{
		/* No -> Get the current map light state */
		Current_map_light_state = (UNSHORT)(Map_ptr->Flags & LIGHT_STATUS);

		/* Get the music number */
		Current_map_music_nr = (UNSHORT) Map_ptr->Music;
	}

	/* Get the map dimensions */
	Map_width	= (UNSHORT) Map_ptr->Width;
	Map_height	= (UNSHORT) Map_ptr->Height;

	Map_size		= Map_width * Map_height;

	/* Get animation speed */
	Animation_period = Map_ptr->Animation_period;
	if (!Animation_period)
		Animation_period = 5;

	/* Get map data offsets */
	Get_map_data_offsets(Map_ptr);

	MEM_Free_pointer(Map_handle);

	/* Is the current X-coordinate legal ? */
	if ((PARTY_DATA.X < 1) || (PARTY_DATA.X > Map_width))
	{
		/* No -> Set to safe value */
		PARTY_DATA.X = 5;

		/* Report error */
		Error(ERROR_ILLEGAL_X_COORDINATE);
	}

	/* Is the current Y-coordinate legal ? */
	if ((PARTY_DATA.Y < 1) || (PARTY_DATA.Y > Map_height))
	{
		/* No -> Set to safe value */
		PARTY_DATA.Y = 5;

		/* Report error */
		Error(ERROR_ILLEGAL_Y_COORDINATE);
	}

	/* Initialize NPC data */
	Init_NPC_data();

	/* Reset animation timer */
	Animation_timer = SYSTEM_GetTicks();
	Remaining_ticks = 0;

	/* Start the map module */
	if (_3D_map)
	{
		Push_module(&M3_Mod);
	}
	else
	{
		Push_module(&M2_Mod);
	}

	/* Set flag */
	Map_initialized = TRUE;

	/* Modify the map */
	Make_modifications
	(
		PARTY_DATA.Map_nr,
		Permanent_modification_list,
		Nr_permanent_modifications
	);

	/* Loading a game ? */
	if (Loading_game)
	{
		/* Yes -> Modify the map */
		Make_modifications
		(
			PARTY_DATA.Map_nr,
			Temporary_modification_list,
			Nr_temporary_modifications
		);
	}
	else
	{
		UNSHORT Zero = 0;

		/* Initialize temporary modifications */
		Nr_temporary_modifications = Init_modifications
		(
			(UNBYTE *) &Zero,
			&Temporary_modification_list
		);
	}

	/* Indicate that the map has changed */
	Map_changed = TRUE;

	/* Catalogue event chains for certain triggermodes */
	Catalogue_event_chains(EVERY_STEP_TRIGGER);
	Catalogue_event_chains(NPC_TRIGGER);

	/* Switch located sound effects on */
	SOUND_Sound_effects_on();
	SOUND_Located_sound_effects_on();

	/* Set map music */
	Set_music(Current_map_music_nr);

	/* Initialize special items */
	Init_special_items();

	/* Was the screen faded out ? */
	if (Screen_was_faded_out)
	{
		/* Yes -> Fade again */
		Recolour_palette
		(
			Current_fade_first_colour,
			Current_fade_nr_colours,
			Current_fade_R,
			Current_fade_G,
			Current_fade_B,
			100
		);
	}

	/* Loading a game ? */
	if (Loading_game)
	{
		/* Yes -> Initialize display */
		Init_display();
		Update_display();
		Switch_screens();
		Switch_screens();

		/* 3D map ? */
		if (_3D_map)
		{
			/* Yes -> Initialize automap */
			Init_automap();

			/* Update at initial position */
			Update_automap(MAX_UPDATE_DEPTH);
		}

		/* Save position */
		Save_position();
	}
	else
	{
		/* No -> Get current map number */
		Old_map_nr = PARTY_DATA.Map_nr;

		/* Execute map-init event chains */
		Execute_chains_in_map(MAP_INIT_TRIGGER);

		/* Still in the same map ? */
		if (Old_map_nr == PARTY_DATA.Map_nr)
		{
			/* Yes -> Initialize display */
			Init_display();
			Update_display();
			Switch_screens();
			Switch_screens();

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Initialize automap */
				Init_automap();

				/* Update at initial position */
				Update_automap(MAX_UPDATE_DEPTH);
			}

			/* Save position */
			Save_position();

			/* Handle first step */
			After_move();
		}
	}

	return TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_map
 * FUNCTION  : Leave the current map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 11:45
 * LAST      : 24.09.95 17:19
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_map(void)
{
	/* Still in move mode ? */
	if (Move_mode_flag)
	{
		/* Yes -> Exit move mode */
		Pop_module();

		/* Clear flag again just to be sure */
		Move_mode_flag = FALSE;
	}

	/* Exit display */
	Exit_display();

	/* Switch located sound effects off */
	SOUND_Located_sound_effects_off();

	/* Exit special items */
	Exit_special_items();

	/* Exit map module */
	Pop_module();

	/* Destroy temporary modifications */
	Exit_modifications(Temporary_modification_list);

	Temporary_modification_list	= NULL;
	Nr_temporary_modifications		= 0;

	/* Destroy catalogued event chains */
	Destroy_event_chain_catalogues();

	/* 3D map ? */
	if (_3D_map)
	{
		/* Yes -> Exit automap */
		Exit_automap();
	}

	/* Clear flag */
	Map_initialized = FALSE;

	/* Free memory */
	MEM_Free_memory(Map_text_handle);

	/* Kill (!!!) memory */
	MEM_Kill_memory(Map_handle);

	/* Clear handles */
	Map_handle = NULL;
	Map_text_handle = NULL;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map_display
 * FUNCTION  : Initialize map display.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 14:54
 * LAST      : 31.08.95 14:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_map_display(void)
{
	/* Initialise map palette */
	Init_map_palette();

	/* Install map mouse pointer */
	Push_mouse(DEFAULT_MPTR);

	/* Show special items */
	Show_special_items();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_map_display
 * FUNCTION  : Exit map display.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 14:55
 * LAST      : 31.08.95 14:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_map_display(void)
{
	/* Hide special items */
	Hide_special_items();

	/* Remove map mouse pointer */
	Pop_mouse();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_map_display
 * FUNCTION  : Update map display.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 19.06.95 15:49
 * LAST      : 19.06.95 15:49
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_map_display(void)
{
	if (_3D_map)
		Display_3D_map();
	else
		Display_2D_map();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_map_stuff
 * FUNCTION  : Update map stuff.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 11.08.94 11:45
 * LAST      : 25.07.95 14:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - Palette activation is handled indirectly by
 *              Colour_cycle_forward.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_map_stuff(void)
{
	UNLONG T;

	/* Get elapsed time and add to remaining ticks */
	T = SYSTEM_GetTicks();

	if (T >= Animation_timer)
		Remaining_ticks += T - Animation_timer + 1;
	else
		Remaining_ticks += Animation_timer - T + 1;

	Animation_timer = T;

	/* Adjust remaining ticks if too high */
	if (Remaining_ticks > 32000)
		Remaining_ticks -= 32000;

	/* Animation required ? */
	if (Animation_period < 150)
	{
		/* Yes -> Time to update colour cycling and animations ? */
		if (Remaining_ticks > Animation_period)
		{
			/* Do colour cycling */
			Cycle_colours();

			/* Update map palette */
			Update_map_palette();

			/* Update animations */
			Update_animation();

			/* Count down */
			while (Remaining_ticks > Animation_period)
				Remaining_ticks -= Animation_period;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map_palette
 * FUNCTION  : Initialise map palette.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 14:32
 * LAST      : 13.10.95 21:05
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_map_palette(void)
{
	static const UNSHORT Second_map_palette_nrs[] = {
		47, 47, 55, 48,  0,  0,  0,  0,  0,  0,
		 0,  0,  0, 49,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0, 49,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		49,  0,  0,  0,  0,  0,  0,  0,  0,  0
	};

	struct Lab_data *Lab_ptr;
	MEM_HANDLE Handle;
	BOOLEAN Result;
	UNSHORT Target_R, Target_G, Target_B;
	UNSHORT Second_palette_nr;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Load main map palette */
	Result = Load_palette(Current_map_palette_nr);
	if (!Result)
		return;

	/* Initialize day palette structure */
	Day_map_palette.entries = 256;
	Day_map_palette.version = 0;

	/* Initialize night palette structure */
	Night_map_palette.entries = 256;
	Night_map_palette.version = 0;

	/* Is there a need to load more than one palette ? */
	if (Current_map_light_state == CHANGING_LIGHT)
	{
		/* Yes -> Get second palette number */
		Second_palette_nr = Second_map_palette_nrs[Current_map_palette_nr - 1];

		/* Any number given ? */
		if (Second_palette_nr)
		{
			/* Yes -> Copy main map palette */
			for (i=0;i<192;i++)
			{
				Day_map_palette.color[i].red		= Palette.color[i].red;
				Day_map_palette.color[i].green	= Palette.color[i].green;
		 		Day_map_palette.color[i].blue		= Palette.color[i].blue;
			}

			/* Load second palette file */
			Handle = Load_subfile(PALETTE, Second_palette_nr);
			if (!Handle)
			{
				Error(ERROR_FILE_LOAD);
				return;
			}

			/* Copy colours to second palette */
			Ptr = MEM_Claim_pointer(Handle);
			for (i=0;i<192;i++)
			{
				Night_map_palette.color[i].red	= *Ptr++;
				Night_map_palette.color[i].green	= *Ptr++;
		 		Night_map_palette.color[i].blue	= *Ptr++;
			}

			/* Destroy palette file */
			MEM_Free_pointer(Handle);
			MEM_Free_memory(Handle);
		}
		else
		{
			/* No -> Copy colours from main map palette */
			for (i=0;i<192;i++)
			{
				Day_map_palette.color[i].red		= Palette.color[i].red;
				Day_map_palette.color[i].green	= Palette.color[i].green;
		 		Day_map_palette.color[i].blue		= Palette.color[i].blue;

				Night_map_palette.color[i].red	= Palette.color[i].red;
				Night_map_palette.color[i].green	= Palette.color[i].green;
		 		Night_map_palette.color[i].blue	= Palette.color[i].blue;
			}
		}

		/* Reset blending */
		Reset_map_palette_blending();
	}
	else
	{
		/* No -> Copy colours from main map palette */
		for (i=0;i<192;i++)
		{
			Day_map_palette.color[i].red		= Palette.color[i].red;
			Day_map_palette.color[i].green	= Palette.color[i].green;
		 	Day_map_palette.color[i].blue		= Palette.color[i].blue;

			Night_map_palette.color[i].red	= Palette.color[i].red;
			Night_map_palette.color[i].green	= Palette.color[i].green;
		 	Night_map_palette.color[i].blue	= Palette.color[i].blue;
		}

		/* 3D map ? */
		if (_3D_map)
		{
			/* Yes -> Get target colour for 3D shade table from lab data */
			Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

			Target_R = Lab_ptr->Target_R;
			Target_G = Lab_ptr->Target_G;
			Target_B = Lab_ptr->Target_B;

			MEM_Free_pointer(Lab_data_handle);

			/* Re-make all 3D shade tables */
			Make_all_shade_tables(Target_R, Target_G, Target_B);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Reset_map_palette_blending
 * FUNCTION  : Reset the map palette blending.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 22:48
 * LAST      : 10.10.95 00:27
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Reset_map_palette_blending(void)
{
	/* Must the map palette be blended ? */
	if (Current_map_light_state == CHANGING_LIGHT)
	{
		/* Yes -> Calculate target blending percentage and set current blending
		 percentage to target */
		Current_map_blending_percentage = Get_target_map_blending_percentage();

		/* Shade tables are no longer valid */
		Shade_tables_are_valid = FALSE;

		/* Blend palette */
		Blend_map_palette(TRUE);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_map_palette
 * FUNCTION  : Update the map palette.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 15:11
 * LAST      : 09.10.95 14:36
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_map_palette(void)
{
	static UNSHORT Last_map_step = 0xFFFF;

	SISHORT Delta;
	SISHORT Target_map_blending_percentage;

	/* Must the map palette be blended ? */
	if (Current_map_light_state == CHANGING_LIGHT)
	{
		/* Yes -> Has any time passed ? */
		if (Current_step != Last_map_step)
		{
			/* Yes -> Store current step */
			Last_map_step = Current_step;

			/* Calculate target blending percentage */
			Target_map_blending_percentage = Get_target_map_blending_percentage();

			/* 2D or 3D map ? */
			if (_3D_map)
			{
				/* 3D map -> Any difference between current and target ? */
				if (Target_map_blending_percentage != Current_map_blending_percentage)
				{
					/* Yes -> Change current blending percentage */
					Current_map_blending_percentage = Target_map_blending_percentage;

					/* Shade tables are no longer valid */
					Shade_tables_are_valid = FALSE;

					/* Re-blend the map palette */
					Blend_map_palette(FALSE);
				}
			}
			else
			{
				/* 2D map -> Calculate difference */
				Delta = Target_map_blending_percentage -
				 Current_map_blending_percentage;

				/* Any difference ? */
				if (Delta)
				{
					/* Yes -> Change current blending percentage */
					if (abs(Delta) < 8)
					{
						Current_map_blending_percentage += Delta;
					}
					else
					{
						Current_map_blending_percentage += sgn(Delta) * 8;
					}

					/* Re-blend the map palette */
					Blend_map_palette(FALSE);
				}
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Blend_map_palette
 * FUNCTION  : Blend the map palette.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 14:54
 * LAST      : 09.10.95 12:32
 * INPUTS    : BOOLEAN Update_flag - TRUE if palette should always be updated.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Blend_map_palette(BOOLEAN Update_flag)
{
	struct BBPALETTE Black_palette;
	struct Lab_data *Lab_ptr;
	BOOLEAN Changed_palette;
	UNSHORT Target_R, Target_G, Target_B;

	/* Must the map palette be blended ? */
	if (Current_map_light_state == CHANGING_LIGHT)
	{
		/* Yes -> 3D map ? */
		if (_3D_map)
		{
			/* Yes -> Which blending stage ? */
			if (Current_map_blending_percentage < 100)
			{
				/* From black to night -> Target colour for 3D shade table is
				  black */
				Target_R = 0;
				Target_G = 0;
				Target_B = 0;
			}
			else
			{
				/* From night to day -> Get target colour for 3D shade table
				  from lab data */
				Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

				Target_R = Lab_ptr->Target_R;
				Target_G = Lab_ptr->Target_G;
				Target_B = Lab_ptr->Target_B;

				MEM_Free_pointer(Lab_data_handle);
			}
		}

		/* Which blending stage ? */
		if (Current_map_blending_percentage < 100)
		{
			/* From black to night -> Clear black palette */
			BASEMEM_FillMemByte
			(
				(UNBYTE *) &Black_palette,
				sizeof(struct BBPALETTE),
				0
			);

			/* Initialize black palette structure */
			Black_palette.entries = 256;

			/* Blend */
			Changed_palette = Blend_palette
			(
				1,
				191,
				Current_map_blending_percentage,
				&Black_palette,
				&Night_map_palette
			);
		}
		else
		{
			/* From night to day */
			Changed_palette = Blend_palette
			(
				1,
				191,
				Current_map_blending_percentage - 100,
				&Night_map_palette,
				&Day_map_palette
			);
		}

		/* Update / has the palette changed ? */
		if (Update_flag || Changed_palette)
		{
			/* Yes -> Update palette */
			Update_palette();

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Re-make all 3D shade tables */
				Make_all_shade_tables(Target_R, Target_G, Target_B);
			}
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_target_map_blending_percentage
 * FUNCTION  : Get the target map blending percentage.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 14:56
 * LAST      : 09.10.95 19:44
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_target_map_blending_percentage(void)
{
	static const UNSHORT Map_blending_percentages[24] = {
	  50,  50,  50,  50,  75, 100,   //  0 -  5
	 125, 150, 175, 200, 200, 200,   //  6 - 11
	 200, 200, 200, 200, 200, 175,	// 12 - 17
	 150, 125, 100,  75,  50,  50		// 18 - 23
	};

	return Map_blending_percentages[PARTY_DATA.Hour];
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Colour_cycle_forward_2D
 * FUNCTION  : Cycle colours forward in a 2D map.
 * FILE      : COLOURS.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.08.95 16:20
 * LAST      : 31.08.95 16:20
 * INPUTS    : UNSHORT Start - Number of first colour (0...255).
 *             UNSHORT Size - Number of colours to cycle.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Colour_cycle_forward_2D(UNSHORT Start, UNSHORT Size)
{
	/* Must the map palette be blended ? */
	if (Current_map_light_state == CHANGING_LIGHT)
	{
		/* Yes -> Cycle day and night palettes */
		Colour_cycle_forward(&Night_map_palette, Start, Size);
		Colour_cycle_forward(&Day_map_palette, Start, Size);
	}

	/* Cycle palette */
	Colour_cycle_forward(&Palette, Start, Size);

	/* Tell CONTROL to activate palette */
	Palette_has_changed = TRUE;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_map_data_offsets
 * FUNCTION  : Calculate offsets into map data.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.07.95 18:08
 * LAST      : 28.07.95 18:08
 * INPUTS    : struct Map_data *Map_ptr - Pointer to map data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Get_map_data_offsets(struct Map_data *Map_ptr)
{
	UNLONG Offset;
	UNSHORT *WPtr;
	UNSHORT i;
	UNBYTE *Ptr;

	Ptr = (UNBYTE *) Map_ptr;

	/* Start after the header */
	Offset = sizeof(struct Map_data);

	/* Skip NPC data */
	if (Map_ptr->Flags & MORE_NPCS)
	{
		Offset += 96 * sizeof(union NPC_data);
	}
	else
	{
		Offset += 32 * sizeof(union NPC_data);
	}

	/* Store map layers offset */
	Map_layers_offset = Offset;

	/* Skip map layers */
	Offset += Map_size * 3;

	/* Store event entry-lists offset */
	Event_entry_offset = Offset;

	/* Skip through the event entry-lists */
	for (i=0;i<=Map_height;i++)
	{
		WPtr = (UNSHORT *)(Ptr + Offset);
		Offset += (*WPtr * sizeof(struct Map_event_entry)) + 2;
	}

	/* Read number of event blocks */
	WPtr = (UNSHORT *)(Ptr + Offset);
	i = *WPtr;
	Offset += 2;

	/* Store number of event blocks & event blocks offset */
	Nr_event_blocks = i;
	Event_data_offset = Offset;

	/* Skip over event blocks */
	Offset += (i * sizeof(struct Event_block));

	/* Store NPC path offset */
	NPC_path_offset = Offset;

	/* Skip through NPC paths */
	Offset = Skip_through_NPC_paths(Map_ptr, Offset);

	/* Is this a 3D map ? */
	if (_3D_map)
	{
		/* Yes -> Read number of goto points */
		WPtr = (UNSHORT *)(Ptr + Offset);
		i = *WPtr;
		Offset += 2;

		/* Store number of goto points & goto point data offset */
		Nr_goto_points = i;
		Goto_point_offset = Offset;

		/* Skip goto point data */
		Offset += (i * sizeof(struct Goto_point));

		/* Store event automapper data offset */
		Event_automap_offset = Offset;

		/* Skip automapper data offset */
		Offset += EVENTS_PER_3D_MAP;
	}

	/* Store event chain data offset */
	Event_chain_offset = Offset;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Set_music
 * FUNCTION  : Set normal and ambient music.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 18:08
 * LAST      : 29.10.95 23:33
 * INPUTS    : UNSHORT Music_nr - New music index.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The music index is not the actual song number but an index
 *              into two tables.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_music(UNSHORT Music_nr)
{
	/* Physical song numbers, indexed by logical map song number */
	static const UNSHORT Normal_song_numbers[60] = {
		  1,  2,  3,  4,  0,  0,  0,  0,  0,  0,	/*  1 - 10 */
		  0,  0,  0,  0,  0,  0, 17, 18, 19, 20,	/* 11 - 20 */
		  0,  0,  4, 24, 25,  0, 27,  0,  0,  0,	/* 21 - 30 */
		  0,  0,  0,  0, 35,  0, 37, 38, 18, 40,	/* 31 - 40 */
		 41, 42,  3, 38, 41, 41, 20, 41, 37,  1,	/* 41 - 50 */
		 24, 19, 24, 39, 23, 43, 44, 45,  0,  0	/* 51 - 60 */
	};

	/* Ambient song numbers, indexed by logical map song number */
	static const UNSHORT Ambient_song_numbers[60] = {
		 15,  6, 34,  5,  0,  0,  0,  0,  0,  0,	/*  1 - 10 */
		  0,  0,  0,  0,  0,  0,  7, 14, 31,  5,	/* 11 - 20 */
		  0,  0, 22, 31,  8,  0, 11,  0,  0,  0,	/* 21 - 30 */
		  0,  0,  0,  0, 21,  0, 15, 34, 14, 36,	/* 31 - 40 */
		 10, 12,  9,  9, 13, 16, 22, 28, 29, 29,	/* 41 - 50 */
		 32, 32, 33,  5, 12,  0,  0,  0,  0,  0	/* 51 - 60 */
	};

	/* Any music / legal number ? */
	if (Music_nr && (Music_nr < 60))
	{
		/* Yes -> Play song */
		SOUND_Play_song(Normal_song_numbers[Music_nr - 1]);

		/* Play ambient song */
		SOUND_Play_ambient_song(Ambient_song_numbers[Music_nr - 1]);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Before_move
 * FUNCTION  : Before move logic.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 11:45
 * LAST      : 21.09.95 22:40
 * INPUTS    : None.
 * RESULT    : BOOLEAN : TRUE - movement allowed, FALSE - movement forbidden.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Before_move(void)
{
	BOOLEAN Result = TRUE;
	UNSHORT i;

	/* Save position */
	Save_position();

	/* Check all party members */
	for (i=0;i<6;i++)
	{
		/* Anyone there ? */
		if (Member_present(i + 1))
		{
			/* Yes -> Overweight ? */
			if (Character_is_overweight(Party_char_handles[i]))
			{
				/* Yes -> Inform the player */
				Subject_char_handle = Party_char_handles[i];
				Set_permanent_message_nr(699);

				/* No movement is allowed */
				Result = FALSE;
				break;
			}
		}
	}

	return Result;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : After_move
 * FUNCTION  : After move logic.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 11:45
 * LAST      : 06.09.94 11:45
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
After_move(void)
{
	Time_after_move();
	Game_after_move();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Time_after_move
 * FUNCTION  : After move time logic.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 11:34
 * LAST      : 21.02.95 11:34
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Time_after_move(void)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Game_after_move
 * FUNCTION  : After move game logic.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 11:34
 * LAST      : 25.07.95 12:04
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Game_after_move(void)
{
	Check_for_new_goto_points();
	Execute_normal_map_events();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Execute_normal_map_events
 * FUNCTION  : Execute normal map events.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 25.07.95 18:55
 * LAST      : 25.07.95 18:55
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Execute_normal_map_events(void)
{
	UNSHORT Old_X, Old_Y;
	UNSHORT Old_map_nr;

	/* Save coordinates and map number */
	Old_X			= PARTY_DATA.X;
	Old_Y			= PARTY_DATA.Y;
	Old_map_nr	= PARTY_DATA.Map_nr;

	/* Handle normal event chains */
	Trigger_map_event_chain
	(
		PARTY_DATA.X,
		PARTY_DATA.Y,
		0xFFFF,
		NORMAL_TRIGGER,
		0
	);

	/* In a 2D map / big guy ? */
	if (!_3D_map && Big_guy)
	{
		/* Yes -> Still in the same place / map / active member capable ? */
		if ((PARTY_DATA.X == Old_X) &&
		 (PARTY_DATA.Y == Old_Y) &&
		 (PARTY_DATA.Map_nr == Old_map_nr) &&
		 (Character_capable(Active_char_handle)))
		{
			/* Yes -> Execute second event */
			Trigger_map_event_chain
			(
				PARTY_DATA.X + 1,
				PARTY_DATA.Y,
				0xFFFF,
				NORMAL_TRIGGER,
				0
			);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_location_status
 * FUNCTION  : Get the status of a location.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 06.09.94 16:47
 * LAST      : 06.09.94 16:47
 * INPUTS    : SISHORT X - X-coordinate in map (1...).
 *             SISHORT Y - Y-coordinate in map (1...).
 *             UNSHORT NPC_index - Index of NPC / 0xFFFF for party.
 * RESULT    : UNLONG : Location status.
 * BUGS      : No known.
 * NOTES     : - This function will return 0 if the map hasn't been
 *              initialized or the coordinates lie outside of the map.
 *             - In 2D maps, this function will output :
 *              the NPC icon status when :
 *                1 - an NPC is present,
 *                2 - it is switched to map graphics, and
 *                3 - the {Underlay priority}-bit is cleared.
 *              Otherwise it will output the overlay icon status when an
 *              overlay is present and the {Underlay priority}-bit is cleared.
 *             - In 3D maps, this function will output :
 *              the wall status when a wall is present.
 *              Otherwise it will output the floor status when a floor is
 *              present.
 *              Objects (and thus NPCs) aren't checked.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNLONG
Get_location_status(SISHORT X, SISHORT Y, UNSHORT NPC_index)
{
	UNLONG Under_status = 0;
	UNLONG Over_status = 0;
	UNLONG NPC_status = 0;
	UNSHORT Underlay_nr = 0;
	UNSHORT Overlay_nr = 0;
	UNSHORT NPC_icon_nr = 0;
	UNSHORT i;
	UNBYTE *Map_data_ptr;

	/* Return 0 if the map isn't initialized */
	if (!Map_initialized)
		return 0;

	/* Return 0 if the coordinates lie outside of the map */
	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return 0;

	/* Get map data */
	Map_data_ptr = MEM_Claim_pointer(Map_handle);

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		struct Lab_data *Lab_ptr;
		struct Square_3D *Map_ptr;
		struct Wall_3D *Wall_ptr;
		struct Overlay_3D *Overlay_ptr;
		struct Floor_3D *Floor_ptr;
		UNSHORT Wall_nr;
		UNSHORT Floor_nr;
		UNBYTE *Ptr;

		/* 3D map -> Get lab data */
		Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

		Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
		Map_ptr += X - 1 + ((Y - 1) * Map_width);

		/* Get wall / object number from map */
		Wall_nr = (UNSHORT) Map_ptr->Wall_layer;

		/* Empty ? */
		if (Wall_nr)
		{
			/* No -> Object group ? */
			if (Wall_nr >= FIRST_WALL)
			{
				/* No -> Legal wall number ? */
				Wall_nr -= FIRST_WALL;
				if (Wall_nr < Nr_3D_walls)
				{
					/* Yes -> Find wall data */
					Ptr = (UNBYTE *) (Lab_ptr + 1)
					 + (Nr_3D_object_groups * sizeof(struct Object_group_3D)) + 2
					 + (Nr_3D_floors * sizeof(struct Floor_3D)) + 2
					 + (Nr_3D_objects * sizeof(struct Object_3D)) + 2;
					Wall_ptr = (struct Wall_3D *) Ptr;

					for (i=0;i<Wall_nr;i++)
					{
						/* Skip overlay data */
						Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
						Overlay_ptr += Wall_ptr->Nr_overlays;
						Wall_ptr = (struct Wall_3D *) Overlay_ptr;
					}

					/* Get wall data */
					Underlay_nr = 0xFFFF;
					Under_status = Wall_ptr->Flags;
				}
			}
		}

		/* Found anything yet ? */
		if (!Underlay_nr)
		{
			/* No -> Get floor number from map */
			Floor_nr = (UNSHORT) Map_ptr->Floor_layer;

			/* Empty ? */
			if (Floor_nr)
			{
				/* No -> Legal floor number ? */
				Floor_nr--;
				if (Floor_nr < Nr_3D_floors)
				{
					/* Yes -> Find floor data */
					Ptr = (UNBYTE *) (Lab_ptr + 1)
					 + (Nr_3D_object_groups * sizeof(struct Object_group_3D)) + 2;
					Floor_ptr = (struct Floor_3D *) Ptr;

					/* Get floor data */
					Underlay_nr = 0xFFFF;
					Under_status = Floor_ptr[Floor_nr].Flags;
				}
			}
		}

		MEM_Free_pointer(Lab_data_handle);
	}
	else
	{
		struct Icon_2D_data *Icon_data;
		struct Square_2D *Map_ptr;
		UNSHORT B1, B2, B3;

		/* 2D map -> Get icon data */
		Icon_data = (struct Icon_2D_data *)
		 MEM_Claim_pointer(Icondata_handle);

		#if FALSE
		/* Check NPCs */
		for(i=0;i<NPCS_PER_MAP;i++)
		{
			/* Anyone there / not checking myself ? */
			if ((NPC_present(i)) && (NPC_index != i))
			{
				/* Yes -> Right coordinates / map graphics ? */
				if ((VNPCs[i].Map_X == X) && (VNPCs[i].Map_Y == Y) &&
				 (!(VNPCs[i].NPC_data.Flags & NPC_GRAPHICS)))
				{
					/* Yes -> Get icon number */
					NPC_icon_nr = VNPCs[i].NPC_data.Graphics_nr;

					/* Any icon ? */
					if (NPC_icon_nr > 1)
					{
						/* Yes -> Get icon data */
						NPC_status = Icon_data[NPC_icon_nr-2].Flags;

						/* Does the "underlay" have priority ? */
						if (NPC_status & UNDERLAY_PRIORITY)
						{
							/* Yes -> Discard data */
							NPC_icon_nr = 0;
							NPC_status = 0;
						}
						else
						{
							/* No -> Don't check any other NPCs */
							break;
						}
					}
				}
			}
		}

		/* Do we need to check the map ? */
		if (!NPC_icon_nr)
		#endif
		{
			/* Yes */
			Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
			Map_ptr += X - 1 + ((Y - 1) * Map_width);

			/* Read bytes from map */
			B1 = (UNSHORT) Map_ptr->m[0];
			B2 = (UNSHORT) Map_ptr->m[1];
			B3 = (UNSHORT) Map_ptr->m[2];

			/* Build overlay and underlay number */
			Underlay_nr = ((B2 & 0x0F) << 8) | B3;
			Overlay_nr = (B1 << 4) | (B2 >> 4);

			/* Any underlay ? */
			if (Underlay_nr > 1)
			{
				/* Yes -> Get icon data */
				Under_status = Icon_data[Underlay_nr-2].Flags;
			}
			else
			{
				/* No */
				Underlay_nr = 0;
			}

			/* Any overlay ? */
			if (Overlay_nr > 1)
			{
				/* Yes -> Get icon data */
				Over_status = Icon_data[Overlay_nr-2].Flags;

				/* Does the underlay have priority ? */
				if (Over_status & UNDERLAY_PRIORITY)
				{
					/* Yes -> Discard data */
					Overlay_nr = 0;
					Over_status = 0;
				}
			}
			else
			{
				/* No */
				Overlay_nr = 0;
			}
		}

		MEM_Free_pointer(Icondata_handle);
	}

	MEM_Free_pointer(Map_handle);

	/* Is there an NPC ? */
	if (NPC_icon_nr)
	{
		/* Yes -> Use NPC status */
		return NPC_status;
	}

	/* Is there an overlay ? */
	if (Overlay_nr)
	{
		/* Yes -> Use overlay status */
		return Over_status;
	}
	else
	{
		/* No -> Use underlay status */
		return Under_status;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Seek_transport
 * FUNCTION  : Seek a transportation.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 01.03.95 14:39
 * LAST      : 01.03.95 14:39
 * INPUTS    : UNSHORT Map_nr - Map number.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 * RESULT    : UNSHORT : Transport type (0 = no transport).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Seek_transport(UNSHORT Map_nr, SISHORT X, SISHORT Y)
{
	struct Trans_data *Trans;
	UNSHORT i;

	/* Check all registered transportations */
	for (i=0;i<MAX_TRANSPORTS;i++)
	{
		Trans = &(PARTY_DATA.Transportations[i]);

		/* Any transportation in this slot ? */
		if (Trans->Type)
		{
			/* Yes -> Right coordinates and map number ? */
			if ((Trans->X == X) && (Trans->Y == Y) && (Trans->Map_nr == Map_nr))
			{
				/* Yes -> Found something */
				return Trans->Type;
			}
		}
	}

	/* Found nothing */
	return 0;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Create_transport
 * FUNCTION  : Create a transportation.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 13:15
 * LAST      : 08.03.95 13:15
 * INPUTS    : UNSHORT Map_nr - Map number.
 *             SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Transport_type - Transport type.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Create_transport(UNSHORT Map_nr, SISHORT X, SISHORT Y, UNSHORT Transport_type)
{
	struct Trans_data *Trans;
	UNSHORT i;

	/* Check all registered transportations */
	for (i=0;i<MAX_TRANSPORTS;i++)
	{
		Trans = &(PARTY_DATA.Transportations[i]);

		/* Any transportation in this slot ? */
		if (!Trans->Type)
		{
			/* No -> Insert data */
			Trans->Type = Transport_type;
			Trans->X = X;
			Trans->Y = Y;
			Trans->Map_nr = Map_nr;

			break;
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Find_Goto_point_data_in_map
 * FUNCTION  : Find goto-point data in a map file.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 14:01
 * LAST      : 09.05.95 14:10
 * INPUTS    : struct Map_data *Map_ptr - Pointer to map data.
 * RESULT    : UNSHORT * : Pointer to number of goto-points.
 * BUGS      : No known.
 * NOTES     : - This function is needed to clone automaps. It does NOT
 *              use the CURRENT map!
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT *
Find_Goto_point_data_in_map(struct Map_data *Map_ptr)
{
	UNLONG Offset = 0;
	UNSHORT *WPtr;
	UNSHORT Width, Height;
	UNSHORT i;
	UNBYTE *Ptr = NULL;

	/* Is this a 3D map ? */
	if (Map_ptr->Display_type == 1)
	{
		/* Yes -> Get map width and height */
		Width = (UNSHORT) Map_ptr->Width;
		Height = (UNSHORT) Map_ptr->Height;

		Ptr = (UNBYTE *) Map_ptr;

		/* Start after the header */
		Offset = sizeof(struct Map_data);

		/* Skip NPC data */
		if (Map_ptr->Flags & MORE_NPCS)
		{
			Offset += 96 * sizeof(union NPC_data);
		}
		else
		{
			Offset += 32 * sizeof(union NPC_data);
		}

		/* Skip map layers */
		Offset += (Width * Height * sizeof(struct Square_3D));

		/* Skip through the event entry-lists */
		for (i=0;i<=Height;i++)
		{
			WPtr = (UNSHORT *)(Ptr + Offset);
			Offset += (*WPtr * sizeof(struct Map_event_entry)) + 2;
		}

		/* Skip over event blocks */
		WPtr = (UNSHORT *)(Ptr + Offset);
		Offset += (*WPtr * sizeof(struct Event_block)) + 2;

		/* Skip through NPC paths */
		Offset = Skip_through_NPC_paths(Map_ptr, Offset);
	}

	return (UNSHORT *) (Ptr + Offset);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Vision_blocked
 * FUNCTION  : Check if a certain position in a 3D map blocks vision.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 07.02.95 16:52
 * LAST      : 01.05.95 18:23
 * INPUTS    : SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 * RESULT    : BOOLEAN : TRUE (blocked) or FALSE (not blocked).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

BOOLEAN
Vision_blocked(SISHORT X, SISHORT Y)
{
	struct Lab_data *Lab_ptr;
	struct Square_3D *Map_ptr;
	struct Wall_3D *Wall_ptr;
	struct Overlay_3D *Overlay_ptr;
	UNLONG Status = 0;
	UNSHORT Wall_nr, i;
	UNBYTE *Map_data_ptr;
	UNBYTE *Ptr;

	/* Return if the map isn't initialized */
	if (!Map_initialized || !_3D_map)
		return (TRUE);

	/* Return if this is a 2D map */
	if (!_3D_map)
		return (FALSE);

	/* Return if the coordinates lie outside of the map */
	if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
		return (TRUE);

	/* Get map and lab data */
	Map_data_ptr = MEM_Claim_pointer(Map_handle);
	Lab_ptr = (struct Lab_data *) MEM_Claim_pointer(Lab_data_handle);

	Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
	Map_ptr += X - 1 + ((Y - 1) * Map_width);

	/* Get wall number from map */
	Wall_nr = (UNSHORT) Map_ptr->Wall_layer;

	/* Empty ? */
	if (Wall_nr)
	{
		/* No -> Object group ? */
		if (Wall_nr >= FIRST_WALL)
		{
			/* No -> Legal wall number ? */
			Wall_nr -= FIRST_WALL;
			if (Wall_nr < Nr_3D_walls)
			{
				/* Yes -> Find wall data */
				Ptr = (UNBYTE *) (Lab_ptr + 1)
				 + (Nr_3D_object_groups * sizeof(struct Object_group_3D)) + 2
				 + (Nr_3D_floors * sizeof(struct Floor_3D)) + 2
				 + (Nr_3D_objects * sizeof(struct Object_3D)) + 2;
				Wall_ptr = (struct Wall_3D *) Ptr;

				for (i=0;i<Wall_nr;i++)
				{
					/* Skip overlay data */
					Overlay_ptr = (struct Overlay_3D *) (Wall_ptr + 1);
					Overlay_ptr += Wall_ptr->Nr_overlays;
					Wall_ptr = (struct Wall_3D *) Overlay_ptr;
				}

				/* Get wall data */
				Status = Wall_ptr->Flags;
			}
		}
	}
	MEM_Free_pointer(Lab_data_handle);
	MEM_Free_pointer(Map_handle);

	/* Return vision blocked status */
	return (BOOLEAN) (Status & WALL_VISION_BLOCKED);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Get_combat_background_nr
 * FUNCTION  : Get combat background number.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 11:28
 * LAST      : 20.10.95 18:22
 * INPUTS    : None.
 * RESULT    : UNSHORT : Combat background number (1...).
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNSHORT
Get_combat_background_nr(void)
{
	struct Map_data *Map_data;
	UNSHORT Combat_background_nr;

	/* Get combat background from current location */
	Combat_background_nr = (Get_location_status
	(
		PARTY_DATA.X,
		PARTY_DATA.Y,
		0xFFFF
	) & COMBAT_BACKGROUND_NR) >> COMBAT_BACKGROUND_B;

	/* Any ? */
	if (!Combat_background_nr)
	{
		/* No -> Get combat background from map data */
		Map_data = (struct Map_data *) MEM_Claim_pointer(Map_handle);

		Combat_background_nr = (UNSHORT) Map_data->Combat_background_nr;

		MEM_Free_pointer(Map_handle);
	}

	return Combat_background_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Change_position
 * FUNCTION  : Change the position of the party in the current map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 15.06.95 14:27
 * LAST      : 13.08.95 01:01
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT View_direction - View direction.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This functions offers a safe way of changing the position
 *              of the party. It is used by the Map Exit event.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Change_position(UNSHORT X, UNSHORT Y, UNSHORT View_direction)
{
	/* Store new coordinates and view direction */
	PARTY_DATA.X = X;
	PARTY_DATA.Y = Y;
	PARTY_DATA.View_direction = View_direction;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D */
		Initialize_3D_position();
	}
	else
	{
		/* 2D */
		Initialize_2D_position();
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Save_position
 * FUNCTION  : Save current player position.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.07.95 15:08
 * LAST      : 12.08.95 12:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Save_position(void)
{
	UNSHORT i;

	/* Save map position */
	Old_position.Map_X = PARTY_DATA.X;
	Old_position.Map_Y = PARTY_DATA.Y;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Save 3D parameters */
		for (i=0;i<3;i++)
		{
			Old_position.Data._3D_data.Player_X[i] = I3DM.Player_X[i];
			Old_position.Data._3D_data.Player_Z[i] = I3DM.Player_Z[i];
		}
		Old_position.Data._3D_data.Camera_angle = I3DM.Camera_angle;
	}
	else
	{
		/* 2D -> Save 2D parameters */
		Get_2D_camera_position(&Old_position.Data._2D_data.Camera_X,
		 &Old_position.Data._2D_data.Camera_Y);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Restore_position
 * FUNCTION  : Restore position.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 04.07.95 15:08
 * LAST      : 12.08.95 12:00
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Restore_position(void)
{
	UNSHORT i;

	/* Restore map position */
	PARTY_DATA.X = Old_position.Map_X;
	PARTY_DATA.Y = Old_position.Map_Y;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Save 3D parameters */
		for (i=0;i<3;i++)
		{
			I3DM.Player_X[i] = Old_position.Data._3D_data.Player_X[i];
			I3DM.Player_Z[i] = Old_position.Data._3D_data.Player_Z[i];
		}
		I3DM.Camera_angle = Old_position.Data._3D_data.Camera_angle;
	}
	else
	{
		/* 2D -> Save 2D parameters */
		Set_2D_camera_position(Old_position.Data._2D_data.Camera_X,
		 Old_position.Data._2D_data.Camera_Y);
	}
}

