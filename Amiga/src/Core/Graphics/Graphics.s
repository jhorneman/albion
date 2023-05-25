; Graphics routines
; Main
; Written by Jurie Horneman (In Tune With The Universe)
; Start : 4-2-1994

	XDEF	Begin_graphics_ops
	XDEF	End_graphics_ops
	XDEF	Push_CA
	XDEF	Pop_CA
	XDEF	Reset_CA_stack
	XDEF	Coord_convert
	XDEF	CA_Sp
	XDEF	Put_unmasked_icon
	XDEF	Put_masked_icon
	XDEF	Put_unmasked_block
	XDEF	Put_masked_block
	XDEF	Put_masked_block2
	XDEF	Put_masked_silhouette
	XDEF	Put_line_buffer
	XDEF	Put_line_buffer_shadow
	XDEF	Duplicate_box
	XDEF	Blend_ebe
	XDEF	Get_block
	XDEF	VScroll_block
	XDEF	Display_chunky_block
	XDEF	Plot_pixel
	XDEF	Get_pixel
	XDEF	Draw_box
	XDEF	Draw_vline
	XDEF	Draw_hline
	XDEF	Darker_box
	XDEF	Brighter_box
	XDEF	Marmor_box
	XDEF	Recolour_box

	SECTION	Program,code
;***************************************************************************
; [ Begin graphics operations ]
; All registers are restored
;***************************************************************************
Begin_graphics_ops:
	movem.l	d0/d1/a0/a1,-(sp)
	tst.b	Graphics_ops
	bne.s	.Exit
	kickGFX	OwnBlitter
	st	Graphics_ops
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;***************************************************************************
; [ End graphics operations ]
; All registers are restored
;***************************************************************************
End_graphics_ops:
	movem.l	d0/d1/a0/a1,-(sp)
	tst.b	Graphics_ops
	beq.s	.Exit
	kickGFX	DisownBlitter
	sf	Graphics_ops
.Exit:	movem.l	(sp)+,d0/d1/a0/a1
	rts

;*****************************************************************************
; [ Push a CA on the stack ]
;   IN : a0 - Pointer to CA (.l)
; All registers are	restored
;*****************************************************************************
Push_CA:
	move.l	a1,-(sp)
	movea.l	CA_Sp,a1
	addq.l	#4,a1
	cmpa.l	#CAStack_end,a1		; Possible ?
	beq.s	.Exit
	move.l	a0,(a1)			; Push
	move.l	a1,CA_Sp
.Exit:	movea.l	(sp)+,a1
	rts

;*****************************************************************************
; [ Pop a CA from the stack ]
; All registers are	restored
;*****************************************************************************
Pop_CA:
	move.l	a0,-(sp)
	movea.l	CA_Sp,a0
	cmpa.l	#CAStack_start,a0		; Possible ?
	beq.s	.Exit
	subq.l	#4,a0			; Pop
	move.l	a0,CA_Sp
.Exit:	movea.l	(sp)+,a0
	rts

;*****************************************************************************
; [ Reset the CA stack ]
; All registers are	restored
;*****************************************************************************
Reset_CA_stack:
	move.l	a0,-(sp)
	lea.l	CAStack_start,a0		; Reset stack
	move.l	a0,CA_Sp
	move.l	#Default_CA,(a0)
	move.l	(sp)+,a0
	rts

;***************************************************************************
; [ Convert coordinate pair to screen offset & pixel number ]
;   IN : d0 - X-coordinate (.w)
;        d1 - Y-coordinate (.w)
;  OUT : d2 - Screen offset (.l)
;        d3 - Pixel number (.w)
; Changed registers : d2,d3
;***************************************************************************
Coord_convert:
	move.l	d0,-(sp)
	move.w	d1,d2
	mulu.w	#Bytes_per_line,d2
	moveq.l	#$f,d3
	and.w	d0,d3
	sub.w	d3,d0
	lsr.w	#3,d0
	ext.l	d0
	add.l	d0,d2
	move.l	(sp)+,d0
	rts

;***************************************************************************	
; The other sources
;***************************************************************************	

	incdir	DDT:
	include	Core/Graphics/Graphics_vector.s
	include	Core/Graphics/Graphics_bitmap.s
	include	Core/Graphics/Graphics_copy.s
	include	Core/Graphics/Graphics_mask.s
	include	Core/Graphics/Graphics_recolour.s

;***************************************************************************	
; The DATA & BSS segments	
;***************************************************************************	
	SECTION	Fast_DATA,data
Default_CA:
	dc.w 0,Screen_width-1
	dc.w 0,Screen_height-1


	SECTION	Fast_BSS,bss
Graphics_ops:	ds.b 1
Mask_buffer_handle:	ds.b 1
	even
CA_Sp:	ds.l 1				; CA stack
CAStack_start:        
	ds.l Max_CA
CAStack_end:
