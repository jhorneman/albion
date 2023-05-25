;***************************************************************************
; [ Display the text line buffer ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Silhouette colour (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
;***************************************************************************
Put_line_buffer:
	movem.l	d0-d7/a0-a2/a5/a6,-(sp)
	lea.l	-Put_block_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	kickGFX	OwnBlitter
	lea.l	Custom,a6
	move.w	d0,BlockX1(a5)		; Store X1
	move.w	d6,d2			; Calculate & store X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	move.w	d2,BlockX2(a5)
	move.w	d4,Mask_colour(a5)		; Store input
	moveq.l	#0,d4			; Clear clipping flags
; --------- Check if block is off-screen ----------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	CA_X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	CA_Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	move.w	BlockX2(a5),d2		; X2 over left edge ?
	cmp.w	CA_X1(a1),d2
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	CA_Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; --------- Clip Y-coordinates --------------------
	cmp.w	CA_Y1(a1),d1		; Check top
	bpl.s	.Check_bottom
	move.l	d4,-(sp)			; Adjust height
	move.w	CA_Y1(a1),d4
	sub.w	d1,d4
	sub.w	d4,d7
	mulu.w	#Bytes_per_plane,d4		; Adjust pointer
	add.l	d4,a0
	move.l	(sp)+,d4
	move.w	CA_Y1(a1),d1		; Set new Y-coordinate
.Check_bottom:
	cmp.w	CA_Y2(a1),d3		; Check bottom
	bmi.s	.Done_Y
	sub.w	CA_Y2(a1),d3		; Adjust height
	sub.w	d3,d7
; --------- Clip X-coordinates --------------------
.Done_Y:	cmp.w	CA_X1(a1),d0		; Check left
	bpl.s	.Check_right
	bset	#0,d4			; Indicate
	move.l	d4,-(sp)			; Adjust width
	move.w	CA_X1(a1),d4
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust graphics pointer
	add.w	d4,a0
	move.l	(sp)+,d4
	move.w	CA_X1(a1),d0		; Set new X-coordinate
.Check_right:
	cmp.w	CA_X2(a1),d2		; Check right
	ble.s	.Done_X
	bset	#1,d4			; Indicate
	sub.w	CA_X2(a1),d2		; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
.Done_X:	tst.w	d7			; New height is 0 ?
	ble	.Exit
; --------- Prepare blitter -----------------------
	move.l	Work_screen,a1		; Calculate screen address
	jsr	Coord_convert
	add.l	d2,a1	
	Wait_4_blitter
	moveq.l	#-1,d0			; Set first mask
	btst	#0,d4			; Clipped left ?
	beq.s	.Write_first
	lea.l	Start_masks+2,a2		; Calculate first mask
	move.w	BlockX1(a5),d0
	and.w	#$000f,d0
	eor.w	#$000f,d0
	add.w	d0,d0
	move.w	0(a2,d0.w),d0
.Write_first:
	move.w	d0,bltafwm(a6)		; Write first mask
	moveq.l	#0,d0			; Set last mask	
	btst	#1,d4			; Clipped right ?
	beq.s	.Write_last
	lea.l	End_masks,a2		; Calculate last mask
	move.w	BlockX2(a5),d0
	addq.w	#1,d0
	and.w	#$000f,d0
	eor.w	#$000f,d0
	add.w	d0,d0
	move.w	0(a2,d0.w),d0
.Write_last:
	move.w	d0,bltafwm+2(a6)		; Write last mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	add.w	d6,d6			;  Screen width x 2
	move.w	#Bytes_per_plane,d0		; Calculate modulo A
	sub.w	d6,d0
	move.w	#Bytes_per_line,d2		; Calculate modulo B & D
	sub.w	d6,d2
	btst	#0,d4			; Clipped left ?
	beq.s	.No_left
	subq.w	#2,d0
	subq.w	#2,d2
	addq.w	#1,d7
	subq.l	#2,a0
	subq.l	#2,a1
.No_left:	btst	#1,d4			; Clipped right ?
	bne.s	.Right
	subq.w	#2,d0
	subq.w	#2,d2
	addq.w	#1,d7
.Right:	move.w	d0,bltamod(a6)		; Write modulo values
	move.w	d2,bltbmod(a6)
	move.w	d2,bltdmod(a6)
	clr.w	bltcon1(a6)		; Clear blitter control (!)
; --------- Blit each plane -----------------------
	move.w	BlockX1(a5),d2		; Prepare blitter control
	and.w	#$f,d2
	ror.w	#4,d2
	or.w	#$0d00,d2
	move.w	Mask_colour(a5),d4		; Do
	moveq.l	#Screen_depth,d5
	bra.s	.Entry2
.Loop2:	ror.b	d4			; Determine plane
	bcc.s	.Blank
	move.b	#$fc,d2
	bra.s	.Do
.Blank:	move.b	#$0c,d2
.Do:	Wait_4_blitter
	move.w	d2,bltcon0(a6)
	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltbpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	#Bytes_per_plane,a1		; Next plane
.Entry2:	dbra	d5,.Loop2
.Exit:	kickGFX	DisownBlitter
	lea.l	Put_block_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a2/a5/a6
	rts
