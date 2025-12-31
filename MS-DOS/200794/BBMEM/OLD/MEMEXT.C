/************
 * NAME     : MEMEXT.C
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 6-6-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    : - These are the external functions needed by the memory manager.
 * SEE ALSO : BBMEM.C, BBMEM.H
 ************/

/* includes */

#include "BBDEF.H"
#include "BBMEM.H"

/* prototypes */

void BASE_Get_memory(struct Memory_entry *Area);
void BASE_Free_memory(struct Memory_entry *Area);

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BASE_Get_memory
 * FUNCTION  : Allocates a large block of memory using the OS, to be used by
 *             the memory manager.
 * FILE      : MEMEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 6.06.94 12:02
 * LAST      : 6.06.94 12:02
 * INPUTS    : struct Memory_entry : *Area - Pointer to the memory area
 *              structure that must be initialized.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function should set the Start, Size and Memory_type
 *              elements of the memory area structure.
 *             - Size 0 should be returned when no memory could be allocated.
 *             - The areas should not be smaller than SMALL_FISH.
 * SEE ALSO  : BBMEM.C, BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */
void
BASE_Get_memory(struct Memory_entry *Area)
{
	UBYTE *Start:
	ULONG Size;

/* Call platform-dependent memory allocation.
	Put start of block in <Start>, size in <Size>. On most platforms,
	the memory type will always be 1.
	Call MEM_Align to align memory blocks. */

/* Initialize memory area */
	Area->Start = (UBYTE *) MEM_Align((ULONG)Start);
	Area->Size = MEM_Align(Size);
	Area->MEMORY_TYPE = 0x01;

/* No other structure elements need be filled in. Return size 0 when no more
 memory can be allocated from the OS. */

}

/*
 *****************************************************************************
 * #FUNCTION HEADER BEGIN#
 * NAME      : BASE_Free_memory
 * FUNCTION  : Returns a memory area to the OS.
 * FILE      : MEMEXT.C
 * AUTHOR    : Jurie Horneman
 * FIRST     : 6.06.94 12:02
 * LAST      : 6.06.94 12:02
 * INPUTS    : struct Memory_entry : *Area - Pointer to the memory area
 *              structure that must be freed.
 * RESULT    : None.
 * BUGS      : No known.
 * NOTES     : - This function should use the Start and Size elements of the
 *              memory area structure.
 *             - The memory area structure need not be modified.
 * SEE ALSO  : BBMEM.C, BBMEM.H
 * #FUNCTION HEADER END#
 */

/* #FUNCTION BEGIN# */
void
BASE_Free_memory(struct Memory_entry *Area)
{
/* Call platform-dependent memory free function.
 The Area structure need not be modified. */
}

