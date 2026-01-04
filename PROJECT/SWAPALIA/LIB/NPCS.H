/************
 * NAME     : NPCS.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 22-11-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef NPCS_H
#define NPCS_H

#include <BBDEF.H>
#include <BBMEM.H>

#include <2D_COLL.H>
#include <2D_DISPL.H>
#include <3D_MAP.H>
#include <NPCSOUND.H>

/*
 ** Macro definitions ******************************************************
 */

#define NPC_present(NPC_index) (VNPCs[NPC_index].Present)

/*
 ** Defines ****************************************************************
 */

#define NPCS_PER_MAP			(96)

#define MONSTER_DELAY_TIME	(4)

#define NPC_PATH_LENGTH		(1152)

/* NPC movement types */
#define ABS_PATH_MOVEMENT	(0)
#define RANDOM_MOVEMENT		(1)
#define WAITING_MOVEMENT	(2)
#define CHASING_MOVEMENT	(3)
#define REL_PATH_MOVEMENT	(4)

/*
 ** Enumerations ***********************************************************
 */

/* NPC types */
enum eNPC_type {
	PARTY_NPC,
	NPC_NPC,
	MONSTER_NPC,
	OBJECT_NPC
};

/* Chase states */
enum eNPC_chase_state {
	WAITING_NPC_CHASE_STATE,
	HUNTING_NPC_CHASE_STATE,
	WANDERING_NPC_CHASE_STATE,
	EVADING_1_NPC_CHASE_STATE,
	EVADING_2_NPC_CHASE_STATE,
	FRUSTRATED_NPC_CHASE_STATE
};

/*
 ** Type definitions *******************************************************
 */

typedef enum eNPC_type NPC_TYPE_T;
typedef enum eNPC_chase_state NPC_CHASE_STATE_T;

/*
 ** Structure definitions **************************************************
 */

/* Old NPC data */
struct Old_NPC_data {
	UNBYTE Number;
	UNBYTE Travel_mode;
	UNSHORT First_block_nr;
	UNSHORT Graphics_nr;
	UNSHORT Flags;
	UNSHORT Trigger_modes;
};

/* Bit masks for Old_NPC_data.Flags */
#define OLD_NPC_TYPE					(0x0003)		/* See above */
#define OLD_NPC_MOVEMENT_TYPE		(0x000c)
#define OLD_NPC_MOVEMENT_TYPE_B	(2)
#define OLD_NPC_SHORT_DIALOGUE	(1 << 4)

//#define ICON_PRIORITY			(0x0020)
//#define NPC_WAVE_ANIM			(0x0040)
//#define NPC_ICON_HEIGHT		(0x0180)
//#define NPC_ICON_HEIGHT_B	(7)
//#define NPC_ASYNC_ANIM		(0x0200)
//#define NPC_RANDOM_ANIM		(0x0400)
//#define NPC_GRAPHICS			(0x0800)

/* New NPC data */
struct New_NPC_data {
	UNBYTE Number;
	UNBYTE Sound_index;
	UNSHORT First_block_nr;
	UNSHORT Graphics_nr;
	UNBYTE Flags;
	UNBYTE Movement_type;		/* See above */
	UNSHORT Trigger_modes;
};

/* Bit masks for New_NPC_data.Flags */
#define NEW_NPC_TYPE					(0x0007)		/* See above */
#define NEW_NPC_CAN_TRIGGER		(1 << 3)
#define NEW_NPC_SHORT_DIALOGUE	(1 << 4)
#define NEW_NPC_ANIMATE_ALWAYS	(1 << 5)
#define NEW_NPC_NOT_ON_FOOT		(1 << 6)

/* NPC data (map) */
union NPC_data {
	struct Old_NPC_data Old_NPC_data;
	struct New_NPC_data New_NPC_data;
};

/* Internal NPC data */
struct VNPC_data {
	UNSHORT Number;
	UNSHORT Graphics_nr;

	NPC_TYPE_T NPC_type;				/* See above */

	UNSHORT Travel_mode;

	UNSHORT Sound_set_index;
	UNSHORT Effect_indices[MAX_NPC_EFFECT_TYPES];

	UNSHORT Trigger_modes;			/* NPC event data */
	UNSHORT First_block_nr;			/* 0xFFFF = no event */

	UNSHORT Movement_type;

	BOOLEAN Present;

	UNLONG Flags;

	UNSHORT Frame;

	/* Offset to path (if necessary) */
	UNLONG Path_offset;

	/* Random path data */
	UNSHORT Rnd_path_length;
	UNSHORT Rnd_path_direction;

	/* Relative path data */
	UNSHORT Current_path_index;

	/* Chase data */
	NPC_CHASE_STATE_T Chase_state;

	/* Current map coordinates */
	/* (to be used for collision and event checks) */
	UNSHORT Map_X, Map_Y;

	/* Old map coordinates */
	UNSHORT Old_X, Old_Y;

	/* Display coordinates in 2D / 3D units */
	UNLONG Display_X, Display_Y;

	/* Movement vector in 2D / 3D units */
	SILONG dX, dY;

	/* Movement speed in 2D / 3D units */
	UNSHORT Movement_speed;

	/* Source and target coordinates used for current position interpolation */
	UNSHORT Source_X, Source_Y;
	UNSHORT Target_X, Target_Y;

	/* For movement of locked NPCs in scripts */
	UNSHORT Locked_move_step;

	union {
		struct {
			struct Object_2D NPC_object;	/* 2D : Display object */
			struct Movement_2D Move;		/* 2D : Movement data */
		} _2D_NPC_data;
	} Data;
};

/* Bitmask for VNPC_data.Flags */
#define NPC_LOCKED				(1 << 0)
#define NPC_CAN_TRIGGER			(1 << 1)
#define NPC_MOVED					(1 << 2)
#define NPC_SLEEPING				(1 << 3)
#define NPC_ANIMATE_ALWAYS		(1 << 4)
#define NPC_SHORT_DIALOGUE		(1 << 5)

/*
 ** External global variables **********************************************
 */

extern BOOLEAN Hide_NPCs;

extern struct VNPC_data VNPCs[NPCS_PER_MAP];

extern struct NPC_3D NPCs_3D[NPCS_PER_MAP];
extern UNSHORT Nr_3D_NPCs;

extern SISHORT Monster_move_delay;

extern BOOLEAN Monster_is_watching;

/*
 ** Prototypes *************************************************************
 */

void Init_NPC_data(void);

void Init_2D_NPCs(void);
void Exit_2D_NPCs(void);

void Init_2D_NPC_graphics(UNSHORT NPC_index);
void Exit_2D_NPC_graphics(UNSHORT NPC_index);

void Init_3D_NPCs(void);
void Exit_3D_NPCs(void);

void Handle_2D_NPCs(void);
void Display_2D_NPCs(void);

void Handle_3D_NPCs(void);
void Display_3D_NPCs(void);

//BOOLEAN NPC_present(UNSHORT NPC_index);

UNSHORT Search_2D_NPC(UNSHORT Map_X, UNSHORT Map_Y, UNSHORT NPC_index);
UNSHORT Search_3D_NPC(UNSHORT Map_X, UNSHORT Map_Y, UNSHORT NPC_index);

void Turn_towards_NPC(UNSHORT NPC_index);

void Update_NPC_present_status(void);

UNLONG Skip_through_NPC_paths(struct Map_data *Map_ptr, UNLONG Offset);

void Lock_NPC(UNSHORT NPC_index);
void Unlock_NPC(UNSHORT NPC_index);

void Unlock_all_NPCs(void);

void Set_NPC_position(UNSHORT NPC_index, UNSHORT New_X, UNSHORT New_Y);
void Move_locked_NPC(UNSHORT NPC_index, SISHORT dX, SISHORT dY);

#endif

