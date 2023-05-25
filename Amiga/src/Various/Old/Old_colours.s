;***************************************************************************	
; [ Find the closest colour in a palette ]
;   IN : d0 - Target R value (.w)
;        d1 - Target G value (.w)
;        d2 - Target B value (.w)
;        d7 - Number of colours (.l)
;        a0 - Pointer to IFF palette (.l)
;  OUT : d0 - Closest colour index (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a0-a2,-(sp)
	move.l	a0,a2
	move.l	#$7fffffff,d6
	bra.s	.Entry
.Loop:	moveq.l	#0,d3			; Get colour
	move.b	(a0)+,d3
	moveq.l	#0,d4
	move.b	(a0)+,d4
	moveq.l	#0,d5
	move.b	(a0)+,d5
	sub.w	d0,d3			; Calculate differences
	sub.w	d1,d4
	sub.w	d2,d5
	muls.w	d3,d3			; Calculate total difference
	muls.w	d4,d4
	muls.w	d5,d5
	add.l	d4,d3
	add.l	d5,d3
	cmp.l	d6,d3			; New record ?
	bpl.s	.Entry
	move.l	d3,d6			; Yes
	move.l	a0,a1
.Entry:	dbra	d7,.Loop
	move.l	a1,d0			; Calculate index
	sub.l	a2,d0
	divu.w	#3,d0
	subq.w	#1,d0
.Exit:	movem.l	(sp)+,d1-d7/a0-a2
	rts

;***************************************************************************
; [ Get the current palette ]
;  OUT : a1 - Pointer to IFF colour list (.l)
; Changed registers : a1
;***************************************************************************
Get_full_palette:
	movem.l	d0/d1/a0,-(sp)
	move.l	My_screen,a0		; Get ColorMap address
	lea.l	sc_ViewPort(a0),a0
	move.l	vp_ColorMap(a0),a0
	moveq.l	#0,d0			; Get current colours
	move.l	#Pal_size,d1
	lea.l	Colour_table,a1
	kickGFX	GetRGB32
	lea.l	Colour_table,a0		; Convert back to IFF
	move.l	a0,a1
	move.w	#256*3-1,d1
.Loop:	move.b	(a0),(a1)+
	addq.l	#4,a0
	dbra	d1,.Loop
	lea.l	Colour_table,a1		; Output
	movem.l	(sp)+,d0/d1/a0
	rts
