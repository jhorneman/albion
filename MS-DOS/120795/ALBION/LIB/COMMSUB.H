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
extern SILONG ip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval);
extern SILONG aip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval);
extern SILONG dip3d(SILONG val0,SILONG val1,SILONG anz,SILONG intval);
extern SILONG calc_delta_xyz(SILONG sx,SILONG sy,SILONG sz,
											SILONG zx,SILONG zy,SILONG zz,
											SILONG *dx,SILONG *dy,SILONG *dz,
											SILONG ab);
extern SILONG rndmm(SILONG min,SILONG max);
extern SILONG rndmm2(SILONG min,SILONG max,SILONG minii,SILONG maxii);
extern void update_n_times(SILONG t);
extern void Add_halo_to_COMOB_2(struct COMOB *Source_COMOB, UNSHORT Nr_halo_COMOBs, UNSHORT Halo_width, UNSHORT Halo_lifespan);
extern BOOLEAN Gen_Funken(SILONG x,SILONG y,SILONG z,MEM_HANDLE obj,SILONG anz);
extern void Gen_Sparks(SILONG x,SILONG y,SILONG z,SILONG amount,SILONG speed,SILONG life,SILONG size,SILONG sparktyp);
extern BOOLEAN	groundglow(SILONG x,SILONG z,SILONG size,SILONG duration);
extern struct COMOB *makeflame(SILONG x,SILONG y,SILONG z,SILONG flamesize);
extern SILONG Dissolve(struct Combat_participant *victim,SILONG xsize,SILONG ysize,struct COMOB **COMOBLIST,SILONG maxcomobs);

extern void size_handler(struct COMOB_behaviour *Behaviour);
extern void flash_handler(struct COMOB_behaviour *Behaviour);
extern void glow_handler(struct COMOB_behaviour *Behaviour);
extern void snow_handler(struct COMOB_behaviour *Behaviour);
extern void FireBounce_handler(struct COMOB_behaviour *Behaviour);
extern void Oscillate_II_handler(struct COMOB_behaviour *Behaviour);
extern void CrazyMove_handler(struct COMOB_behaviour *Behaviour);
extern void connect_handler(struct COMOB_behaviour *Behaviour);
extern void funken_handler(struct COMOB_behaviour *Behaviour);
extern void groundglow_handler(struct COMOB_behaviour *Behaviour);
extern void FairyII_handler(struct COMOB_behaviour *Behaviour);
extern void DustII_handler(struct COMOB_behaviour *Behaviour);

#endif
