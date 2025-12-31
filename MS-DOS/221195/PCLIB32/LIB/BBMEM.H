/************
 * NAME     : BBMEM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 30-5-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    :
 * SEE ALSO :
 ************/

#ifndef BBMEM_H
#define BBMEM_H

#include <BBDEF.H>

/*
 ** Macro definitions ******************************************************
 */

#ifdef DEBUG

#define MEM_Handle_is_legal(a) MEM_Check_if_handle_is_legal(a, __LINE__, __FILE__)

#else

#define MEM_Handle_is_legal(a) MEM_Check_if_handle_is_legal(a, 0, NULL)

#endif

/*
 ** Defines ****************************************************************
 */

#define MEMORY_AREAS_MAX		(10)
#define MEMORY_ENTRIES_MAX		(750)
#define MEMORY_HANDLES_MAX		(400)
#define MEMORY_TYPES_MAX		(8)

#define MEMORY_WORKSPACES_MAX	(5)
#define WORKSPACE_STACK_MAX	(8)

/*
 ** Type definitions *******************************************************
 */

typedef struct Memory_handle *MEM_HANDLE;

typedef MEM_HANDLE (*MEM_Alloc_pass) (UNLONG, UNBYTE);

typedef void (*MEM_Relocator) (MEM_HANDLE, UNBYTE*, UNBYTE*, UNLONG);

typedef void (*MEM_Out_of_memory_handler) (UNLONG Size, UNLONG Findex, struct File_type *Ftype);

/*
 ** Structure definitions **************************************************
 */

struct Memory_request {
	UNBYTE Memory_type;						/* Memory type for BBMEM */
	UNLONG Base_memory_type;				/* Memory type for BASEMEM */
	UNLONG Minimum;
	UNLONG Maximum;							/* Use 0xFFFFFFFF to get everything */
	UNLONG Safety;								/* Always leave this */
};

struct Grabbed_memory {
	UNBYTE *Start;
	UNLONG Size;
};

struct Memory_entry {
	UNBYTE *Start;								/* Start address */
	UNLONG Size;  								/* Size, 0 means free block */
	struct Memory_entry *Previous;		/* Always NULL for areas */
	struct Memory_entry *Next;  			/* NULL means end of list */
	union {
		struct {
			UNSHORT xWorkspace_number;	 	/* Workspace number */
			UNBYTE xMemory_type;	 			/* Memory type */
		} xArea_info;
		struct {
			MEM_HANDLE xBlock_handle;  	/* Handle index */
		} xBlock_info;
	} xInfo;
};

#define WORKSPACE_NUMBER	xInfo.xArea_info.xWorkspace_number
#define MEMORY_TYPE			xInfo.xArea_info.xMemory_type
#define BLOCK_HANDLE			xInfo.xBlock_info.xBlock_handle

struct Memory_handle {
	UNBYTE Flags;
	UNBYTE Priority;
	UNBYTE Claim_counter;
	UNBYTE Load_counter;
	UNBYTE Size_low_byte;
	UNLONG File_index;
	struct File_type *File_type_ptr;		/* Is assumed to be not NULL */
	struct Memory_entry *Entry_ptr;	 	/* NULL means free handle */
};

/* Bit masks for Memory_handle.Flags */
#define MEM_ALLOCATED	(1<<0)			/* This block is allocated */
#define MEM_INVALID		(1<<1)			/* Do not re-use this block */

struct File_type {
	MEM_Relocator Relocator;
	UNBYTE Memory_type;
	UNBYTE Priority;
	UNBYTE Flags;								/* See below */
	UNCHAR *Name;								/* This is NOT a filename! */
};

/* Bit masks for File_type.Flags */
#define MEM_KILL_ALWAYS	(1<<0)			/* Always kill when freeing */
#define MEM_LOCK			(1<<1)			/* VMM lock memory */

struct Memory_workspace {
	UNSHORT Flags;								/* See below */
	UNSHORT Nr_of_areas;						/* 0 means free workspace */
	struct Memory_workspace *Parent_workspace;	/* NULL means original workspace */
	MEM_HANDLE Workspace_handle; 			/* NULL means original workspace */
	void (*Alloc_init)(void);
	void (*Alloc_exit)(void);
	MEM_Out_of_memory_handler Out_of_memory;
	MEM_Alloc_pass *Pass_list;
	struct Memory_entry Areas[MEMORY_AREAS_MAX];
	struct File_type Workspace_ftype;	/* The file type of this workspace */
	struct File_type *Default_ftype;		/* The default file type for this
														workspace */
};

/* Bit masks for Memory_workspace.Flags */
#define MEM_NO_MEMORY_MOVEMENT	(1<<0)	/* Garbage collection etc. not allowed */
#define MEM_NO_PERSISTENCE			(1<<1)	/* No persistent memory blocks */

/*
 ** External global variables **********************************************
 */

extern struct Memory_handle MEM_Handles[MEMORY_HANDLES_MAX];
extern struct Memory_workspace MEM_Workspaces[MEMORY_WORKSPACES_MAX];

extern struct Memory_workspace *MEM_Workspace_stack[WORKSPACE_STACK_MAX];
extern UNSHORT MEM_Workspace_stack_index;

extern void (*MEM_Critical_error_handler)(void);
extern void (*MEM_Start_lengthy_operation_handler)(void);
extern void (*MEM_End_lengthy_operation_handler)(void);

/*
 ** Prototypes *************************************************************
 */

BOOLEAN MEM_Init_memory(UNSHORT Nr_requests, struct Memory_request *Request_list);
void MEM_Exit_memory(void);
void MEM_Reset_memory(void);

UNLONG MEM_Get_total_memory(UNBYTE Type);

void MEM_Clear_all_handles(void);

BOOLEAN MEM_Check_if_handle_is_legal(MEM_HANDLE Handle, int Line_nr, char *Filename);

UNBYTE *MEM_Get_pointer(MEM_HANDLE Handle);
UNBYTE *MEM_Claim_pointer(MEM_HANDLE Handle);
void MEM_Free_pointer(MEM_HANDLE Handle);

void MEM_Relocate(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target, UNLONG Size);

UNLONG MEM_Get_block_size(MEM_HANDLE Handle);
UNLONG MEM_Get_file_index(MEM_HANDLE Handle);

void MEM_Resize_memory(MEM_HANDLE Handle, UNLONG New_size);
void MEM_Free_memory(MEM_HANDLE Handle);
void MEM_Kill_memory(MEM_HANDLE Handle);
void MEM_Invalidate_memory(MEM_HANDLE Handle);
void MEM_Clear_memory(MEM_HANDLE Handle);

BOOLEAN MEM_Inquire_memory(UNLONG Size, UNBYTE Type);

MEM_HANDLE MEM_Allocate_memory(UNLONG Size);

MEM_HANDLE MEM_Do_allocate(UNLONG Size, UNLONG Findex, struct File_type *Ftype);
void MEM_Armageddon(void);

MEM_HANDLE MEM_Reallocate_memory(UNLONG Findex, struct File_type *Ftype);
MEM_HANDLE MEM_Duplicate_memory(UNLONG Findex, struct File_type *Ftype);

struct Memory_entry *MEM_Find_LFB(struct Memory_entry *Area);

UNLONG MEM_Calculate_TFM(struct Memory_entry *Area);
UNLONG MEM_Calculate_EMG(struct Memory_entry *Area, UNSHORT Sea_level);

/* Memory allocation passes */
MEM_HANDLE MEM_Alloc_pass_standard(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Alloc_pass_garbage(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Alloc_pass_drown(UNLONG Size, UNBYTE Type);
//MEM_HANDLE MEM_Alloc_pass_juggle(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Alloc_pass_clean(UNLONG Size, UNBYTE Type);

BOOLEAN MEM_Check_memory(void);

//void MEM_List(void);

/*
 ** External global variables **********************************************
 */

extern struct File_type MEM_Default_file_type;

extern MEM_Alloc_pass MEM_Default_pass_list[];

#endif

