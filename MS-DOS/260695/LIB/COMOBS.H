/************
 * NAME     : COMOBS.H
 * AUTOR    : Jurie Horneman, BlueByte
 * START    : 25-1-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMOBS_H
#define COMOBS_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

/*
Move_steps	EQU 4
Max_projectiles	EQU 3
Projectile_speed	EQU 30
Projectile_height	EQU 30
Combat_animspeed	EQU 8
Fast_combat_delay	EQU 20
Combat_row_width	EQU 270
Nr_flames	EQU 5
Monster_delay_time	EQU 15
*/

#define MAX_COMOBS					(400)
#define MAX_COMOB_BEHAVIOURS		(200)

/* Combat display detail levels */
#define MIN_COMBAT_DISPLAY_DETAIL_LEVEL		(0)
#define MAX_COMBAT_DISPLAY_DETAIL_LEVEL		(100)
#define DEFAULT_COMBAT_DISPLAY_DETAIL_LEVEL	(100)

/* Combat 3D projection data */
#define COMOB_DEC_FACTOR		(100)
#define COMBAT_PROJ_FACTOR		(148)
#define COMBAT_CAMERA_HEIGHT	(83)
#define MINIMUM_Z					(-110)
#define COMBAT_Z_OFFSET			(0)

#define COMBAT_SQUARE_WIDTH	(32 * COMOB_DEC_FACTOR)
#define COMBAT_SQUARE_DEPTH	(64 * COMOB_DEC_FACTOR)

#define PARTY_Z					(0 - (110 * COMOB_DEC_FACTOR))

#define NR_COMBAT_STARS		(3)
#define NR_COMBAT_SPARKS	(8)

/* Combat display window coordinates and size */
#define COMBAT_X						(0)
#define COMBAT_Y						(0)
#define COMBAT_WIDTH					(360)
#define COMBAT_HEIGHT				(192)

/* Bitmasks for COMOB.System_flags */
#define COMOB_PRESENT	(1 << 0)
#define COMOB_WAVEDIR	(1 << 1)
#define COMOB_HIDDEN		(1 << 2)
#define COMOB_ACTIVE		(1 << 3)
#define COMOB_DELETED	(1 << 4)

/* Bitmasks for COMOB.User_flags */
#define COMOB_WAVEANIM				(1 << 0)
#define COMOB_X_MIRROR				(1 << 1)
#define COMOB_SHADOW					(1 << 2)
#define COMOB_FREE_GFX_ON_DELETE	(1 << 3)

/* COMOB draw modes */
#define NORMAL_COMOB_DRAWMODE			(0)
#define SILHOUETTE_COMOB_DRAWMODE	(1)
#define COLOURED_COMOB_DRAWMODE		(2)
#define COLOURING_COMOB_DRAWMODE		(3)
#define TRANSPARENT_COMOB_DRAWMODE	(4)

/* Oscillate types */
#define OSCILLATE_X			(0)
#define OSCILLATE_Y			(1)
#define OSCILLATE_Z			(2)
#define OSCILLATE_WIDTH		(3)
#define OSCILLATE_HEIGHT	(4)
#define OSCILLATE_SIZE		(5)

/* Fairy dust types */
#define DUST_LEAD		(0)
#define DUST_TINY		(1)
#define DUST_SPARKLE	(2)

/*
 ** Type definitions *******************************************************
 */

typedef void (*COMOB_behaviour_handler) (struct COMOB_behaviour *Behaviour);

/*
 ** Structure definitions **************************************************
 */

/* COMbat OBject data */
struct COMOB {
	UNSHORT Priority;								/* 0...100 */
	UNSHORT System_flags;						/* See above DO NOT SET MANUALLY */
	UNSHORT User_flags;							/* See above */
	UNSHORT Draw_mode;							/* See above */

	UNSHORT Lifespan;								/* 0 = infinite */

	SILONG X_3D, Y_3D, Z_3D;					/* 3D coordinates * COMOB_DEC_FACTOR */
	SILONG dX_3D, dY_3D, dZ_3D;				/* 3D vector * COMOB_DEC_FACTOR */

	UNSHORT Hotspot_X_offset;					/* In % */
	UNSHORT Hotspot_Y_offset;					/* In % */

	UNSHORT Display_width;						/* Display size in % */
	UNSHORT Display_height;

	UNSHORT Frame;									/* Current frame (0...) */
	UNSHORT Nr_frames;                     /* 0 = no animation */

	MEM_HANDLE Graphics_handle;				/* Handle of graphics */
	UNLONG Graphics_offset;						/* Offset to graphics */

	UNSHORT Colour;								/* For special draw modes */
	MEM_HANDLE Special_handle;
	UNLONG Special_offset;
};

/* COMbat OBject behaviour data */
union COMOB_behaviour_data {
	struct {
		SISHORT Gravity;	  						/* * COMOB_DEC_FACTOR, 0 = no gravity */
		UNSHORT Bounce; 							/* In %, 0 = no bounce */
		UNSHORT Air_friction;					/* In promille, 0 = no air friction */
	} Bounce_data;
	struct {
		UNSHORT Type;								/* See above */
		UNSHORT Period;							/* Period of sine wave */
		UNSHORT Value;								/* Current position on sine wave
														  (0...Period-1) */
		SILONG Amplitude;							/* Depending on type */
		SILONG Old_delta;							/* Set by oscillate handler */
	} Oscillate_data;
	struct {
		UNSHORT Current_distance;
		UNSHORT Minimum_distance;
		UNSHORT Maximum_distance;
	} Trail_data;
	struct {
		UNSHORT Type;								/* See above */
	} Dust_data;
};

/* Combat object behaviour */
struct COMOB_behaviour {
	struct COMOB *First_COMOB;					/* Pointer to behaving COMOB */
	struct COMOB *Second_COMOB;				/* Pointer to attached COMOB */
	COMOB_behaviour_handler Handler;			/* Behaviour handler */
	union COMOB_behaviour_data Data;			/* Behaviour data */
};

/*
 ** External global variables **********************************************
 */

extern UNSHORT Combat_display_detail_level;

extern UNSHORT Combat_projection_factor;
extern UNSHORT Combat_camera_height;
extern SISHORT Combat_Z_offset;

extern BOOLEAN Combat_grid_flag;

extern MEM_HANDLE Star_gfx_handles[NR_COMBAT_STARS];
extern MEM_HANDLE Spark_gfx_handles[NR_COMBAT_SPARKS];

/*
 ** Prototypes *************************************************************
 */

/* Main system initialization function */
void Init_COMOB_system(void);
void Exit_COMOB_system(void);

/* COMOB management functions */
struct COMOB *Add_COMOB(UNSHORT Priority);
void Delete_COMOB(struct COMOB *COMOB);
struct COMOB *Duplicate_COMOB(struct COMOB *Source_COMOB);

void Hide_COMOB(struct COMOB *COMOB);
void Show_COMOB(struct COMOB *COMOB);

/* COMOB behaviour management functions */
struct COMOB_behaviour *Add_COMOB_behaviour(struct COMOB *First_COMOB,
 struct COMOB *Second_COMOB, COMOB_behaviour_handler Handler);
void Delete_COMOB_behaviour(struct COMOB_behaviour *Behaviour);

/* COMOB display functions */
void Draw_combat_screen(void);
void Update_COMOBs(void);

/* COMOB display support functions */
void Clean_up_COMOB_list(void);
void Draw_COMOB(struct COMOB *COMOB);

/* COMOB sorting functions */
void Swap_COMOBs(UNLONG A, UNLONG B, UNBYTE *Data);
BOOLEAN Compare_COMOBs(UNLONG A, UNLONG B, UNBYTE *Data);

/* COMOB functions */
void Get_COMOB_source_size(struct COMOB *COMOB, UNSHORT *Width_ptr,
 UNSHORT *Height_ptr);

void Get_COMOB_rectangle(struct COMOB *COMOB, SILONG *Left_3D_ptr,
 SILONG *Top_3D_ptr, SILONG *Right_3D_ptr, SILONG *Bottom_3D_ptr);

/* COMOB animation functions */
void Animate_all_COMOBs(void);
void Circle_COMOB(struct COMOB *COMOB);
void Wave_COMOB(struct COMOB *COMOB);

/* COMOB recolouring table generation functions */
MEM_HANDLE Calculate_transparency_table(UNSHORT Intensity);
MEM_HANDLE Calculate_luminance_table(void);
MEM_HANDLE Calculate_transluminance_table(void);

/* COMOB behaviour handlers */
void Bounce_handler(struct COMOB_behaviour *Behaviour);
void Oscillate_handler(struct COMOB_behaviour *Behaviour);
void Trail_handler(struct COMOB_behaviour *Behaviour);
void Shadow_handler(struct COMOB_behaviour *Behaviour);
void Fairy_handler(struct COMOB_behaviour *Behaviour);
void Dust_handler(struct COMOB_behaviour *Behaviour);

/* COMOB behaviour functions */
void Add_shadow_to_COMOB(struct COMOB *Casting_COMOB);
void Add_trail_to_COMOB(struct COMOB *Trailed_COMOB, UNSHORT Nr_trailing_COMOBs);
void Add_halo_to_COMOB(struct COMOB *Source_COMOB, UNSHORT Nr_halo_COMOBs,
 UNSHORT Halo_width, UNSHORT Halo_lifespan);

/* Diagnostic functions */
void Draw_combat_grid(void);

#endif

