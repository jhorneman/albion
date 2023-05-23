; Graphics routines
; Recolouring
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 28-2-1994

	SECTION	Program,code
;***************************************************************************
; [ Make a box darker CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
; All registers are restored
;***************************************************************************
Darker_box:
	move.l	a4,-(sp)
	lea.l	Darker_table,a4
	jsr	Recolour_marmor_box
	move.l	(sp)+,a4
	rts

;***************************************************************************
; [ Make a box brighter CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
; All registers are restored
;***************************************************************************
Brighter_box:
	move.l	a4,-(sp)
	lea.l	Brighter_table,a4
	jsr	Recolour_marmor_box
	move.l	(sp)+,a4
	rts

;***************************************************************************
; [ Draw a recoloured marmor box on the screen CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
;        a4 - Pointer to recolouring table (.l)
; All registers are restored
;***************************************************************************
Recolour_marmor_box:
	movem.l	d0-d7/a0-a2/a5,-(sp)
	lea.l	-Draw_box_LDS(sp),sp	; Create local variables
	move.l	sp,a5
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
	sub.w	d0,d2			; Store width & height
	addq.w	#1,d2
	move.w	d2,BoxWidth(a5)
	sub.w	d1,d3
	addq.w	#1,d3
	move.w	d3,BoxHeight(a5)
; --------- Try special case ----------------------
	move.w	BoxX1(a5),d0		; A & C in one trunc ?
	lsr.w	#4,d0
	move.w	BoxX2(a5),d1
	lsr.w	#4,d1
	cmp.w	d0,d1
	bne	.Do_A			; No -> normal routine
	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	lea.l	Start_masks,a2		; Get start mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	lea.l	End_masks,a2		; Get end mask
	move.w	BoxX2(a5),d3
	and.w	#$000f,d3
	add.w	d3,d3
	and.w	0(a2,d3.w),d0		; Combine
	jsr	.Do_trunc			; Do trunc
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
	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	lea.l	Start_masks,a2		; Get start mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	jsr	.Do_trunc			; Do trunc
; --------- Try part B (middle truncs) ------------
	move.w	BoxX1(a5),d0		; Adjust X1
	and.w	#$fff0,d0
	add.w	#16,d0
	move.w	d0,BoxX1(a5)
.Do_B:	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	bne.s	.Skip
	add.w	#16,BoxWidth(a5)		; Yes -> do extra trunc
.Skip:	move.w	BoxWidth(a5),d6		; Get width
	lsr.w	#4,d6
	beq	.Do_C
	move.w	BoxHeight(a5),d7		; Get height
	subq.w	#1,d7
	move.w	#Bytes_per_plane,d0		; Calculate offset
	sub.w	d6,d0
	sub.w	d6,d0
	move.w	d0,d1
	add.w	#Bytes_per_plane*3,d0
	add.w	#(Screen_depth-1)*Bytes_per_plane,d1
	btst	#0,d6			; Odd or even ?
	bne	.Loop3
; --------- EVEN loop -----------------------------
.Loop1:	move.w	d6,d5
.Loop2:	jsr	.Recolour_2truncs		; Recolour double truncs
	subq.w	#2,d5
	bne.s	.Loop2
.Next1:	adda.w	d0,a0			; Next line
	adda.w	d1,a1
	dbra	d7,.Loop1
	bra.s	.Do_C
; --------- ODD loop ------------------------------
.Loop3:	move.w	d6,d5
	jsr	.Recolour_1trunc		; Recolour first single trunc
	subq.w	#1,d5
	beq.s	.Next3
.Loop4:	jsr	.Recolour_2truncs		; Recolour double truncs
	subq.w	#2,d5
	bne.s	.Loop4
.Next3:	adda.w	d0,a0			; Next line
	adda.w	d1,a1
	dbra	d7,.Loop3
; --------- Try part C (right trunc) --------------
.Do_C:	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	beq	.Exit			; Yes -> exit
	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX2(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	lea.l	End_masks,a2		; Get end mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	jsr	.Do_trunc			; Do trunc
.Exit:	lea.l	Draw_box_LDS(sp),sp		; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a2/a5
	rts

; [ Draw one masked column ]
;   IN : d0 - Mask (.w)
;        a0 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
;        a5 - Pointer to local variables (.l)
; Changed registers : d5,d6,d7,a0,a1
.Do_trunc:
	move.w	#Bytes_per_plane*4-2,d5
	move.w	#Bytes_per_line-2,d6
	move.w	BoxHeight(a5),d7
	subq.w	#1,d7
.Loop5:	jsr	.Recolour_masked_trunc
	add.w	d5,a0
	add.w	d6,a1
	dbra	d7,.Loop5
	rts

; [ Recolour 16 masked pixels in 4 bitplanes ]
;   IN : d0 - Mask (.w)
;        a0 - Pointer to marmor slab (.l)
;        a1 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
; Changed registers : a0,a1
.Recolour_masked_trunc:
	movem.l	d0-d7,-(sp)
	move.w	d0,d4
	move.w	Bytes_per_plane(a0),d1	; Read 16	pixels
	move.w	Bytes_per_plane*2(a0),d2	;  in 4 bitplanes
	move.w	Bytes_per_plane*3(a0),d3
	move.w	(a0)+,d0
	and.w	d4,d0			; Mask
	and.w	d4,d1
	and.w	d4,d2
	and.w	d4,d3
	moveq.l	#0,d6			; Initial d6 is zero
	moveq.l	#8-1,d7			; Do 8 shifts
.Loop6:	moveq.l	#0,d5			; Clear d5
	add.b	d6,d6			; d5 <- [Plane 4] <- d6
	addx.w	d3,d3
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 3] <- d6
	addx.w	d2,d2
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 2] <- d6
	addx.w	d1,d1
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 1] <- d6
	addx.w	d0,d0
	addx.w	d5,d5
	move.b	0(a4,d5.w),d5		; Recolour d5
	lsl.b	#4,d5
	moveq.l	#0,d6
	add.b	d5,d5			; d6 <- [Plane 4] <- d5
	addx.w	d3,d3
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 3] <- d5
	addx.w	d2,d2
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 2] <- d5
	addx.w	d1,d1
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 1] <- d5
	addx.w	d0,d0
	addx.w	d6,d6
	move.b	0(a4,d6.w),d6		; Recolour d6
	lsl.b	#4,d6
	dbra	d7,.Loop6			; Next pixel
	add.b	d6,d6			; [Plane 4] <- d6
	addx.w	d3,d3
	add.b	d6,d6			; [Plane 3] <- d6
	addx.w	d2,d2
	add.b	d6,d6			; [Plane 2] <- d6
	addx.w	d1,d1
	add.b	d6,d6			; [Plane 1] <- d6
	addx.w	d0,d0
	or.w	d4,Bytes_per_plane*4(a1)	; Fill other planes
	or.w	d4,Bytes_per_plane*5(a1)
	or.w	d4,Bytes_per_plane*6(a1)
	or.w	d4,Bytes_per_plane*7(a1)
	not.w	d4
	and.w	d4,Bytes_per_plane(a1)	; Write 16 pixels
	or.w	d1,Bytes_per_plane(a1)	;  in 4 bitplanes
	and.w	d4,Bytes_per_plane*2(a1)
	or.w	d2,Bytes_per_plane*2(a1)
	and.w	d4,Bytes_per_plane*3(a1)
	or.w	d3,Bytes_per_plane*3(a1)
	and.w	d4,(a1)
	or.w	d0,(a1)+
	movem.l	(sp)+,d0-d7
	rts

; [ Recolour 16 pixels in 4 bitplanes ]
;   IN : a0 - Pointer to marmor slab (.l)
;        a1 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
; Changed registers : a0,a1
.Recolour_1trunc:
	movem.l	d0-d3/d5-d7,-(sp)
	move.w	Bytes_per_plane(a0),d1	; Read 16	pixels
	move.w	Bytes_per_plane*2(a0),d2	;  in 4 bitplanes
	move.w	Bytes_per_plane*3(a0),d3
	move.w	(a0)+,d0
	moveq.l	#0,d6			; Initial d6 is zero
	moveq.l	#8-1,d7			; Do 8 shifts
.Loop7:	moveq.l	#0,d5			; Clear d5
	add.b	d6,d6			; d5 <- [Plane 4] <- d6
	addx.w	d3,d3
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 3] <- d6
	addx.w	d2,d2
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 2] <- d6
	addx.w	d1,d1
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 1] <- d6
	addx.w	d0,d0
	addx.w	d5,d5
	move.b	0(a4,d5.w),d5		; Recolour d5
	lsl.b	#4,d5
	moveq.l	#0,d6
	add.b	d5,d5			; d6 <- [Plane 4] <- d5
	addx.w	d3,d3
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 3] <- d5
	addx.w	d2,d2
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 2] <- d5
	addx.w	d1,d1
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 1] <- d5
	addx.w	d0,d0
	addx.w	d6,d6
	move.b	0(a4,d6.w),d6		; Recolour d6
	lsl.b	#4,d6
	dbra	d7,.Loop7			; Next pixel
	add.b	d6,d6			; [Plane 4] <- d6
	addx.w	d3,d3
	add.b	d6,d6			; [Plane 3] <- d6
	addx.w	d2,d2
	add.b	d6,d6			; [Plane 2] <- d6
	addx.w	d1,d1
	add.b	d6,d6			; [Plane 1] <- d6
	addx.w	d0,d0
	move.w	#-1,Bytes_per_plane*4(a1)	; Fill other planes
	move.w	#-1,Bytes_per_plane*5(a1)
	move.w	#-1,Bytes_per_plane*6(a1)
	move.w	#-1,Bytes_per_plane*7(a1)
	move.w	d1,Bytes_per_plane(a1)	; Write 16 pixels
	move.w	d2,Bytes_per_plane*2(a1)	;  in 4 bitplanes
	move.w	d3,Bytes_per_plane*3(a1)
	move.w	d0,(a1)+
	movem.l	(sp)+,d0-d3/d5-d7
	rts

; [ Recolour 32 pixels in 4 bitplanes ]
;   IN : a0 - Pointer to marmor slab (.l)
;        a1 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
; Changed registers : a0,a1
.Recolour_2truncs:
	movem.l	d0-d3/d5-d7,-(sp)
	move.l	Bytes_per_plane(a0),d1	; Read 32	pixels
	move.l	Bytes_per_plane*2(a0),d2	;  in 4 bitplanes
	move.l	Bytes_per_plane*3(a0),d3
	move.l	(a0)+,d0
	moveq.l	#0,d6			; Initial d6 is zero
	moveq.l	#16-1,d7			; Do 16 shifts
.Loop8:	moveq.l	#0,d5			; Clear d5
	add.b	d6,d6			; d5 <- [Plane 4] <- d6
	addx.l	d3,d3
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 3] <- d6
	addx.l	d2,d2
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 2] <- d6
	addx.l	d1,d1
	addx.w	d5,d5
	add.b	d6,d6			; d5 <- [Plane 1] <- d6
	addx.l	d0,d0
	addx.w	d5,d5
	move.b	0(a4,d5.w),d5		; Recolour d5
	lsl.b	#4,d5
	moveq.l	#0,d6			; Clear d6
	add.b	d5,d5			; d6 <- [Plane 4] <- d5
	addx.l	d3,d3
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 3] <- d5
	addx.l	d2,d2
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 2] <- d5
	addx.l	d1,d1
	addx.w	d6,d6
	add.b	d5,d5			; d6 <- [Plane 1] <- d5
	addx.l	d0,d0
	addx.w	d6,d6
	move.b	0(a4,d6.w),d6		; Recolour d6
	lsl.b	#4,d6
	dbra	d7,.Loop8			; Next pixel
	add.b	d6,d6			; [Plane 4] <- d6
	addx.l	d3,d3
	add.b	d6,d6			; [Plane 3] <- d6
	addx.l	d2,d2
	add.b	d6,d6			; [Plane 2] <- d6
	addx.l	d1,d1
	add.b	d6,d6			; [Plane 1] <- d6
	addx.l	d0,d0
	move.l	#-1,Bytes_per_plane*4(a1)	; Fill other planes
	move.l	#-1,Bytes_per_plane*5(a1)
	move.l	#-1,Bytes_per_plane*6(a1)
	move.l	#-1,Bytes_per_plane*7(a1)
	move.l	d1,Bytes_per_plane(a1)	; Write 32 pixels
	move.l	d2,Bytes_per_plane*2(a1)	;  in 4 bitplanes
	move.l	d3,Bytes_per_plane*3(a1)
	move.l	d0,(a1)+
	movem.l	(sp)+,d0-d3/d5-d7
	rts

;***************************************************************************
; [ Convert coordinate pair to marmor slab offset ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;  OUT : d2 - Offset (.l)
; Changed registers : d2
;***************************************************************************
Coord4_convert:
	move.l	d0,-(sp)
	move.w	d1,d2
	mulu.w	#4*Bytes_per_plane,d2
	and.w	#$fff0,d0
	lsr.w	#3,d0
	add.w	d0,d2
	move.l	(sp)+,d0
	rts

;***************************************************************************
; [ Draw a marmor box on the screen CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
; All registers are restored
;***************************************************************************
Marmor_box:
	movem.l	d0-d7/a0-a2/a5,-(sp)
	lea.l	-Draw_box_LDS(sp),sp	; Create local variables
	move.l	sp,a5
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
	sub.w	d0,d2			; Store width & height
	addq.w	#1,d2
	move.w	d2,BoxWidth(a5)
	sub.w	d1,d3
	addq.w	#1,d3
	move.w	d3,BoxHeight(a5)
; --------- Try special case ----------------------
	move.w	BoxX1(a5),d0		; A & C in one trunc ?
	lsr.w	#4,d0
	move.w	BoxX2(a5),d1
	lsr.w	#4,d1
	cmp.w	d0,d1
	bne	.Do_A			; No -> normal routine
	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	lea.l	Start_masks,a2		; Get start mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	lea.l	End_masks,a2		; Get end mask
	move.w	BoxX2(a5),d3
	and.w	#$000f,d3
	add.w	d3,d3
	and.w	0(a2,d3.w),d0		; Combine
	jsr	.Do_trunc			; Do trunc
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
	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	lea.l	Start_masks,a2		; Get start mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	jsr	.Do_trunc			; Do trunc
; --------- Try part B (middle truncs) ------------
	move.w	BoxX1(a5),d0		; Adjust X1
	and.w	#$fff0,d0
	add.w	#16,d0
	move.w	d0,BoxX1(a5)
.Do_B:	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX1(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	bne.s	.Skip
	add.w	#16,BoxWidth(a5)		; Yes -> do extra trunc
.Skip:	move.w	BoxWidth(a5),d6		; Get width
	lsr.w	#4,d6
	beq	.Do_C
	move.w	BoxHeight(a5),d7		; Get height
	subq.w	#1,d7
	move.w	#Bytes_per_plane,d0		; Calculate offset
	sub.w	d6,d0
	sub.w	d6,d0
	move.w	d0,d1
	add.w	#Bytes_per_plane*3,d0
	add.w	#(Screen_depth-1)*Bytes_per_plane,d1
	btst	#0,d6			; Odd or even ?
	bne	.Loop3
; --------- EVEN loop -----------------------------
.Loop1:	move.w	d6,d5
.Loop2:	jsr	.Draw_2truncs		; Recolour double truncs
	subq.w	#2,d5
	bne.s	.Loop2
.Next1:	adda.w	d0,a0			; Next line
	adda.w	d1,a1
	dbra	d7,.Loop1
	bra.s	.Do_C
; --------- ODD loop ------------------------------
.Loop3:	move.w	d6,d5
	jsr	.Draw_1trunc		; Recolour first single trunc
	subq.w	#1,d5
	beq.s	.Next3
.Loop4:	jsr	.Draw_2truncs		; Recolour double truncs
	subq.w	#2,d5
	bne.s	.Loop4
.Next3:	adda.w	d0,a0			; Next line
	adda.w	d1,a1
	dbra	d7,.Loop3
; --------- Try part C (right trunc) --------------
.Do_C:	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	beq	.Exit			; Yes -> exit
	lea.l	Marmor_slab,a0		; Calculate screen address
 	move.l	Work_screen,a1
	move.w	BoxX2(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a1
	jsr	Coord4_convert
	add.l	d2,a0
	lea.l	End_masks,a2		; Get end mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	jsr	.Do_trunc			; Do trunc
.Exit:	lea.l	Draw_box_LDS(sp),sp		; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a2/a5
	rts

; [ Draw one masked column ]
;   IN : d0 - Mask (.w)
;        a0 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
;        a5 - Pointer to local variables (.l)
; Changed registers : d5,d6,d7,a0,a1
.Do_trunc:
	move.w	#Bytes_per_plane*4-2,d5
	move.w	#Bytes_per_line-2,d6
	move.w	BoxHeight(a5),d7
	subq.w	#1,d7
.Loop5:	jsr	.Draw_masked_trunc
	add.w	d5,a0
	add.w	d6,a1
	dbra	d7,.Loop5
	rts

; [ Draw 16 masked pixels in 4 bitplanes ]
;   IN : d0 - Mask (.w)
;        a0 - Pointer to marmor slab (.l)
;        a1 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
; Changed registers : a0,a1
.Draw_masked_trunc:
	movem.l	d0-d4,-(sp)
	move.w	d0,d4
	move.w	Bytes_per_plane(a0),d1	; Read 16	pixels
	move.w	Bytes_per_plane*2(a0),d2	;  in 4 bitplanes
	move.w	Bytes_per_plane*3(a0),d3
	move.w	(a0)+,d0
	and.w	d4,d0			; Mask
	and.w	d4,d1
	and.w	d4,d2
	and.w	d4,d3
	or.w	d4,Bytes_per_plane*4(a1)	; Fill other planes
	or.w	d4,Bytes_per_plane*5(a1)
	or.w	d4,Bytes_per_plane*6(a1)
	or.w	d4,Bytes_per_plane*7(a1)
	not.w	d4
	and.w	d4,Bytes_per_plane(a1)	; Write 16 pixels
	or.w	d1,Bytes_per_plane(a1)	;  in 4 bitplanes
	and.w	d4,Bytes_per_plane*2(a1)
	or.w	d2,Bytes_per_plane*2(a1)
	and.w	d4,Bytes_per_plane*3(a1)
	or.w	d3,Bytes_per_plane*3(a1)
	and.w	d4,(a1)
	or.w	d0,(a1)+
	movem.l	(sp)+,d0-d4
	rts

; [ Draw 16 pixels in 4 bitplanes ]
;   IN : a0 - Pointer to marmor slab (.l)
;        a1 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
; Changed registers : a0,a1
.Draw_1trunc:
	movem.l	d0-d3,-(sp)
	move.w	Bytes_per_plane(a0),d1	; Read 16	pixels
	move.w	Bytes_per_plane*2(a0),d2	;  in 4 bitplanes
	move.w	Bytes_per_plane*3(a0),d3
	move.w	(a0)+,d0
	move.w	#-1,Bytes_per_plane*4(a1)	; Fill other planes
	move.w	#-1,Bytes_per_plane*5(a1)
	move.w	#-1,Bytes_per_plane*6(a1)
	move.w	#-1,Bytes_per_plane*7(a1)
	move.w	d1,Bytes_per_plane(a1)	; Write 16 pixels
	move.w	d2,Bytes_per_plane*2(a1)	;  in 4 bitplanes
	move.w	d3,Bytes_per_plane*3(a1)
	move.w	d0,(a1)+
	movem.l	(sp)+,d0-d3
	rts

; [ Recolour 32 pixels in 4 bitplanes ]
;   IN : a0 - Pointer to marmor slab (.l)
;        a1 - Pointer to screen (.l)
;        a4 - Pointer to recolouring table (.l)
; Changed registers : a0,a1
.Draw_2truncs:
	movem.l	d0-d3,-(sp)
	move.l	Bytes_per_plane(a0),d1	; Read 32	pixels
	move.l	Bytes_per_plane*2(a0),d2	;  in 4 bitplanes
	move.l	Bytes_per_plane*3(a0),d3
	move.l	(a0)+,d0
	move.l	#-1,Bytes_per_plane*4(a1)	; Fill other planes
	move.l	#-1,Bytes_per_plane*5(a1)
	move.l	#-1,Bytes_per_plane*6(a1)
	move.l	#-1,Bytes_per_plane*7(a1)
	move.l	d1,Bytes_per_plane(a1)	; Write 32 pixels
	move.l	d2,Bytes_per_plane*2(a1)	;  in 4 bitplanes
	move.l	d3,Bytes_per_plane*3(a1)
	move.l	d0,(a1)+
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************
; [ Draw a recoloured box on the screen CLIPPED ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Bottom-right X-coordinate (.w)
;        d3 - Bottom-right Y-coordinate (.w)
;        a0 - Pointer to recolouring table (.l)
; All registers are restored
;***************************************************************************
Recolour_box:
	movem.l	d0-d7/a0-a2/a5,-(sp)
	lea.l	-Draw_box_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	move.l	a0,a1
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
	sub.w	d0,d2			; Store width & height
	addq.w	#1,d2
	move.w	d2,BoxWidth(a5)
	sub.w	d1,d3
	addq.w	#1,d3
	move.w	d3,BoxHeight(a5)
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
	lea.l	Start_masks,a2		; Get start mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	lea.l	End_masks,a2		; Get end mask
	move.w	BoxX2(a5),d3
	and.w	#$000f,d3
	add.w	d3,d3
	and.w	0(a2,d3.w),d0		; Combine
	jsr	.Do_trunc			; Do trunc
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
	lea.l	Start_masks,a2		; Get start mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	jsr	.Do_trunc			; Do trunc
; --------- Try part B (middle truncs) ------------
	move.w	BoxX1(a5),d0		; Adjust X1
	and.w	#$fff0,d0
	add.w	#16,d0
	move.w	d0,BoxX1(a5)
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
.Skip:	move.w	BoxWidth(a5),d6		; Get width
	lsr.w	#4,d6
	beq.s	.Do_C
	move.w	BoxHeight(a5),d7		; Get height
	jsr	Recolour_block		; Recolour
; --------- Try part C (right trunc) --------------
.Do_C:	move.w	BoxX2(a5),d0		; Part C empty ?
	and.w	#$000f,d0
	cmp.w	#$000f,d0
	beq	.Exit			; Yes -> exit
	move.l	Work_screen,a0		; Calculate screen address
	move.w	BoxX2(a5),d0
	move.w	BoxY1(a5),d1
	jsr	Coord_convert
	add.l	d2,a0
	lea.l	End_masks,a2		; Get end mask
	add.w	d3,d3
	move.w	0(a2,d3.w),d0
	jsr	.Do_trunc			; Do trunc
.Exit:	lea.l	Draw_box_LDS(sp),sp		; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a2/a5
	rts

; [ Draw one masked column ]
;   IN : d0 - Mask (.w)
;        a0 - Pointer to screen (.l)
;        a1 - Pointer to recolouring table (.l)
;        a5 - Pointer to local variables (.l)
; Changed registers : d7,a0
.Do_trunc:
	move.w	BoxHeight(a5),d7

; !!! Routine continues !!!

;***************************************************************************
; [ Recolour one masked trunc ]
;   IN : d0 - Mask (.w)
;        d7 - Height in pixels (.w)
;        a0 - Pointer to screen (.l)
;        a1 - Pointer to recolouring table (.l)
; All registers are restored
;***************************************************************************
Recolour_masked_trunc:
	movem.l	d0-d7/a0/a2-a6,-(sp)
	move.l	#$0f0f0f0f,a2		; Load masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	move.w	d0,d6			; Save mask
	ror.w	#8,d6
	moveq.l	#0,d2
	subq.w	#1,d7
.Loop_Y:	moveq.l	#2-1,d3
.Loop_X:	move.b	7*Bytes_per_plane(a0),d0	; Read graphics
	and.b	d6,d0
	lsl.w	#8,d0
	move.b	6*Bytes_per_plane(a0),d0
	and.b	d6,d0
	swap	d0
	move.b	5*Bytes_per_plane(a0),d0
	and.b	d6,d0
	lsl.w	#8,d0
	move.b	4*Bytes_per_plane(a0),d0
	and.b	d6,d0
	move.b	3*Bytes_per_plane(a0),d1
	and.b	d6,d1
	lsl.w	#8,d1
	move.b	2*Bytes_per_plane(a0),d1
	and.b	d6,d1
	swap	d1
	move.b	Bytes_per_plane(a0),d1
	and.b	d6,d1
	lsl.w	#8,d1
	move.b	(a0),d1
	and.b	d6,d1
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
	not.b	d6
	and.b	d6,4*Bytes_per_plane(a0)	; Write graphics
	or.b	d0,4*Bytes_per_plane(a0)
	lsr.w	#8,d0
	and.b	d6,5*Bytes_per_plane(a0)
	or.b	d0,5*Bytes_per_plane(a0)
	swap	d0
	and.b	d6,6*Bytes_per_plane(a0)
	or.b	d0,6*Bytes_per_plane(a0)
	lsr.w	#8,d0
	and.b	d6,7*Bytes_per_plane(a0)
	or.b	d0,7*Bytes_per_plane(a0)
	and.b	d6,(a0)
	or.b	d1,(a0)
	lsr.w	#8,d1
	and.b	d6,Bytes_per_plane(a0)
	or.b	d1,Bytes_per_plane(a0)
	swap	d1
	and.b	d6,2*Bytes_per_plane(a0)
	or.b	d1,2*Bytes_per_plane(a0)
	lsr.w	#8,d1
	and.b	d6,3*Bytes_per_plane(a0)
	or.b	d1,3*Bytes_per_plane(a0)
	addq.l	#1,a0			; Other byte
	not.b	d6
	ror.w	#8,d6
	dbra	d3,.Loop_X
	lea.l	Bytes_per_line-2(a0),a0	; Next line
	dbra	d7,.Loop_Y
	movem.l	(sp)+,d0-d7/a0/a2-a6
	rts

;***************************************************************************
; [ Recolour block ]
;   IN : d6 - Width in truncs (.w)
;        d7 - Height in pixels (.w)
;        a0 - Pointer to screen (.l)
;        a1 - Pointer to recolouring table (.l)
; All registers are restored
;***************************************************************************
Recolour_block:
	movem.l	d0-d7/a0/a2-a6,-(sp)
	move.l	#$0f0f0f0f,a2		; Load masks
	move.l	#$f0f0f0f0,a3
	move.l	#$3333cccc,a4
	move.l	#$55005500,a5
	move.l	#$00aa00aa,a6
	moveq.l	#0,d2
	add.w	d6,d6
	subq.w	#1,d7
.Loop_Y:	move.w	d6,d3
	subq.w	#1,d3
.Loop_X:	move.b	7*Bytes_per_plane(a0),d0	; Read graphics
	lsl.w	#8,d0
	move.b	6*Bytes_per_plane(a0),d0
	swap	d0
	move.b	5*Bytes_per_plane(a0),d0
	lsl.w	#8,d0
	move.b	4*Bytes_per_plane(a0),d0
	move.b	3*Bytes_per_plane(a0),d1
	lsl.w	#8,d1
	move.b	2*Bytes_per_plane(a0),d1
	swap	d1
	move.b	Bytes_per_plane(a0),d1
	lsl.w	#8,d1
	move.b	(a0),d1
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
	move.b	d0,4*Bytes_per_plane(a0)	; Write graphics
	lsr.w	#8,d0
	move.b	d0,5*Bytes_per_plane(a0)
	swap	d0
	move.b	d0,6*Bytes_per_plane(a0)
	lsr.w	#8,d0
	move.b	d0,7*Bytes_per_plane(a0)
	move.b	d1,(a0)
	lsr.w	#8,d1
	move.b	d1,Bytes_per_plane(a0)
	swap	d1
	move.b	d1,2*Bytes_per_plane(a0)
	lsr.w	#8,d1
	move.b	d1,3*Bytes_per_plane(a0)
	addq.l	#1,a0			; Next 8 pixels
	dbra	d3,.Loop_X
	move.w	#Bytes_per_line,d0		; Next line
	sub.w	d6,d0
	add.w	d0,a0
	dbra	d7,.Loop_Y
	movem.l	(sp)+,d0-d7/a0/a2-a6
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Darker_table:
	dc.b 0,0,0,1,2,3,4,5
	dc.b 6,7,8,9,10,11,12,13
Brighter_table:
	dc.b 0,3,4,5,6,7,8,9
	dc.b 10,11,12,13,14,15,15,15
	even
