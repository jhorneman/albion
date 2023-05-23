; Dynamic Lists
; Written by J.Horneman (In Tune With The Universe)
; Start : 17-1-1994

	XDEF	Create_DL_chain
	XDEF	Destroy_DL_chain
	XDEF	Add_DL_entry
	XDEF	Delete_DL_entry
	XDEF	For_all_DL
	XDEF	Abort_for_all_DL

	SECTION	Program,code
;*****************************************************************************
; [ Create a DL-chain ]
;   IN : d0 - Initial number of entries (.w)
;        a0 - Pointer to DL-structure (.l)
; All registers are restored
; Notes :
;   - No checks for illegal input are made.
;*****************************************************************************
Create_DL_chain:
	movem.l	d0-d3/a1,-(sp)
	move.w	d0,d1
	move.w	DL_entries_per_block(a0),d2	; Calculate DLB size
	mulu.w	DL_entry_size(a0),d2
	add.l	#DLB_header_size,d2
	move.l	d2,d0			; Allocate first DLB
	jsr	Allocate_memory
	move.w	d0,DL_first(a0)		; Store handle
	move.w	d0,d3
.Again:	move.w	d3,d0
	jsr	Clear_memory		; Clear
	jsr	Claim_pointer		; Get address
	move.l	d0,a1
	sub.w	DL_entries_per_block(a0),d1	; Enough ?
	bmi.s	.Exit
	move.l	d2,d0			; No -> Allocate next DLB
	jsr	Allocate_memory
	move.w	d0,DLB_next(a1)		; Store handle
	exg.l	d0,d3
	jsr	Free_pointer
	bra.s	.Again
.Exit:	clr.w	DLB_next(a1)		; End of chain
	move.w	d3,d0
	jsr	Free_pointer
	neg.w	d1			; Store free entries
	move.w	d1,DL_free_in_last(a0)
	movem.l	(sp)+,d0-d3/a1
	rts

;*****************************************************************************
; [ Destroy a DL-chain ]
;   IN : a0 - Pointer to DL-structure (.l)
; All registers are restored
;*****************************************************************************
Destroy_DL_chain:
	movem.l	d0-d2/a1,-(sp)
	move.w	DL_first(a0),d1		; Get first handle
.Again:	move.w	d1,d0			; Get DLB address
	jsr	Claim_pointer
	move.l	d0,a1
	move.w	DLB_next(a1),d2		; Get handle of next DLB
	move.w	d1,d0			; Free DLB
	jsr	Free_pointer
	jsr	Free_memory
	move.w	d2,d1			; End of chain ?
	bne.s	.Again
	clr.w	DL_first(a0)		; Yes -> Clear
	clr.w	DL_free_in_last(a0)
	movem.l	(sp)+,d0-d2/a1
	rts

;*****************************************************************************
; [ Add an entry to a DL-chain ]
;   IN : a0 - Pointer to DL-structure (.l)
;  OUT : d0 - Current DLB memory handle (.b)
;        a0 - Pointer to new entry (.l)
; Changed registers : d0,a0
; Notes :
;   - After having inserted the new entry at a0, free the pointer in d0.
;*****************************************************************************
Add_DL_entry:
	movem.l	d1/d2/a1,-(sp)
; ---------- Find last DLB ------------------------
	move.w	DL_first(a0),d1		; Get first handle
.Again:	move.w	d1,d0			; Get DLB address
	jsr	Claim_pointer
	move.l	d0,a1
	move.w	DLB_next(a1),d2		; Get handle of next DLB
	beq.s	.Last			; End of chain ?
	move.w	d1,d0			; No -> Next DLB
	jsr	Free_pointer
	move.w	d2,d1
	bra.s	.Again
.Last:	move.w	DL_free_in_last(a0),d0	; Any free entries ?
	beq	.New
; ---------- New entry in last DLB ----------------
	subq.w	#1,DL_free_in_last(a0)	; Yes -> Count
	sub.w	DL_entries_per_block(a0),d0	; Get free entry address
	neg.w	d0
	mulu.w	DL_entry_size(a0),d0
	lea.l	DLB_header_size(a1,d0.l),a0
	move.w	d1,d0			; Output handle
	bra	.Exit
; ---------- New DLB is needed --------------------
.New:	move.w	DL_entries_per_block(a0),d0	; No -> Make new DLB
	subq.w	#1,d0
	move.w	d0,DL_free_in_last(a0)
	move.w	DL_entries_per_block(a0),d0	; Calculate DLB size
	mulu.w	DL_entry_size(a0),d0
	add.l	#DLB_header_size,d0
	jsr	Allocate_memory		; Allocate DLB
	jsr	Clear_memory		; Clear
	move.w	d0,DLB_next(a1)		; Store handle
	exg.l	d0,d1
	jsr	Free_pointer
	move.w	d1,d0			; Prepare new DLB
	jsr	Claim_pointer
	move.l	d0,a0
	clr.w	DLB_next(a0)
	add.w	#DLB_header_size,a0
	move.w	d1,d0			; Output handle
.Exit:	movem.l	(sp)+,d1/d2/a1
	rts

;*****************************************************************************
; [ Delete an entry from the end of a DL-chain ]
;   IN : a0 - Pointer to DL-structure (.l)
; All registers are restored
;*****************************************************************************
Delete_DL_entry:
	movem.l	d0-d3/a1,-(sp)
; ---------- Check first DLB ----------------------
	move.w	DL_first(a0),d1		; Check first DLB
	move.w	d1,d0
	jsr	Claim_pointer
	move.l	d0,a1
	tst.w	DLB_next(a1)		; Is last in chain ?
	bne.s	.Entry
	move.w	d0,d1			; Yes
	jsr	Free_pointer
	move.w	DL_free_in_last(a0),d0	; DLB empty ?
	cmp.w	DL_entries_per_block(a0),d0
	beq	.Exit
	addq.w	#1,d0			; No
	move.w	d0,DL_free_in_last(a0)
	bra	.Exit
; ---------- Find last DLB ------------------------
.Again:	move.w	d1,d0			; Get DLB address
	jsr	Claim_pointer
	move.l	d0,a1
.Entry:	move.l	d1,d3			; Save for later
	move.w	DLB_next(a1),d2		; Get handle of next DLB
	beq.s	.Last			; End of chain ?
	move.w	d1,d0			; No -> Next DLB
	jsr	Free_pointer
	move.w	d2,d1
	bra.s	.Again
; ---------- DLB empty now ? ----------------------
.Last:	move.w	DL_free_in_last(a0),d0	; DLB empty now ?
	addq.w	#1,d0
	cmp.w	DL_entries_per_block(a0),d0
	bpl.s	.Discard
	move.w	d0,DL_free_in_last(a0)	; No
	move.w	d1,d0
	jsr	Free_pointer
	bra	.Exit
; ---------- Discard DLB --------------------------
.Discard:	move.w	d1,d0			; Return memory
	jsr	Free_pointer
	jsr	Free_memory
	move.w	d2,d0			; Clear link
	jsr	Claim_pointer
	move.l	d0,a1
	clr.w	DLB_next(a1)
	move.w	d2,d0
	jsr	Free_pointer
	move.w	DL_entries_per_block(a0),DL_free_in_last(a0)	; Reset
.Exit:	movem.l	(sp)+,d0-d3/a1
	rts

;*****************************************************************************
; [ Execute a function for each entry in a DL-chain ]
;   IN : a0 - Pointer to DL-structure (.l)
;        a1 - Pointer to function (.l)
; All registers are restored
; Notes :
;   - The function will receive a pointer to the current DL entry in a0.
;   - This procedure can be aborted by calling [ Abort_for_all ].
;*****************************************************************************
For_all_DL:
	movem.l	d0/d7/a0-a2,-(sp)
	move.l	a0,a2
	sf	Abort_for_all_flag		; Clear flag
.Again:	move.w	d0,-(sp)			; Save handle
	jsr	Claim_pointer		; Get DLB address
	move.l	d0,a1
	move.l	a1,a0
	move.w	DL_entries_per_block(a2),d7	; Do all entries in DL-block
	bra.s	.Entry
.Loop:	movem.l	d0/d7/a1/a2,-(sp)		; Execute function
	jsr	(a2)
	movem.l	(sp)+,d0/d7/a1/a2
	tst.b	Abort_for_all_flag		; Aborted ?
	beq.s	.Next
	move.w	(sp)+,d0			; Yes -> Exit
	jsr	Free_pointer
	bra.s	.Exit
.Next:	add.w	DL_entry_size(a2),a0	; No -> Next entry
.Entry:	dbra	d7,.Loop
	move.w	(sp)+,d0			; Finished this block
	jsr	Free_pointer
	move.w	DLB_next(a1),d0		; Last block in chain ?
	bne.s	.Again
.Exit:	movem.l	(sp)+,d0/d7/a0-a2
	rts

;*****************************************************************************
; [ Abort For_all_DL ]
; All registers are restored
;*****************************************************************************
Abort_for_all_DL:
	st	Abort_for_all_flag		; Set flag
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_BSS,bss
Abort_for_all_flag:	ds.b 1
	even
