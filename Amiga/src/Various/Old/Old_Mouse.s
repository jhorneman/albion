; Mouse-pointer handling (display only)
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 31-1-1994

	MODULE	DDT_Mouse

	OUTPUT	DDT:Objects/Core/Mouse.o


	XREF	Mouse_X
	XREF	Mouse_Y
	XREF	Default_Mptr
	XREF	Graphics_base
	XREF	Intuition_base
	XREF	My_window
	XREF	Fatal_error
	XREF	My_vsync

	XDEF	Init_mouse
	XDEF	Exit_mouse
	XDEF	Mouse_on
	XDEF	Mouse_off
	XDEF	Reset_mouse
	XDEF	Push_MA
	XDEF	Pop_MA
	XDEF	Reset_MA_stack
	XDEF	Push_Mptr
;	XDEF	Push_busy_Mptr
	XDEF	Pop_Mptr
	XDEF	Reset_Mptr_stack
	XDEF	Change_Mptr
	XDEF	Draw_mouse
	XDEF	MA_Sp
	XDEF	Default_MA


	incdir	DDT:
	include	Constants/Global.i
	include	Constants/OS/OS.i
	include	Constants/Core.i


	SECTION	Program,code
;*****************************************************************************
; [ Switch the mouse on ]
; All registers are	restored
;*****************************************************************************
Mouse_on:
	tst.b	Suppress_mouse		; Already on ?
	beq.s	.Exit
	subq.b	#1,Suppress_mouse		; Decrease flag
	bne.s	.Exit			; Switch on ?
	jsr	My_vsync			; Yes -> Update position
	jsr	Draw_mouse
	move.l	a0,-(sp)			; Restore image
	move.l	Mptr_Sp,a0
	move.l	(a0),a0
	jsr	Init_Mptr
	move.l	(sp)+,a0
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
	jsr	Erase_Mptr		; Yes
.No:	addq.b	#1,Suppress_mouse		; Increase flag
.Exit:	rts

;*****************************************************************************
; [ Reset the mouse state ]
; All registers are	restored
;*****************************************************************************
Reset_mouse:
	clr.b	Suppress_mouse		; Turn it on
	rts
	
;*****************************************************************************
; [ Initialize the mouse ]
; All registers are	restored
;*****************************************************************************
Init_mouse:
	movem.l	d0-d3/a0-a2,-(sp)
	move.l	My_window,a0		; Set dummy Intuition pointer
	lea.l	Dummy_sprite,a1
	moveq.l	#1,d0
	moveq.l	#16,d1
	moveq.l	#0,d2
	moveq.l	#0,d3
	kickINTU	SetPointer
	lea.l	Mouse_sprite_1,a0		; Get 1st sprite
	moveq.l	#2,d0
	kickGFX	GetSprite
	cmp.w	#-1,d0			; Success ?
	bne.s	.Yes
	move.l	#GET_SPRITE_ERROR,d0	; No
	jmp	Fatal_error
.Yes:	move.w	d0,Mouse_sprite_1_nr
	LOCAL
	lea.l	Mouse_sprite_2,a0		; Get 2nd sprite
	moveq.l	#3,d0
	kickGFX	GetSprite
	cmp.w	#-1,d0
	bne.s	.Yes
	move.l	#GET_SPRITE_ERROR,d0	; No
	jmp	Fatal_error
.Yes:	move.w	d0,Mouse_sprite_2_nr
	LOCAL
	move.w	Mouse_X,d0		; Get mouse coordinates
	move.w	Mouse_Y,d1
	sub.w	HotSpot_X,d0		; Add hot-spot coordinates
	sub.w	HotSpot_Y,d1
	lea.l	Mouse_sprite_1,a0		; Set position and height
	move.w	d0,ss_x(a0)
	move.w	d1,ss_y(a0)
	move.w	#Mptr_height,ss_height(a0)
	lea.l	Mouse_sprite_2,a0
	move.w	d0,ss_x(a0)
	move.w	d1,ss_y(a0)
	move.w	#Mptr_height,ss_height(a0)
	sub.l	a0,a0			; Set images
	lea.l	Mouse_sprite_1,a1
	lea.l	Mouse_image_1,a2
	kickGFX	ChangeSprite
	sub.l	a0,a0
	lea.l	Mouse_sprite_2,a1
	lea.l	Mouse_image_2,a2
	move.w	2(a2),d0
	bset	#7,d0
	move.w	d0,2(a2)
	kickGFX	ChangeSprite
	movem.l	(sp)+,d0-d3/a0-a2
	rts

;*****************************************************************************
; [ Free the mouse ]
; All registers are	restored
;*****************************************************************************
Exit_mouse:
	movem.l	d0/d1/a0/a1,-(sp)
	st	Suppress_mouse		; Off
	move.w	Mouse_sprite_1_nr,d0	; Destroy sprites
	cmp.w	#-1,d0
	beq.s	.No
	kickGFX	FreeSprite
	move.w	#-1,Mouse_sprite_1_nr
.No:	LOCAL
	move.w	Mouse_sprite_2_nr,d0
	cmp.w	#-1,d0
	beq.s	.No
	kickGFX	FreeSprite
	move.w	#-1,Mouse_sprite_2_nr
.No:	LOCAL
	move.l	My_window,a0		; Restore Intuition pointer
	kickINTU	ClearPointer
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
	jsr	Init_Mptr			; Initialize new mouse
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
	jsr	Init_Mptr			; Initialize new mouse
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
	jsr	Init_Mptr
.Exit:	movea.l	(sp)+,a0
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
	jsr	Init_Mptr			; Initialize default mouse
	movem.l	(sp)+,a0/a1
	rts

;*****************************************************************************
; [ Initialize a mouse pointer ]
;   IN : a0 - Pointer to mouse pointer (.l)
; All registers are	restored
;*****************************************************************************
Init_Mptr:
	movem.l	d0/d1/d7/a0-a2,-(sp)
	tst.b	Suppress_mouse		; Off ?
	bne.s	.Exit
	move.w	(a0)+,HotSpot_X		; Set Hot Spot
	move.w	(a0)+,HotSpot_Y
	lea.l	Mouse_image_1+4,a1		; Create images
	lea.l	Mouse_image_2+4,a2
	moveq.l	#Mptr_height-1,d7
.Loop:	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a2)+
	dbra	d7,.Loop
	sub.l	a0,a0			; Set images
	lea.l	Mouse_sprite_1,a1
	lea.l	Mouse_image_1,a2
	kickGFX	ChangeSprite
	sub.l	a0,a0
	lea.l	Mouse_sprite_2,a1
	lea.l	Mouse_image_2,a2
	move.w	2(a2),d0
	bset	#7,d0
	move.w	d0,2(a2)
	kickGFX	ChangeSprite
.Exit:	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

;*****************************************************************************
; [ Initialize an empty mouse pointer ]
; All registers are	restored
;*****************************************************************************
Erase_Mptr:
	movem.l	d0/d1/d7/a0-a2,-(sp)
	clr.w	HotSpot_X			; Set Hot Spot
	clr.w	HotSpot_Y
	lea.l	Mouse_image_1+4,a1		; Create images
	lea.l	Mouse_image_2+4,a2
	moveq.l	#Mptr_height-1,d7
.Loop:	clr.l	(a1)+
	clr.l	(a2)+
	dbra	d7,.Loop
	sub.l	a0,a0			; Set images
	lea.l	Mouse_sprite_1,a1
	lea.l	Mouse_image_1,a2
	kickGFX	ChangeSprite
	sub.l	a0,a0
	lea.l	Mouse_sprite_2,a1
	lea.l	Mouse_image_2,a2
	move.w	2(a2),d0
	bset	#7,d0
	move.w	d0,2(a2)
	kickGFX	ChangeSprite
	movem.l	(sp)+,d0/d1/d7/a0-a2
	rts

;*****************************************************************************
; [ "Draw" the mouse pointer ]
; All registers are	restored
;*****************************************************************************
Draw_mouse:
	movem.l	d0-d3/a0/a1,-(sp)
	tst.b	Suppress_mouse		; Mouse on ?
	bne.s	.Exit
	move.w	Mouse_X,d2		; Get mouse coordinates
	move.w	Mouse_Y,d3
	sub.w	HotSpot_X,d2		; Add hot-spot coordinates
	sub.w	HotSpot_Y,d3
	subq.w	#1,d2			; OS bug correction
	ext.l	d2
	ext.l	d3
	move.l	d2,d0			; Move sprite 1
	move.l	d3,d1
	sub.l	a0,a0
	lea.l	Mouse_sprite_1,a1
	kickGFX	MoveSprite
	move.l	d2,d0			; Move sprite 2
	move.l	d3,d1
	sub.l	a0,a0
	lea.l	Mouse_sprite_2,a1
	kickGFX	MoveSprite
.Exit:	movem.l	(sp)+,d0-d3/a0/a1
	rts

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************
	SECTION	Fast_DATA,data
Suppress_mouse:	dc.b -1
	even
Default_MA:
	dc.w 0,Screen_width-1
	dc.w 0,Screen_height-1


	SECTION	Chip_BSS,bss_c
Dummy_sprite:
	ds.l 3
Mouse_image_1:
	ds.l Mptr_height+2
Mouse_image_2:
	ds.l Mptr_height+2


	SECTION	Fast_BSS,bss
Mouse_sprite_1:
	ds.b ss_data_size
Mouse_sprite_2:
	ds.b ss_data_size
Mouse_sprite_1_nr:	ds.w 1
Mouse_sprite_2_nr:	ds.w 1

HotSpot_X:	ds.w 1
HotSpot_Y:	ds.w 1

Mptr_Sp:	ds.l 1			; Mouse pointer stack
MptrStack_start:        
	ds.l Max_Mptr
MptrStack_end:
	
MA_Sp:	ds.l 1			; MA stack
MAStack_start:        
	ds.l Max_MA
MAStack_end:
