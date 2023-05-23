; Memory Manager IV (Son Of Chip-mem)
; Self-check routines
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 26-1-1994

	SECTION	Program,code
;*****************************************************************************
; [ Age the priorities of all de-allocated blocks ]
; All registers are	restored
;*****************************************************************************
Update_memory_time:
	movem.l	d0/d7/a0/a1,-(sp)
; --------- Check memory --------------------------
	addq.l	#1,Memory_time		; Update time
	move.w	Check_counter,d0		; Time to check ?
	beq.s	.No
	subq.w	#1,d0
	bne.s	.No
	jsr	Check_memory		; Check
	move.w	#Check_frequency,d0		; Reset timer
.No:	move.w	d0,Check_counter
; --------- Age all de-allocated blocks -----------
	lea.l	Memory_lists,a0		; All blocks
	move.w	#Max_blocks-1,d7
.Loop:	btst	#Allocated,Block_flags(a0)	; Allocated ?
	bne.s	.Next
	jsr	Find_file_info		; No -> Is it a file ?
	bne.s	.Next
	move.b	File_priority(a1),d0	; Yes -> Age priority
	subq.b	#1,d0
	beq.s	.Next			; Zero ?
	move.b	d0,File_priority(a1)	; No -> store
.Next:	lea.l	Block_data_size(a0),a0	; Next block
	dbra	d7,.Loop
	movem.l	(sp)+,d0/d7/a0/a1
	rts

;*****************************************************************************
; [ Check memory ]
; All registers are restored
;*****************************************************************************
Check_memory:
	movem.l	d0/d1/d6/d7/a0/a1,-(sp)
; --------- Clear CHECKED flags -------------------
	lea.l 	Memory_lists,a0		; All entries
	move.w	#Max_blocks-1,d7
.Loop1:	bclr	#Checked,Block_flags(a0)	; Clear flag
	lea.l	Block_data_size(a0),a0
	dbra	d7,.Loop1
; --------- Check areas ---------------------------
	lea.l	Memory_areas,a0		; All areas
	move.w	Number_of_areas,d7
	bra.s	.Entry2
.Loop2:	jsr	Check_area		; Check
	lea.l	Area_data_size(a0),a0
.Entry2:	dbra	d7,.Loop2
.Exit:	movem.l	(sp)+,d0/d1/d6/d7/a0/a1
	rts

;*****************************************************************************
; [ Check area ]
;   IN : a0 - Pointer to area (.l)
; All registers are restored
; NOTE :
;  - This routine checks :
;     if all memory entries lie within the memory lists area,
;     if all memory blocks lie within the memory area,
;     if no circular links or links to other areas exist, and
;     if all blocks form one whole.
;*****************************************************************************
Check_area:
	movem.l	d0-d5/d7/a0,-(sp)
	move.l	Area_start(a0),d0		; Get start & end of area
	move.l	d0,d1
	move.l	d0,d5
	add.l	Area_size(a0),d1
	move.l	Area_size(a0),d2		; Get area size
	move.l	#Memory_lists,d3		; Get start & end of lists
	move.l	#End_of_memory_lists,d4	
.Again:	move.l	Block_next(a0),d7		; End of chain ?
	beq.s	.End
	cmp.l	d3,d7			; Too small ?
	bmi	.Error
	cmp.l	d4,d7			; Too large ?
	bpl	.Error
	move.l	d7,a0			; Next block
	btst	#Checked,Block_flags(a0)	; Already checked ?
	bne	.Error
	bset	#Checked,Block_flags(a0)	; Check !
	move.l	Block_start(a0),d7		; Get start of block
	cmp.l	d5,d7			; = end of previous block ?
	bne	.Error
	cmp.l	d0,d7			; Start under area ?
	bmi	.Error
	cmp.l	d1,d7			; Start over area ?
	bpl	.Error
	add.l	Block_size(a0),d7		; Get end of block
	move.l	d7,d5
	subq.l	#1,d7
	cmp.l	d0,d7			; End under area ?
	bmi.s	.Error
	cmp.l	d1,d7			; End over area ?
	bpl.s	.Error
	sub.l	Block_size(a0),d2		; Count down total size
	bra.s	.Again
.Error:	move.l	#AREA_CORRUPTED,d0		; Error !
	jmp	Fatal_error
.End:	tst.l	d2			; Total size OK ?
	bne.s	.Error
	cmp.l	d5,d1
	bne.s	.Error
	movem.l	(sp)+,d0-d5/d7/a0
	rts

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_BSS,bss
Check_counter:	ds.w 1
Memory_time:	ds.l 1
