; Windows
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 28-2-1994

	XDEF	Window_class
	XDEF	Text_window_class

	SECTION	Program,code
;***************************************************************************	
; [ Initialize method for window objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to OID (.l)
; All registers are restored
;***************************************************************************
Init_window:
	movem.l	d0-d7/a0/a1,-(sp)
	jsr	Set_rectangle
	jsr	Push_MA
	move.l	a0,a1
	move.w	X1(a1),d0			; Calculate width & height
	move.w	Y1(a1),d1
	move.w	X2(a1),d6
	move.w	Y2(a1),d7
	add.w	#16,d6
	and.w	#$fff0,d0
	and.w	#$fff0,d6
	sub.w	d0,d6
	sub.w	d1,d7
	lsr.w	#4,d6
	addq.w	#1,d7
	move.w	#Screen_depth*2,d0		; Calculate buffer size
	mulu.w	d6,d0
	mulu.w	d7,d0
	jsr	Allocate_CHIP		; Allocate
	move.b	d0,Window_bg_handle(a1)
	jsr	Claim_pointer		; Save background
	move.l	d0,a0
	move.w	X1(a1),d0
	and.w	#$fff0,d0
	moveq.l	#Screen_depth,d5
	jsr	Get_block
	Free	Window_bg_handle(a1)
	movem.l	(sp)+,d0-d7/a0/a1
	rts

;***************************************************************************	
; [ Exit method for window objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Exit_window:
	movem.l	d0/d1/d5-d7/a0/a1,-(sp)
	move.l	a0,a1
	move.w	X1(a1),d0			; Calculate width & height
	move.w	Y1(a1),d1
	move.w	X2(a1),d6
	move.w	Y2(a1),d7
	add.w	#16,d6
	and.w	#$fff0,d0
	and.w	#$fff0,d6
	sub.w	d0,d6
	sub.w	d1,d7
	lsr.w	#4,d6
	addq.w	#1,d7
	Get	Window_bg_handle(a1),a0	; Restore background
	moveq.l	#Screen_depth,d5
	jsr	Put_unmasked_block
	move.b	Window_bg_handle(a1),d0
	jsr	Free_pointer
	jsr	Free_memory
	jsr	Pop_MA
	movem.l	(sp)+,d0/d1/d5-d7/a0/a1
	rts

;***************************************************************************	
; [ Init method for text window objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Init_text_window:
	movem.l	d0-d3/a0,-(sp)
	move.w	X1(a0),d0			; Create PA
	move.w	Y1(a0),d1
	move.w	X2(a0),d2
	move.w	Y2(a0),d3
	lea.l	Text_window_PA(a0),a0
	add.w	#9+4,d0
	add.w	#9+2,d1
	sub.w	#16+Scroll_bar_width,d2
	sub.w	#8+2,d3
	move.w	d0,X1(a0)
	move.w	d1,Y1(a0)
	move.w	d2,X2(a0)
	move.w	d3,Y2(a0)
	move.w	OID_Ink(a1),PA_Ink(a0)	; Set rest of PA
	move.w	OID_Shadow(a1),PA_Shadow(a0)
	move.w	OID_Paper(a1),PA_Paper(a0)
	movem.l	(sp)+,d0-d3/a0
	rts

;***************************************************************************	
; [ Exit method for text window objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Exit_text_window:
	rts

;***************************************************************************	
; [ Draw method for text window objects ]
;   IN : a0 - Pointer to object (.l)
; All registers are restored
;***************************************************************************
Draw_text_window:
	movem.l	d0-d3/a0/a1,-(sp)
	move.l	a0,a1
	move.w	X1(a1),d0			; Draw darker box
	move.w	Y1(a1),d1
	move.w	X2(a1),d2
	move.w	Y2(a1),d3
	add.w	#9,d0
	add.w	#9,d1
	sub.w	#8,d2
	sub.w	#8,d3
	lea.l	Darker_table,a0
	jsr	Recolour_box
	sub.w	#9,d0			; Draw border
	sub.w	#9,d1
	move.w	X2(a1),d2
	move.w	Y2(a1),d3
	sub.w	d0,d2
	sub.w	d1,d3
	addq.w	#1,d2
	addq.w	#1,d3
	jsr	Draw_window_border
	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************	
; [ Print method for text window objects ]
;   IN : a0 - Pointer to object (.l)
;        a1 - Pointer to text (.l)
; All registers are restored
;***************************************************************************
Print_text_window:
	movem.l	a0/a1,-(sp)

	ifne	FALSE
	move.l	a0,a1
	Make_OID	Scroll_bar,a1		; Make scroll bar
	move.w	IL_nr_items(a0),OID_Total_units(a1)
	move.w	OID_IL_width(a2),OID_Units_width(a1)
	move.w	OID_IL_height(a2),OID_Units_height(a1)
	move.w	d2,d0
	add.w	#Between,d0
	move.w	d0,OID_X(a1)
	clr.w	OID_Y(a1)
	move.w	d3,OID_Scroll_bar_height(a1)
	move.w	Object_self(a0),d0		; Add
	move.l	a0,-(sp)
	lea.l	Scroll_bar_class,a0
	jsr	Add_object
	move.l	(sp)+,a0
	Free_OID
	endc

	lea.l	Text_window_PA(a0),a0
	jsr	Push_PA
	move.l	a1,a0
	jsr	Process_text

	jsr	Display_processed_text
	Pop	PA
	movem.l	(sp)+,a0/a1
	rts




;***************************************************************************	
; [ Draw window border ]
;   IN : d0 - Left X-coordinate (.w)
;        d1 - Top Y-coordinate (.w)
;        d2 - Width of border in pixels (.w)
;        d3 - Height of border in pixels (.w)
; All registers are restored
;***************************************************************************
Draw_window_border:
	movem.l	d0-d7/a0,-(sp)
	move.w	#192,d4
	moveq.l	#6,d5
	moveq.l	#1,d6
	moveq.l	#16,d7
	lea.l	Windows_corners,a0
	movem.l	d0/d1,-(sp)
	jsr	Put_masked_block		; Draw top-right corner
	lea.l	192(a0),a0
	add.w	#16,d0			; Draw top edge
	addq.w	#4,d1
	sub.w	#34,d2
	jsr	Draw_window_hedge
	add.w	d2,d0			; Draw top-left corner
	subq.w	#4,d1
	jsr	Put_masked_block
	lea.l	192(a0),a0
	add.w	#9,d0			; Draw right edge
	add.w	#16,d1
	sub.w	#32,d3
	jsr	Draw_window_vedge
	movem.l	(sp)+,d0/d1
	addq.w	#4,d0			; Draw left edge
	add.w	#16,d1
	subq.w	#2,d3
	jsr	Draw_window_vedge
	subq.w	#4,d0			; Draw bottom-right corner
	add.w	d3,d1
	jsr	Put_masked_block
	lea.l	192(a0),a0
	add.w	#16,d0			; Draw bottom edge
	add.w	#9,d1
	addq.w	#2,d2
	jsr	Draw_window_hedge
	add.w	d2,d0			; Draw bottom-right corner
	sub.w	#7,d1
	jsr	Put_masked_block
	movem.l	(sp)+,d0-d7/a0
	rts

; [ Draw a horizontal edge of a window-border ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Width (.w)
Draw_window_hedge:
	movem.l	d0-d7/a0/a1,-(sp)
	move.w	d0,d3			; Save
	lea.l	Windows_H,a1
	move.w	#192,d4
	moveq.l	#6,d5
	moveq.l	#1,d6
	moveq.l	#5,d7
	move.w	d2,-(sp)
	lsr.w	#4,d2			; Do whole blocks
	bra.s	.Entry
.Loop:	move.l	a1,a0			; Select graphics
	jsr	Random
	and.w	#$0007,d0
	cmp.w	#4,d0
	bpl.s	.No
	addq.w	#1,d0
	mulu.w	#60,d0
	add.w	d0,a0
.No:	move.w	d3,d0			; Display
	jsr	Put_masked_block
	add.w	#16,d3			; Next
.Entry:	dbra	d2,.Loop
	move.w	(sp)+,d2
	and.w	#$000f,d2			; Do remainder
	beq	.Exit			; If any
	move.l	a1,a0
	move.w	d3,d0
	sub.w	#16,d2
	add.w	d2,d0
	jsr	Put_masked_block
.Exit:	movem.l	(sp)+,d0-d7/a0/a1
	rts

; [ Draw a vertical edge of a window-border ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d3 - Height (.w)
Draw_window_vedge:
	movem.l	d0-d7/a0/a1,-(sp)
	move.w	d0,d2			; Save
	lea.l	Windows_V,a1
	move.w	#192,d4
	moveq.l	#6,d5
	moveq.l	#1,d6
	moveq.l	#16,d7
	move.w	d3,-(sp)
	lsr.w	#4,d3			; Do whole blocks
	bra.s	.Entry
.Loop:	move.l	a1,a0			; Select graphics
	jsr	Random
	and.w	#$0007,d0
	cmp.w	#4,d0
	bpl.s	.No
	addq.w	#1,d0
	mulu.w	#192,d0
	add.w	d0,a0
.No:	move.w	d2,d0			; Display
	jsr	Put_masked_block
	add.w	#16,d1			; Next
.Entry:	dbra	d3,.Loop
	move.w	(sp)+,d7
	and.w	#$000f,d7			; Do remainder
	beq	.Exit			; If any
	move.l	a1,a0
	move.w	d2,d0
	jsr	Put_masked_block
.Exit:	movem.l	(sp)+,d0-d7/a0/a1
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Window_class:
	dc.l 0
	dc.w Window_object_size
	Method Init,Init_window
	Method Exit,Exit_window
	dc.w -1
Text_window_class:
	dc.l Window_class
	dc.w Window_object_size
	Method Init,Init_text_window
	Method Exit,Exit_text_window
	Method Draw,Draw_text_window
	Method Print,Print_text_window
	dc.w -1
