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

#include <NPCS.H>

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
#define TRAPDOOR_UP_MX_MODE		(1)
#define TRAPDOOR_DOWN_MX_MODE		(2)
#define FALL_DOWN_MX_MODE			(3)

/* States */
#define STANDING_STATE			(0)
#define SITTING_NL_STATE  		(1)
#define SITTING_NR_STATE  		(2)
#define SITTING_E_STATE			(3)
#define SITTING_SL_STATE  		(4)
#define SITTING_SR_STATE  		(5)
#define SITTING_W_STATE			(6)
#define SLEEPING_STATE			(7)
#define SWIMMING_STATE			(8)
#define CLIMBING_STATE			(9)
#define WADING_STATE				(10)
#define WALKING_GRASS_STATE	(11)

/* Icon change / modification types */
#define UNDERLAY_2D_CHANGE		(0)
#define OVERLAY_2D_CHANGE		(1)
#define NORMAL_3D_CHANGE		(2)
#define FLOOR_3D_CHANGE			(3)
#define CEILING_3D_CHANGE		(4)
#define NPC_MOVE_CHANGE			(5)
#define NPC_GFX_CHANGE			(6)
#define EVENT_ENTRY_CHANGE		(7)
#define BLOCK_2D_REPLACE		(8)
#define BLOCK_2D_MIX				(9)
#define TRIGGER_MODE_CHANGE	(10)

/* Special item status bits */
#define COMPASS_INITIALIZED	(1 << 0)
#define COMPASS_VISIBLE			(1 << 1)
#define CLOCK_INITIALIZED		(1 << 2)
#define CLOCK_VISIBLE			(1 << 3)
#define MONSTER_EYE_VISIBLE	(1 << 4)

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
			UNBYTE xIcon_set_1_nr;
			UNBYTE xIcon_set_2_nr;
		} x2D_map_info;
		struct {
			UNBYTE xLab_data_nr;
			UNBYTE xpad01;
		} x3D_map_info;
	} xMap_info;
	UNBYTE Colour_pal_nr;
	UNBYTE Animation_period;
};

#define ICON_SET_1_NR	xMap_info.x2D_map_info.xIcon_set_1_nr
#define ICON_SET_2_NR	xMap_info.x2D_map_info.xIcon_set_2_nr
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
	struct Modification Modifications[100];
};

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Map_handle;
extern MEM_HANDLE Map_text_handle;

extern BOOLEAN _3D_map;
extern BOOLEAN Map_initialized;
extern BOOLEAN Map_changed;
extern BOOLEAN Map_events_changed;
extern BOOLEAN Move_mode_flag;

extern UNSHORT Special_map_exit_mode;

extern UNSHORT Current_map_type;
extern UNSHORT Current_map_palette_nr;
extern UNSHORT Current_map_music_nr;
extern UNSHORT Map_width;
extern UNSHORT Map_height;
extern UNLONG Map_size;

extern UNSHORT Map_selected_X, Map_selected_Y;

extern UNLONG Map_layers_offset;
extern UNLONG Event_data_offset;
extern UNLONG Event_entry_offset;
extern UNLONG NPC_path_offset;
extern UNLONG Goto_point_offset;
extern UNLONG Event_automap_offset;
extern UNLONG Event_chain_offset;

extern UNSHORT Nr_goto_points;
extern UNSHORT Nr_event_blocks;

extern UNLONG Map_text_0_sub_block_offsets[];

extern SISHORT Offsets4[4][2];
extern SISHORT Offsets8[8][2];
extern UNSHORT View_directions[3][3];
extern UNSHORT Move_directions[3][3];

extern struct PUM Map_PUM;

/*
 ** Prototypes *************************************************************
 */

BOOLEAN Init_map(void);
void Exit_map(void);

void Init_map_display(void);
void Exit_map_display(void);

void Update_map_stuff(void);

void Colour_cycle_forward(UNSHORT Start, UNSHORT Size);

BOOLEAN Before_move(void);
void After_move(void);
void Time_after_move(void);
void Game_after_move(void);

UNLONG Get_location_status(SISHORT X, SISHORT Y, UNSHORT NPC_index);

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

void Init_modifications(UNBYTE *Data);
MEM_HANDLE Prepare_modifications_for_saving(void);
void Make_modifications(UNSHORT Map_nr);
void Clone_modifications(UNSHORT Source_map_nr, UNSHORT Target_map_nr);
void Add_modification(UNSHORT X, UNSHORT Y, UNSHORT Type, UNSHORT Sub_type,
 UNSHORT Value, UNSHORT Map_nr);
void Do_change_icon(SISHORT X, SISHORT Y, UNSHORT Type, UNSHORT Sub_type,
 UNSHORT Value);

UNSHORT Seek_transport(UNSHORT Map_nr, SISHORT X, SISHORT Y);
void Create_transport(UNSHORT Map_nr, SISHORT X, SISHORT Y,
 UNSHORT Transport_type);

UNSHORT *Find_Goto_point_data_in_map(struct Map_data *Map_ptr);

BOOLEAN Vision_blocked(SISHORT X, SISHORT Y);

void Init_special_items(void);
void Exit_special_items(void);
void Update_special_items(void);

#endif

