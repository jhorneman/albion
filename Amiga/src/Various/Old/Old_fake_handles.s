

;*****************************************************************************
; [ Create a fake memory handle ]
;   IN : a0 - Pointer to memory (.l)
;  OUT : d0 - Fake memory handle (.b)
; Changed registers : d0
;*****************************************************************************
Create_fake_handle:
	movem.l	d7/a1,-(sp)
	move.w	#Max_handles,d0		; Seek free handle
	lea.l	Fake_handles,a1
	moveq.l	#Max_fake_handles-1,d7
.Loop:	tst.l	(a1)+			; Free ?
	bne.s	.Next
	move.l	a0,-4(a1)			; Yes -> insert address
	bra.s	.Exit
.Next:	addq.b	#1,d0			; Next handle
	dbra	d7,.Loop
	move.l	#NOT_ENOUGH_FAKE_HANDLES,Return_value	; Error !
	jmp	Exit_program
.Exit:	movem.l	(sp)+,d7/a1
	rts

;*****************************************************************************
; [ Destroy a fake memory handle ]
;   IN : d0 - Fake memory handle (.b)
; All registers are restored
;*****************************************************************************
Destroy_fake_handle:
	movem.l	d0/a0,-(sp)
	lea.l	Fake_handles,a0		; Clear entry
	and.w	#$00ff,d0
	sub.w	#Max_handles,d0
	lsl.w	#2,d0
	clr.l	0(a0,d0.w)
	movem.l	(sp)+,d0/a0
	rts

;*****************************************************************************
; [ Claim a memory block ]
;   IN : d0 - Memory handle (.b)
;  OUT : d0 - Pointer to memory block (.l)
; Changed registers : d0
;*****************************************************************************
Claim_pointer:
	move.l	a0,-(sp)
	and.w	#$00ff,d0			; Real or fake ?
	cmp.w	#Max_handles,d0
	bmi.s	.Real
	lea.l	Fake_handles,a0		; Get address
	sub.w	#Max_handles,d0
	and.w	#$00ff,d0
	lsl.w	#2,d0
	move.l	0(a0,d0.w),d0
	bra	.Exit
.Real:	jsr	Find_entry		; Find entry
	bne.s	.Exit
	cmp.b	#-1,Block_claim(a0)		; Claimed too often ?
	beq.s	.Skip
	addq.b	#1,Block_claim(a0)		; Claim
.Skip:	move.l	Block_start(a0),d0		; Get start
.Exit:	move.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Free a pointer to a memory block ]
;   IN : d0 - Memory handle (.b)
; All registers are	restored
;*****************************************************************************
Free_pointer:
	move.l	a0,-(sp)
	and.w	#$00ff,d0			; Real or fake ?
	cmp.w	#Max_handles,d0
	bpl.s	.Exit
	jsr	Find_entry		; Find entry
	bne.s	.Exit
	tst.b	Block_claim(a0)		; Not claimed ?
	beq.s	.Exit
	subq.b	#1,Block_claim(a0)		; Free
.Exit:	move.l	(sp)+,a0
	rts

	lea.l	Fake_handles,a0		; Clear fake handles
	moveq.l	#Max_fake_handles-1,d7
.Loop3:	clr.l	(a0)+
	dbra	d7,.Loop3


Fake_handles:	ds.l Max_fake_handles
