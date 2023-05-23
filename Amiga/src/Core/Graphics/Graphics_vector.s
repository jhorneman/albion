; Graphics routines
; Vector graphics
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-2-1994

	SECTION	Program,code
;*****************************************************************************
; [ Plot pixel CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Colour (.w)
; All registers are restored
; Idea by C.Jungen
; Notes :
;  - This routine will plot {Screen_depth} bitplanes.
;*****************************************************************************
Plot_pixel:
	movem.l	d0-d3/a0-a2,-(sp)
	Wait_4_blitter			; !!!
	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	X1(a0),d0			; X over left edge ?
	bmi	.Exit
	cmp.w	Y1(a0),d1			; Y over top edge ?
	bmi	.Exit
	cmp.w	X2(a0),d0			; X over right edge ?
	bgt	.Exit
	cmp.w	Y2(a0),d1			; Y over bottom edge ?
	bgt	.Exit
	move.l	Work_screen,a0		; Get screen address
	jsr	Coord_convert
	add.l	d2,a0
	btst	#3,d3			; Odd ?
	beq.s	.No
	addq.l	#1,a0			; Yes -> next byte
.No:	and.w	#7,d3			; Get bit number
	eor.w	#7,d3
	lea.l	.Plot_ptrs,a1		; Plot first 4 planes
	move.w	d4,d0
	and.w	#$000f,d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	0(a1,d0.w),a2
	jsr	(a2)
	lea.l	Bytes_per_plane*4(a0),a0	; Plot second 4 planes
	move.w	d4,d0
	and.w	#$00f0,d0
	lsr.w	#2,d0
	move.l	0(a1,d0.w),a2
	jsr	(a2)
.Exit:	movem.l	(sp)+,d0-d3/a0-a2
	rts

.Plot_ptrs:
	dc.l .Plot00,.Plot01,.Plot02,.Plot03
	dc.l .Plot04,.Plot05,.Plot06,.Plot07
	dc.l .Plot08,.Plot09,.Plot10,.Plot11
	dc.l .Plot12,.Plot13,.Plot14,.Plot15

.Plot00:	bclr	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot01:	bset	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot02:	bclr	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot03:	bset	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot04:	bclr	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot05:	bset	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot06:	bclr	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot07:	bset	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bclr	d3,Bytes_per_plane*3(a0)
	rts
.Plot08:	bclr	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot09:	bset	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot10:	bclr	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot11:	bset	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bclr	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot12:	bclr	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot13:	bset	d3,(a0)
	bclr	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot14:	bclr	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts
.Plot15:	bset	d3,(a0)
	bset	d3,Bytes_per_plane(a0)
	bset	d3,Bytes_per_plane*2(a0)
	bset	d3,Bytes_per_plane*3(a0)
	rts

;*****************************************************************************
; [ Get pixel colour CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;  OUT : d4 - Colour (.w)
; Changed registers : d4
; Written by M.Bittner
; Notes :
;  - This routine will get {Screen_depth} bitplanes.
;  - It will return zero if the coordinates are clipped out.
;*****************************************************************************
Get_pixel:
	movem.l	d0-d3/a0,-(sp)
	Wait_4_blitter			; !!!
	moveq.l	#0,d4			; Default
	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	X1(a0),d0			; X over left edge ?
	bmi	.Exit
	cmp.w	Y1(a0),d1			; Y over top edge ?
	bmi	.Exit
	cmp.w	X2(a0),d0			; X over right edge ?
	bgt	.Exit
	cmp.w	Y2(a0),d1			; Y over bottom edge ?
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

;***************************************************************************
; [ Draw a box CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
;        d4 - Colour (.w)
; All registers are restored
;***************************************************************************
Draw_box:
	movem.l	d0-d7/a0-a6,-(sp)
	lea.l	-Draw_box_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	kickGFX	OwnBlitter
	lea.l	Custom,a6
; --------- Adjust coordinates --------------------
	cmp.w	d0,d2			; X1 > X2 ?
	bpl.s	.X_OK
	exg.l	d0,d2			; Yes, swap them
.X_OK:	cmp.w	d1,d3			; Y1 > Y2 ?
	bpl.s	.Y_OK
	exg.l	d1,d3			; Yes, swap them
; --------- Check if box is off-screen ------------
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
; --------- Clip coordinates ----------------------
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
; --------- Store data ----------------------------
.Done:	move.w	d0,BoxX1(a5)		; Store input
	move.w	d1,BoxY1(a5)
	move.w	d2,BoxX2(a5)
	move.w	d3,BoxY2(a5)
	move.w	d4,BoxColour(a5)
	sub.w	d0,d2			; Store width & height
	addq.w	#1,d2
	move.w	d2,BoxWidth(a5)
	sub.w	d1,d3
	addq.w	#1,d3
	move.w	d3,BoxHeight(a5)
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
; --------- Try special case ----------------------
	move.w	BoxX1(a5),d0		; A & C in one trunc ?
	lsr.w	#4,d0
	move.w	BoxX2(a5),d1
	lsr.w	#4,d1
	cmp.w	d0,d1
	bne	.Do_A			; No -> normal routine
	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	Start_masks,a1		; Get start mask
	add.w	d3,d3
	move.w	0(a1,d3.w),d0
	lea.l	End_masks,a1		; Get end mask
	move.w	BoxX2(a5),d3
	and.w	#$000f,d3
	add.w	d3,d3
	and.w	0(a1,d3.w),d0		; Combine & write
	move.w	d0,bltadat(a6)
	jsr	.Do_trunc			; Blit trunc
	bra	.Exit
; --------- Try part A (left trunc) ---------------
.Do_A:	move.w	BoxX2(a5),d0		; Adjust width of B
	and.w	#$000f,d0
	sub.w	d0,BoxWidth(a5)	
	move.w	BoxX1(a5),d0		; Part A empty ?
	and.w	#$000f,d0
	beq	.Do_B			; Yes -> do B
	sub.w	#16,d0			; Adjust width of B
	add.w	d0,BoxWidth(a5)
 	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	Start_masks,a1		; Get start mask
	add.w	d3,d3
	move.w	0(a1,d3.w),bltadat(a6)
	jsr	.Do_trunc			; Blit trunc
; --------- Try part B (middle truncs) ------------
	move.w	BoxX1(a5),d0		; Adjust X1
	and.w	#$fff0,d0
	add.w	#16,d0
	move.w	d0,BoxX1(a5)
	Wait_4_blitter			; Wait
.Do_B:	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	bne.s	.Skip
	add.w	#16,BoxWidth(a5)		; Yes -> do extra trunc
.Skip:	move.w	BoxHeight(a5),d6		; Calculate blit size
	lsl.w	#6,d6
	move.w	BoxWidth(a5),d5
	lsr.w	#4,d5
	beq	.Do_C
	add.w	d5,d6
	move.w	BoxColour(a5),d5		; Get colour
	move.w	BoxWidth(a5),d4		; Set modulo D
	and.w	#$fff0,d4
	lsr.w	#3,d4
	sub.w	#Bytes_per_line,d4
	neg.w	d4
	move.w	d4,bltdmod(a6)
	move.w	#0,bltcon1(a6)		; Set blitter control
	move.w	#$0100,d4
	moveq.l	#Screen_depth-1,d7
.LoopB:	Wait_4_blitter			; Wait
	ror.b	d5			; Determine plane
	scs	d4
	move.w	d4,bltcon0(a6)
	move.l	a0,bltdpt(a6)		; Set D pointer
	move.w	d6,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a0),a0	; Next plane
	dbra	d7,.LoopB
; --------- Try part C (right trunc) --------------
.Do_C:	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	beq	.Exit			; Yes -> exit
	Wait_4_blitter			; Wait
 	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX2(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	End_masks,a1		; Get end mask
	add.w	d3,d3
	move.w	0(a1,d3.w),bltadat(a6)
	jsr	.Do_trunc			; Blit trunc
.Exit:	kickGFX	DisownBlitter
	lea.l	Draw_box_LDS(sp),sp		; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a6
	rts

.Do_trunc:
	move.w	BoxHeight(a5),d6		; Calculate blit size
	lsl.w	#6,d6
	addq.w	#1,d6
	move.w	BoxColour(a5),d5		; Get colour
	move.w	#Bytes_per_line-2,bltbmod(a6)	; Set modulo B & D
	move.w	#Bytes_per_line-2,bltdmod(a6)
	move.w	#0,bltcon1(a6)		; Set blitter control
	moveq.l	#Screen_depth-1,d7
.Loop2:	Wait_4_blitter			; Wait
	ror.b	d5			; Determine plane
	bcc.s	.Zero2
	move.w	#$05fc,bltcon0(a6)
	bra.s	.Go_on2
.Zero2:	move.w	#$050c,bltcon0(a6)
.Go_on2:	move.l	a0,bltbpt(a6)		; Set B & D pointers
	move.l	a0,bltdpt(a6)
	move.w	d6,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a0),a0	; Next plane
	dbra	d7,.Loop2
	rts

	rsreset
BoxX1:	rs.w 1			; Input coordinates
BoxY1:	rs.w 1
BoxX2:	rs.w 1
BoxY2:	rs.w 1
BoxColour:	rs.w 1			; Colour
BoxWidth:	rs.w 1			; Width & height
BoxHeight:	rs.w 1
Draw_box_LDS:	rs.b 0

;***************************************************************************
; [ Draw a vertical line CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Top Y-coordinate (.w)
;        d3 - Bottom Y-coordinate (.w)
;        d4 - Colour (.w)
; All registers are restored
;***************************************************************************
Draw_vline:
	movem.l	d0-d7/a0-a6,-(sp)
	lea.l	-Draw_box_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	kickGFX	OwnBlitter
	lea.l	Custom,a6
; --------- Adjust coordinates --------------------
	cmp.w	d1,d3			; Y1 > Y2 ?
	bpl.s	.Y_OK
	exg.l	d1,d3			; Yes, swap them
; --------- Check if line is off-screen -----------
.Y_OK:	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	X2(a0),d0			; X over right edge ?
	bgt	.Exit
	cmp.w	Y2(a0),d1			; Y1 over bottom edge ?
	bgt	.Exit
	cmp.w	X1(a0),d0			; X over left edge ?
	bmi	.Exit
	cmp.w	Y1(a0),d3			; Y2 over top edge ?
	bmi	.Exit
; --------- Clip coordinates ----------------------
	cmp.w	X1(a0),d0			; Check left
	bpl.s	.Check_top
	move.w	X1(a0),d0
.Check_top:
	cmp.w	Y1(a0),d1			; Check top
	bpl.s	.Check_right
	move.w	Y1(a0),d1
.Check_right:
	cmp.w	X2(a0),d0			; Check right
	ble.s	.Check_bottom
	move.w	X2(a0),d0
.Check_bottom:
	cmp.w	Y2(a0),d3			; Check bottom
	bmi.s	.Done
	move.w	Y2(a0),d3
; --------- Store data ----------------------------
.Done	move.w	d0,BoxX1(a5)		; Store input
	move.w	d1,BoxY1(a5)
	move.w	d3,BoxY2(a5)
	move.w	d4,BoxColour(a5)
	sub.w	d1,d3			; Store height
	addq.w	#1,d3
	move.w	d3,BoxHeight(a5)
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
; --------- Do special case -----------------------
	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	Vline_masks,a1		; Get mask
	add.w	d3,d3
	move.w	0(a1,d3.w),d0
	move.w	d0,bltadat(a6)		; Write
	move.w	BoxHeight(a5),d6		; Calculate blit size
	lsl.w	#6,d6
	addq.w	#1,d6
	move.w	BoxColour(a5),d5		; Get colour
	move.w	#Bytes_per_line-2,bltbmod(a6)	; Set modulo B & D
	move.w	#Bytes_per_line-2,bltdmod(a6)
	move.w	#0,bltcon1(a6)		; Set blitter control
	moveq.l	#Screen_depth-1,d7
.Loop:	Wait_4_blitter			; Wait
	ror.b	d5			; Determine plane
	bcc.s	.Zero
	move.w	#$05fc,bltcon0(a6)
	bra.s	.Go_on
.Zero:	move.w	#$050c,bltcon0(a6)
.Go_on:	move.l	a0,bltbpt(a6)		; Set B & D pointers
	move.l	a0,bltdpt(a6)
	move.w	d6,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a0),a0	; Next plane
	dbra	d7,.Loop
.Exit:	kickGFX	DisownBlitter
	lea.l	Draw_box_LDS(sp),sp		; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;***************************************************************************
; [ Draw a horizontal line CLIPPED ]
;   IN : d0 - Left X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Right X-coordinate (.w)
;        d4 - Colour (.w)
; All registers are restored
;***************************************************************************
Draw_hline:
	movem.l	d0-d7/a0-a6,-(sp)
	lea.l	-Draw_box_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	kickGFX	OwnBlitter
	lea.l	Custom,a6
; --------- Adjust coordinates --------------------
	cmp.w	d0,d2			; X1 > X2 ?
	bpl.s	.X_OK
	exg.l	d0,d2			; Yes, swap them
; --------- Check if line is off-screen -----------
.X_OK:	move.l	CA_Sp,a0			; Get CA
	move.l	(a0),a0
	cmp.w	X2(a0),d0			; X over right edge ?
	bgt	.Exit
	cmp.w	Y2(a0),d1			; Y1 over bottom edge ?
	bgt	.Exit
	cmp.w	X1(a0),d0			; X over left edge ?
	bmi	.Exit
	cmp.w	Y1(a0),d3			; Y2 over top edge ?
	bmi	.Exit
; --------- Clip coordinates ----------------------
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
	cmp.w	Y2(a0),d1			; Check bottom
	bmi.s	.Done
	move.w	Y2(a0),d1
; --------- Store data ----------------------------
.Done:	move.w	d0,BoxX1(a5)		; Store input
	move.w	d1,BoxY1(a5)
	move.w	d2,BoxX2(a5)
	move.w	d4,BoxColour(a5)
	sub.w	d0,d2			; Store width
	addq.w	#1,d2
	move.w	d2,BoxWidth(a5)
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
; --------- Try special case ----------------------
	move.w	BoxX1(a5),d0		; A & C in one trunc ?
	lsr.w	#4,d0
	move.w	BoxX2(a5),d1
	lsr.w	#4,d1
	cmp.w	d0,d1
	bne	.Do_A			; No -> normal routine
	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	Start_masks,a1		; Get start mask
	add.w	d3,d3
	move.w	0(a1,d3.w),d0
	lea.l	End_masks,a1		; Get end mask
	move.w	BoxX2(a5),d3
	and.w	#$000f,d3
	add.w	d3,d3
	and.w	0(a1,d3.w),d0		; Combine & write
	move.w	d0,bltadat(a6)
	jsr	.Do_trunc			; Blit trunc
	bra	.Exit
; --------- Try part A (left trunc) ---------------
.Do_A:	move.w	BoxX2(a5),d0		; Adjust width of B
	and.w	#$000f,d0
	sub.w	d0,BoxWidth(a5)	
	move.w	BoxX1(a5),d0		; Part A empty ?
	and.w	#$000f,d0
	beq	.Do_B			; Yes -> do B
	sub.w	#16,d0			; Adjust width of B
	add.w	d0,BoxWidth(a5)
 	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	Start_masks,a1		; Get start mask
	add.w	d3,d3
	move.w	0(a1,d3.w),bltadat(a6)
	jsr	.Do_trunc			; Blit trunc
; --------- Try part B (middle truncs) ------------
	move.w	BoxX1(a5),d0		; Adjust X1
	and.w	#$fff0,d0
	add.w	#16,d0
	move.w	d0,BoxX1(a5)
	Wait_4_blitter			; Wait
.Do_B:	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	bne.s	.Skip
	add.w	#16,BoxWidth(a5)		; Yes -> do extra trunc
.Skip:	moveq.l	#1*64,d6			; Calculate blit size
	move.w	BoxWidth(a5),d5
	lsr.w	#4,d5
	beq	.Do_C
	add.w	d5,d6
	move.w	BoxColour(a5),d5		; Get colour
	move.w	BoxWidth(a5),d4		; Set modulo D
	and.w	#$fff0,d4
	lsr.w	#3,d4
	sub.w	#Bytes_per_line,d4
	neg.w	d4
	move.w	d4,bltdmod(a6)
	move.w	#0,bltcon1(a6)		; Set blitter control
	move.w	#$0100,d4
	moveq.l	#Screen_depth-1,d7
.LoopB:	Wait_4_blitter			; Wait
	ror.b	d5			; Determine plane
	scs	d4
	move.w	d4,bltcon0(a6)
	move.l	a0,bltdpt(a6)		; Set D pointer
	move.w	d6,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a0),a0	; Next plane
	dbra	d7,.LoopB
; --------- Try part C (right trunc) --------------
.Do_C:	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	beq	.Exit			; Yes -> exit
	Wait_4_blitter			; Wait
 	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX2(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	End_masks,a1		; Get end mask
	add.w	d3,d3
	move.w	0(a1,d3.w),bltadat(a6)
	jsr	.Do_trunc			; Blit trunc
.Exit:	kickGFX	DisownBlitter
	lea.l	Draw_box_LDS(sp),sp		; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a6
	rts

.Do_trunc:
	moveq.l	#1*64+1,d6		; Calculate blit size
	move.w	BoxColour(a5),d5		; Get colour
	move.w	#Bytes_per_line-2,bltbmod(a6)	; Set modulo B & D
	move.w	#Bytes_per_line-2,bltdmod(a6)
	move.w	#0,bltcon1(a6)		; Set blitter control
	moveq.l	#Screen_depth-1,d7
.Loop2:	Wait_4_blitter			; Wait
	ror.b	d5			; Determine plane
	bcc.s	.Zero2
	move.w	#$05fc,bltcon0(a6)
	bra.s	.Go_on2
.Zero2:	move.w	#$050c,bltcon0(a6)
.Go_on2:	move.l	a0,bltbpt(a6)		; Set B & D pointers
	move.l	a0,bltdpt(a6)
	move.w	d6,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a0),a0	; Next plane
	dbra	d7,.Loop2
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Start_masks:
	dc.w $ffff,$7fff,$3fff,$1fff
	dc.w $0fff,$07ff,$03ff,$01ff
	dc.w $00ff,$007f,$003f,$001f
	dc.w $000f,$0007,$0003,$0001
	dc.w 0
End_masks:
	dc.w $8000,$c000,$e000,$f000
	dc.w $f800,$fc00,$fe00,$ff00
	dc.w $ff80,$ffc0,$ffe0,$fff0
	dc.w $fff8,$fffc,$fffe,$ffff
Vline_masks:
	dc.w $8000,$4000,$2000,$1000
	dc.w $0800,$0400,$0200,$0100
	dc.w $0080,$0040,$0020,$0010
	dc.w $0008,$0004,$0002,$0001
