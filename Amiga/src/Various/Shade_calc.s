
	OPT	AMIGA

	OUTPUT	RAM:DDT_test

LOCAL	macro
Local\@:
	endm

FALSE	equ 0

Pal_size	EQU 192

	rsreset
H_index:	rs.b 1		; H / 30 : 0-11
S_index:	rs.b 1		; S / 16 : 0-15
V_index:	rs.b 1		; V / 16 : 0-15
Colour_nr:	rs.b 1		; 0-255
H_value:	rs.w 1		; 0-359
S_value:	rs.b 1		; 0-255
V_value:	rs.b 1		; 0-255
; Data size must be 8 bytes because of scaled addressing modes !!!


	SECTION	Program,code
Start:
; ---------- Build quick colour list --------------
	lea.l	Palette,a0
	lea.l	QCol_list,a1
	moveq.l	#30,d3
	moveq.l	#0,d7
.Loop:	move.b	(a0)+,d0			; Get RGB colour
	move.b	(a0)+,d1
	move.b	(a0)+,d2
	jsr	RGB_to_HSV		; Convert to HSV
	move.w	d0,H_value(a1)		; Store HSV
	move.b	d1,S_value(a1)
	move.b	d2,V_value(a1)
	ext.l	d0			; Calculate indices
	divu.w	d3,d0
	lsr.w	#4,d1
	lsr.w	#4,d2
	move.b	d0,H_index(a1)		; Store indices
	move.b	d1,S_index(a1)
	move.b	d2,V_index(a1)
	move.b	d7,Colour_nr(a1)		; Store colour number
	addq.l	#8,a1			; Next colour
	addq.w	#1,d7
	cmp.w	#Pal_size,d7
	bmi.s	.Loop
	LOCAL
; ---------- Sort quick colour list ---------------
	lea.l	QCol_list,a6
	move.w	#Pal_size,d7
	lea.l	Compare_colours,a0
	lea.l	Swap_colours,a1
	jsr	Shellsort
; ---------- Build quick colour indices -----------
	lea.l	QCol_list,a0
	lea.l	Pal_size*8(a0),a1
	lea.l	QCol_indices,a2
	moveq.l	#0,d0
	moveq.l	#12-1,d7
.Loop_H:	moveq.l	#16-1,d6
.Loop_S:	moveq.l	#16-1,d5
.Loop_V:	moveq.l	#0,d1			; Clear counter
.Again:	cmp.b	H_index(a0),d7		; The right colour ?
	bne.s	.Wrong
	cmp.b	S_index(a0),d6
	bne.s	.Wrong
	cmp.b	V_index(a0),d5
	bne.s	.Wrong
	addq.w	#1,d1			; Yes -> Count up
	addq.l	#8,a0			; Next colour
	cmp.l	a1,a0			; Last ?
	bmi.s	.Again
	move.b	d0,(a2)+			; Store last
	move.b	d1,(a2)+
	move.l	#QCol_indices+2*12*16*16,d7	; Clear the rest
	sub.l	a2,d7
	bra.s	.Entry
.Loop:	clr.w	(a2)+
.Entry:	dbra	d7,.Loop
	bra.s	.Done
.Wrong:	move.b	d0,(a2)+			; Store
	move.b	d1,(a2)+
	add.w	d1,d0
	dbra	d5,.Loop_V		; Next
	dbra	d6,.Loop_S
	dbra	d7,.Loop_H
.Done:


	moveq.l	#0,d0
	rts

; [ Compare two colours ]
;   IN : d5 - Source index {1...} (.w)
;        d6 - Destination index {1...} (.w)
;        a6 - Pointer to colour list (.l)
;  OUT : eq - Source  = Destination
;        gs - Source >= Destination
;        ls - Source <= Destination
; All registers are restored
Compare_colours:
	movem.l	d0/d1,-(sp)
	move.l	-8(a6,d5.w*8),d0		; Get first colour
	lsr.l	#8,d0
	move.l	-8(a6,d6.w*8),d1		; Get second colour
	lsr.l	#8,d1
	cmp.l	d0,d1			; Compare
	movem.l	(sp)+,d0/d1
	rts

; [ Swap two colours ]
;   IN : d5 - Source index {1...} (.w)
;        d6 - Destination index {1...} (.w)
;        a6 - Pointer to quick colour list (.l)
; All registers are restored
Swap_colours:
	move.l	d0,-(sp)
	move.l	-8(a6,d5.w*8),d0
	move.l	-8(a6,d6.w*8),-8(a6,d5.w*8)
	move.l	d0,-8(a6,d6.w*8)
	move.l	-8+4(a6,d5.w*8),d0
	move.l	-8+4(a6,d6.w*8),-8+4(a6,d5.w*8)
	move.l	d0,-8+4(a6,d6.w*8)
	move.l	(sp)+,d0
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
;  OUT : d0 - Closest colour index / (-1 = no match) (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a1,-(sp)
	lea.l	QCol_indices,a0
	move.w	d0,d3
	move.w	d1,d4
	move.w	d2,d5
	ext.l	d3			; Calculate indices
	divu.w	#60,d3
	lsr.w	#4,d4
	lsr.w	#4,d5
	move.w	d3,d6			; Add
	lsl.w	#4,d6
	add.w	d4,d6
	lsl.w	#4,d6
	add.w	d5,d6
	tst.b	1(a0,d6.w*2)



.Exit:	movem.l	(sp)+,d1-d7/a1
	rts





	ifne	FALSE
;***************************************************************************	
; [ Find the closest colour in a palette ]
;   IN : d0 - Target Red value (.b)
;        d1 - Target Green value (.b)
;        d2 - Target Blue value (.b)
;        d7 - Number of colours in the palette (.l)
;        a0 - Pointer to palette (.l)
;  OUT : d0 - Closest colour index / (-1 = no match) (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a1,-(sp)
	move.l	a0,-(sp)
	move.l	#$7fffffff,d6
	bra.s	.Entry
.Loop:	moveq.l	#0,d3			; Clear top word
	move.b	(a0)+,d3			; Get colour
	move.b	(a0)+,d4
	move.b	(a0)+,d5
	sub.b	d0,d3			; Calculate difference
	sub.b	d1,d4
	sub.w	d2,d5
	ext.w	d3			; Take absolute value
	bpl.s	.Ok1
	neg.w	d3
.Ok1:	ext.w	d4
	bpl.s	.Ok2
	neg.w	d4
.Ok2:	ext.w	d5
	bpl.s	.Ok3
	neg.w	d5
.Ok3:	add.w	d4,d3			; Calculate mean value
	add.w	d5,d3
	cmp.w	d6,d3			; New record ?
	bpl.s	.Entry
	move.w	d3,d6			; Yes
	move.l	a0,a1
.Entry:	dbra	d7,.Loop
	moveq.l	#-1,d0			; Default is no match
	move.l	(sp)+,a0
	cmp.l	#$7fffffff,d6		; Anything found ?
	beq.s	.Exit
	move.l	a1,d0			; Yes -> Calculate index
	sub.l	a0,d0
	divu.w	#3,d0
	subq.w	#1,d0
.Exit:	movem.l	(sp)+,d1-d7/a1
	rts
	endc


;*****************************************************************************
; [ Generic Shellsort ]
;   IN : d7 - Number of elements (.w)
;        a0 - Pointer to Compare routine (.l)
;        a1 - Pointer to Swap routine (.l)
; All registers are restored
;*****************************************************************************
Shellsort:
	movem.l	d0-d2/d5/d6,-(sp)
	cmp.w	#1+1,d7			; More than 1 element ?
	bmi	.Done1
	move.w	d7,d0			; Inc = Count
.Again1:	cmp.w	#1,d0			; While (Inc>1)
	bls.s	.Done1
	lsr.w	#1,d0			; Inc = Inc/2
	moveq.l	#1,d1			; L = 1
.Again2:	move.w	d7,d2			; While (L <= Count - Inc)
	sub.w	d0,d2
	cmp.w	d1,d2
	bmi.s	.Done2
	move.w	d1,d5			; If (L) > (L+Inc)
	move.w	d1,d6
	add.w	d0,d6
	jsr	(a0)
	ble.s	.Endif1
	jsr	(a1)			; Swap(L,L+Inc)
	move.w	d1,d2			; Ps = L-Inc
	sub.w	d0,d2
.Again3:	tst.w	d2			; While (Ps>0)
	ble.s	.Done3
	move.w	d2,d5			; If (Ps) > (Ps+Inc)
	move.w	d2,d6
	add.w	d0,d6
	jsr	(a0)
	ble.s	.Else2
	jsr	(a1)			; Swap(Ps,Ps+Inc)
	sub.w	d0,d2			; Ps = Ps-Inc
	bra.s	.Endif2
.Else2:	moveq.l	#0,d2			; Ps = 0
.Endif2:	bra.s	.Again3			; }
.Done3:
.Endif1:	addq.w	#1,d1			; L++
	bra.s	.Again2			; }
.Done2:	bra.s	.Again1			; }
.Done1:	movem.l	(sp)+,d0-d2/d5/d6
	rts


	SECTION	Fast_DATA,data
Palette:	incbin	DDT:Graphics/Ready/Test/Test_192.pal
	even

	SECTION	Fast_BSS,bss
QCol_list:	ds.l Pal_size*2
QCol_indices:	ds.w 12*16*16
