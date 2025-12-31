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

#include <2D_MAP.H>
#include <2D_DISPL.H>

/*
 ** Macro definitions ******************************************************
 */

#define sgn(A) (((A)==0) ? (0) : (((A)>0) ? (1) : (-1)))

#define NPC_present(NPC_index) (VNPCs[NPC_index].Present)

/*
 ** Defines ****************************************************************
 */

#define NPCS_PER_MAP			(96)
#define NPC_PATH_LENGTH		(1152)
#define MAX_2D_LOS_LENGTH	(20)

/* NPC types */
#define PARTY_NPC			(0)
#define NPC_NPC			(1)
#define MONSTER_NPC		(2)
#define OBJECT_NPC		(3)

/* NPC movement types */
#define ABS_PATH_MOVEMENT	(0)
#define RANDOM_MOVEMENT		(1)
#define WAITING_MOVEMENT	(2)
#define CHASING_MOVEMENT	(3)
#define REL_PATH_MOVEMENT	(4)

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
	UNBYTE Travel_mode;
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

/* NPC data (map) */
union NPC_data {
	struct Old_NPC_data Old_NPC_data;
	struct New_NPC_data New_NPC_data;
};

/* Internal NPC data */
struct VNPC_data {
	UNSHORT Number;
	UNSHORT Graphics_nr;

	UNSHORT NPC_type;					/* See above */

	UNSHORT Travel_mode;

	UNSHORT Trigger_modes;			/* NPC event data */
	UNSHORT First_block_nr;			/* 0xFFFF = no event */

	UNSHORT Movement_type;

	BOOLEAN Present;

	UNLONG Flags;

	UNSHORT Frame;
	UNLONG Path_offset;				/* Offset to path (if necessary) */

	UNSHORT Rnd_path_length; 		/* Random path information */
	UNSHORT Rnd_path_direction;

	UNSHORT Current_path_index;

	UNSHORT Map_X, Map_Y;			/* Current map coordinates */
	UNSHORT Next_X, Next_Y;			/* Next map coordinates */
	UNSHORT Old_X, Old_Y;			/* Old map coordinates */

	UNLONG Source_X, Source_Y;		/* Source coordinates in 2D / 3D units */

	struct Object_2D NPC_object;	/* 2D : Display object */
	struct Movement_2D Move;		/* 2D : Movement data */
};

/* Bitmask for VNPC_data.Flags */
#define NPC_MOVEMENT_ENDED		(1<<0)
#define NPC_COLLIDED				(1<<1)
#define NPC_MOVING_RANDOMLY	(1<<2)
#define NPC_SLEEPING				(1<<3)

#define NPC_SHORT_DIALOGUE		(1<<5)
#define NPC_CAN_TRIGGER			(1<<6)
#define NPC_ANIMATE_ALWAYS		(1<<7)

/*
 ** External global variables **********************************************
 */

extern struct VNPC_data VNPCs[NPCS_PER_MAP];

extern struct NPC_3D NPCs_3D[NPCS_PER_MAP];
extern UNSHORT Nr_3D_NPCs;

extern BOOLEAN Monster_is_watching;

/*
 ** Prototypes *************************************************************
 */

void Update_NPC_present_status(void);

UNLONG Skip_through_NPC_paths(struct Map_data *Map_ptr, UNLONG Offset);
void Init_NPC_data(void);

void Init_2D_NPCs(void);
void Exit_2D_NPCs(void);

void Init_3D_NPCs(void);
void Exit_3D_NPCs(void);

void Handle_2D_NPCs(void);
void Display_2D_NPCs(void);

void Handle_3D_NPCs(void);
void Display_3D_NPCs(void);

void Do_path_NPC_2D(UNSHORT NPC_index);
void Do_random_NPC_2D(UNSHORT NPC_index);
void Do_chasing_NPC_2D(UNSHORT NPC_index);

void Do_path_NPC_3D(UNSHORT NPC_index);
void Do_random_NPC_3D(UNSHORT NPC_index);
void Do_chasing_NPC_3D(UNSHORT NPC_index);

BOOLEAN Check_line_of_sight(UNSHORT NPC_index);
void Check_for_NPC_reaction(UNSHORT NPC_index);

//BOOLEAN NPC_present(UNSHORT NPC_index);

UNSHORT Search_2D_NPC(UNSHORT X, UNSHORT Y, UNSHORT NPC_index);

#endif

