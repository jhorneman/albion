; Memory Manager IV (Son Of Chip-mem)
; Main
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 26-1-1994

	XDEF	Clear_all_claims
	XDEF	Get_pointer
	XDEF	Claim_pointer
	XDEF	Free_pointer
	XDEF	Reallocate_memory
	XDEF	Duplicate_memory
	XDEF	Allocate_CHIP
	XDEF	Allocate_FAST
	XDEF	Allocate_memory
	XDEF	File_allocate
	XDEF	Shrink_memory
	XDEF	Free_memory
	XDEF	Kill_memory
	XDEF	Clear_memory
	XDEF	Get_memory_length
	XDEF	Invalidate_memory
	XDEF	Kill_unclaimed_memory
	XDEF	Total_FAST_memory
	XDEF	Reset_memory
	XDEF	Init_memory
	XDEF	Exit_memory

; Notes :
;   - The first memory block in a memory list is the memory area descriptor.
;   - If Block_info is zero, this is a free block of memory, because :
;     File or memory	Allocated		Reason
;	Memory		No		Is automatically killed
;	File		No		Block_file_index <> 0
;	Memory		Yes		Block_flags <> 0
;	File		Yes		Block_file_index <> 0
;   - If a block is allocated and Block_file_index is zero, this is an
;     allocated block of memory and not a file.

	SECTION	Program,code
;*****************************************************************************
; [ Invalidate a memory block ]
;   IN : d0 - Memory handle (.b)
; All registers are restored
;*****************************************************************************
Invalidate_memory:
	move.l	a0,-(sp)
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	bset	#Invalid,Block_flags(a0)	; Invalidate
.Exit:	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Get the length of a memory block ]
;   IN : d0 - Memory handle (.b)
;  OUT : d0 - Length of memory block (.l)
; Changed registers : d0
;*****************************************************************************
Get_memory_length:
	movem.l	a0/a1,-(sp)
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	move.l	Block_size(a0),d0		; Get length
	jsr	Find_file_info		; Find file info
	bne.s	.Exit
	move.b	File_length_low(a1),d0	; Restore old length
.Exit:	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Shrink memory block ]
;   IN : d0 - Memory handle (.b)
;        d1 - New size of memory block (.l)
; All registers are restored
;*****************************************************************************
Shrink_memory:
	movem.l	d0/d1/a0/a1,-(sp)
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	jsr	Find_file_info		; Just a memory block ?
	bne.s	.Skip
	move.b	d1,File_length_low(a1)	; No -> Write new low byte
.Skip:	addq.l	#1,d1			; Force word boundary
	andi.b	#$fe,d1
	cmp.l	Block_size(a0),d1		; Same size ?
	bpl.s	.Exit
	move.l	d1,d0			; Split memory block
	jsr	Split_memory_block
	move.l	Block_next(a0),a0		; Try to merge
	jsr	Merge_memory_block	
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Free memory block ]
;   IN : d0 - Memory handle (.b)
; All registers are restored
; NOTE :
;  - A memory block (not a file) will be killed automatically.
;  - If the memory block is no longer claimed, the handle will be
;    destroyed.
;*****************************************************************************
Free_memory:
	movem.l	a0/a1,-(sp)
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	btst	#Allocated,Block_flags(a0)	; Already freed ?
	beq.s	.Exit
	tst.b	Block_claim(a0)		; No -> Claimed ?
	bne.s	.Exit
	jsr	Find_file_info		; No -> Find file info
	beq.s	.File			; Just memory ?
.Kill:	jsr	Kill_memory		; Yes -> Kill !
	bra.s	.Exit
.File:	tst.b	File_load_counter(a1)	; Count down
	beq.s	.Free
	subq.b	#1,File_load_counter(a1)
	bne.s	.Exit			; Loaded more than once ?
	btst	#Invalid,Block_flags(a0)	; No -> Invalid ?
	bne.s	.Kill
.Free:	jsr	Destroy_memory_handle	; No -> Destroy handle
	bclr	#Allocated,Block_flags(a0)	; De-allocate
.Exit:	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Kill memory block ]
;   IN : d0 - Memory handle (.b)
; All registers are restored
; NOTE :
;  - If the memory block is still claimed or loaded more than once, it
;    will be invalidated.
;*****************************************************************************
Kill_memory:
	movem.l	a0/a1,-(sp)
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	tst.b	Block_claim(a0)		; Claimed ?
	bne.s	.Invalid
	jsr	Find_file_info		; No -> Find file info
	bne.s	.Kill2			; Just memory ?
	cmp.b	#1,File_load_counter(a1)	; No -> Loaded more than once ?
	ble.s	.Kill1
.Invalid:	bset	#Invalid,Block_flags(a0)	; I'll get you next time...
	bra.s	.Exit
.Kill1:	clr.b	File_type(a1)		; Clear file info
.Kill2:	jsr	Destroy_memory_handle	; Destroy handle
	clr.l	Block_info(a0)		; Erase info
	jsr	Merge_memory_block		; Try to merge
.Exit:	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Clear a memory block ]
;   IN : d0 - Memory handle (.b)
; All registers are restored
;*****************************************************************************
Clear_memory:
	movem.l	d0/d1/a0,-(sp)
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	move.l	Block_size(a0),d0		; Clear
	moveq.l	#0,d1
	move.l	Block_start(a0),a0
	jsr	Fill_memory
.Exit:	movem.l	(sp)+,d0/d1/a0
	rts

;*****************************************************************************
; [ Find a free file info block ]
;  OUT : d0 - File index (.b)
;        a1 - Pointer to file info block (.l)
; Changed registers : d0,a1
;*****************************************************************************
Find_free_file_info_block:
	move.l	d7,-(sp)
	lea.l	File_infos,a1		; Search
	moveq.l	#1,d0
	move.w	#Max_files-1,d7
.Loop:	tst.b	File_type(a1)		; Free ?
	beq.s	.Exit
	addq.w	#1,d0			; No -> Next
	lea.l	File_data_size(a1),a1
	dbra	d7,.Loop
	move.l	#NOT_ENOUGH_FILES,d0	; Error !
	jmp	Fatal_error
.Exit:	move.l	(sp)+,d7
	rts

;***************************************************************************	
; The other sources
;***************************************************************************	

	incdir	DDT:
	include	Core/Memory/Memory_allocate.s
	include	Core/Memory/Memory_checks.s
	include	Core/Memory/Memory_handles.s
	include	Core/Memory/Memory_init.s
	include	Core/Memory/Memory_lists.s

;***************************************************************************	
; The DATA & BSS segments
;***************************************************************************	
	SECTION	Fast_BSS,bss
Number_of_areas:	ds.w 1
Memory_areas:	ds.b Max_areas*Area_data_size
	even

File_infos:	ds.b Max_files*File_data_size
	even

Memory_lists:	ds.b Max_blocks*Block_data_size
End_of_memory_lists:
	even
