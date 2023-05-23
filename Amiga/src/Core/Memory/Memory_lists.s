; Memory Manager IV (Son Of Chip-mem)
; Memory list handling
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 26-1-1994

	SECTION	Program,code
;*****************************************************************************
; [ Find a free entry in the memory lists ]
;  OUT : a0 - Pointer to free entry (.l)
; Changed registers : a0
; NOTES :
;   - The flags, the handle, the claim and the file index are cleared.
;   - A quick scan method is implemented. Whenever an entry is deleted,
;     it's address is marked.
;*****************************************************************************
Find_free_entry:
	movem.l	d0/d7,-(sp)
	tst.l	Quick_free_block		; One available ?
	beq.s	.Do
	movea.l	Quick_free_block,a0		; Get it
	clr.l	Quick_free_block
	tst.l	(a0)			; Really free ?
	beq	.Found
.Do:	moveq.l	#0,d0			; Sea level = 0
.Again:	lea.l	Memory_lists,a0		; Search
	move.w	#Max_blocks-1,d7
.Loop1:	tst.l	(a0)			; Free block ?
	beq	.Found
	lea.l	Block_data_size(a0),a0	; Next block
	dbra	d7,.Loop1
	lea.l	Memory_areas,a0		; Drown all areas
	move.w	Number_of_areas,d7
	bra.s	.Entry2
.Loop2:	jsr	Drown_memory
	lea.l	Area_data_size(a0),a0
.Entry2:	dbra	d7,.Loop2
	addq.b	#1,d0			; Flood
	bne.s	.Again
	move.l	#NOT_ENOUGH_BLOCKS,d0	; Error
	jmp	Fatal_error
.Found:	clr.l	Block_info(a0)		; Erase info
.Exit:	movem.l	(sp)+,d0/d7
	rts

;*****************************************************************************
; [ Insert an entry in the memory lists ]
;   IN : a0 - Pointer to new entry (.l)
;        a1 - Pointer to entry AFTER which the new entry will be
;              inserted (.l)
; All registers are restored
; NOTE :
;  - a1 points to the entry AFTER which the new entry will be inserted,
;    so it is possible to add entries to the start by having a1 point to
;    the area descriptor.
;*****************************************************************************
Add_entry:
	move.l	a2,-(sp)
	movea.l	Block_next(a1),a2		; Get next
	cmpa.l	#0,a2			; End ?
	beq.s	.Skip
	move.l	a0,Block_previous(a2)	; Link new to next
.Skip:	move.l	a2,Block_next(a0)
	move.l	a1,Block_previous(a0)	; Link previous to new
	move.l	a0,Block_next(a1)
	movea.l	(sp)+,a2
	rts

;*****************************************************************************
; [ Delete an entry from the memory lists ]
;   IN : a0 - Pointer to entry that must be deleted (.l)
; All registers are restored
; NOTE :
;   - A quick scan method is implemented. Whenever an entry is deleted,
;     it's address is marked.
;   - This routine does not take care of any memory block logic.
;*****************************************************************************
Delete_entry:
	movem.l	a1/a2,-(sp)
	movea.l	Block_previous(a0),a1	; Get next & previous
	movea.l	Block_next(a0),a2
	cmpa.l	#0,a2
	beq.s	.Skip
	move.l	a1,Block_previous(a2)	; Link previous to next
.Skip:	move.l	a2,Block_next(a1)
	clr.l	(a0)			; Clear entry
	move.l	a0,Quick_free_block		; Store
	movem.l	(sp)+,a1/a2
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_BSS,bss
Quick_free_block:	ds.l 1
