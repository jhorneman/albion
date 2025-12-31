/************
 * NAME     : BBMEMVAR.H
 * AUTHOR   : Jurie Horneman, BlueByte
 * START    : 1-6-1994
 * PROJECT  : Blue Byte memory manager V (Son of garbage collector)
 * NOTES    :
 * SEE ALSO : BBMEMVAR.H, BBMEM.C, BBMEM.H
 ************/

/* global variable external definitions */

extern struct Memory_entry MEM_Entries[MEMORY_ENTRIES_MAX];
extern struct Memory_handle MEM_Handles[MEMORY_HANDLES_MAX];
extern struct Memory_workspace MEM_Workspaces[MEMORY_WORKSPACES_MAX];

extern struct Memory_workspace MEM_First_workspace;

extern struct Memory_entry *MEM_Quick_free_entry;

extern struct Memory_workspace *MEM_Workspace_stack[WORKSPACE_STACK_MAX];
extern UNSHORT MEM_Workspace_stack_index;

extern UNBYTE MEM_Initialized;
extern UNBYTE MEM_Debugging;
extern UNBYTE MEM_Debugging2;
extern UNBYTE MEM_Handles_invalid;

extern struct File_type MEM_Default_file_type;

extern MEM_Alloc_pass *MEM_Pass_list_ptr;
extern MEM_Alloc_pass MEM_Default_pass_list[];

extern UNCHAR MEM_Library_name[];

extern UNCHAR *MEM_Error_strings[];

