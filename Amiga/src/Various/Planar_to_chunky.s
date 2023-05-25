; Planar to chunky conversion
; Written by J.Horneman (In Tune With The Universe)
; Based on a routine by M.Bittner
; Start : 3-3-1994

	OPT	DEBUG
	OPT	AMIGA

;  IN : plane 7,6,5,4,3,2,1,0 (.b)
; OUT : pixel 0,1,2,3,4,5,6,7 (.b)

	SECTION	Program,code
;***************************************************************************
; [ Convert a block from planar to chunky ]
;   IN : d5 - Number of planes (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to source picture (.l)
;        a1 - Pointer to destination buffer (.l)
; All registers are restored
;***************************************************************************
Convert_block:
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	#$0f0f0f0f,a2		; Masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	move.w	d5,d3
	add.w	d6,d6
	subq.w	#1,d7
.Loop1:	move.w	d6,d2
	subq.w	#1,d2
.Loop2:	movem.l	a0/a1,-(sp)		; Read planar graphics
	lea.l	Read_buffer(pc),a1
	clr.l	(a1)+
	clr.l	(a1)+
	move.w	d3,d0
	subq.w	#1,d0
.Loop3:	move.b	(a0),-(a1)
	add.w	d6,a0
	dbra	d0,.Loop3
	lea.l	Read_buffer(pc),a1
	move.l	(a1)+,d0
	move.l	(a1),d1
	movem.l	(sp)+,a0/a1
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
	move.l	d0,(a1)+			; Write chunky graphics
	move.l	d1,(a1)+
	addq.l	#1,a0			; Next 8 pixels
	dbra	d2,.Loop2
	move.w	d3,d0			; Next line
	subq.w	#1,d0
	mulu.w	d6,d0
	add.w	d0,a0
	dbra	d7,.Loop3
	movem.l	(sp)+,d0-d7/a0-a6
	rts

Read_buffer:	dc.l 0,0
