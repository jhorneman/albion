; Get pixel routine
; Written by M.Bittner
; Start : 19-1-1994

	XDEF	Get_pixel

	XREF	CA_Sp
	XREF	Work_screen
	XREF	Coord_convert

	include	Earth.s
	include	Offsets.s

	SECTION	Program,code
;*****************************************************************************
; [ Get pixel colour CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;  OUT : d4 - Colour (.w)
; Changed registers : d4
; Notes :
;  - This routine will get 8 bitplanes.
;  - It will return zero if the coordinates are clipped out.
;*****************************************************************************
Get_pixel:
	movem.l	d0-d3/a0,-(sp)
	Wait_4_blitter			; !!!
	moveq.l	#0,d4			; Default
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
	sub.w	#15,d3			; Bit number from right
	neg.w	d3
	moveq.l	#0,d2			; Make pixel mask
	bset	d3,d2
	move.w	7*Bytes_per_plane(a0),d0	; Get plane 7
	and.w	d2,d0
	or.w	d0,d4
	move.w	6*Bytes_per_plane(a0),d0	; Get plane 6
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	move.w	5*Bytes_per_plane(a0),d0	; Get plane 5
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	move.w	4*Bytes_per_plane(a0),d0	; Get plane 4
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	move.w	3*Bytes_per_plane(a0),d0	; Get plane 3
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	move.w	2*Bytes_per_plane(a0),d0	; Get plane 2
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	move.w	Bytes_per_plane(a0),d0	; Get plane 1
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	move.w	(a0),d0			; Get plane 0
	and.w	d2,d0
	add.w	d4,d4
	or.w	d0,d4
	lsr.w	d3,d4			; Shift back
.Exit:	movem.l	(sp)+,d0-d3/a0
	rts
