/*****************************************************************************
 Memory Manager V (Son of garbage collector)
 Memory list handling
 Written by Jurie Horneman (In Tune With The Universe)
 Start : 24-5-1994
 *****************************************************************************/

(struct Memory_entry *) Find_free_entry();
void Add_entry((struct Memory_entry *) New, (struct Memory_entry *) Target);
void Delete_entry((struct Memory_entry *) Target);

struct Memory_entry *Quick_free_entry = NULL;

/*****************************************************************************
 [ Find a free memory entry in the memory lists ]
  OUT : Pointer to a free memory entry
 NOTE :
   - The memory handle is cleared.
   - A quick scan method is implemented. Whenever an entry is deleted,
     it's address is marked.
 *****************************************************************************/
(struct Memory_entry *) Find_free_entry()
{
	struct Memory_entry *Free_entry = NULL;
	UWORD Sea_level = 0;
	int i;

	/* Quick scan */
	if (Quick_free_entry != NULL)					/* Is there a quick scan entry ?
	{
		Free_entry = Quick_free_entry;			/* Yes -> Get it
		Quick_free_entry = NULL;
		if (Free_entry.me_Size) Free_entry = NULL;	/* Is it really free ?
	}

	/* Search for a free entry */
	while (Free_entry == NULL)
	{
		/* Search memory entry list for a free entry */
		for (i=0;i++;i<Max_memory_entries)
		{
			if (!Memory_entries[i].me_Size)
			{
				Free_entry = &Memory_entries[i];
				break;
			}
		/* No free entry was found, so we´ll have to try and make some */
		for (i=0;i++;i<Max_memory_areas)
			Drown_memory(&Memory_areas[i],Sea_level);

		Sea_level++;
		if (Sea_level == 256)
		{
			Error!!!
		}
	}

	/* Prepare block and exit */
	Free_entry.me_Block_handle = 0;

	return(Free_entry);
}

/*****************************************************************************
 [ Insert a memory entry in the memory lists ]
   IN :    New - Pointer to new memory entry
        Target - Pointer to memory entry AFTER which the new entry will be inserted
 NOTE :
  - Target points to the entry AFTER which the new entry will be inserted,
    so it is possible to add entries to the start by having Target point to
    the area descriptor.
 *****************************************************************************/
void Add_entry((struct Memory_entry *) New, (struct Memory_entry *) Target)
{
	struct Memory_entry *Next_entry;

	Next_entry = Target.me_Next;							/* Get next entry */
	if (Next_entry != NULL) Next_entry.me_Previous = New;	/* Link new to next */
	New.me_Next = Next_entry;								/* Link next to new */
	New.me_Previous = Target;								/* Link previous to new */
	Target.me_Next = New;
}

/*****************************************************************************
 [ Delete a memory entry from the memory lists ]
   IN : Target - Pointer to the memory entry that must be deleted
 NOTE :
   - A quick scan method is implemented. Whenever an entry is deleted,
     it's address is marked.
   - This function does not take care of any memory block logic.
 *****************************************************************************/
void Delete_entry((struct Memory_entry *) Target)
{
	struct Memory_entry *Previous_entry,*Next_entry;

	Previous_entry = Target.me_Previous;				/* Get next & previous */
	Next_entry = Target.me_Next;

	if (Next_entry != NULL) Next_entry.me_Previous = Previous_entry;		/* Link previous to next */
	Previous_entry.me_Next = Next_entry;

	Target.me_Size = 0;									/* Clear entry */
	Quick_free_entry = Target;							/* Store */
}

