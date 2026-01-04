/************
 * NAME     : BBMISC.H
 * AUTHOR   : Thomas HÑuser & Jurie Horneman, BlueByte
 * START    : 30-11-1994
 * PROJECT  : Miscellaneous functions
 * NOTES    :
 * SEE ALSO : BBMISC.C
 ************/

#ifndef BBMISC_H
#define BBMISC_H

#include <BBDEF.H>

/*
 ** Macro definitions ******************************************************
 */

/* ByteFolge fÅr UNLONG Ñndern */
UNSHORT MISC_Swap_unshort(UNSHORT a);
#pragma aux MISC_Swap_unshort = \
 "xchg al,ah"   \
 parm [AX] 		\
 value [AX] 		\
 modify [];

/* ByteFolge fÅr UNLONG Ñndern */
UNLONG MISC_Swap_unlong(UNLONG a);
#pragma aux MISC_Swap_unlong = \
 "xchg al,ah"   \
 "rol	eax,16"   \
 "xchg al,ah"   \
 parm [EAX] 		\
 value [EAX] 		\
 modify [];

#ifdef BB32_BIGENDIAN
	#define MISC_INTEL2UNSHORT(a) (MISC_Swap_unshort(a))
	#define MISC_INTEL2UNLONG(a) (MISC_Swap_unlong(a))

	#define MISC_UNSHORT2INTEL(a) (MISC_Swap_unshort(a))
	#define MISC_UNLONG2INTEL(a) (MISC_Swap_unlong(a))

	#define MISC_MOTOROLA2UNSHORT(a) (a)
	#define MISC_MOTOROLA2UNLONG(a) (a)

	#define MISC_UNSHORT2MOTOROLA(a) (a)
	#define MISC_UNLONG2MOTOROLA(a) (a)
#endif

#ifdef BB32_LITTLEENDIAN
	#define MISC_INTEL2UNSHORT(a) (a)
	#define MISC_INTEL2UNLONG(a) (a)

	#define MISC_UNSHORT2INTEL(a) (a)
	#define MISC_UNLONG2INTEL(a) (a)

	#define MISC_MOTOROLA2UNSHORT(a) (MISC_Swap_unshort(a))
	#define MISC_MOTOROLA2UNLONG(a) (MISC_Swap_unlong(a))

	#define MISC_UNSHORT2MOTOROLA(a) (MISC_Swap_unshort(a))
	#define MISC_UNLONG2MOTOROLA(a) (MISC_Swap_unlong(a))
#endif

#endif
