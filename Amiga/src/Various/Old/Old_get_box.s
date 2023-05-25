;***************************************************************************
; [ Get an any-sized box from the screen CLIPPED / any planes ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
;        d5 - Number of planes (.w)
;        a0 - Pointer to buffer (.l)
; All registers are restored
; Notes :
;  - The horizontal coordinates will be rounded to trunc boundaries.
;***************************************************************************
Get_box:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Get_box_all
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
.Y_OK:	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	CA_X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	CA_Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	cmp.w	CA_X1(a1),d2		; X2 over left edge ?
	bmi	.Exit
	cmp.w	CA_Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip coordinates ---------------------
	move.w	d2,d6			; Calculate width
	sub.w	d0,d6
	lsr.w	#4,d6
	cmp.w	CA_X1(a1),d0		; Check left
	bpl.s	.Check_top
	move.w	CA_X1(a1),d4		; Adjust buffer pointer
	sub.w	d0,d4
	and.w	#$fff0,d4
	lsr.w	#3,d4
	add.w	d4,a0
	move.w	CA_X1(a1),d0
.Check_top:
	cmp.w	CA_Y1(a1),d1		; Check top
	bpl.s	.Check_right
	move.w	CA_Y1(a1),d4		; Adjust buffer pointer
	sub.w	d1,d4
	add.w	d4,d4
	mulu.w	d5,d4
	mulu.w	d6,d4
	add.l	d4,a0
	move.w	CA_Y1(a1),d1
.Check_right:
	cmp.w	CA_X2(a1),d2		; Check right
	ble.s	.Check_bottom
	move.w	CA_X2(a1),d2
.Check_bottom:
	cmp.w	CA_Y2(a1),d3		; Check bottom
	bmi.s	.Done
	move.w	CA_Y2(a1),d3
; ---------- Prepare blitter ----------------------
.Done:	movem.l	d2/d3,-(sp)		; Calculate screen address
	move.l	Work_screen,a1
	jsr	Coord_convert
	add.l	d2,a1	
	movem.l	(sp)+,d2/d3
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	move.w	d2,d4			; Calculate width
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d1,d3			; Calculate height
	addq.w	#1,d3
	move.w	d3,d7			; Calculate blit size
	lsl.w	#6,d7
	add.w	d4,d7
	add.w	d4,d4			; Screen width x 2
	add.w	d6,d6			; Block width x 2
	move.w	#Bytes_per_line,d2		; Set modulo A
	sub.w	d4,d2
	move.w	d2,bltamod(a6)
	move.w	d6,d2			; Set modulo D
	mulu.w	d5,d2			;  = (W x 2) x (D - 1)
	sub.w	d6,d2
	move.w	d2,bltdmod(a6)
; ---------- Blit ---------------------------------
	subq.w	#1,d5
	bra.s	.Entry
.Loop:	Wait_4_blitter
.Entry:	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	d6,a0			; Next plane
	add.w	#Bytes_per_plane,a1
	dbra	d5,.Loop
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

;***************************************************************************
; [ Get an any-sized box from the screen CLIPPED / all planes ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
;        a0 - Pointer to buffer (.l)
; All registers are restored
; Notes :
;  - The horizontal coordinates will be rounded to trunc boundaries.
;***************************************************************************
Get_box_all:
	movem.l	d0-d7/a0/a1/a6,-(sp)
;	kickGFX	OwnBlitter
	lea.l	Custom,a6
	Wait_4_blitter			; Wait
; ---------- Adjust coordinates -------------------

	move.w	d2,d6
	sub.w	d0,d6
	add.w	#16+15,d6
	and.w	#$fff0,d6

	move.w	d0,d2
	add.w	d6,d2
	subq.w	#1,d2

	and.w	#$fff0,d0			; Round to trunc boundaries
;	add.w	#15,d2
;	and.w	#$fff0,d2

	cmp.w	d0,d2			; X1 > X2 ?
	bpl.s	.X_OK
	exg.l	d0,d2			; Yes, swap them
.X_OK:	cmp.w	d1,d3			; Y1 > Y2 ?
	bpl.s	.Y_OK
	exg.l	d1,d3			; Yes, swap them
; ---------- Check if block is off-screen ---------
.Y_OK:	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	CA_X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	CA_Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	cmp.w	CA_X1(a1),d2		; X2 over left edge ?
	bmi	.Exit
	cmp.w	CA_Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip coordinates ---------------------
;	move.w	d2,d6			; Calculate block width
;	sub.w	d0,d6
	lsr.w	#4,d6

	cmp.w	#5,d6
	beq.s	.Whee
	nop
.Whee:

	cmp.w	CA_X1(a1),d0		; Check left
	bpl.s	.Check_top
	move.w	CA_X1(a1),d4		; Adjust buffer pointer
	sub.w	d0,d4
	and.w	#$fff0,d4
	lsr.w	#3,d4
	add.w	d4,a0
	move.w	CA_X1(a1),d0
.Check_top:
	cmp.w	CA_Y1(a1),d1		; Check top
	bpl.s	.Check_right
	move.w	CA_Y1(a1),d4		; Adjust buffer pointer
	sub.w	d1,d4
	mulu.w	#Screen_depth*2,d4
	mulu.w	d6,d4
	add.l	d4,a0
	move.w	CA_Y1(a1),d1
.Check_right:
	cmp.w	CA_X2(a1),d2		; Check right
	ble.s	.Check_bottom
	move.w	CA_X2(a1),d2
.Check_bottom:
	cmp.w	CA_Y2(a1),d3		; Check bottom
	bmi.s	.Done
	move.w	CA_Y2(a1),d3
; ---------- Prepare blitter ----------------------
.Done:	movem.l	d2/d3,-(sp)		; Calculate screen address
	move.l	Work_screen,a1
	jsr	Coord_convert
	add.l	d2,a1	
	movem.l	(sp)+,d2/d3

	move.w	d2,d4			; Calculate screen width
	sub.w	d0,d4
	lsr.w	#4,d4

	kickGFX	OwnBlitter

	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
;	move.w	d2,d4			; Calculate screen width
;	sub.w	d0,d4
;	lsr.w	#4,d4

;	moveq.l	#5,d4
;	moveq.l	#5,d6

	move.w	#Bytes_per_plane,d2		; Set modulo A & D
	sub.w	d4,d2
	sub.w	d4,d2
	move.w	d2,bltamod(a6)
	move.w	d6,d2			; Set modulo D
	add.w	d2,d2
	sub.w	d4,d2
	sub.w	d4,d2
	move.w	d2,bltdmod(a6)
; ---------- Blit ---------------------------------
	sub.w	d1,d3			; Calculate height
	addq.w	#1,d3
	cmp.w	#127,d3			; Too large ?
	bmi.s	.No
	move.w	#127*Screen_depth*64,d7	; Yes -> Blit first part
	add.w	d4,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	sub.w	#127,d3			; Blit the rest
	mulu.w	#127,d6
	add.l	d6,a0
	add.l	#127*Bytes_per_line,a1
	Wait_4_blitter
.No:	move.w	d3,d7			; Calculate blit size
	mulu.w	#Screen_depth*64,d7
	add.w	d4,d7
	move.l	a1,bltapt(a6)		; Set pointers
	move.l	a0,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
.Exit:	kickGFX	DisownBlitter
	movem.l	(sp)+,d0-d7/a0/a1/a6
	rts

