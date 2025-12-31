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

#define NPCS_PER_MAP		(32)

/* Light status */
#define ALWAYS_LIGHT		(0)
#define ALWAYS_DARK		(1)
#define CHANGING_LIGHT	(2)

/* Map types */
#define CITY_MAP			(0)
#define DUNGEON_MAP		(1)
#define WILDERNESS_MAP	(2)
#define INTERIOR_MAP		(3)

/* NPC types */
#define PARTY_NPC			(0)
#define NPC_NPC			(1)
#define MONSTER_NPC		(2)
#define OBJECT_NPC		(3)

/* NPC movement types */
#define PATH_MOVEMENT		(0)
#define RANDOM_MOVEMENT		(1)
#define WAITING_MOVEMENT	(2)
#define CHASING_MOVEMENT	(3)

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

/*
 ** Structure definitions **************************************************
 */

/* NPC data */
struct NPC_data {
	UNBYTE Number;
	UNBYTE Travel_mode;
	UNSHORT Event_nr;
	UNSHORT Graphics_nr;
	UNSHORT Flags;
	UNSHORT Trigger_mode;
};

/* Bit masks for NPC_data.Flags */
#define NPC_TYPE					(0x0003)
#define MOVEMENT_TYPE			(0x000c)
#define MOVEMENT_TYPE_B			(2)
#define SHORT_DIALOGUE			(0x0010)
#define ICON_PRIORITY			(0x0020)
#define NPC_WAVE_ANIM			(0x0040)
#define NPC_ICON_HEIGHT			(0x0180)
#define NPC_ICON_HEIGHT_B		(7)
#define NPC_ASYNC_ANIM			(0x0200)
#define NPC_RANDOM_ANIM			(0x0400)
#define MAP_GRAPHICS				(0x0800)

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
	UNBYTE _pad02;
	struct NPC_data NPCs[NPCS_PER_MAP];
};

#define ICON_SET_1_NR	xMap_info.x2D_map_info.xIcon_set_1_nr
#define ICON_SET_2_NR	xMap_info.x2D_map_info.xIcon_set_2_nr
#define LAB_DATA_NR		xMap_info.x3D_map_info.xLab_data_nr

/* Bit masks for Map_data.Flags */
#define LIGHT_STATUS			(0x03)
#define MAP_TYPE				(0x0C)
#define MAP_TYPE_B			(3)
#define PLANET_SPACESHIP	(0x10)

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
	UNBYTE X,Y;
	UNBYTE Direction;
	UNBYTE Bit_nr;
};

/* Event data block */
struct Event_block {
	UNSHORT Byte[6];
	UNSHORT Word[2];
	UNSHORT Next;
};

/*
 ** External global variables **********************************************
 */

extern MEM_HANDLE Map_handle;

extern struct Map_data *Map_ptr;

extern SISHORT _3D_map;
extern UNSHORT Map_type;
extern UNSHORT Palette_nr;
extern UNSHORT Map_width, Map_height, Map_size;

extern UNLONG Event_automap_offset;
extern UNLONG Goto_point_offset;
extern UNLONG NPC_path_offset;
extern UNLONG Event_data_offset;
extern UNLONG Event_entry_offset;

extern SISHORT Offsets4[4][2];
extern SISHORT Offsets8[8][2];

/*
 ** Prototypes *************************************************************
 */

void Init_map(void);
void Exit_map(void);

void Colour_cycle_forward(UNSHORT Start, UNSHORT Size);

BOOLEAN Before_move(void);
void After_move(void);
UNLONG Get_location_status(SISHORT X, SISHORT Y);

#endif

