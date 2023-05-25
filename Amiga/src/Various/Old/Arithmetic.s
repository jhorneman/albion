;*****************************************************************************
; [ 32-bit unsigned multiplication ]
;   IN : d0 - Value 1 (.l)
;        d1 - Value 2 (.l)
;  OUT : d0 - Value 1 x Value 2 (.l)
; Changed registers : d0
;*****************************************************************************
Unsigned_32_bit_multiply:
	movem.l	d1/d3-d5,-(sp)
	move.w	d0,d3			; d3 = B x D
	mulu.w	d1,d3
	move.w	d0,d5
	swap	d0
	move.w	d0,d4
	mulu.w	d1,d4			; d4 = A x D
	swap	d1
	mulu.w	d1,d5			; d5 = B x C
	add.w	d5,d4
	swap	d3
	add.w	d3,d4
	move.w	d4,d3
	swap	d3
	move.l	d3,d0
	movem.l	(sp)+,d1/d3-d5
	rts

;*****************************************************************************
; [ 32-bit signed multiplication ]
;   IN : d0 - Value 1 (.l)
;        d1 - Value 2 (.l)
;  OUT : d0 - Value 1 x Value 2 (.l)
; Changed registers : d0
;*****************************************************************************
Signed_32_bit_multiply:
	movem.l	d1-d5,-(sp)
	moveq.l	#0,d2			; Clear negate flag
	tst.l	d0			; Value 1 negative ?
	bge.s	.Nomin1
	neg.l	d0			; Make positive
	addq.w	#1,d2			; Flag negation
.Nomin1:	tst.l	d1			; Value 2 negative ?
	bge.s	.Nomin2
	neg.l	d1			; Make positive
	addq.w	#1,d2			; Flag negation
.Nomin2:	move.w	d0,d3			; Calculate product
	mulu.w	d1,d3
	move.w	d0,d5
	swap	d0
	move.w	d0,d4
	mulu.w	d1,d4
	swap	d1
	mulu.w	d1,d5
	add.w	d5,d4
	swap	d3
	add.w	d3,d4
	move.w	d4,d3
	swap	d3
	move.l	d3,d0
	btst	#0,d2			; Negate ?
	beq.s	.Exit
	neg.l	d0			; Negate
.Exit:	movem.l	(sp)+,d1-d5
	rts
