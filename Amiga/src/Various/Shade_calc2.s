
	OPT	AMIGA

	OUTPUT	RAM:DDT_test

LOCAL	macro
Local\@:
	endm

FALSE	equ 0

Pal_size	EQU 192

	SECTION	Program,code
Start:
	lea.l	Palette,a0		; Convert palette to HSV
	lea.l	HSV_palette,a1
	move.w	#Pal_size-1,d7
.Loop:	move.b	(a0)+,d0			; Get RGB colour
	move.b	(a0)+,d1
	move.b	(a0)+,d2
	jsr	RGB_to_HSV		; Convert to HSV
	move.w	d0,(a1)+			; Store HSV colour
	move.b	d1,(a1)+
	move.b	d2,(a1)+
	dbra	d7,.Loop
	LOCAL
	lea.l	HSV_palette,a0
	move.l	a0,a1
	lea.l	Darker_table,a2
	moveq.l	#0,d1
	moveq.l	#0,d2
	move.w	#Pal_size,d7
	move.w	d7,d6
	subq.w	#1,d6
.Loop:	move.w	(a1)+,d0
	move.b	(a1)+,d1
	move.b	(a1)+,d2
	lsr.w	#1,d1
	lsr.w	#1,d2
	jsr	Find_closest_colour
	move.b	d0,(a2)+
	dbra	d6,.Loop
	LOCAL
	moveq.l	#0,d0
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
	cmp.w	d1,d0			; First < Second ?
	bpl.s	.No1
	exg.l	d0,d1			; Swap first & second
.No1:	cmp.w	d2,d1			; Second < Third ?
	bpl.s	.Sorted
	cmp.w	d2,d0			; First < Third ?
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
;  OUT : d0 - Closest colour index / (-1 = no match) (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a0-a2,-(sp)
	move.l	a0,a2
	move.w	#$7fff,d6
	bra.s	.Entry
.Loop:	move.w	(a0)+,d3			; Get colour
	moveq.l	#0,d4
	move.b	(a0)+,d4
	moveq.l	#0,d5
	move.b	(a0)+,d5
	sub.w	d0,d3			; Calculate absolute differences
	bpl.s	.Ok1
	neg.w	d3
.Ok1:	sub.w	d1,d4
	bpl.s	.Ok2
	neg.w	d4
.Ok2:	sub.w	d2,d5
	bpl.s	.Ok3
	neg.w	d5
.Ok3:	cmp.w	#180,d3			; Is there a shorter H-interval ?
	bmi.s	.No
	sub.w	#360,d3			; Yes -> Use it
	neg.w	d3
.No:	add.w	d4,d3			; Calculate mean value
	add.w	d5,d3
	cmp.w	d6,d3			; New record ?
	bpl.s	.Entry
	move.w	d3,d6			; Yes
	move.l	a0,a1
.Entry:	dbra	d7,.Loop
	moveq.l	#-1,d0			; Default is no match
	cmp.w	#$7fff,d6			; Anything found ?
	beq.s	.Exit
	move.l	a1,d0			; Yes -> Calculate index
	sub.l	a2,d0
	lsr.w	#2,d0
	subq.w	#1,d0
.Exit:	movem.l	(sp)+,d1-d7/a0-a2
	rts


	SECTION	Fast_DATA,data
Palette:	incbin	DDT:Graphics/Ready/Test/Test_192.pal
	even

	SECTION	Fast_BSS,bss
Darker_table:	ds.b Pal_size
HSV_palette:	ds.l Pal_size
