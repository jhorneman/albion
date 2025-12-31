
/* Sizes and addresses will be aligned to this value.
   This must be a power of 2 and may not exceed 256 !!! */
#define ALIGNMENT	(8)

void BASE_Get_memory(struct Memory_entry *Area);
void BASE_Free_memory(struct Memory_entry *Area);

void MEM_Fill_memory(UNBYTE *Start, UNLONG Size, UNBYTE Value);
void MEM_Fill_memory_long(UNBYTE *Start, UNLONG Size, UNLONG Value);
void XXX_Copy_memory(UNBYTE *Source, UNBYTE *Target, UNLONG Size);

UNLONG MEM_Align(UNLONG Value);

