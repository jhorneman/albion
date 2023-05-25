; Chunky frame cutter
; Written by J.Horneman (In Tune With The Universe)
; Chunky to planar conversion based on a routine by M.Bittner
; Start : 7-3-1994

	OPT	DEBUG
	OPT	AMIGA

Screen_height	EQU 200
Screen_width	EQU 320
Screen_depth	EQU 8
Bytes_per_line	EQU 320
Bytes_per_plane	EQU 40

	SECTION	Program,code
;***************************************************************************
; [ Cut an any-sized block from a chunky picture ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - 0 - Leave chunky
;             1 - Convert to planar (.w)
;        d5 - Depth in bitplanes (.w)
;        d6 - Width in pixels (.w)
;        d7 - Height in pixels (.w)
;        a0 - Pointer to a chunky picture (.l)
;        a1 - Pointer to destination buffer (.l)
;  OUT : d0 - Size of cut data in bytes / -1 (ERROR) (.l)
; Changed registers : d0
; Notes :
;   - The screen is presumed to have a size of 320 x 200 pixels.
;   - An error will occur if any parameters are wrong.
;***************************************************************************
Cut_chunky_block:
	movem.l	d1-d7/a0-a3/a6,-(sp)
	lea.l	-Cut_chunky_LDS(sp),sp	; Create local variables
	move.l	sp,a6
	move.w	d2,Flag(a6)		; Store input
	move.w	d5,Depth(a6)
	move.w	d6,Width(a6)
	move.w	d7,Height(a6)
	move.l	a0,Picture(a6)
	move.l	a1,Buffer(a6)
; ---------- Check input --------------------------
	move.l	#-1,Size(a6)		; Default is error
	tst.w	d0			; X on screen ?
	bmi	.Exit
	cmp.w	#Screen_width,d0
	bpl	.Exit
	tst.w	d1			; Y on screen ?
	bmi	.Exit
	cmp.w	#Screen_height,d1
	bpl	.Exit
	tst.w	d2			; Legal number of planes ?
	beq.s	.Skip
	tst.w	d5
	beq	.Exit
	cmp.w	#Screen_depth,d5
	bgt	.Exit
.Skip:	tst.w	d6			; Width & height OK ?
	ble	.Exit
	tst.w	d7
	ble	.Exit
	move.w	d0,d2			; X2 on screen ?
	add.w	d6,d2
	cmp.w	#Screen_width+1,d2
	bpl	.Exit
	move.w	d1,d2			; Y2 on screen ?
	add.w	d7,d2
	cmp.w	#Screen_height+1,d2
	bpl	.Exit
; ---------- Prepare to cut -----------------------
	add.w	d0,a0			; Calculate screen address
	mulu.w	#Bytes_per_line,d1
	add.l	d1,a0
	tst.w	Flag(a6)			; Convert to planar ?
	bne	.Planar
; ---------- Cut chunky block ---------------------
	subq.w	#1,d6			; Cut
	subq.w	#1,d7
.Loop_Y1:	move.l	a0,a2
	move.w	d6,d5
.Loop_X1:	move.b	(a2)+,(a1)+
	dbra	d5,.Loop_X1
	lea.l	Bytes_per_line(a0),a0
	dbra	d7,.Loop_Y1
	move.l	a1,d0			; Calculate size
	sub.l	Buffer(a6),d0
	move.l	d0,Size(a6)
	bra	.Exit
; ---------- Cut planar block ---------------------
.Planar:	move.w	d6,d0			; Calculate spill
	and.w	#$000f,d0
	sub.w	#16,d0
	neg.w	d0
	and.w	#$000f,d0
	subq.w	#1,d6			; Cut
	subq.w	#1,d7
.Loop_Y2:	move.l	a0,a2
	move.w	d6,d5
.Loop_X2:	move.b	(a2)+,(a1)+
	dbra	d5,.Loop_X2
	move.w	d0,d1			; Clear spill
	bra.s	.Entry1
.Loop1:	clr.b	(a1)+
.Entry1:	dbra	d1,.Loop1
	lea.l	Bytes_per_line(a0),a0
	dbra	d7,.Loop_Y2
; ---------- Convert block to planar --------------
	move.l	Buffer(a6),a0
	move.l	a0,a1
	move.w	Width(a6),d6
	add.w	#15,d6
	move.	d6,d5
	and.w	#$fff0,d5
	lsr.w	#4,d6
	move.w	Height(a6),d7
	subq.w	#1,d7
.Loop_Y3:	bsr	Convert_one_line		; Convert one line
	lea.l	Conversion(pc),a2		; Write to buffer
	move.w	Depth(a6),d0
	subq.w	#1,d0
.Loop2:	move.l	a2,a3
	move.w	d6,d1
	subq.w	#1,d1
.Loop3:	move.w	(a3)+,(a1)+
	dbra	d1,.Loop3
	lea.l	Bytes_per_plane(a2),a2
	dbra	d0,.Loop2
	add.w	d5,a0			; Next line
	dbra	d7,.Loop_Y3
	move.l	a1,d0			; Calculate size
	sub.l	Buffer(a6),d0
	move.l	d0,Size(a6)
.Exit:	move.l	Size(a6),d0		; Get output
	lea.l	Cut_chunky_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d1-d7/a0-a3/a6
	rts

	rsreset
Flag:	rs.w 1
Depth:	rs.w 1
Width:	rs.w 1
Height:	rs.w 1
Picture:	rs.l 1
Buffer:	rs.l 1
Size:	rs.l 1
Cut_chunky_LDS:	rs.b 0

;***************************************************************************
; [ Convert one line from chunky to planar ]
;   IN : d6 - Width in truncs (.w)
;        a0 - Pointer to chunky line (.l)
; All registers are restored
; Notes :
;   - This routine will simply convert all 8 bitplanes into a buffer.
;     All bitplanes will always have the same position (independent of the
;     line width).
;***************************************************************************
Convert_one_line:
	movem.l	d0/d1/d4-d6/a0-a6,-(sp)
	lea.l	Conversion(pc),a1
	move.l	#$0f0f0f0f,a2		; Load masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	add.w	d6,d6
	subq.w	#1,d6
.Loop:	move.b	(a0)+,d0			; Read graphics
	lsl.w	#8,d0
	move.b	(a0)+,d0
	swap	d0
	move.b	(a0)+,d0
	lsl.w	#8,d0
	move.b	(a0)+,d0
	move.b	(a0)+,d1
	lsl.w	#8,d1
	move.b	(a0)+,d1
	swap	d1
	move.b	(a0)+,d1
	lsl.w	#8,d1
	move.b	(a0)+,d1
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
	addq.l	#1,a1			; Next 8 pixels
	dbra	d6,.Loop
	movem.l	(sp)+,d0/d1/d4-d6/a0-a6
	rts

Conversion:
	dcb.b Bytes_per_line,0
	even
