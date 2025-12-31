/************
 * NAME     : MAP.C
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 10-8-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/* includes */

#include <string.h>

#include <BBDEF.H>
#include <BBMEM.H>
#include <BBDSA.H>
#include <BBSYSTEM.H>

#include <HDOB.H>
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
#include <MUSIC.H>
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

/* defines */

#define MODIFICATIONS_PER_LIST	(100)

/* structure definitions */

/* Modification list entry */
struct Modification {
	UNBYTE X, Y;
	UNBYTE Type, Sub_type;
	UNSHORT Value;
	UNSHORT Map_nr;
};

/* Modification list */
struct Modification_list {
	MEM_HANDLE Next_list;
	struct Modification Modifications[MODIFICATIONS_PER_LIST];
};

/* prototypes */

void Get_map_data_offsets(struct Map_data *Map_ptr);

void Destroy_modification_list(MEM_HANDLE Handle);

/* Map pop-up menu functions */
void Map_PUM_evaluator(struct PUM *PUM);

void PUM_Map_Talk(UNLONG Data);
void PUM_Map_Examine(UNLONG Data);
void PUM_Map_Manipulate(UNLONG Data);
void PUM_Map_Use_item(UNLONG Data);
void PUM_Map_Take_item(UNLONG Data);

void PUM_Map_Change_transport(UNLONG Data);
void PUM_Map_Enter_automapper(UNLONG Data);
void PUM_Map_Build_camp(UNLONG Data);
void PUM_Map_Party_status(UNLONG Data);

/* global variables */

MEM_HANDLE Map_handle;
MEM_HANDLE Map_text_handle;

/* Global flags */
BOOLEAN _3D_map;
BOOLEAN Map_initialized = FALSE;
BOOLEAN Map_display_initialized = FALSE;
BOOLEAN Map_changed = FALSE;
BOOLEAN Map_events_changed = FALSE;
BOOLEAN Move_mode_flag = FALSE;
BOOLEAN Bumped;
BOOLEAN Moved;

UNSHORT Map_square_size = 1;

UNSHORT Special_map_exit_mode = NO_MX_MODE;

struct Position_data Old_position;

static UNSHORT Special_item_status = 0;

/* Data from the current map */
UNSHORT Current_map_type;
UNSHORT Current_map_palette_nr;
UNSHORT Current_map_music_nr;
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

UNLONG Map_text_0_subblock_offsets[GOTO_POINTS_PER_MAP + 1];

static UNLONG Animation_timer;
static UNSHORT Remaining_ticks;
static UNSHORT Animation_period;

static MEM_HANDLE First_modification_list = NULL;
static UNSHORT Nr_modifications;

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

/*	0xFFFF,	NORTH,	0xFFFF,
	WEST,		0xFFFF,	EAST,
	0xFFFF,	SOUTH,	0xFFFF */
};

/* Current map selection data */
struct Map_selection_data Current_map_selection_data;

/* Map pop-up menu entries */
static struct PUME Map_PUMEs[] = {
	{PUME_AUTO_CLOSE, 0, 6, PUM_Map_Talk},
	{PUME_AUTO_CLOSE, 0, 7, PUM_Map_Examine},
	{PUME_AUTO_CLOSE, 0, 8, PUM_Map_Manipulate},
	{PUME_AUTO_CLOSE, 0, 9, PUM_Map_Use_item},
	{PUME_AUTO_CLOSE, 0, 74, PUM_Map_Take_item},
	{PUME_NOT_SELECTABLE, 0, 0, NULL},
	{PUME_AUTO_CLOSE, 0, 11, PUM_Map_Change_transport},
	{PUME_AUTO_CLOSE, 0, 130, PUM_Map_Enter_automapper},
	{PUME_AUTO_CLOSE, 0, 12, PUM_Map_Build_camp},
	{PUME_AUTO_CLOSE, 0, 13, PUM_Map_Party_status}
};

/* Map pop-up menu */
struct PUM Map_PUM = {
	10,
	NULL,
	0,
	Map_PUM_evaluator,
	Map_PUMEs
};

/* Compass OPM data */
static MEM_HANDLE Compass_OPM_handle;
static struct OPM Compass_OPM;

/* Clock OPM data */
static MEM_HANDLE Clock_OPM_handle;
static struct OPM Clock_OPM;

/* Special item HDOB handles */
static UNSHORT Compass_HDOB_nr;
static UNSHORT Monster_eye_HDOB_nr;
static UNSHORT Clock_HDOB_nr;

/* Special item HDOBs */
static struct HDOB Compass_HDOB = {
	HDOB_MASK,
	0,
	7, 5,
	30, 29,
	1,
	0,
	NULL,
	0,
	NULL,
	NULL,
	0,
	0,
	NULL
};
static struct HDOB Monster_eye_HDOB = {
	HDOB_MASK,
	0,
	5, 40,
	32, 27,
	1,
	0,
	NULL,
	(UNLONG) Eye_symbols,
	NULL,
	NULL,
	0,
	0,
	NULL
};
static struct HDOB Clock_HDOB = {
	HDOB_MASK,
	0,
	5, 74,
	32, 25,
	1,
	0,
	NULL,
	0,
	NULL,
	NULL,
	0,
	0,
	NULL
};

/* Modification list file type */
static UNCHAR Modification_list_fname[] = "Modification list";

static struct File_type Modification_list_ftype = {
	NULL,
	0xFF,
	0,
	MEM_KILL_ALWAYS,
	Modification_list_fname
};

/* Ambient song numbers, indexed by song number */
static UNSHORT Ambient_song_numbers[] = {
	15,  6, 34,  5,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  7, 14, 11, 22,
	 0,  0, 22, 31,  8,  0, 11,  0,  0,  0,
	 0,  0,  0,  0
};

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_map
 * FUNCTION  : Initialize a new map
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 10.08.94 13:30
 * LAST      : 13.08.95 11:10
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
	UNSHORT Old_map_nr;
	UNBYTE *Ptr;

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

	/* Build sub-block catalogue of map text 0 */
	Ptr = MEM_Claim_pointer(Map_text_handle);

	Ptr = Find_text_block(Ptr, 0);

	Build_subblock_catalogue(Ptr, &(Map_text_0_subblock_offsets[0]),
	 GOTO_POINTS_PER_MAP + 1);

	MEM_Free_pointer(Map_text_handle);

	/* Analyze map data */
	Map_ptr = (struct Map_data *) MEM_Claim_pointer(Map_handle);

	/* Is it a 2D or a 3D map ? */
	if (Map_ptr->Display_type == 1)
		_3D_map = TRUE;
	else
		_3D_map = FALSE;

	/* Get the map type */
	Current_map_type = (Map_ptr->Flags & MAP_TYPE) >> MAP_TYPE_B;

	/* Get the palette number */
	Current_map_palette_nr = (UNSHORT) Map_ptr->Colour_pal_nr;

	/* Get the music number */
	Current_map_music_nr = (UNSHORT) Map_ptr->Music;

	/* Get the map dimensions */
	Map_width = (UNSHORT) Map_ptr->Width;
	Map_height = (UNSHORT) Map_ptr->Height;

	Map_size = Map_width * Map_height;

	/* Get animation speed */
	Animation_period = Map_ptr->Animation_period;
	if (!Animation_period)
		Animation_period = 5;

	/* Initialize time */
	Init_map_time(Map_ptr->Flags);

	/* Get map data offsets */
	Get_map_data_offsets(Map_ptr);

	MEM_Free_pointer(Map_handle);

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
	Make_modifications(PARTY_DATA.Map_nr);

	/* Indicate that the map has changed */
	Map_changed = TRUE;

	/* Catalogue event chains for certain triggermodes */
	Catalogue_event_chains(EVERY_STEP_TRIGGER);
	Catalogue_event_chains(NPC_TRIGGER);

	/* Switch located sound effects on */
	Located_sound_effects_on();

	/* Set map music */
	Set_map_music();

	/* Initialize special items */
	Init_special_items();

	/* Get current map number */
	Old_map_nr = PARTY_DATA.Map_nr;

	/* Execute map-init event chains */
	Execute_chains_in_map(MAP_INIT_TRIGGER);

	/* Still in the same map ? */
	if (Old_map_nr == PARTY_DATA.Map_nr)
	{
		/* Yes -> Initialize display */
		Init_display();

		/* 3D map ? */
		if (_3D_map)
		{
			/* Yes -> Initialize automap */
			Init_automap();

			/* Update at initial position */
			Update_automap();
		}

		/* Save position */
		Save_position();

		/* Handle first step */
		After_move();
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
 * LAST      : 28.07.95 18:13
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
	Located_sound_effects_off();

	/* Exit special items */
	Exit_special_items();

	/* Exit map module */
	Pop_module();

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
	MEM_Free_memory(Map_handle);

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
 * LAST      : 13.08.95 14:47
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
	BOOLEAN Result;

	/* Load map palette */
	Result = Load_palette(Current_map_palette_nr);
	if (!Result)
		return;

	/* Install map mouse pointer */
	Push_mouse(DEFAULT_MPTR);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_map_display
 * FUNCTION  : Exit map display.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 09.01.95 14:55
 * LAST      : 13.08.95 14:48
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
	/* Remove map mouse pointer */
	Pop_mouse();

	/* Display slab */
//	OPM_CopyOPMOPM(&Slab_OPM, &Main_OPM, 0, 0, 360, Panel_Y, 0, 0);
//	Switch_screens();
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
			Cycle_colours(Current_map_palette_nr);

			/* Update animations */
			Update_animation();

			/* Count down */
			while (Remaining_ticks > Animation_period)
				Remaining_ticks -= Animation_period;
		}
	}

	/* Update special items */
	Update_special_items();
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
 * NAME      : Set_map_music
 * FUNCTION  : Set map music.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 26.07.95 18:08
 * LAST      : 26.07.95 18:08
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Set_map_music(void)
{
	UNSHORT Ambient_song_nr;

	/* Play map music */
//	Play_song(Current_map_music_nr);

	/* Any music ? */
	if (Current_map_music_nr)
	{
		/* Yes -> Get appropriate ambient song number */
		Ambient_song_nr = Ambient_song_numbers[Current_map_music_nr - 1];

		/* Play it */
		Play_ambient_song(Ambient_song_nr);
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
 * LAST      : 06.09.94 11:45
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
	/* Save position */
	Save_position();

	return TRUE;
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
	Trigger_map_event_chain(PARTY_DATA.X, PARTY_DATA.Y, 0xFFFF,
	 NORMAL_TRIGGER, 0);

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
			Trigger_map_event_chain(PARTY_DATA.X + 1, PARTY_DATA.Y, 0xFFFF,
			 NORMAL_TRIGGER, 0);
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
				if (Wall_nr < Nr_of_walls)
				{
					/* Yes -> Find wall data */
					Ptr = (UNBYTE *) (Lab_ptr + 1)
					 + (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2
					 + (Nr_of_floors * sizeof(struct Floor_3D)) + 2
					 + (Nr_of_objects * sizeof(struct Object_3D)) + 2;
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
				if (Floor_nr < Nr_of_floors)
				{
					/* Yes -> Find floor data */
					Ptr = (UNBYTE *) (Lab_ptr + 1)
					 + (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2;
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
 * NAME      : Map_PUM_evaluator
 * FUNCTION  : Evaluate map pop-up menu.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 22.12.94 12:30
 * LAST      : 22.07.95 14:54
 * INPUTS    : struct PUM *PUM - Pointer to pop-up menu data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Map_PUM_evaluator(struct PUM *PUM)
{
	struct PUME *PUMES;
	UNLONG Distance;
	UNSHORT State;
	UNSHORT Trigger_modes;
	UNSHORT NPC_index;

	PUMES = PUM->PUME_list;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Was an NPC selected ? */
		if (Current_map_selection_data.NPC_index != 0xFFFF)
		{
			/* Yes -> Set pop-up menu title */
			PUM->Title = System_text_ptrs[513];
		}
		else
		{
			/* No -> Set pop-up menu title */
			PUM->Title = System_text_ptrs[14];
		}

		/* Get map selection data */
		State				= Current_map_selection_data.State;
		Trigger_modes	= Current_map_selection_data.Trigger_modes;
		Distance			= Current_map_selection_data.Distance;

		/* Speak trigger-mode set ? */
		if (Trigger_modes & (1 << SPEAK_TRIGGER))
		{
			/* Yes -> Is this too far away for talking ? */
			if (Distance > MAX_TALK_DISTANCE)
			{
				/* Yes -> Block PUM entry */
				PUMES[0].Flags |= PUME_BLOCKED;
				PUMES[0].Blocked_message_nr = 536;
			}
		}
		else
		{
			/* No -> Speaking is not possible */
			PUMES[0].Flags |= PUME_ABSENT;
		}

		/* Examine trigger-mode set ? */
		if (Trigger_modes & (1 << EXAMINE_TRIGGER))
		{
			/* Yes -> Is this too far away for examining ? */
			if (Distance > MAX_EXAMINE_DISTANCE)
			{
				/* Yes -> Block PUM entry */
				PUMES[1].Flags |= PUME_BLOCKED;
				PUMES[1].Blocked_message_nr = 536;
			}
		}
		else
		{
			/* No -> Examining is not possible */
			PUMES[1].Flags |= PUME_ABSENT;
		}

		/* Touch trigger-mode set ? */
		if (Trigger_modes & (1 << TOUCH_TRIGGER))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if (Distance > MAX_MANIPULATE_DISTANCE)
			{
				/* Yes -> Block PUM entry */
				PUMES[2].Flags |= PUME_BLOCKED;
				PUMES[2].Blocked_message_nr = 536;
			}
			else
			{
				/* No -> Blocked ? */
				if (State == BLOCKED_MAP_SELECTION)
				{
					/* Yes -> Block PUM entry */
					PUMES[2].Flags |= PUME_BLOCKED;
					PUMES[2].Blocked_message_nr = 537;
				}
			}
		}
		else
		{
			/* No -> Touching is not possible */
			PUMES[2].Flags |= PUME_ABSENT;
		}

		/* Use item trigger-mode set ? */
		if (Trigger_modes & (1 << USE_ITEM_TRIGGER))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if (Distance > MAX_MANIPULATE_DISTANCE)
			{
				/* Yes -> Block PUM entry */
				PUMES[3].Flags |= PUME_BLOCKED;
				PUMES[3].Blocked_message_nr = 536;
			}
			else
			{
				/* No -> Blocked ? */
				if (State == BLOCKED_MAP_SELECTION)
				{
					/* Yes -> Block PUM entry */
					PUMES[3].Flags |= PUME_BLOCKED;
					PUMES[3].Blocked_message_nr = 537;
				}
			}
		}
		else
		{
			/* No -> Using item is not possible */
			PUMES[3].Flags |= PUME_ABSENT;
		}

		/* Take trigger-mode set ? */
		if (Trigger_modes & (1 << TAKE_TRIGGER))
		{
			/* Yes -> Is this too far away for manipulating ? */
			if (Distance > MAX_MANIPULATE_DISTANCE)
			{
				/* Yes -> Block PUM entry */
				PUMES[4].Flags |= PUME_BLOCKED;
				PUMES[4].Blocked_message_nr = 536;
			}
			else
			{
				/* No -> Blocked ? */
				if (State == BLOCKED_MAP_SELECTION)
				{
					/* Yes -> Block PUM entry */
					PUMES[4].Flags |= PUME_BLOCKED;
					PUMES[4].Blocked_message_nr = 537;
				}
				else
				{
					/* No -> Is the active character overweight ? */
					if (Character_is_overweight(Active_char_handle))
					{
						/* Yes -> Block PUM entry */
						PUMES[4].Flags |= PUME_BLOCKED;
						PUMES[4].Blocked_message_nr = 489;
					}
					else
					{
						/* No -> Is there a free slot in the active character's
						 backpack ? */
						if (Char_inventory_full(Active_char_handle))
						{
							/* No -> Block PUM entry */
							PUMES[4].Flags |= PUME_BLOCKED;
							PUMES[4].Blocked_message_nr = 490;
						}
					}
				}
			}
		}
		else
		{
			/* No -> Taking is not possible */
			PUMES[4].Flags |= PUME_ABSENT;
		}

		/* Cannot change transportation */
		PUMES[6].Flags |= PUME_ABSENT;

		/* Can the party build a camp ? */

	}
	else
	{
		/* 2D -> Search for NPCs */
		NPC_index = Search_2D_NPC(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF);

		/* Is there an NPC here ? */
		if (NPC_index != 0xFFFF)
		{
			/* Yes -> Has event ? */
			if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
			{
				/* Yes -> Get triggermodes */
				Trigger_modes = VNPCs[NPC_index].Trigger_modes;
			}
			else
			{
				/* No -> Use standard "triggermodes" */
				Trigger_modes = (1 << SPEAK_TRIGGER);
			}

			/* Set pop-up menu title */
			PUM->Title = System_text_ptrs[513];
		}
		else
		{
			/* No -> Get trigger modes for this position */
			Trigger_modes = Get_event_triggers(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF);

			/* Set pop-up menu title */
			PUM->Title = System_text_ptrs[14];
		}

		/* Change PUM entries depending on triggermodes */
		if (!(Trigger_modes & (1 << SPEAK_TRIGGER)))
			PUMES[0].Flags |= PUME_ABSENT;

		if (!(Trigger_modes & (1 << EXAMINE_TRIGGER)))
			PUMES[1].Flags |= PUME_ABSENT;

		if (!(Trigger_modes & (1 << TOUCH_TRIGGER)))
			PUMES[2].Flags |= PUME_ABSENT;

		if (!(Trigger_modes & (1 << USE_ITEM_TRIGGER)))
			PUMES[3].Flags |= PUME_ABSENT;

		/* Take trigger-mode set ? */
		if (Trigger_modes & (1 << TAKE_TRIGGER))
		{
			/* Yes -> Is the active character overweight ? */
			if (Character_is_overweight(Active_char_handle))
			{
				/* Yes -> Block PUM entry */
				PUMES[4].Flags |= PUME_BLOCKED;
				PUMES[4].Blocked_message_nr = 489;
			}
			else
			{
				/* No -> Is there a free slot in the active character's
				 backpack ? */
				if (Char_inventory_full(Active_char_handle))
				{
					/* No -> Block PUM entry */
					PUMES[4].Flags |= PUME_BLOCKED;
					PUMES[4].Blocked_message_nr = 490;
				}
			}
		}
		else
		{
			/* No -> Taking is not possible */
			PUMES[4].Flags |= PUME_ABSENT;
		}

		/* Is the party on foot ? */
		if (PARTY_DATA.Travel_mode == ON_FOOT)
		{
			/* Yes -> Is there a transportation here ? */
			if (!(Seek_transport(PARTY_DATA.Map_nr, PARTY_DATA.X, PARTY_DATA.Y)))
				PUMES[6].Flags |= PUME_ABSENT;
		}
		else
		{
			/* No -> Can the party go on foot here ? */
			if (Get_location_status(PARTY_DATA.X, PARTY_DATA.Y, 0xFFFF) &
			 BLOCKED_TRAVELMODES)
			{
				/* Yes */
				PUMES[6].Flags |= PUME_ABSENT;
			}
		}

		/* Cannot enter automapper */
		PUMES[7].Flags |= PUME_ABSENT;

		/* Can the party build a camp ? */

	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Talk
 * FUNCTION  : Talk (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 10.08.95 16:56
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Talk(UNLONG Data)
{
	MEM_HANDLE NPC_char_handle;
	UNSHORT NPC_index;
	UNSHORT Selected_word;
	UNSHORT Char_file_type;
	UNSHORT State;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Get NPC index */
		NPC_index = Current_map_selection_data.NPC_index;
	}
	else
	{
		/* 2D -> Search for NPCs */
		NPC_index = Search_2D_NPC(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF);
	}

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger talk event for NPC */
			Trigger_NPC_event_chain(NPC_index, SPEAK_TRIGGER, 0);
		}
		else
		{
			/* No -> Is this a person ? */
			if ((VNPCs[NPC_index].NPC_type == PARTY_NPC) ||
			 (VNPCs[NPC_index].NPC_type == NPC_NPC))
			{
				/* Yes -> Is this person asleep ? */
				if (!_3D_map)
				{
					State = VNPCs[NPC_index].Data._2D_NPC_data.Move.State;
					if ((State == SLEEPING1_STATE) || (State == SLEEPING2_STATE))
					{
						/* Yes -> "Do not disturb!" */
						Set_permanent_message_nr(539);

						/* Exit */
						return;
					}
				}

				/* Turn towards NPC */
				Turn_towards_NPC(NPC_index);

				/* Long or short dialogue ? */
				if (VNPCs[NPC_index].Flags & NPC_SHORT_DIALOGUE)
				{
					/* Short -> Do short dialogue */
					Do_text_file_window(Map_text_handle, VNPCs[NPC_index].Number);
				}
				else
				{
					/* Long -> Determine character data type
					 (At this point I can be sure that only these two types
					 of NPC are possible.) */
					switch (VNPCs[NPC_index].NPC_type)
					{
						case PARTY_NPC:
						{
							Char_file_type = PARTY_CHAR;
							break;
						}
						case NPC_NPC:
						{
							Char_file_type = NPC_CHAR;
							break;
						}
					}

					/* Load NPC character data */
					NPC_char_handle = Load_subfile(Char_file_type,
					 VNPCs[NPC_index].Number);

					if (!NPC_char_handle)
					{
						Error(ERROR_FILE_LOAD);

						Exit_program();

						return;
					}

					/* Do any of the known languages of the active party
						member match with the NPC's ?  */
					if ((Get_known_languages(Active_char_handle) &
					 Get_known_languages(NPC_char_handle)) || Cheat_mode)
					{
						/* Yes -> Destroy NPC character data */
						MEM_Free_memory(NPC_char_handle);

						/* Do dialogue */
						/* (the first two NPC types match exactly with the
							  character types) */
						Do_Dialogue(NORMAL_DIALOGUE, (UNSHORT) VNPCs[NPC_index].NPC_type,
						 VNPCs[NPC_index].Number, NPC_index);
					}
					else
					{
						/* No -> Destroy NPC character data */
						MEM_Free_memory(NPC_char_handle);

						/* "No comprendo, signor." */
						Set_permanent_message_nr(540);
					}
				}
			}
		}
	}
	else
	{
		/* No -> Select a word from the word list */
		Selected_word = Select_word();

		/* Any selected ? */
		if (Selected_word != 0xFFFF)
		{
			/* Yes -> Trigger talk event for map */
			Trigger_map_event_chain(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF, SPEAK_TRIGGER,
			 Selected_word);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Examine
 * FUNCTION  : Examine (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 10.08.95 16:59
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Examine(UNLONG Data)
{
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Get NPC index */
		NPC_index = Current_map_selection_data.NPC_index;
	}
	else
	{
		/* 2D -> Search for NPCs */
		NPC_index = Search_2D_NPC(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF);
	}

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger examine event for NPC */
			Trigger_NPC_event_chain(NPC_index, EXAMINE_TRIGGER, 0);

			/* There was an NPC event */
			NPC_event = TRUE;
		}
	}

	/* Was there an NPC event ? */
	if (!NPC_event)
	{
		/* No -> Trigger examine event for map */
		Trigger_map_event_chain(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF, EXAMINE_TRIGGER, 0);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Manipulate
 * FUNCTION  : Manipulate (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 22.04.95 18:45
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Manipulate(UNLONG Data)
{
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Get NPC index */
		NPC_index = Current_map_selection_data.NPC_index;
	}
	else
	{
		/* 2D -> Search for NPCs */
		NPC_index = Search_2D_NPC(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF);
	}

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger touch event for NPC */
			Trigger_NPC_event_chain(NPC_index, TOUCH_TRIGGER, 0);

			/* There was an NPC event */
			NPC_event = TRUE;
		}
	}

	/* Was there an NPC event ? */
	if (!NPC_event)
	{
		/* No -> Trigger touch event for map */
		Trigger_map_event_chain(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF, TOUCH_TRIGGER, 0);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Use_item
 * FUNCTION  : Use item (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:56
 * LAST      : 10.08.95 17:00
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Use_item(UNLONG Data)
{
	struct Character_data *Char;
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;
	UNSHORT Selected_item_slot_index;
	UNSHORT Used_item_index;

	/* Select item to be used */
	Selected_item_slot_index = Select_character_item(Active_char_handle,
	 System_text_ptrs[430], NULL);

	/* Any selected ? */
	if (Selected_item_slot_index != 0xFFFF)
	{
		/* Yes -> Get selected item index */
		Char = (struct Character_data *) MEM_Claim_pointer(Active_char_handle);

		if (Selected_item_slot_index <= ITEMS_ON_BODY)
		{
			Used_item_index = Char->Body_items[Selected_item_slot_index - 1].Index;
		}
		else
		{
			Used_item_index = Char->Backpack_items[Selected_item_slot_index -
			 ITEMS_ON_BODY - 1].Index;
		}

		MEM_Free_pointer(Active_char_handle);

		/* 2D or 3D map ? */
		if (_3D_map)
		{
			/* 3D -> Get NPC index */
			NPC_index = Current_map_selection_data.NPC_index;
		}
		else
		{
			/* 2D -> Search for NPCs */
			NPC_index = Search_2D_NPC(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF);
		}

		/* Is there an NPC here ? */
		if (NPC_index != 0xFFFF)
		{
			/* Yes -> Has event ? */
			if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
			{
				/* Yes -> Trigger use item event for NPC */
				Trigger_NPC_event_chain(NPC_index, USE_ITEM_TRIGGER,
				 Used_item_index);

				/* There was an NPC event */
				NPC_event = TRUE;
			}
		}

		/* Was there an NPC event ? */
		if (!NPC_event)
		{
			/* No -> Trigger use item event for map */
			Trigger_map_event_chain(Current_map_selection_data.Map_X,
			 Current_map_selection_data.Map_Y, 0xFFFF, USE_ITEM_TRIGGER,
			 Used_item_index);
		}
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Take_item
 * FUNCTION  : Take item (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 31.01.95 10:46
 * LAST      : 10.08.95 17:00
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Take_item(UNLONG Data)
{
	BOOLEAN NPC_event = FALSE;
	UNSHORT NPC_index;

	/* 2D or 3D map ? */
	if (_3D_map)
	{
		/* 3D -> Get NPC index */
		NPC_index = Current_map_selection_data.NPC_index;
	}
	else
	{
		/* 2D -> Search for NPCs */
		NPC_index = Search_2D_NPC(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF);
	}

	/* Is there an NPC here ? */
	if (NPC_index != 0xFFFF)
	{
		/* Yes -> Has event ? */
		if (VNPCs[NPC_index].First_block_nr != 0xFFFF)
		{
			/* Yes -> Trigger take event for NPC */
			Trigger_NPC_event_chain(NPC_index, TAKE_TRIGGER, 0);

			/* There was an NPC event */
			NPC_event = TRUE;
		}
	}

	/* Was there an NPC event ? */
	if (!NPC_event)
	{
		/* No -> Trigger take event for map */
		Trigger_map_event_chain(Current_map_selection_data.Map_X,
		 Current_map_selection_data.Map_Y, 0xFFFF, TAKE_TRIGGER, 0);
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Change_transport
 * FUNCTION  : Change transport (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:57
 * LAST      : 28.12.94 16:57
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Change_transport(UNLONG Data)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Enter_automapper
 * FUNCTION  : Enter the automapper (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 16:30
 * LAST      : 03.05.95 16:30
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Enter_automapper(UNLONG Data)
{
	Enter_Automap();
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Build_camp
 * FUNCTION  : Build a camp (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 28.12.94 16:57
 * LAST      : 28.12.94 16:57
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Build_camp(UNLONG Data)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : PUM_Map_Party_status
 * FUNCTION  : Enter Party Status screen (map pop-up menu).
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     :
 * LAST      :
 * INPUTS    : UNLONG Data - Pop-up menu user data.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
PUM_Map_Party_status(UNLONG Data)
{
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_modifications
 * FUNCTION  : Initialize modification list.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 15:54
 * LAST      : 30.06.95 10:34
 * INPUTS    : UNBYTE *Data - Pointer to start of modification data.
 * RESULT    : UNBYTE * : Pointer to end of modification data.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

UNBYTE *
Init_modifications(UNBYTE *Data)
{
	struct Modification_list *List;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;

	/* Read number of modifications */
	Nr_modifications = *((UNSHORT *) Data);
	Data += 2;

	/* Allocate first modification list */
	Handle = MEM_Do_allocate(sizeof(struct Modification_list), 0,
	 &Modification_list_ftype);

	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Store handle */
	First_modification_list = Handle;

	/* Clear next list entry */
	List->Next_list = NULL;

	/* Copy complete lists */
	for (i=0;i<Nr_modifications / MODIFICATIONS_PER_LIST;i++)
	{
		/* Copy all modifications in this list */
		memcpy((UNBYTE *) &(List->Modifications[0]), Data,
		 MODIFICATIONS_PER_LIST * sizeof(struct Modification));

		Data += MODIFICATIONS_PER_LIST * sizeof(struct Modification);

		/* Allocate next modification list */
		Handle2 = MEM_Do_allocate(sizeof(struct Modification_list),
		 (UNLONG) i + 1, &Modification_list_ftype);

		/* Insert handle in current list */
		List->Next_list = Handle2;

		/* Switch to next list */
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Modification_list *) MEM_Claim_pointer(Handle);

		/* Clear next list entry */
		List->Next_list = NULL;
	}

	/* Any modifications left ? */
	if (Nr_modifications % MODIFICATIONS_PER_LIST)
	{
		/* Yes -> Copy these as well */
		memcpy(&(List->Modifications[0]), Data, (Nr_modifications %
		 MODIFICATIONS_PER_LIST) * sizeof(struct Modification));

		Data += (Nr_modifications % MODIFICATIONS_PER_LIST) *
		 sizeof(struct Modification);
	}

	MEM_Free_pointer(Handle);

	return Data;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_modification_list
 * FUNCTION  : Exit modification list.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 30.06.95 11:48
 * LAST      : 30.06.95 11:48
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_modification_list(void)
{
	Destroy_modification_list(First_modification_list);
}

void
Destroy_modification_list(MEM_HANDLE Handle)
{
	struct Modification_list *List;

	/* Get pointer to list */
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Is there a next list ? */
	if (List->Next_list)
	{
		/* Yes -> Destroy the next list first */
		Destroy_modification_list(List->Next_list);
	}

	/* Destroy this list */
	MEM_Free_pointer(Handle);
	MEM_Free_memory(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Prepare_modifications_for_saving
 * FUNCTION  : Prepare the modification list for saving.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 21.02.95 17:57
 * LAST      : 01.08.95 14:39
 * INPUTS    : None.
 * RESULT    : MEM_HANDLE : Memory handle of prepared modification list.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

MEM_HANDLE
Prepare_modifications_for_saving(void)
{
	struct Modification_list *List;
	MEM_HANDLE Output_handle;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;
	UNBYTE *Ptr;

	/* Allocate memory */
	Output_handle = MEM_Allocate_memory(2 + Nr_modifications *
	 sizeof(struct Modification));

	Ptr = MEM_Claim_pointer(Output_handle);

	/* Write number of modifications */
	*((UNSHORT *) Ptr) = Nr_modifications;
	Ptr += 2;

	/* Get first modification list */
	Handle = First_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Copy complete lists */
	for (i=0;i<Nr_modifications / MODIFICATIONS_PER_LIST;i++)
	{
		/* Copy all modifications in this list */
		memcpy(Ptr, &(List->Modifications[0]), MODIFICATIONS_PER_LIST *
		 sizeof(struct Modification));
		Ptr += MODIFICATIONS_PER_LIST * sizeof(struct Modification);

		/* Switch to next list */
		Handle2 = List->Next_list;
		MEM_Free_pointer(Handle);
		Handle = Handle2;
		List = (struct Modification_list *) MEM_Claim_pointer(Handle);
	}

	/* Any modifications left ? */
	if (Nr_modifications % MODIFICATIONS_PER_LIST)
	{
		/* Yes -> Copy these as well */
		memcpy(Ptr, &(List->Modifications[0]), (Nr_modifications %
		 MODIFICATIONS_PER_LIST) * sizeof(struct Modification));
	}

	MEM_Free_pointer(Handle);
	MEM_Free_pointer(Output_handle);

	return Output_handle;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Make_modifications
 * FUNCTION  : Make modifications to a map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 16:24
 * LAST      : 30.06.95 10:35
 * INPUTS    : UNSHORT Map_nr - Map number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Make_modifications(UNSHORT Map_nr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;

	/* Get first modification list */
	Handle = First_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all modifications */
	for (i=0;i<Nr_modifications;i++)
	{
		/* In the right map ? */
		if (List->Modifications[i].Map_nr == Map_nr)
		{
			/* Yes -> Make a modification */
			Do_change_icon(List->Modifications[i].X,
			 List->Modifications[i].Y,
			 List->Modifications[i].Type,
			 List->Modifications[i].Sub_type,
			 List->Modifications[i].Value);
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Clone_modifications
 * FUNCTION  : Clone modifications of one map to another map.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 08.03.95 13:25
 * LAST      : 30.06.95 10:35
 * INPUTS    : UNSHORT Source_map_nr - Source map number.
 *             UNSHORT Target_map_nr - Target map number.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This routine MUST deal with the fact that the modification
 *              list grows during the search.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Clone_modifications(UNSHORT Source_map_nr, UNSHORT Target_map_nr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle, Handle2;
	UNSHORT i;

	/* Get first modification list */
	Handle = First_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all the modifications */
	for (i=0;i<Nr_modifications;i++)
	{
		/* In the source map ? */
		if (List->Modifications[i].Map_nr == Source_map_nr)
		{
			/* Yes -> Make same modification in target map */
			Add_modification(List->Modifications[i].X,
			 List->Modifications[i].Y,
			 List->Modifications[i].Type,
			 List->Modifications[i].Sub_type,
			 List->Modifications[i].Value,
			 Target_map_nr);
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Add_modification
 * FUNCTION  : Add a new modification to the list.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 15:59
 * LAST      : 30.06.95 10:31
 * INPUTS    : UNSHORT X - X-coordinate.
 *             UNSHORT Y - Y-coordinate.
 *             UNSHORT Type - Modification type.
 *             UNSHORT Sub_type - Modification sub-type.
 *             UNSHORT Value - Modification value.
 *             UNSHORT Map_nr - Map number.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Add_modification(UNSHORT X, UNSHORT Y, UNSHORT Type, UNSHORT Sub_type,
 UNSHORT Value, UNSHORT Map_nr)
{
	struct Modification_list *List;
	MEM_HANDLE Handle, Handle2;
	BOOLEAN Exit = FALSE;
	UNSHORT i;

	/* Get first modification list */
	Handle = First_modification_list;
	List = (struct Modification_list *) MEM_Claim_pointer(Handle);

	/* Check all current entries */
	for (i=0;i<Nr_modifications;i++)
	{
		/* Is this the same entry as the one we're adding ? */
		if ((List->Modifications[i].X			== X) &&
		 (List->Modifications[i].Y				== Y) &&
		 (List->Modifications[i].Type			== Type) &&
		 (List->Modifications[i].Sub_type	== Sub_type) &&
		 (List->Modifications[i].Map_nr		== Map_nr))
		{
			/* Yes -> Same value ? */
			if (List->Modifications[i].Value != Value)
			{
				/* No -> Just change it */
				List->Modifications[i].Value = Value;
			}

			/* Break */
			Exit = TRUE;
			break;
		}

		/* End of this list ? */
		if ((i % MODIFICATIONS_PER_LIST) == MODIFICATIONS_PER_LIST - 1)
		{
			/* Yes -> Switch to next list */
			Handle2 = List->Next_list;
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);
		}
	}

	/* Do we still have to add ? */
	if (!Exit)
	{
		/* Yes -> Insert new modification */
		i = Nr_modifications % MODIFICATIONS_PER_LIST;

		List->Modifications[i].X			= X;
		List->Modifications[i].Y			= Y;
		List->Modifications[i].Type		= Type;
		List->Modifications[i].Sub_type	= Sub_type;
		List->Modifications[i].Value		= Value;
		List->Modifications[i].Map_nr		= Map_nr;

		/* Count up */
		Nr_modifications++;

		/* Time for a new list ? */
		if (!(Nr_modifications % MODIFICATIONS_PER_LIST))
		{
			/* Yes -> Allocate next modification list */
			Handle2 = MEM_Do_allocate(sizeof(struct Modification_list),
			 (UNLONG) (Nr_modifications / MODIFICATIONS_PER_LIST),
			 &Modification_list_ftype);

			/* Insert handle in current list */
			List->Next_list = Handle2;

			/* Switch to next list */
			MEM_Free_pointer(Handle);
			Handle = Handle2;
			List = (struct Modification_list *) MEM_Claim_pointer(Handle);

			/* Clear next list entry */
			List->Next_list = NULL;
		}
	}

	MEM_Free_pointer(Handle);
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Do_change_icon
 * FUNCTION  : Change an icon.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 29.12.94 16:41
 * LAST      : 02.08.95 14:04
 * INPUTS    : SISHORT X - X-coordinate.
 *             SISHORT Y - Y-coordinate.
 *             UNSHORT Type - Modification type.
 *             UNSHORT Sub_type - Modification sub-type.
 *             UNSHORT Value - Modification value.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - The icon is changed in the current map.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Do_change_icon(SISHORT X, SISHORT Y, UNSHORT Type, UNSHORT Sub_type,
 UNSHORT Value)
{
	UNBYTE *Map_data_ptr;

	/* Exit if the map isn't initialized */
	if (!Map_initialized)
		return;

	/* Get map data */
	Map_data_ptr = MEM_Claim_pointer(Map_handle);

	/* Which change type ? */
	switch (Type)
	{
		/* 2D underlay */
		case UNDERLAY_2D_CHANGE:
		{
			struct Square_2D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 2D map ? */
			if (!_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->m[1] &= 0xF0;
				Map_ptr->m[1] |= (UNBYTE) ((Value & 0x0F00) >> 8);
				Map_ptr->m[2] = (UNBYTE) (Value & 0x00FF);

				/* Update map area */
				Update_map_area(X, Y, 1, 1);
			}
			break;
		}
		/* 2D overlay */
		case OVERLAY_2D_CHANGE:
		{
			struct Square_2D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 2D map ? */
			if (!_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->m[0] = (UNBYTE) ((Value & 0x0FF0) >> 4);
				Map_ptr->m[1] &= 0x0F;
				Map_ptr->m[1] |= (UNBYTE) ((Value & 0x000F) << 4);

				/* Update map area */
				Update_map_area(X, Y, 1, 1);
			}
			break;
		}
		/* 3D wall or object group */
		case NORMAL_3D_CHANGE:
		{
			struct Square_3D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->Wall_layer = (UNBYTE) Value;
			}
			break;
		}
		/* 3D floor */
		case FLOOR_3D_CHANGE:
		{
			struct Square_3D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->Floor_layer = (UNBYTE) Value;
			}
			break;
		}
		/* 3D ceiling */
		case CEILING_3D_CHANGE:
		{
			struct Square_3D *Map_ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* 3D map ? */
			if (_3D_map)
			{
				/* Yes -> Change icon */
				Map_ptr = (struct Square_3D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);

				Map_ptr->Ceiling_layer = (UNBYTE) Value;
			}
			break;
		}
		/* NPC movement mode */
		case NPC_MOVE_CHANGE:
		{
			/* Exit if the NPC index is illegal */
			if ((X < 0) || (X >= NPCS_PER_MAP))
				break;

			/* Exit if the new movement mode is absolute or relative path */
			if ((Value == ABS_PATH_MOVEMENT) || (Value == REL_PATH_MOVEMENT))
				break;

			/* Change NPC movement mode */
			VNPCs[X].Movement_type = Value;

			break;
		}
		/* NPC graphics */
		case NPC_GFX_CHANGE:
		{
			/* Exit if the NPC index is illegal */
			if ((X < 0) || (X >= NPCS_PER_MAP))
				break;

			/* 2D or 3D map ? */
			if (_3D_map)
			{
				/* 3D map -> Change NPC graphics */
				VNPCs[X].Graphics_nr = Value;
			}
			else
			{
				/* 2D map -> Exit current graphics */
				Exit_2D_NPC_graphics(X);

				/* Change NPC graphics */
				VNPCs[X].Graphics_nr = Value;

				/* Init new graphics */
				Init_2D_NPC_graphics(X);
			}

			break;
		}
		/* Event entry */
		case EVENT_ENTRY_CHANGE:
		{
			struct Map_event_entry *Entry_data;
			UNSHORT Nr_entries;
			UNSHORT i;
			UNBYTE *Ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* Get pointer to event entry data */
			Ptr = Map_data_ptr + Event_entry_offset;

			/* Find the event entry-list for the desired Y-coordinate */
			for (i=0;i<Y;i++)
			{
				Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
			}
			Nr_entries = *((UNSHORT *) Ptr);
			Ptr += 2;
			Entry_data = (struct Map_event_entry *) Ptr;

			/* Search the event entries for the desired X-coordinate */
			for (i=0;i<Nr_entries;i++)
			{
				/* Right X-coordinate ? */
				if (Entry_data[i].X == X)
				{
					/* Yes -> Change event entry */
					Entry_data[i].First_block_nr = Value;

					/* Indicate that the map events have changed */
					Map_events_changed = TRUE;

					break;
				}
			}
			break;
		}
		/* Replace 2D cut-map block */
		case BLOCK_2D_REPLACE:
		{
			struct Square_2D *Map_ptr;
			struct Square_2D *Block;
			struct Cut_block_2D *Block_ptr;
			UNSHORT Underlay_nr;
			UNSHORT Overlay_nr;
			UNSHORT B1, B2, B3;
			UNSHORT i, j;
 			UNBYTE *Ptr;

			/* 2D map ? */
			if (!_3D_map)
			{
				/* Yes -> Find cut-map block */
				Ptr = MEM_Claim_pointer(Blocklist_handle);

				for(i=0;i<Value;i++)
				{
					Block_ptr = (struct Cut_block_2D *) Ptr;
					Ptr += sizeof(struct Cut_block_2D) + (Block_ptr->Width *
					 Block_ptr->Height * sizeof(struct Square_2D));
				}
				Block_ptr = (struct Cut_block_2D *) Ptr;

				/* Update map area */
				Update_map_area(X, Y, Block_ptr->Width, Block_ptr->Height);

				/* Change icons */
				Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);
				Block = (struct Square_2D *) (Block_ptr + 1);

				for (i=0;i<Block_ptr->Height;i++)
				{
					for (j=0;j<Block_ptr->Width;j++)
					{
						/* Are the coordinates inside of the map ? */
						if ((X > 0) && (X <= Map_width) &&
						 (Y > 0) && (Y <= Map_height))
						{
							/* Yes -> Read bytes from block */
							B1 = (UNSHORT) Block->m[0];
							B2 = (UNSHORT) Block->m[1];
							B3 = (UNSHORT) Block->m[2];

							/* Build overlay and underlay number */
							Underlay_nr = ((B2 & 0x0F) << 8) | B3;
							Overlay_nr = (B1 << 4) | (B2 >> 4);

							/* Copy underlays ? */
							if (Sub_type & 1)
							{
								/* Yes -> Put in map */
								Map_ptr->m[1] &= 0xF0;
								Map_ptr->m[1] |= (UNBYTE) ((Underlay_nr & 0x0F00) >> 8);
								Map_ptr->m[2] = (UNBYTE) (Underlay_nr & 0x00FF);
							}

							/* Copy overlays ? */
							if (Sub_type & 2)
							{
								/* Yes -> Put in map */
								Map_ptr->m[0] = (UNBYTE) ((Overlay_nr & 0x0FF0) >> 4);
								Map_ptr->m[1] &= 0x0F;
								Map_ptr->m[1] |= (UNBYTE) ((Overlay_nr & 0x000F) << 4);
							}
						}

						/* Next X-coordinate */
						X++;
						Map_ptr++;
						Block++;
					}
					/* Next Y-coordinate */
					X -= Block_ptr->Width;
					Y++;
					Map_ptr += Map_width - Block_ptr->Width;
				}

				MEM_Free_pointer(Blocklist_handle);
			}
			break;
		}
		/* Mix 2D cut-map block */
		case BLOCK_2D_MIX:
		{
			struct Square_2D *Map_ptr;
			struct Square_2D *Block;
			struct Cut_block_2D *Block_ptr;
			UNSHORT Underlay_nr;
			UNSHORT Overlay_nr;
			UNSHORT B1, B2, B3;
			UNSHORT i, j;
			UNBYTE *Ptr;

			/* 2D map ? */
			if (!_3D_map)
			{
				/* Yes -> Find cut-map block */
				Ptr = MEM_Claim_pointer(Blocklist_handle);

				for(i=0;i<Value;i++)
				{
					Block_ptr = (struct Cut_block_2D *) Ptr;
					Ptr += sizeof(struct Cut_block_2D) + (Block_ptr->Width *
					 Block_ptr->Height * sizeof(struct Square_2D));
				}
				Block_ptr = (struct Cut_block_2D *) Ptr;

				/* Update map area */
				Update_map_area(X, Y, Block_ptr->Width, Block_ptr->Height);

				/* Change icons */
				Map_ptr = (struct Square_2D *)(Map_data_ptr + Map_layers_offset);
				Map_ptr += X - 1 + ((Y - 1) * Map_width);
				Block = (struct Square_2D *) (Block_ptr + 1);

				for (i=0;i<Block_ptr->Height;i++)
				{
					for (j=0;j<Block_ptr->Width;j++)
					{
						/* Are the coordinates inside of the map ? */
						if ((X > 0) && (X <= Map_width) &&
						 (Y > 0) && (Y <= Map_height))
						{
							/* Yes -> Read bytes from block */
							B1 = (UNSHORT) Block->m[0];
							B2 = (UNSHORT) Block->m[1];
							B3 = (UNSHORT) Block->m[2];

							/* Build overlay and underlay number */
							Underlay_nr = ((B2 & 0x0F) << 8) | B3;
							Overlay_nr = (B1 << 4) | (B2 >> 4);

							/* Any underlay / copy underlays ? */
							if ((Underlay_nr > 1) && (Sub_type & 1))
							{
								/* Yes -> Put in map */
								Map_ptr->m[1] &= 0xF0;
								Map_ptr->m[1] |= (UNBYTE) ((Underlay_nr & 0x0F00) >> 8);
								Map_ptr->m[2] = (UNBYTE) (Underlay_nr & 0x00FF);
							}

							/* Any overlay / copy overlays ? */
							if ((Overlay_nr > 1) && (Sub_type & 2))
							{
								/* Yes -> Put in map */
								Map_ptr->m[0] = (UNBYTE) ((Overlay_nr & 0x0FF0) >> 4);
								Map_ptr->m[1] &= 0x0F;
								Map_ptr->m[1] |= (UNBYTE) ((Overlay_nr & 0x000F) << 4);
							}
						}

						/* Next X-coordinate */
						X++;
						Map_ptr++;
						Block++;
					}
					/* Next Y-coordinate */
					X -= Block_ptr->Width;
					Y++;
					Map_ptr += Map_width - Block_ptr->Width;
				}

				MEM_Free_pointer(Blocklist_handle);
			}
			break;
		}
		/* Trigger mode */
		case TRIGGER_MODE_CHANGE:
		{
			struct Map_event_entry *Entry_data;
			UNSHORT Nr_entries;
			UNSHORT i;
			UNBYTE *Ptr;

			/* Exit if the coordinates lie outside of the map */
			if ((X < 1) || (X > Map_width) || (Y < 1) || (Y > Map_height))
				break;

			/* Get pointer to event entry data */
			Ptr = Map_data_ptr + Event_entry_offset;

			/* Find the event entry-list for the desired Y-coordinate */
			for (i=0;i<Y;i++)
			{
				Ptr += (*((UNSHORT *) Ptr) * sizeof(struct Map_event_entry)) + 2;
			}
			Nr_entries = *((UNSHORT *) Ptr);
			Ptr += 2;
			Entry_data = (struct Map_event_entry *) Ptr;

			/* Search the event entries for the desired X-coordinate */
			for (i=0;i<Nr_entries;i++)
			{
				/* Right X-coordinate ? */
				if (Entry_data[i].X == X)
				{
					/* Yes -> Change trigger modes */
					Entry_data[i].Trigger_modes = Value;

					/* Indicate that the map events have changed */
					Map_events_changed = TRUE;

					break;
				}
			}
			break;
		}
	}
	MEM_Free_pointer(Map_handle);
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
	for (i=0;i<TRANSPORTS_MAX;i++)
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
	for (i=0;i<TRANSPORTS_MAX;i++)
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
			if (Wall_nr < Nr_of_walls)
			{
				/* Yes -> Find wall data */
				Ptr = (UNBYTE *) (Lab_ptr + 1)
				 + (Nr_of_object_groups * sizeof(struct Object_group_3D)) + 2
				 + (Nr_of_floors * sizeof(struct Floor_3D)) + 2
				 + (Nr_of_objects * sizeof(struct Object_3D)) + 2;
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
 * LAST      : 29.07.95 19:57
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
	Combat_background_nr = (Get_location_status(PARTY_DATA.X,
	 PARTY_DATA.Y, 0xFFFF) & COMBAT_BACKGROUND_NR) >> COMBAT_BACKGROUND_B;

	/* Any ? */
	if (!Combat_background_nr)
	{
		/* No -> Get combat background from map data */
		Map_data = (struct Map_data *) MEM_Claim_pointer(Map_handle);
		Combat_background_nr = (UNSHORT) Map_data->Combat_background_nr;
		MEM_Free_pointer(Map_handle);

		/* Any ? */
		if (!Combat_background_nr)
		{
			/* No -> Oops! */
			Combat_background_nr = 1;
		}
	}

	return Combat_background_nr;
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Init_special_items
 * FUNCTION  : Initialize special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 14:51
 * LAST      : 03.05.95 14:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Init_special_items(void)
{
	BOOLEAN Result;
	UNBYTE *Ptr;

	/* Compass initialized ? */
	if (!(Special_item_status & COMPASS_INITIALIZED))
	{
		/* No -> Allocate memory for compass graphics */
		Compass_OPM_handle = MEM_Do_allocate(30 * 29, (UNLONG) &Compass_OPM,
		 &OPM_ftype);
		if (!Compass_OPM_handle)
		{
			Error(ERROR_OUT_OF_MEMORY);
			return;
		}

		/* Clear memory */
		MEM_Clear_memory(Compass_OPM_handle);

		/* Insert handle in HDOB data */
		Compass_HDOB.Graphics_handle = Compass_OPM_handle;

		/* Create a new OPM */
		Ptr = MEM_Claim_pointer(Compass_OPM_handle);
		Result = OPM_New(30, 29, 1, &Compass_OPM, Ptr);
		MEM_Free_pointer(Compass_OPM_handle);

		/* Success ? */
		if (!Result)
		{
			/* No -> Free OPM memory */
			MEM_Free_memory(Compass_OPM_handle);
			Compass_OPM_handle = NULL;

			/* Report error */
			Error(ERROR_NO_OPM);

			return;
		}

		/* Indicate compass was initialized */
		Special_item_status |= COMPASS_INITIALIZED;
	}

	/* Clock initialized ? */
	if (!(Special_item_status & CLOCK_INITIALIZED))
	{
		/* No -> Allocate memory for clock graphics */
		Clock_OPM_handle = MEM_Do_allocate(32 * 25, (UNLONG) &Clock_OPM,
		 &OPM_ftype);
		if (!Clock_OPM_handle)
		{
			Error(ERROR_OUT_OF_MEMORY);
			return;
		}

		/* Clear memory */
		MEM_Clear_memory(Clock_OPM_handle);

		/* Insert handle in HDOB data */
		Clock_HDOB.Graphics_handle = Clock_OPM_handle;

		/* Create a new OPM */
		Ptr = MEM_Claim_pointer(Clock_OPM_handle);
		Result = OPM_New(32, 25, 1, &Clock_OPM, Ptr);
		MEM_Free_pointer(Clock_OPM_handle);

		/* Success ? */
		if (!Result)
		{
			/* No -> Free OPM memory */
			MEM_Free_memory(Clock_OPM_handle);
			Clock_OPM_handle = NULL;

			/* Report error */
			Error(ERROR_NO_OPM);

			return;
		}

		/* Indicate clock was initialized */
		Special_item_status |= CLOCK_INITIALIZED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Exit_special_items
 * FUNCTION  : Exit special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 14:51
 * LAST      : 03.05.95 14:51
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Exit_special_items(void)
{
	/* Compass initialized ? */
	if (Special_item_status & COMPASS_INITIALIZED)
	{
		/* Yes -> Delete compass OPM and free memory */
		OPM_Del(&Compass_OPM);
		MEM_Free_memory(Compass_OPM_handle);

		/* Indicate compass is no longer initialized */
		Special_item_status &= ~COMPASS_INITIALIZED;
	}

	/* Clock initialized ? */
	if (Special_item_status & CLOCK_INITIALIZED)
	{
		/* Yes -> Delete clock OPM and free memory */
		OPM_Del(&Clock_OPM);
		MEM_Free_memory(Clock_OPM_handle);

		/* Indicate clock is no longer initialized */
		Special_item_status &= ~CLOCK_INITIALIZED;
	}
}

/*
 ******************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : Update_special_items
 * FUNCTION  : Update special items.
 * FILE      : MAP.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 03.05.95 15:30
 * LAST      : 04.05.95 14:54
 * INPUTS    : None.
 * RESULT    : None.
 * BUGS      : No known.
 * SEE ALSO  :
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */

void
Update_special_items(void)
{
	static UNSHORT Compass_bit_frame = 0;
	UNSHORT Special_item_flags;
	UNSHORT Index;

	#if FALSE
	/* Cheat mode on ? */
	if (Cheat_mode)
	{
		/* Yes -> Party has all special items */
		Special_item_flags = 0xFFFF;
	}
	else
	{
		/* No -> Get special item flags */
		Special_item_flags = PARTY_DATA.Special_item_flags;
	}
	#endif

	Special_item_flags = PARTY_DATA.Special_item_flags;

	/* Compass initialized ? */
	if (Special_item_status & COMPASS_INITIALIZED)
	{
		/* Yes -> Does the party have the compass / 3D map ? */
		if ((Special_item_flags & COMPASS_ITEM_FLAG) && _3D_map)
		{
			/* Yes -> Re-build compass */
			Put_unmasked_block(&Compass_OPM, 0, 0, 30, 29, &Compass_symbol[0]);

			/* Get angle position index */
			Index = ((((0 - (I3DM.Camera_angle / 65536)) - (ANGLE_STEPS / 120)) &
			 (ANGLE_STEPS - 1)) / (ANGLE_STEPS / 60) + 1) % 60;

			/* Draw compass bit */
			Put_masked_block(&Compass_OPM, Compass_bit_coordinates[Index][0],
			 Compass_bit_coordinates[Index][1], 6, 6, &Compass_bit[0] +
			 Compass_bit_frame * 36);

			/* Update compass bit animation frame */
			Compass_bit_frame = (Compass_bit_frame + 1) & 0x0007;

			/* Compass already visible ? */
			if (!(Special_item_status & COMPASS_VISIBLE))
			{
				/* No -> Add compass HDOB */
				Compass_HDOB_nr = Add_HDOB(&Compass_HDOB);

				/* Indicate the compass is now visible */
				Special_item_status |= COMPASS_VISIBLE;
			}
		}
		else
		{
			/* No -> Compass visible ? */
			if (Special_item_status & COMPASS_VISIBLE)
			{
				/* Yes -> Remove compass HDOB */
				Delete_HDOB(Compass_HDOB_nr);

				/* Indicate the compass is no longer visible */
				Special_item_status &= ~COMPASS_VISIBLE;
			}
		}
	}

	/* Does the party have the monster eye / 3D map ? */
	if ((Special_item_flags & MONSTER_EYE_ITEM_FLAG) && _3D_map)
	{
		/* Yes -> Monster eye already visible ? */
		if (!(Special_item_status & MONSTER_EYE_VISIBLE))
		{
			/* No -> Add monster eye HDOB */
			Monster_eye_HDOB_nr = Add_HDOB(&Monster_eye_HDOB);

			/* Indicate the monster eye is now visible */
			Special_item_status |= MONSTER_EYE_VISIBLE;
		}

		/* Is a monster watching ? */
		if (Monster_is_watching)
		{
			/* Yes -> Open monster eye */
			Monster_eye_HDOB.Graphics_offset = (UNLONG) (&Eye_symbols[0] +
			 32 * 27);
		}
		else
		{
			/* No -> Close monster eye */
			Monster_eye_HDOB.Graphics_offset = (UNLONG) (&Eye_symbols[0]);
		}

		/* Change monster eye HDOB */
		Change_HDOB(Monster_eye_HDOB_nr, &Monster_eye_HDOB);
	}
	else
	{
		/* No -> Monster eye visible ? */
		if (Special_item_status & MONSTER_EYE_VISIBLE)
		{
			/* Yes -> Remove monster_eye HDOB */
			Delete_HDOB(Monster_eye_HDOB_nr);

			/* Indicate the monster_eye is no longer visible */
			Special_item_status &= ~MONSTER_EYE_VISIBLE;
		}
	}

	/* Clock initialized ? */
	if (Special_item_status & CLOCK_INITIALIZED)
	{
		/* Yes -> Does the party have the clock ? */
		if (Special_item_flags & CLOCK_ITEM_FLAG)
		{
			/* Yes -> Re-build clock */
			Put_unmasked_block(&Clock_OPM, 0, 0, 32, 25, &Clock_symbol[0]);

			/* Draw first digit of current hour */
			Put_masked_block(&Clock_OPM, 2, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Hour / 10) * 42);

			/* Draw second digit of current hour */
			Put_masked_block(&Clock_OPM, 8, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Hour % 10) * 42);

			/* Draw first digit of current minute */
			Put_masked_block(&Clock_OPM, 16, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Minute / 10) * 42);

			/* Draw second digit of current minute */
			Put_masked_block(&Clock_OPM, 22, 10, 6, 7, &Time_font[0] +
			 (PARTY_DATA.Minute % 10) * 42);

			/* Clock already visible ? */
			if (!(Special_item_status & CLOCK_VISIBLE))
			{
				/* No -> Add clock HDOB */
				Clock_HDOB_nr = Add_HDOB(&Clock_HDOB);

				/* Indicate the clock is now visible */
				Special_item_status |= CLOCK_VISIBLE;
			}
		}
		else
		{
			/* No -> Clock visible ? */
			if (Special_item_status & CLOCK_VISIBLE)
			{
				/* Yes -> Remove clock HDOB */
				Delete_HDOB(Clock_HDOB_nr);

				/* Indicate the clock is no longer visible */
				Special_item_status &= ~CLOCK_VISIBLE;
			}
		}
	}
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

