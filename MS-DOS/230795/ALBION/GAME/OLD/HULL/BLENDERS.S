; Blender routines
; Written by J.Horneman (In Tune With The Universe)
; Start : 7-4-1994

	XDEF	Horizontal_wipe
	XDEF	Vertical_wipe
	XDEF	Random_wipe

	SECTION	Program,code	
;*****************************************************************************
; [ Horizontal wipe ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Width of area in ebes (.w)
;        d3 - Height of area in ebes (.w)
; All registers are restored
;*****************************************************************************
Horizontal_wipe:
	movem.l	d0-d7,-(sp)
	ext.l	d4			; Store speed
	divu.w	d3,d4
	move.w	d4,Wipe_speed
.Again:	jsr	My_vsync			; Do some rows
	move.w	Wipe_speed,d4
	subq.w	#1,d4
.Loop_X:	move.l	d1,d5			; Do one column
	move.w	d3,d6
	subq.w	#1,d6
.Loop_Y:	jsr	Blend_ebe
	addq.w	#8,d1
	dbra	d6,.Loop_Y
	subq.w	#1,d2			; Count down
	beq.s	.Exit
	move.w	d5,d1			; Next column
	addq.w	#8,d0
	dbra	d4,.Loop_X
.Exit:	movem.l	(sp)+,d0-d7
	rts

;*****************************************************************************
; [ Vertical wipe ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Width of area in ebes (.w)
;        d3 - Height of area in ebes (.w)
;        d4 - Wipe speed (.w)
; All registers are restored
;*****************************************************************************
Vertical_wipe:
	movem.l	d0-d7,-(sp)
	ext.l	d4			; Store speed
	divu.w	d2,d4
	move.w	d4,Wipe_speed
.Again:	jsr	My_vsync			; Do some rows
	move.w	Wipe_speed,d4
	subq.w	#1,d4
.Loop_Y:	move.l	d0,d5			; Do one row
	move.w	d2,d6
	subq.w	#1,d6
.Loop_X:	jsr	Blend_ebe
	addq.w	#8,d0
	dbra	d6,.Loop_X
	subq.w	#1,d3			; Count down
	beq.s	.Exit
	move.w	d5,d0			; Next row
	addq.w	#8,d1
	dbra	d4,.Loop_Y
	bra.s	.Again
.Exit:	movem.l	(sp)+,d0-d7
	rts

;*****************************************************************************
; [ Random wipe ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;        d2 - Width of area in ebes (.w)
;        d3 - Height of area in ebes (.w)
; All registers are restored
;*****************************************************************************
Random_wipe:
	movem.l	d0-d7/a0,-(sp)
	move.w	d0,Random_X		; Store coordinates
	move.w	d1,Random_Y
	move.w	d4,Wipe_speed		; Store speed
	lea.l	Random_map,a0		; Clear map
	move.l	#(((Screen_width/8)*(Screen_height/8))+7)/8,d0
	moveq.l	#0,d1
	jsr	Fill_memory
	move.w	d2,d7			; Calculate counter
	mulu.w	d3,d7
.Again:	jsr	My_vsync			; Do some ebes
	move.w	Wipe_speed,d6
	subq.w	#1,d6
.Loop:	jsr	Random			; Choose random Y
	mulu.w	d3,d0
	swap	d0
	move.w	d0,d1
	jsr	Random			; Choose random X
	mulu.w	d2,d0
	swap	d0
	move.w	d1,d4			; Calculate index
	mulu.w	d2,d4
	add.w	d0,d4
	moveq.l	#7,d5
	and.w	d4,d5
	lsr.w	#3,d4
	btst	d5,0(a0,d4.w)		; Already done this ?
	bne.s	.Loop
	bset	d5,0(a0,d4.w)		; No -> Set
	lsl.w	#3,d0			; Calculate coordinates
	add.w	Random_X,d0
	lsl.w	#3,d1
	add.w	Random_Y,d1
	jsr	Blend_ebe			; Blend
	subq.w	#1,d7			; Count down
	beq.s	.Exit
	dbra	d6,.Loop			; Next ebe
	bra.s	.Again
.Exit:	movem.l	(sp)+,d0-d7/a0
	rts

;*****************************************************************************
; The DATA & BSS segments
;*****************************************************************************
	SECTION	Fast_DATA,data
Wipe_speed:	ds.w 1
Random_X:	ds.w 1
Random_Y:	ds.w 1
Random_map:
	ds.b (((Screen_width/8)*(Screen_height/8))+7)/8
	even
