;***************************************************************************
; [ Calculate the recolouring table for part of a palette ]
;   IN : d0 - First colour {0...255} (.w)
;        d1 - Number of colours {1...256} (.w)
;        a0 - Pointer to IFF colour list (.l)
;        a1 - Pointer to recolouring table (.l)
;        a2 - Pointer to recolouring function (.l)
; All registers are restored
; Notes :
;   - The recolouring function will receive the current red, green and blue
;     values in d0-d2 (.w) and should return the target values in these
;     same registers, leaving all other registers alone.
;***************************************************************************
Recolour_palette_part:
	movem.l	d0-d2/d5-d7/a1/a3,-(sp)
	move.w	d0,d4
	move.w	d1,d7
	move.l	a0,a3
	move.w	d4,d5
	move.w	d7,d6
	bra.s	.Entry
.Loop:	moveq.l	#0,d0			; Get current colour
	move.b	(a3)+,d0
	moveq.l	#0,d1
	move.b	(a3)+,d1
	moveq.l	#0,d2
	move.b	(a3)+,d2
	jsr	(a2)			; Get target colour
	jsr	Find_closest_colour		; Find
	cmp.w	#-1,d0			; Found ?
	bne.s	.Yes
	move.b	d5,(a1)+			; No
	bra.s	.Next
.Yes:	add.b	d4,d0			; Yes
	move.b	d0,(a1)+
.Next:	addq.w	#1,d5			; Next colour
.Entry:	dbra	d6,.Loop
	movem.l	(sp)+,d0-d2/d5-d7/a1/a3
	rts

;***************************************************************************	
; [ Find the closest colour in a palette ]
;   IN : d0 - Target Red value (.b)
;        d1 - Target Green value (.b)
;        d2 - Target Blue value (.b)
;        d7 - Number of colours in the palette (.l)
;        a0 - Pointer to IFF palette (.l)
;  OUT : d0 - Closest colour index / (-1 = no match) (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a1,-(sp)
	move.l	a0,-(sp)
	move.w	#$7fff,d6
	bra.s	.Entry
.Loop:	moveq.l	#0,d3			; Get colour
	move.b	(a0)+,d3
	moveq.l	#0,d4
	move.b	(a0)+,d4
	moveq.l	#0,d5
	move.b	(a0)+,d5
	sub.w	d0,d3			; Calculate difference
	bpl.s	.Ok1
	neg.w	d3
.Ok1:	sub.w	d1,d4
	bpl.s	.Ok2
	neg.w	d4
.Ok2:	sub.w	d2,d5
	bpl.s	.Ok3
	neg.w	d5
.Ok3:	move.l	d6,-(sp)
	cmp.w	d3,d4			; Get highest deviant
	bpl.s	.Green
	cmp.w	d3,d5
	bpl.s	.Blue1
	move.w	d3,d6
	bra.s	.Go_on
.Blue1:	move.w	d5,d6
	bra.s	.Go_on
.Green:	cmp.w	d4,d5
	bpl.s	.Blue2
	move.w	d4,d6
	bra.s	.Go_on
.Blue2:	move.w	d5,d6
.Go_on:	add.w	d6,d3
	add.w	d6,d6
	add.w	d6,d3
	move.l	(sp)+,d6
	add.w	d4,d3			; Calculate mean value
	add.w	d5,d3
	cmp.w	d6,d3			; New record ?
	bpl.s	.Entry
	move.w	d3,d6			; Yes
	move.l	a0,a1
.Entry:	dbra	d7,.Loop
	moveq.l	#-1,d0			; Default is no match
	move.l	(sp)+,a0
	cmp.w	#$7fff,d6			; Anything found ?
	beq.s	.Exit
	move.l	a1,d0			; Yes -> Calculate index
	sub.l	a0,d0
	divu.w	#3,d0
	subq.w	#1,d0
.Exit:	movem.l	(sp)+,d1-d7/a1
	rts
