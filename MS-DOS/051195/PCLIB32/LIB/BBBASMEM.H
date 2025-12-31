/************
 * NAME     : BBBASMEM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 28-12-1994
 * PROJECT  : Base memory management / DPMI
 * VERSION  : 2.0
 * SEE ALSO : BBBASMEM.C
 ************/

#ifndef BBBASEMEM_H
#define BBBASEMEM_H

#include <stdio.h>

#include <BBDEF.H>

/*
 ** Defines ****************************************************************
 */

/* Memory types (MUST be unique bit in LSB !) */
#define BASEMEM_Status_Flat	(1L << 0)
#define BASEMEM_Status_Dos		(1L << 1)

/* Flags for BASEMEM_Alloc */
#define BASEMEM_Status_Clear	(1L << 8)		/* Clear memory on allocation */
#define BASEMEM_Status_Lock	(1L << 9)		/* Lock memory on allocation */

#define BASEMEM_Status_Normal	(BASEMEM_Status_Flat | BASEMEM_Status_Clear)

/* Alignment for memory pointers on this system */
#define BASEMEM_ALIGNMENT		(8)

/*
 ** Structure definitions **************************************************
 */

/*
 ** External global variables **********************************************
 */

/*
 ** Prototypes *************************************************************
 */

extern BOOLEAN BASEMEM_Init(void);
extern void BASEMEM_Exit(void);

extern UNLONG BASEMEM_Get_largest_block(UNLONG flagset);
extern UNLONG BASEMEM_Get_largest_real_block(UNLONG flagset);
extern UNLONG BASEMEM_Get_free_memory(UNLONG flagset);

extern UNBYTE *BASEMEM_Alloc(UNLONG size, UNLONG flagset);
extern BOOLEAN BASEMEM_Free(UNBYTE *pointer);
extern UNBYTE *BASEMEM_Resize(UNBYTE *pointer, UNLONG size);

extern BOOLEAN BASEMEM_Lock_region(UNBYTE *pointer, UNLONG size);
extern BOOLEAN BASEMEM_Unlock_region(UNBYTE *pointer, UNLONG size);

extern void BASEMEM_FillMemByte(UNBYTE *ptr, UNLONG size, UNBYTE fillbyte);
extern void BASEMEM_FillMemLong(UNBYTE *ptr, UNLONG size, UNLONG filllong);

extern void BASEMEM_CopyMem(UNBYTE *srcptr, UNBYTE *destptr, UNLONG bytestocopy);

extern UNLONG BASEMEM_Align(UNLONG Value);

extern void BASEMEM_Report(FILE *Output_file);

#endif
