/************
 * NAME     : BBMEMVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 1-6-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    :
 * SEE ALSO : BBMEM.H
 ************/

#ifndef BBMEMVAR_H
#define BBMEMVAR_H

#include <BBDEF.H>
#include <BBMEM.H>

/*
 ** Defines ****************************************************************
 */

#define SMALL_FISH	(5*1024)			/* Too small to manage */

/* Debugging fill patterns */
#define DEBUGGING_PAD_SIZE		(16)

#define MEM_FILLER_1 (0xC0DEDBAD)	/* Free memory */
#define MEM_FILLER_2 (0xDEADFACE)	/* At start and end of block */
#define MEM_FILLER_3 (0xABADCAFE)	/* In allocated block */

/* Error codes */
#define MEMERR_OUT_OF_ENTRIES					(1)
#define MEMERR_OUT_OF_HANDLES 				(2)
#define MEMERR_HANDLE_MISMATCH				(3)
#define MEMERR_REQUEST_DENIED					(4)
#define MEMERR_ENTRY_OUTSIDE_ARRAY			(5)
#define MEMERR_NULL_FILE_TYPE					(6)
#define MEMERR_FREE_ENTRY_IN_CHAIN			(7)
#define MEMERR_AREA_IN_CHAIN					(8)
#define MEMERR_WRONG_BACKWARD_LINK			(9)
#define MEMERR_BLOCK_OUTSIDE_AREA			(10)
#define MEMERR_GAP_BETWEEN_BLOCKS			(11)
#define MEMERR_WRONG_AREA_SIZE				(12)
#define MEMERR_OUT_OF_MEMORY					(13)
// #define MEMERR_OUT_OF_WORKSPACES				(14)
// #define MEMERR_WORKSPACE_STACK_OVERFLOW	(15)
// #define MEMERR_WORKSPACE_STACK_UNDERFLOW	(16)
#define MEMERR_OUT_OF_AREAS					(17)
#define MEMERR_ODD_REQUEST						(18)
#define MEMERR_CLAIMED_FREE_HANDLE	 		(19)
#define MEMERR_ILLEGAL_HANDLE					(20)

#define MEMERR_MAX (20)

/*
 ** Structure definitions **************************************************
 */

/* Memory error data */
struct Memory_error {
	UNSHORT Code;
};

/*
 ** External global variables **********************************************
 */

extern struct Memory_entry MEM_Entries[MEMORY_ENTRIES_MAX];

extern struct Memory_entry *MEM_Quick_free_entry;

extern struct Grabbed_memory Grabbed_memory[];

extern UNBYTE MEM_Initialized;
extern UNBYTE MEM_Debugging2;
extern UNBYTE MEM_Handles_invalid;

extern MEM_Alloc_pass *MEM_Pass_list_ptr;

extern UNCHAR MEM_Library_name[];

extern UNCHAR *MEM_Error_strings[];

/*
 ** Prototypes *************************************************************
 */

void MEM_Error(UNSHORT Error_code);

void MEM_Critical_error(UNSHORT Error_code);

struct Memory_entry *MEM_Find_free_entry(void);

void MEM_Add_entry(struct Memory_entry *New, struct Memory_entry *Target);

void MEM_Delete_entry(struct Memory_entry *Target);

MEM_HANDLE MEM_Create_memory_handle(struct Memory_entry *New_entry);

void MEM_Destroy_memory_handle(struct Memory_entry *Target);

void MEM_Drown_memory(struct Memory_entry *Area, UNSHORT Sea_level);

struct Memory_entry *MEM_Merge_memory_block(struct Memory_entry *Merged);

void MEM_Split_memory_block(struct Memory_entry *Split, UNLONG New_size);

#endif
