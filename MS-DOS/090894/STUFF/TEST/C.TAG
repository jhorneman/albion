ÄÄ´ BBMEM.C       ÃÄÄÄÄÄÄ´ 09.06.94 17:35 ÃÄÄÄÄÄÄÄþDT=482970722
MEM_Init_memory	void_proc	void
MEM_Exit_memory	void_proc	void
MEM_Reset_memory	void_proc	void
MEM_Init_workspace	void_proc	void
MEM_Init_area	void_proc	void
MEM_Error	void_proc	void
MEM_Print_error	void_proc	void
)	struct_type	struct Memory_entry *
MEM_Add_entry	struct_proc	MEM_Add_entry(struct Memory_entry *New, struct Memory_entry *Target)
MEM_Delete_entry	struct_proc	MEM_Delete_entry(struct Memory_entry *Target)
MEM_Clear_all_handles	struct_proc	MEM_Clear_all_handles(void)
MEM_Clear_all_claims	struct_proc	MEM_Clear_all_claims(void)
MEM_Get_pointer	struct_proc	MEM_Get_pointer(UNSHORT Mem_handle)
MEM_Claim_pointer	struct_proc	MEM_Claim_pointer(UNSHORT Mem_handle)
MEM_Free_pointer	struct_proc	MEM_Free_pointer(UNSHORT Mem_handle)
MEM_Create_memory_handle	struct_proc	MEM_Create_memory_handle(struct Memory_entry *New_entry)
MEM_Destroy_memory_handle	struct_proc	MEM_Destroy_memory_handle(struct Memory_entry *Target)
MEM_Find_entry	struct_proc	MEM_Find_entry(UNSHORT Mem_handle)
