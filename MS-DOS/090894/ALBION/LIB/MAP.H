/************
 * NAME     : MAP.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 26-7-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

/*
 ** Defines ****************************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/* Map data header */
typedef struct {
	UNSHORT Flags;
	UNBYTE Map_display_type;
	UNBYTE Map_music;
	UNBYTE Map_width;
	UNBYTE Map_height;
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
} Map_data;

#define ICON_SET_1_NR	xMap_info.x2D_map_info.xIcon_set_1_nr
#define ICON_SET_2_NR	xMap_info.x2D_map_info.xIcon_set_2_nr
#define LAB_DATA_NR		xMap_info.x3D_map_info.xLab_data_nr

/* Bit masks for Map_data.Flags */
#define LIGHT_STATUS			(0x03)
#define MAP_TYPE				(0x0C)
#define MAP_TYPE_B			(3)
#define PLANET_SPACESHIP	(0x10)

/* NPC data */
typedef struct {
	UNBYTE Number;
	UNBYTE Travel_mode;
	UNSHORT Event_nr;
	UNSHORT Graphics_nr;
	UNSHORT Flags;
	UNSHORT Trigger_mode;
} NPC_data;

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

/*
 ** Prototypes *************************************************************
 */

