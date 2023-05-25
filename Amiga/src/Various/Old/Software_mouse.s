; Mouse-pointer handling (display only)
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Mouse_on
	XDEF	Mouse_off
	XDEF	Reset_mouse
	XDEF	Push_MA
	XDEF	Pop_MA
	XDEF	Reset_MA_stack
	XDEF	Push_Mptr
	XDEF	Push_busy_Mptr
	XDEF	Pop_Mptr
	XDEF	Reset_Mptr_stack
	XDEF	Change_Mptr
	XDEF	MA_Sp
	XDEF	Default_MA

	SECTION	Program,code
;*****************************************************************************
; [ Switch the mouse on ]
; All registers are	restored
;*****************************************************************************
Mouse_on:
	tst.b	Suppress_mouse		; Already on ?
	beq.s	.Exit
	cmp.b	#1,Suppress_mouse		; Switching on ?
	bne.s	.No
	jsr	My_vsync			; Yes
	clr.l	Erase_screen
.No:	subq.b	#1,Suppress_mouse		; Decrease flag
.Exit:	rts

;*****************************************************************************
; [ Switch the mouse off ]
; All registers are	restored
;*****************************************************************************
Mouse_off:
	cmpi.b	#-1,Suppress_mouse		; At maximum ?
	beq.s	.Exit
	tst.b	Suppress_mouse		; Switch off ?
	bne.s	.No
	jsr	My_vsync			; Yes
	jsr	Erase_mouse
.No:	addq.b	#1,Suppress_mouse		; Increase flag
.Exit:	rts

;*****************************************************************************
; [ Reset the mouse state ]
; All registers are	restored
;*****************************************************************************
Reset_mouse:
	jsr	My_vsync			; Turn it on
	clr.l	Erase_screen
	clr.b	Suppress_mouse
	rts
	
;*****************************************************************************
; [ Initialize the mouse ]
; All registers are	restored
;*****************************************************************************
Init_mouse:
	movem.l	d0-d3/a0/a1,-(sp)
	move.l	My_window,a0		; Set dummy Intuition pointer
	lea.l	Dummy_sprite,a1
	moveq.l	#1,d0
	moveq.l	#16,d1
	moveq.l	#0,d2
	moveq.l	#0,d3
	kickINTU	SetPointer
	kickGFX	WaitTOF
	kickGFX	WaitTOF
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;*****************************************************************************
; [ Free the mouse ]
; All registers are	restored
;*****************************************************************************
Exit_mouse:
	movem.l	d0/d1/a0/a1,-(sp)
	move.l	My_window,a0		; Restore Intuition pointer
	kickINTU	ClearPointer
	kickGFX	WaitTOF
	kickGFX	WaitTOF
	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Push a MA on the stack ]
;   IN : a0 - Pointer to MA (.l)
; All registers are	restored
;*****************************************************************************
Push_MA:
	move.l	a1,-(sp)
	movea.l	MA_Sp,a1
	addq.l	#4,a1
	cmpa.l	#MAStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,MA_Sp
	jsr	Init_MA			; Initialize new MA
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a MA from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_MA:
	move.l	a0,-(sp)
	movea.l	MA_Sp,a0
	cmpa.l	#MAStack_start,a0		; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,MA_Sp
	move.l	(a0),a0			; Initialize old MA
	jsr	Init_MA
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the MA stack ]
; All registers are	restored
;*****************************************************************************
Reset_MA_stack:
	movem.l	a0/a1,-(sp)
	lea.l	Default_MA,a0		; Reset stack
	lea.l	MAStack_start,a1
	move.l	a1,MA_Sp
	move.l	a0,(a1)
	jsr	Init_MA			; Initialize default MA
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Initialize a MA ]
;   IN : a0 - Pointer to MA (.l)
; All registers are	restored
;*****************************************************************************
Init_MA:
	movem.l	d0/d1,-(sp)
	move.w	Mouse_X,d0		; Load coordinates
	move.w	Mouse_Y,d1
	cmp.w	X1(a0),d0			; X too low ?
	bpl.s	.Left_OK
	move.w	X1(a0),d0
	bra.s	.Check_Y
.Left_OK:	cmp.w	X2(a0),d0			; X too high ?
	blt.s	.Check_Y
	move.w	X2(a0),d0
.Check_Y:	cmp.w	Y1(a0),d1			; Y too low ?
	bpl.s	.Top_OK
	move.w	Y1(a0),d1
	bra.s	.Done
.Top_OK:	cmp.w	Y2(a0),d1			; Y too high ?
	blt.s	.Done
	move.w	Y2(a0),d1
.Done:	move.w	d0,Mouse_X		; Store coordinates
	move.w	d1,Mouse_Y
	movem.l	(sp)+,d0/d1
	rts

;*****************************************************************************
; [ Change the current mouse pointer ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Change_Mptr:
	move.l	a1,-(sp)
	movea.l	Mptr_Sp,a1
	cmp.l	(a1),a0			; Change ?
	beq.s	.Skip
	move.l	a0,(a1)			; Change !
	jsr	My_vsync
	jsr	Init_Mptr
.Skip:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Push a mouse pointer on the stack ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Push_Mptr:
	move.l	a1,-(sp)
	movea.l	Mptr_Sp,a1
	addq.l	#4,a1
	cmpa.l	#MptrStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,Mptr_Sp
	jsr	My_vsync
	jsr	Init_Mptr
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a mouse pointer from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_Mptr:
	move.l	a0,-(sp)
	movea.l	Mptr_Sp,a0
	cmpa.l	#MptrStack_start,a0		; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,Mptr_Sp
	move.l	(a0),a0			; Initialize old mouse
	jsr	My_vsync
	jsr	Init_Mptr
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Push a busy delayed mouse pointer on the stack ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Push_busy_Mptr:
	move.l	a1,-(sp)
	movea.l	Mptr_Sp,a1
	addq.l	#4,a1
	cmpa.l	#MptrStack_end,a1		; Possible ?
	beq.s	.Exit
	clr.w	Busy_timer		; Yes
	move.l	a0,Busy_Mptr		; Store busy pointer
	move.l	-4(a1),(a1)		; Duplicate current
	move.l	a1,Mptr_Sp
	move.w	#Busy_interval,Busy_timer	; Set timer
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Reset the mouse pointer stack ]
; All registers are	restored
;*****************************************************************************
Reset_Mptr_stack:
	movem.l	a0/a1,-(sp)
	lea.l	Default_Mptr,a0		; Reset stack
	lea.l	MptrStack_start,a1
	move.l	a0,(a1)
	move.l	a1,Mptr_Sp
	jsr	My_vsync
	jsr	Init_Mptr
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Initialize a mouse pointer ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
; Notes :
;   - Do not call [ My_vsync ] here since this routine can be called in
;     the vertical blank.
;*****************************************************************************
Init_Mptr:
	movem.l	d0/d5/d7/a0/a1,-(sp)
	addq.l	#4,a0			; Skip hot-spot
	lea.l	Mouse_mask,a1		; Calculate new mask
	moveq.l	#16-1,d7
.Loop_Y:	move.w	(a0)+,d0
	moveq.l	#6-2,d5
.Loop_Z:	or.w	(a0)+,d0
	dbra	d5,.Loop_Z
	not.w	d0
	move.w	d0,(a1)+
	dbra	d7,.Loop_Y
	movem.l	(sp)+,d0/d5/d7/a0/a1
	rts

;*****************************************************************************
; [ Update the mouse ]
; All registers are	restored
;*****************************************************************************
Update_mouse:
	tst.b	Suppress_mouse		; Mouse on ?
	bne.s	.Exit
	Wait_4_blitter
;	jsr	Erase_mouse		; Erase
;	jsr	Draw_mouse		; Draw
	tst.w	Busy_timer		; Timing ?
	beq.s	.Exit
	subq.w	#1,Busy_timer		; Yes -> Is it time ?
	bgt.s	.Exit
	movem.l	a0/a1,-(sp)
	move.l	Busy_Mptr,a0		; Yes -> Switch on
	movea.l	Mptr_Sp,a1
	move.l	a0,(a1)
	jsr	Init_Mptr
	movem.l	(sp)+,a0/a1
.Exit:	rts

;*****************************************************************************
; [ Draw the mouse pointer ]
; All registers are	restored
;*****************************************************************************
Draw_mouse:
	movem.l	d0-d7/a0-a3/a6,-(sp)
	move.l	Mptr_Sp,a0		; Yes -> Get current pointer
	move.l	(a0),a0
	move.w	Mouse_X,d0		; Get mouse coordinates
	move.w	Mouse_Y,d1
	sub.w	(a0)+,d0			; Add hot-spot coordinates
	sub.w	(a0)+,d1
	lea.l	Mouse_mask,a1		; Clip
	lea.l	Mouse_background,a2
	lea.l	Draw_mouse_normal,a6
	move.w	d0,d6
	and.w	#$000f,d6
	eori.w	#$000f,d6
	moveq.l	#16,d7
	tst.w	d0			; Clip left ?
	bpl.s	.No_left
	lea.l	Draw_mouse_left,a6		; Yes
	moveq.l	#0,d0
	bra.s	.No_right
.No_left:	cmp.w	#Trunced_width-16,d0	; Clip right ?
	bmi.s	.No_right
	lea.l	Draw_mouse_right,a6		; Yes
.No_right:	tst.w	d1			; Clip top ?
	bpl.s	.No_top
	add.w	d1,d7			; Yes
	neg.w	d1
	add.w	d1,d1
	add.w	d1,a1
	mulu.w	#6,d1
	add.w	d1,a0
	moveq.l	#0,d1
	bra.s	.No_bott
.No_top:	cmp.w	#Screen_height-16,d1	; Clip bottom ?
	bmi.s	.No_bott
	move.w	#Screen_height,d7		; Yes
	sub.w	d1,d7
.No_bott:	tst.w	d7			; Anything left ?
	beq.s	.Exit
	move.l	On_screen,a3		; Yes
	move.w	d7,Erase_height		; Store erase info
	move.l	a3,Erase_screen
	jsr	Coord_convert		; Get screen address
	add.l	d2,a3
	move.l	d2,Erase_offset
	subq.w	#1,d7			; Draw
	jsr	(a6)
.Exit:	movem.l	(sp)+,d0-d7/a0-a3/a6
	rts

; [ Actual draw routines ]
;    IN: d6 - Shift left value (.w)
;        d7 - Height - 1 (not 0) (.w)
;        a0 - Pointer to mouse graphics (.l)
;        a1 - Pointer to mouse mask (.l)
;        a2 - Pointer to background buffer (.l)
;        a3 - Pointer to screen (.l)
; Changed registers : d0,d1,d5,d7,a0,a1,a2,a3
Draw_mouse_normal:
.Loop_Y:	moveq.l	#-1,d1			; Get mask
	move.w	(a1)+,d1
	rol.l	d6,d1
	moveq.l	#6-1,d5			; Do planes 1 to 6
.Loop_Z:	move.l	(a3),(a2)+		; Save background
	moveq.l	#0,d0			; Write plane
	move.w	(a0)+,d0
	lsl.l	d6,d0
	and.l	d1,(a3)
	or.l	d0,(a3)
	lea.l	Bytes_per_plane(a3),a3	; Next plane
	dbra	d5,.Loop_Z
	not.l	d1
	move.l	(a3),(a2)+		; Save background
	or.l	d1,(a3)			; Set plane 7
	lea.l	Bytes_per_plane(a3),a3
	move.l	(a3),(a2)+		; Save background
	or.l	d1,(a3)			; Set plane 8
	lea.l	Bytes_per_plane(a3),a3
	dbra	d7,.Loop_Y		; Next line
	rts

Draw_mouse_left:
.Loop_Y:	moveq.l	#-1,d1			; Get mask
	move.w	(a1)+,d1
	rol.l	d6,d1
	moveq.l	#6-1,d5			; Do planes 1 to 6
.Loop_Z:	move.l	(a3),(a2)+		; Save background
	moveq.l	#0,d0			; Write plane
	move.w	(a0)+,d0
	lsl.l	d6,d0
	and.w	d1,(a3)
	or.w	d0,(a3)
	lea.l	Bytes_per_plane(a3),a3	; Next plane
	dbra	d5,.Loop_Z
	not.w	d1
	move.l	(a3),(a2)+		; Save background
	or.w	d1,(a3)			; Set plane 7
	lea.l	Bytes_per_plane(a3),a3
	move.l	(a3),(a2)+		; Save background
	or.w	d1,(a3)			; Set plane 8
	lea.l	Bytes_per_plane(a3),a3
	dbra	d7,.Loop_Y		; Next line
	rts

Draw_mouse_right:
	sub.l	#2,Erase_offset
	subq.l	#2,a3
	eori.w	#$000f,d6
.Loop_Y:	moveq.l	#-1,d1			; Get mask
	move.w	(a1)+,d1
	ror.l	d6,d1
	moveq.l	#6-1,d5			; Do planes 1 to 6
.Loop_Z:	move.l	(a3),(a2)+		; Save background
	moveq.l	#0,d0			; Write plane
	move.w	(a0)+,d0
	lsr.w	d6,d0
	and.w	d1,2(a3)
	or.w	d0,2(a3)
	lea.l	Bytes_per_plane(a3),a3	; Next plane
	dbra	d5,.Loop_Z
	not.l	d1
	move.l	(a3),(a2)+		; Save background
	or.w	d1,2(a3)			; Set plane 7
	lea.l	Bytes_per_plane(a3),a3
	move.l	(a3),(a2)+		; Save background
	or.w	d1,2(a3)			; Set plane 8
	lea.l	Bytes_per_plane(a3),a3
	dbra	d7,.Loop_Y		; Next line
	rts

;*****************************************************************************
; [ Erase the mouse pointer ]
; All registers are	restored
;*****************************************************************************
Erase_mouse:
	movem.l	d7/a0/a1,-(sp)
	move.l	Erase_screen,d7		; Yes -> Erase
	beq	.Exit
	lea.l	Mouse_background,a0
	move.l	d7,a1
	add.l	Erase_offset,a1
	move.w	Erase_height,d7
	subq.w	#1,d7
.Loop1:	move.l	(a0)+,(a1)
	move.l	(a0)+,Bytes_per_plane(a1)
	move.l	(a0)+,Bytes_per_plane*2(a1)
	move.l	(a0)+,Bytes_per_plane*3(a1)
	move.l	(a0)+,Bytes_per_plane*4(a1)
	move.l	(a0)+,Bytes_per_plane*5(a1)
	move.l	(a0)+,Bytes_per_plane*6(a1)
	move.l	(a0)+,Bytes_per_plane*7(a1)
	lea.l	Bytes_per_line(a1),a1
	dbra	d7,.Loop1
	tst.b	Double_mouse		; Doubled ?
	beq.s	.Exit
	lea.l	Mouse_background,a0		; Yes -> Erase some more
	move.l	Off_screen,a1
	add.l	Erase_offset,a1
	move.w	Erase_height,d7
	subq.w	#1,d7
.Loop2:	move.l	(a0)+,(a1)
	move.l	(a0)+,Bytes_per_plane(a1)
	move.l	(a0)+,Bytes_per_plane*2(a1)
	move.l	(a0)+,Bytes_per_plane*3(a1)
	move.l	(a0)+,Bytes_per_plane*4(a1)
	move.l	(a0)+,Bytes_per_plane*5(a1)
	move.l	(a0)+,Bytes_per_plane*6(a1)
	move.l	(a0)+,Bytes_per_plane*7(a1)
	lea.l	Bytes_per_line(a1),a1
	dbra	d7,.Loop2
.Exit:	movem.l	(sp)+,d7/a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Suppress_mouse:	dc.b -1
Double_mouse:	dc.b 0
	even
Default_MA:
	dc.w 0,Screen_width-1
	dc.w 0,Screen_height-1


	SECTION	Chip_BSS,bss_c
Dummy_sprite:
	ds.l 3
My_Mptr:
	ds.l 50


	SECTION	Fast_BSS,bss
Mptr_Sp:	ds.l 1			; Mouse pointer stack
MptrStack_start:        
	ds.l Max_Mptr
MptrStack_end:
	
MA_Sp:	ds.l 1			; MA stack
MAStack_start:        
	ds.l Max_MA
MAStack_end:

Busy_timer:	ds.w 1
Busy_Mptr:	ds.l 1

Erase_screen:	ds.l 1
Erase_offset:	ds.l 1
Erase_height:	ds.w 1
Mouse_mask:	ds.w 16
Mouse_background:	ds.l 16*Screen_depth
