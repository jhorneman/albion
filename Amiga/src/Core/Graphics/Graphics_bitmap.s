; Graphics routines
; Bitmap graphics
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-2-1994

	SECTION	Program,code
;***************************************************************************
; [ Put a 16 by 16 masked block on the screen UNCLIPPED / all planes ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
; Notes :
;   - This routine assumes the blitter is owned.
;***************************************************************************
Put_masked_icon:
	movem.l	d0-d3/d7/a0-a2/a6,-(sp)
	lea.l	Custom,a6
; ---------- Prepare blitter ----------------------
	jsr	Calculate_small_mask_all
	move.l	Work_screen,a1		; Calculate screen address
	and.w	#$fff0,d0
	jsr	Coord_convert
	add.l	d2,a1	
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$0fca0000,bltcon0(a6)	; Set blitter control
	move.w	#16*64+1,d7		; Calculate blit size
	move.w	#0,bltamod(a6)		; Set modulo A & B
	move.w	#(Screen_depth-1)*2,bltbmod(a6)
	move.w	#Bytes_per_line-2,d2	; Set modulo C & D
	move.w	d2,bltcmod(a6)
	move.w	d2,bltdmod(a6)
; ---------- Blit each plane ----------------------
	moveq.l	#Screen_depth-1,d0
	bra.s	.Entry
.Loop:	Wait_4_blitter
.Entry:	move.l	a2,bltapt(a6)		; Set pointers
	move.l	a0,bltbpt(a6)
	move.l	a1,bltcpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	addq.l	#2,a0			; Next plane
	add.w	#Bytes_per_plane,a1
	dbra	d0,.Loop
	movem.l	(sp)+,d0-d3/d7/a0-a2/a6
	rts

;***************************************************************************
; [ Put a 16 by 16 unmasked block on the screen UNCLIPPED / all planes ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
; Notes :
;   - This routine assumes the blitter is owned.
;***************************************************************************
Put_unmasked_icon:
	movem.l	d0-d3/a0/a1/a6,-(sp)
	lea.l	Custom,a6
; ---------- Prepare blitter ----------------------
	move.l	Work_screen,a1		; Calculate screen address
	and.w	#$fff0,d0
	jsr	Coord_convert
	add.l	d2,a1
	Wait_4_blitter			; Wait
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	move.w	#0,bltamod(a6)		; Set modulos
	move.w	#Bytes_per_plane-2,bltdmod(a6)
; ---------- Do all planes ------------------------
	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltdpt(a6)
	move.w	#64*16*Screen_depth+1,bltsize(a6)	; Set blit size & start
	movem.l	(sp)+,d0-d3/a0/a1/a6
	rts

;***************************************************************************
; [ Put an any-sized masked block on the screen CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Base colour (if d5 <> Screen_depth) (.w)
;        d5 - Number of planes {0...Screen_depth} (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
;***************************************************************************
Put_masked_block:
;	tst.b	Blitter_or_CPU		; CPU ?
;	bne	Put_masked_block2
	movem.l	d0-d7/a0-a3/a5/a6,-(sp)
	lea.l	-Put_block_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned1
	kickGFX	OwnBlitter		; No
.Owned1:	lea.l	Custom,a6
	move.w	d0,BlockX1(a5)		; Store X1
	move.w	d6,d2			; Calculate & store X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	move.w	d2,BlockX2(a5)
	move.w	d4,Mask_colour(a5)		; Store input
	moveq.l	#0,d4			; Clear clipping flags
	jsr	Calculate_mask		; Calculate mask
; ---------- Check if block is off-screen ---------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0			; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1			; Y1 over bottom edge ?
	bgt	.Exit
	move.w	BlockX2(a5),d2		; X2 over left edge ?
	cmp.w	X1(a1),d2
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3			; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1			; Check top
	bpl.s	.Check_bottom
	move.l	d4,-(sp)			; Adjust height
	move.w	Y1(a1),d4
	sub.w	d1,d4
	sub.w	d4,d7
	add.w	d4,d4			; Adjust mask pointer
	mulu.w	d6,d4
	add.l	d4,a2
	mulu.w	d5,d4			; Adjust graphics pointer
	add.l	d4,a0
	move.l	(sp)+,d4
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
	bset	#0,d4			; Indicate
	move.l	d4,-(sp)			; Adjust width
	move.w	X1(a1),d4
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust graphics pointer
	add.w	d4,a0
	add.w	d4,a2			; Adjust mask pointer
	move.l	(sp)+,d4
	move.w	X1(a1),d0			; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2			; Check right
	ble.s	.Done_X
	bset	#1,d4			; Indicate
	sub.w	X2(a1),d2			; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
.Done_X:	tst.w	d7			; New height is 0 ?
	ble	.Exit
; ---------- REGISTER CONTENTS : ------------------

;	d0 - Clipped X-coordinate, rounded to trunc boundary (.w)
;	d1 - Clipped Y-coordinate (.w)
;	d3 - Real width of block in truncs (.w)
;	d4 - Clipping flags (.w)
;	d5 - Number of planes (.w)
;	d6 - Visible width of block in truncs (.w)
;	d7 - Visible height of block in lines (.w)

; ---------- Prepare blitter ----------------------
	move.l	d3,-(sp)			; Calculate screen address
	move.l	Work_screen,a1
	jsr	Coord_convert
	add.l	d2,a1
	move.l	(sp)+,d3
	Wait_4_blitter
	moveq.l	#-1,d0			; Set first mask
	btst	#0,d4			; Clipped left ?
	beq.s	.Write_first
	lea.l	Start_masks+2,a3		; Yes -> Get first mask
	move.w	BlockX1(a5),d0
	and.w	#$000f,d0
	eor.w	#$000f,d0
	add.w	d0,d0
	move.w	0(a3,d0.w),d0
.Write_first:
	move.w	d0,bltafwm(a6)		; Write first mask
	moveq.l	#0,d0			; Set last mask	
	btst	#1,d4			; Clipped right ?
	beq.s	.Write_last
	lea.l	End_masks,a3		; Yes -> Get last mask
	move.w	BlockX2(a5),d0
	addq.w	#1,d0
	and.w	#$000f,d0
	eor.w	#$000f,d0
	add.w	d0,d0
	move.w	0(a3,d0.w),d0
.Write_last:
	move.w	d0,bltafwm+2(a6)		; Write last mask
	move.w	BlockX1(a5),d2		; Set blitter control
	and.w	#$f,d2
	ror.w	#4,d2
	move.w	d2,bltcon1(a6)
	or.w	#$0fca,d2
	move.w	d2,bltcon0(a6)
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	add.w	d3,d3			;  Block width x 2
	add.w	d6,d6			;  Screen width x 2
	move.w	d3,d0			; Calculate modulo A
	sub.w	d6,d0
	move.w	d3,d1			; Calculate modulo B
	mulu.w	d5,d1
	sub.w	d6,d1
	move.w	#Bytes_per_line,d2		; Calculate modulo C & D
	sub.w	d6,d2
	btst	#0,d4			; Clipped left ?
	beq.s	.No_left
	subq.w	#2,d0
	subq.w	#2,d1
	subq.w	#2,d2
	addq.w	#1,d7
	subq.l	#2,a0
	subq.l	#2,a1
	subq.l	#2,a2
.No_left:	btst	#1,d4			; Clipped right ?
	bne.s	.Right
	subq.w	#2,d0
	subq.w	#2,d1
	subq.w	#2,d2
	addq.w	#1,d7
.Right:	move.w	d0,bltamod(a6)		; Write modulo values
	move.w	d1,bltbmod(a6)
	move.w	d2,bltcmod(a6)
	move.w	d2,bltdmod(a6)
; ---------- Blit each plane ----------------------
	move.w	d5,-(sp)
	bra.s	.Entry1
.Loop1:	Wait_4_blitter
	move.l	a2,bltapt(a6)		; Set pointers
	move.l	a0,bltbpt(a6)
	move.l	a1,bltcpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	d3,a0			; Next plane
	add.w	#Bytes_per_plane,a1
.Entry1:	dbra	d5,.Loop1
	move.w	(sp)+,d0
; ---------- Blit each remaining plane ------------
	cmp.w	#Screen_depth,d0		; Any ?
	beq	.Exit
	move.w	Mask_colour(a5),d4
	ror.b	d0,d4
	Wait_4_blitter			; Set new modulo B
	move.w	d2,bltbmod(a6)
	clr.w	bltcon1(a6)		; Clear blitter control (!)
	move.w	BlockX1(a5),d2		; Prepare blitter control
	and.w	#$f,d2
	ror.w	#4,d2
	or.w	#$0d00,d2
	moveq.l	#Screen_depth,d5		; Do
	sub.w	d0,d5
	bra.s	.Entry2
.Loop2:	ror.b	d4			; Determine plane
	bcc.s	.Blank
	move.b	#$fc,d2
	bra.s	.Do
.Blank:	move.b	#$0c,d2
.Do:	Wait_4_blitter
	move.w	d2,bltcon0(a6)
	move.l	a2,bltapt(a6)		; Set pointers
	move.l	a1,bltbpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	#Bytes_per_plane,a1		; Next plane
.Entry2:	dbra	d5,.Loop2
.Exit:	move.b	Mask_buffer_handle,d0	; Destroy mask buffer
	beq.s	.No_buf			; (if any)
	jsr	Free_pointer
	jsr	Free_memory
.No_buf:	clr.b	Mask_buffer_handle
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned2
	kickGFX	DisownBlitter		; No
.Owned2:	lea.l	Put_block_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a3/a5/a6
	rts

	rsreset
BlockX1:	rs.w 1
BlockX2:	rs.w 1
Mask_colour:	rs.w 1
Put_block_LDS:	rs.b 0

;***************************************************************************
; [ Put an any-sized masked block on the screen CLIPPED using the CPU ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Base colour (if d5 <> Screen_depth) (.w)
;        d5 - Number of planes {0...Screen_depth} (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
;***************************************************************************
Put_masked_block2:
	movem.l	d0-d7/a0-a6,-(sp)
	Wait_4_blitter
	lea.l	-Put_block2_LDS(sp),sp	; Create local variables
	move.l	sp,a6
	move.w	d0,BlockX1(a6)		; Store X1
	move.w	d6,d2			; Calculate & store X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	move.w	d2,BlockX2(a6)
	move.w	d4,Mask_colour(a6)		; Store input
	move.w	d5,Block_depth(a6)
	clr.w	Clip_left_flag(a6)		; Clear flags
	jsr	Calculate_mask_CPU		; Calculate mask
; ---------- Check if block is off-screen ---------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	move.w	BlockX2(a6),d2		; X2 over left edge ?
	cmp.w	X1(a1),d2
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1		; Check top
	bpl.s	.Check_bottom
	move.w	Y1(a1),d4		; Adjust height
	sub.w	d1,d4
	sub.w	d4,d7
	add.w	d4,d4			; Adjust mask pointer
	mulu.w	d6,d4
	add.l	d4,a2
	mulu.w	d5,d4			; Adjust graphics pointer
	add.l	d4,a0
	move.w	Y1(a1),d1		; Set new Y-coordinate
.Check_bottom:
	cmp.w	Y2(a1),d3		; Check bottom
	bmi.s	.Done_Y
	sub.w	Y2(a1),d3		; Adjust height
	sub.w	d3,d7
; ---------- Clip X-coordinates -------------------
.Done_Y:	move.w	d6,d3			; Save width of block
	cmp.w	X1(a1),d0		; Check left
	bpl.s	.Check_right
	st	Clip_left_flag(a6)		; Clip !
	move.w	X1(a1),d4		; Adjust width
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust graphics pointer
	add.w	d4,a0
	add.w	d4,a2			; Adjust mask pointer
	move.w	X1(a1),d0		; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2		; Check right
	ble.s	.Done_X
	st	Clip_right_flag(a6)		; Clip !
	sub.w	X2(a1),d2		; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
.Done_X:	tst.w	d7
	ble	.Exit

; ---------- REGISTER CONTENTS : ------------------

;	d0 - Clipped X-coordinate, rounded to trunc boundary (.w)
;	d1 - Clipped Y-coordinate (.w)
;	d3 - Real width of block in truncs (.w)
;	d5 - Number of planes (.w)
;	d6 - Visible width of block in truncs (.w)
;	d7 - Visible height of block in lines (.w)

; ---------- Prepare ------------------------------
	add.w	d3,d3			; Store local variables
	move.w	d3,Size_of_gfx_line(a6)
	move.w	d6,Nr_visible_truncs(a6)
	move.l	Work_screen,a1		; Calculate screen address
	jsr	Coord_convert
	add.l	d2,a1
	move.w	BlockX1(a6),d0		; Calculate shift value
	and.w	#$000f,d0
; ---------- Do -----------------------------------
	subq.w	#1,d7			; DBRA correction
.Loop_Y:	move.w	Block_depth(a6),d6		; Do planes
	bra.s	.Entry_Z1
.Loop_Z1:	move.l	a0,a3			; Duplicate pointers
	move.l	a1,a4
	move.l	a2,a5

	moveq.l	#0,d3			; Clear
	moveq.l	#0,d4
	tst.b	Clip_left_flag(a6)		; Clipped left ?
	beq.s	.No_first1

	moveq.l	#0,d1			; Get mask
	move.w	-2(a5),d1
	ror.l	d0,d1			; Rotate
	swap	d1			; Set first old mask
	move.w	d1,d4

	moveq.l	#0,d1			; Get graphics
	move.w	-2(a3),d1
	ror.l	d0,d1			; Rotate
	swap	d1			; Set first old graphics
	move.w	d1,d3
.No_first1:
	move.w	Nr_visible_truncs(a6),d5
	bra.s	.Entry_X1
.Loop_X1:
	moveq.l	#0,d1			; Get mask
	move.w	(a5)+,d1
	ror.l	d0,d1			; Rotate
	move.l	d1,d2			; Save
	swap	d2
	or.w	d4,d1			; Merge with old mask
	move.w	d2,d4			; Set new old mask
	not.w	d1			; Mask off screen
	and.w	d1,(a4)
	moveq.l	#0,d1			; Get graphics
	move.w	(a3)+,d1
	ror.l	d0,d1			; Rotate
	move.l	d1,d2			; Save
	swap	d2
	or.w	d3,d1			; Merge with old graphics
	move.w	d2,d3			; Set new old graphics
	or.w	d1,(a4)+			; Put graphics on screen

.Entry_X1:	dbra	d5,.Loop_X1		; Next trunc

	tst.b	Clip_right_flag(a6)		; Clipped right ?
	bne.s	.No_last
	not.w	d4			; Do last trunc
	and.w	d4,(a4)
	or.w	d3,(a4)
.No_last:
	add.w	Size_of_gfx_line(a6),a0	; Next plane
	lea.l	Bytes_per_plane(a1),a1
.Entry_Z1:	dbra	d6,.Loop_Z1

	move.w	Block_depth(a6),d1		; Any remaining planes ?
	cmp.w	#Screen_depth,d1
	beq	.Next_Y
	move.w	Mask_colour(a6),d5		; Yes
	ror.b	d1,d5
	moveq.l	#Screen_depth,d6		; Do remaining planes
	sub.w	d1,d6
	subq.w	#1,d6
.Loop_Z2:	move.l	a1,a4			; Duplicate pointers
	move.l	a2,a5

	moveq.l	#0,d4			; Clear
	tst.b	Clip_left_flag(a6)		; Clipped left ?
	beq.s	.No_first2

	moveq.l	#0,d1			; Get mask
	move.w	-2(a5),d1
	ror.l	d0,d1			; Rotate
	swap	d1			; Set first old mask
	move.w	d1,d4
.No_first2:

	ror.b	d5			; Set or clear plane ?	
	bcs.s	.Set
	move.w	d5,-(sp)
	move.w	Nr_visible_truncs(a6),d5	; Clear plane
	bra.s	.Entry_X2A
.Loop_X2A:
	moveq.l	#0,d1			; Get mask
	move.w	(a5)+,d1
	ror.l	d0,d1			; Rotate
	move.l	d1,d2			; Save
	swap	d2
	or.w	d4,d1			; Merge with old mask
	move.w	d2,d4			; Set new old mask
	not.w	d1			; Put on screen
	and.w	d1,(a4)+

.Entry_X2A:
	dbra	d5,.Loop_X2A		; Next trunc

	tst.b	Clip_right_flag(a6)		; Clipped right ?
	bne.s	.Next_Z2
	not.w	d4			; Do last trunc
	and.w	d4,(a4)

	bra.s	.Next_Z2
.Set:	move.w	d5,-(sp)
	move.w	Nr_visible_truncs(a6),d5	; Set plane
	bra.s	.Entry_X2B
.Loop_X2B:
	moveq.l	#0,d1			; Get mask
	move.w	(a5)+,d1
	ror.l	d0,d1			; Rotate
	move.l	d1,d2			; Save
	swap	d2
	or.w	d4,d1			; Merge with old mask
	move.w	d2,d4			; Set new old mask
	or.w	d1,(a4)+			; Put on screen
.Entry_X2B:
	dbra	d5,.Loop_X2B		; Next trunc

	tst.b	Clip_right_flag(a6)		; Clipped right ?
	bne.s	.Next_Z2

	or.w	d4,(a4)			; Do last trunc

.Next_Z2:	move.w	(sp)+,d5
	lea.l	Bytes_per_plane(a1),a1	; Next plane
	dbra	d6,.Loop_Z2
.Next_Y:	add.w	Size_of_gfx_line(a6),a2	; Next line
	dbra	d7,.Loop_Y
; ---------- Exit ---------------------------------
.Exit:	move.b	Mask_buffer_handle,d0	; Destroy mask buffer
	beq.s	.No_buf			; (if any)
	jsr	Free_pointer
	jsr	Free_memory
.No_buf:	clr.b	Mask_buffer_handle
	lea.l	Put_block2_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a6
	rts

	rsreset
	rs.w 1
	rs.w 1
	rs.w 1
Size_of_gfx_line:	rs.w 1
Block_depth:	rs.w 1
Nr_visible_truncs:	rs.w 1
Clip_left_flag:	rs.b 1
Clip_right_flag:	rs.b 1
Put_block2_LDS:	rs.b 0

;***************************************************************************
; [ Put an any-sized masked SILHOUETTE on the screen CLIPPED ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Silhouette colour (.w)
;        d5 - Number of planes of graphics {0...Screen_depth} (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
;***************************************************************************
Put_masked_silhouette:
	movem.l	d0-d7/a0-a3/a5/a6,-(sp)
	lea.l	-Put_block_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned1
	kickGFX	OwnBlitter		; No
.Owned1:	lea.l	Custom,a6
	move.w	d0,BlockX1(a5)		; Store X1
	move.w	d6,d2			; Calculate & store X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	move.w	d2,BlockX2(a5)
	move.w	d4,Mask_colour(a5)		; Store input
	moveq.l	#0,d4			; Clear clipping flags
	jsr	Calculate_mask		; Calculate mask
; --------- Check if block is off-screen ----------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	move.w	BlockX2(a5),d2		; X2 over left edge ?
	cmp.w	X1(a1),d2
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1		; Check top
	bpl.s	.Check_bottom
	move.l	d4,-(sp)			; Adjust height
	move.w	Y1(a1),d4
	sub.w	d1,d4
	sub.w	d4,d7
	add.w	d4,d4			; Adjust mask pointer
	mulu.w	d6,d4
	add.l	d4,a2
	move.l	(sp)+,d4
	move.w	Y1(a1),d1		; Set new Y-coordinate
.Check_bottom:
	cmp.w	Y2(a1),d3		; Check bottom
	bmi.s	.Done_Y
	sub.w	Y2(a1),d3		; Adjust height
	sub.w	d3,d7
; ---------- Clip X-coordinates -------------------
.Done_Y:	move.w	d6,d3			; Save width of block
	cmp.w	X1(a1),d0		; Check left
	bpl.s	.Check_right
	bset	#0,d4			; Indicate
	move.l	d4,-(sp)			; Adjust width
	move.w	X1(a1),d4
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust mask pointer
	move.l	(sp)+,d4
	move.w	X1(a1),d0		; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2		; Check right
	ble.s	.Done_X
	bset	#1,d4			; Indicate
	sub.w	X2(a1),d2		; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
.Done_X:	tst.w	d7			; New height is 0 ?
	ble	.Exit
; ---------- Prepare blitter ----------------------
	move.l	d3,-(sp)			; Calculate screen address
	move.l	Work_screen,a1
	jsr	Coord_convert
	add.l	d2,a1	
	move.l	(sp)+,d3
	Wait_4_blitter
	moveq.l	#-1,d0			; Set first mask
	btst	#0,d4			; Clipped left ?
	beq.s	.Write_first
	lea.l	Start_masks+2,a3		; Calculate first mask
	move.w	BlockX1(a5),d0
	and.w	#$000f,d0
	eor.w	#$000f,d0
	add.w	d0,d0
	move.w	0(a3,d0.w),d0
.Write_first:
	move.w	d0,bltafwm(a6)		; Write first mask
	moveq.l	#0,d0			; Set last mask	
	btst	#1,d4			; Clipped right ?
	beq.s	.Write_last
	lea.l	End_masks,a3		; Calculate last mask
	move.w	BlockX2(a5),d0
	addq.w	#1,d0
	and.w	#$000f,d0
	eor.w	#$000f,d0
	add.w	d0,d0
	move.w	0(a3,d0.w),d0
.Write_last:
	move.w	d0,bltafwm+2(a6)		; Write last mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	add.w	d3,d3			;  Block width x 2
	add.w	d6,d6			;  Screen width x 2
	move.w	d3,d0			; Calculate modulo A
	sub.w	d6,d0
	move.w	#Bytes_per_line,d2		; Calculate modulo B & D
	sub.w	d6,d2
	btst	#0,d4			; Clipped left ?
	beq.s	.No_left
	subq.w	#2,d0
	subq.w	#2,d1
	subq.w	#2,d2
	addq.w	#1,d7
	subq.l	#2,a1
	subq.l	#2,a2
.No_left:	btst	#1,d4			; Clipped right ?
	bne.s	.Right
	subq.w	#2,d0
	subq.w	#2,d1
	subq.w	#2,d2
	addq.w	#1,d7
.Right:	move.w	d0,bltamod(a6)		; Write modulo values
	move.w	d2,bltbmod(a6)
	move.w	d2,bltdmod(a6)
	clr.w	bltcon1(a6)		; Clear blitter control (!)
; ---------- Blit each plane ----------------------
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
	move.l	a2,bltapt(a6)		; Set pointers
	move.l	a1,bltbpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	#Bytes_per_plane,a1		; Next plane
.Entry2:	dbra	d5,.Loop2
.Exit:	move.b	Mask_buffer_handle,d0	; Destroy mask buffer
	beq.s	.No_buf			; (if any)
	jsr	Free_pointer
	jsr	Free_memory
.No_buf:	clr.b	Mask_buffer_handle
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned2
	kickGFX	DisownBlitter		; No
.Owned2:	lea.l	Put_block_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a3/a5/a6
	rts

;***************************************************************************
; [ Put an any-sized unmasked block on the screen CLIPPED / any planes ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        d4 - Base colour (if d5 <> Screen_depth) (.w)
;        d5 - Number of planes {0...Screen_depth} (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
;***************************************************************************
Put_unmasked_block:
	cmp.w	#Screen_depth,d5		; All planes ?
	beq	Put_unmasked_block_all
	movem.l	d0-d7/a0-a2/a5/a6,-(sp)
	lea.l	-Put_block_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned1
	kickGFX	OwnBlitter		; No
.Owned1:	lea.l	Custom,a6
	move.w	d4,Mask_colour(a5)		; Store colour
	cmp.w	#Screen_depth,d5		; Mask needed ?
	beq.s	.No_mask
	jsr	Calculate_mask		; Calculate mask
; ---------- Check if block is off-screen ---------
.No_mask:	and.w	#$fff0,d0			; Round to trunc
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
	add.w	d4,d4			; Adjust mask pointer
	mulu.w	d6,d4
	add.w	d4,a2
	mulu.w	d5,d4			; Adjust graphics pointer
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
	add.w	d4,d4			; Adjust graphics pointer
	add.w	d4,a0
	add.w	d4,a2			; Adjust mask pointer
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
	Wait_4_blitter
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	move.w	#Bytes_per_line,d2		; Set modulo D
	add.w	d6,d6
	sub.w	d6,d2
	move.w	d2,bltdmod(a6)
	add.w	d3,d3			; Block width x 2
	move.w	d3,d1			; Calculate mask modulo
	sub.w	d6,d1
	move.w	d3,d2			; Set modulo A
	mulu.w	d5,d2			;  = (W x 2) x (D - 1)
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
; ---------- Blit each plane ----------------------
	move.w	d5,-(sp)
	bra.s	.Entry1
.Loop1:	Wait_4_blitter
	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	d3,a0			; Next plane
	add.w	#Bytes_per_plane,a1
.Entry1:	dbra	d5,.Loop1
	move.w	(sp)+,d0
; ---------- Blit each remaining plane ------------
	cmp.w	#Screen_depth,d0		; Any ?
	beq	.Exit
	move.w	Mask_colour(a5),d4
	ror.b	d0,d4
	Wait_4_blitter			; Set new modulo A
	move.w	d1,bltamod(a6)
	moveq.l	#Screen_depth,d5		; Do
	sub.w	d0,d5
	bra.s	.Entry2
.Loop2:	Wait_4_blitter
	ror.b	d4			; Determine plane
	bcc.s	.Zero
	move.w	#$09f0,bltcon0(a6)
	bra.s	.Go_on
.Zero:	move.w	#$0900,bltcon0(a6)
.Go_on:	move.l	a2,bltapt(a6)		; Set pointers
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	add.w	#Bytes_per_plane,a1		; Next plane
.Entry2:	dbra	d5,.Loop2
.Exit:	move.b	Mask_buffer_handle,d0	; Destroy mask buffer
	beq.s	.No_buf			; (if any)
	jsr	Free_pointer
	jsr	Free_memory
.No_buf:	clr.b	Mask_buffer_handle
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned2
	kickGFX	DisownBlitter		; No
.Owned2:	lea.l	Put_block_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d0-d7/a0-a2/a5/a6
	rts

;***************************************************************************
; [ Put an any-sized unmasked block on the screen CLIPPED / all planes ]
;   IN : d0 - X-coordinate (on trunc boundary) (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
;        a0 - Pointer to graphics (.l)
; All registers are restored
;***************************************************************************
Put_unmasked_block_all:
	movem.l	d0-d7/a0/a1/a5/a6,-(sp)
	lea.l	-Put_block_LDS(sp),sp	; Create local variables
	move.l	sp,a5
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned1
	kickGFX	OwnBlitter		; No
.Owned1:	lea.l	Custom,a6
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
	add.w	d4,d4			; Adjust graphics pointer
	mulu.w	d6,d4
	mulu.w	#Screen_depth,d4
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
	add.w	d4,d4			; Adjust graphics pointer
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
	Wait_4_blitter
	move.l	#-1,bltafwm(a6)		; Set mask
	move.l	#$09f00000,bltcon0(a6)	; Set blitter control
	move.w	#Bytes_per_plane,d2		; Set modulo D
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltdmod(a6)
	add.w	d3,d3			; Block width x 2
	move.w	d3,d2			; Set modulo A
	sub.w	d6,d2
	sub.w	d6,d2
	move.w	d2,bltamod(a6)
; ---------- Blit ---------------------------------
	cmp.w	#127,d7			; Too large ?
	bmi.s	.No
	move.w	d7,-(sp)
	move.w	#127*Screen_depth*64,d7	; Yes -> Blit first part
	add.w	d6,d7
	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
	move.w	(sp)+,d7
	sub.w	#127,d7			; Blit the rest
	mulu.w	#127*Screen_depth,d3
	add.l	d3,a0
	add.l	#127*Bytes_per_line,a1
	Wait_4_blitter
.No:	mulu.w	#Screen_depth*64,d7		; Calculate blit size
	add.w	d6,d7
	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Set blit size & start
.Exit:	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned2
	kickGFX	DisownBlitter		; No
.Owned2:	lea.l	Put_block_LDS(sp),sp	; Destroy local variables
	movem.l	(sp)+,d0-d7/a0/a1/a5/a6
	rts

;***************************************************************************
; [ Display the text line buffer ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
; All registers are restored
;***************************************************************************
Put_line_buffer:
	movem.l	d0-d7/a0-a1/a6,-(sp)
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned1
	kickGFX	OwnBlitter		; No
.Owned1:	lea.l	Custom,a6
	lea.l	Line_buffer,a0
; ---------- Check if block is off-screen ---------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	move.w	d6,d2			; Calculate X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	cmp.w	X1(a1),d2		; X2 over left edge ?
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1		; Check top
	bpl.s	.Check_bottom
	move.w	Y1(a1),d4		; Adjust height
	sub.w	d1,d4
	sub.w	d4,d7
	mulu.w	#Bytes_per_plane,d4		; Adjust graphics pointer
	add.l	d4,a0
	move.w	Y1(a1),d1		; Set new Y-coordinate
.Check_bottom:
	cmp.w	Y2(a1),d3		; Check bottom
	bmi.s	.Done_Y
	sub.w	Y2(a1),d3		; Adjust height
	sub.w	d3,d7
; ---------- Clip X-coordinates -------------------
.Done_Y:	cmp.w	X1(a1),d0		; Check left
	bpl.s	.Check_right
	move.w	X1(a1),d4		; Adjust width
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust graphics pointer
	add.w	d4,a0
	move.w	X1(a1),d0		; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2		; Check right
	bmi.s	.Done_X
	sub.w	X2(a1),d2		; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
.Done_X:	tst.w	d7			; New height is 0 ?
	ble	.Exit
; ---------- Prepare blitter ----------------------
	move.l	Work_screen,a1		; Calculate screen address
	jsr	Coord_convert
	add.l	d2,a1
	Wait_4_blitter
	move.l	#$ffff0000,bltafwm(a6)	; Set mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	addq.w	#1,d7
	add.w	d6,d6			; Width x 2
	move.w	#Bytes_per_plane-2,d0	; Set modulo A
	sub.w	d6,d0
	move.w	d0,bltamod(a6)
	move.w	#Bytes_per_line-2,d0	; Set modulo B & D
	sub.w	d6,d0
	move.w	d0,bltbmod(a6)
	move.w	d0,bltdmod(a6)
; ---------- Do ink -------------------------------
	move.w	Ink_colour,d0		; Get colour
	move.w	#$0dfc,d1
	move.w	#$0d0c,d2
	move.w	#0,bltcon1(a6)
	moveq.l	#Screen_depth-1,d5
	bra.s	.Entry
.Loop:	Wait_4_blitter
.Entry:	ror.b	d0			; Determine plane
	bcc.s	.Zero
	move.w	d1,bltcon0(a6)
	bra.s	.Go_on
.Zero:	move.w	d2,bltcon0(a6)
.Go_on:	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltbpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a1),a1	; Next plane
	dbra	d5,.Loop
.Exit:	Wait_4_blitter
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned2
	kickGFX	DisownBlitter		; No
.Owned2:	movem.l	(sp)+,d0-d7/a0-a1/a6
	rts

;***************************************************************************
; [ Display the text line buffer's shadow ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d6 - Width of block in truncs (.w)
;        d7 - Height of block in pixels (.w)
; All registers are restored
;***************************************************************************
Put_line_buffer_shadow:
	tst.w	Shadow_colour		; Any shadow ?
	bmi	.No_shadow
	movem.l	d0-d7/a0-a1/a6,-(sp)
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned1
	kickGFX	OwnBlitter		; No
.Owned1:	lea.l	Custom,a6
	lea.l	Line_buffer,a0
	addq.w	#1,d1			; Down one
; ---------- Check if block is off-screen ---------
	and.w	#$fff0,d0			; Round to trunc
	move.l	CA_Sp,a1			; Get CA
	move.l	(a1),a1
	cmp.w	X2(a1),d0		; X1 over right edge ?
	bgt	.Exit
	cmp.w	Y2(a1),d1		; Y1 over bottom edge ?
	bgt	.Exit
	move.w	d6,d2			; Calculate X2
	lsl.w	#4,d2
	add.w	d0,d2
	subq.w	#1,d2
	cmp.w	X1(a1),d2		; X2 over left edge ?
	bmi	.Exit
	move.w	d1,d3			; Calculate Y2
	add.w	d7,d3
	subq.w	#1,d3
	cmp.w	Y1(a1),d3		; Y2 over top edge ?
	bmi	.Exit
; ---------- Clip Y-coordinates -------------------
	cmp.w	Y1(a1),d1		; Check top
	bpl.s	.Check_bottom
	move.w	Y1(a1),d4		; Adjust height
	sub.w	d1,d4
	sub.w	d4,d7
	mulu.w	#Bytes_per_plane,d4		; Adjust graphics pointer
	add.l	d4,a0
	move.w	Y1(a1),d1		; Set new Y-coordinate
	bset	#0,d5			; Mark
.Check_bottom:
	cmp.w	Y2(a1),d3		; Check bottom
	bmi.s	.Done_Y
	sub.w	Y2(a1),d3		; Adjust height
	sub.w	d3,d7
	bset	#1,d5			; Mark
; ---------- Clip X-coordinates -------------------
.Done_Y:	cmp.w	X1(a1),d0		; Check left
	bpl.s	.Check_right
	move.w	X1(a1),d4		; Adjust width
	sub.w	d0,d4
	lsr.w	#4,d4
	sub.w	d4,d6
	add.w	d4,d4			; Adjust graphics pointer
	add.w	d4,a0
	move.w	X1(a1),d0		; Set new X-coordinate
.Check_right:
	cmp.w	X2(a1),d2		; Check right
	bmi.s	.Done_X
	sub.w	X2(a1),d2		; Adjust width
	lsr.w	#4,d2
	sub.w	d2,d6
.Done_X:	tst.w	d7			; New height is 0 ?
	bmi	.Exit
; ---------- Prepare blitter ----------------------
	move.l	Work_screen,a1		; Calculate screen address
	jsr	Coord_convert
	add.l	d2,a1
	Wait_4_blitter
	move.l	#$ffff0000,bltafwm(a6)	; Set mask
	lsl.w	#6,d7			; Calculate blit size
	add.w	d6,d7
	addq.w	#1,d7
	add.w	d6,d6			; Width x 2
	move.w	#Bytes_per_plane-2,d0	; Set modulo A
	sub.w	d6,d0
	move.w	d0,bltamod(a6)
	move.w	#Bytes_per_line-2,d0	; Set modulo B & D
	sub.w	d6,d0
	move.w	d0,bltbmod(a6)
	move.w	d0,bltdmod(a6)
; ---------- Do shadow ----------------------------
	move.w	Shadow_colour,d0		; Get colour
	move.w	#$1dfc,d1
	move.w	#$1d0c,d2
	move.w	#0,bltcon1(a6)
	moveq.l	#Screen_depth-1,d5
	bra.s	.Entry
.Loop:	Wait_4_blitter
.Entry:	ror.b	d0			; Determine plane
	bcc.s	.Zero
	move.w	d1,bltcon0(a6)
	bra.s	.Go_on
.Zero:	move.w	d2,bltcon0(a6)
.Go_on:	move.l	a0,bltapt(a6)		; Set pointers
	move.l	a1,bltbpt(a6)
	move.l	a1,bltdpt(a6)
	move.w	d7,bltsize(a6)		; Blit
	lea.l	Bytes_per_plane(a1),a1	; Next plane
	dbra	d5,.Loop
.Exit:	Wait_4_blitter
	tst.b	Graphics_ops		; Already owned ?
	bne.s	.Owned2
	kickGFX	DisownBlitter		; No
.Owned2:	movem.l	(sp)+,d0-d7/a0-a1/a6
.No_shadow:
	rts
