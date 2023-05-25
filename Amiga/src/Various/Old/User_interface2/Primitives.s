; Graphics primitives
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 5-3-1994

	XDEF	Draw_symbol
	XDEF	Draw_rectangle
	XDEF	Draw_high_border
	XDEF	Draw_deep_border
	XDEF	Draw_normal_box
	XDEF	Draw_high_box
	XDEF	Draw_deep_box

	SECTION	Program,code
;***************************************************************************	
; [ Draw a symbol ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        a0 - Pointer to Symbol data (.l)
; All registers are restored
;***************************************************************************	
Draw_symbol:
	movem.l	d4-d7/a0/a1,-(sp)
	move.l	a0,a1
	Get	Symbol_gfx_handle(a1),a0	; Get graphics address
	add.l	Symbol_offset(a1),a0
	move.w	Symbol_base_colour(a1),d4	; Get symbol data
	moveq.l	#0,d5
	move.b	Symbol_depth(a1),d5
	move.w	Symbol_width(a1),d6
	move.w	Symbol_height(a1),d7
	jsr	Put_masked_block
	Free	Symbol_gfx_handle(a1)	; Free pointer
	movem.l	(sp)+,d4-d7/a0/a1
	rts

;***************************************************************************	
; [ Draw a rectangle ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of rectangle (.w)
;        d3 - Height of rectangle (.w)
;        d4 - Colour (.w)
; All registers are restored
;***************************************************************************	
Draw_rectangle:
	movem.l	d0-d3/d6/d7,-(sp)
	move.w	d2,d6			; Save width & height
	move.w	d3,d7
	subq.w	#1,d6
	subq.w	#1,d7
	add.w	d0,d2			; Top H-line
	subq.w	#1,d2
	move.w	d1,d3
	jsr	Draw_hline
	move.w	d2,d0			; Right V-line
	add.w	d7,d3
	jsr	Draw_vline
	move.w	d3,d1			; Bottom H-line
	sub.w	d6,d2
	jsr	Draw_hline
	move.w	d2,d0			; Left V-line
	sub.w	d7,d3
	jsr	Draw_vline
	movem.l	(sp)+,d0-d3/d6/d7
	rts

;***************************************************************************	
; [ Draw a high border ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of border (.w)
;        d3 - Height of border (.w)
; All registers are restored
;***************************************************************************	
Draw_high_border:
	movem.l	d0-d5,-(sp)
	move.w	d0,d5			; Save
	move.w	#Brightest,d4		; Top-left pixel
	jsr	Plot_pixel
	addq.w	#1,d0			; Top line
	add.w	d0,d2
	subq.w	#3,d2
	move.w	#Brighter,d4
	jsr	Draw_hline
	move.w	d2,d0			; Top-right pixel
	addq.w	#1,d0
	move.w	#Normal,d4
	jsr	Plot_pixel
	move.w	d5,d0			; Left line
	addq.w	#1,d1
	add.w	d1,d3
	subq.w	#3,d3
	move.w	#Brighter,d4
	jsr	Draw_vline
	move.w	d2,d0			; Right line
	addq.w	#1,d0
	move.w	#Darker,d4
	jsr	Draw_vline
	move.w	d5,d0			; Bottom-left pixel
	move.w	d3,d1
	addq.w	#1,d1
	move.w	#Normal,d4
	jsr	Plot_pixel
	addq.w	#1,d0			; Bottom line
	addq.w	#1,d2
	move.w	#Darker,d4
	jsr	Draw_hline
	movem.l	(sp)+,d0-d5
	rts

;***************************************************************************	
; [ Draw a deep border ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of border (.w)
;        d3 - Height of border (.w)
; All registers are restored
;***************************************************************************	
Draw_deep_border:
	movem.l	d0-d5,-(sp)
	move.w	d0,d5			; Save
	add.w	d0,d2			; Top line
	subq.w	#2,d2
	move.w	#Darker,d4
	jsr	Draw_hline
	move.w	d2,d0			; Top-right pixel
	addq.w	#1,d0
	move.w	#Normal,d4
	jsr	Plot_pixel
	move.w	d5,d0			; Left line
	addq.w	#1,d1
	add.w	d1,d3
	subq.w	#3,d3
	move.w	#Darker,d4
	jsr	Draw_vline
	move.w	d2,d0			; Right line
	addq.w	#1,d0
	move.w	#Bright,d4
	jsr	Draw_vline
	move.w	d5,d0			; Bottom-left pixel
	move.w	d3,d1
	addq.w	#1,d1
	move.w	#Normal,d4
	jsr	Plot_pixel
	addq.w	#1,d0			; Bottom line
	addq.w	#1,d2
	move.w	#Bright,d4
	jsr	Draw_hline
	movem.l	(sp)+,d0-d5
	rts

;***************************************************************************	
; [ Draw a normal box ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of box (.w)
;        d3 - Height of box (.w)
; All registers are restored
;***************************************************************************	
Draw_normal_box:
	movem.l	d0-d3,-(sp)
	jsr	Draw_high_border
	add.w	d0,d2			; Main box
	subq.w	#3,d2
	add.w	d1,d3
	subq.w	#3,d3
	addq.w	#1,d0
	addq.w	#1,d1
	jsr	Marmor_box
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Draw a high box ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of box (.w)
;        d3 - Height of box (.w)
; All registers are restored
;***************************************************************************	
Draw_high_box:
	movem.l	d0-d3,-(sp)
	jsr	Draw_high_border
	add.w	d0,d2			; Main box
	subq.w	#3,d2
	add.w	d1,d3
	subq.w	#3,d3
	addq.w	#1,d0
	addq.w	#1,d1
	jsr	Brighter_box
	movem.l	(sp)+,d0-d3
	rts

;***************************************************************************	
; [ Draw a deep box ]
;   IN : d0 - Top-left X-coordinate (.w)
;        d1 - Top-left Y-coordinate (.w)
;        d2 - Width of box (.w)
;        d3 - Height of box (.w)
; All registers are restored
;***************************************************************************	
Draw_deep_box:
	movem.l	d0-d3,-(sp)
	jsr	Draw_deep_border
	add.w	d0,d2			; Main box
	subq.w	#3,d2
	add.w	d1,d3
	subq.w	#3,d3
	addq.w	#1,d0
	addq.w	#1,d1
	jsr	Darker_box
	movem.l	(sp)+,d0-d3
	rts
