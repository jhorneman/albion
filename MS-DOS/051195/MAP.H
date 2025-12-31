/************
 * NAME     : MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 26-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef MAP_H
#define MAP_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

/* Light status */
#define ALWAYS_LIGHT		(0)
#define ALWAYS_DARK		(1)
#define CHANGING_LIGHT	(2)

/* Map types */
#define CITY_MAP			(0)
#define DUNGEON_MAP		(1)
#define WILDERNESS_MAP	(2)
#define INTERIOR_MAP		(3)

/* Special map exit modes */
#define NO_MX_MODE					(0)
#define TELEPORT_MX_MODE			(1)
#define JUMP_MX_MODE					(2)
#define TRAPDOOR_UP_MX_MODE		(3)
#define TRAPDOOR_DOWN_MX_MODE		(4)
#define FALL_DOWN_MX_MODE			(5)

/* States */
#define STANDING_STATE			(0)
#define SITTING_NL_STATE  		(1)
#define SITTING_NR_STATE  		(2)
#define SITTING_E_STATE			(3)
#define SITTING_SL_STATE  		(4)
#define SITTING_SR_STATE  		(5)
#define SITTING_W_STATE			(6)
#define SLEEPING1_STATE			(7)
#define SWIMMING_STATE			(8)
#define CLIMBING_STATE			(9)
#define WADING_STATE				(10)
#define WALKING_GRASS_STATE	(11)
#define SLEEPING2_STATE			(12)

/* Raw key codes from SYSINTRN.H */
#define RAW_UP          (72)
#define RAW_DOWN        (80)
#define RAW_LEFT			(75)
#define RAW_RIGHT       (77)

#define RAWSHIFTL			(42)
#define RAWSHIFTR			(54)

/*
 ** Structure definitions **************************************************
 */

/* Map data header */
struct Map_data {
	UNSHORT Flags;
	UNBYTE Display_type;
	UNBYTE Music;
	UNBYTE Width;
	UNBYTE Height;
	union {
		struct {
			UNBYTE xIcon_set_nr;
		} x2D_map_info;
		struct {
			UNBYTE xLab_data_nr;
		} x3D_map_info;
	} xMap_info;
	UNBYTE Combat_background_nr;
	UNBYTE Colour_pal_nr;
	UNBYTE Animation_period;
};

#define ICON_SET_NR		xMap_info.x2D_map_info.xIcon_set_nr
#define LAB_DATA_NR		xMap_info.x3D_map_info.xLab_data_nr

/* Bit masks for Map_data.Flags */
#define LIGHT_STATUS			(0x03)
#define MAP_TYPE				(0x0C)
#define MAP_TYPE_B			(2)
#define PLANET_SPACESHIP	(1 << 4)
#define NEW_NPC_FORMAT		(1 << 13)
#define MORE_NPCS				(1 << 14)

/* Bit masks for 2D & 3D icon data */
#define WAVE_ANIM					(0x00000001)
#define ASYNCH_ANIM				(0x00000008)
#define RANDOM_ANIM				(0x00000010)
#define BLOCKED_TRAVELMODES	(0x0007F800)
#define BLOCKED_TRAVELMODES_B	(11)
#define STATE						(0x07800000)
#define STATE_B					(23)
#define COMBAT_BACKGROUND_NR	(0xF8000000)
#define COMBAT_BACKGROUND_B	(27)

/* Goto point data */
struct Goto_point {
	UNBYTE X, Y;
	UNBYTE Direction;
	UNBYTE Bit_nr;
	UNBYTE _pad[15];
};

/* Position data */
struct Position_data {
	UNSHORT Map_X, Map_Y;
	union {
		struct {
			UNLONG Camera_X, Camera_Y;
		} _2D_data;
		struct {
			UNSHORT Player_X[3];
			UNSHORT Player_Z[3];
			UNLONG Camera_angle;
		} _3D_data;
	} Data;
};

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Map_handle;
extern MEM_HANDLE Map_text_handle;

extern BOOLEAN _3D_map;
extern BOOLEAN Spaceship_map;
extern BOOLEAN Map_initialized;
extern BOOLEAN Map_display_initialized;
extern BOOLEAN Map_changed;
extern BOOLEAN Map_events_changed;
extern BOOLEAN Move_mode_flag;
extern BOOLEAN Moved;

extern UNSHORT Map_square_size;

extern UNSHORT Special_map_exit_mode;

extern UNSHORT NPC_reaction_radius;

extern struct Position_data Old_position;

extern struct BBPALETTE Day_map_palette;

extern UNSHORT Current_map_type;
extern UNSHORT Current_map_palette_nr;
extern UNSHORT Current_map_music_nr;
extern UNSHORT Current_map_light_state;
extern UNSHORT Map_width;
extern UNSHORT Map_height;
extern UNLONG Map_size;

extern UNLONG Map_layers_offset;
extern UNLONG Event_data_offset;
extern UNLONG Event_entry_offset;
extern UNLONG NPC_path_offset;
extern UNLONG Goto_point_offset;
extern UNLONG Event_automap_offset;
extern UNLONG Event_chain_offset;

extern UNSHORT Nr_goto_points;
extern UNSHORT Nr_event_blocks;

extern SISHORT Offsets4[4][2];
extern SISHORT Offsets8[8][2];
extern UNSHORT View_directions[3][3];
extern UNSHORT Move_directions[3][3];

extern SIBYTE pressedkeytab[];

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_map(void);
void Exit_map(void);

void Init_map_display(void);
void Exit_map_display(void);
void Update_map_display(void);

void Update_map_stuff(void);

void Init_map_palette(void);

void Reset_map_palette_blending(void);

void Colour_cycle_forward_2D(UNSHORT Start, UNSHORT Size);

void Set_music(UNSHORT Music_nr);

BOOLEAN Before_move(void);
void After_move(void);
void Time_after_move(void);
void Game_after_move(void);

void Execute_normal_map_events(void);

UNLONG Get_location_status(SISHORT X, SISHORT Y, UNSHORT NPC_index);

UNSHORT Seek_transport(UNSHORT Map_nr, SISHORT X, SISHORT Y);
void Create_transport(UNSHORT Map_nr, SISHORT X, SISHORT Y,
 UNSHORT Transport_type);

UNSHORT *Find_Goto_point_data_in_map(struct Map_data *Map_ptr);

BOOLEAN Vision_blocked(SISHORT X, SISHORT Y);

UNSHORT Get_combat_background_nr(void);

/* Position management functions */
void Change_position(UNSHORT X, UNSHORT Y, UNSHORT View_direction);
void Save_position(void);
void Restore_position(void);

#endif

