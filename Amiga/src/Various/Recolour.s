; Recolouring routine
; Written by J.Horneman (In Tune With The Universe)
; Based on a routine by M.Bittner
; Start : 3-3-1994

	OPT	DEBUG
	OPT	AMIGA

	SECTION	Program,code

	lea.l	Buffer,a0
	lea.l	Recolouring_table,a1
	jsr	Recolour_8_pixels
	moveq.l	#0,d0
	rts

;  IN : plane 7,6,5,4,3,2,1,0 (.b)
; OUT : pixel 0,1,2,3,4,5,6,7 (.b)

Buffer:	dc.l $80402010
	dc.l $08040201
Recolouring_table:
x	set 0
	rept 256
	dc.b 255-x
x	set x+1
	endr
	even

;***************************************************************************
; [ Recolour 16 pixels ]
;   IN : a0 - Pointer to graphics (.l)
;        a1 - Pointer to recolouring table (.l)
;  OUT : a0 - Updated pointer (.l)
; Changed registers : a0
;***************************************************************************
Recolour_16_pixels:
	movem.l	d0-d7/a1-a6,-(sp)
	move.l	#$0f0f0f0f,a2		; Load masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	moveq.l	#2-1,d3
.Loop:	move.l	(a0),d0			; Read graphics
	move.l	4(a0),d1
; ---------- Convert planar to chunky -------------
	move.l	a5,d4			; Third swapping
	move.l	a6,d5
	and.l	d0,d4
	and.l	d0,d5
	eor.l	d4,d0
	eor.l	d5,d0
	lsr.l	#7,d4
	lsl.l	#7,d5
	or.l	d4,d0
	or.l	d5,d0
	move.l	a5,d4
	move.l	a6,d5
	and.l	d1,d4
	and.l	d1,d5
	eor.l	d4,d1
	eor.l	d5,d1
	lsr.l	#7,d4
	lsl.l	#7,d5
	or.l	d4,d1
	or.l	d5,d1
	move.l	a4,d4			; Second swapping
	and.l	d0,d4
	eor.l	d4,d0
	lsr.w	#2,d4
	swap	d4	
	lsl.w	#2,d4
	or.l	d4,d0
	move.l	a4,d4
	and.l	d1,d4
	eor.l	d4,d1
	lsr.w	#2,d4
	swap	d4	
	lsl.w	#2,d4
	or.l	d4,d1
	move.l	a2,d4			; First swapping
	move.l	a3,d5
	and.l	d0,d4
	and.l	d1,d5
	eor.l	d4,d0
	eor.l	d5,d1
	lsl.l	#4,d4
	lsr.l	#4,d5
	or.l	d5,d0
	or.l	d4,d1
; ---------- Recolour -----------------------------
	moveq.l	#0,d2
	move.b	d0,d2			; First 4 pixels
	move.b	0(a1,d2.w),d0
	ror.l	#8,d0
	move.b	d0,d2
	move.b	0(a1,d2.w),d0
	ror.l	#8,d0
	move.b	d0,d2
	move.b	0(a1,d2.w),d0
	ror.l	#8,d0
	move.b	d0,d2
	move.b	0(a1,d2.w),d0
	ror.l	#8,d0
	move.b	d1,d2			; Second 4 pixels
	move.b	0(a1,d2.w),d1
	ror.l	#8,d1
	move.b	d1,d2
	move.b	0(a1,d2.w),d1
	ror.l	#8,d1
	move.b	d1,d2
	move.b	0(a1,d2.w),d1
	ror.l	#8,d1
	move.b	d1,d2
	move.b	0(a1,d2.w),d1
	ror.l	#8,d1
; ---------- Convert chunky to planar -------------
	move.l	a2,d4			; First swapping
	move.l	a3,d5
	and.l	d0,d4
	and.l	d1,d5
	eor.l	d4,d0
	eor.l	d5,d1
	lsl.l	#4,d4
	lsr.l	#4,d5
	or.l	d5,d0
	or.l	d4,d1
	move.l	a4,d4			; Second swapping
	and.l	d0,d4
	eor.l	d4,d0
	lsr.w	#2,d4
	swap	d4	
	lsl.w	#2,d4
	or.l	d4,d0
	move.l	a4,d4
	and.l	d1,d4
	eor.l	d4,d1
	lsr.w	#2,d4
	swap	d4	
	lsl.w	#2,d4
	or.l	d4,d1
	move.l	a5,d4			; Third swapping
	move.l	a6,d5
	and.l	d0,d4
	and.l	d0,d5
	eor.l	d4,d0
	eor.l	d5,d0
	lsr.l	#7,d4
	lsl.l	#7,d5
	or.l	d4,d0
	or.l	d5,d0
	move.l	a5,d4
	move.l	a6,d5
	and.l	d1,d4
	and.l	d1,d5
	eor.l	d4,d1
	eor.l	d5,d1
	lsr.l	#7,d4
	lsl.l	#7,d5
	or.l	d4,d1
	or.l	d5,d1
	move.l	d0,(a0)+			; Write graphics
	move.l	d1,(a0)+
	dbra	d3,.Loop
	movem.l	(sp)+,d0-d7/a1-a6
	rts
