;*****************************************************************************
; [ Plot pixel CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Colour (.w)
; All registers are restored
; Notes :
;  - This routine will plot {Screen_depth} bitplanes.
;*****************************************************************************
Plot_pixel:
	movem.l	d0-d4/d7/a0,-(sp)
	Wait_4_blitter			; !!!
	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	CA_X1(a0),d0		; X over left edge ?
	bmi	.Exit
	cmp.w	CA_Y1(a0),d1		; Y over top edge ?
	bmi	.Exit
	cmp.w	CA_X2(a0),d0		; X over right edge ?
	bgt	.Exit
	cmp.w	CA_Y2(a0),d1		; Y over bottom edge ?
	bgt	.Exit
	move.l	Work_screen,a0		; Get screen address
	jsr	Coord_convert
	add.l	d2,a0
	btst	#3,d3			; Odd ?
	beq.s	.No
	addq.l	#1,a0			; Yes -> next byte
.No:	btst	#3,d3			; Odd ?
	beq.s	.Even
	addq.l	#1,a0			; Yes -> next byte
.Even:	and.w	#7,d3			; Get bit number
	eor.w	#7,d3
.Loop:	ror.w	d4
	bcc.s	.Clear
	bset	d3,(a0)
	bra.s	.Next
.Clear:	bclr	d3,(a0)
.Next:	add.w	#Bytes_per_plane,a0
	dbra	d2,.Loop
.Exit:	movem.l	(sp)+,d0-d4/d7/a0
	rts
