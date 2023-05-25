
	XDEF	Calculate_dark_palette
	XDEF	Calculate_dark_palette_part
	XDEF	RGB_to_HSV
	XDEF	Find_closest_colour

;***************************************************************************
; [ Calculate the darker table for an entire 256-colour palette ]
;   IN : a0 - Pointer to IFF palette (.l)
;        a1 - Pointer to recolouring table (.l)
; All registers are restored
;***************************************************************************
Calculate_dark_palette:
	movem.l	d0/d1,-(sp)
	moveq.l	#0,d0
	move.w	#Pal_size,d1
	jsr	Calculate_dark_palette_part
	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************
; [ Calculate the darker table for part of a palette ]
;   IN : d0 - First colour {0...255} (.w)
;        d1 - Number of colours {1...256} (.w)
;        a0 - Pointer to IFF colour list (.l)
;        a1 - Pointer to recolouring table (.l)
; All registers are restored
;***************************************************************************
Calculate_dark_palette_part:
	movem.l	d0-d7/a1/a2,-(sp)
	move.w	d0,d4
	move.w	d1,d7
	move.l	a0,a2
	move.w	d7,d6
	bra.s	.Entry
.Loop:	move.b	(a2)+,d0			; Get current colour
	move.b	(a2)+,d1
	move.b	(a2)+,d2
	jsr	RGB_to_HSV		; Convert to HSV
	move.w	d1,d3			; S x 0.75
	lsr.w	#2,d3
	sub.w	d3,d1
	lsr.w	#1,d2			; V x 0.50
	jsr	Find_closest_colour		; Find closest
	add.b	d4,d0			; Store
	move.b	d0,(a1)+
.Entry:	dbra	d6,.Loop			; Next colour
	movem.l	(sp)+,d0-d7/a1/a2
	rts

;***************************************************************************	
; [ Convert from RGB to HSV ]
;   IN : d0 - Red value (.b)
;        d1 - Green value (.b)
;        d2 - Blue value (.b)
;  OUT : d0 - Hue {0...359} (.w)
;        d1 - Saturation {0...255} (.w)
;        d2 - Value {0...255} (.w)
; Changed registers : d0,d1,d2
;***************************************************************************	
RGB_to_HSV:
	movem.l	d3-d7,-(sp)
	moveq.l	#0,d5			; Clear H,S,V
	moveq.l	#0,d6
	moveq.l	#0,d7
	and.w	#$00ff,d0			; Mask off rubbish
	and.w	#$00ff,d1
	and.w	#$00ff,d2
	ext.l	d0			; Insert colour numbers
	swap	d1
	swap	d2
	move.w	#1,d1
	move.w	#2,d2
	swap	d1
	swap	d2
	cmp.w	d1,d0			; First < second ?
	bpl.s	.No1
	exg.l	d0,d1			; Swap first & second
.No1:	cmp.w	d2,d1			; Second < third ?
	bpl.s	.Sorted
	cmp.w	d2,d0			; First < third ?
	bpl.s	.No2
	exg.l	d1,d2			; Put third at top
	exg.l	d0,d1
	bra.s	.Sorted
.No2:	exg.l	d1,d2			; Swap second & third
.Sorted:	cmp.w	d0,d1			; First two the same ?
	bne.s	.Not_same
	swap	d0			; Yes
	swap	d1
	move.w	d0,d4			; Build index
	add.w	d4,d4
	add.w	d0,d4
	add.w	d1,d4
	btst	d4,#%011001100		; Swap ?
	beq.s	.Done
	exg.l	d0,d1			; Yes
.Done:	swap	d0
	swap	d1
.Not_same:	move.w	d0,d7			; Store V
	tst.w	d7			; Exit if V = 0
	beq.s	.Exit
	moveq.l	#0,d3			; Calculate S
	move.w	d0,d3
	sub.w	d2,d3
	move.l	d3,d4
	lsl.l	#8,d3
	sub.l	d4,d3
	divs.w	d0,d3
	move.w	d3,d6
	tst.w	d6			; Exit if S = 0
	beq.s	.Exit
	moveq.l	#0,d3			; Prepare H
	move.w	d1,d3
	mulu.w	#60,d3
	divu.w	d0,d3
	swap	d0
	swap	d1
	swap	d2
	move.w	d0,d5
	mulu.w	#120,d5
	move.w	d0,d4			; Build index
	add.w	d4,d4
	add.w	d0,d4
	add.w	d1,d4
	btst	d4,#%10001100		; Add or subtract ?
	bne.s	.Sub
	add.w	d3,d5			; Add
	bra.s	.Exit
.Sub:	sub.w	d3,d5			; Subtract
	bpl.s	.Exit
	add.w	#360,d5
.Exit:	move.w	d5,d0			; Output
	move.w	d6,d1
	move.w	d7,d2
	movem.l	(sp)+,d3-d7
	rts

;***************************************************************************	
; [ Find the closest colour in a palette ]
;   IN : d0 - Target H value (.w)
;        d1 - Target S value (.w)
;        d2 - Target V value (.w)
;        d7 - Number of colours (.l)
;        a0 - Pointer to HSV palette (.l)
;  OUT : d0 - Closest colour index (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a0-a2,-(sp)
	move.l	a0,a2
	move.l	#$7fffffff,d6
	bra.s	.Entry
.Loop:	move.w	(a0)+,d3			; Get colour
	moveq.l	#0,d4
	move.b	(a0)+,d4
	moveq.l	#0,d5
	move.b	(a0)+,d5
	sub.w	d0,d3			; Calculate differences
	bpl.s	.Ok
	neg.w	d3
.Ok:	sub.w	d1,d4
	sub.w	d2,d5
	cmp.w	#180,d3			; Is there a shorter H-interval ?
	bmi.s	.No
	sub.w	#360,d3			; Yes -> Use it
	neg.w	d3
.No:	mulu.w	d3,d3			; Calculate total difference
	lsr.l	#1,d3
	muls.w	d4,d4
	muls.w	d5,d5
	add.l	d5,d3
	add.l	d4,d3
	cmp.l	d6,d3			; New record ?
	bpl.s	.Entry
	move.l	d3,d6			; Yes
	move.l	a0,a1
.Entry:	dbra	d7,.Loop
	move.l	a1,d0			; Calculate index
	sub.l	a2,d0
	lsr.w	#2,d0
	subq.w	#1,d0
.Exit:	movem.l	(sp)+,d1-d7/a0-a2
	rts
