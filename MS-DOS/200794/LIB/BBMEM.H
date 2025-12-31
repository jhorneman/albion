/************
 * NAME     : BBMEM.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 30-5-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    :
 * SEE ALSO : BBMEM.C
 ************/

#ifndef BBMEM_H
#define BBMEM_H

/*
 ** Defines ****************************************************************
 */

#define MEMORY_AREAS_MAX	(10)
#define MEMORY_ENTRIES_MAX	(500)
#define MEMORY_HANDLES_MAX	(200)
#define MEMORY_TYPES_MAX (8)

#define MEMORY_WORKSPACES_MAX (5)
#define WORKSPACE_STACK_MAX	(8)

#define SMALL_FISH	(5*1024)			/* Too small to manage */

/* Debugging fill patterns */

#define MEM_FILLER_1 (0xC0DEDBAD)		/* Free memory */
#define MEM_FILLER_2 (0xDEADFACE)		/* At start and end of block */
#define MEM_FILLER_3 (0xABADCAFE)	/* In allocated block */

/* Error codes */

#define MEMERR_OUT_OF_ENTRIES					(1)
#define MEMERR_OUT_OF_HANDLES 				(2)
#define MEMERR_HANDLE_MISMATCH				(3)
#define MEMERR_NO_AREAS							(4)
#define MEMERR_ENTRY_OUTSIDE_ARRAY			(5)
#define MEMERR_NULL_FILE_TYPE					(6)
#define MEMERR_FREE_ENTRY_IN_CHAIN			(7)
#define MEMERR_AREA_IN_CHAIN					(8)
#define MEMERR_WRONG_BACKWARD_LINK			(9)
#define MEMERR_BLOCK_OUTSIDE_AREA			(10)
#define MEMERR_GAP_BETWEEN_BLOCKS			(11)
#define MEMERR_WRONG_AREA_SIZE				(12)
#define MEMERR_OUT_OF_MEMORY					(13)
#define MEMERR_OUT_OF_WORKSPACES				(14)
#define MEMERR_WORKSPACE_STACK_OVERFLOW	(15)
#define MEMERR_WORKSPACE_STACK_UNDERFLOW	(16)

#define MEMERR_MAX (16)

/*
 ** Type definitions *******************************************************
 */

typedef UNBYTE BOOL;

typedef struct Memory_handle *MEM_HANDLE;
typedef MEM_HANDLE (*MEM_Alloc_pass) (UNLONG, UNBYTE);
typedef void (*MEM_Relocator) (MEM_HANDLE, UNBYTE*, UNBYTE*, UNLONG);

/*
 ** Structure definitions **************************************************
 */

struct Memory_entry {
	UNBYTE *Start;								/* Start address */
	UNLONG Size;  								/* Size, 0 means
free block */
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

struct Memory_workspace {
	UNSHORT Flags;								/* See below */
	UNSHORT Nr_of_areas;						/* 0 means free workspace */
	struct Memory_workspace *Parent_workspace;	/* NULL means original workspace */
	MEM_HANDLE Workspace_handle; 			/* NULL means original workspace */
	void (*Alloc_init)(void);
	void (*Alloc_exit)(void);
	void (*Out_of_memory)(void);
	MEM_Alloc_pass *Pass_list;
	struct Memory_entry Areas[MEMORY_AREAS_MAX];
	struct File_type Workspace_ftype;	/* The file type of this workspace */
	struct File_type *Default_ftype;		/* The default file type for this
													workspace */
};

/* Bit masks for Memory_workspace.Flags */
#define MEM_NO_MEMORY_MOVEMENT	(1<<0)	/* Garbage collection etc. not allowed */
#define MEM_NO_PERSISTENCE			(1<<1)	/* No persistent memory blocks */

struct Memory_error {
	UNSHORT Code;
};

/*
 ** Prototypes *************************************************************
 */

void MEM_Init_memory(void);
void MEM_Exit_memory(void);
void MEM_Reset_memory(void);
void MEM_Init_workspace(struct Memory_workspace *Workspace);
void MEM_Reset_workspace(struct Memory_workspace *Workspace);
void MEM_Init_area(struct Memory_entry *Area);

void MEM_Error(UNSHORT Error_code);
void MEM_Print_error(UNCHAR *buffer, UNBYTE *data);
void MEM_Default_out_of_memory(void);

struct Memory_entry *MEM_Find_free_entry(void);
void MEM_Add_entry(struct Memory_entry *New, struct Memory_entry *Target);
void MEM_Delete_entry(struct Memory_entry *Target);

void MEM_Clear_all_handles(void);
void MEM_Clear_all_claims(void);
UNBYTE *MEM_Get_pointer(MEM_HANDLE Handle);
UNBYTE *MEM_Claim_pointer(MEM_HANDLE Handle);
void MEM_Free_pointer(MEM_HANDLE Handle);
MEM_HANDLE MEM_Create_memory_handle(struct Memory_entry *New_entry);
void MEM_Destroy_memory_handle(struct Memory_entry *Target);

UNLONG MEM_Get_block_size(MEM_HANDLE Handle);
void MEM_Resize_memory(MEM_HANDLE Handle, UNLONG New_size);
void MEM_Free_memory(MEM_HANDLE Handle);
void MEM_Kill_memory(MEM_HANDLE Handle);
void MEM_Invalidate_memory(MEM_HANDLE Handle);
void MEM_Clear_memory(MEM_HANDLE Handle);

void MEM_Relocate(MEM_HANDLE Handle, UNBYTE *Source, UNBYTE *Target, UNLONG Size);
struct Memory_workspace *MEM_Add_workspace (UNLONG Size, UNBYTE Memory_type);
void MEM_Delete_workspace (struct Memory_workspace *New);

void MEM_Push_workspace (struct Memory_workspace *New);
void MEM_Pop_workspace (void);
void MEM_Change_workspace (struct Memory_workspace *New);

BOOL MEM_Inquire_memory(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Allocate_memory(UNLONG Size);
MEM_HANDLE MEM_Do_allocate(UNLONG Size, UNLONG Findex, struct File_type *Ftype);
void MEM_Armageddon(void);
MEM_HANDLE MEM_Allocate_block(UNLONG Size, struct Memory_entry *Entry);
void MEM_Collect_garbage(struct Memory_entry *Area);
struct Memory_entry *MEM_Relocate_memory_block(struct Memory_entry *Area, struct Memory_entry
*Source);
struct Memory_entry *MEM_Find_LFB_in_garbage(struct Memory_entry *Area, struct Memory_entry
*Last);
struct Memory_entry *MEM_Move_memory_block(struct Memory_entry *Source, struct Memory_entry
*Target);

struct Memory_entry *MEM_Find_LFB(struct Memory_entry *Area);
UNLONG MEM_Calculate_TFM(struct Memory_entry *Area);
UNLONG MEM_Calculate_EMG(struct Memory_entry *Area, UNSHORT Sea_level);
void MEM_Drown_memory(struct Memory_entry *Area, UNSHORT Sea_level);
struct Memory_entry *MEM_Merge_memory_block(struct Memory_entry *Merged);
void MEM_Split_memory_block(struct Memory_entry *Split, UNLONG New_size);
void MEM_Validate_handles(void);
void MEM_Kill_unclaimed_memory(void);

MEM_HANDLE MEM_Alloc_pass_standard(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Alloc_pass_garbage(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Alloc_pass_drown(UNLONG Size, UNBYTE Type);
MEM_HANDLE MEM_Alloc_pass_juggle(UNLONG Size, UNBYTE Type);

MEM_HANDLE MEM_Reallocate_memory(UNLONG Findex, struct File_type *Ftype);
MEM_HANDLE MEM_Duplicate_memory(UNLONG Findex, struct File_type *Ftype);

MEM_HANDLE MEM_Search_persistent_block(UNLONG Findex, struct File_type *Ftype);
BOOL MEM_Do_reallocate(MEM_HANDLE New);

struct Memory_entry *MEM_Find_memory_area(struct Memory_entry *Entry);
struct Memory_workspace *MEM_Find_memory_workspace(struct Memory_entry *Area);

void MEM_Check_memory(void);
void MEM_Check_workspace(struct Memory_workspace *Workspace);
void MEM_Check_area(struct Memory_entry *Area);
void MEM_List(void);

#endif

