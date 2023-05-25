; Colour palette management
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	XDEF	Find_closest_colour
	XDEF	Set_full_palette
	XDEF	Set_palette_part
	XDEF	Current_palette

; Notes :
;   - The colour table needed by [ LoadRGB32 ] cannot be allocated because
;     this routine can be called in the Vbl queue. If memory becomes tight,
;     [ Set_palette_part ] can be adapted to split the colour list in
;     several smaller parts. However, this might look odd.

	SECTION	Program,code
;***************************************************************************	
; [ Find the closest colour in a palette ]
;   IN : d0 - Target R value (.w)
;        d1 - Target G value (.w)
;        d2 - Target B value (.w)
;        d7 - Number of colours (.l)
;        a0 - Pointer to IFF palette (.l)
;  OUT : d0 - Closest colour index (.w)
; Changed registers : d0
;***************************************************************************	
Find_closest_colour:
	movem.l	d1-d7/a0-a2,-(sp)
	move.l	a0,a2
	move.l	#$7fffffff,d6
	bra.s	.Entry
.Loop:	moveq.l	#0,d3			; Get colour
	move.b	(a0)+,d3
	moveq.l	#0,d4
	move.b	(a0)+,d4
	moveq.l	#0,d5
	move.b	(a0)+,d5
	sub.w	d0,d3			; Calculate differences
	sub.w	d1,d4
	sub.w	d2,d5
	muls.w	d3,d3			; Calculate total difference
	muls.w	d4,d4
	muls.w	d5,d5
	add.l	d4,d3
	add.l	d5,d3
	cmp.l	d6,d3			; New record ?
	bpl.s	.Entry
	move.l	d3,d6			; Yes
	move.l	a0,a1
.Entry:	dbra	d7,.Loop
	move.l	a1,d0			; Calculate index
	sub.l	a2,d0
	divu.w	#3,d0
	subq.w	#1,d0
.Exit:	movem.l	(sp)+,d1-d7/a0-a2
	rts

;***************************************************************************
; [ Set an entire 256-colour palette ]
;   IN : a0 - Pointer to IFF palette (.l)
; All registers are restored
;***************************************************************************
Set_full_palette:
	movem.l	d0/d1,-(sp)
	moveq.l	#0,d0
	move.w	#Pal_size,d1
	jsr	Set_palette_part
	movem.l	(sp)+,d0/d1
	rts

;***************************************************************************
; [ Set part of a palette ]
;   IN : d0 - First colour {0...255} (.w)
;        d1 - Number of colours {1...256} (.w)
;        a0 - Pointer to IFF colour list (.l)
; All registers are restored
; Notes :
;   - This routine will handle 0 colours but will cause a crash when the
;     number of colours is larger than 256.
;***************************************************************************
Set_palette_part:
	movem.l	d0/d1/a0-a2,-(sp)
	lea.l	Colour_table,a1		; Write header
	move.w	d1,(a1)+
	move.w	d0,(a1)+
	lea.l	Current_palette,a2
	add.w	d0,a2
	add.w	d0,d0
	add.w	d0,a2
	move.w	d1,d0			; Write colours
	add.w	d1,d1
	add.w	d0,d1
	bra.s	.Entry
.Loop:	move.b	(a0)+,d0			; Load 8-bit colour
	move.b	d0,(a1)+			; Store 32-bit colour
	move.b	d0,(a1)+
	move.b	d0,(a1)+
	move.b	d0,(a1)+
	move.b	d0,(a2)+			; Store 8-bit colour
.Entry:	dbra	d1,.Loop
	clr.w	(a1)			; Write sentinel
	move.l	My_screen,a0		; Set colours
	lea.l	sc_ViewPort(a0),a0
	lea.l	Colour_table,a1
	kickGFX	LoadRGB32
	movem.l	(sp)+,d0/d1/a0-a2
	rts

;***************************************************************************
; [ Get the current palette ]
;  OUT : a1 - Pointer to IFF colour list (.l)
; Changed registers : a1
;***************************************************************************
Get_full_palette:
	lea.l	Current_palette,a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_BSS,bss
Colour_table:
	ds.w 2			; Header
	ds.l 3*Pal_size		; 256 colours
	ds.w 1			; Sentinel

Current_palette:	ds.b 3*Pal_size
