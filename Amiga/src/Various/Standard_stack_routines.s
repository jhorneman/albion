; General stack handling routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 17-1-1994

; Notes :
;   - Just copy these and replace "xx" by the name of the data-structure.

	XDEF	Push_xx
	XDEF	Pop_xx
	XDEF	Reset_xx_stack
	XDEF	Change_xx

;*****************************************************************************
; [ Push a xx on the stack ]
;   IN : a0 - Pointer to xx (.l)
; All registers are	restored
;*****************************************************************************
Push_xx:
	move.l	a1,-(sp)
	movea.l	xx_Sp,a1
	addq.l	#4,a1
	cmpa.l	#xxStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,xx_Sp
	jsr	Init_xx			; Initialize new xx
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a xx from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_xx:
	move.l	a0,-(sp)
	movea.l	xx_Sp,a0
	cmpa.l	#xxStack_start,a0		; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,xx_Sp
	move.l	(a0),a0			; Initialize old xx
	jsr	Init_xx
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the xx stack ]
; All registers are	restored
;*****************************************************************************
Reset_xx_stack:
	movem.l	a0/a1,-(sp)
	lea.l	Default_xx,a0		; Reset stack
	lea.l	xxStack_start,a1
	move.l	a1,xx_Sp
	move.l	a0,(a1)
	jsr	Init_xx			; Initialize default xx
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Change the current xx ]
;   IN : a0 - Pointer to xx (.l)
; All registers are	restored
;*****************************************************************************
Change_xx:
	move.l	a1,-(sp)
	movea.l	xx_Sp,a1
	cmp.l	(a1),a0			; Change ?
	beq.s	.Skip
	move.l	a0,(a1)			; Change !
	jsr	Init_xx			; Initialize new xx
.Skip:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Initialize a xx ]
;   IN : a0 - Pointer to xx (.l)
; All registers are	restored
;*****************************************************************************
Init_xx:
	rts

Default_xx:

xx_Sp:	ds.l 1				; xx stack
xxStack_start:
	ds.l Max_xx
xxStack_end:
