/************
 * NAME     : COMMSUB.H
 * AUTOR    : Rainer Reber, BlueByte
 * START    : 24-4-1994
 * PROJECT  : Albion
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef COMMSUB_H
#define COMMSUB_H

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* fÅr Gen_Sparks */
#define GEN_SPARK_TYP_BLUE 1
#define GEN_SPARK_TYP_ORANGE 2
#define GEN_SPARK_TYP_ALL 3

/* fÅr Gen_COMOB */
#define GC_NORM (1L<<0)
#define GC_TRANS (1L<<1)
#define GC_FIRE (1L<<2)
#define GC_MAGIC (1L<<3)

/* Connect verhalten */
#define CONNECT_X (1L<<0)
#define CONNECT_Y (1L<<1)
#define CONNECT_Z (1L<<2)
#define CONNECT_POSITION (CONNECT_X|CONNECT_Y|CONNECT_Z)
#define CONNECT_WIDTH (1L<<3)
#define CONNECT_HEIGHT (1L<<4)
#define CONNECT_SIZE (CONNECT_WIDTH|CONNECT_HEIGHT)
#define CONNECT_ALL (CONNECT_POSITION|CONNECT_SIZE)

/* Test verhalten */
#define TEST_X_GREATER (1L<<0)
#define TEST_X_LOWER (1L<<1)
#define TEST_Y_GREATER (1L<<2)
#define TEST_Y_LOWER (1L<<3)
#define TEST_Z_GREATER (1L<<4)
#define TEST_Z_LOWER (1L<<5)

/*
 ** Type definitions *******************************************************
 */

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/* global variables */
extern struct Combat_participant *Current_target_parts[18 + 6];
extern UNSHORT Current_nr_target_parts;

/*
 ** Prototypes *************************************************************
 */

/* Prototypes */
extern void Do_build_spell_target_list(struct Combat_participant *Victim_part,
 UNSHORT Tactical_X, UNSHORT Tactical_Y, UNSHORT Strength);

extern struct COMOB *Gen_COMOB(SILONG x,SILONG y,SILONG z,SILONG life,
 SILONG size,MEM_HANDLE gfx,SILONG flag);

extern SILONG ip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval);

extern SILONG aip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval);

extern SILONG dip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval);

extern SILONG calc_strength(SILONG strength,SILONG min,SILONG max);

extern SILONG calc_delta_xyz(SILONG sx,SILONG sy,SILONG sz,
 SILONG zx,SILONG zy,SILONG zz,
 SILONG *dx,SILONG *dy,SILONG *dz,
 SILONG ab);

extern SILONG rndmm(SILONG min,SILONG max);

extern SILONG rndmm2(SILONG min,SILONG max,SILONG minii,SILONG maxii);

extern void update_n_times(SILONG t);

extern struct COMOB **Add_halo_to_COMOB_2(struct COMOB *Source_COMOB,
 UNSHORT Nr_halo_COMOBs, UNSHORT Halo_width, UNSHORT Halo_lifespan,UNSHORT flag);

extern BOOLEAN Gen_Funken(SILONG x,SILONG y,SILONG z,MEM_HANDLE obj,
 SILONG anz);

extern void Gen_Sparks(SILONG x,SILONG y,SILONG z,SILONG amount,SILONG speed,
 SILONG life,SILONG size,SILONG sparktyp);

extern BOOLEAN	groundglow(SILONG x,SILONG z,SILONG size,SILONG duration);

extern struct COMOB *makeflame(SILONG x,SILONG y,SILONG z,SILONG flamesize);

extern struct COMOB *gen_Dummy_COMOB(void);

extern SILONG Dissolve(struct Combat_participant *victim,SILONG xsize,
 SILONG ysize,struct COMOB **COMOBLIST,SILONG maxcomobs);

extern BOOLEAN Test_COMOB_mask(struct COMOB *source_COMOB,SILONG x3d,
 SILONG y3d);

extern void Get_COMOB_max_size(struct COMOB *COMOB, UNSHORT *Width_ptr,
 UNSHORT *Height_ptr);

extern struct COMOB * Show_flashstar(SILONG x,SILONG y,SILONG z);

extern struct COMOB **Add_schweif(struct COMOB *COMOB,SILONG trails,
 SILONG distance,SILONG sizelose);

extern void Show_deflect(struct Combat_participant *Part, UNSHORT Strength);


extern void size_handler(struct COMOB_behaviour *Behaviour);

extern void size_II_handler(struct COMOB_behaviour *Behaviour);

extern void size_III_handler(struct COMOB_behaviour *Behaviour);

extern void flash_handler(struct COMOB_behaviour *Behaviour);

extern void glow_handler(struct COMOB_behaviour *Behaviour);

extern void snow_handler(struct COMOB_behaviour *Behaviour);

extern void snow_II_handler(struct COMOB_behaviour *Behaviour);

extern void FireBounce_handler(struct COMOB_behaviour *Behaviour);

extern void Oscillate_II_handler(struct COMOB_behaviour *Behaviour);

extern void CrazyMove_handler(struct COMOB_behaviour *Behaviour);

extern void connect_handler(struct COMOB_behaviour *Behaviour);

extern void funken_handler(struct COMOB_behaviour *Behaviour);

extern void groundglow_handler(struct COMOB_behaviour *Behaviour);

extern void lifedead_handler(struct COMOB_behaviour *Behaviour);

extern void Fairy_II_handler(struct COMOB_behaviour *Behaviour);

extern void Dust_II_handler(struct COMOB_behaviour *Behaviour);

extern void Animcontrol_II_handler(struct COMOB_behaviour *Behaviour);

extern void Trail_II_handler(struct COMOB_behaviour *Behaviour);

extern void Gravity_handler(struct COMOB_behaviour *Behaviour);

extern void Party_magic_effect(struct Combat_participant *Victim_part,
 UNSHORT Strength);

#endif
