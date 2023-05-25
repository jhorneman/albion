;***************************************************************************
; [ Display a chunky block UNCLIPPED from fast RAM ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of block in truncs (must be even) (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to chunky block (.l)
; All registers are restored
;***************************************************************************
Display_chunky_block_FAST:
	movem.l	d0-d7/a0-a6,-(sp)
	Wait_4_blitter			; Wait
	jsr	Coord_convert		; Get screen address
	add.l	Work_screen,d2
	move.l	#$0f0f0f0f,a2		; Load masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	subq.w	#1,d7
.Loop_Y:	lea.l	Chunky_line_buffer,a1
	move.w	d6,d3
	add.w	d3,d3
	subq.w	#1,d3
.Loop_X1:	move.l	(a0)+,d0			; Read graphics
	move.l	(a0)+,d1
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
	move.b	d0,4*Bytes_per_plane(a1)	; Write graphics
	lsr.w	#8,d0
	move.b	d0,5*Bytes_per_plane(a1)
	swap	d0
	move.b	d0,6*Bytes_per_plane(a1)
	lsr.w	#8,d0
	move.b	d0,7*Bytes_per_plane(a1)
	move.b	d1,(a1)
	lsr.w	#8,d1
	move.b	d1,Bytes_per_plane(a1)
	swap	d1
	move.b	d1,2*Bytes_per_plane(a1)
	lsr.w	#8,d1
	move.b	d1,3*Bytes_per_plane(a1)
	addq.l	#1,a1			; Next byte
	dbra	d3,.Loop_X1
	move.l	a0,-(sp)
	lea.l	Chunky_line_buffer,a0	; Copy line to screen
	move.l	d2,a1
	move.w	#Bytes_per_plane,d0
	sub.w	d6,d0
	sub.w	d6,d0
	moveq.l	#Screen_depth-1,d1
.Loop_Z:	move.w	d6,d3
	lsr.w	#1,d3
	subq.w	#1,d3
.Loop_X2:	move.l	(a0)+,(a1)+
	dbra	d3,.Loop_X2
	add.w	d0,a0
	add.w	d0,a1
	dbra	d1,.Loop_Z
	move.l	(sp)+,a0
	add.l	#Bytes_per_line,d2		; Next line
	dbra	d7,.Loop_Y
	movem.l	(sp)+,d0-d7/a0-a6
	rts


	SECTION	Longword_BSS
	CNOP 0,4
Chunky_line_buffer:	ds.b Bytes_per_line
	even
