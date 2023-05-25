; Memory Manager IV (Son Of Chip-mem)
; Memory handle management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 26-1-1994

; Notes :
;   - A memory handle cannot be zero. The routines will accept this, however.
;   - It is VITAL that [ Claim_pointer ] return zero when handle zero is
;     claimed.

	SECTION	Program,code
;*****************************************************************************
; [ Clear all memory handles ]
; All registers are restored
;*****************************************************************************
Clear_all_handles:
	movem.l	d7/a0,-(sp)
	lea.l	Memory_handles,a0		; Clear handles
	move.w	#Max_handles-1,d7
.Loop:	clr.l	(a0)+
	dbra	d7,.Loop
	movem.l	(sp)+,d7/a0
	rts

;*****************************************************************************
; [ Clear all memory claims ]
; All registers are restored
;*****************************************************************************
Clear_all_claims:
	movem.l	d0/d7/a0/a1,-(sp)
	lea.l	Memory_areas,a1		; Search all areas
	move.w	Number_of_areas,d7
	bra.s	.Entry
.Loop:	move.l	a1,a0			; Search area
.Again:	move.l	Block_next(a0),d0		; End of chain ?
	beq.s	.Next
	move.l	d0,a0			; No
	clr.b	Block_claim(a0)		; Clear claim
	bra.s	.Again
.Next:	lea.l	Area_data_size(a1),a1	; Next area
.Entry:	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a0/a1
	rts

;*****************************************************************************
; [ Get an UNCLAIMED pointer to a memory block ]
;   IN : d0 - Memory handle (.b)
;  OUT : d0 - Pointer to memory (.l)
; Changed registers : d0
;*****************************************************************************
Get_pointer:
	move.l	a0,-(sp)
	tst.b	d0			; Zero ?
	bne.s	.Not_zero
	moveq.l	#0,d0			; Yes -> Return zero
	bra.s	.Exit
.Not_zero:	jsr	Find_entry		; No -> Find entry
	bne.s	.Exit
	move.l	Block_start(a0),d0		; Get start address
.Exit:	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Claim a memory block ]
;   IN : d0 - Memory handle (.b)
;  OUT : d0 - Pointer to memory (.l)
; Changed registers : d0
;*****************************************************************************
Claim_pointer:
	move.l	a0,-(sp)
	tst.b	d0			; Zero ?
	bne.s	.Not_zero
	moveq.l	#0,d0			; Yes -> Return zero
	bra.s	.Exit
.Not_zero:	jsr	Find_entry		; No -> Find entry
	bne.s	.Exit
	cmp.b	#-1,Block_claim(a0)		; Claimed too often ?
	beq.s	.Skip
	addq.b	#1,Block_claim(a0)		; No -> Claim
.Skip:	move.l	Block_start(a0),d0		; Get start address
.Exit:	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Free a pointer to a memory block ]
;   IN : d0 - Memory handle (.b)
; All registers are	restored
;*****************************************************************************
Free_pointer:
	move.l	a0,-(sp)
	jsr	Find_entry		; No -> Find entry
	bne.s	.Exit
	tst.b	Block_claim(a0)		; Claimed ?
	beq.s	.Exit
	subq.b	#1,Block_claim(a0)		; Yes -> Free
.Exit:	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Create a new memory handle ]
;   IN : a0 - Pointer to memory entry (.l)
;  OUT : d0 - Memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Create_memory_handle:
	movem.l	d7/a1,-(sp)
	move.b	Block_handle(a0),d0		; Already has a handle ?
	bne.s	.Exit
	moveq.l	#0,d0			; No
	tst.b	Block_claim(a0)		; Claimed ?
	bne.s	.Done
	lea.l	Memory_handles,a1		; No -> Search a free handle
	moveq.l	#1,d0
	move.w	#Max_handles-1,d7
.Loop:	tst.l	(a1)+			; Free ?
	beq.s	.Found
	addq.w	#1,d0			; No -> Next handle
	dbra	d7,.Loop
	move.l	#NOT_ENOUGH_HANDLES,d0	; Error !
	jmp	Fatal_error
.Found:	move.l	a0,-4(a1)			; Insert in list
.Done:	move.b	d0,Block_handle(a0)		; Insert handle
.Exit:	movem.l	(sp)+,d7/a1
	rts

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
