/*****************************************************************************
 Memory Manager V (Son of garbage collector)
 Memory handle management
 Written by Jurie Horneman (In Tune With The Universe)
 Start : 24-5-1994
 Notes :
   - A memory handle cannot be zero. The functions will accept this, however.
   - It is VITAL that [ Claim_pointer ] return zero when handle zero is
     claimed.
 *****************************************************************************/

/*****************************************************************************
  [ Clear all memory handles ]
 *****************************************************************************/
void Clear_all_handles()
{
	int i;

	for (i=0;i++;i<Max_memory_handles) Memory_handles[i].mh_Memory_entry = NULL;
}

/*****************************************************************************
 [ Clear all memory claims ]
 *****************************************************************************/
void Clear_all_claims()
{
	int i;

	for (i=0;i++;i<Max_memory_handles) Memory_handles[i].mh_Claim_counter = 0;
}

/*****************************************************************************
 [ Get an UNCLAIMED pointer to a memory block ]
   IN : Mem_handle - Memory handle
  OUT : Pointer to memory
 *****************************************************************************/
UBYTE *Get_pointer(UWORD Mem_handle)
{
	struct Memory_entry *Claimed_entry;

	if (!Memory_handle) return(0L);					/* Return 0 for handle 0 */

	Claimed_entry = Find_entry(Mem_handle);			/* Find the memory entry */

	return(Claimed_entry->Block_start);				/* Return pointer */
}

/*****************************************************************************
 [ Claim a memory block ]
   IN : Mem_handle - Memory handle
  OUT : Pointer to memory
 *****************************************************************************/
UBYTE *Claim_pointer(UWORD Mem_handle)
{
	struct Memory_entry *Claimed_entry;

	if (!Memory_handle)	return(0L);					/* Return 0 for handle 0 */

	Claimed_entry = Find_entry(Mem_handle);			/* Find the memory entry */

	/* If possible, increase the claim counter */
	if (Claimed_entry->Claim_counter != 255) Claimed_entry->Claim_counter++;

	return(Claimed_entry->Block_start);				/* Return pointer */
}

/*****************************************************************************
 [ Free a pointer to a memory block ]
   IN : Mem_handle - Memory handle
 *****************************************************************************/
void Free_pointer(UWORD Mem_handle)
{
	struct Memory_entry *Freed_entry;

	if (Memory_handle)
	{
		Claimed_entry = Find_entry(Mem_handle);			/* Find the memory entry */

		/* If possible, decrease the claim counter */
		if (Claimed_entry->Claim_counter) Claimed_entry->Claim_counter--;
	}
}

/*****************************************************************************
 [ Create a new memory handle ]
   IN : New_entry - Pointer to memory entry
  OUT : Memory handle
  Notes :
    - The memory handle is automatically inserted in the memory entry
 *****************************************************************************/
UWORD Create_memory_handle(struct Memory_entry *New_entry)
{
	struct Memory_handle *Free_handle = NULL;
	UWORD New_handle = 0,Sea_level = 0;
	int i;

	if (New_entry->me_Block_handle)				/* Exit if the entry already has a handle */
		return(New_entry->me_Block_handle);
	else
	{
		if (!New_entry->me_Claim_counter)		/* Return 0 if the entry is claimed */
		{
			/* Search for a free handle */
			while (Free_handle == NULL)
			{
				/* Search memory handle list for a free handle */
				for (i=0;i++;i<Max_memory_handles)
				{
					if (!Memory_handles[i].mh_Memory_entry)
					{
						Free_handle = &Memory_handles[i];
						break;
					}
				/* No free handle was found, so we´ll have to try and make some */
				for (i=0;i++;i<Max_memory_areas)
					Drown_memory(&Memory_areas[i],Sea_level);

				Sea_level++;
				if (Sea_level == 256)
				{
					Error!!!
				}
			}
		/* Initialize memory handle structure */
		Free_entry->mh_Allocated = 0;
		Free_entry->mh_Invalid = 0;
		Free_entry->mh_Checked = 0;
		Free_entry->mh_Low_nibble = 0;
		Free_entry->mh_Priority = 0;
		Free_entry->mh_Claim_counter = 0;
		Free_entry->mh_Load_counter = 0;
		Free_entry->mh_File_ID = 0;
		Free_entry->mh_File_index = 0;
		Free_entry->mh_Memory_entry = New_entry;

		/* Insert index in memory entry */
		New_entry->me_Block_handle = i+1;

		return(i+1);		
		}
	}
}

/*****************************************************************************
 [ Destroy a memory handle ]
   IN : _entry - Pointer to memory entry
 *****************************************************************************/
UWORD Destroy_memory_handle(struct Memory_entry *New_entry)
{
	struct Memory_handle *Free_handle = NULL;

;*****************************************************************************
; [ Destroy a memory handle ]
;   IN : a0 - Pointer to memory entry (.l)
; All registers are restored
;*****************************************************************************
Destroy_memory_handle:
	movem.l	d0/a0,-(sp)
	tst.b	Block_claim(a0)		; Claimed ?
	bne.s	.Exit
	moveq.l	#0,d0			; Any handle ?
	move.b	Block_handle(a0),d0
	beq.s	.Exit
	clr.b	Block_handle(a0)		; Yes -> Destroy entry
	lea.l	Memory_handles,a0		; Remove handle
	lsl.w	#2,d0
	clr.l	-4(a0,d0.w)
.Exit:	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Find a memory entry belonging to a handle ]
;   IN : d0 - Memory handle (.b)
;  OUT : a0 - Pointer to memory entry (.l)
;        eq - Found
;        ne - Not found
; Changed registers : a0
; Notes :
;   - This routine uses a self-repairing quick access list.
;*****************************************************************************
Find_entry:
	movem.l	d0-d2/a2,-(sp)
	moveq.l	#-1,d1			; Default is not found
	and.w	#$00ff,d0			; Mask off
	beq.s	.Exit
	lea.l	Memory_handles,a2		; Get pointer
	move.l	-4(a2,d0.w*4),d2
	beq.s	.No			; Any ?
	move.l	d2,a0			; Yes
	cmp.b	Block_handle(a0),d0		; Is it the right one ?
	beq.s	.Ok
.No:	movem.l	d7/a1,-(sp)
	lea.l	Memory_areas,a1		; No -> Search all areas
	move.w	d0,d2
	lsl.w	#2,d2
	add.w	d2,a2
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	a1,a0			; Search area
.Again:	move.l	Block_next(a0),d2		; End of chain ?
	beq.s	.Next
	move.l	d2,a0
	cmp.b	Block_handle(a0),d0		; Is this the one ?
	bne.s	.Again
	move.l	a0,(a2)			; Yes -> Correction
	bra.s	.Ok
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d7/a1
	bra.s	.Exit
.Ok:	moveq.l	#0,d1			; Yay!
.Exit:	tst.w	d1			; Found ?
	movem.l	(sp)+,d0-d2/a2
	rts

;*****************************************************************************
; [ Find the file info belonging to a memory entry ]
;   IN : a0 - Pointer to memory entry (.l)
;  OUT : a1 - Pointer to file info (.l)
;        eq - Found
;        ne - Not found
; Changed registers : a1
;*****************************************************************************
Find_file_info:
	movem.l	d0-d2,-(sp)
	moveq.l	#-1,d1			; Default is not found
	moveq.l	#0,d0			; Is this a file ?
	move.b	Block_file_index(a0),d0
	beq.s	.Exit
	lea.l	File_infos,a1		; Yes -> Get file info
	subq.w	#1,d0
	mulu.w	#File_data_size,d0
	add.w	d0,a1
	moveq.l	#0,d1			; Success !
.Exit:	tst.w	d1
	movem.l	(sp)+,d0-d2
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_BSS,bss
Memory_handles:	ds.l Max_handles
