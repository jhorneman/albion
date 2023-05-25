; Graphics routines
; Graphics copying
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-2-1994

	SECTION	Program,code
;***************************************************************************
; [ Duplicate an any-sized box from the visible to the invisible screen CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
; All registers are restored
; Notes :
;  - This routine will handle {Screen_depth} bitplanes.
;  - The horizontal coordinates will be rounded to trunc boundaries.
;***************************************************************************
Duplicate_box:
	movem.l	d0-d7/a0/a1/a6,-(sp)
	kickGFX	OwnBlitter
	lea.l	Custom,a6
	Wait_4_blitter			; Wait
; ---------- Adjust coordinates -------------------
	and.w	#$fff0,d0			; Round to trunc boundaries
	add.w	#15,d2
	and.w	#$fff0,d2
	cmp.w	d0,d2			; X1 > X2 ?
	bpl.s	.X_OK
	exg.l	d0,d2			; Yes, swap them
.X_OK:	cmp.w	d1,d3			; Y1 > Y2 ?
	bpl.s	.Y_OK
	exg.l	d1,d3			; Yes, swap them
; ---------- Check if block is off-screen ---------
.Y_OK:	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	X2(a0),d0			; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a0),d1			; Y1 over bottom edge ?
	bgt	.Exit
	cmp.w	X1(a0),d2			; X2 over left edge ?
	bmi	.Exit
	cmp.w	Y1(a0),d3			; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip coordinates ---------------------
	cmp.w	X1(a0),d0			; Check left
	bpl.s	.Check_top
	move.w	X1(a0),d0
.Check_top:
	cmp.w	Y1(a0),d1			; Check top
	bpl.s	.Check_right
	move.w	Y1(a0),d1
.Check_right:
	cmp.w	X2(a0),d2			; Check right
	ble.s	.Check_bottom
	move.w	X2(a0),d2
.Check_bottom:
	cmp.w	Y2(a0),d3			; Check bottom
	bmi.s	.Done
	move.w	Y2(a0),d3
; ---------- Prepare blitter ----------------------
.Done:	movem.l	d2/d3,-(sp)		; Calculate screen addresses
	move.l	Off_screen,a0
	move.l	On_screen,a1
	jsr	Coord_convert
	add.l	d2,a0
	add.l	d2,a1	
	movem.l	(sp)+,d2/d3
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	move.w	d2,d6			; Calculate width
	sub.w	d0,d6
	lsr.w	#4,d6
	move.w	#Bytes_per_plane,d2		; Set modulo A & D
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
	move.w	d2,bltdmod(a6)
; ---------- Blit ---------------------------------
	sub.w	d1,d3			; Calculate height
	addq.w	#1,d3
	cmp.w	#127,d3			; Too large ?
	bmi.s	.No
	move.w	#127*Screen_depth*64,d7	; Yes -> Blit first part
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	sub.w	#127,d3			; Blit the rest
	add.l	#127*Bytes_per_line,a0
	add.l	#127*Bytes_per_line,a1
	Wait_4_blitter
.No:	move.w	d3,d7			; Calculate blit size
	mulu.w	#Screen_depth*64,d7
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

;***************************************************************************
; [ Duplicate an 8x8 block from the invisible to the visible screen CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
; All registers are restored
; Notes :
;  - This routine will handle {Screen_depth} bitplanes.
;  - The coordinates will be rounded to byte boundaries.
;  - This routine won't do "real" clipping but will check if the ebe is in
;     the current CA.
;***************************************************************************
Blend_ebe:
	movem.l	d0-d3/a0/a1,-(sp)
	Wait_4_blitter			; Wait
	and.w	#$fff8,d0			; Round to byte boundaries
	and.w	#$fff8,d1
; ---------- Check if block is off-screen ---------
	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	X2(a0),d0			; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a0),d1			; Y1 over bottom edge ?
	bgt	.Exit
	move.w	d0,d2			; Calculate X2
	addq.w	#7,d2
	cmp.w	X1(a0),d2			; X2 over left edge ?
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	addq.w	#7,d3
	cmp.w	Y1(a0),d3			; Y2 over top edge ?
	bmi	.Exit
; ---------- Just do it ---------------------------
	move.l	On_screen,a0		; Calculate screen addresses
	move.l	Off_screen,a1
	jsr	Coord_convert
	add.l	d2,a0
	add.l	d2,a1
	cmp.w	#8,d3			; Odd ?
	bmi.s	.Even
	addq.l	#1,a0			; Yes -> Other byte
	addq.l	#1,a1
.Even:	move.w	#Bytes_per_line,d0		; Just do it
	moveq.l	#8-1,d2
.Loop:	move.b	(a1),(a0)
	move.b	Bytes_per_plane(a1),Bytes_per_plane(a0)
	move.b	Bytes_per_plane*2(a1),Bytes_per_plane*2(a0)
	move.b	Bytes_per_plane*3(a1),Bytes_per_plane*3(a0)
	move.b	Bytes_per_plane*4(a1),Bytes_per_plane*4(a0)
	move.b	Bytes_per_plane*5(a1),Bytes_per_plane*5(a0)
	move.b	Bytes_per_plane*6(a1),Bytes_per_plane*6(a0)
	move.b	Bytes_per_plane*7(a1),Bytes_per_plane*7(a0)
	add.w	d0,a0
	add.w	d0,a1
	dbra	d2,.Loop
.Exit:	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************
; [ Get an any-sized block from the screen CLIPPED / any planes ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d5 - Number of planes (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to buffer (.l)
; All registers are restored
;***************************************************************************
Get_block:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Get_block_all
	movem.l	d0-d7/a0/a1/a6,-(sp)
	kickGFX	OwnBlitter
	lea.l	Custom,a6
	Wait_4_blitter
; ---------- Check if block is off-screen ---------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0			; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1			; Y1 over bottom edge ?
	bgt	.Exit
	move.w	d6,d2			; Calculate X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	cmp.w	X1(a1),d2			; X2 over left edge ?
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3			; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1			; Check top
	bpl.s	.Check_bottom
	move.w	Y1(a1),d4			; Adjust height
	sub.w	d1,d4
	sub.w	d4,d7
	add.w	d4,d4			; Adjust buffer pointer
	mulu.w	d5,d4
	mulu.w	d6,d4
	add.l	d4,a0
	move.w	Y1(a1),d1			; Set new Y-coordinate
.Check_bottom:
	cmp.w	Y2(a1),d3			; Check bottom
	bmi.s	.Done_Y
	sub.w	Y2(a1),d3			; Adjust height
	sub.w	d3,d7
; --------- Clip X-coordinates --------------------
.Done_Y:	move.w	d6,d3			; Save width of block
	cmp.w	X1(a1),d0			; Check left
	bpl.s	.Check_right
	move.w	X1(a1),d4			; Adjust width
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust buffer pointer
	add.w	d4,a0
	move.w	X1(a1),d0			; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2			; Check right
	ble.s	.Done_X
	sub.w	X2(a1),d2			; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
; ---------- Check if anything is left ------------
.Done_X:	tst.w	d6			; Anything left ?
	bne	.Notzero
	moveq.l	#1,d6
.Notzero:	tst.w	d7
	ble	.Exit
; ---------- Prepare blitter ----------------------
	move.l	d3,-(sp)			; Calculate screen address
	move.l	Work_screen,a1
	jsr	Coord_convert
	add.l	d2,a1
	move.l	(sp)+,d3
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	move.w	#Bytes_per_line,d2		; Set modulo A
	add.w	d6,d6
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
	add.w	d3,d3			; Block width x 2
	move.w	d3,d2			; Set modulo D
	mulu.w	d5,d2			;  = (W x 2) x (D - 1)
	sub.w	d6,d2
	move.w	d2,bltdmod(a6)
; ---------- Blit each plane ----------------------
	subq.w	#1,d5
	bra.s	.Entry
.Loop:	Wait_4_blitter a6
.Entry:	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	d3,a0			; Next plane
	add.w	#Bytes_per_plane,a1
	dbra	d5,.Loop
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

;***************************************************************************
; [ Get an any-sized block from the screen CLIPPED / all planes ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to buffer (.l)
; All registers are restored
;***************************************************************************
Get_block_all:
	movem.l	d0-d7/a0/a1/a6,-(sp)
	kickGFX	OwnBlitter
	lea.l	Custom,a6
	Wait_4_blitter
; ---------- Check if block is off-screen ---------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0			; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1			; Y1 over bottom edge ?
	bgt	.Exit
	move.w	d6,d2			; Calculate X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	cmp.w	X1(a1),d2			; X2 over left edge ?
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3			; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1			; Check top
	bpl.s	.Check_bottom
	move.w	Y1(a1),d4			; Adjust height
	sub.w	d1,d4
	sub.w	d4,d7
	add.w	d4,d4			; Adjust buffer pointer
	mulu.w	d5,d4
	mulu.w	d6,d4
	add.l	d4,a0
	move.w	Y1(a1),d1			; Set new Y-coordinate
.Check_bottom:
	cmp.w	Y2(a1),d3			; Check bottom
	bmi.s	.Done_Y
	sub.w	Y2(a1),d3			; Adjust height
	sub.w	d3,d7
; ---------- Clip X-coordinates -------------------
.Done_Y:	move.w	d6,d3			; Save width of block
	cmp.w	X1(a1),d0			; Check left
	bpl.s	.Check_right
	move.w	X1(a1),d4			; Adjust width
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust buffer pointer
	add.w	d4,a0
	move.w	X1(a1),d0			; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2			; Check right
	ble.s	.Done_X
	sub.w	X2(a1),d2			; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
; ---------- Check if anything is left ------------
.Done_X:	tst.w	d6			; Anything left ?
	bne	.Notzero
	moveq.l	#1,d6
.Notzero:	tst.w	d7
	ble	.Exit
; ---------- Prepare blitter ----------------------
	move.l	d3,-(sp)			; Calculate screen address
	move.l	Work_screen,a1
	jsr	Coord_convert
	add.l	d2,a1
	move.l	(sp)+,d3
	Wait_4_blitter a6			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	move.w	#Bytes_per_plane,d2		; Set modulo A
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
	add.w	d3,d3			; Block width x 2
	move.w	d3,d2			; Set modulo D
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltdmod(a6)
; ---------- Blit ---------------------------------
	cmp.w	#127,d7			; Too large ?
	bmi.s	.No
	move.w	d7,-(sp)
	move.w	#127*Screen_depth*64,d7	; Yes -> Blit first part
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	move.w	(sp)+,d7
	sub.w	#127,d7			; Blit the rest
	mulu.w	#127*Screen_depth,d3
	add.l	d3,a0
	add.l	#127*Bytes_per_line,a1
	Wait_4_blitter
.No:	mulu.w	#Screen_depth*64,d7		; Calculate blit size
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

;***************************************************************************
; [ Vertically scroll an any-sized block on the screen UNCLIPPED / any planes ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Delta Y (.w)
;        d5 - Number of planes (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
; All registers are restored
;***************************************************************************
VScroll_block:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	VScroll_block_all
	movem.l	d0-d7/a0/a1/a6,-(sp)
	kickGFX	OwnBlitter
	lea.l	Custom,a6
	Wait_4_blitter			; Wait
; --------- Check input ---------------------------
	tst.w	d4			; Scroll at all ?
	beq	.Exit
	move.w	d4,d2			; Scroll too much ?
	bpl.s	.Skip
	neg.w	d2
.Skip:	cmp.w	d7,d2
	bpl	.Exit
; --------- Ascending / descending logic ----------
	move.w	d6,d2			; Calculate X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	tst.w	d4			; Ascending or descending ?
	bpl.s	.Descending
	add.w	d4,d7			; Adjust height
	neg.w	d4			; Calculate Y3
	add.w	d1,d4
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	bra.s	.Do
.Descending:
	sub.w	d4,d7			; Adjust height
	sub.w	d3,d4			; Calculate Y3
	neg.w	d4
	move.w	d2,d0			; Other corner
	move.w	d3,d1
	move.l	#$09f00002,bltcon0(a6)	; Set blitter control
; --------- Prepare blitter -----------------------
.Do:	move.l	Work_screen,a0		; Calculate screen addresses
	move.l	a0,a1
	jsr	Coord_convert
	add.l	d2,a0
	move.w	d4,d1
	jsr	Coord_convert
	add.l	d2,a1
	move.l	#-1,bltafwm(a6)		; Set mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	move.w	#Bytes_per_line,d2		; Set modulo A & D
	add.w	d6,d6
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
	move.w	d2,bltdmod(a6)
; --------- Blit each plane -----------------------
	move.w	#Bytes_per_plane,d3
	subq.w	#1,d5
.Loop:	Wait_4_blitter
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	d3,a0			; Next plane
	add.w	d3,a1
	dbra	d5,.Loop
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

;***************************************************************************
; [ Vertically scroll an any-sized block on the screen UNCLIPPED / all planes ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Delta Y (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
; All registers are restored
;***************************************************************************
VScroll_block_all:
	movem.l	d0-d7/a0/a1/a6,-(sp)
	kickGFX	OwnBlitter
	lea.l	Custom,a6
	Wait_4_blitter			; Wait
; --------- Check input ---------------------------
	tst.w	d4			; Scroll at all ?
	beq	.Exit
	move.w	d4,d2			; Scroll too much ?
	bpl.s	.Skip
	neg.w	d2
.Skip:	cmp.w	d7,d2
	bpl	.Exit
; --------- Ascending / descending logic ----------
	move.w	d6,d2			; Calculate X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	tst.w	d4			; Ascending or descending ?
	bpl	.Descending
; ---------- Prepare blitter ----------------------
	add.w	d4,d7			; Adjust height
	neg.w	d4			; Calculate Y3
	add.w	d1,d4
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	move.l	Work_screen,a0		; Calculate screen addresses
	move.l	a0,a1
	jsr	Coord_convert
	add.l	d2,a0
	move.w	d4,d1
	jsr	Coord_convert
	add.l	d2,a1
	move.l	#-1,bltafwm(a6)		; Set mask
	move.w	#Bytes_per_plane,d2		; Set modulo A & D
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
	move.w	d2,bltdmod(a6)
; ---------- Blit ---------------------------------
	cmp.w	#127,d7			; Too large ?
	bmi.s	.No1
	move.w	d7,-(sp)
	move.w	#127*Screen_depth*64,d7	; Yes -> Blit first part
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	move.w	(sp)+,d7
	sub.w	#127,d7			; Blit the rest
	add.l	#127*Bytes_per_line,a0
	add.l	#127*Bytes_per_line,a1
	Wait_4_blitter
.No1:	mulu.w	#Screen_depth*64,d7		; Calculate blit size
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	bra	.Exit
; ---------- Prepare blitter ----------------------
.Descending:
	sub.w	d4,d7			; Adjust height
	subq.w	#1,d7
	sub.w	d3,d4			; Calculate Y3
	neg.w	d4
	move.w	d2,d0			; Other corner
	move.w	d3,d1
	move.l	#$09f00002,bltcon0(a6)	; Set blitter control
	move.l	Work_screen,a0		; Calculate screen addresses
	move.l	a0,a1
	jsr	Coord_convert
	add.l	d2,a0
	move.w	d4,d1
	jsr	Coord_convert
	add.l	d2,a1
	move.l	#-1,bltafwm(a6)		; Set mask
	move.w	#Bytes_per_plane,d2		; Set modulo A & D
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
	move.w	d2,bltdmod(a6)
; ---------- Blit ---------------------------------
	cmp.w	#127,d7			; Too large ?
	bmi.s	.No2
	move.w	d7,-(sp)
	move.w	#127*Screen_depth*64,d7	; Yes -> Blit first part
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	move.w	(sp)+,d7
	sub.w	#127,d7			; Blit the rest
	sub.l	#127*Bytes_per_line,a0
	sub.l	#127*Bytes_per_line,a1
	Wait_4_blitter
.No2:	mulu.w	#Screen_depth*64,d7		; Calculate blit size
	add.w	d6,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

;***************************************************************************
; [ Display a chunky block UNCLIPPED ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of block in truncs (must be even) (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to chunky block (.l)
; All registers are restored
;***************************************************************************
Display_chunky_block:
	movem.l	d0-d7/a0-a6,-(sp)
	Wait_4_blitter			; Wait
	move.l	Work_screen,a1		; Get screen address
	jsr	Coord_convert
	add.l	d2,a1
	move.w	#Bytes_per_line,d2		; Calculate line offset
	sub.w	d6,d2
	sub.w	d6,d2
	move.l	#$0f0f0f0f,a2		; Load masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	subq.w	#1,d7
.Loop_Y:	move.w	d6,d3
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
	add.w	d2,a1			; Next line
	dbra	d7,.Loop_Y
	movem.l	(sp)+,d0-d7/a0-a6
	rts
