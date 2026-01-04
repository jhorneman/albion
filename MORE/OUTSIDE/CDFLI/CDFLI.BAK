#ifndef CDFLI_H
#define CDFLI_H


/* Includes */

#include "BBDEFxx.H"



/* Defines */


/* Buffer size */

// #define CDBUFSIZE128K
// #define CDBUFSIZE512K
// #define CDBUFSIZE1024K
#define CDBUFSIZE2048K


/* Defines for different buffer sizes */

#ifdef CDBUFSIZE128K

	/* 128K Preload Buffer */

	#define FLI_EMS_PAGES		8
	#define FLI_EMS_PAGES_AND	7
	#define FLI_MEM	  			0x0020000L

	/* Bytes to preload */

	#define FLI_PRELOAD			0x0020000L

#endif

#ifdef CDBUFSIZE512K

	/* 512K Preload Buffer */

	#define FLI_EMS_PAGES		32
	#define FLI_EMS_PAGES_AND	31
	#define FLI_MEM	  			0x0080000L

	/* Bytes to preload */

	#define FLI_PRELOAD			0x0040000L

#endif

#ifdef CDBUFSIZE1024K

	/* 1024K Preload Buffer */

	#define FLI_EMS_PAGES		64
	#define FLI_EMS_PAGES_AND	63
	#define FLI_MEM	  			0x0100000L

	/* Bytes to preload */

	#define FLI_PRELOAD			0x0040000L

#endif

#ifdef CDBUFSIZE2048K

	/* 1024K Preload Buffer */

	#define FLI_EMS_PAGES		128
	#define FLI_EMS_PAGES_AND	127
	#define FLI_MEM	  			0x0200000L

	/* Bytes to preload */

	#define FLI_PRELOAD			0x0040000L

#endif



/* Prototypes */

void INITVMODE13( void );
void EXITVMODE13( void );
void SETVMODE13COL( UNSHORT c, UNBYTE r, UNBYTE g, UNBYTE b );
void SETVMODE13PAL( UNBYTE * palptr );


short CDFLI_PlayFli( short ems_handle, short ems_offset, UNBYTE far * memptr, char * fliname, unsigned short framestoplay, short setmode13 );


#endif
